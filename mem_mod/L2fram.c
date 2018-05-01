


/**************************  L2FRAM.C  *****************************************
 *
 * Level 2 Routines to read and write the FERRO-RAM chip
 *
 *
 * V1.00 12/05/2003 wzr
 *	Started
 *
 ******************************************************************************/

/*lint -e526 *//* function not defined */
/*lint -e657 *//* Unusual (nonportable) anonymous struct or union */
/*lint -e714 *//* symbol not referenced */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
/*lint -e757 *//* global declarator not referenced */
/*lint -e758 *//* global union not referenced */
/*lint -e768 *//* global struct member not referenced */

#include "L2fram.h" 		//Level 2 FRAM handler routines
#include "diag.h"			//diagnostic defines
#include "std.h"			//standard defines
#include "serial.h"			//serial port routines
#include "rom.h" 			//Rom configuration data
#include "comm.h"			//msg routines
#include "task.h"			//Task management routines
#include "time.h"			//system type routines
#include "sensor.h"			//sensor name
#include "modopt.h" 		//role flags for wiz routines
#include "fram.h"	//Ferro ram memory functions
#include "config.h" 	//configuration parameters
#include "report.h"
#include "main.h"
/**********************  DEFINES  *******************************************/

#define FRAM_ID_ADDR_XI						0	//4 bytes
#define FRAM_ID_ADDR_HI						1
#define FRAM_ID_ADDR_MD						2
#define FRAM_ID_ADDR_LO						3
#define FRAM_ID_VAL_XI							'F'
#define FRAM_ID_VAL_HI							'R'
#define FRAM_ID_VAL_MD							'A'
#define FRAM_ID_VAL_LO							'M'

#define FRAM_VER_ADDR							4	//2 bytes
#define FRAM_VER_ADDR_HI					4
#define FRAM_VER_ADDR_LO					5

#define FRAM_TEST_VAL_POSITIVE_UL	0xCC118855
#define FRAM_TEST_VAL_NEGATIVE_UL	(~FRAM_TEST_VAL_POSITIVE_UL)

#define FRAM_ID							1
#define VERSION							2
#define	TEST_ADDRESS				3
#define	SD_CARD_PTRS				4
#define	TIME								5
#define REBOOT_COUNT				6
#define NETWORK_ID					7
#define OPTION_BYTES				8
#define	SHUTDOWN_STATE			9
#define REPORTINGPRIORITY		10
#define	TASK_STATE_BLOCKS		11
#define SD_CARD_BUFFER			12
#define	Y_TRIGGER						13


/**********************  DECLARATIONS  ***************************************/

static void vL2FRAM_Init_SDCardPtrs(void);
static void vL2FRAM_CleanSDCardBuff(void);
static void vL2FRAM_setTSBTblCount(uchar ucBlkCount);
void vL2FRAM_WriteNFL_SDCardBuff(unsigned int uiAddress);
/////////////////////////////////////////////////////////////////////////////
//! \fn vFRAM_SecureAllMemory
//!
//! \brief Secures all memory locations in FRAM
/////////////////////////////////////////////////////////////////////////////
void vL2FRAM_LockAllMemory(void){

	// Lock memory at the driver level
	vFRAM_Security(0xFFFF, 0xFFFF);
}

/////////////////////////////////////////////////////////////////////////////
//! \fn vL2FRAM_SetSecurity
//!
//! \brief Secures all memory locations in FRAM
/////////////////////////////////////////////////////////////////////////////
void vL2FRAM_SetSecurity(uchar ucSection, uchar ucState){

	uint uiStartAddress;
	uint uiEndAddress;

	// If state is lock then set addresses to the locked state
	if (ucState == FRAM_LOCK) {
		uiStartAddress = 0xFFFF;
		uiEndAddress = 0xFFFF;

		// Set the security at the driver level
		vFRAM_Security(uiStartAddress, uiEndAddress);

		return;
	}

	// Set unlock bit and unlock section of memory at the driver level
	switch (ucSection)
	{
		case FRAM_ID:
			uiStartAddress = FRAM_ID_ADDR_XI;
			uiEndAddress = FRAM_ID_ADDR_LO;
		break;

		case VERSION:
			uiStartAddress = FRAM_VER_ADDR_HI;
			uiEndAddress = FRAM_VER_ADDR_LO;
		break;

		case TEST_ADDRESS:
			uiStartAddress = FRAM_TEST_ADDR_XI;
			uiEndAddress = FRAM_TEST_ADDR_LO;
		break;

		case SD_CARD_PTRS:
			uiStartAddress = FRAM_SDCARD_BLOCK_NUM_ADDR;
			uiEndAddress = FRAM_SDCARD_BUFF_END_PTR_ADDR + 1;
		break;

		case TIME:
			uiStartAddress = FRAM_TIME_SAVE_AREA_ADDR;
			uiEndAddress = FRAM_TIME_SAVE_AREA_ADDR + 3;
		break;

		case REBOOT_COUNT:
			uiStartAddress = FRAM_REBOOT_COUNT_ADDR;
			uiEndAddress = FRAM_REBOOT_COUNT_ADDR + 1;
		break;

		case NETWORK_ID:
			uiStartAddress = FRAM_USER_ID_ADDR;
			uiEndAddress = FRAM_USER_ID_ADDR + 1;
		break;

		case OPTION_BYTES:
			uiStartAddress = FRAM_OPTION_BYTE_0_ADDR;
			uiEndAddress = FRAM_OPTION_IDX_ADDR;
		break;

		case SHUTDOWN_STATE:
			uiStartAddress = FRAM_STATE_ON_SHUTDOWN_ADDR;
			uiEndAddress = FRAM_STATE_ON_SHUTDOWN_ADDR;
		break;

		case REPORTINGPRIORITY:
			uiStartAddress = FRAM_RPT_PRTY_ADDR;
			uiEndAddress = FRAM_RPT_PRTY_ADDR;
		break;

		case TASK_STATE_BLOCKS:
			uiStartAddress = FRAM_ST_BLK_COUNT_ADDR;
			uiEndAddress = FRAM_LAST_ST_BLK_ADDR;
		break;

		case SD_CARD_BUFFER:
			uiStartAddress = FRAM_SD_CARD_BUFF_BEG_ADDR;
			uiEndAddress = FRAM_SD_CARD_OVRFLO_END_ADDR;
		break;

		case Y_TRIGGER:
			uiStartAddress = FRAM_Y_TRIG_AREA_BEG_ADDR;
			uiEndAddress = FRAM_Y_TRIG_AREA_END_ADDR;
		break;

		default:
			uiStartAddress = 0xFFFF;
			uiEndAddress = 0xFFFF;
		break;
	}

	// If state is lock then set addresses to the locked state
	if (ucState == FRAM_LOCK) {
		uiStartAddress = 0xFFFF;
		uiEndAddress = 0xFFFF;
	}

	// Set the security at the driver level
	vFRAM_Security(uiStartAddress, uiEndAddress);
}

/**********************  ucL2FRAM_chk_for_fram()  *****************************
 *
 * This routine:
 *	1. turns on the FRAM
 *	2. Tests the FRAM test locations to make sure SPI and FRAM are working
 *	3. shuts off the FRAM
 *
 * RET:	1 = good
 *		0 = bad
 *
 *******************************************************************************/

uchar ucL2FRAM_chk_for_fram(uchar ucReportMode //FRAM_CHK_SILENT_MODE, FRAM_CHK_REPORT_MODE
    )
{
	uchar ucRetVal;
	ulong ulTestVal;

	/* ASSUME THE RET VAL IS GOOD */
	ucRetVal = 1;

	// Unlock the test area in FRAM
	vL2FRAM_SetSecurity(TEST_ADDRESS, FRAM_UNLOCK);

	/* WRITE THE NEG TEST VALUE IN THE TEST LOCATION */
	ucFRAM_write_B32(FRAM_TEST_ADDR, 0x11111111); //clr first
	ucFRAM_write_B32(FRAM_TEST_ADDR, FRAM_TEST_VAL_NEGATIVE_UL);

	/* READ IT BACK */
	ucFRAM_read_B32(FRAM_TEST_ADDR, &ulTestVal);

	if (ulTestVal != FRAM_TEST_VAL_NEGATIVE_UL) {
		if (ucReportMode) {
			vSERIAL_sout("FramFail ", 9);
			vSERIAL_sout(" wrote= 0x", 10);
			vSERIAL_HBV32out(FRAM_TEST_VAL_NEGATIVE_UL);
			vSERIAL_sout(" read= 0x", 9);
			vSERIAL_HBV32out(ulTestVal);
			vSERIAL_sout(" xor= 0x", 8);
			vSERIAL_HBV32out(FRAM_TEST_VAL_NEGATIVE_UL ^ ulTestVal);
			vSERIAL_crlf();
		}
		ucRetVal = 0;
	}

	/* WRITE THE POS TEST VALUE IN THE TEST LOCATION */
	ucFRAM_write_B32(FRAM_TEST_ADDR, 0x22222222); //clr first
	ucFRAM_write_B32(FRAM_TEST_ADDR, FRAM_TEST_VAL_POSITIVE_UL);

	/* READ IT BACK */
	ucFRAM_read_B32(FRAM_TEST_ADDR, &ulTestVal);
	if (ulTestVal != FRAM_TEST_VAL_POSITIVE_UL)
	{
		if (ucReportMode)
		{
			vSERIAL_sout("FramFail ", 9);
			vSERIAL_sout(" wrote= 0x", 10);
			vSERIAL_HBV32out(FRAM_TEST_VAL_POSITIVE_UL);
			vSERIAL_sout(" read= 0x", 9);
			vSERIAL_HBV32out(ulTestVal);
			vSERIAL_sout(" xor= 0x", 8);
			vSERIAL_HBV32out(FRAM_TEST_VAL_POSITIVE_UL ^ ulTestVal);
			vSERIAL_crlf();
		}
		ucRetVal = 0;
	}

	// Unlock the test area in FRAM
	vL2FRAM_SetSecurity(TEST_ADDRESS, FRAM_LOCK);

	return (ucRetVal);

}/* END: ucL2FRAM_chk_for_fram() */

/*******************  vL2FRAM_format_fram()  ********************************
 *
 * This routine formats the FRAM
 *
 *****************************************************************************/
void vL2FRAM_format_fram(void)
{
	uchar ucii, ucTSBCount;

	// Unlock the ID section
	vL2FRAM_SetSecurity(FRAM_ID, FRAM_UNLOCK);

	/* WRITE THE FRAM ID AT THE FRONT OF THE MEM */
	ucFRAM_write_B8(FRAM_ID_ADDR_XI, FRAM_ID_VAL_XI);
	ucFRAM_write_B8(FRAM_ID_ADDR_HI, FRAM_ID_VAL_HI);
	ucFRAM_write_B8(FRAM_ID_ADDR_MD, FRAM_ID_VAL_MD);
	ucFRAM_write_B8(FRAM_ID_ADDR_LO, FRAM_ID_VAL_LO);


	/* WRITE THE FRAM VERSION NUMBER */
	vL2FRAM_SetSecurity(VERSION, FRAM_UNLOCK);
	ucFRAM_write_B16(FRAM_VER_ADDR, FRAM_VERSION);

	/* WRITE THE FRAM TEST AREA */
	vL2FRAM_SetSecurity(TEST_ADDRESS, FRAM_UNLOCK);
	ucFRAM_write_B32(FRAM_TEST_ADDR, FRAM_TEST_VAL_NEGATIVE_UL);

	// Write the FRAM SD Card pointers and zero out the buffer (These functions handle security)
	vL2FRAM_Init_SDCardPtrs();
	vL2FRAM_CleanSDCardBuff();

	/* WRITE THE FRAM TIME SAVE AREA */
	vL2FRAM_SetSecurity(TIME, FRAM_UNLOCK);
	ucFRAM_write_B32(FRAM_TIME_SAVE_AREA_ADDR, (ulong) lTIME_getSysTimeAsLong());

	/* WRITE THE FRAM REBOOT COUNT AREA */
	vL2FRAM_SetSecurity(REBOOT_COUNT, FRAM_UNLOCK);
	ucFRAM_write_B16(FRAM_REBOOT_COUNT_ADDR, 0);

	/* WRITE THE FRAM WIZARD ID AREA */
	vL2FRAM_SetSecurity(NETWORK_ID, FRAM_UNLOCK);
	ucFRAM_write_B16(FRAM_USER_ID_ADDR, uiROM_getRomConfigSnumAsUint());

	// Write the shutdown state area to indicate a successful shutdown
	vL2FRAM_SetSecurity(SHUTDOWN_STATE, FRAM_UNLOCK);
	ucFRAM_write_B8(FRAM_STATE_ON_SHUTDOWN_ADDR, 0x00);

	// Set the systems reporting priority to the default
	vL2FRAM_SetReportingPriority(DEFAULTREPORTINGPRIORITY);

	/* WRITE THE DEFAULT OPTION ARRAY INTO THE FRAM ARRAY */
	vMODOPT_copyAllRomOptionsToFramOptions(DEFAULT_ROLE_IDX);
	vMODOPT_copyAllFramOptionsToRamOptions(); //copy to RAM

	// Set the task state block number to 0 and clear all TSBs
	vL2FRAM_setTSBTblCount(0);
	for(ucTSBCount = 0; ucTSBCount<FRAM_MAX_TSB_COUNT; ucTSBCount++)
		vL2FRAM_deleteTSB(ucTSBCount);

	/*----------------  NOW INIT Y TRIGGER TABLE  --------------------------*/
	for (ucii = 0; ucii < SENSOR_MAX_VALUE; ucii++){
		vL2FRAM_putYtriggerVal(ucii, 0);
	}

	// Lock FRAM
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return;

}/* END: vL2FRAM_format_fram() */

/*******************  uiL2FRAM_chk_for_fram_format()  ***********************
 *
 * This routine checks the FRAM for formatting information
 * (IE. If the Fram logo is set in bytes 0-3),
 * and reports version number if formatted and 0 if not
 *
 *
 * RET:	Version = yes FRAM is formatted
 *		0 = no FRAM is not formatted
 *
 *****************************************************************************/
uint uiL2FRAM_chk_for_fram_format(void)
{
	uint uiRetVal;
	uchar ucTemp;


	// Unlock FRAM and read the version
	vL2FRAM_SetSecurity(VERSION, FRAM_UNLOCK);
	ucFRAM_read_B16(FRAM_VER_ADDR, &uiRetVal);

	if (uiRetVal != FRAM_VERSION)
	{
#if 0
		vSERIAL_sout("L2FRAM:VerErr,ReadVer= ", 23);
		vSERIAL_HB16out(uiRetVal);
		vSERIAL_sout(" needed= ", 9);
		vSERIAL_HB16out(FRAM_VERSION);
		vSERIAL_crlf();
#endif
		uiRetVal = 0;
	}

	// Unlock the FRAM ID section
	vL2FRAM_SetSecurity(FRAM_ID, FRAM_UNLOCK);

	ucFRAM_read_B8(FRAM_ID_ADDR_XI, &ucTemp);
	if (ucTemp != FRAM_ID_VAL_XI)
		uiRetVal = 0;

	ucFRAM_read_B8(FRAM_ID_ADDR_HI, &ucTemp);
	if (ucTemp != FRAM_ID_VAL_HI)
		uiRetVal = 0;

	ucFRAM_read_B8(FRAM_ID_ADDR_MD, &ucTemp);
	if (ucTemp != FRAM_ID_VAL_MD)
		uiRetVal = 0;

	ucFRAM_read_B8(FRAM_ID_ADDR_LO, &ucTemp);
	if (ucTemp != FRAM_ID_VAL_LO)
		uiRetVal = 0;

	// Lock FRAM
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return (uiRetVal);

}/* END: uiL2FRAM_chk_for_fram_format() */


/////////////////////////////////////////////////////////////////////////////////////
//!	\fn vL2FRAM_force_fram_unformat
//! \brief Clears ALL of FRAM
//!
//!
//!
/////////////////////////////////////////////////////////////////////////////////////
void vL2FRAM_force_fram_unformat(void)
{

	vFRAM_fillFramBlk(0, FRAM_MAX_ADDRESS, 0);

	return;

}/* END: vL2FRAM_force_fram_unformat() */


//////////////////////////////////////////////////////////////////////////////
//! \fn vL2FRAM_stuffTSB
//!
//! \brief stuff a single task state blk with the data.
//!
//! \param ucTSBNum, ucTskIndex, uiTaskID, uiFlagVal, ucState, ulParam1, ulParam2, ulParam3, ulParam4
//////////////////////////////////////////////////////////////////////////////
void vL2FRAM_stuffTSB(uchar ucTSBNum, uchar ucTskIndex, uint uiTaskID, uint uiFlagVal,
		uchar ucState, ulong ulParam1, ulong ulParam2, ulong ulParam3, ulong ulParam4)
{

	vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_TASK_IDX, (ulong) ucTskIndex);
	vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_TASK_ID, (ulong) uiTaskID);
	vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_FLAGS, (ulong) uiFlagVal);
	vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_TASK_STATE, (ulong) ucState);
	vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM1, ulParam1);
	vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM2, ulParam2);
	vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM3, ulParam3);
	vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM4, ulParam4);

	return;

}/* END: vL2FRAM_stuffTSB() */

/*********************  vL2FRAM_showTSBTbl()  ******************************
 *
 *
 *
 *
 ******************************************************************************/

void vL2FRAM_showTSBTbl(void)
{
	uchar ucii;
	uchar ucTSBCount;
	uchar ucTskIndex, ucState;
	uint uiFlagVal;
	ulong ulParam1, ulParam2, ulParam3, ulParam4;
	uint uiTaskID;

	// Unlock FRAM
	vL2FRAM_SetSecurity(TASK_STATE_BLOCKS, FRAM_UNLOCK);

	// Read the number of task state blocks
	ucFRAM_read_B8(FRAM_ST_BLK_COUNT_ADDR, &ucTSBCount);

	/* SHOW THE TABLE HEADER */
	vSERIAL_sout("\r\n\r\n----  FRAM TSB TBL ----\r\n   (size=", 38);
	vSERIAL_UIV8out(ucTSBCount);
	vSERIAL_sout(") (ID:", 6);
	vL2FRAM_showSysID();

	vSERIAL_sout(" ", 1);
	vMODOPT_showCurRole();
	vSERIAL_sout(")\r\n", 3);

	vSERIAL_sout("## NAME   ID   FLAGS  STATE    PARAM 1     PARAM 2     PARAM 3     PARAM 4\r\n", 76);
	for (ucii = 0; ucii < ucTSBCount; ucii++)
	{

		ucTskIndex = (uchar) ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_TASK_IDX);
		uiTaskID = (uint) ulL2FRAM_getTSBEntryVal(ucii,  FRAM_ST_BLK_TASK_ID);
		uiFlagVal = (uint) ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_FLAGS);
		ucState = (uchar)  ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_TASK_STATE);
		ulParam1 = ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_PARAM1);
		ulParam2 = ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_PARAM2);
		ulParam3 = ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_PARAM3);
		ulParam4 = ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_PARAM4);

		vSERIAL_UI8_2char_out(ucii, ' ');
		vSERIAL_colTab(3);
		vTask_showTaskName(ucTskIndex);
		vSERIAL_colTab(10);
		vSERIAL_HB16out(uiTaskID);
		vSERIAL_sout(" ", 1);
		vSERIAL_HB16out(uiFlagVal);
		vSERIAL_sout("    ", 4);
		vSERIAL_HB8out(ucState);
		vSERIAL_sout("   ", 3);
		vSERIAL_UI32out(ulParam1);
		vSERIAL_sout("  ", 2);
		vSERIAL_UI32out(ulParam2);
		vSERIAL_sout("  ", 2);
		vSERIAL_UI32out(ulParam3);
		vSERIAL_sout("  ", 2);
		vSERIAL_UI32out(ulParam4);
		vSERIAL_crlf();

	}/* END: for(ucii) */

	vSERIAL_crlf();

	// Lock FRAM
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return;

}/* END: vL2FRAM_showTSBTbl() */

/*********************  uiL2FRAM_get_version_num()  **************************
 *
 * RET:	version number
 *
 ******************************************************************************/

uint uiL2FRAM_get_version_num(void)
{
	uint uiVersion;

	vL2FRAM_SetSecurity(VERSION, FRAM_UNLOCK);
	ucFRAM_read_B16(FRAM_VER_ADDR, &uiVersion);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return uiVersion;

} // END: uiL2FRAM_get_version_num()


/////////////////////////////////////////////////////////////////////////////
//!
//! \brief Checks the state on shutdown byte
//!
//! If the batteries die in a WiSARD then it dumps all data to the SD card
//! When the batteries are replaced the data resides in the SD card but has
//! not yet reached the database.  In this case we read the most recent block
//! from the SD card and put it in the message queue.
//!
//! \param none
//! \return 1 if shutdown was bad else 0
/////////////////////////////////////////////////////////////////////////////
uchar ucL2FRAM_GetStateOnShutdown(void)
{
	uchar ucShutdownState;

	vL2FRAM_SetSecurity(SHUTDOWN_STATE, FRAM_UNLOCK);
	ucFRAM_read_B8(FRAM_STATE_ON_SHUTDOWN_ADDR, &ucShutdownState);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return ucShutdownState;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! \brief Sets the state on shutdown byte
//!
//! If the batteries die in a WiSARD then it dumps all data to the SD card
//! When the batteries are replaced the data resides in the SD card but has
//! not yet reached the database.  In this case we read the most recent block
//! from the SD card and put it in the message queue.
//!
//! \param ucState
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vL2FRAM_SetStateOnShutdown(uchar ucState)
{
	vL2FRAM_SetSecurity(SHUTDOWN_STATE, FRAM_UNLOCK);
	ucFRAM_write_B8(FRAM_STATE_ON_SHUTDOWN_ADDR, ucState);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! \brief Reads the next free block number from FRAM
//!
//! \param none
//! \return lBlockNum
/////////////////////////////////////////////////////////////////////////////
ulong ulL2FRAM_GetSDCardBlockNum(void)
{
	unsigned long ulBlockNum;

	vL2FRAM_SetSecurity(SD_CARD_PTRS, FRAM_UNLOCK);
	ucFRAM_read_B32(FRAM_SDCARD_BLOCK_NUM_ADDR, &ulBlockNum); //read the ptr
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return (ulBlockNum);

}/* lL2FRAM_GetSDCardBlockNum() */

/////////////////////////////////////////////////////////////////////////////
//!
//! \brief Writes the next free block number to FRAM
//!
//! \param lBlockNum
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vL2FRAM_SetSDCardBlockNum(ulong ulBlockNum)
{
	vL2FRAM_SetSecurity(SD_CARD_PTRS, FRAM_UNLOCK);
	ucFRAM_write_B32(FRAM_SDCARD_BLOCK_NUM_ADDR, ulBlockNum);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return;

}/* vL2FRAM_SetSDCardBlockNum() */

//////////////////////////////////////////////////////////////////////////////
//!
//! \brief Sets the SD card address pointer to the next free location
//!
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vL2FRAM_IncrementSDCardBlockNum(void)
{
	ulong ulAddress;

	// Get the current address
	ulAddress = ulL2FRAM_GetSDCardBlockNum();

	//Increment the block address
	ulAddress += 1;

	if(ulAddress > ulSD_GetCapacity())
		ulAddress = SD_CARD_START_BLOCK;

	// Store the new address
	vL2FRAM_SetSDCardBlockNum(ulAddress);

}

//////////////////////////////////////////////////////////////////////////////
//!
//! \brief Returns the previous SD card address
//!
//!
//! \param none
//! \return ulAddress
//////////////////////////////////////////////////////////////////////////////
ulong ulL2FRAM_GetLastSDCardBlockNum(void)
{
	ulong ulAddress;

	// Get the current address
	ulAddress = ulL2FRAM_GetSDCardBlockNum();

	// If we have used the entire SD card and are starting again from the beginning then the previous block is the last block
	if(ulAddress == SD_CARD_START_BLOCK)
		ulAddress = ulSD_GetCapacity();
	else
		ulAddress -= 1; // Otherwise decrement by one

	return(ulAddress);

}

//////////////////////////////////////////////////////////////////////////////
//!
//! \brief Get the next free location (NFL) in the SD card buffer stored in
//! FRAM.
//!
//! \param none
//! \return Address
//////////////////////////////////////////////////////////////////////////////
uint uiL2FRAM_ReadNFL_SDCardBuff(void)
{
	uint uiNFL;

	vL2FRAM_SetSecurity(SD_CARD_PTRS, FRAM_UNLOCK);
	ucFRAM_read_B16(FRAM_SDCARD_BUFF_END_PTR_ADDR, &uiNFL);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

// If the NFL is out of range then set to the start address
	if(uiNFL > FRAM_SD_CARD_OVRFLO_END_ADDR || uiNFL < FRAM_SD_CARD_BUFF_BEG_ADDR){
		uiNFL = FRAM_SD_CARD_BUFF_BEG_ADDR;
		vSERIAL_sout("SD buff NFL out of range\r\n", 26);
	}

#if 0
	vSERIAL_sout("SD card buffer NFL (read) = ", 28);
	vSERIAL_UI16out(uiNFL);
	vSERIAL_crlf();
#endif
	return (uiNFL);
}

//////////////////////////////////////////////////////////////////////////////
//!
//! \brief Writes the next free location (NFL) of the SD card buffer stored in
//! FRAM.
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vL2FRAM_WriteNFL_SDCardBuff(uint uiAddress)
{

	vL2FRAM_SetSecurity(SD_CARD_PTRS, FRAM_UNLOCK);

	if(uiAddress > FRAM_SD_CARD_OVRFLO_END_ADDR || uiAddress < FRAM_SD_CARD_BUFF_BEG_ADDR){
		uiAddress = FRAM_SD_CARD_BUFF_BEG_ADDR;
		vSERIAL_sout("SD buff NFL out of range\r\n", 26);
	}

	// If there is not enough room in the overflow buffer for the next message
	// then start overwriting the buffer.  This could happen if the SD card is dead.
	if (uiAddress >= (FRAM_SD_CARD_OVRFLO_END_ADDR - MAX_MSG_SIZE)) {
		ucFRAM_write_B16(FRAM_SDCARD_BUFF_END_PTR_ADDR, FRAM_SD_CARD_BUFF_BEG_ADDR);
	}
	else {
		ucFRAM_write_B16(FRAM_SDCARD_BUFF_END_PTR_ADDR, uiAddress);
	}

	vL2FRAM_SetSecurity(0, FRAM_LOCK);

#if 0
	vSERIAL_sout("SD card buffer NFL (write) = ", 29);
	vSERIAL_UI16out(uiAddress);
	vSERIAL_crlf();
#endif
}

////////////////////////////////////////////////////////////////////////////
//!
//! \brief Initializes the pointers used to manage the SD Card
//!
//! In FRAM there is a buffer used to store messages before they are written
//!	to the SD card.  This is done because most SD cards can only be written
//! to in blocks.  So instead of reading a block from the SD card, adding the
//! new message to the block, and then writing the entire block back to the
//! SD card, we create a block here in FRAM and then write it to the SD card.
//!
//! \param none
//! \return none
///////////////////////////////////////////////////////////////////////////
void vL2FRAM_Init_SDCardPtrs(void)
{
	vL2FRAM_SetSecurity(SD_CARD_PTRS, FRAM_UNLOCK);

	// Start the SD Card block count at two to leave room for metadata
	ucFRAM_write_B32(FRAM_SDCARD_BLOCK_NUM_ADDR, SD_CARD_START_BLOCK);

	// Set the SD Card message buffer addresses to the start of the buffer
	ucFRAM_write_B16(FRAM_SDCARD_BUFF_START_PTR_ADDR, FRAM_SD_CARD_BUFF_BEG_ADDR);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	vL2FRAM_WriteNFL_SDCardBuff(FRAM_SD_CARD_BUFF_BEG_ADDR);

}/* END: vL2FRAM_initFramSDCardPtrs() */

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Zeros the SD card buffer in FRAM
//!
//!
//!
///////////////////////////////////////////////////////////////////////////////
void vL2FRAM_CleanSDCardBuff(void)
{
	uint uiIndex;

	vL2FRAM_SetSecurity(SD_CARD_BUFFER, FRAM_UNLOCK);

	for(uiIndex = FRAM_SD_CARD_BUFF_BEG_ADDR; uiIndex < FRAM_SD_CARD_BUFF_END_ADDR; uiIndex++)
		ucFRAM_write_B8(uiIndex, 0);

	vL2FRAM_SetSecurity(0, FRAM_LOCK);

}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Writes a message to the SD card buffer in FRAM.
//!
//! In FRAM there is a buffer used to store messages before they are written
//!	to the SD card.  This is done because most SD cards can only be written
//! to in blocks.  So instead of reading a block from the SD card, adding the
//! new message to the block, and then writing the entire block back to the
//! SD card, we create a block here in FRAM and then write it to the SD card
//! once it is larger than the block size.
//!
//! \param p_ucReport, ucLength
//! \return ucRetVal
////////////////////////////////////////////////////////////////////////////////
uchar ucL2FRAM_WriteReportToSDCardBuff(volatile uchar * p_ucReport, uchar ucLength)
{

	uint uiSDCardBuffNFL; // next free location in the SD card buffer
	uchar ucRetVal;
	uchar ucIndex;

	// Assume that there is enough room in the buffer for two or more messages
	ucRetVal = 0;

	// Get the next free location in the SD card buffer
	uiSDCardBuffNFL = uiL2FRAM_ReadNFL_SDCardBuff();

#if 0
	vSERIAL_sout("SD card buffer address = ", 25);
	vSERIAL_UI16out(uiSDCardBuffNFL);
	vSERIAL_crlf();
#endif

	vL2FRAM_SetSecurity(SD_CARD_BUFFER, FRAM_UNLOCK);

	// Write the report to FRAM
	for (ucIndex = 0; ucIndex < ucLength; ucIndex++)
		ucFRAM_write_B8(uiSDCardBuffNFL++, *p_ucReport++);

	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	// Check the pointer and see if the next report can exceed the SD card block size
	if ((uiSDCardBuffNFL < FRAM_SD_CARD_BUFF_END_ADDR) && ((FRAM_SD_CARD_BUFF_END_ADDR - uiSDCardBuffNFL) < MAX_MSG_SIZE))
	{
		// Set the return value to indicate the buffer is full
		ucRetVal = 1;

		// set the pointer at the overflow so all messages get written there until the block is moved to the SD card
		vL2FRAM_WriteNFL_SDCardBuff(FRAM_SD_CARD_OVRFLO_BEG_ADDR);
	}
	else
		vL2FRAM_WriteNFL_SDCardBuff(uiSDCardBuffNFL); // Set the pointer at the next free location in the buffer

	return ucRetVal;
}


////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Writes the contents in the overflow area to the SD card buffer
//!
//! \param none
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vL2FRAM_SDCardOvrflowToBuff(void)
{
	uint uiSDCardBuffNFL; // next free location in the SD card buffer
	uint uiSrcAddress;
	uint uiDestAddress;
	uchar ucData;

	// Get the next free location in the SD card buffer
	uiSDCardBuffNFL = uiL2FRAM_ReadNFL_SDCardBuff();

	// If the next free location is in the overflow area then rewrite the contents to the buffer
	if(uiSDCardBuffNFL >= FRAM_SD_CARD_BUFF_END_ADDR)
	{

		// Zero all entries in the buffer
		vL2FRAM_CleanSDCardBuff();

		// Start the destination at the beginning of the buffer
		uiDestAddress = FRAM_SD_CARD_BUFF_BEG_ADDR;

		vL2FRAM_SetSecurity(SD_CARD_BUFFER, FRAM_UNLOCK);

		// Starting at the overflow area, read out each byte and write it to the start of the buffer
		for (uiSrcAddress = FRAM_SD_CARD_OVRFLO_BEG_ADDR; uiSrcAddress < uiSDCardBuffNFL; uiSrcAddress++){
			ucFRAM_read_B8(uiSrcAddress, &ucData);
			ucFRAM_write_B8(uiDestAddress++, ucData);
		}

		vL2FRAM_SetSecurity(0, FRAM_LOCK);

		// Set the next free location
		vL2FRAM_WriteNFL_SDCardBuff(uiDestAddress);
	}
}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Reads the SD card buffer into addresses given by the pointer
//!
//!
//! \param p_ucBlock
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vL2FRAM_ReadSDCardBuffer(uchar * p_ucBlock)
{
	uint uiIndex;
	uint uiSrcAddr;

	vL2FRAM_SetSecurity(SD_CARD_BUFFER, FRAM_UNLOCK);

	uiSrcAddr = FRAM_SD_CARD_BUFF_BEG_ADDR;
	for (uiIndex = 0; uiIndex < FRAM_SD_CARD_BUFF_SIZE; uiIndex++)
		ucFRAM_read_B8(uiSrcAddr++, p_ucBlock++);

	vL2FRAM_SetSecurity(0, FRAM_LOCK);
}

/******************** vL2FRAM_stuffSavedTime()  ******************************
 *
 *
 *
 ******************************************************************************/

void vL2FRAM_stuffSavedTime(ulong ulSavedTimeVal)
{

	vL2FRAM_SetSecurity(TIME, FRAM_UNLOCK);
	ucFRAM_write_B32(FRAM_TIME_SAVE_AREA_ADDR, ulSavedTimeVal);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return;

}/* END: vL2FRAM_stuffSavedTime() */

/******************** ulL2FRAM_getSavedTime()  *******************************
 *
 *
 *
 ******************************************************************************/

ulong ulL2FRAM_getSavedTime(void)
{
	ulong ulTime;

	vL2FRAM_SetSecurity(TIME, FRAM_UNLOCK);
	ucFRAM_read_B32(FRAM_TIME_SAVE_AREA_ADDR, &ulTime);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return ulTime;

}/* END: ulL2FRAM_getSavedTime() */

/******************** vL2FRAM_writeRebootCount()  ******************************
 *
 *
 *
 ******************************************************************************/

void vL2FRAM_writeRebootCount(uint uiRebootCountVal)
{

	vL2FRAM_SetSecurity(REBOOT_COUNT, FRAM_UNLOCK);
	ucFRAM_write_B16(FRAM_REBOOT_COUNT_ADDR, uiRebootCountVal);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return;

}/* END: vL2FRAM_writeRebootCount() */

/******************** uiL2FRAM_getRebootCount()  *******************************
 *
 *
 *
 ******************************************************************************/

uint uiL2FRAM_getRebootCount(void)
{
	uint uiRebootCount;

	vL2FRAM_SetSecurity(REBOOT_COUNT, FRAM_UNLOCK);
	ucFRAM_read_B16(FRAM_REBOOT_COUNT_ADDR, &uiRebootCount);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return uiRebootCount;

}/* END: uiL2FRAM_getRebootCount() */

/******************** vL2FRAM_incRebootCount()  ******************************
 *
 *
 *
 ******************************************************************************/

void vL2FRAM_incRebootCount(void)
{
	uint uiRebootCountVal;

	vL2FRAM_SetSecurity(REBOOT_COUNT, FRAM_UNLOCK);
	ucFRAM_read_B16(FRAM_REBOOT_COUNT_ADDR, &uiRebootCountVal);
	uiRebootCountVal++;
	ucFRAM_write_B16(FRAM_REBOOT_COUNT_ADDR, uiRebootCountVal);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return;

}/* END: vL2FRAM_incRebootCount() */

/************************  ucL2FRAM_isHub()  *********************************
 *
 * Returns == 1 if yes HUB
 * Returns == 0 if not HUB
 *
 ******************************************************************************/

uchar ucL2FRAM_isHub(void)
{
	uchar ucRoleBits;

	ucRoleBits = ucMODOPT_getCurRole();

	/* IF YOU DO RECEIVE & YOU DO NOT SEND -- YOU ARE A HUB */
	if ((ucRoleBits & ROLE_RECEIVE_BIT) && ((ucRoleBits & ROLE_SEND_BIT) == 0))
	{
		return (1);
	}

	return (0);

}/* END: ucL2FRAM_isHub() */

/************************  ucL2FRAM_isSender()  ******************************
 *
 * RET: 1 if yes SENDER
 *      0 if not SENDER
 *
 ******************************************************************************/
uchar ucL2FRAM_isSender(void)
{

	return (ucMODOPT_getCurRole() & ROLE_SEND_BIT);

}/* END: ucL2FRAM_isSender() */

/************************  ucL2FRAM_isSampler()  ******************************
 *
 * RET: 1 if yes SAMPLER
 *      0 if not SAMPLER
 *
 ******************************************************************************/
uchar ucL2FRAM_isSampler(void)
{

	return (ucMODOPT_getCurRole() & ROLE_SAMPLE_BIT);

}/* END: ucL2FRAM_isSampler() */

/************************  ucL2FRAM_isReceiver()  ******************************
 *
 * RET: 1 if yes Receiver
 *      0 if not Receiver
 *
 ******************************************************************************/
uchar ucL2FRAM_isReceiver(void)
{

	return (ucMODOPT_getCurRole() & ROLE_RECEIVE_BIT);

}/* END: ucL2FRAM_isReceiver() */


/////////////////////////////////////////////////////////////////////////////////
//!	\fn ucL2FRAM_GetOptionByte
//! \brief Reads an option byte from FRAM
//!
//!
/////////////////////////////////////////////////////////////////////////////////
uchar ucL2FRAM_GetOptionByte(uchar ucOptionByteIdx, uchar *ucByteVal){

	uchar ucRetVal;

	// Assume success
	ucRetVal = 0;

	vL2FRAM_SetSecurity(OPTION_BYTES, FRAM_UNLOCK);

	if(ucFRAM_read_B8((uint)FRAM_OPTION_BYTE_0_ADDR + ucOptionByteIdx, ucByteVal) != 0)
		ucRetVal = 1;

	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return ucRetVal;
}

/////////////////////////////////////////////////////////////////////////////////
//!	\fn ucL2FRAM_SetOptionByte
//! \brief Writes an option byte from FRAM
//!
//!
/////////////////////////////////////////////////////////////////////////////////
uchar ucL2FRAM_SetOptionByte(uchar ucOptionByteIdx, uchar ucByteVal){

	uchar ucRetVal;

	// Assume success
	ucRetVal = 0;

	vL2FRAM_SetSecurity(OPTION_BYTES, FRAM_UNLOCK);

	if(ucFRAM_write_B8((uint)FRAM_OPTION_BYTE_0_ADDR + ucOptionByteIdx, ucByteVal) != 0)
		ucRetVal = 1;

	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return ucRetVal;
}

/**********************  uiL2FRAM_getSnumLo16AsUint()  ************************************
 *
 * Returns the two low bytes of the serial number to the caller
 *
 *
 ******************************************************************************/

uint uiL2FRAM_getSnumLo16AsUint(void)
{
	uint uiSN;

	vL2FRAM_SetSecurity(NETWORK_ID, FRAM_UNLOCK);
	ucFRAM_read_B16(FRAM_USER_ID_ADDR, &uiSN);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return uiSN;

}/* END: uiL2FRAM_getSnumLo16AsUint() */

/***********************  vL2FRAM_copySnumLo16ToBytes()  *************************
 *
 ******************************************************************************/

void vL2FRAM_copySnumLo16ToBytes(uchar *ucpToPtr)
{
	uint uiSN;

	uiSN = uiL2FRAM_getSnumLo16AsUint();

	*ucpToPtr = (uchar) (uiSN >> 8);
	ucpToPtr++;
	*ucpToPtr = (uchar) uiSN;

	return;

}/* END vL2FRAM_copySnumLo16ToBytes() */

/**********************  vL2FRAM_setSysID()  ********************************
 *
 * Set the system ID
 *
 ******************************************************************************/
void vL2FRAM_setSysID(uint uiSysID)
{

	vL2FRAM_SetSecurity(NETWORK_ID, FRAM_UNLOCK);
	ucFRAM_write_B16(FRAM_USER_ID_ADDR, uiSysID);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return;

}/* END: vL2FRAM_setSysID() */

/**********************  vL2FRAM_showSysID()  ********************************
 *
 * Show the system ID
 *
 ******************************************************************************/
void vL2FRAM_showSysID(void)
{
	uint uiSysID;

	uiSysID = uiL2FRAM_getSnumLo16AsUint();
	vSERIAL_UI16out(uiSysID);

	return;

}/* END: vL2FRAM_showSysID() */

//////////////////////////////////////////////////////////////////////////////
//! \fn ucL2FRAM_getTSBTblCount
//! \brief Return state block table entry count
//!
///////////////////////////////////////////////////////////////////////////////
uchar ucL2FRAM_getTSBTblCount(void)
{
	uchar ucTSBCount;

	vL2FRAM_SetSecurity(TASK_STATE_BLOCKS, FRAM_UNLOCK);

	// Read the value from FRAM
	ucFRAM_read_B8(FRAM_ST_BLK_COUNT_ADDR, &ucTSBCount);

	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	// Keep the count within range
	if (ucTSBCount > MAXNUMTASKS)
		ucTSBCount = MAXNUMTASKS;

	return ucTSBCount;
}/* END: ucL2FRAM_getTSBTblCount() */

/**********************  vL2FRAM_setTSBTblCount()  **************************
 *
 * Return Start Blk Table Entry Count
 *
 ******************************************************************************/
void vL2FRAM_setTSBTblCount(uchar ucBlkCount)
{
	vL2FRAM_SetSecurity(TASK_STATE_BLOCKS, FRAM_UNLOCK);
	ucFRAM_write_B8(FRAM_ST_BLK_COUNT_ADDR, ucBlkCount);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);
	return;

}/* END: ucL2FRAM_setTSBTblCount() */

//////////////////////////////////////////////////////////////////////////////
//! \fn ucL2FRAM_getNextFreeTSB
//!
//! \brief Gets the location of the next available task state-block
//!
//!	The state blocks are added sequentially so there are no empty locations in
//! the list.  If a state block is deleted then the deleted location is filled
//! with the last state block in the list.
//!
//! \return ucTSBCount
//////////////////////////////////////////////////////////////////////////////
static uchar ucL2FRAM_getNextFreeTSB( uchar *ucTSBNum)
{
	uchar ucTSBCount;

	// Get the state-block count
	ucTSBCount = ucL2FRAM_getTSBTblCount();
	*ucTSBNum = ucTSBCount;

	if (ucTSBCount >= FRAM_MAX_TSB_COUNT)
		return 1; // No state-blocks available
	else
		vL2FRAM_setTSBTblCount(ucTSBCount + 1); // Write the next one

	return 0;

}/* END: ucL2FRAM_getNextFreeTSB() */


////////////////////////////////////////////////////////////////////////////////
//! \fn ulL2FRAM_getTSBEntryVal
//! \brief Return TSB entry value
//!
//! \param ucTSBNum, ucTSBEntryIdx
//! \return ulRetVal
////////////////////////////////////////////////////////////////////////////////
ulong ulL2FRAM_getTSBEntryVal( //RET: TSB Entry Val
    uchar ucTSBNum, //blk number
    uchar ucTSBEntryIdx //index into the blk
    )
{
	uint uiOffset;
	uint uiAddr;
	ulong ulRetVal;
	uint uiRetVal;
	uchar ucRetVal;

	uiOffset = ((uint) ucTSBNum) * FRAM_ST_BLK_SIZE;
	uiAddr = FRAM_ST_BLK_0_ADDR + uiOffset + ucTSBEntryIdx;

	vL2FRAM_SetSecurity(TASK_STATE_BLOCKS, FRAM_UNLOCK);

	ulRetVal = 0UL;
	switch (ucTSBEntryIdx)
	{
		case FRAM_ST_BLK_TASK_IDX: //byte
		case FRAM_ST_BLK_TASK_STATE:
			ucFRAM_read_B8(uiAddr, &ucRetVal);
			ulRetVal = (ulong)ucRetVal;
		break;

		case FRAM_ST_BLK_TASK_ID: // Integers
		case FRAM_ST_BLK_FLAGS:
			ucFRAM_read_B16(uiAddr, &uiRetVal);
			ulRetVal = (ulong)uiRetVal;
		break;

		case FRAM_ST_BLK_PARAM1: // Longs
		case FRAM_ST_BLK_PARAM2:
		case FRAM_ST_BLK_PARAM3:
		case FRAM_ST_BLK_PARAM4:
			ucFRAM_read_B32(uiAddr, &ulRetVal);
		break;

		default:
			vSERIAL_sout("L2FRM:BadTSBIdx\r\n", 17);
			ulRetVal = 0;
		break;

	}/* END: switch() */

	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return (ulRetVal);

}/* END: ulL2FRAM_getTSBEntryVal() */

////////////////////////////////////////////////////////////////////////////////
//! \fn vL2FRAM_putTSBEntryVal
//!
//! \brief Return state-block entry value
//!
//! \param ucTSBNum, ucTSBEntryIdx, ulVal
////////////////////////////////////////////////////////////////////////////////
void vL2FRAM_putTSBEntryVal( //Stuff the Value int the TSB
    uchar ucTSBNum, //blk number
    uchar ucTSBEntryIdx, //index into the blk
    ulong ulVal //value to put
    )
{
	uint uiOffset;
	uint uiAddr;

	uiOffset = ((uint) ucTSBNum) * FRAM_ST_BLK_SIZE;
	uiAddr = FRAM_ST_BLK_0_ADDR + uiOffset + ucTSBEntryIdx;
	vL2FRAM_SetSecurity(TASK_STATE_BLOCKS, FRAM_UNLOCK);

	switch (ucTSBEntryIdx)
	{
		case FRAM_ST_BLK_TASK_IDX: //byte
		case FRAM_ST_BLK_TASK_STATE:
			ucFRAM_write_B8(uiAddr, (uchar) ulVal);
		break;

		case FRAM_ST_BLK_TASK_ID: // Integers
		case FRAM_ST_BLK_FLAGS:
			ucFRAM_write_B16(uiAddr, (uint) ulVal);
		break;

		case FRAM_ST_BLK_PARAM1: // Longs
		case FRAM_ST_BLK_PARAM2:
		case FRAM_ST_BLK_PARAM3:
		case FRAM_ST_BLK_PARAM4:
			ucFRAM_write_B32(uiAddr, ulVal);
		break;

#if 0
			vSERIAL_sout("SenseActBlk=", 12);
			vSERIAL_UIV8out(ucTSBNum);
			vSERIAL_sout(",  Idx=", 7);
			vSERIAL_UIV8out(ucTSBEntryIdx);
			vSERIAL_sout(", Val=", 6);
			vSERIAL_HB32out(ulVal);
			vSERIAL_crlf();
#endif

		default:
			vSERIAL_sout("L2FRM:WriteBdTSBAddr= ", 22);
			vSERIAL_UIV16out(uiAddr);
			vSERIAL_crlf();
			vSERIAL_sout("Blk=", 4);
			vSERIAL_UIV8out(ucTSBNum);
			vSERIAL_sout(",  Idx=", 7);
			vSERIAL_UIV8out(ucTSBEntryIdx);
			vSERIAL_crlf();
		break;

	}/* END: switch() */

	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return;

}/* END: vL2FRAM_putTSBEntryVal() */

///////////////////////////////////////////////////////////////////////////////
//! \fn ucL2FRAM_findTSBTask
//!
//! \brief Searches for a task state block using the task ID
//!
//! \param uiTaskID
//! \return ucii or 255 (error)
///////////////////////////////////////////////////////////////////////////////
uchar ucL2FRAM_findTSB( //Ret: TSB num,  255 if none
		uint uiTaskID)
{
	uchar ucii;
	uchar ucTSBMax;
	uint uiTempID;

	// Get the number of TSBs in memory
	ucTSBMax = ucL2FRAM_getTSBTblCount();

	for (ucii = 0; ucii < ucTSBMax; ucii++) {

		// Read the task ID for this state block
		uiTempID = (uint) ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_TASK_ID);

		// If they match then return the state block index
		if (uiTempID == uiTaskID)
			return (ucii);

	}/*END: for() */

	return (255);

}/* END: ucL2FRAM_findTSB() */

///////////////////////////////////////////////////////////////////////////////
//! \fn ucL2FRAM_findTSBTask
//!
//! \brief Searches for a task state block using the serial number
//!
//! Must be sure that the task is a comm. task before calling this function
//!
//! \param uiSerialNum
//! \return ucii or 255 (error)
///////////////////////////////////////////////////////////////////////////////
uchar ucL2FRAM_findTSB_SN( //Ret: TSB num,  255 if none
		uint uiSerialNum)
{
	uchar ucii;
	uchar ucTSBMax;
	uint uiTaskID;
	uint uiTempSN;

	// Get the number of TSBs in memory
	ucTSBMax = ucL2FRAM_getTSBTblCount();

	for (ucii = 0; ucii < ucTSBMax; ucii++) {

		// Read the task ID and make sure it is an RF comm. task
		uiTaskID = (uint) ulL2FRAM_getTSBEntryVal(ucii,FRAM_ST_BLK_TASK_ID);
		if(uiTaskID == TASK_ID_SOM || uiTaskID == TASK_ID_ROM){

		// Read the serial number
		uiTempSN = (uint) ulL2FRAM_getTSBEntryVal(ucii, FRAM_ST_BLK_PARAM1);

		// If they match then return the state block index
			if (uiTempSN == uiSerialNum) {
				return (ucii);
			}
		}// END: if()

	}/*END: for() */

	return (255);

}/* END: ucL2FRAM_findTSB() */


//////////////////////////////////////////////////////////////////////////////
//! \fn ucL2FRAM_addTSB
//!
//! \brief Adds a task state control block to the list
//!
//!	\param ucTskIndex
//!	\return
//////////////////////////////////////////////////////////////////////////////
signed char cL2FRAM_addTSB( //RET: TSB num of new entry
		uchar ucTskIndex //task number to search for
		)
{
	uchar ucNewTSBNum;
	ulong ulParam1, ulParam2, ulParam3, ulParam4;
	ulong ulTaskID, ulState, ulFlags;

	/* GET A NEW TSB NUMBER */
	if(ucL2FRAM_getNextFreeTSB(&ucNewTSBNum) != 0)
		return -1;

	// If there is an error reading the task ID then exit with an error
	if (ucTask_GetField(ucTskIndex, TSK_ID, &ulTaskID) != TASKMNGR_OK) //Task ID
		return -1;

	ucTask_GetField(ucTskIndex, TSK_FLAGS, &ulFlags); //Start flag value
	ucTask_GetField(ucTskIndex, TSK_STATE, &ulState); //Start flag value

	// Get the values in the parameter
	ucTask_GetParam(ucTskIndex, 0, &ulParam1); // Parameter 1
	ucTask_GetParam(ucTskIndex, 1, &ulParam2); // Parameter 2
	ucTask_GetParam(ucTskIndex, 2, &ulParam3); // Parameter 3
	ucTask_GetParam(ucTskIndex, 3, &ulParam4); // Parameter 4

	/* ADD THIS ACTION TO THE LIST OF START BLKS */
	vL2FRAM_stuffTSB(ucNewTSBNum, //start blk index
			ucTskIndex, //Task index
			(uint)ulTaskID, 	//Task ID
			(uint)ulFlags, 	//Start flag value
			(uchar)ulState, 	// Task state
			ulParam1, 	//Parameter 1
			ulParam2, 	//Parameter 2
			ulParam3, 	//Parameter 3
			ulParam4 	//Parameter 4
			);


#if 0
	vL2FRAM_showTSBTbl();
#endif

	return (ucNewTSBNum);

}/* END: ucL2FRAm_addRuntimeTSB() */

/**********************  vL2FRAM_deleteTSB()  ******************************
 *
 * This routine deletes a task state block (TSB) from the List.
 *
 * NOTE: There is not an explicit way to detect a deleted (TSB).  A deleted
 *		St-Blk looks exactly like a turned off sleep blk( all zros), So the
 *		method used	is to look for a zero action (SLEEP) blk that is not in
 *		the block 0 position.
 *
 ******************************************************************************/
void vL2FRAM_deleteTSB(uchar ucTSBNum //Task State Block Num
    )
{

	uchar ucLastTSB;
	uchar ucTaskIndex, ucState;
	uint uiTaskID, uiFlags;
	ulong ulParam1, ulParam2, ulParam3, ulParam4;

	// Get the location of the last TSB in the list
	ucLastTSB = ucL2FRAM_getTSBTblCount();
	if(ucLastTSB > 0)
		ucLastTSB--;

	// If we are not deleting the last TSB then get a local copy of the last TSB and overwright the TSB to be deleted
	if (ucLastTSB != ucTSBNum) {

		// Get a local copy of the last TSB in the list
		ucTaskIndex = (uchar) ulL2FRAM_getTSBEntryVal(ucLastTSB, FRAM_ST_BLK_TASK_IDX);
		uiTaskID = (uint) ulL2FRAM_getTSBEntryVal(ucLastTSB, FRAM_ST_BLK_TASK_ID);
		uiFlags = (uint) ulL2FRAM_getTSBEntryVal(ucLastTSB, FRAM_ST_BLK_FLAGS);
		ucState = (uchar) ulL2FRAM_getTSBEntryVal(ucLastTSB, FRAM_ST_BLK_TASK_STATE);
		ulParam1 = ulL2FRAM_getTSBEntryVal(ucLastTSB, FRAM_ST_BLK_PARAM1);
		ulParam2 = ulL2FRAM_getTSBEntryVal(ucLastTSB, FRAM_ST_BLK_PARAM2);
		ulParam3 = ulL2FRAM_getTSBEntryVal(ucLastTSB, FRAM_ST_BLK_PARAM3);
		ulParam4 = ulL2FRAM_getTSBEntryVal(ucLastTSB, FRAM_ST_BLK_PARAM4);

		// Overwrite the task state block
		vL2FRAM_stuffTSB(ucTSBNum, //St blk Idx
				ucTaskIndex, //Task index
				uiTaskID, 	//Task ID
				uiFlags, 	//Start flag value
				ucState, 	// Task state
				ulParam1, 	//Parameter 1
				ulParam2, 	//Parameter 2
				ulParam3, 	//Parameter 3
				ulParam4 	//Parameter 4
				);
	}

	// Clear the last task state block.
	vL2FRAM_stuffTSB(ucLastTSB, //St blk Idx
			0,  // Task index
			0, 	// Task ID
			0, 	// Start flag value
			0, 	// Task state
			0, 	// Parameter 1
			0, 	// Parameter 2
			0, 	// Parameter 3
			0 	// Parameter 4
			);

	// Set the new task state block count
	vL2FRAM_setTSBTblCount(ucLastTSB);

#if 0
	vL2FRAM_showTSBTbl();
#endif

	return;

}/* END: vL2FRAM_deleteTSB() */

/**********************  vL2FRAM_putYtriggerVal()  ***************************
 *
 * stuff a trigger value
 *
 ******************************************************************************/
void vL2FRAM_putYtriggerVal( //Stuff Val into FRAM trigger Area
    uchar ucSensorNum, uint uiYtriggerVal)
{
	uint uiFramAddr;

	uiFramAddr = FRAM_Y_TRIG_AREA_BEG_ADDR + (2 * ucSensorNum);

	vL2FRAM_SetSecurity(Y_TRIGGER, FRAM_UNLOCK);
	ucFRAM_write_B16(uiFramAddr, uiYtriggerVal);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);
	return;

}/* END: vL2FRAM_putYtriggerVal() */

/**********************  uiL2FRAM_getYtriggerVal()  ***************************
 *
 * return a trigger value
 *
 ******************************************************************************/
uint uiL2FRAM_getYtriggerVal( //ret trigger Val from FRAM
    uchar ucSensorNum)
{
	uint uiFramAddr;
	uint uiTriggerVal;

	uiFramAddr = FRAM_Y_TRIG_AREA_BEG_ADDR + (2 * ucSensorNum);

	vL2FRAM_SetSecurity(Y_TRIGGER, FRAM_UNLOCK);
	ucFRAM_read_B16(uiFramAddr, &uiTriggerVal);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return (uiTriggerVal);

}/* END: uiL2FRAM_getYtriggerVal() */

/**********************  vL2FRAM_showYtriggerTbl()  ***************************
 *
 * show the trigger table
 *
 ******************************************************************************/
void vL2FRAM_showYtriggerTbl(void)
{
	uchar ucii;
	uchar ucjj;

	vSERIAL_sout("FramYTrigTbl:\r\n", 15);
	vSERIAL_sout("ID SNSACT  TRIG    ID SNSACT  TRIG    ID SNSACT  TRIG    ID SNSACT  TRIG\r\n", 74);
	for (ucii = 0, ucjj = 0; ucii < TASKPARTITION; ucii++) //ucii=sensor#  ucjj=format count
	{
		if ((ucjj % 4) == 0)
			vSERIAL_crlf();
		vSERIAL_HB8out(ucii);
		vSERIAL_bout(' ');
		vTask_showTaskName(ucii);
		vSERIAL_bout(' ');
		vSERIAL_UI16out(uiL2FRAM_getYtriggerVal(ucii));
		if (((ucjj + 1) % 4) != 0)
			vSERIAL_sout("    ", 4);
		ucjj++;

	}/* END: for(ucii) */

	vSERIAL_sout("\r\n\n", 3);

	return;

}/* END: vL2FRAM_showYtriggerTbl() */

/////////////////////////////////////////////////////////////////////////////////
//! \fn vL2FRAM_SetReportingPriority
//!	\brief Sets the reporting priority of the WiSARD in FRAM
//! Reporting priority is used to determine what data elements will be recorded and
//! transmitted.  Many diagnostic reports can be shut off to reduce message load
//! and save energy.  Crisis level reports are always stored in flash of the MCU.
/////////////////////////////////////////////////////////////////////////////////
void vL2FRAM_SetReportingPriority(uchar ucReportingPriority){

	vL2FRAM_SetSecurity(REPORTINGPRIORITY, FRAM_UNLOCK);
	ucFRAM_write_B8(FRAM_RPT_PRTY_ADDR, ucReportingPriority);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

}

/////////////////////////////////////////////////////////////////////////////////
//! \fn ucL2FRAM_GetReportingPriority
//!	\brief Gets the reporting priority of the WiSARD from FRAM
//! Reporting priority is used to determine what data elements will be recorded and
//! transmitted.  Many diagnostic reports can be shut off to reduce message load
//! and save energy.  Crisis level reports are always stored in flash of the MCU.
/////////////////////////////////////////////////////////////////////////////////
uchar ucL2FRAM_GetReportingPriority(void){

	uchar ucReportingPriority = 0xFF;

	vL2FRAM_SetSecurity(REPORTINGPRIORITY, FRAM_UNLOCK);
	ucFRAM_read_B8(FRAM_RPT_PRTY_ADDR, &ucReportingPriority);
	vL2FRAM_SetSecurity(0, FRAM_LOCK);

	return ucReportingPriority;
}


void vL2FRAM_test(void){
	uint ucII, ucJJ;
	uchar ucaReport[50], ucB;

	for(ucII = 0; ucII < 50; ucII++){
		ucaReport[ucII] = 0;
	}

	ucB = 0;

	for (ucII = 0; ucII < 30; ucII++) {

		for (ucJJ = 0; ucJJ < 50; ucJJ++)
			ucaReport[ucJJ] = ucB;

		ucB++;

		ucL2FRAM_WriteReportToSDCardBuff(ucaReport, 33);
		vFRAM_show_fram(FRAM_SD_CARD_BUFF_BEG_ADDR, 800);
	}

}
/*-------------------------------  MODULE END  ------------------------------*/
