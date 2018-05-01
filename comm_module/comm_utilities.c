/**************************  comm.C  *****************************************
 *
 * Routines to perform MSGs during events
 *
 *
 * V1.00 10/04/2003 wzr
 *		Started
 *
 * V2.00 10/15/2013 cp
 *
 *
 ******************************************************************************/
//! \file comm_utilities.c
//! \brief File provides the communications module with functions that apply to two or more individual files
//! \addtogroup Communications
//! @{
/*lint -e526 *//* function not defined */
/*lint -e657 *//* Unusual (nonportable) anonymous struct or union */
/*lint -e714 *//* symbol not referenced */
/*lint -e716 *//* while(1)... */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
/*lint -e757 *//* global declarator not referenced */
/*lint -e752 *//* local declarator not referenced */
/*lint -e758 *//* global union not referenced */
/*lint -e768 *//* global struct member not referenced */

#include <msp430x54x.h>		//processor reg description */
#include "../diag.h"			//Diagnostic package
#include "std.h"			//standard defines
#include "misc.h"			//homeless functions
#include "crc.h"			//CRC calculation module
#include "serial.h"			//serial IO port stuff
#include "comm.h"    		//event MSG module
#include "time.h"			//Time routines
#include "gid.h"			//group ID routines
#include "rand.h"			//random number generator
#include "sensor.h"			//sensor name routines
#include "task.h"
#include "rts.h" //scheduling functions
#include "config.h" 	//system configuration description file
#include "buz.h"
#include "adf7020.h"//radio driver
#include "l2fram.h" 		//Level 2 Fram routines
#include "l2sram.h"  		//disk storage module
#include "main.h"
#include "report.h"

//! \var union DE_Command UCommandDE
//! \brief This union contains the fields required for a command data element
union DE_Command UCommandDE;
//! \var union DE_Report UReportDE
//! \brief This union contains the fields required for a report data element
union DE_Report UReportDE;
//! \var union DE_Code UCodeDE
//! \brief This union contains the fields required for a program code update data element
union DE_Code UCodeDE;

extern volatile uint8 ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];

extern uint uiGLOB_curMsgSeqNum;

extern volatile union //ucFLAG1_BYTE
{
	uchar byte;

	struct
	{
		unsigned FLG1_X_DONE_BIT :1; //bit 0
		unsigned FLG1_X_LAST_BIT_BIT :1; //bit 1
		unsigned FLG1_X_FLAG_BIT :1; //bit 2 ;1=XMIT, 0=RECEIVE
		unsigned FLG1_R_HAVE_MSG_BIT :1; //bit 3	;1=REC has a msg, 0=no msg
		unsigned FLG1_R_CODE_PHASE_BIT :1; //bit 4 ;1=MSG PHASE, 0=BARKER PHASE
		unsigned FLG1_R_ABORT_BIT :1; //bit 5
		unsigned FLG1_X_NXT_LEVEL_BIT :1; //bit 6
		unsigned FLG1_R_SAMPLE_BIT :1; //bit 7
	} FLAG1_STRUCT;

} ucFLAG1_BYTE;

extern volatile union //ucFLAG2_BYTE
{
	uchar byte;
	struct
	{
		unsigned FLG2_T3_ALARM_MCH_BIT :1; //bit 0 ;1=T3 Alarm, 0=no alarm
		unsigned FLG2_T1_ALARM_MCH_BIT :1; //bit 1 ;1=T1 Alarm, 0=no alarm
		unsigned FLG2_T2_ALARM_MCH_BIT :1; //bit 2 ;1=T2 Alarm, 0=no alarm
		unsigned FLG2_CLK_INT_BIT :1; //bit 3	;1=clk ticked, 0=not
		unsigned FLG2_X_FROM_MSG_BUFF_BIT :1; //bit 4
		unsigned FLG2_R_BUSY_BIT :1; //bit 5 ;int: 1=REC BUSY, 0=IDLE
		unsigned FLG2_R_BARKER_ODD_EVEN_BIT :1; //bit 6 ;int: 1=odd, 0=even
		unsigned FLG2_R_BITVAL_BIT :1; //bit 7 ;int:
	} FLAG2_STRUCT;

} ucFLAG2_BYTE;

extern volatile union
{
	uint8 byte;
	struct
	{
		unsigned FLG3_RADIO_ON_BIT :1;
		unsigned FLG3_RADIO_MODE_BIT :1;
		unsigned FLG3_RADIO_PROGRAMMED :1;
		unsigned FLG2_BUTTON_INT_BIT :1; //bit 2 ;1=XMIT, 0=RECEIVE
		unsigned FLG3_LINKSLOT_ALARM :1;
		unsigned FLG3_LPM_DELAY_ALARM :1;
		unsigned FLG3_KEY_PRESSED :1;
		unsigned FLG3_GSV_COM_BIT :1; // 1=GSV com is active 0= GSV com is inactive
	} FLAG3_STRUCT;
} ucFLAG3_BYTE;


//! \def FREQ_ADJ_COUNT_MAX
//! \brief Determines how many samples of the GS time stamp are required before adjusting the sub-second timer register
#define FREQ_ADJ_COUNT_MAX 	5

//! \var ucFreqAdjustTable[]
//! \brief This table stores the adjustments made to the sub-second timer register.
//! The information is used to synchronize clock frequencies with the garden server
signed char cFreqAdjustTable[FREQ_ADJ_COUNT_MAX];

//! \var ucFreqAdjustIndex
//! \brief Index into the ucFreqAdjustTable array
uchar ucFreqAdjustIndex;

//! \var g_ucFreqLocked
//! \brief Indicates that we have synchronized frequencies
uchar g_ucFreqLocked;

T_Text S_MsgName[MSG_TYPE_MAX_COUNT] =
{
		{"NONE", 4}, 	//0 reserved
		{"BCN", 3},		//1 beacon message
		{"RTJ", 3},		//2 request to join
		{"OPL", 3},		//3 operational message
		{"RTR", 3},		//4
		{"LRQ", 3},		//5
		{"RTS", 3},
    };

/*****************************  CODE STARTS HERE  ****************************/

/*WiSARDNet discovery routines*/



///////////////////////////////////////////////////////////////////////////////
//! \brief Prepends the destination and source addresses to the packet
//!
//! Handles network layer responsibilities of communication protocol by building
//! the Net Package in front of a message
//!
//! \param none
//! \return none
///////////////////////////////////////////////////////////////////////////////
void vComm_NetPkg_buildHdr(uint uiDest)
{
	//Stuff the destination address
	ucaMSG_BUFF[NET_IDX_DEST_HI] = (uchar) (uiDest >> 8);
	ucaMSG_BUFF[NET_IDX_DEST_LO] = (uchar) (uiDest);

	// Stuff the source address
	vL2FRAM_copySnumLo16ToBytes((uchar *) &ucaMSG_BUFF[NET_IDX_SRC_HI]);

} //END: vComm_NetPkg_buildHdr()


///////////////////////////////////////////////////////////////////////////////
//! \brief Builds the operational message
//!
//! This message contain can contain reports, commands, and code.  In the
//! event that it is the first message sent in a slot from a parent node
//! to a child then the first DE is an update time command and is explicitly packed
//! here
//!
///////////////////////////////////////////////////////////////////////////////
void vComm_Msg_buildOperational(uchar ucFlags, uint uiMsgNum, uint uiDest, uchar ucMsgID)
{

	/* STUFF MSG TYPE */
	ucaMSG_BUFF[MSG_IDX_ID] = ucMsgID;

	// Stuff the message flags
	ucaMSG_BUFF[MSG_IDX_FLG] = ucFlags;

	// Stuff the message number
	ucaMSG_BUFF[MSG_IDX_NUM_HI] = (uchar) (uiMsgNum >> 8);
	ucaMSG_BUFF[MSG_IDX_NUM_LO] = (uchar) uiMsgNum;

	//Stuff the address fields with address
	ucaMSG_BUFF[MSG_IDX_ADDR_HI] = (uchar) (uiDest >> 8);
	ucaMSG_BUFF[MSG_IDX_ADDR_LO] = (uchar) uiDest;

} //END: vComm_Msg_buildOperational()


///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Builds the report data element header
//!
//! \param ucProcID, ucSrcID, ucLength, p_ucData,
//! \return The next free location in the message buffer
///////////////////////////////////////////////////////////////////////////////
void vComm_DE_BuildReportHdr(uchar ucProcID, uchar ucPayloadLen, uchar ucVersion)
{
	long lTime;

	//get the system time
	lTime = lTIME_getSysTimeAsLong();

	//stuff the received fields into the data element structure
	ucaMSG_BUFF[DE_IDX_ID] = REPORT_DATA;
	ucaMSG_BUFF[DE_IDX_LENGTH] = ucPayloadLen + 8;
	ucaMSG_BUFF[DE_IDX_VERSION] = ucVersion;
	ucaMSG_BUFF[DE_IDX_RPT_PROCID] = ucProcID;

	// Pack the time into the buffer
	ucaMSG_BUFF[DE_IDX_TIME_SEC_LO] = (uchar) lTime;
	lTime = lTime >> 8;
	ucaMSG_BUFF[DE_IDX_TIME_SEC_MD] = (uchar) lTime;
	lTime = lTime >> 8;
	ucaMSG_BUFF[DE_IDX_TIME_SEC_HI] = (uchar) lTime;
	lTime = lTime >> 8;
	ucaMSG_BUFF[DE_IDX_TIME_SEC_XI] = (uchar) lTime;

}


///////////////////////////////////////////////////////////////////////////////
//!
//! This routine checks the message header in this order:
//! 1. CRC
//! 2. Message Type
//! 3. Group ID
//! 4. Dest SN
//! 5. Src SN
//!
//!
//! \param ucChkByteBits
//!	<ul>
//!   <li>BIT7 Check CRC
//!   <li>BIT6 Check MSG ID
//!   <li>BIT5 Check group select
//!   <li>BIT4 Check group ID
//!   <li>BIT3 Check destination
//!   <li>BIT2 Check source
//!	  <li>BIT1	unused
//!	  <li>BIT0 unused
//!	</ul>
//!
//! \param ucReportByteBits
//! <ul>
//!   <li>BIT7 Report CRC error
//!   <li>BIT6 Report MSG ID error
//!   <li>BIT5 Report group select error
//!   <li>BIT4 Report group ID error
//!   <li>BIT3 Report destination error
//!   <li>BIT2 Report source error
//!	  <li>BIT1 unused
//!	  <li>BIT0 unused
//!	</ul>
//!
//! \return ucErrRetVal
//!	<ul>
//!   <li>BIT7 CRC error
//!   <li>BIT6	MSG ID error
//!   <li>BIT5 Group select error
//!   <li>BIT4 Group ID error
//!   <li>BIT3	Destination error
//!   <li>BIT2 Source error
//!	  <li>BIT1	unused
//!	  <li>BIT0 unused
//!	</ul>
///////////////////////////////////////////////////////////////////////////////
uchar ucComm_chkMsgIntegrity(
//RET: BitMask if BAD,  0 if OK
    uchar ucChkByteBits, uchar ucReportByteBits, uchar ucMsgType, uint uiExpectedSrcSN, uint uiExpectedDestSN)
{
	uchar ucErrRetVal;
	uint uiMsgDestSN;
	uint uiMsgSrcSN;
	uchar ucPacketLength;

	ucErrRetVal = 0; //assume no errors

	// Set the length of the packet
	ucPacketLength = ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ;

	/* CHECK THE CRC -- IF ITS BAD -- LOOP BACK */
	if ((ucChkByteBits & CHKBIT_CRC) && (!ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_REC, ucaMSG_BUFF, ucPacketLength))) {
		if (ucReportByteBits & CHKBIT_CRC) {
			vSERIAL_sout("MSG:BdCRC\r\n", 11);
		}
		ucErrRetVal |= CHKBIT_CRC;
	}

	/* CHECK FOR PROPER MSG TYPE */
	if ((ucChkByteBits & CHKBIT_MSG_TYPE) && (ucaMSG_BUFF[MSG_IDX_ID] != ucMsgType)) {
		if (ucReportByteBits & CHKBIT_MSG_TYPE) {
			uchar ucGotMsgType;
			ucGotMsgType = ucaMSG_BUFF[MSG_IDX_ID];

			vSERIAL_sout("MSG:MsgJammedExp ", 17);
			//vSERIAL_UIV8out(ucMsgType);
			vSERIAL_sout("(", 1);
			if (ucMsgType < MSG_TYPE_MAX_COUNT)
				vSERIAL_sout(S_MsgName[ucMsgType].m_cText, S_MsgName[ucMsgType].m_uiLength);
			vSERIAL_sout(") got ", 6);

			//vSERIAL_UIV8out(ucGotMsgType);
			vSERIAL_sout("(", 1);
			if (ucGotMsgType < MSG_TYPE_MAX_COUNT)
				vSERIAL_sout(S_MsgName[ucGotMsgType].m_cText, S_MsgName[ucGotMsgType].m_uiLength);
			vSERIAL_sout(")\r\n", 3);
		}
		ucErrRetVal |= CHKBIT_MSG_TYPE;
	}

	/* CHECK FOR A GROUP SELECTOR MATCH */
	if ((ucChkByteBits & CHKBIT_GRP_SEL)
			&& (!ucGID_compareOnlySysGrpSelectToBytes((uchar *) &ucaMSG_BUFF[MSG_IDX_GID_HI], ucReportByteBits & CHKBIT_GRP_SEL, //report flag
					YES_CRLF))) {
		ucErrRetVal |= CHKBIT_GRP_SEL;
	}

	/* CHECK FOR GROUP ID */
	if ((ucChkByteBits & CHKBIT_GID) && (ucGID_getSysGrpSelectAsByte() == 0)) {
		if (!ucGID_compareOnlySysGidToBytes((uchar *) &ucaMSG_BUFF[MSG_IDX_GID_HI], ucReportByteBits & CHKBIT_GID, YES_CRLF)) {
			ucErrRetVal |= CHKBIT_GID;
		}
	}

	/* CHECK DEST ID */
	if (ucChkByteBits & CHKBIT_DEST_SN) {
		uiMsgDestSN = uiMISC_buildUintFromBytes((uchar *) &ucaMSG_BUFF[NET_IDX_DEST_HI], NO_NOINT);
		if (uiMsgDestSN != uiExpectedDestSN) {
			if (ucReportByteBits & CHKBIT_DEST_SN) {
				vSERIAL_sout("MSG:BdDstSN ", 12);
				vComm_showSNmismatch(uiExpectedDestSN, uiMsgDestSN, YES_CRLF);
			}
			ucErrRetVal |= CHKBIT_DEST_SN;
		}

	}/* END: if() */

	/* CHECK THE SOURCE SN */
	if (ucChkByteBits & CHKBIT_SRC_SN) {
		uiMsgSrcSN = uiMISC_buildUintFromBytes((uchar *) &ucaMSG_BUFF[NET_IDX_SRC_HI], NO_NOINT);
		if (uiMsgSrcSN != uiExpectedSrcSN) {
			if (ucReportByteBits & CHKBIT_SRC_SN) {
				vSERIAL_sout("MSG:BDSrcSN ", 12);
				vComm_showSNmismatch(uiExpectedSrcSN, uiMsgSrcSN, YES_CRLF);
			}
			ucErrRetVal |= CHKBIT_SRC_SN;
		}

	}/* END: if() */

	return (ucErrRetVal);

}/* END: ucComm_chkMsgIntegrity() */

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Monitors sub-second clock adjustments to synchronize frequencies
//! with the parent node or garden server
//!
//! \param ucClockAdjust, adjustment in ticks/second
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vComm_SynchFreq(signed char cClockAdjust)
{
	int iFreqAdjust;
	uchar ucIndex;
	uint uiNewCompareVal;
	uint uiOldCompareVal;
	uchar ucMsgIndex;

	// Store the adjustment to the sub-second timer in units of ticks/second
	cFreqAdjustTable[ucFreqAdjustIndex] = cClockAdjust; //note: slow wisard clock gets positive values

	ucFreqAdjustIndex++;

	if(ucFreqAdjustIndex == FREQ_ADJ_COUNT_MAX)
	{
		iFreqAdjust = 0;
		for(ucIndex = 0; ucIndex < FREQ_ADJ_COUNT_MAX; ucIndex++)
		{
			iFreqAdjust += cFreqAdjustTable[ucIndex];
		}

		iFreqAdjust = iFreqAdjust/FREQ_ADJ_COUNT_MAX;

		// Get the current value from the timer register
		uiOldCompareVal = uiTime_GetSlotCompareReg();

		// Calculate the new time
		uiNewCompareVal = uiOldCompareVal - iFreqAdjust;


		// Testing
		if(uiNewCompareVal < uiOldCompareVal)
			uiNewCompareVal = uiOldCompareVal - 1;
		if(uiNewCompareVal > uiOldCompareVal)
			uiNewCompareVal = uiOldCompareVal + 1;

		// Prevent erroneous adjustments to the compare register value by restricting the range
		if(uiNewCompareVal > 32773)
			uiNewCompareVal= 32773;
		else if (uiNewCompareVal < 32763)
			uiNewCompareVal = 32763;

		// Report the adjustment
		vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SUBSECOND_ADJUST;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiNewCompareVal >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiNewCompareVal;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_SUBSECOND_ADJUST);

		// Write the new time to the register
		vTime_SetSlotCompareReg(uiNewCompareVal);

		ucFreqAdjustIndex = 0;
		g_ucFreqLocked++;
	}

}


///////////////////////////////////////////////////////////////////////////////
//! \fn ucComm_doSubSecXmit
//! \brief Transmits a message at a particular time
//!
//!
//! \param lSendTimeSec, uiSubSecSendTime, ucClkChoiceFlag, ucStartRecFlag
//! \return 0-sent, 1-too late
//////////////////////////////////////////////////////////////////////////////
uchar ucComm_doSubSecXmit(
    ulong lSendTimeSec,
		uint uiSubSecSendTime,
    uchar ucClkChoiceFlag, //USE_CLK2, USE_CLK1
    uchar ucStartRecFlag //YES_RECEIVER_START, NO_RECEIVER_START
    )
{
	long lCurSec;

	/* CHECK THE START SEC AGAINST CUR TIME */
	if (ucClkChoiceFlag == USE_CLK1) {
		lCurSec = lTIME_getSysTimeAsLong(); //wups was SysTime clk
	}
	else {
		lCurSec = lTIME_getClk2AsLong(); //clk2
	}

	// If we are late, report it
	if (lCurSec > lSendTimeSec) {
#if 1
		vSERIAL_sout("MSG:MissdMsgTmWas ", 18);
		vSERIAL_HBV32out((ulong) lCurSec);
		vSERIAL_sout("  wanted ", 9);
		vSERIAL_HBV32out((ulong) lSendTimeSec);
		vSERIAL_crlf();
#endif
		return (1);
	}

	/* WAIT FOR FULL SEC TIC TO COME UP */
	if (ucClkChoiceFlag == USE_CLK2) {
		while (lSendTimeSec > lTIME_getClk2AsLong()); //lint !e722
	}
	else {
		while (lSendTimeSec > lTIME_getSysTimeAsLong()); //lint !e722
	}

#if 0
	vSERIAL_sout("MSG:BeforRadioXmt= ", 19);
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
#endif

	// Load the message into the buffer
	vADF7020_SetPacketLength(ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);
	unADF7020_LoadTXBuffer((uint8*) &ucaMSG_BUFF);

	/* WAIT FOR SUB SECOND TIC TO COME UP */
	while (uiSubSecSendTime > SUB_SEC_TIM);

	// Set the radio state to TX, and send the message
	vADF7020_TXRXSwitch(RADIO_TX_MODE);
	vADF7020_SendMsg();

#if 0
	vSERIAL_sout("MSG:AfterRadioXmt= ", 19);
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
#endif

	/* CHECK FOR IMMEDIATE START OF RECEIVER */
	if (ucStartRecFlag) {
		vADF7020_TXRXSwitch(RADIO_RX_MODE);
	}

#if 0
	vSERIAL_sout("MSG:AfterRadioRec= ", 19);
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);
#endif

	return (0);

}/* vComm_doSubSecXmit() */


/**********************  vComm_showSNmismatch()  ***********************
 *
 * Show the values of a serial number mismatch
 *
 *
 *****************************************************************************/
void vComm_showSNmismatch(uint uiExpectedVal, uint uiGotVal, uchar ucCRLF_flag //YES_CRLF, NO_CRLF
    )
{
	vSERIAL_sout("EXP= #", 6);
	vSERIAL_UIV16out(uiExpectedVal);
	vSERIAL_sout(" GOT= #", 7);
	vSERIAL_UIV16out(uiGotVal);
	if (ucCRLF_flag)
		vSERIAL_crlf();

	return;

}/* END: vComm_showSNmismatch() */

////////////////////////////////////////////////////////////////////////////
//!
//! \brief Checks the task manager to see if the task exists, if it does then
//! set the state to active so it will be scheduled.
//!
//! \param uiSensorID
//! \return RetVal, error code
////////////////////////////////////////////////////////////////////////////
uchar ucComm_Handle_Command(S_Cmd S_CommandData)
{
	uchar ucCount;
	uchar ucTaskIndex;
	uchar ucCmdParam[MAXCMDPARAM];
	S_Task_Ctl S_Task;

	// Fill the command parameter array with data or set to 0
	for (ucCount = 0; ucCount < MAXCMDPARAM; ucCount++) {

		if (ucCount < S_CommandData.m_ucCmdLength) {
			ucCmdParam[ucCount] = S_CommandData.m_ucCmdParam[ucCount];
		}
		else {
			ucCmdParam[ucCount] = 0;
			break;
		}
	}

	// Only create the task if it does not already exist
	if (S_CommandData.m_uiTaskId == TASK_ID_RPT_HID && ucTask_FetchTaskIndex(TASK_ID_RPT_HID) == INVALID_TASKINDEX) {

		S_Task.m_uiTask_ID = TASK_ID_RPT_HID;
		S_Task.m_uiFlags = TASK_FLAGS_RPT_HID;
		S_Task.m_ucPriority = TASK_PRIORITY_RPT_HID;
		S_Task.m_ucProcessorID = (uchar) (TASK_ID_RPT_HID >> 8);
		S_Task.m_ulTransducerID = (ulong) (TASK_ID_RPT_HID & TASK_TRANSDUCER_ID_MASK);
		S_Task.m_ucState = TASK_STATE_ACTIVE;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_RPT_HID;
		S_Task.m_ulParameters[0] = 0;
		S_Task.m_ulParameters[1] = 0;
		S_Task.m_ulParameters[2] = 0;
		S_Task.m_ulParameters[3] = 0;
		S_Task.m_cName = TASK_NAME_RPT_HID;
		S_Task.ptrTaskHandler = vTask_ReportHID;
		ucTask_CreateTask(S_Task);
	}

	// Check for a task index if it exists
	ucTaskIndex = (uchar) ucTask_FetchTaskIndex(S_CommandData.m_uiTaskId);

	// If the task is valid then set the state, else return an error code
	if (ucTaskIndex != INVALID_TASKINDEX) {
		ucTask_SetField(ucTaskIndex, TSK_CMDLENGTH, S_CommandData.m_ucCmdLength);
		ucTask_SetCmdParam(ucTaskIndex, ucCmdParam);
		ucTask_SetField(ucTaskIndex, TSK_STATE, TASK_STATE_ACTIVE);
		return 0;
	}
	else
		return 1;

}

/*******************  vMSG_showStorageErr()  ********************************
 *
 *
 *
 *****************************************************************************/
void vMSG_showStorageErr(char *cpLeadinMsg, uint uiStrLength, unsigned long ulFailAddr, unsigned long ulWroteVal, unsigned long ulReadVal)
{

	vSERIAL_sout(cpLeadinMsg, uiStrLength);
	vSERIAL_sout(" at ", 4);
	vSERIAL_HB32out(ulFailAddr);
//	vSERIAL_crlf();
	vSERIAL_sout(" Wrote ", 7);
	vSERIAL_HB32out(ulWroteVal);
	vSERIAL_sout(" read ", 6);
	vSERIAL_HB32out(ulReadVal);
	vSERIAL_sout(" xor= ", 6);
	vSERIAL_HB32out(ulWroteVal ^ ulReadVal);
	vSERIAL_crlf();

}/* END: vMSG_showStorageErr() */

/**********************  ucMSG_waitForMsgOrTimeout() *****************************
 *
 * This routine assumes a clk alarm has already been setup.
 *
 * RET:	1 = GOT A MSG
 *		0 = Timed out
 *
 ******************************************************************************/
uchar ucComm_waitForMsgOrTimeout(unsigned char ucReadRSSI)
{
	uchar ucRetVal;

	// Assume timeout
	ucRetVal = 0;

	/* WAIT FOR MESSAGE COMPLETE */
	while (TRUE) //lint !e774
	{
		if (ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT) //End of slot
			break;

		if (ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T2_ALARM_MCH_BIT) //SubSlot Buffer Started
			break;

		if (ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT) //SubSlot Ended
			break;

		if (ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_LINKSLOT_ALARM) //Link slot ended
			break;

		if (ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT) //We have a message
		{
			unADF7020_ReadRXBuffer(ucaMSG_BUFF);
			ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 0;
			ucRetVal = 1;
			break;
		}

		// If we are receiving a message and the RSSI task is active then sample RSSI
		if (unADF7020_GetRadioState() == RX_ACTIVE && ucReadRSSI == YES_RSSI) {
			ucADF7020_SampleRSSI();
		} // END: if()

	}/* END: while() */

	return (ucRetVal);

}/* END: ucMSG_waitForMsgOrTimeout()*/

/************************  uiComm_incMsgSeqNum()  ******************************
 *
 * Increment the message seq number
 *
 * NOTE: The Seq number cannot be 0 or 255
 *
 * RET: Incremented Msg Seq Num
 *
 ******************************************************************************/
uint uiComm_incMsgSeqNum( //RET: Incremented Msg Seq Num (not 0 or 255)
    void)
{
	uiGLOB_curMsgSeqNum++;

	/* NOT 0 AND NOT 255 */
	if (uiGLOB_curMsgSeqNum >= 0xFFFF)
		uiGLOB_curMsgSeqNum = 1;

	return (uiGLOB_curMsgSeqNum);

}/* END: vMSG_incMsgSeqNum() */

/********************  ucMSG_getLastFilledEntryInOM2()  **********************
 *
 * RET:	 0 = illegal
 *		## = index for Last entry in the msg (pts to sensor ID)
 *
 ******************************************************************************/
uchar ucMSG_getLastFilledEntryInOM2( //RET: 0=none, ##=idx of last entry in OM2
    void)
{
	uchar ucMsgLastDataStart;

	/* GET AN INDEX FOR THE LAST BEGINNING OF THE LAST ENTRY IN THE MSG */
	ucMsgLastDataStart = (ucaMSG_BUFF[0] & MAX_MSG_SIZE_MASK) - 4;

	/* NOW CHECK THAT END COUNT TO MAKE SURE ITS OK */
	switch (ucMsgLastDataStart)
	{
		case 16: //normal msg lengths
		case 19:
		case 22:
		case 25:
		break;

		case 27:
			ucMsgLastDataStart = 25; //had a short msg also
		break;

		default:
			ucMsgLastDataStart = 0; //illegal length
		break;

	}/* END: switch() */

	return (ucMsgLastDataStart);

}/* END: ucMSG_getLastFilledEntryInOM2() */

/*-------------------------------  MODULE END  ------------------------------*/
//! @}
