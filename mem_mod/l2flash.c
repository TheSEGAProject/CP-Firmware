
/**************************  L2FLASH.C  *****************************************
*
* Routines to read and write the LEVEL 2 FLASH
*
*
*
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



#include <msp430x54x.h>		//processor reg description
#include "../std.h"			//standard definitions
#include "../diag.h"			//runtine configuration parameters
#include "../serial.h"			//serial port routines
#include "l2flash.h"		//Level 2 FLASH routines
#include "l2fram.h"			//level 2 FRAM routines
#include "comm.h"			//msg routines
#include "crc.h"			//crc computation routine
#include "modopt.h"			//Modify Options routines
#include "drivers/buz.h"	//buzzer routines
#include "drivers/flash.h"	//level 1 FLASH routines
#include "hal/config.h"		//hardware configuration parameters


/*********************  ucL2FLASH_chkForFlash()  ******************************
*
* This routine:
*	1. Turns on FLASH
*	2. Checks if the FLASH and SPI are working.
*	3. Turns off FLASH
*
*
* RET:	1 - good
*		0 - error
*
******************************************************************************/

unsigned char ucL2FLASH_chkForFlash(
		unsigned char ucReportMode,	//FLASH_CHK_SILENT_MODE, FLASH_CHK_REPORT_MODE
		unsigned char ucInitQuitFlag /* YES_INITQUIT, NO_INITQUIT */
		)
	{
//	uint uii;
//	unsigned char ucData;
//	uint uiPage;
//	unsigned char ucRetVal;
//
//	uiPage = 0x7;
//	ucRetVal = 1;			//assume flash is OK
//
////! If the Flash has not been initialized then quit
//	if(ucInitQuitFlag == YES_INITQUIT)
//		if(!ucFLASH_init())
//			{
//			vFLASH_quit();
//			return(0);
//			}
//
//
//	/* WRITE USER TO BUFFER */
//	for(uii=0; uii<528;  uii++)
//		{
//		vFLASH_write_byte_to_buff(FLASH_BUFFER_2,uii,((unsigned char) (uii&0xFF)));
//		}
//
//	/* CHECK THE BUFFER DATA */
//	for(uii=0; uii<528;  uii++)
//		{
//		/* CHECK THE BYTE IN MEM */
//		ucData = ucFLASH_read_byte_from_buff(FLASH_BUFFER_2, uii);
//		if(ucData != (unsigned char) (uii&0xFF))
//			{
//			if(ucReportMode == FLASH_CHK_REPORT_MODE)
//				{
//				vSERIAL_rom_sout("FlshFail(BufWrit)atPage ");
//				vSERIAL_HB16out(uiPage);
//				vSERIAL_rom_sout(" loc ");
//				vSERIAL_HB16out(uii);
//				vSERIAL_rom_sout(" wrote ");
//				vSERIAL_HB8out((unsigned char)(uii&0xFF));
//				vSERIAL_rom_sout(" read ");
//				vSERIAL_HB8out(ucData);
//				vSERIAL_crlf();
//				}
//
//			ucRetVal = 0;
//
//			}/* END: if() */
//
//		}/* for(uii) */
//
//
//	/* WRITE BUFFER TO MEM */
//	vFLASH_write_buff_to_mem(FLASH_BUFFER_2,uiPage);
//
//	/* READ MEM TO BUFFER */
//	vFLASH_read_mem_to_buff(FLASH_BUFFER_2,uiPage);
////__delay_cycles(2000);
//	/* CHECK THE WRITTEN DATA */
//	for(uii=0; uii<528;  uii++)
//		{
//		/* CHECK THE BYTE IN MEM */
//		ucData = ucFLASH_read_byte_from_buff(FLASH_BUFFER_2, uii);
//		if(ucData != (unsigned char) (uii&0xFF))
//			{
//			if(ucReportMode == FLASH_CHK_REPORT_MODE)
//				{
//				vSERIAL_rom_sout("FlshFail(MemWrit)atPage ");
//				vSERIAL_HB16out(uiPage);
//				vSERIAL_rom_sout(" loc ");
//				vSERIAL_HB16out(uii);
//				vSERIAL_rom_sout(" wrote ");
//				vSERIAL_HB8out((unsigned char)(uii&0xFF));
//				vSERIAL_rom_sout(" was ");
//				vSERIAL_HB8out(ucData);
//				vSERIAL_crlf();
//				}
//
//			ucRetVal = 0;
//
//			}/* END: if() */
//
//		}/* for(uii) */
//
//	/* SHUTOFF THE FLASH */
//	if(ucInitQuitFlag == YES_INITQUIT)
//		vFLASH_quit();
//
//	return(ucRetVal);
return(0);
	}/* END: ucL2FLASH_chkForFlash() */







/************************  vL2FLASH_storeMsgToFlash() ************************
*
* This routine:
*	1. Turns on FLASH
*	2. stores the msg int the msg buffer to FLASH
*	3. Turns off FLASH
*
* add a new message to the FLASH storage
*
* NOTE: This routine does the actual function of stuffing the msg buffer into
*		the FLASH, It does not check to see if its the right thing to do.
*		do not put guards in this code.
*
******************************************************************************/

void vL2FLASH_storeMsgToFlash(
		void
		)

	{
//	uchar uci;
//	uint uii;
//	uint uiON_flash_pagenum;
//	uint uiON_flash_msgnum;
//	uint uiFlash_refresh_pagenum;
//	long lLinear_ON_flash_ptr;
//	uchar ucMsgCount;
//
//	#if 0
//	vSERIAL_rom_sout("E:L2FLASH_storeMsgToFlash\r\n");
//	#endif
//
//	/* IF WE ARE STREAMING SEND A COPY OF THE MSG TO THE CONSOLE */
//	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_STRMING))
//		{
//		ucMsgCount = ucaMSG_BUFF[0] & MAX_MSG_SIZE_MASK;
//
//		vSERIAL_bout('^');				//header char announcing stream
//		for(uci=0;  uci<=ucMsgCount; uci++)
//			{
//			vSERIAL_HB8out(ucaMSG_BUFF[uci]);
//
//			}/* END: for(uci) */
//
//		vSERIAL_crlf();
//
//		}/* END: if() */
//
//
//	/* GET THE CURRENT PAGE THAT WE ARE WRITING -- AND THE REFRESH PAGE */
//	lLinear_ON_flash_ptr = lL2FRAM_getLinearFlashPtr(
//								FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR
//								);
//	uiFlash_refresh_pagenum = (uint)iL2FRAM_getFlashRefreshPtr();
//
//	/* CONVERT THE LINEAR FLASH PTR TO A PAGE NUMBER */
//	uiON_flash_pagenum = (uint)((lLinear_ON_flash_ptr & FLASH_LINEAR_ADDR_PAGE_MASK_L) >> 9);
//
//	/* GET THE SUB PAGE INDEX VALUE */
//	uiON_flash_msgnum = (uint)(lLinear_ON_flash_ptr & 0x1FF);
//
//
//	/* TURN FLASH ON */
//	if(!ucFLASH_init())
//		{
//		vSERIAL_rom_sout("FlshInitFail\r\n");
//		vBUZ_raygun_down();
//		}
//
//	/* LOAD THE FLASH PAGE INTO THE FLASH BUFFER */
//	vFLASH_read_mem_to_buff(FLASH_BUFFER_1,  uiON_flash_pagenum);
//
//	/* COMPUTE THE CURRENT CRC FOR THIS MSG */
//	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC
//
//	/* COPY THE CURRENT MSG BUFFER INTO THE FLASH BUFFER */
//	for(uii=0;  uii<MAX_MSG_SIZE;  uii++)
//		{
//		vFLASH_write_byte_to_buff(FLASH_BUFFER_1,		//buff num
//								uiON_flash_msgnum + uii,//buff idx + byte num
//								ucaMSG_BUFF[uii]		//byte val
//								);
//		}/* END: for() */
//
//
//	/* WRITE OUT THE FLASH BUFFER TO THE FLASH MEM */
//	vFLASH_write_buff_to_mem(FLASH_BUFFER_1,  uiON_flash_pagenum);
//
//	/* WRITE THE REFRESH FLASH PAGE ALSO */
//	vFLASH_auto_rewrite_single_page(FLASH_BUFFER_2,  uiFlash_refresh_pagenum);
//
//	/* SHUTOFF THE FLASH */
//	vFLASH_quit();
//
//	/* NOW UPDATE THE FRAM POINTERS */
//	vL2FRAM_incFlashOnPtr();
//	vL2FRAM_incFlashRefreshPtr();
//
//	#if 0
//	vSERIAL_bout('X');
//	#endif
//
//	return;

	}/* END: vL2FLASH_storeMsgToFlash() */







/************************  vL2FLASH_getNextMsgFromFlash() *********************
*
* gets a msg from the FLASH storage using the readout pointer
*
******************************************************************************/

void vL2FLASH_getNextMsgFromFlash(
		void
		)

	{
//	uint uii;
//	uint uiREADOUT_flash_pagenum;
//	uint uiREADOUT_flash_msgBaseIdx;
//
//	ulong ulFlashReadPtr;
//
//	#if 0
//	vSERIAL_rom_sout("E: L2FLASH_getNextMsgFromFlash\r\n");
//	#endif
//
//	/* GET THE CURRENT PAGE THAT WE ARE READING -- AND THE REFRESH PAGE */
//	ulFlashReadPtr = (ulong)lL2FRAM_getLinearFlashPtr(
//										FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR
//										);
//
//	/* CONVERT THE LINEAR FLASH PTR TO A PAGE NUMBER */
//	uiREADOUT_flash_pagenum = (uint)((ulFlashReadPtr >> 9) & 0x00000FFF);
//
//	/* GET THE SUB PAGE INDEX VALUE */
//	uiREADOUT_flash_msgBaseIdx = (uint)(ulFlashReadPtr & 0x1FF);
//
//
//	/* TURN ON THE FLASH */
//	if(!ucFLASH_init())
//		{
//		#if 0
//		vSERIAL_rom_sout("FlshInit2Fail\r\n");
//		#endif
//		vBUZ_raygun_down();
//		}
//
//	/* LOAD THE FLASH PAGE INTO THE FLASH BUFFER */
//	vFLASH_read_mem_to_buff(FLASH_BUFFER_1,  uiREADOUT_flash_pagenum);
//
//
//	/* COPY THE FLASH MSG TO THE MAIN MSG BUFFER */
//	for(uii=0;  uii<MAX_MSG_SIZE;  uii++)
//		{
//		 ucaMSG_BUFF[uii] = ucFLASH_read_byte_from_buff(
//		 						FLASH_BUFFER_1,
//								uiREADOUT_flash_msgBaseIdx + uii
//								);
//		}/* END: for() */
//
//
//	/* SHUTOFF THE FLASH */
//	vFLASH_quit();
//
//	/* NOW UPDATE THE FRAM POINTERS */
//	vL2FRAM_incFlashReadPtr();
//
//	return;

	}/* END: vL2FLASH_getNextMsgFromFlash() */





/*-------------------------------  MODULE END  ------------------------------*/
