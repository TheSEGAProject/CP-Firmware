
/**************************  FRAM.C  *****************************************
*
* Routines to handle low level FRAM control.
*
*
* V1.00 10/14/2003 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include <msp430.h>		//processor reg description */
#include "std.h"			//standard definitions
#include "config.h" 		//system configuration description file
#include "serial.h" 		//serial IO port stuff
#include "fram.h"			//FRAM control routines
#include "comm.h"			//msg helper routines
#include "mem_mod.h"
#include "report.h"		// for reporting errors
#include "task.h"			// Obtaining task IDs
#include "main.h"			// Obtaining CP firmware version

#define SUCCESS 					0
#define ACCESS_VIOLATION	1


extern volatile uchar ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];
extern uchar g_ucaCurrentTskIndex;
uchar ucLastTaskIndextoWriteToFRAM = 0xFF;

/*************************  DECLARATIONS  ***********************************/

static void vFRAM_send_WE_cmd(
		void
		);

static uchar ucFRAM_bin(
		uint uiAddr,
		uchar *ucData
		);

static uchar ucFRAM_bout(
		uint uiAddr,
		uchar ucData
		);

/*************************  GLOBAL VARIABLES  ********************************/

//! \var g_uiUnlockStart
//! \brief Start address of unlocked memory
unsigned int g_uiUnlockStart = 0xFFFF;
//! \var g_uiUnlockEnd
//! \brief End address of unlocked memory
unsigned int g_uiUnlockEnd = 0xFFFF;


/******************  vFRAM_init()  *******************************************
*
* Routine to initialize the fram SPI for use
*
*******************************************************************************/
void vFRAM_init(
		void
		)
	{
		

	/* SETUP THE SPI REGS */
	vSPI_Init(SPI_MODE_0, RATE_1);
	vSD_PowerOn(); // SD Card bug fix
	P3REN &= ~BIT2;
	P3OUT &= ~BIT2;
	return;

	}/* END: vFRAM_init() */

/******************  vFRAM_quit()  *******************************************
*
* Routine to shut off the Fram
*
******************************************************************************/

void vFRAM_quit(
		void
		)
	{

	/* TURN OFF THE SPI REGS */
	vSPI_Quit();
	vSD_PowerOff(); // SD Card bug fix

	/* SELECT THE FRAM CHIP */
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;					//de-select FRAM

	return;

	}/* END: vFRAM_quit() */

////////////////////////////////////////////////////////////////////////
//! \fn vFRAM_Security
//!
//! \brief Sets the accessible range of memory.  If all memory is locked
//! then both variables = 0xFFFF
///////////////////////////////////////////////////////////////////////
void vFRAM_Security(unsigned int uiStartAddr, unsigned int uiEndAddress)
{
	g_uiUnlockStart = uiStartAddr;
	g_uiUnlockEnd = uiEndAddress;
}

///////////////////////////////////////////////////////////////////////
//! \fn vFRAM_ReportAccessViolation
//!	\brief Reports FRAM access violations.  The idea is to catch the
//! violation as well as the task responsible for the violation.
//!
///////////////////////////////////////////////////////////////////////
void vFRAM_ReportAccessViolation(void){

	uchar ucMsgIndex;

	// set to invalid
	ulong ulTaskID = 0xFFFF;

	// Get the ID of the currently running task
	ucTask_GetField(g_ucaCurrentTskIndex, TSK_ID, &ulTaskID);

	// Build the report data element header
	vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
	ucMsgIndex = 8;

	// Load the rest of the DE
	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_FRAM_ACCESS_VIOLATION;
	ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) (ulTaskID >> 8); // TaskID Hi
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) ulTaskID; // TaskID Lo

	// Store DE
	vReport_LogDataElement(RPT_PRTY_FRAM_ACCESS_VIOLATION);
}

///////////////////////////////////////////////////////////////////////
//! \fn vFRAM_ReportWriteFail
//!	\brief Reports FRAM write failures.
///////////////////////////////////////////////////////////////////////
void vFRAM_ReportWriteFail(void){

	uchar ucMsgIndex;

	// set to invalid
	ulong ulTaskID = 0xFFFF;

	// Get the ID of the currently running task
	ucTask_GetField(g_ucaCurrentTskIndex, TSK_ID, &ulTaskID);

	// Build the report data element header
	vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
	ucMsgIndex = 8;

	// Load the rest of the DE
	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_FRAM_WRITE_FAIL;
	ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) (ulTaskID >> 8); // TaskID Hi
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) ulTaskID; // TaskID Lo

	// Store DE
	vReport_LogDataElement(RPT_PRTY_FRAM_WRITE_FAIL);
}

/****************** ucFRAM_read_sts_reg()  ***********************************
*
* Return the status byte from the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

uchar ucFRAM_read_sts_reg(
		void
		)
	{
	uchar ucData;

	FRAM_SEL_OUT_PORT &= ~FRAM_SEL_BIT;	//select the chip

	vSPI_bout(FRAM_READ_STS_REG_CMD); 	//send the read ststus cmd
	ucData = ucSPI_bin();				//get the data
	
	/* DROP FRAM CHIP SELECT FOR NEXT CMD */
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;						//deselect chip

	return(ucData);

	}/* END: ucFRAM_read_sts_reg() */





/****************** vFRAM_send_WE_cmd()  ***********************************
*
* Return the status byte from the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

static void vFRAM_send_WE_cmd(
		void
		)
	{

	FRAM_SEL_OUT_PORT &= ~FRAM_SEL_BIT;					//select the chip
	vSPI_bout(FRAM_WRITE_ENA_CMD);		//send write enable cmd
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;						//deselect chip

	return;

	}/* END: vFRAM_send_WE_cmd() */





/****************** ucFRAM_bin()  **************************************
*
* Byte input from fram
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

static uchar ucFRAM_bin(
		uint uiAddr,
		uchar *ucData
		)
	{
	uchar ucAddrHI;
	uchar ucAddrLO;
	
	if(uiAddr < g_uiUnlockStart || uiAddr > g_uiUnlockEnd)
		return ACCESS_VIOLATION;

	ucAddrHI = (uchar)((uiAddr & 0x1FFF)>>8);
	ucAddrLO = (uchar)(uiAddr & 0xFF);
	
	FRAM_SEL_OUT_PORT &= ~FRAM_SEL_BIT;							//select the chip

	vSPI_bout(FRAM_READ_DATA_CMD);			//CMD

	vSPI_bout(ucAddrHI);						//HI addr
	vSPI_bout(ucAddrLO);						//LO addr
	*ucData = ucSPI_bin();						//Read the data
	
	/* DROP FRAM CHIP SELECT FOR NEXT CMD */
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;								//deselect chip

	// return success
	return SUCCESS;

	}/* END: ucFRAM_bin() */


/****************** ucFRAM_bout()  ********************************************
*
* byte output to FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

static uchar ucFRAM_bout(
		uint uiAddr,
		uchar ucData
		)
	{
	uchar ucAddrHI;
	uchar ucAddrLO;

	if(uiAddr == 16 && ucData < 0x06)
			__no_operation();

	if(uiAddr == 16)
			ucLastTaskIndextoWriteToFRAM = g_ucaCurrentTskIndex;


	if(uiAddr < g_uiUnlockStart || uiAddr > g_uiUnlockEnd)
		return ACCESS_VIOLATION;

	ucAddrHI = (uchar)((uiAddr & 0x1FFF)>>8);
	ucAddrLO = (uchar)(uiAddr & 0xFF);

	vFRAM_send_WE_cmd();				//turn on write enable

	FRAM_SEL_OUT_PORT &= ~FRAM_SEL_BIT;					//select the chip

	vSPI_bout(FRAM_WRITE_DATA_CMD);		//CMD with zro HI addr bit
	vSPI_bout(ucAddrHI);
	vSPI_bout(ucAddrLO);					//LO addr
	vSPI_bout(ucData);						//data value
	/* DROP FRAM CHIP SELECT FOR NEXT CMD */
	FRAM_SEL_OUT_PORT |= FRAM_SEL_BIT;								//deselect chip

	#if 0
	vSERIAL_sout("FramWrit: Addr=", 15);
	vSERIAL_HB8out(ucAddrHI);
	vSERIAL_HB8out(ucAddrLO);
	vSERIAL_sout("  Val=", 6);
	vSERIAL_HB8out(ucData);
	vSERIAL_crlf();
	#endif

	return SUCCESS;

	}/* END: ucFRAM_bout() */


/****************** ucFRAM_read_B8()  **************************************
*
* Return the data at a specified address
*
* NOTE: This routine turn on the SPI bus on entry and off on exit
*
******************************************************************************/

uchar ucFRAM_read_B8(
		uint uiAddr,
		uchar *ucData
		)
	{
	uchar ucRetVal;

	// Why not be optimistic
	ucRetVal = SUCCESS;

	vFRAM_init();

	if(ucFRAM_bin(uiAddr, ucData) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	vFRAM_quit();

	// Report an access violation
	if (ucRetVal == ACCESS_VIOLATION)
		vFRAM_ReportAccessViolation();

	return ucRetVal;

	}/* END: ucFRAM_read_B8() */








/****************** vFRAM_write_B8()  **************************************
*
* Write a byte of data at a specific addr to FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
******************************************************************************/

unsigned char ucFRAM_write_B8(
		uint uiAddr,
		uchar ucData
		)
	{
	unsigned char ucRetVal;

	// Why not be optimistic
	ucRetVal = SUCCESS;

	vFRAM_init();

	if(ucFRAM_bout(uiAddr,ucData) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	vFRAM_quit();

	// Report an access violation
	if (ucRetVal == ACCESS_VIOLATION)
		vFRAM_ReportAccessViolation();

	return ucRetVal;

	}/* END: vFRAM_write_B8() */







/***********************  ucFRAM_read_B16  ***********************************
*
* Read a Word from the FRAM
*
*
*****************************************************************************/

uchar ucFRAM_read_B16(
		uint uiAddr,
		uint *uiData
		)
	{
	uchar ucHI_val;
	uchar ucLO_val;
	uint uiTmp;
	uchar ucRetVal;

	// Why not be optimistic
	ucRetVal = SUCCESS;

	vFRAM_init();

	/* GET THE HI BYTE VALUE */
	if(ucFRAM_bin(uiAddr, &ucHI_val) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	/* GET THE LOW BYTE VALUE */
	uiAddr++;
	if(ucFRAM_bin(uiAddr, &ucLO_val) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	vFRAM_quit();

	uiTmp = (uint) ucHI_val;
	uiTmp =  uiTmp << 8;
	uiTmp |= (uint) ucLO_val;

	*uiData = uiTmp;

	// Report an access violation
	if (ucRetVal == ACCESS_VIOLATION)
		vFRAM_ReportAccessViolation();
	
	return(ucRetVal);

	}/* END: uiFRAM_read_B16() */






/***********************  ucFRAM_write_B16  ***********************************
*
* Write a 16 bit value to the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/

uchar ucFRAM_write_B16(
		uint uiAddr,
		uint uiData
		)
	{
	uchar ucVal;
	uchar ucRetVal;

	// Why not be optimistic
	ucRetVal = SUCCESS;

	vFRAM_init();

	/* WRITE THE HI BYTE VALUE */
	ucVal = ((uchar) (uiData >> 8));
	if(ucFRAM_bout(uiAddr, ucVal) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;
	
	/* INC THE ADDR PTR */
	uiAddr++;

	/* WRITE THE LOW BYTE VALUE */
	ucVal = ((uchar) uiData);
	if(ucFRAM_bout(uiAddr, ucVal) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	vFRAM_quit();

	// Report an access violation
	if (ucRetVal == ACCESS_VIOLATION)
		vFRAM_ReportAccessViolation();

	return ucRetVal;

	}/* END: ucFRAM_write_B16() */



/***********************  ucFRAM_read_B32  **********************************
*
* Read a 32bit word from the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/

uchar ucFRAM_read_B32(
		uint uiAddr,
		ulong *ulData
		)
	{
	uchar ucXI_val;
	uchar ucHI_val;
	uchar ucMD_val;
	uchar ucLO_val;
	uchar ucRetVal;

	// Why not be optimistic
	ucRetVal = SUCCESS;

	vFRAM_init();

	// Get the XI byte val and increment address
	if(ucFRAM_bin(uiAddr++, &ucXI_val) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	// Get the HI byte val and increment address
	if(ucFRAM_bin(uiAddr++, &ucHI_val) == ACCESS_VIOLATION)
			ucRetVal = ACCESS_VIOLATION;
	
	// Get the MD byte val and increment address
	if(ucFRAM_bin(uiAddr++, &ucMD_val) == ACCESS_VIOLATION)
			ucRetVal = ACCESS_VIOLATION;

	// Get the LO byte val and increment address
	if(ucFRAM_bin(uiAddr, &ucLO_val) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	vFRAM_quit();

	*ulData = ucXI_val;
	*ulData <<= 8;
	*ulData |= (ulong) ucHI_val;
	*ulData <<= 8;
	*ulData |= (ulong) ucMD_val;
	*ulData <<= 8;
	*ulData |= (ulong) ucLO_val;

	// Report an access violation
	if (ucRetVal == ACCESS_VIOLATION)
		vFRAM_ReportAccessViolation();

	return ucRetVal;

	}/* END: ucFRAM_read_B32() */






/***********************  ucFRAM_write_B32  ***********************************
*
* Write a 32 bit value to the FRAM
*
* NOTE: This routine assumes that the SPI bus is already turned on.
*
*****************************************************************************/

unsigned char ucFRAM_write_B32(
		uint uiAddr,
		ulong ulData
		)
	{
	uchar ucVal;
	uchar ucRetVal;

	// Why not be optimistic
	ucRetVal = SUCCESS;

	vFRAM_init();

	/* WRITE THE XI BYTE VALUE */
	ucVal = ((uchar) (ulData >> 24));
	if(ucFRAM_bout(uiAddr++, ucVal) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	/* WRITE THE HI BYTE VALUE */
	ucVal = ((uchar) (ulData >> 16));
	if(ucFRAM_bout(uiAddr++, ucVal) == ACCESS_VIOLATION)
			ucRetVal = ACCESS_VIOLATION;
	
	/* WRITE THE MD BYTE VALUE */
	ucVal = ((uchar) (ulData >> 8));
	if(ucFRAM_bout(uiAddr++, ucVal) == ACCESS_VIOLATION)
		ucRetVal = ACCESS_VIOLATION;

	/* WRITE THE LOW BYTE VALUE */
	ucVal = ((uchar) ulData);
	if(ucFRAM_bout(uiAddr, ucVal) == ACCESS_VIOLATION)
			ucRetVal = ACCESS_VIOLATION;

	vFRAM_quit();

	// Report an access violation
	if (ucRetVal == ACCESS_VIOLATION)
		vFRAM_ReportAccessViolation();

	return ucRetVal;

	}/* END: vFRAM_write_B32() */





/*******************  vFRAM_fillFramBlk()  *******************************
*
* This is a block memory setter.  It is used to fill blocks of memory with
* a specified value.
*
*****************************************************************************/

void vFRAM_fillFramBlk(
		uint uiStartAddr,
		uint uiCount,
		uchar ucSetVal
		)
	{
	uint uiCurAddr;

	// Unlock the all FRAM for diagnostics
	vFRAM_Security(0, FRAM_MAX_ADDRESS);

	for (uiCurAddr = uiStartAddr; uiCurAddr < (uiStartAddr + uiCount); uiCurAddr++) {
		ucFRAM_write_B8(uiCurAddr, ucSetVal);
	}/* END: for(uiCurAddr) */

	// Lock the all FRAM
	vFRAM_Security(0xFFFF, 0xFFFF);
	return;

	}/* END: vFRAM_fillFramBlk() */







/*******************  ucFRAM_chk_fram_blk()  *******************************
*
* This is a block memory set checker.  It is used to check blks of mem for
* a specified value.
*
* RET:	1 = ok
*		0 = err
*
*****************************************************************************/

uchar ucFRAM_chk_fram_blk(
		uint uiStartAddr,
		uint uiCount,
		uchar ucSetVal
		)
	{
	uint uiCurAddr;
	uchar ucReadVal;
	uchar ucRetVal;

	ucRetVal = 1;						//assume FRAM works good return

	// Unlock the all FRAM for diagnostics
	vFRAM_Security(0, FRAM_MAX_ADDRESS);

	for (uiCurAddr = uiStartAddr; uiCurAddr < (uiStartAddr + uiCount); uiCurAddr++) {
		ucFRAM_read_B8(uiCurAddr, &ucReadVal);
		if (ucReadVal != ucSetVal) {
			vMSG_showStorageErr("FRAM byte fail ", 15, (ulong) uiCurAddr, (ulong) ucSetVal, (ulong) ucReadVal);
			ucRetVal = 0;
		}

	}/* END: for(uiCurAddr) */

	// Lock the all FRAM
	vFRAM_Security(0xFFFF, 0xFFFF);
	
	return(ucRetVal);

	}/* END: ucFRAM_chk_fram_blk() */



/**********************  vFRAM_show_fram()  **********************************
*
*
******************************************************************************/

void vFRAM_show_fram(
		uint uiStartAddr,
		uint uiCount
		)
	{
	uint uiCurAddr;
	uchar ucVal;

	vSERIAL_sout("Fram[", 5);
	vSERIAL_HB16out(uiStartAddr);
	vSERIAL_sout("]=", 2);

	// Unlock the all FRAM for diagnostics
	vFRAM_Security(0, FRAM_MAX_ADDRESS);

	for (uiCurAddr = uiStartAddr; uiCurAddr < (uiStartAddr + uiCount); uiCurAddr++) {
		ucFRAM_read_B8(uiCurAddr, &ucVal);

		vSERIAL_HB8out(ucVal);
		vSERIAL_bout(',');

	}/* END: for(uiCurAddr) */

	vSERIAL_crlf();

	// Lock the all FRAM
	vFRAM_Security(0xFFFF, 0xFFFF);

	return;

	}/* END: vFRAM_show_fram() */




/*-------------------------------  MODULE END  ------------------------------*/
