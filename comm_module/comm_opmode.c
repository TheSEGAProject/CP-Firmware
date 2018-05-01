/////////////////////////////////////////////////////////////////////////////
//! \file comm_opmode.c
//! \addtogroup Communications
//! @{
//! \brief This file is part of the communications module and handles communication
//! 	protocols while the node is attempting to join a network.
//!
//! Chris Porter 3/14
//!
/////////////////////////////////////////////////////////////////////////////

//! \file comm_opmode.c
//! \brief File handles all discovery mode communications
//! \addtogroup Communications
//! @{

#include <msp430x54x.h>		//processor reg description */
#include "task.h"					//Task management module
#include "comm.h"    			//event MSG module
#include "buz.h"					//Buzzer
#include "adf7020.h"			//radio driver
#include "mem_mod.h"			//memory module
#include "misc.h"					//homeless functions
#include "crc.h"					//CRC calculation module
#include "time.h"					//Time routines
#include "gid.h"					//group ID routines
#include "serial.h"				//serial IO port stuff
#include "rand.h"					//random number generator
#include "rts.h" 					//scheduling functions
#include "lnkblk.h"				//Link block routines
#include "report.h"				//Reporting
#include "delay.h"				// Delay module
#include "main.h"				// For getting software version
#include "ota.h"					// Handlers for program code
///////////////////   externs    /////////////////////////
extern volatile uint8 ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];
extern uchar g_ucaCurrentTskIndex;
extern int iGLOB_completeSysLFactor;
extern uint uiGLOB_lostROM2connections; //counts lost ROM2's
extern uint uiGLOB_lostSOM2connections; //counts lost SOM2's

extern uint uiGLOB_ROM2attempts; //count ROM2 attempts
extern uint uiGLOB_SOM2attempts; //count SOM2 attempts
extern uint8 ucGLOB_myLevel; //senders level +1

extern volatile union //ucFLAG0_BYTE
{
	uint8 byte;
	struct
	{
		unsigned FLG0_BIGSUB_CARRY_BIT :1; //bit 0 ;1=CARRY, 0=NO-CARRY
		unsigned FLG0_BIGSUB_6_BYTE_Z_BIT :1; //bit 1 ;1=all diff 0, 0=otherwise
		unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT :1; //bit 2 ;1=top 4 bytes 0, 0=otherwise
		unsigned FLG0_REDIRECT_COMM_TO_ESPORT_BIT :1; //bit 3 ;1=REDIRECT, 0=COMM1
		unsigned FLG0_RESET_ALL_TIME_BIT :1; //bit 4 ;1=do time  reset, 0=dont
		//SET:	when RDC4 gets finds first
		//		SOM2.
		//		or
		//		In a Hub when it is reset.
		//
		//CLR: 	when vMAIN_computeDispatchTiming()
		//		runs next.
		unsigned FLG0_SERIAL_BINARY_MODE_BIT :1; //bit 5 1=binary mode, 0=text mode
		unsigned FLG0_HAVE_WIZ_GROUP_TIME_BIT :1; //bit 6 1=Wizard group time has
		//        been aquired from a DC4
		//      0=We are using startup time
		unsigned FLG0_ECLK_OFFLINE_BIT :1; //bit 7 1=ECLK is not being used
	//      0=ECLK is being used
	} FLAG0_STRUCT;
} ucFLAG0_BYTE;

ulong g_ulLastCommTime = 0;

// Internal function definitions for this module
uchar ucComm_WaitForAck(uint uiMsgNumber);
void vComm_SendAck(uint uiMsgNumber);

//TODO remove
long ulRand = 0;

/////////////////////////////////////////////////////////////////////////////
//!
//!	\brief Handles RF channel selection
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
static void vComm_SetChannel(uchar ucChannel)
{
	// Do not use channels 48 through 51.  The frequencies are close to integer multiples of the PFD (see adf7020 datasheet)
	const uchar ucChannelTable[10] = { 8, 16, 24, 32, 40, 46, 56, 64, 72, 80 };
	ulong ulRand, ulTime, ulSerialNumber;
	uchar ucChannelIndex;

	if ((CURRENT_CHANNELS == RANDOM_CHANNEL) && (ucChannel == DATA_CHANNEL)) {

		// Get the serial number
		ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulSerialNumber);

		// Get the random number
		ucLNKBLK_ReadRand((uint) ulSerialNumber, &ulRand);

		ulTime = lTIME_getSysTimeAsLong();
		ucChannelIndex = ((uchar) (ulTime ^ ulRand)) % 10;
		unADF7020_SetChannel(ucChannelTable[ucChannelIndex]);

#if 0

		vSERIAL_sout("RAND = ", 7);
		vSERIAL_UI32out(ulRand);
		vSERIAL_crlf();

		vSERIAL_sout("TIME = ", 7);
		vSERIAL_UI32out(ulTime);
		vSERIAL_crlf();

		vSERIAL_sout("CHNL = ", 7);
		vSERIAL_UIV8out(ucChannelTable[ucChannelIndex]);
		vSERIAL_crlf();
#endif

	}
	else {
		unADF7020_SetChannel(ucChannel);
	}
}

/////////////////////////////////////////////////////////////////////////////
//!
//!	\brief zeros the missed message count between two nodes
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vComm_zroMissedMsgCnt(void)
{
	ulong ulSerialNumber;

	// Get the serial number of the corresponding node
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulSerialNumber);

	// Set the missed message count to zero
	ucLNKBLK_WriteMsdMsgCount((uint) ulSerialNumber, 0);

	// Set the missed message count to zero
	ucLNKBLK_WritePriority((uint) ulSerialNumber, 0);
	return;

} // END: vComm_zroMissedMsgCnt();

/////////////////////////////////////////////////////////////////////////////
//!
//!	\brief Called at the start of the slot by a parent and child to see
//! if the link still exists.
//!
//! \param none
//! \return 1, link closed; 0, link open
/////////////////////////////////////////////////////////////////////////////
uchar ucComm_incLnkBrkCntAndChkForDeadOM2(void)
{
	ulong ulTaskID, ulSN;
	uint uiTaskID;
	uint uiSN;
	uchar ucMissedMsgCnt;

	// Get the serial number of the corresponding node
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulSN);
	uiSN = (uint) ulSN;

	// Fetched the missed message count
	ucLNKBLK_ReadMsdMsgCount(uiSN, &ucMissedMsgCnt);

	/* INC THE MISSED MSG COUNT */
	ucMissedMsgCnt++;

	// Set the status
	ucLNKBLK_WriteMsdMsgCount(uiSN, ucMissedMsgCnt);

#if 1
	vSERIAL_sout("OM2failCnt= ", 12);
	vSERIAL_UIV8out(ucMissedMsgCnt);
	vSERIAL_crlf();
#endif

	// IF NODE IS STILL CONNECTED -- LEAVE NOW
	if ((ucMissedMsgCnt) < BROKEN_LINK_MAX_COUNT)
		return (0);

	/*----- IF WE ARE HERE THE LINK IS CONSIDERED DEAD --------------------*/

	/* INC THE GLOBAL DEBUG COUNTERS */
	ucTask_GetField(g_ucaCurrentTskIndex, TSK_ID, &ulTaskID);
	uiTaskID = (uint) ulTaskID;

	if (uiTaskID == TASK_ID_ROM)
		uiGLOB_lostROM2connections++;
	if (uiTaskID == TASK_ID_SOM)
		uiGLOB_lostSOM2connections++;

#if 1
	/* REPORT TO THE CONSOLE */
	vSERIAL_sout("OM2>", 4);
	vSERIAL_UI16out(uiSN);
	vSERIAL_sout(" LkClose,ROM2Lost=", 18);
	vSERIAL_UIV16out(uiGLOB_lostROM2connections);
	vSERIAL_sout(", SOM2Lost=", 11);
	vSERIAL_UIV16out(uiGLOB_lostSOM2connections);
	vSERIAL_crlf();
#endif

	// Remove the dropped node from the link block table
	ucLNKBLK_RemoveNode(uiSN);

	// Remove the dropped node and all its descendants from the edge list
	ucRoute_NodeUnjoin(uiSN);

	// Destroy this task
	ucTask_DestroyTask(g_ucaCurrentTskIndex);

	return (1); //ret link broken flag

}/* END: ucComm_incLnkBrkCntAndChkForDeadOM2() */

//////////////////////////////////////////////////////////////////////////////
//!
//! \brief returns a link byte value based on the system load
//!
//! \return ucOM2LinkVal
//////////////////////////////////////////////////////////////////////////////
uchar ucComm_getOM2LinkByteVal(void)
{
	uchar ucOM2LinkVal;

	ucOM2LinkVal = ucLNKBLK_computeMultipleLnkReqFromSysLoad(iGLOB_completeSysLFactor);

	return ucOM2LinkVal;
//	return LNKREQ_1FRAME_1LNK;
}/* END: ucComm_getOM2LinkByteVal() */

/////////////////////////////////////////////////////////////////////////////
//! \brief This function sends a command if one exists
//!
//!
//!
//! \param none
//! \return ucRetVal, 1 if command is loaded else 0
/////////////////////////////////////////////////////////////////////////////
uchar ucComm_Send_Command(void)
{
	ulong ulOtherGuysSN;
	uint uiOtherGuysSN; //!< Serial number of destination
	uchar ucRetVal;

	// GET THE OTHER LINK'S SERIAL NUM
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	// If there is a command in memory for this node then load it
	if (ucL2SRAM_LoadCmdIfExists(uiOtherGuysSN)) {

		// Prepend network layer with destination address
		vComm_NetPkg_buildHdr(uiOtherGuysSN);

		// COMPUTE THE CRC
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); //lint !e534 //compute the CRC

		// Load message into TX buffer and set the radio mode to TX
		vADF7020_SetPacketLength(ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);
		unADF7020_LoadTXBuffer((uint8*) &ucaMSG_BUFF);
		vADF7020_TXRXSwitch(RADIO_TX_MODE);

		// Send the Message
		vADF7020_SendMsg();

		ucRetVal = 1;
	}
	else {
		// Build operational message: single message no Ack
		vComm_Msg_buildOperational((MSG_FLG_SINGLE | MSG_FLG_ACKRQST), 0, uiOtherGuysSN, MSG_ID_OPERATIONAL);
		ucaMSG_BUFF[MSG_IDX_LEN] = 10;

		// Request data command DE
		ucaMSG_BUFF[MSG_IDX_PAYLD] = REQUEST_DATA; //DEID
		ucaMSG_BUFF[MSG_IDX_PAYLD + 1] = 0x03; // DE Length
		ucaMSG_BUFF[MSG_IDX_PAYLD + 2] = 0x6E; // DE Version

		// Prepend network layer with an illegal destination address
		vComm_NetPkg_buildHdr(uiOtherGuysSN);

		// COMPUTE THE CRC
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); //lint !e534 //compute the CRC

		// Load message into TX buffer and set the radio mode to TX
		vADF7020_SetPacketLength(ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);
		unADF7020_LoadTXBuffer((uint8*) &ucaMSG_BUFF);
		vADF7020_TXRXSwitch(RADIO_TX_MODE);

		// Send the Message
		vADF7020_SendMsg();

		ucRetVal = 0;
	}
#if 0
	uchar ucjj;
	vSERIAL_sout("Tx=", 3);
	for (ucjj = 0; ucjj < 19; ucjj++)
	{
		vSERIAL_HB8out(ucaMSG_BUFF[ucjj]);
		vSERIAL_crlf();
	}
#endif

	return ucRetVal;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief This function sends as much data from the memory as possible
//!
//!
//!
//! \param none
//! \return ucRetVal, 0 if there is no more data to send
/////////////////////////////////////////////////////////////////////////////
void vComm_SendData(void)
{
	ulong ulOtherGuysSN;
	uint uiOtherGuysSN;
	uint uiMsgCount;
	uint uiMsgNumber;
	uchar ucMsgIndex;
	uchar ucAttemptCount;
	uchar ucTXMsgCount;

	// Initialize the attempt count
	ucAttemptCount = 0;
	ucTXMsgCount = 0;

	// Loop while the sub-slot alarms aren't set
	while (ucTimeCheckForAlarms(SUBSLOT_ALARMS) == 0) {
		// Get the first message from memory
		if (ucL2SRAM_getCopyOfCurMsg()) // if there is a message
		{
			// Get the number of messages in memory
			uiMsgCount = uiL2SRAM_getMsgCount();

			// Get the other links serial number and build the network header
			ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
			uiOtherGuysSN = (uint) ulOtherGuysSN;

			vComm_NetPkg_buildHdr(uiOtherGuysSN);

			// Store a local copy of the message number
			uiMsgNumber = ((ucaMSG_BUFF[MSG_IDX_NUM_HI] << 8) | ucaMSG_BUFF[MSG_IDX_NUM_LO]);

			// If this is the last message then
			if (uiMsgCount == 1)
				ucaMSG_BUFF[MSG_IDX_FLG] |= MSG_FLG_END;

			// COMPUTE THE CRC
			ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); //lint !e534 //compute the CRC

#if 0 // Display the message
			uchar ucIndex;
			for(ucIndex = 0; ucIndex < (ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); ucIndex++)
			{
				vSERIAL_HB8out(ucaMSG_BUFF[ucIndex]);
				vSERIAL_crlf();
			}
#endif

			// Load message into TX buffer and set the radio mode to TX.
			vADF7020_SetPacketLength(ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);
			unADF7020_LoadTXBuffer((uint8*) &ucaMSG_BUFF);
			vADF7020_TXRXSwitch(RADIO_TX_MODE);

			// Send the Message
			vADF7020_SendMsg();

			// Set the radio into RX mode
			vADF7020_TXRXSwitch(RADIO_RX_MODE);

			if (ucComm_WaitForAck(uiMsgNumber)) {
				// Clear the attempt count
				ucAttemptCount = 0;
				ucTXMsgCount++;

				// need to get an ack on rx by parent in operation
				vL2SRAM_delCurMsg();
			}
			else {
				// Delete the message if it fails 5 times
				if (ucAttemptCount == 5) {
					// Build the report data element header
					vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
					ucMsgIndex = DE_IDX_RPT_PAYLOAD;

					ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_MSG_DELIVERY_FAIL;
					ucaMSG_BUFF[ucMsgIndex] = 0; // data length

					// Store DE
					vReport_LogDataElement(RPT_PRTY_MSG_DELIVERY_FAIL);

					// Delete message
					vL2SRAM_delCurMsg();
				}
				else {
					ucAttemptCount++;
				}
			}
		}
		else {
			// If there are no more messages to send then exit
			break;
		}
	}

	vSERIAL_sout("Msgs: ", 6);
	vSERIAL_UI8out(ucTXMsgCount);
	vSERIAL_crlf();

	// Get the number of messages in memory
	uiMsgCount = uiL2SRAM_getMsgCount();

	if (uiMsgCount > 0) {
		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_MSG_COUNT;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiMsgCount >> 8);
		ucaMSG_BUFF[ucMsgIndex] = (uchar) uiMsgCount;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_MSG_COUNT);
	}
}

/////////////////////////////////////////////////////////////////////////////
//! \brief This function receives data from the child and sends the acks
//!
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vComm_Receive_Data(void)
{
	uchar ucRXMsgCount;
	uint uiMsgNumber;

	ucRXMsgCount = 0;

	// Loop while the sub-slot alarms aren't set
	while (ucTimeCheckForAlarms(SUBSLOT_ALARMS) == 0) {
		// set the radio into RX mode
		vADF7020_TXRXSwitch(RADIO_RX_MODE);

		// Wait for a reply from the child node
		if (ucComm_waitForMsgOrTimeout(NO_RSSI)) {
			/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
			if (!(ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
					CHKBIT_CRC + CHKBIT_MSG_TYPE, //chk flags
					CHKBIT_CRC + CHKBIT_MSG_TYPE, //report flags
					MSG_ID_OPERATIONAL, //msg type
					0, //src SN
					0 //Dst SN
					))) {
#if 0
				vSERIAL_sout("RX Data \r\n", 10);
#endif
				// If the message contains data then store it
				if (ucaMSG_BUFF[MSG_IDX_LEN] != MSG_HDR_SZ) {
					vL2SRAM_storeMsgToSram();

					// The hub stores all information in the SD card so any packet that comes up through the network will
					// be stored there
					if (ucL2FRAM_isHub()) {

						// Log to the SD card
						vREPORT_LogReport();
					}

					// Increment the received message count
					ucRXMsgCount++;

					uiMsgNumber = ((ucaMSG_BUFF[MSG_IDX_NUM_HI] << 8) | ucaMSG_BUFF[MSG_IDX_NUM_LO]);
					vComm_SendAck(uiMsgNumber);
				}

				// If this is the last message from the child then break out
				if (ucaMSG_BUFF[MSG_IDX_ID] & MSG_FLG_END)
					break;
			}

		}
	}

	vSERIAL_sout("Msgs: ", 6);
	vSERIAL_UI8out(ucRXMsgCount);
	vSERIAL_crlf();
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Reads the message and determines the required action
//!
//!
//!
//! \param none
//! \return flags indicating the action to take
/////////////////////////////////////////////////////////////////////////////
uchar ucComm_ParseMsg(void)
{
	uint uiMySN;
	uint uiDestSN;
	uchar ucMsgLength;
	uchar ucCount, ucByteCount;
	uchar ucRetVal;
	uchar ucDEID;
	uchar ucDELength;
	uchar ucDEVersion;
	uchar ucProcID;
	uchar ucTransID;
	uchar ucMsgIndex;
	ucRetVal = 0;
	uint uiNextHop;
	S_Cmd S_CommandData;
	union DE_Code U_ProgramCode;
	long lExpTime;

	//Get serial numbers
	uiMySN = uiL2FRAM_getSnumLo16AsUint();
	uiDestSN = ucaMSG_BUFF[MSG_IDX_ADDR_HI] << 8 | ucaMSG_BUFF[MSG_IDX_ADDR_LO];

	// Check the final destination of the message.
	if (uiDestSN != uiMySN) {
		// Check the edge list to determine what the next hop for this packet is (if it exists)
		uiNextHop = uiRoute_GetNextHop(uiDestSN);
		if (uiNextHop != 0) {
			vSERIAL_sout("Cmd: Saved\r\n", 12);
			// Store the command for the next time we talk to the next hop node
			ucL2SRAM_PutCMD_inQueue(uiNextHop);
		}
		else {
			vSERIAL_sout("Cmd: Delivery failed\r\n", 22);
			// If the destination node is not in the route list then report the delivery failure
			vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
			ucMsgIndex = DE_IDX_RPT_PAYLOAD;

			ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_ROUTING_FAIL;
			ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
			ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiDestSN >> 8);
			ucaMSG_BUFF[ucMsgIndex] = (uchar) uiDestSN;
			// Store DE
			vReport_LogDataElement(RPT_PRTY_ROUTING_FAIL);
		}

		return 2;
	}

	// Get the length of the message
	ucMsgLength = ucaMSG_BUFF[MSG_IDX_LEN];

	// Set the starting point to the payload
	ucCount = MSG_IDX_PAYLD;

	// Get the data element ID
	ucDEID = ucaMSG_BUFF[ucCount++];
	ucDELength = ucaMSG_BUFF[ucCount++];
	ucDEVersion = ucaMSG_BUFF[ucCount++];

	switch (ucDEID)
	{
		case COMMAND_PKT:

			// Get the processor and transducer ID
			ucProcID = ucaMSG_BUFF[ucCount++];
			lExpTime = (long) ulMISC_buildUlongFromBytes((uchar*) &ucaMSG_BUFF[ucCount], YES_NOINT);
			ucCount += 4;
			ucTransID = ucaMSG_BUFF[ucCount++];
			S_CommandData.m_uiTaskId = (uint) ((ucProcID << 8) | ucTransID);
			S_CommandData.m_ucCmdLength = ucDELength - 9;

			// Ensure the command parameters meet size constraints
			if (S_CommandData.m_ucCmdLength > MAXCMDPARAM) {

				// Build the report data element header
				vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
				ucMsgIndex = DE_IDX_RPT_PAYLOAD;

				ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_MSG_DELIVERY_FAIL;
				ucaMSG_BUFF[ucMsgIndex] = 0; // data length

				// Store DE
				vReport_LogDataElement(RPT_PRTY_MSG_DELIVERY_FAIL);

				return 2;
			}

			// Get the command parameters from the message
			for (ucByteCount = 0; ucByteCount < S_CommandData.m_ucCmdLength; ucByteCount++) {
				S_CommandData.m_ucCmdParam[ucByteCount] = ucaMSG_BUFF[ucCount++];
			}

			// If the command has expired then report it.  Otherwise, go handle the command.
			if (lExpTime < lTIME_getSysTimeAsLong()) {
#if 0
				vSERIAL_sout("Cmd Expired\r\n", 13);
				vSERIAL_sout("Cmd Time: ", 10);
				vSERIAL_HB32out((ulong)lExpTime);
				vSERIAL_crlf();
				vSERIAL_sout("Time Now: ", 11);
				vSERIAL_HB32out((ulong)lTIME_getSysTimeAsLong());
				vSERIAL_crlf();
#endif

				// If the command has expired then report the failure
				vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
				ucMsgIndex = DE_IDX_RPT_PAYLOAD;

				ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_CMD_EXPIRED;
				ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
				ucaMSG_BUFF[ucMsgIndex++] = ucProcID;
				ucaMSG_BUFF[ucMsgIndex] = ucTransID;
				// Store DE
				vReport_LogDataElement(RPT_PRTY_CMD_EXPIRED);

				return 2;
			}

			// Command passed all checks, go set it up
			ucComm_Handle_Command(S_CommandData);

			ucRetVal = 0x02;
		break;

		case PROGRAM_CODE:

			// Write the data element to the program code union
			U_ProgramCode.fields.m_ucDEID = ucDEID;
			U_ProgramCode.fields.m_ucDE_Length = ucDELength;
			U_ProgramCode.fields.m_ucVersion = ucDEVersion;
			U_ProgramCode.fields.m_ucBoardNum = ucaMSG_BUFF[ucCount++];
			U_ProgramCode.fields.m_ucBoardType = ucaMSG_BUFF[ucCount++];
			U_ProgramCode.fields.m_ucProgID = ucaMSG_BUFF[ucCount++];
			U_ProgramCode.fields.m_uiComponentNum = uiMISC_buildUintFromBytes((uchar*) &ucaMSG_BUFF[ucCount], YES_NOINT);
			ucCount += 2;
			U_ProgramCode.fields.m_ulStartAddr = (long) ulMISC_buildUlongFromBytes((uchar*) &ucaMSG_BUFF[ucCount], YES_NOINT);
			ucCount += 4;

			// Check the size of the program code before copying to the buffer
			if (U_ProgramCode.fields.m_ucDE_Length > MAX_CODE) {
				// Build the report data element header
				vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
				ucMsgIndex = DE_IDX_RPT_PAYLOAD;

				ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_MSG_DELIVERY_FAIL;
				ucaMSG_BUFF[ucMsgIndex] = 0; // data length

				// Store DE
				vReport_LogDataElement(RPT_PRTY_MSG_DELIVERY_FAIL);

				return 2;
			}

			// Get the command parameters from the message
			for (ucByteCount = 0; ucByteCount < (U_ProgramCode.fields.m_ucDE_Length - CODEHEADERSIZE); ucByteCount++) {
				U_ProgramCode.fields.m_ucCode[ucByteCount] = ucaMSG_BUFF[ucCount++];
			}

			// Take the code packet and store it
			vOTA_ReceiveCodePacket(&U_ProgramCode);

		break;

		case REQUEST_DATA:
			ucRetVal = 0x01;
		break;

	} // END switch (DEID)

	if (ucRetVal == 0)
		vSERIAL_sout("Unkwn Cmd\r\n", 11);

	return ucRetVal;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Sends the ready to receive operational message at the start of the slot
//!
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vComm_SendRTR(void)
{
	ulong ulOtherGuysSN;
	uint uiOtherGuysSN;
	uint uiSubSeconds;

	// GET THE OTHER LINK'S SERIAL NUM
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	// Build the net layer header
	vComm_NetPkg_buildHdr(uiOtherGuysSN);

	// Build ready to receive message layer header
	vComm_Msg_buildOperational(MSG_FLG_SINGLE, 1, uiOtherGuysSN, MSG_ID_RTR);

	//Stuff the message length
	ucaMSG_BUFF[MSG_IDX_LEN] = 13;

	//Stuff the synch time in seconds
	vMISC_copyUlongIntoBytes(lTIME_getSysTimeAsLong(), (uchar *) &ucaMSG_BUFF[OPMSG_IDX_TIME_SEC_XI], NO_NOINT);

	// Fetch the subsecond time and load it
	uiSubSeconds = uiTIME_getSubSecAsUint();
	ucaMSG_BUFF[OPMSG_IDX_TIME_SUBSEC_HI] = (uchar) (uiSubSeconds >> 8);
	ucaMSG_BUFF[OPMSG_IDX_TIME_SUBSEC_LO] = (uchar) uiSubSeconds;

	// COMPUTE THE CRC
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); //lint !e534 //compute the CRC

	// Load message into TX buffer
	vADF7020_SetPacketLength(ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);
	unADF7020_LoadTXBuffer((uint8*) &ucaMSG_BUFF);

	vADF7020_TXRXSwitch(RADIO_TX_MODE);

	// Send the Message
	vADF7020_SendMsg();

}

/////////////////////////////////////////////////////////////////////////////
//! \brief Sends the link request message
//!
//!	This packet tells the parent how many slots to allocate during the next
//!	frame in addition additions to the child's routing table
//!
//! \param ucLinkByte
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vComm_SendLRQ(uchar ucLinkByte)
{
	ulong ulOtherGuysSN;
	uint uiOtherGuysSN;
	uchar ucUpdateCount;
	uchar ucSpaceLeft;

	// GET THE OTHER LINK'S SERIAL NUM
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	// Network Layer
	vComm_NetPkg_buildHdr(uiOtherGuysSN);

	// Single message
	vComm_Msg_buildOperational(MSG_FLG_SINGLE, 1, uiOtherGuysSN, MSG_ID_LRQ);

	// Add the message length as well as the link request and network update fields
	ucaMSG_BUFF[MSG_IDX_LEN] = MSG_HDR_SZ + 3;
	ucaMSG_BUFF[MSG_IDX_LRQ] = ucLinkByte;

	// Get the total number of routing updates
	ucUpdateCount = ucRoute_GetUpdateCountBytes(F_JOIN | F_DROP);

	// If there are no updates then just send the link byte and exit
	if (ucUpdateCount == 0) {
		// Set the join and drop lengths to 0
		ucaMSG_BUFF[(uchar) (MSG_IDX_LRQ + 1)] = 0;
		ucaMSG_BUFF[(uchar) (MSG_IDX_LRQ + 2)] = 0;

		// COMPUTE THE CRC
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); //lint !e534 //compute the CRC

		// Load message into TX buffer and set the radio mode to TX
		vADF7020_SetPacketLength(ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);
		unADF7020_LoadTXBuffer((uint8*) &ucaMSG_BUFF);
		vADF7020_TXRXSwitch(RADIO_TX_MODE);

		// Send the Message
		vADF7020_SendMsg();

		return;
	}

	// Get the available space in the message
	ucSpaceLeft = MAX_MSG_SIZE - (ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);

	// If there is enough room in the current message to send all required updates
	while (ucUpdateCount != 0) {
		// Set the link request byte
		ucaMSG_BUFF[MSG_IDX_LRQ] = ucLinkByte;

		// Add the update count to the current message length
		ucaMSG_BUFF[MSG_IDX_LEN] = ucUpdateCount + MSG_HDR_SZ + 4;

		// Load the updates into the message buffer after the link request byte
		vRoute_GetUpdates(&ucaMSG_BUFF[(uchar) (MSG_IDX_LRQ + 1)], ucSpaceLeft);

		// Build the header
		if (ucUpdateCount > ucSpaceLeft) {
			vComm_NetPkg_buildHdr(uiOtherGuysSN);
			vComm_Msg_buildOperational(MSG_FLG_ACKRQST, 1, uiOtherGuysSN, MSG_ID_LRQ);
			ucaMSG_BUFF[MSG_IDX_LEN] = ucSpaceLeft + MSG_HDR_SZ + 4;
		}
		else {
			vComm_NetPkg_buildHdr(uiOtherGuysSN);
			vComm_Msg_buildOperational(MSG_FLG_SINGLE, 1, uiOtherGuysSN, MSG_ID_LRQ);
		}

		// COMPUTE THE CRC
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); //lint !e534 //compute the CRC

		// Load message into TX buffer
		vADF7020_SetPacketLength(ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);
		unADF7020_LoadTXBuffer((uint8*) &ucaMSG_BUFF);

		// Send the Message
		vADF7020_SendMsg();

		// If we have sent all updates then exit the loop
		if (ucUpdateCount < ucSpaceLeft)
			break;

		// Wait for an ack
		vADF7020_TXRXSwitch(RADIO_RX_MODE);
		if (!ucComm_WaitForAck(1))
			return;

		// Since we have received an ACK we can remove the flagged updates
		vRouteClrFlaggedUpdates();

		// Get the total number of routing updates
		ucUpdateCount = ucRoute_GetUpdateCountBytes(F_JOIN | F_DROP);

	}

}

/////////////////////////////////////////////////////////////////////////////
//! \brief Wait for the link request message
//!
//!	The link request message contains routing information in addition to the
//!	link request byte.  The child node will send routing data with the ack request
//! flag set when the amount of network update data exceeds the maximum packet size.
//! When this
//!
//! \param none
//! \return 1 for success, 0 for failure
/////////////////////////////////////////////////////////////////////////////
static uchar ucComm_WaitFor_LRQ(void)
{
	uchar ucLinkFailReason;
	uchar ucLinkFailPriority;
	uchar ucRetVal, ucLnkFlags;
	uchar ucLinkRequest;
	uchar ucMsgIndex;
	uint uiOtherGuysSN;
	uint uiMySN;
	signed int iRSSI;
	ulong ulOtherGuysSN;

	// Assume failure
	ucRetVal = 0;

	// Initialize the link failed variable
	ucLinkFailReason = 0;
	ucLinkFailPriority = 0;

	/* INC THE ATTEMPT COUNT */
	uiGLOB_ROM2attempts++;

	// Get the serial number
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	// Get my serial number
	uiMySN = uiL2FRAM_getSnumLo16AsUint();

//	//	Might be receiving a message from another node, keep listening instead of giving up after one message
//	while (ucTimeCheckForAlarms(SUBSLOT_WARNING_ALARM_BIT) == 0) {

	// Wait for a reply from the child node
	if (ucComm_waitForMsgOrTimeout(YES_RSSI)) {
		/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
		if (!(ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
				CHKBIT_CRC + CHKBIT_MSG_TYPE + CHKBIT_DEST_SN + CHKBIT_SRC_SN, //chk flags
				CHKBIT_CRC + CHKBIT_MSG_TYPE + CHKBIT_DEST_SN + CHKBIT_SRC_SN, //report flags
				MSG_ID_LRQ, //msg type
				uiOtherGuysSN, //src SN
				uiMySN //Dst SN
				))) {

			// Zero the missed messages
			vComm_zroMissedMsgCnt();

			ucLinkRequest = ucaMSG_BUFF[MSG_IDX_LRQ];

			vSERIAL_sout("LnkByte: ", 9);
			vLNKBLK_showLnkReq(ucLinkRequest);
			vSERIAL_crlf();

			// Update the edge list (routing data).
			vRoute_SetUpdates(&ucaMSG_BUFF[(uchar) (MSG_IDX_LRQ + 1)]);

			// Stash the link request if allowed
			ucLNKBLK_ReadFlags(uiOtherGuysSN, &ucLnkFlags);
			if (ucLnkFlags & F_OVERWRITE) {
				ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiOtherGuysSN, //Node SN
						ucLinkRequest, //Lnk Confirm
						lTIME_getSysTimeAsLong());
			}

			if (ucaMSG_BUFF[MSG_IDX_FLG] & MSG_FLG_ACKRQST)
				ucRetVal = 2; // Indicates that the parent should send an ACK and expect more link data
			else
				ucRetVal = 1; // Indicates all link data has been received from the child

//				break;
		} // END: if(Integrity)
	} // END: if(Timeout or message received)

//		// Reset the radio in the event that the message failed the integrity check
//		vADF7020_TXRXSwitch(RADIO_RX_MODE);
//	}

	// No message received
	if (ucRetVal == 0) {
		// Stash the link request if allowed
		ucLNKBLK_ReadFlags(uiOtherGuysSN, &ucLnkFlags);
		if (ucLnkFlags & F_OVERWRITE) {
			// SETUP A DEFAULT LINK BYTE
			ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiOtherGuysSN, //Tbl Idx
					LNKREQ_1FRAME_1LNK, //Lnk Confirm
					lTIME_getSysTimeAsLong());
			// Set the over write flag.  If we manage to communicate again this frame we want to be able
			// to set up a new link block
			ucLNKBLK_SetFlag(uiOtherGuysSN, F_OVERWRITE);
		}

		ucLinkFailReason = SRC_ID_MISSED_LRQ; //assume its a simple miss
		ucLinkFailPriority = RPT_PRTY_MISSED_LRQ;

		// Check for a broken link
		if (ucComm_incLnkBrkCntAndChkForDeadOM2()) {
			ucLinkFailReason = SRC_ID_LINK_BROKEN; //wups this is the big fail
			ucLinkFailPriority = RPT_PRTY_LINK_BROKEN;
		}

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = ucLinkFailReason;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiOtherGuysSN >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiOtherGuysSN;

		// Store DE
		vReport_LogDataElement(ucLinkFailPriority);
	}
	// If the link request worked then store an RSSI report if the RSSI task is active
	else {

		// Read out the RSSI and format it into a data element
		iRSSI = iADF7020_RequestRSSI();

		// Write the rssi to the link block
		ucLNKBLK_WriteRSSI(uiOtherGuysSN, iRSSI);

		// Store the RSSI in a data element
		vComm_DE_BuildReportHdr(CP_ID, 6, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;
		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RSSI;
		ucaMSG_BUFF[ucMsgIndex++] = 0x04; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiOtherGuysSN >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiOtherGuysSN;
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (iRSSI >> 8);
		ucaMSG_BUFF[ucMsgIndex] = (uchar) iRSSI;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_RSSI);
	}
	return ucRetVal; // Return the status
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Wait for the ready to receive message
//!
//!
//!
//! \param none
//! \return 1 for success, 0 for failure
/////////////////////////////////////////////////////////////////////////////
uchar ucComm_WaitFor_RTR(void)
{
	uchar ucLinkFailReason;
	uchar ucLinkFailPriority;
	uchar ucRetVal;
	uint uiSubSecLatency;
	uint uiSubSecTemp, uiPrevSubsec;
	uchar ucMsgIndex;
	uint uiOtherGuysSN;
	uint uiMySN;
	signed int iRSSI;
	ulong ulCurrentTime, ulOtherGuysSN;
	signed char cFreqAdjust;
	uchar ucFlags;
	uchar ucTaskIndex;

	// Assume there will be a failure
	ucRetVal = 0;

	// Define no reason for failure
	ucLinkFailReason = 0;
	ucLinkFailPriority = 0;

	// Increment the global count of attempts
	uiGLOB_SOM2attempts++;

	// Get the serial number if allowed
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	// Get my serial number
	uiMySN = uiL2FRAM_getSnumLo16AsUint();

	//TODO
//	Might be receiving a message from another node, keep listening instead of giving up after one message
//	while(alarm == 0){}

	// Wait for a reply from the child node
	if (ucComm_waitForMsgOrTimeout(YES_RSSI)) {
		/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
		if (!(ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
				CHKBIT_CRC + CHKBIT_MSG_TYPE + CHKBIT_DEST_SN + CHKBIT_SRC_SN, //chk flags
				CHKBIT_CRC + CHKBIT_MSG_TYPE + CHKBIT_DEST_SN + CHKBIT_SRC_SN, //report flags
				MSG_ID_RTR, //msg type
				uiOtherGuysSN, //src SN
				uiMySN //Dst SN
				))) {
			//Stop the latency timer
			vTime_LatencyTimer(OFF);

			uiPrevSubsec = uiTIME_getSubSecAsUint();

			//read time from the timer register
			uiSubSecLatency = LATENCY_TIMER;

			// Save the new time in Clk2, therefore Clk1 and alarms are still good
			vTIME_setClk2FromBytes((uchar *) &ucaMSG_BUFF[OPMSG_IDX_TIME_SEC_XI]);

			// Get the sub-second time from the message
			uiSubSecTemp = (uint) ((ucaMSG_BUFF[OPMSG_IDX_TIME_SUBSEC_HI] << 8) | ucaMSG_BUFF[OPMSG_IDX_TIME_SUBSEC_LO]);

			uint uiSupSecParent = uiSubSecTemp;

			// Determine the new timer value from the received value + measured value + constant
			uiSubSecTemp += uiSubSecLatency + 0xA0;

			// Set the timer
			vTIME_setSubSecFromUint(uiSubSecTemp);

			ulCurrentTime = (ulong) lTIME_getSysTimeAsLong();

			if (g_ulLastCommTime != 0) {
				cFreqAdjust = (signed char) ((uiSubSecTemp - uiPrevSubsec) / (ulCurrentTime - g_ulLastCommTime));
				// Report the adjustment to the sub-second clock
				vComm_SynchFreq(cFreqAdjust);
			}

#if 0
			vSERIAL_sout("Latency Time = ", 15);
			vSERIAL_UI16out(uiSubSecLatency);
			vSERIAL_crlf();

			vSERIAL_sout("Old = ", 6);
			vSERIAL_UI16out(uiPrevSubsec);
			vSERIAL_crlf();

			vSERIAL_sout("Updated Time = ", 15);
			vSERIAL_UI16out(uiSubSecTemp);
			vSERIAL_crlf();

			vSERIAL_sout("Comm Diff = ", 12);
			vSERIAL_UI8out((uchar)(ulCurrentTime - g_ulLastCommTime));
			vSERIAL_crlf();

			vSERIAL_sout("Parent = ", 9);
			vSERIAL_UI16out(uiSupSecParent);
			vSERIAL_crlf();

#endif

			g_ulLastCommTime = ulCurrentTime;

			// Zero out the missed message count in the task status field
			vComm_zroMissedMsgCnt();

			// Set the return byte indicating success
			ucRetVal = 1;
		}
	}

	// If the link failed then set up a default link, determine the severity of the failure and log it
	if (ucRetVal == 0) {
		//Make sure the latency timer is off whether or not the RTR is received
		vTime_LatencyTimer(OFF);

		// Stash the link request if allowed
		ucLNKBLK_ReadFlags(uiOtherGuysSN, &ucFlags);
		if (ucFlags & F_OVERWRITE) {
			// SETUP A DEFAULT LINK BYTE
			ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiOtherGuysSN, LNKREQ_1FRAME_1LNK, lTIME_getSysTimeAsLong());

			// Set the over write flag.  If we manage to communicate again this frame we want to be able
			// to set up a new link block
			ucLNKBLK_SetFlag(uiOtherGuysSN, F_OVERWRITE);
		}
		ucLinkFailReason = SRC_ID_MISSED_RTR; //assume its a simple miss
		ucLinkFailPriority = RPT_PRTY_MISSED_RTR;

		// Check for a broken link
		if (ucComm_incLnkBrkCntAndChkForDeadOM2()) {
			// Set the RTJ task state to active to allow scheduling
			ucTask_SetField(ucTask_FetchTaskIndex(TASK_ID_RTJ), TSK_STATE, TASK_STATE_ACTIVE);

			// Set discovery to partial
			vCommSetDiscMode(PARTIALDISCOVERY);

			ucLinkFailReason = SRC_ID_LINK_BROKEN; //wups this is the big fail
			ucLinkFailPriority = RPT_PRTY_LINK_BROKEN;

			// Reset the nodes level in the network to the maximum value
			ucGLOB_myLevel = LEVEL_MAX_VAL;
		}

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = ucLinkFailReason;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiOtherGuysSN >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiOtherGuysSN;

		// Store DE
		vReport_LogDataElement(ucLinkFailPriority);
	}
	// If the ready to receive was successful then store an RSSI report if the RSSI task is active
	else {
		// Read out the RSSI and store it in the link block structure
		iRSSI = iADF7020_RequestRSSI();
		ucLNKBLK_WriteRSSI(uiOtherGuysSN, iRSSI);

		// Store the RSSI in a data element
		vComm_DE_BuildReportHdr(CP_ID, 6, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;
		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RSSI;
		ucaMSG_BUFF[ucMsgIndex++] = 0x04; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiOtherGuysSN >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiOtherGuysSN;
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (iRSSI >> 8);
		ucaMSG_BUFF[ucMsgIndex] = (uchar) iRSSI;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_RSSI);
	}
	return ucRetVal; // Return the status

}

/////////////////////////////////////////////////////////////////////////////
//! \brief Waits for an ack message.
//!
//! \param none
//! \return 1 for an ack, else 0
/////////////////////////////////////////////////////////////////////////////
uchar ucComm_WaitForAck(uint uiMsgNumber)
{
	uchar ucRetVal;

	// Assume failure
	ucRetVal = 0;

	// Wait for a reply from the child node
	if (ucComm_waitForMsgOrTimeout(NO_RSSI)) {
		/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
		if (!(ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
				CHKBIT_CRC + CHKBIT_MSG_TYPE, //chk flags
				CHKBIT_CRC + CHKBIT_MSG_TYPE, //report flags
				MSG_ID_OPERATIONAL, //msg type
				0, //src SN
				0 //Dst SN
				))) {
			// Check the ack flag and the message number
			if ((uiMsgNumber == ((ucaMSG_BUFF[MSG_IDX_NUM_HI] << 8) | ucaMSG_BUFF[MSG_IDX_NUM_LO])) && (ucaMSG_BUFF[MSG_IDX_FLG] & MSG_FLG_ACK)) {
				// Set the return byte indicating success
				ucRetVal = 1;
			}
		}
	}
	return ucRetVal; // Return the status
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Sends an ack message.  This function is intended to be sent
//! immediately after receiving a message.  It is assumed that the message
//!number is the same as when the message was received.
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vComm_SendAck(uint uiMsgNumber)
{
	ulong ulOtherGuysSN;
	uint uiOtherGuysSN;
	uint8 ucaAckMessage[MSG_HDR_SZ + NET_HDR_SZ + CRC_SZ];

	// GET THE OTHER LINK'S SERIAL NUM
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	//Stuff the destination address
	ucaAckMessage[NET_IDX_DEST_HI] = (uchar) (uiOtherGuysSN >> 8);
	ucaAckMessage[NET_IDX_DEST_LO] = (uchar) (uiOtherGuysSN);

	// Stuff the source address
	vL2FRAM_copySnumLo16ToBytes((uchar *) &ucaAckMessage[NET_IDX_SRC_HI]);

	/* STUFF MSG TYPE */
	ucaAckMessage[MSG_IDX_ID] = MSG_ID_OPERATIONAL;

	// Stuff the message flags
	ucaAckMessage[MSG_IDX_FLG] = (MSG_FLG_SINGLE | MSG_FLG_ACK);

	// Stuff the message number
	ucaAckMessage[MSG_IDX_NUM_HI] = (uchar) (uiMsgNumber >> 8);
	ucaAckMessage[MSG_IDX_NUM_LO] = (uchar) uiMsgNumber;

	//Stuff the address fields with address
	ucaAckMessage[MSG_IDX_ADDR_HI] = (uchar) (uiOtherGuysSN >> 8);
	ucaAckMessage[MSG_IDX_ADDR_LO] = (uchar) uiOtherGuysSN;

	ucaAckMessage[MSG_IDX_LEN] = MSG_HDR_SZ;

	// COMPUTE THE CRC
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaAckMessage, ucaAckMessage[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); //lint !e534 //compute the CRC

	// Load message into TX buffer and set the radio mode to TX
	vADF7020_SetPacketLength(ucaAckMessage[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ);
	unADF7020_LoadTXBuffer(ucaAckMessage);
	vADF7020_TXRXSwitch(RADIO_TX_MODE);

	// Send the Message
	vADF7020_SendMsg();
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Handles communication for a parent node
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vComm_Parent(void)
{
	uint uiMsgNumber; // used for both incoming and outgoing messages
	uchar ucLRQRetVal;

	// Sleep in LPM 1 for 20 ms to allow the receiver to turn on and get ready for the RTR
	// Waiting accounts for most clock drift cases as a result of crystal imperfections/temperatures
	vDELAY_LPMWait1us(20000, SLEEPMODE_2);

	// Power up and initialize the radio
	vADF7020_WakeUp();

	// Set the RF channel
	vComm_SetChannel(DATA_CHANNEL);

	///////////////// Debug - remove ////////////////////////

	uint uiOtherGuysSN;
	ulong ulOtherGuysSN;
	uchar ucMsgIndex;

	// Get the serial number if allowed
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	// Build the report data element stating that the link has been broken
	vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;

	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SEND_RTR;
	ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiOtherGuysSN >> 8);
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiOtherGuysSN;

	// Store DE
	vReport_LogDataElement(RPT_PRTY_SEND_RTR);
	/////////////////**************************////////////////////////

	// TODO
	//Resend RTR if there is no response

	// Build and send the ready to receive message at the start of the communication window
	vComm_SendRTR();

	//Set the radio state to RX mode, enabling interrupts
	vADF7020_StartReceiver();

	// Wait for the link request packet
	ucLRQRetVal = ucComm_WaitFor_LRQ();

	// If the LRQ return value equals 1 then it is either the only LRQ packet or the last one in the sequence
	while (ucLRQRetVal != 1) {
		if (ucLRQRetVal == 0) // The LRQ timed out
				{
			vADF7020_Quit();
			return;
		}
		else if (ucLRQRetVal == 2) // The LRQ is part of a sequence and the child is waiting for an ACK
				{
			uiMsgNumber = ((ucaMSG_BUFF[MSG_IDX_NUM_HI] << 8) | ucaMSG_BUFF[MSG_IDX_NUM_LO]);
			vComm_SendAck(uiMsgNumber);

			// set the radio state to RX mode
			vADF7020_TXRXSwitch(RADIO_RX_MODE);
			ucLRQRetVal = ucComm_WaitFor_LRQ();
		}
	}

	// Loop while the sub-slot alarms aren't set
	while (ucTimeCheckForAlarms(SUBSLOT_ALARMS) == 0) {
		// Send a command and set the Ack. pending byte.
		if (ucComm_Send_Command()) // if there is a command to send then send it or request data
		{
			// Store a local copy of the message number
			uiMsgNumber = ((ucaMSG_BUFF[MSG_IDX_NUM_HI] << 8) | ucaMSG_BUFF[MSG_IDX_NUM_LO]);

			// set the radio into RX mode
			vADF7020_TXRXSwitch(RADIO_RX_MODE);

			// Wait for an ack message indicating the cmd was received
			if (ucComm_WaitForAck(uiMsgNumber)) {
				ucL2SRAM_Del_current_CMD();
			}
		}
		else // else wait for data
		{
			vComm_Receive_Data(); // this returns when either all messages are received or when time is up
			break;
		}
	} // End: while(alarms)

	vADF7020_Quit();
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Handles communication for a child node
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vComm_Child(void)
{
	uchar ucParseRet;
	uchar ucLinkByte;
	uint uiOtherGuysSN;
	uint uiMySN;
	ulong ulOtherGuysSN;
	uchar ucLnkFlags;
	uchar ucLnkRqstSuccess;
	uint uiMsgNumber;

	///////////////// Debug - remove ////////////////////////

	uchar ucMsgIndex;
	// Get the serial number if allowed
	ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN);
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	// Build the report data element stating that the link has been broken
	vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;

	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_WAIT_RTR;
	ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiOtherGuysSN >> 8);
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiOtherGuysSN;

	// Store DE
	vReport_LogDataElement(RPT_PRTY_WAIT_RTR);
	/////////////////**************************////////////////////////

	//Configure the timer to measure latency
	vTime_LatencyTimer(ON);

	// Set the channel
	vComm_SetChannel(DATA_CHANNEL);

	// Power up and initialize the radio
	vADF7020_WakeUp();

	if (!ucComm_WaitFor_RTR()) {
		vADF7020_Quit();
		return;
	}

	// Get the serial number if allowed
	if (ucTask_GetField(g_ucaCurrentTskIndex, PARAM_SN, &ulOtherGuysSN) != TASKMNGR_OK)
		return;
	uiOtherGuysSN = (uint) ulOtherGuysSN;

	// Get my serial number
	uiMySN = uiL2FRAM_getSnumLo16AsUint();

	// Compute the link byte based on system load
	ucLinkByte = ucComm_getOM2LinkByteVal();

	// Send a packet with link request information
	vComm_SendLRQ(ucLinkByte);

	//Set the radio state to RX mode, enabling interrupts
	vADF7020_TXRXSwitch(RADIO_RX_MODE);

	// Assume that the hub will not receive the link request message
	ucLnkRqstSuccess = FALSE;

	// Loop and wait for commands until time out
	while (ucTimeCheckForAlarms(SUBSLOT_ALARMS) == 0) {
		if (ucComm_waitForMsgOrTimeout(NO_RSSI)) {
			/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, DEST_SN */
			if (!(ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
					CHKBIT_CRC + CHKBIT_MSG_TYPE + CHKBIT_DEST_SN + CHKBIT_SRC_SN, //chk flags
					CHKBIT_CRC + CHKBIT_MSG_TYPE + CHKBIT_DEST_SN + CHKBIT_SRC_SN, //report flags
					MSG_ID_OPERATIONAL, //msg type
					uiOtherGuysSN, //src SN
					uiMySN //Dst SN
					))) {

				// The link request was received
				ucLnkRqstSuccess = TRUE;

				// Clear the update table... This should be placed elsewhere
				vRouteClrFlaggedUpdates();

				// Store a local copy of the message number for the ack
				uiMsgNumber = ((ucaMSG_BUFF[MSG_IDX_NUM_HI] << 8) | ucaMSG_BUFF[MSG_IDX_NUM_LO]);

				// Parse the message to determine the command
				ucParseRet = ucComm_ParseMsg();
				if (ucParseRet == 0x01) {
					// Once this function returns exit, either we are out of time or there are no more messages to send
					vComm_SendData();
					break;
				}

				// Send the Ack after the message is parsed to avoid timing errors between sender and receiver
				vComm_SendAck(uiMsgNumber);

				// Set the radio into RX mode
				vADF7020_TXRXSwitch(RADIO_RX_MODE);

			} // END: if(check integrity)
		} // END: if(wait for message)
	} // END: while(check for alarms)

	// Read flags for this link
	ucLNKBLK_ReadFlags(uiOtherGuysSN, &ucLnkFlags);

	// If we made it here then we can set up the new link requirements
	if (ucLnkFlags & F_OVERWRITE && ucLnkRqstSuccess == TRUE) {
		ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiOtherGuysSN, ucLinkByte, lTIME_getSysTimeAsLong());
	}
	// If the parent node did not recieve the link request then set up a default link for the next frame.
	else if ((ucLnkFlags & F_OVERWRITE && ucLnkRqstSuccess == FALSE)) {
		ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiOtherGuysSN, LNKREQ_1FRAME_1LNK, lTIME_getSysTimeAsLong());
	}

	//Not sure if this is the best place for this but the latency timer must be stopped before exiting this function
	vTime_LatencyTimer(OFF);
	vADF7020_Quit();

//Set the flags indicating time has been reset.  Clock 1 is updated in main_ComputeDispatchTiming
	ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT = 1;
	ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT = 1;

}

/**********************    *****************************
 *
 * This routine searches the table for all ROM2 and SOM2 entrys
 * and then prints out the list
 *
 * NOTE: This routine was re-written to eliminate the array storage and
 *		that is why it is in 3 passes.
 *
 *****************************************************************************/
void vComm_showSOMandROMcounts(uchar ucCRLF_termFlag)
{
	uchar ucc;
	uchar ucActiveSOM2LocalIdx;
	uchar ucActiveROM2LocalIdx;
	uint uiTaskID;
	ulong ulTaskID, ulSN;
	uchar ucMissCnt;
	uint uiSN;
	uint uiCountTotals;

	/* FIRST PASS COUNTS HOW MANY ROMS AND SOM2 THERE ARE */
	uiCountTotals = uiTask_countSOM2andROM2entrys();
	ucActiveSOM2LocalIdx = (uchar) (uiCountTotals >> 8);
	ucActiveROM2LocalIdx = (uchar) uiCountTotals;

	/* PASS 2: SHOW THE SOM2 STUFF */
	vSERIAL_sout("SOM2#", 5);
	vSERIAL_UIV8out(ucActiveSOM2LocalIdx);

	if (ucActiveSOM2LocalIdx != 0) {
		vSERIAL_bout('>');

		for (ucc = 0; ucc < MAXNUMTASKS; ucc++) {
			ucTask_GetField(ucc, TSK_ID, &ulTaskID);
			uiTaskID = (uint) ulTaskID;

			if (uiTaskID == TASK_ID_SOM) {
				/* SHOW THE NAME */
				vTask_showTaskName(ucc);
				ucTask_GetField(ucc, PARAM_SN, &ulSN);
				uiSN = (uint) ulSN;
				vSERIAL_UI16out(uiSN);

				/* SHOW THE MISS COUNT IN PARENTHESIS */
				ucLNKBLK_ReadMsdMsgCount(uiSN, &ucMissCnt);
				if (ucMissCnt != 0) {
					vSERIAL_bout('(');
					vSERIAL_UIV8out(ucMissCnt);
					vSERIAL_bout(')');
				}
				vSERIAL_bout(',');
			}
		}/* END: for(ucc) */
	}/* END: if() */

	vSERIAL_bout(' ');

	/* PASS 3: SHOW THE ROM2 STUFF */
	vSERIAL_sout("ROM2#", 5);
	vSERIAL_UIV8out(ucActiveROM2LocalIdx);

	if (ucActiveROM2LocalIdx != 0) {
		vSERIAL_bout('<');

		for (ucc = 0; ucc < MAXNUMTASKS; ucc++) {
			ucTask_GetField(ucc, TSK_ID, &ulTaskID);
			uiTaskID = (uint) ulTaskID;
#if 0
			vSERIAL_sout("\r\nActnNum= ", 11);
			vSERIAL_UIV8out(ucActionNum);
			vSERIAL_crlf();
#endif

			if (uiTaskID == TASK_ID_ROM) {
				/* SHOW THE NAME */
				vTask_showTaskName(ucc);
				ucTask_GetField(ucc, PARAM_SN, &ulSN);
				uiSN = (uint) ulSN;
				vSERIAL_UI16out(uiSN);

				/* SHOW THE MISS COUNT IN PARENTHESIS */
				ucLNKBLK_ReadMsdMsgCount(uiSN, &ucMissCnt);
				if (ucMissCnt != 0) {
					vSERIAL_bout('(');
					vSERIAL_UIV8out(ucMissCnt);
					vSERIAL_bout(')');
				}

				vSERIAL_bout(',');
			}
		}/* END: for(ucc) */
		vSERIAL_bout(' ');
	}/* END: if() */

	if (ucCRLF_termFlag)
		vSERIAL_crlf();

	return;

}/* END: vSTBL_showSOM2andROM2counts() */

void vCommTest(void)
{
	S_Edge S_edgeListTemp[20];

	// Build a test network
	S_edgeListTemp[0].m_uiSrc = (0x770);
	S_edgeListTemp[0].m_uiDest = (0x771);

	S_edgeListTemp[1].m_uiSrc = (0x770);
	S_edgeListTemp[1].m_uiDest = (0x778);

	S_edgeListTemp[2].m_uiSrc = 0x771;
	S_edgeListTemp[2].m_uiDest = 0x772;

	S_edgeListTemp[3].m_uiSrc = 0x772;
	S_edgeListTemp[3].m_uiDest = 0x773;

	S_edgeListTemp[4].m_uiSrc = 0x772;
	S_edgeListTemp[4].m_uiDest = 0x774;

	S_edgeListTemp[5].m_uiSrc = 0x778;
	S_edgeListTemp[5].m_uiDest = 0x775;

	S_edgeListTemp[6].m_uiSrc = 0x778;
	S_edgeListTemp[6].m_uiDest = 0x776;

	S_edgeListTemp[7].m_uiSrc = 0x776;
	S_edgeListTemp[7].m_uiDest = 0x777;

	S_edgeListTemp[8].m_uiSrc = 0x776;
	S_edgeListTemp[8].m_uiDest = 0x780;

	S_edgeListTemp[9].m_uiSrc = 0x776;
	S_edgeListTemp[9].m_uiDest = 0x781;

	S_edgeListTemp[10].m_uiSrc = 0x776;
	S_edgeListTemp[10].m_uiDest = 0x782;

	S_edgeListTemp[11].m_uiSrc = 0x776;
	S_edgeListTemp[11].m_uiDest = 0x783;

	S_edgeListTemp[12].m_uiSrc = 0x776;
	S_edgeListTemp[12].m_uiDest = 0x784;

	S_edgeListTemp[13].m_uiSrc = 0x776;
	S_edgeListTemp[13].m_uiDest = 0x785;

	S_edgeListTemp[14].m_uiSrc = 0x776;
	S_edgeListTemp[14].m_uiDest = 0x786;

	S_edgeListTemp[15].m_uiSrc = 0x776;
	S_edgeListTemp[15].m_uiDest = 0x787;

	S_edgeListTemp[16].m_uiSrc = 0x776;
	S_edgeListTemp[16].m_uiDest = 0x788;

	S_edgeListTemp[17].m_uiSrc = 0x776;
	S_edgeListTemp[17].m_uiDest = 0x789;

	S_edgeListTemp[18].m_uiSrc = 0x776;
	S_edgeListTemp[18].m_uiDest = 0x78A;

	S_edgeListTemp[19].m_uiSrc = 0x776;
	S_edgeListTemp[19].m_uiDest = 0x78B;

	ucRoute_NodeJoin(0, 0x770, S_edgeListTemp, 15);

	vRoute_DisplayEdges();

//	vComm_SendLRQ(0x12);
//
//	ucComm_WaitFor_LRQ();
}

//! @} 
