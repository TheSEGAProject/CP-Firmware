/**************************  REPORT.C  *****************************************
 *
 * Routines to perform REPORTs during events
 *
 *
 * V1.00 10/04/2003 wzr
 *		Started
 *
 ******************************************************************************/

#include "DIAG.h"			//Diagnostic package
/*lint -e526 *//* function not defined */
/*lint -e657 *//* Unusual (nonportable) anonymous struct or union */
/*lint -e714 *//* symbol not referenced */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
/*lint -e757 *//* global declarator not referenced */
/*lint -e752 *//* local declarator not referenced */
/*lint -e758 *//* global union not referenced */
/*lint -e768 *//* global struct member not referenced */

#include "STD.h"			//standard defines
#include "CRC.h"			//CRC calculation module
#include "SERIAL.h" 		//serial IO port stuff
#include "TIME.h"			//Time routines
#include "REPORT.h"			//report generator routines
#include "MODOPT.h"			//Modify Options routines
#include "SD_Card.h"
#include "mem_mod.h"		// Memory module
#include "task.h"
#include "comm.h"			// Communications module
#include "main.h"			// Main functions
#include "flash_mcu.h"

//! \def CRISIS_START
//! \brief Start address of the crisis message section in flash (cannot be changed without modifying linker file)
#define CRISIS_START	0x10000
//! \def CRISIS_END
//! \brief End address of the crisis message section in flash (cannot be changed without modifying linker file)
#define CRISIS_END		0x1FFFF
//! \def CRISIS_NFL
//! \brief Address of the next free location variable in the crisis log
#define CRISIS_NFL_ADDR		0x10000
//! \def CRISIS_MSG_COUNT
//! \brief Address of the message count variable in the crisis log
#define CRISIS_MSG_COUNT		0x10004
//! \def CRISIS_META_LENGTH_BYTES
//! \brief Length of the metadata section in the crisis log (segment 0)
#define CRISIS_META_LENGTH_BYTES		512
//! \def SEGMENT_LENGTH_BYTES
//! \brief Length of segments in the crisis section of flash
#define SEGMENT_LENGTH_BYTES				512L
//! \def SEGMENT_LENGTH_LONG
//! \brief Length of segments in the crisis section of flash in 32-bit longs
#define SEGMENT_LENGTH_LONG					128L


extern volatile uchar ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];
extern unsigned long ulGLOB_msgSysLFactor; //global load factor

//! \var g_lHourlyTime
//! \brief Used to store the time on hourly intervals.  It is used in the algorithm that calculates system message load.
long g_lHourlyTime = 0;
//! \var g_lMessageCount
//! \brief Stores number of messages generated per hour
long g_lMessageCount;

struct S_Queue S_RAM_Queue;

/*****************************  CODE STARTS HERE  ****************************/

////////////////////////////////////////////////////////////////////////////////
//! \brief Initializes the variables for the on-chip RAM message queue
//!
//! \param none
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vReport_RAM_QueueInit(void)
{
	uint uiCounter;

	//Clean Buffer
	for (uiCounter = 0; uiCounter < QUEUE_SIZE; uiCounter++)
	{
		S_RAM_Queue.m_ucaQueue[uiCounter] = 0x00;
	}

	// Set message count and pointers to 0
	S_RAM_Queue.m_uiQueueCount = 0x00;
	S_RAM_Queue.m_uiQueueHead = 0x00;
	S_RAM_Queue.m_uiQueueTail = 0x00;

}

////////////////////////////////////////////////////////////////////////////////
//! \brief Returns the number of messages in the queue
//!
//! \param none
//! \return S_RAM_Queue.m_uiQueueCount, messages in the queue
////////////////////////////////////////////////////////////////////////////////
uint uiReport_RAM_QueueCount(void)
{

	return S_RAM_Queue.m_uiQueueCount;

}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Stores messages to the on-chip RAM and to flash if the message is crisis worthy
//!
//! \param none
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vReport_LogDataElement(uchar ucPriority)
{
	uchar ucIndex;
	uchar ucDE_Length;
	uchar ucReportingPriority;

	// Get the reporting priority of the system
	ucReportingPriority = ucL2FRAM_GetReportingPriority();
	if(ucReportingPriority > MAXREPORTPRIORITY)
		ucReportingPriority = DEFAULTREPORTINGPRIORITY;

	// If this data element priority is less than the systems reporting priority then exit without saving
	if(ucPriority < ucReportingPriority)
		return;

	// Delete the previous message
	for (ucIndex = 0; ucIndex < MAX_DE_LEN; ucIndex++)
		S_RAM_Queue.m_ucaQueue[S_RAM_Queue.m_uiQueueTail + ucIndex] = 0x00;

	// the length of the DE is the second byte in the buffer
	ucDE_Length = ucaMSG_BUFF[SP_MSG_LEN_IDX];

	// Range check the length of the data element
	if (ucDE_Length > MAX_DE_LEN)
		ucDE_Length = MAX_DE_LEN;

	// Loop through the message buffer and write contents to the RAM queue
	for (ucIndex = 0; ucIndex < ucDE_Length; ucIndex++)
	{
		S_RAM_Queue.m_ucaQueue[S_RAM_Queue.m_uiQueueTail + ucIndex] = ucaMSG_BUFF[ucIndex];
	}

	// Increment the tail to point to the next free location
	S_RAM_Queue.m_uiQueueTail = S_RAM_Queue.m_uiQueueTail + MAX_DE_LEN;

	// Update the DE count
	if (S_RAM_Queue.m_uiQueueTail >= S_RAM_Queue.m_uiQueueHead){
		S_RAM_Queue.m_uiQueueCount = (S_RAM_Queue.m_uiQueueTail - S_RAM_Queue.m_uiQueueHead) / MAX_DE_LEN;
	}
	else {
		S_RAM_Queue.m_uiQueueCount = ((QUEUE_SIZE - S_RAM_Queue.m_uiQueueHead) + S_RAM_Queue.m_uiQueueTail) / MAX_DE_LEN;
	}

	//If the RAM queue is too close to the max then reset to the start
	if (S_RAM_Queue.m_uiQueueTail == QUEUE_SIZE)
		S_RAM_Queue.m_uiQueueTail = 0x00;

	// Log in flash if the report is critical
	if(ucPriority == MAXREPORTPRIORITY)
		ucReport_CrisisLog();
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Reads messages from the on-chip RAM
//!
//! \param none
//! \return 0 success, 1 error
////////////////////////////////////////////////////////////////////////////////
uchar ucReport_ReadDEFromRAM(uchar *p_ucaBuff)
{
	uchar ucIndex;
	uint ucMessageLength;

	// If there are no messages in the queue then return
	if (S_RAM_Queue.m_uiQueueCount == 0)
		return 1;

	// Get the length of the first message in the queue
	ucMessageLength = S_RAM_Queue.m_ucaQueue[S_RAM_Queue.m_uiQueueHead + 1];

	if (ucMessageLength > MAX_DE_LEN)
		return 1;

	// Loop through the queue and write contents to the message buffer
	for (ucIndex = 0; ucIndex < ucMessageLength; ucIndex++)
	{
		// Allow room in the message buffer for the network layer information
		*p_ucaBuff++ = S_RAM_Queue.m_ucaQueue[S_RAM_Queue.m_uiQueueHead + ucIndex];
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Removes message from the on-chip RAM
//!
//! The message is not actually deleted from memory but the pointer
//! points to the next message ignoring the old one
//!
//! \param none
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vReport_RemoveDEFromRAM(void)
{

	// Decrement the counter for the number of messages in the queue
	S_RAM_Queue.m_uiQueueCount--;

	// Increment the head to point at the next message
	S_RAM_Queue.m_uiQueueHead = S_RAM_Queue.m_uiQueueHead + MAX_DE_LEN;

	if (S_RAM_Queue.m_uiQueueHead == QUEUE_SIZE)
		S_RAM_Queue.m_uiQueueHead = 0x00;

}
////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Builds messages from the data elements generated during the slot
//!		     The network headers are added before transmission and the DEs are
//!				 correctly positioned in memory so when the message is pulled out
//!				 formatting of DEs is taken care of.
//!
//! \param none
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vReport_BuildMsgsFromDEs(void)
{
	uint uiNumOfDE;
	uint uiDECount;
	uint uiMsgNumber;
	uchar ucCount;
	uchar ucDELength;
	uchar ucMsgLength;
	uchar ucMsgPtr;
	uchar ucaTemp[MAX_DE_LEN]; // temporary buffer for storing the DE

	// Clean out the message buffer
	vSERIAL_CleanBuffer((uchar *)ucaMSG_BUFF);

	// Get the number of DEs in RAM  if there are none then exit
	uiNumOfDE = uiReport_RAM_QueueCount();
	if (uiNumOfDE == 0)
		return;

	// Start the message length at the start of the payload
	ucMsgLength = MSG_HDR_SZ;
	ucMsgPtr = MSG_IDX_PAYLD;
	for(uiDECount=0; uiDECount<uiNumOfDE; uiDECount++)
	{
	
		// if the message is bad
		if (ucReport_ReadDEFromRAM(ucaTemp) == 0) {
			// Get the length of the DE and update the message length
			ucDELength = ucaTemp[0x01];
			
			// If the remaining space is less than the length of the DE then store the message
			if((MAX_MSG_SIZE - (ucMsgLength + NET_HDR_SZ + CRC_SZ)) < ucDELength)
			{
				ucaMSG_BUFF[MSG_IDX_LEN] = ucMsgLength; //write the message length

				// Build the operational message header
				uiMsgNumber = uiComm_incMsgSeqNum();
				vComm_Msg_buildOperational(MSG_FLG_SINGLE, uiMsgNumber, uiL2FRAM_getSnumLo16AsUint(), MSG_ID_OPERATIONAL);

				ucMsgLength = MSG_HDR_SZ; //reset the message length
				ucMsgPtr = MSG_IDX_PAYLD;

				// log the report
				vREPORT_LogReport();

				//store the message in SRAM
				vL2SRAM_storeMsgToSramIfAllowed();

				// Log it on SD card
				vREPORT_LogReport();

				// Clean out the message buffer
				vSERIAL_CleanBuffer((uchar *) ucaMSG_BUFF);
			}

			// Write the DE to the message buffer
			for(ucCount=0; ucCount<ucDELength; ucCount++)
				ucaMSG_BUFF[ucMsgPtr + ucCount] = ucaTemp[ucCount];
			
			// Add the length of the DE to the length of the message
			ucMsgLength += ucDELength;
			ucMsgPtr += ucDELength;

			// Once the DE is written to the MSG_BUFF then remove it from RAM 
			vReport_RemoveDEFromRAM();

			// If there are no more messages in the Queue then write what we have to SRAM
			if(uiReport_RAM_QueueCount() == 0)
			{
				ucaMSG_BUFF[MSG_IDX_LEN] = ucMsgLength; //write the message length

				// Build the operational message header
				uiMsgNumber = uiComm_incMsgSeqNum();
				vComm_Msg_buildOperational(MSG_FLG_SINGLE, uiMsgNumber, uiL2FRAM_getSnumLo16AsUint(), MSG_ID_OPERATIONAL);

				//store the message in SRAM
				vL2SRAM_storeMsgToSramIfAllowed();

				// Log to SD card
				vREPORT_LogReport();

				// Clean out the message buffer
				vSERIAL_CleanBuffer((uchar *)ucaMSG_BUFF);
			}
		}
	}
	// Reset the queue for the next slot
	vReport_RAM_QueueInit();
}


////////////////////////////////////////////////////////////////////////////////
//!	\fn vREPORT_LogReport
//!
//! \brief Stores messages in the FRAM buffers.  When the SD card buffer is almost
//! full this function request that the SD card task gets scheduled.
//!
//!
//! \param none
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vREPORT_LogReport(void)
{
	uchar ucRepLength;
	S_Task_Ctl S_Task;
	uchar ucTaskIndex;
	long lTime;

	ucRepLength = ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ;

	// Gaurd in case of message length error
	if(ucRepLength > MAX_MSG_SIZE)
		ucRepLength = MAX_MSG_SIZE;

	// Get the current time
	lTime = lTIME_getSysTimeAsLong();

	// Initialize system load variables
	if(g_lHourlyTime == 0){
		g_lHourlyTime = lTime;
		g_lMessageCount = 0;
	}

	// Run system load calculations
	if((lTime - g_lHourlyTime) >= 3600){
		ulGLOB_msgSysLFactor = g_lMessageCount;
		g_lMessageCount = 0;
		g_lHourlyTime = lTime;
	}
	else if((lTime - g_lHourlyTime) < 3600){
		ulGLOB_msgSysLFactor = 3600/(lTime - g_lHourlyTime);
		ulGLOB_msgSysLFactor *= g_lMessageCount;
	}

	// Increment the message count
	g_lMessageCount++;

	// Write in the SD card address at the network layer
	vComm_NetPkg_buildHdr(0xFFF0);

	// If writing the report returns 1 then the FRAM buffer is full
	if (ucL2FRAM_WriteReportToSDCardBuff(ucaMSG_BUFF, ucRepLength) == 1)
	{
		// Get the task index
		ucTaskIndex = ucTask_FetchTaskIndex(TASK_ID_FRAM_TO_SDCARD);

		// Only create the task if it does not already exist
		if (ucTaskIndex== INVALID_TASKINDEX)
		{
			// Create the write FRAM to SD card task
			S_Task.m_uiTask_ID = TASK_ID_FRAM_TO_SDCARD;
			S_Task.m_uiFlags = (F_SUICIDE | TASK_FLAGS_FRAM_TO_SDCARD);
			S_Task.m_ucPriority = TASK_PRIORITY_FRAM_TO_SDCARD;
			S_Task.m_ucProcessorID = (uchar) (TASK_ID_FRAM_TO_SDCARD >> 8);
			S_Task.m_ulTransducerID = (ulong) (TASK_ID_FRAM_TO_SDCARD & TASK_TRANSDUCER_ID_MASK);
			S_Task.m_ucState = TASK_STATE_ACTIVE;
			S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_FRAM_TO_SDCARD;
			S_Task.m_ulParameters[0] = 0;
			S_Task.m_ulParameters[1] = 0;
			S_Task.m_ulParameters[2] = 0;
			S_Task.m_ulParameters[3] = 0;
			S_Task.m_cName = "SD   ";
			S_Task.ptrTaskHandler = vTask_FRAM_to_SDCard;

			ucTask_CreateTask(S_Task);

			// Update the task index if the task was created
			ucTaskIndex = ucTask_FetchTaskIndex(TASK_ID_FRAM_TO_SDCARD);
		}

		// Try to schedule it as soon as possible (if not normal scheduler will get it done)
		vRTS_schedule_ASAP(ucTaskIndex, lTIME_getSysTimeAsLong());
	}

}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Reads the most recently stored block from the SD card and stores the
//! contents in SRAM.
//!
//! If the batteries die in a WiSARD then it dumps all data to the SD card
//! When the batteries are replaced the data resides in the SD card but has
//! not yet reached the database.  In this case we read the most recent block
//! from the SD card and put it in the message queue.
//!
//! \param none
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vReport_LoadSRAMFromSDCard(void)
{
	ulong ulAddress;
	uchar ucMsgIndex;
	uchar ucBlockIndex;
	uchar ucBlock[SD_CARD_BLOCKLEN];
	uchar ucErrorCode;
	uchar ucErrorCodePriority;
	uint uiCount;

	// Get the address of the last block written to
	ulAddress = ulL2FRAM_GetLastSDCardBlockNum();

	// Assume no errors
	ucErrorCode = ucErrorCodePriority = 0;

	// Power up the SD card and run the initialization sequence
	vSD_PowerOn();
	if (ucSD_Init())
	{
		ucErrorCode = SRC_ID_SDCARD_INIT_FAIL;
		ucErrorCodePriority = RPT_PRTY_SDCARD_INIT_FAIL;
#if 0
		vSERIAL_sout("SD Init Fail", 12);
#endif
	}
	else
	{
		// Only proceed if setting the block length worked
		if (ucErrorCode == 0)
		{
			// Write the block to the SD card
			for (uiCount = 0; uiCount < SD_CMD_TIMEOUT; uiCount++)
			{
				if (SD_Read_Block(ucBlock, ulAddress) == SD_SUCCESS)
					break;
			}
			if (uiCount == SD_CMD_TIMEOUT)
			{
				ucErrorCode = SRC_ID_SDCARD_READ_FAIL;
				ucErrorCodePriority = RPT_PRTY_SDCARD_READ_FAIL;
#if 0
				vSERIAL_sout("SD read fail\r\n", 14);
#endif
			}
		}
		//Power down the SD card
		vSD_PowerOff();
	}

	// If there was a failure report it
	if (ucErrorCode != 0)
	{

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		// Load the rest of the DE
		ucaMSG_BUFF[ucMsgIndex++] = ucErrorCode;
		ucaMSG_BUFF[ucMsgIndex++] = 0; // data length

		// Store DE
		vReport_LogDataElement(ucErrorCodePriority);
	}
	else
	{
		// Loop through the block and parse out the messages
		for (ucBlockIndex = 0; ucBlockIndex < SD_CARD_BLOCKLEN;)
		{
			// Load the message header
			for (ucMsgIndex = 0; ucMsgIndex < MSG_IDX_PAYLD;)
			{
				ucaMSG_BUFF[ucMsgIndex++] = ucBlock[ucBlockIndex++];
			}

			// Exit once all the messages are retrieved or we reach a corrupted message
			if(ucaMSG_BUFF[MSG_IDX_LEN] == 0 || ucaMSG_BUFF[MSG_IDX_LEN] > MAX_MSG_SIZE)
				return;

			// once the header is acquired we know the message size so read the rest of the message
			while (ucMsgIndex < ucaMSG_BUFF[MSG_IDX_LEN])
			{
				ucaMSG_BUFF[ucMsgIndex++] = ucBlock[ucBlockIndex++];
			}

			// Write the message to SRAM
			vL2SRAM_storeMsgToSram();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//! \fn ucReport_CrisisLog
//! \brief Logs critical data elements into flash memory of the MCU
//!
//!
///////////////////////////////////////////////////////////////////////////////////
uchar ucReport_CrisisLog(void)
{
	uchar ucIndex;
	uchar ucDE_Length;
	ulong ulNextAddr, ulSegmentStart;
	ulong ulSegmentPtr, ulSegmentNum, ulTempPtr;
	ulong ulDataSegment[128], ulMetaSegment[128];

	// Fetch a local copy of the metadata segment
	vFlash_Read_Segment(ulMetaSegment, CRISIS_NFL_ADDR);
	ulNextAddr = ulMetaSegment[0];
	ulSegmentNum = ulMetaSegment[1];
	ulSegmentPtr = ulMetaSegment[2];

	// If the address is out of range then reset to the start
	if(ulNextAddr < (CRISIS_START + CRISIS_META_LENGTH_BYTES) || ulNextAddr > CRISIS_END){
		ulNextAddr = (CRISIS_START + CRISIS_META_LENGTH_BYTES);
		ulSegmentNum = 1;
		ulSegmentPtr = 0;
		ulMetaSegment[3] = 1; // Indicates that there was a rollover in the crisis log
	}

	// the length of the DE is the second byte in the buffer
	ucDE_Length = ucaMSG_BUFF[SP_MSG_LEN_IDX];

	// Range check the length of the data element
	if (ucDE_Length > MAX_DE_LEN)
		ucDE_Length = MAX_DE_LEN;

	// If we are going to overflow this segment with the next data element then start the next segment
	if(((ulSegmentPtr*4) + ucDE_Length) > SEGMENT_LENGTH_BYTES){
		ulSegmentNum++;
		ulSegmentPtr = 0;
		ulNextAddr = (CRISIS_START + ulSegmentNum*SEGMENT_LENGTH_BYTES);

		// If the address is out of range then reset to the start
		if(ulNextAddr < (CRISIS_START + CRISIS_META_LENGTH_BYTES) || ulNextAddr > CRISIS_END){
			ulNextAddr = (CRISIS_START + CRISIS_META_LENGTH_BYTES);
			ulSegmentNum = 1;
			ulSegmentPtr = 0;
		}
	}

	// Read the current data segment from flash
	ulSegmentStart = (CRISIS_START + (ulSegmentNum * SEGMENT_LENGTH_BYTES));
	vFlash_Read_Segment(ulDataSegment,  ulSegmentStart);

	ulTempPtr = ulSegmentPtr;
	// Write the data element to an array of longs (flash programming is faster with longs than bytes)
	for (ucIndex = 0; ucIndex < ucDE_Length;) {
		ulDataSegment[ulTempPtr] = (ulong)ucaMSG_BUFF[ucIndex++] << 24;
		ulDataSegment[ulTempPtr] |= (ulong)ucaMSG_BUFF[ucIndex++] << 16;
		ulDataSegment[ulTempPtr] |= (ulong)(ucaMSG_BUFF[ucIndex++] << 8);
		ulDataSegment[ulTempPtr++] |= (ulong)ucaMSG_BUFF[ucIndex++];
	}

	// Write the segment back to flash
	vFlash_Write_Segment(ulDataSegment, ulSegmentStart);

	// Update the next free address
	ulMetaSegment[0] = ulNextAddr + MAX_DE_LEN;
	ulMetaSegment[1] = ulSegmentNum;
	ulMetaSegment[2] = ulSegmentPtr + (MAX_DE_LEN/4); // Increment the pointer by the max DE length
	vFlash_Write_Segment(ulMetaSegment, CRISIS_NFL_ADDR);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucReport_PrintCrisisLog
//! \brief Composes data elements in the crisis log into messages and prints to terminal
//!
//////////////////////////////////////////////////////////////////////////////////////////
void vReport_PrintCrisisLog(void){

	uint uiLongIndex;
	uint uiByteIndex;
	ulong ulDataSegment[128], ulMetaSegment[128];
	uchar ucDataSegmentBytes[512];
	uchar ucDELength;
	ulong ulNextAddr, ulSegmentStart;
	ulong ulSegmentPtr, ulSegmentNum;
	ulong ulSegmentCounter;
	uchar ucMsgPtr;
	uint uiMsgNumber;
	uint uiDEPtr;
	ulong ulRollOverFlag;

	// Fetch a local copy of the metadata segment
	vFlash_Read_Segment(ulMetaSegment, CRISIS_NFL_ADDR);
	ulNextAddr = ulMetaSegment[0];
	ulSegmentNum = ulMetaSegment[1];
	ulSegmentPtr = ulMetaSegment[2];
	ulRollOverFlag = ulMetaSegment[3];

	// If there has been a roll over then set the segment number and pointer to force this function to print out all of the crisis log
	if(ulRollOverFlag == 1){
		ulSegmentNum = 128; // last segment
		ulSegmentPtr = 512;	// last byte in segment
	}

	// If the address is out of range then reset to the start (should only happen when the device is reprogrammed)
	if(ulNextAddr < (CRISIS_START + CRISIS_META_LENGTH_BYTES) || ulNextAddr > CRISIS_END){
		ulNextAddr = (CRISIS_START + CRISIS_META_LENGTH_BYTES);
		ulSegmentNum = 1;
		ulSegmentPtr = 0;
	}

	// Loop over all the segments
	for(ulSegmentCounter = 1; ulSegmentCounter < ulSegmentNum; ulSegmentCounter++){

		// Read the current data segment from flash
		ulSegmentStart = (CRISIS_START + (ulSegmentCounter * SEGMENT_LENGTH_BYTES));
		vFlash_Read_Segment(ulDataSegment, ulSegmentStart);

		// Write the segment to a byte array
		uiByteIndex = 0;
		for (uiLongIndex = 0; uiLongIndex < 128; uiLongIndex++) {
			ucDataSegmentBytes[uiByteIndex++] = (uchar) (ulDataSegment[uiLongIndex] >> 24);
			ucDataSegmentBytes[uiByteIndex++] = (uchar) (ulDataSegment[uiLongIndex] >> 16);
			ucDataSegmentBytes[uiByteIndex++] = (uchar) (ulDataSegment[uiLongIndex] >> 8);
			ucDataSegmentBytes[uiByteIndex++] = (uchar) ulDataSegment[uiLongIndex];
		}

		// Init DE pointer to point to the first DE in the segment
		uiDEPtr = 0;

		while(uiDEPtr < 512){

		// Get the length of the DE and update the message length
		ucDELength = ucDataSegmentBytes[(uint)(uiDEPtr+1)];

		// Range check the length of the data element
		if (ucDELength > MAX_DE_LEN)
			ucDELength = MAX_DE_LEN;

		// Prepend network layer with destination address
		vComm_NetPkg_buildHdr(0xFFFF);

		// Build the operational message header
		uiMsgNumber = uiComm_incMsgSeqNum();
		vComm_Msg_buildOperational(MSG_FLG_SINGLE, uiMsgNumber, uiL2FRAM_getSnumLo16AsUint(), MSG_ID_OPERATIONAL);

		//write the message length
		ucaMSG_BUFF[MSG_IDX_LEN] = MSG_HDR_SZ + ucDELength;

		// Start the message length at the start of the payload
		ucMsgPtr = MSG_IDX_PAYLD;

		// Write the DE into the message
		for(uiByteIndex = 0; uiByteIndex < ucDELength; uiByteIndex++){
			ucaMSG_BUFF[ucMsgPtr++] = ucDataSegmentBytes[(uint)(uiDEPtr + uiByteIndex)];
		}

		// COMPUTE THE CRC
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ) ; //lint !e534 //compute the CRC

		// Print out the message
		for(uiByteIndex = 0; uiByteIndex < (ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); uiByteIndex++){
		vSERIAL_HB8out(ucaMSG_BUFF[uiByteIndex]);
		}
		vSERIAL_crlf();

		// Increment the DE pointer to the next data element
		uiDEPtr += MAX_DE_LEN;

		// If we are at the last message in the last segment then break
		if(ulSegmentCounter == ulSegmentNum && uiDEPtr >= ulSegmentPtr*4)
			break;
		}
	}

}

/////////////////////////////////////////////////////////////////////////////////////////
//! \fn vReport_EraseCrisisLog
//! \brief Erases all of the crisis log
//!
/////////////////////////////////////////////////////////////////////////////////////////
void vReport_EraseCrisisLog(void){
	ulong ulCounter;
	ulong ulSegmentNum, ulMaxSegment;
	ulong ulMetaSegment[128];

	// Calculate the number of segments
	ulMaxSegment = (CRISIS_END - CRISIS_START)/SEGMENT_LENGTH_BYTES;

	// Loop over all the segments
	for(ulCounter = 0; ulCounter < ulMaxSegment; ulCounter++){

		ulSegmentNum = (CRISIS_START + (ulCounter * SEGMENT_LENGTH_BYTES));

		// Write the segment back to flash
		vFlash_Erase_Segment(ulSegmentNum);
	}

	// Fetch a local copy of the metadata segment
	vFlash_Read_Segment(ulMetaSegment, CRISIS_NFL_ADDR);
	// Update the meta data section
	ulMetaSegment[0] = CRISIS_START + CRISIS_META_LENGTH_BYTES;
	ulMetaSegment[1] = 1;
	ulMetaSegment[2] = 0;
	ulMetaSegment[3] = 0;

	vFlash_Write_Segment(ulMetaSegment, CRISIS_NFL_ADDR);

}


uchar ucReport_Test(void)
{

	uint uiCounter;

//	// Verify that the correct DE are being written to Flash based on message priority
//	for(uiCounter = 0; uiCounter < 30; uiCounter++){
//		vTask_Batt_Sense(); // Reduce voltage during test to failure, triggering the write to the crisis log
//		vTask_MCUTemp();
//	}

	// Verify that flash is being filled properly, each DE is allocated space and segment overflow handled properly
	for(uiCounter = 0; uiCounter < 5; uiCounter++){
		vTask_Batt_Sense(); // Reduce voltage during test to failure, triggering the write to the crisis log
		vReport_LoadSRAMFromSDCard(); // SD Card removed to trigger crisis log
		vTask_Batt_Sense(); // Reduce voltage during test to failure, triggering the write to the crisis log
		vReport_LoadSRAMFromSDCard(); // SD Card removed to trigger crisis log
		vTask_Batt_Sense(); // Reduce voltage during test to failure, triggering the write to the crisis log

	}

	// Read and print out the crisis log
	vReport_PrintCrisisLog();
	vReport_EraseCrisisLog();
	vReport_PrintCrisisLog();

	return 1;
}

/*-------------------------------  MODULE END  ------------------------------*/
