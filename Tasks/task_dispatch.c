//////////////////////////////////////////////////////////////////////////////
//! \file task_dispatch.c
//!
//! \brief Contains task routines and the dispatcher
//!
//! The task manager and the dispatcher are seperated to make the task module
//!	more approachable.  This file handles all aspects of dispatching to and
//! executing a task, where the task manager exposes task characteristics.
//!
//!
//!
//////////////////////////////////////////////////////////////////////////////

#include "task.h" 	// task definitions
#include "msp430.h"	// MCU definitions
#include "rts.h"	// Scheduler
#include "time.h"		// Time keeping module
#include "std.h"		// standard definitions
#include "SP.h"			// Satellite processor driver
#include "serial.h"		// Serial communications driver
#include "report.h"		// Logging
#include "mem_mod.h" 	// memory module definitions
#include "misc.h"		// miscellaneous functions
#include "pmm.h"		// Power management module
#include "lnkblk.h"		// RF link routines
#include "main.h"		// For getting software version
#include "rand.h"
#include "delay.h"
#include "buz.h"
#include "adf7020.h"
#include "ucs.h"
#include "scc.h"
#include "flash_mcu.h"

extern volatile uint8 ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];
extern S_Task_Ctl p_saTaskList[MAXNUMTASKS];
extern uchar g_ucaCurrentTskIndex;

//////////////////////////////////////////////////////////////////////////
//! \fn vTask_Sleep(void)
//!
//! \brief Sets the MCU to deep sleep for the slot.
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////
void vTask_Sleep(void)
{

	//Sleep until the subslot warning alarm is triggered
	while (ucTimeCheckForAlarms(SUBSLOT_WARNING_ALARM_BIT) == 0)
		LPM3;

}

//////////////////////////////////////////////////////////////////////////
//! \fn vTask_ModifyTCB(void)
//!
//! \brief Handles remote modification of task control block parameters
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////
void vTask_ModifyTCB(void)
{
	uchar ucCmdParam[MAXCMDPARAM];
	uchar ucTaskIndex, ucErrCode;
	uint uiTaskID;
	uchar ucParamID, ucMsgIndex, ucByteCount;
	ulong ulNewVal,ulCmdLength;

	// Assume no errors
	ucErrCode = 0;

	// Load the command parameters
	if (ucTask_GetCmdParam(g_ucaCurrentTskIndex, ucCmdParam) != TASKMNGR_OK)
		ucErrCode = SRC_ID_MODIFYTCB_FAIL;

	// Get the length of the command
	if(ucTask_GetField(g_ucaCurrentTskIndex, TSK_CMDLENGTH, &ulCmdLength) != TASKMNGR_OK)
		ucErrCode = SRC_ID_MODIFYTCB_FAIL;

	if (ucErrCode == 0) {
		// Get the ID for the task being modified
		uiTaskID = (uint) (ucCmdParam[0] << 8) | ucCmdParam[1];

		// Get the parameter being modified
		ucParamID = ucCmdParam[2];

		// For now, only accept 32-bits
		if(ulCmdLength>7)
			ulCmdLength = 7;

		ulNewVal = 0;

		// Get the new value
		for (ucByteCount = 3; ucByteCount < ulCmdLength; ucByteCount++) {
			ulNewVal  = ulNewVal << 8;
			ulNewVal |= (ulong) ucCmdParam[ucByteCount];
		}

		// Fetch the index of the task being modified
		ucTaskIndex = ucTask_FetchTaskIndex(uiTaskID);

		// If the task does not exist in the list set the error code
		if (ucTaskIndex == INVALID_TASKINDEX) {
			ucErrCode = SRC_ID_MODIFYTCB_FAIL;
		}
		else {
			// Modify the parameter if allowed
			if (ucTask_SetField(ucTaskIndex, ucParamID, ulNewVal) != TASKMNGR_OK)
				ucErrCode = SRC_ID_MODIFYTCB_FAIL;
		}
	}

	// If there was an error then generate the report
	if (ucErrCode != 0) {
		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 5, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		// Load the rest of the DE
		ucaMSG_BUFF[ucMsgIndex++] = ucErrCode;
		ucaMSG_BUFF[ucMsgIndex++] = 3; // data length
		ucaMSG_BUFF[ucMsgIndex++] = ucCmdParam[0]; // TaskID Hi
		ucaMSG_BUFF[ucMsgIndex++] = ucCmdParam[1]; // TaskID Lo
		ucaMSG_BUFF[ucMsgIndex] = ucParamID; // ParamID

		// Store DE
		vReport_LogDataElement(RPT_PRTY_MODIFYTCB_FAIL);
	}
}

//////////////////////////////////////////////////////////////////////////
//! \fn vTask_FRAM_to_SDCard(void)
//!
//! \brief Writes the reports from the SD Card buffer in FRAM to the SD Card
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////
void vTask_FRAM_to_SDCard(void)
{
	uchar ucBlock[SD_CARD_BLOCKLEN];
	uchar ucAttemptCount;
	ulong ulAddress;
	uint uiCount;
	uchar ucErrorCode;
	uchar ucErrorCodePriority;
	uchar ucMsgIndex;
	const uchar ucTimeout = 50;

	// Get the next free SD card address from FRAM
	ulAddress = ulL2FRAM_GetSDCardBlockNum();

	// Read the contents of the buffer to the local block
	vL2FRAM_ReadSDCardBuffer(ucBlock);

	// If at first you don't succeed.....
	ucAttemptCount = 5;

	// Try 5 times or until the subslot ends, whichever comes first
	while (ucAttemptCount-- > 0 && (ucTimeCheckForAlarms(SUBSLOT_WARNING_ALARM_BIT) == 0)) {

		// Assume success
		ucErrorCode = ucErrorCodePriority = 0;

		// Power up the SD card and run the initialization sequence
		vSD_PowerOn();

		// Try the initialization function a few times
		for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
			if (ucSD_Init() == SD_SUCCESS)
				break;
		}
		if (uiCount == ucTimeout) {
			ucErrorCode = SRC_ID_SDCARD_INIT_FAIL;
			ucErrorCodePriority = RPT_PRTY_SDCARD_INIT_FAIL;
#if 1
			vSERIAL_sout("SD Init Fail\r\n", 14);
#endif
		}

		// Only proceed if initialization was successful
		if (ucErrorCode == 0) {
			// Write the block to the SD card
			for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
				if (SD_Write_Block(ucBlock, ulAddress) == SD_SUCCESS)
					break;
			}
			if (uiCount == ucTimeout) {
				ucErrorCode = SRC_ID_SDCARD_WRITE_FAIL;
				ucErrorCodePriority = RPT_PRTY_SDCARD_WRITE_FAIL;
#if 1
				vSERIAL_sout("SD write fail\r\n", 15);
#endif
			}
			else{
				//Power down the SD card and exit
				vSD_PowerOff();
				break;
			}
		}

		//Power down the SD card
		vSD_PowerOff();

		// Allow time for the voltage to decay
		vDELAY_LPMWait1us(5000, 1);

	} // END: while(ucAttemptCount)


	// If there was an error...
	if (ucErrorCode != 0) {

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;
		ucaMSG_BUFF[ucMsgIndex++] = ucErrorCode;
		ucaMSG_BUFF[ucMsgIndex++] = 0; // data length

		// Store DE
		vReport_LogDataElement(ucErrorCodePriority);

	}
	else {

		// Update the SD card address
		vL2FRAM_IncrementSDCardBlockNum();

		// Write the messages in the overflow area to the buffer
		vL2FRAM_SDCardOvrflowToBuff();
	}

}

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_showBattReading(void)
//!
//! \brief Performs the battery voltage for display only.  Does not generate
//!				 a data element
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_showBattReading(void)
{
	uint uiBattReading_in_mV;
	uint uiVolts_whole;
	uint uiVolts_fraction;

	uiBattReading_in_mV = uiMISC_doCompensatedBattRead();

	uiVolts_whole = uiBattReading_in_mV / 1000;
	vSERIAL_UIV16out(uiVolts_whole);
	vSERIAL_bout('.');

	uiVolts_fraction = uiBattReading_in_mV - (uiVolts_whole * 1000);
	vSERIAL_UIV16out(uiVolts_fraction);
	vSERIAL_sout("V", 1);

	return;

}/* END: vTask_showBattReading() */

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_Batt_Sense(void)
//!
//! \brief Performs the battery voltage sensing operation
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_Batt_Sense(void)

{
	uint uiBattReading_in_mV;
	uchar ucMsgIndex;

	/* GET THE BATTERY READING */
	uiBattReading_in_mV = uiMISC_doCompensatedBattRead();

	// Build the report data element header
	vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;

	// If the input voltage is low then both internal and external power supplies are failing
	// When this happens generate a report, store all data elements to SRAM, do an emergency write
	// to the SD card.
	if ((uiBattReading_in_mV < 2900) && (ucPMM_chkLowVoltage(BATT_LOW_V300) != 0)) {

		vSERIAL_sout("BattBd\r\n", 8);
		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_INPUT_VOLTAGE_LOW;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiBattReading_in_mV >> 8);
		ucaMSG_BUFF[ucMsgIndex] = (uchar) uiBattReading_in_mV;

		vReport_LogDataElement(RPT_PRTY_INPUT_VOLTAGE_LOW); // Store DE
		vReport_BuildMsgsFromDEs(); // Build message
		vTask_FRAM_to_SDCard(); // Emergency write to the SD card

		// Set the byte that indicates that the shutdown was bad
		vL2FRAM_SetStateOnShutdown(0x01);

		// Reset the device.
		vPMM_Reset();

	}
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) SRC_ID_INPUT_VOLTAGE;
	ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiBattReading_in_mV >> 8);
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiBattReading_in_mV;

	// Store DE
	vReport_LogDataElement(RPT_PRTY_INPUT_VOLTAGE);

#if 0
	vSERIAL_sout("  Batt= ", 8);
	vTask_showBattReading();
	vSERIAL_crlf();
#endif

}/* END: vTask_do_Batt_Sense() */

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_MCUTemp(void)
//!
//! \brief Measures the processors temperature using it's ADC
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_MCUTemp(void)
{
	uint uiTempRaw;
	uint uiTempCal;
	uchar ucReadingCount;
	uchar ucMsgIndex;
	ulong lVal;

	lVal = 0;

	// Take several samples and then average
	for (ucReadingCount = 0; ucReadingCount < 10; ucReadingCount++) {
		lVal += (long) uiAD_full_init_setup_read_and_shutdown(INTERNAL_TEMP);
	}

	uiTempRaw = (uint) (lVal / 10);

	uiTempCal = uiAD_CalibrateTemp(uiTempRaw);

	// Build the report data element header
	vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;

	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_INTERNAL_TEMP;
	ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiTempCal >> 8);
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiTempCal;

	// Store DE
	vReport_LogDataElement(RPT_PRTY_INTERNAL_TEMP);

}/* END: vTask_InternalTemp() */

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_ReportHID(void)
//!
//! \brief Reports the HID of all boards attached to the WiSARD
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_ReportHID(void)
{
	uchar ucSPIndex, ucSPNumber;
	uchar ucMsgIndex;
	signed char cByteCount;
	uint uiHID[4];
	uchar ucSP_HID[8];

	/******************** CP Board *********************/
	// Start the payload length off at the length of the CP HID and the message pointer to the payload
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;

	// Add the report ID field to the message
	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RPT_HID;

	// Add the length field
	ucaMSG_BUFF[ucMsgIndex++] = 8;

	// Get the CP HID and copy it to the message buffer
	vFlash_GetHID(uiHID);
	cByteCount = 4;
	while(cByteCount-- > 0){
		ucaMSG_BUFF[ucMsgIndex++] = (uchar)(uiHID[(uchar)cByteCount] >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar)uiHID[(uchar)cByteCount];
	}

	// Build the report data element header and store it
	vComm_DE_BuildReportHdr(CP_ID, 10, ucMAIN_GetVersion());
	vReport_LogDataElement(RPT_PRTY_RPT_HID);

	/******************* Radio Board ********************/
	// Report the radio HID
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;

	// Add the report ID field to the message
	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RPT_HID;

	// Add the length field
	ucaMSG_BUFF[ucMsgIndex++] = 8;

	// Get the radio HID and copy it to the message buffer
	vFlash_GetRadioHID(uiHID);
	cByteCount = 4;
	while(cByteCount-- > 0){
		ucaMSG_BUFF[ucMsgIndex++] = (uchar)(uiHID[(uchar)cByteCount] >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar)uiHID[(uchar)cByteCount];
	}

	// Build the report data element header and store it
	vComm_DE_BuildReportHdr(RADIO_BOARD, 10, ucMAIN_GetVersion());
	vReport_LogDataElement(RPT_PRTY_RPT_HID);

	/******************* SP Boards ********************/
	// Load the SP board HIDs
	for(ucSPIndex = 0; ucSPIndex < NUMBER_SPBOARDS; ucSPIndex++){
		if(ucSP_IsAttached(ucSPIndex)){

			// SP indexing starts at 0 (subsequently ruining my ability to count like a normal human)
			ucSPNumber = ucSPIndex + 1;

			// Start the message index at the payload
			ucMsgIndex = DE_IDX_RPT_PAYLOAD;

			// Add the report ID field to the message
			ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RPT_HID;

			// Add the length field
			ucaMSG_BUFF[ucMsgIndex++] = 8;

			// Get the HID
			ucSP_GetHID(ucSPIndex, ucSP_HID);

			// Copy the SP hardware ID to the message buffer
			cByteCount = 8;
			while(cByteCount-- > 0)
				ucaMSG_BUFF[ucMsgIndex++] = ucSP_HID[(uchar)cByteCount];

			// Build the report data element header and store it
			vComm_DE_BuildReportHdr(ucSPNumber , 10, ucSP_FetchMsgVersion(ucSPIndex));
			vReport_LogDataElement(RPT_PRTY_RPT_HID);
		}
	}

	// Get the HID of the SCC board if it is present
	if(ucSCC_IsAttached()){
		ucSCC_GetHID(ucSP_HID);

		// Start the message index at the payload
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		// Add the report ID field to the message
		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RPT_HID;

		// Add the length field
		ucaMSG_BUFF[ucMsgIndex++] = 8;

		// Copy the SCC hardware ID to the message buffer
		cByteCount = 8;
		while(cByteCount-- > 0)
			ucaMSG_BUFF[ucMsgIndex++] = ucSP_HID[(uchar)cByteCount];

		// Build the report data element header and store it
		vComm_DE_BuildReportHdr(6 , 10, ucSCC_FetchMsgVersion());
		vReport_LogDataElement(RPT_PRTY_RPT_HID);
	}

}/* END: vTask_ReportHID() */

//////////////////////////////////////////////////////////////////////////
//! \fn vTask_SP_StartSlot(void)
//!
//! \brief Task check to see if any new SP boards are attached.  In the event
//!				 that an SP is added to the board and it is not reset, the CP will
//!				 eventually catch the new SP and incorporate it
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_SP_CheckBoards(void)
{
	uchar ucaSerialNumberOld[8];
	uchar ucaSerialNumberNew[8];
	uchar ucFoundNewSP;
	uchar ucSPCounter;
	uchar ucCounter;
	uchar ucMsgIndex;
	char cByteCount;
	S_Task_Ctl S_Task;

	// Get the serial number of the attached board (if there is one attached)
	// then restart the SP board and check if the old serial number and the new
	// are the different.  If they are different then delete the old tasks and
	// add the new ones

	ucFoundNewSP = 0; // assume the first SP board is known

	// Loop through the SP boards
	for (ucSPCounter = 0; ucSPCounter < NUMBER_SPBOARDS; ucSPCounter++) {

		// Make sure the SP is not busy with another task before proceeding
		if (ucSP_FetchSPState(ucSPCounter) == SP_STATE_INACTIVE) {
			ucSP_GetHID(ucSPCounter, ucaSerialNumberOld);
			ucSP_Start(ucSPCounter);
			vSP_TurnOff(ucSPCounter);
			// If the SP is different and the location wasn't previously empty then destroy the old tasks
			ucSP_GetHID(ucSPCounter, ucaSerialNumberNew);

			for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
				// If the HID does not match then this is a new board and it must be interrogated
				if (ucaSerialNumberOld[ucCounter] != ucaSerialNumberNew[ucCounter]) {
					ucFoundNewSP = 1;
					break;
				}
			}

			// If the SP is new then remove the previous one and set up the new one
			if (ucFoundNewSP) {
				ucTask_RemoveProcessor(ucSPCounter + 1); // Remove all old tasks associated with this processor
				ucTask_FetchSingleSPTasks(ucSPCounter); // Add all new tasks associated with this SP board
			}
		}
		ucFoundNewSP = 0; // assume the next SP board is known
	}

	ucSCC_GetHID(ucaSerialNumberOld);
	vSCC_Init();
	if (ucSCC_GetHID(ucaSerialNumberNew) == 0) {

		for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
			if (ucaSerialNumberOld[ucCounter] != ucaSerialNumberNew[ucCounter]) {

				// If we have a new SCC board then delete the old one
				ucTask_RemoveProcessor(6);

				// Start the message index at the payload
				ucMsgIndex = DE_IDX_RPT_PAYLOAD;

				// Add the report ID field to the message
				ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RPT_HID;

				// Add the length field
				ucaMSG_BUFF[ucMsgIndex++] = 8;

				// Copy the SCC hardware ID to the message buffer
				cByteCount = 8;
				while (cByteCount-- > 0)
					ucaMSG_BUFF[ucMsgIndex++] = ucaSerialNumberNew[(uchar) cByteCount];

				// Build the report data element header and store it
				vComm_DE_BuildReportHdr(6, 10, ucSCC_FetchMsgVersion());
				vReport_LogDataElement(RPT_PRTY_RPT_HID);

				// Add the SCC tasks if it is present
				if (ucSCC_IsAttached() == TRUE) {
					S_Task.m_uiTask_ID = 0x0601;
					S_Task.m_uiFlags = TASK_FLAGS_SP;
					S_Task.m_ucPriority = TASK_PRIORITY_SP;
					S_Task.m_ucProcessorID = 0x06;
					S_Task.m_ucState = TASK_STATE_ACTIVE;
					S_Task.m_ucWhoCanRun = RBIT_ALL;
					S_Task.m_ulTransducerID = 1L; //Trans 1
					S_Task.m_ulParameters[0x00] = (((ulong) ((7) * 5) << 24) | MINUTES_3);
					S_Task.m_ulParameters[0x01] = 0;
					S_Task.m_ulParameters[0x02] = ucSCC_GetSampleDuration();
					S_Task.m_ulParameters[0x03] = 0;
					S_Task.m_cName = "SCC  ";
					S_Task.ptrTaskHandler = vTask_SCC_StartSlot;
					ucTask_CreateTask(S_Task);

					S_Task.m_uiTask_ID = 0x0602;
					S_Task.m_uiFlags = (TASK_FLAGS_SP | F_SUSPEND);
					S_Task.m_ucPriority = (PRIORITY_5 | SCHED_CMD_SLOT);
					S_Task.m_ucProcessorID = 0x06; // Start with SP 1
					S_Task.m_ucState = TASK_STATE_IDLE;
					S_Task.m_ucWhoCanRun = RBIT_ALL;
					S_Task.m_ulTransducerID = 2L; //Trans 0 is test so add 1
					S_Task.m_ulParameters[0x00] = 0;
					S_Task.m_ulParameters[0x01] = 0;
					S_Task.m_ulParameters[0x02] = 0;
					S_Task.m_ulParameters[0x03] = 0;
					S_Task.m_cName = "SCC  ";
					S_Task.ptrTaskHandler = vTask_SCC_StartSlot;
					ucTask_CreateTask(S_Task);
				}

				break;
			}
		}
	}

}

//////////////////////////////////////////////////////////////////////////
//! \fn vTask_SP_StartSlot(void)
//!
//! \brief Task handles start of slot SP interaction
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_SP_StartSlot(void)
{

	uchar ucSPNumber = 0;
	uchar ucSPIndex;
	ulong ulTransID = 0;
	uchar ucCmdLength = 0;
	uchar ucCmdByteCount;
	uchar ucCmdParam[MAXCMDPARAM];
	uchar ucMsgIndex;
	uchar ucPayloadLength;
	uchar ucPayload[20];
	ulong ulProcID, ulCmdLength;

	// Get SPID (SP indexing starts at zero) and transducer ID if allowed
	if (ucTask_GetField(g_ucaCurrentTskIndex, TSK_PROCESSORID, &ulProcID) != TASKMNGR_OK)
		return;
	if (ucTask_GetField(g_ucaCurrentTskIndex, TSK_TRANSDUCERID, &ulTransID) != TASKMNGR_OK)
		return;
	ucSPNumber = (uchar) ulProcID;
	ucSPIndex = ucSPNumber - 1;

	// Get the length of the command parameters (if any)
	ucTask_GetField(g_ucaCurrentTskIndex, TSK_CMDLENGTH, &ulCmdLength);
	ucCmdLength = (uchar) ulCmdLength;

	// If there is a group of transducers then load each one individually into the payload
	ucPayloadLength = 0;
	while ((uchar) ulTransID != 0) {
		ucPayload[ucPayloadLength++] = (uchar) ulTransID;
		ulTransID = ulTransID >> 8;

		// Set the command parameters length value
		ucPayload[ucPayloadLength++] = ucCmdLength;
	}

	// If there are command parameters then load them now
	if (ucCmdLength != 0) {
		ucTask_GetCmdParam(g_ucaCurrentTskIndex, ucCmdParam);

		for (ucCmdByteCount = 0; ucCmdByteCount < ucCmdLength; ucCmdByteCount++) {
			ucPayload[ucPayloadLength++] = ucCmdParam[ucCmdByteCount];
		}
	}

	// Only run the SP_Start function if the SP is not already running
	if (ucSP_FetchSPState(ucSPIndex) == SP_STATE_INACTIVE) {
		// Wake up the SP
		if (ucSP_Start(ucSPIndex) != COMM_OK) {
			// The SP failed to start...report it

			// Build the report data element header
			vComm_DE_BuildReportHdr(CP_ID, 3, ucMAIN_GetVersion());
			ucMsgIndex = DE_IDX_RPT_PAYLOAD;

			ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SP_COMM_ERR;
			ucaMSG_BUFF[ucMsgIndex++] = 1; // data length
			ucaMSG_BUFF[ucMsgIndex++] = ucSPNumber;

			// Store DE
			vReport_LogDataElement(RPT_PRTY_SP_COMM_ERR);
		}
	}

	// Send the command
	if (ucSP_SendCommand(ucSPIndex, ucPayloadLength, ucPayload) != COMM_OK) {
		// A communication error occured with this SP.

		// Shutdown SP board
		vSP_TurnOff(ucSPIndex);

		// Set SP state to inactive
		vSP_SetSPState(ucSPIndex, SP_STATE_INACTIVE);

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 3, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SP_COMM_ERR;
		ucaMSG_BUFF[ucMsgIndex++] = 1; // data length
		ucaMSG_BUFF[ucMsgIndex++] = ucSPNumber;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_SP_COMM_ERR);
	}
	else // Command succeeded
	{
		// If command is a success then clear the command length
		ucTask_SetField(g_ucaCurrentTskIndex, TSK_CMDLENGTH, 0);

		// Set the state of the SP board to active
		vSP_SetSPState(ucSPIndex, SP_STATE_ACTIVE);
	}
}

//////////////////////////////////////////////////////////////////////////
//! \fn vTask_SCC_StartSlot
//! \brief Dispatches tasks to the SCC board
//////////////////////////////////////////////////////////////////////////
void vTask_SCC_StartSlot(void){

	uchar ucMsgIndex;
	uchar ucPayloadLength;
	uchar ucPayload[20];
	ulong ulTransID = 0;
	ulong ulCmdLength;
	uchar ucCmdLength = 0;
	uchar ucCmdByteCount;
	uchar ucCmdParam[MAXCMDPARAM];

	if (ucTask_GetField(g_ucaCurrentTskIndex, TSK_TRANSDUCERID, &ulTransID) != TASKMNGR_OK)
		return;

	// Get the length of the command parameters (if any)
	ucTask_GetField(g_ucaCurrentTskIndex, TSK_CMDLENGTH, &ulCmdLength);
	ucCmdLength = (uchar) ulCmdLength;

	// If there is a group of transducers then load each one individually into the payload
	ucPayloadLength = 0;
	ucPayload[ucPayloadLength++] = (uchar) ulTransID;

	// Set the command parameters length value
	ucPayload[ucPayloadLength++] = ucCmdLength;

	// If there are command parameters then load them now
	if (ucCmdLength != 0) {
		ucTask_GetCmdParam(g_ucaCurrentTskIndex, ucCmdParam);

		for (ucCmdByteCount = 0; ucCmdByteCount < ucCmdLength; ucCmdByteCount++) {
			ucPayload[ucPayloadLength++] = ucCmdParam[ucCmdByteCount];
		}
	}

	// Send the command, if success then set state to active else report failure
	if(ucSCC_SendCommand(ucPayloadLength, ucPayload) != COMM_OK){
		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 3, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SP_COMM_ERR;
		ucaMSG_BUFF[ucMsgIndex++] = 1; // data length
		ucaMSG_BUFF[ucMsgIndex++] = 6;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_SP_COMM_ERR);

		ucSCC_SetState(SP_STATE_INACTIVE);
	}
	else{
		ucSCC_SetState(SP_STATE_ACTIVE);
	}

}

//////////////////////////////////////////////////////////////////////////
//! \fn vTask_SCC_EndSlot
//! \brief Requests data from the SCC board
//////////////////////////////////////////////////////////////////////////
void vTask_SCC_EndSlot(void){

	uchar ucMsgIndex;
	uchar ucByteCount;
	union SP_DataMessage S_DataMsg;

	if(ucSCC_RequestData() != COMM_OK){
		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 3, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SP_COMM_ERR;
		ucaMSG_BUFF[ucMsgIndex++] = 1; // data length
		ucaMSG_BUFF[ucMsgIndex++] = 6;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_SP_COMM_ERR);
	}
	else{
		vSERIAL_sout("RX SP[", 6);
		vSERIAL_UIV8out(0x06);
		vSERIAL_sout("]\r\n", 3);

		// Read the SCC buffer into the the data message structure
		ucSCC_GrabMessageFromBuffer(&S_DataMsg);

		// Build the message header
		vComm_DE_BuildReportHdr(6, (S_DataMsg.fields.ucMsgSize - SP_HEADERSIZE), S_DataMsg.fields.ucMsgVersion);
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		//stuff the report into the data element
		for (ucByteCount = 0; ucByteCount < (S_DataMsg.fields.ucMsgSize - SP_HEADERSIZE); ucByteCount++) {
			ucaMSG_BUFF[ucMsgIndex++] = S_DataMsg.fields.ucaData[ucByteCount];
		}

		// Store DE
		vReport_LogDataElement(4);
	}


}


//////////////////////////////////////////////////////////////////////////
//! \fn vTask_SP_EndSlot(void)
//!
//! \brief Checks the SPs at end of slot and if one is active then request data
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_SP_EndSlot(void)
{
	uchar ucSPNumber;
	uchar ucSPIndex;
	uchar ucByteCount;
	uchar ucMsgIndex;
	ulong ulProcID;
	union SP_DataMessage S_DataMsg;

	// Get SP number if allowed
	if (ucTask_GetField(g_ucaCurrentTskIndex, TSK_PROCESSORID, &ulProcID) != TASKMNGR_OK)
		return;
	ucSPNumber = (uchar) ulProcID;

	// (SP indexing starts at zero)
	ucSPIndex = ucSPNumber - 1;

	if (ucSP_IsAttached(ucSPIndex)) // Is there an SP board attached in this location
			{
		if (ucSP_FetchSPState(ucSPIndex) == SP_STATE_ACTIVE) // Is the SP board active
		{
			// If we have data from the SP board then format it as a Data Element and store it in SRAM
			if (ucSP_RequestData(ucSPIndex) != COMM_OK) // Request data from the SP, if returns 0 then success
			{
				// A communication error occured when requesting data from this SP...report it

				// Shutdown SP board
				vSP_TurnOff(ucSPIndex);

				// Set SP state to inactive
				vSP_SetSPState(ucSPIndex, SP_STATE_INACTIVE);

				// Build the report data element header
				vComm_DE_BuildReportHdr(CP_ID, 3, ucMAIN_GetVersion());
				ucMsgIndex = DE_IDX_RPT_PAYLOAD;

				ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SP_COMM_ERR;
				ucaMSG_BUFF[ucMsgIndex++] = 1; // data length
				ucaMSG_BUFF[ucMsgIndex++] = ucSPNumber;

				// Store DE
				vReport_LogDataElement(RPT_PRTY_SP_COMM_ERR);
			}
			else // We have data from this SP...report it
			{
				vSERIAL_sout("RX SP[", 6);
				vSERIAL_UIV8out(ucSPNumber);
				vSERIAL_sout("]\r\n", 3);

				// Read the SP buffer into the the data message structure
				ucSP_GrabMessageFromBuffer(&S_DataMsg);

				if ((S_DataMsg.fields.ucFlags & SP_SHUTDOWN_BIT) != 0) {
					// Shutdown SP board
					vSP_TurnOff(ucSPIndex);

					// Set SP state to inactive
					vSP_SetSPState(ucSPIndex, SP_STATE_INACTIVE);
				}

				// Build the message header
				vComm_DE_BuildReportHdr((ucSPNumber), (S_DataMsg.fields.ucMsgSize - SP_HEADERSIZE), S_DataMsg.fields.ucMsgVersion);
				ucMsgIndex = DE_IDX_RPT_PAYLOAD;

				//stuff the report into the data element
				for (ucByteCount = 0; ucByteCount < (S_DataMsg.fields.ucMsgSize - SP_HEADERSIZE); ucByteCount++) {
					ucaMSG_BUFF[ucMsgIndex++] = S_DataMsg.fields.ucaData[ucByteCount];
				}

				// Store DE
				vReport_LogDataElement(4);
			}

		} // END: if(SP active)
	} // END: if(rMAXNUMTASKSequest data)
}

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_RuntimeRadioDiag
//! \brief Performs a diagnostic on the radio and reports errors
/////////////////////////////////////////////////////////////////////////
void vTask_RuntimeRadioDiag(void){
	uchar ucReturnCode, ucReturnLength, ucaRetData[10];
	uchar ucMsgIndex, ucIndex;

	vADF7020_RunTimeDiagnostic(&ucReturnCode, &ucReturnLength, ucaRetData);

	// Build the report data element header and message
	vComm_DE_BuildReportHdr(CP_ID, (ucReturnLength + 2), ucMAIN_GetVersion());
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;

	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RDIO_DIAG;
	ucaMSG_BUFF[ucMsgIndex++] = ucReturnLength; // data length
	ucaMSG_BUFF[ucMsgIndex++] = ucReturnCode;
	ucIndex = 0;
	while(ucIndex < ucReturnLength){
		ucaMSG_BUFF[ucMsgIndex++] = ucaRetData[ucIndex++];
	}
	// Store DE
	vReport_LogDataElement(RPT_PRTY_RDIO_DIAG);

}

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_RSSI(void)
//!
//! \brief Does nothing, this function only exists to satisfy the requirement
//! that the task control block has a function to handle the task.
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_RSSI(void)
{
 __no_operation();
}

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_Reset(void)
//!
//! \brief Resets the WiSARD using the power management module
//!
//! While it may seem poor form to call a single function from here
//! it maintains the layered architecture, allowing changes in the HAL to
//! have a minimal effect on the rest of the system.
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_Reset(void)
{
	// Set the task to idle
	ucTask_SetField(g_ucaCurrentTskIndex, TSK_STATE, TASK_STATE_IDLE);

	// Pull the power on reset bit high to reset
	vPMM_Reset();
}

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_VerifyChangeableCommand(void)
//!
//! \brief prior to changing or overwriting any task parameters such as
//!		   sampling intervals, rssi, or whatever, this method checks to
//!		   ensure that the parameter is changeable and that the intended
//!		   task is actually the task being changed. This method will
//!		   return to the invokation a permission parameter
//!
//! \param ucTaskIndex
//! \return ucMdfTskPrmsn
/////////////////////////////////////////////////////////////////////////
uchar ucTask_VerifyChangeableCommand(uchar ucTaskIndex)
{
	// define variables
	//uchar ucStBlkNum;

	// Get the location of the task in the start block
	//ucStBlkNum = ucL2FRAM_findStBlkTask(ucTaskIndex);

	// 1 - make sure task is active or idle
//	if(p_saTaskList[ucTaskIndex].m_ucState != TASK_STATE_ACTIVE && p_saTaskList[ucTaskIndex].m_ucState != TASK_STATE_IDLE)
//	{
//		vSERIAL_sout("TaskInactive\r\n", 14);
//		return CHANGE_PERMISSION_NO;
//	}

	// 2 - verify that the task IDs match
//	if(uiTask_FetchTaskID(ucTaskIndex) != ulL2FRAM_getStBlkEntryVal(ucStBlkNum, FRAM_ST_BLK_TASK_IDX))
//	{
//		vSERIAL_sout("CorrectTaskNotFound\r\n", 21);
//		return CHANGE_PERMISSION_NO;
//	}

	// 4 - Check for interval scheduling
	switch (p_saTaskList[ucTaskIndex].m_ucPriority)
	{
		case (PRIORITY_0 | SCHED_FUNC_INTERVAL_SLOT):
			return CHANGE_PERMISSION_YES;

		case (PRIORITY_1 | SCHED_FUNC_INTERVAL_SLOT):
			return CHANGE_PERMISSION_YES;

		case (PRIORITY_2 | SCHED_FUNC_INTERVAL_SLOT):
			return CHANGE_PERMISSION_YES;

		case (PRIORITY_3 | SCHED_FUNC_INTERVAL_SLOT):
			return CHANGE_PERMISSION_YES;

		case (PRIORITY_4 | SCHED_FUNC_INTERVAL_SLOT):
			return CHANGE_PERMISSION_YES;

		case (PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT):
			return CHANGE_PERMISSION_YES;

		case (PRIORITY_6 | SCHED_FUNC_INTERVAL_SLOT):
			return CHANGE_PERMISSION_YES;

		case (PRIORITY_7 | SCHED_FUNC_INTERVAL_SLOT):
			return CHANGE_PERMISSION_YES;

		default:
			vSERIAL_sout("TaskNotChangeable\r\n", 19);
			return CHANGE_PERMISSION_NO;
	}
}

/////////////////////////////////////////////////////////////////////////
//! \fn vTask_Dispatch(uchar ucNSTtblNum, uchar ucNSTslotNum)
//!
//! \brief Dispatches to the tasks scheduled in the NST table
//!
//! \param ucMSTtblNum, ucNSTslotNum
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_Dispatch(uchar ucNSTtblNum, //table num (0 or 1)
		uchar ucNSTslotNum //slot num in table
		)
{
	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];
	uint uiaFlagArray[MAXNUM_TASKS_PERSLOT];
	uchar ucTaskCounter;
	ulong ulFlags;
	signed char cUseFullSlotIdx;

	// Assume that no task requires a full slot
	cUseFullSlotIdx = -1;

	//Fetch the tasks in the slot and store them in the ucaSlotArray
	vRTS_getNSTentry(ucNSTtblNum, ucNSTslotNum, ucaSlotArray);

	//Get the flags for all tasks in the slot
	for (ucTaskCounter = 0; ucTaskCounter < MAXNUM_TASKS_PERSLOT; ucTaskCounter++) {
		// Get flags if allowed otherwise flags = 0
		if (ucTask_GetField(ucaSlotArray[ucTaskCounter], TSK_FLAGS, &ulFlags) == TASKMNGR_OK) {
			uiaFlagArray[ucTaskCounter] = (uint) ulFlags;

			// Check to see if there is a task that requires a full slot
			if ((uiaFlagArray[ucTaskCounter] & F_USE_FULL_SLOT))
				cUseFullSlotIdx = ucTaskCounter;
		}
		else {
			uiaFlagArray[ucTaskCounter] = 0;
		}
	} //END: for(ucTaskCounter)

	//------------------------------  Begin dispatching to tasks --------------------------

	//if the task requires the full slot then dispatch now and skip the rest of the sub slots
	if (cUseFullSlotIdx != -1) {
		if (ucTime_SetSubslotAlarm(SUBSLOT_THREE_END, SUBSLOT_THREE_BUFFER_SIZE) == 0) { // If no error setting alarm
			g_ucaCurrentTskIndex = ucaSlotArray[(uchar) cUseFullSlotIdx];
			// vector to task handler
			p_saTaskList[g_ucaCurrentTskIndex].ptrTaskHandler();
		}
		//Go into LPM to ensure the start of the next sub-slot happens on time
		while (ucTimeCheckForAlarms(SUBSLOT_END_ALARM_BIT) == 0)
			LPM0;
	}
	else {
		// Start timer and dispatch to sub-slot 1
		if (ucTime_SetSubslotAlarm(SUBSLOT_ONE_END, SUBSLOT_ONE_BUFFER_SIZE) == 0) //if no errors setting alarm
				{
			for (ucTaskCounter = 0; ucTaskCounter < MAXNUM_TASKS_PERSLOT; ucTaskCounter++) {/* DISPATCH TO THE ROUTINE */

				//determine which tasks will be executed in the first section of the slot
				if (uiaFlagArray[ucTaskCounter] & F_USE_START_OF_SLOT) {
					g_ucaCurrentTskIndex = ucaSlotArray[ucTaskCounter];
					p_saTaskList[ucaSlotArray[ucTaskCounter]].ptrTaskHandler();
				}
			}

			//Go into LPM to ensure the start of the next sub-slot happens on time
			while (ucTimeCheckForAlarms(SUBSLOT_END_ALARM_BIT) == 0)
				LPM0;
		}

		// Start timer and dispatch to sub-slot 2
		if (ucTime_SetSubslotAlarm(SUBSLOT_TWO_END, SUBSLOT_TWO_BUFFER_SIZE) == 0) //if no errors setting alarm
				{
			for (ucTaskCounter = 0; ucTaskCounter < MAXNUM_TASKS_PERSLOT; ucTaskCounter++) {
				// vector to task
				if (uiaFlagArray[ucTaskCounter] & F_USE_MIDDLE_OF_SLOT) {
					g_ucaCurrentTskIndex = ucaSlotArray[ucTaskCounter];
					p_saTaskList[ucaSlotArray[ucTaskCounter]].ptrTaskHandler();
				}
			}
			//Go into LPM to ensure the start of the next sub-slot happens on time
			while (ucTimeCheckForAlarms(SUBSLOT_END_ALARM_BIT) == 0)
				LPM0;
		}

		// Start timer and dispatch to sub-slot 3
		if (ucTime_SetSubslotAlarm(SUBSLOT_THREE_END, SUBSLOT_THREE_BUFFER_SIZE) == 0) //if no errors setting alarm
				{
			for (ucTaskCounter = 0; ucTaskCounter < MAXNUM_TASKS_PERSLOT; ucTaskCounter++) {
				// vector to task
				if (uiaFlagArray[ucTaskCounter] & F_USE_END_OF_SLOT) {
					g_ucaCurrentTskIndex = ucaSlotArray[ucTaskCounter];
					p_saTaskList[ucaSlotArray[ucTaskCounter]].ptrTaskHandler();
				}
			}
		}
	} // END: else (task does not use full slot)

	// Build messages from the DEs generated during the slot
	vReport_BuildMsgsFromDEs();

	/* SHOW THE ACTION HEADER LINE */
	vRTS_showTaskHdrLine(YES_CRLF);

	// Loop through the tasks and check for any that require deletion
	for (ucTaskCounter = 0; ucTaskCounter < MAXNUM_TASKS_PERSLOT; ucTaskCounter++) {
		// Destroy the task if required
		if (uiaFlagArray[ucTaskCounter] & F_SUICIDE)
			ucTask_DestroyTask(ucaSlotArray[ucTaskCounter]);

		// Suspend the task from further scheduling if required
		if (uiaFlagArray[ucTaskCounter] & F_SUSPEND)
			ucTask_SetField(ucaSlotArray[ucTaskCounter], TSK_STATE, (ulong) TASK_STATE_IDLE);
	}
} // END: vTask_Dispatch()

