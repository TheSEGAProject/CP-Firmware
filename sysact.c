

/**************************  SYSACT.C  *****************************************
*
* Routines to perform SYSACT functions during
*
*
* V1.00 10/04/2003 wzr
*		Started
*
******************************************************************************/

#include "diag.h"			//Diagnostic package

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
//*lint -e714 */  	/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
//*lint -e757 */  	/* global declarator not referenced */
//*lint -e752 */  	/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */

#include "std.h"			//standard defines
#include "misc.h"			//homeless functions
#include "serial.h" 		//serial IO port stuff
#include "sysact.h"			//SYSACT generator routines
#include "comm.h"			//msg handling routines
#include "rts.h"			//Real Time Scheduler
#include "pick.h"			//SSP table handler routines
#include "MODOPT.h"			//Modify Options routines
#include "LNKBLK.h"			//LnkBlk table handler routines
#include "hal/config.h" 		//system configuration description file	//
#include "mem_mod/l2sram.h"  		//disk storage module
#include "mem_mod/l2fram.h" 		//level 2 fram routines
#include "mem_mod/l2flash.h"		//level 2 flash routines


extern uchar ucGLOB_lastAwakeSlot;	//Nearest thing to cur slot
extern uchar ucGLOB_testByte;		//counts thermocouple onewire dropouts
extern uchar ucGLOB_testByte2;		//counts button return type 2 errors
extern uchar ucGLOB_lastAwakeNSTtblNum; //nearest thing to cur NST tbl num


#if 0
extern volatile union						//ucFLAG0_BYTE
  {
  uchar byte;
  struct
    {
	unsigned FLG0_BIGSUB_CARRY_BIT:1;		//bit 0 ;1=CARRY, 0=NO-CARRY
	unsigned FLG0_BIGSUB_6_BYTE_Z_BIT:1;	//bit 1 ;1=all diff 0, 0=otherwise
	unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT:1;//bit 2 ;1=top 4 bytes 0, 0=otherwise
	unsigned FLG0_NOTUSED_3_BIT:1;			//bit 3 ;1=SOM2 link exists, 0=none
											//SET:	when any SOM2 links exist
											//CLR: 	when the SOM2 link is lost
	unsigned FLG0_RESET_ALL_TIME_BIT:1;		//bit 4 ;1=do time  reset, 0=dont
											//SET:	when RDC4 gets finds first
											//		SOM2.
											//		or
											//		In a hub when it is reset.
											//
											//CLR: 	when vMAIN_computeDispatchTiming()
											//		runs next.
	unsigned FLG0_SERIAL_BINARY_MODE_BIT:1;	//bit 5 1=binary mode, 0=text mode
	unsigned FLG0_HAVE_WIZ_GROUP_TIME_BIT:1;//bit 6 1=Wizard group time has
											//        been aquired from a DC4
											//      0=We are using startup time
	unsigned FLG0_NOTUSED7_BIT:1;			//bit 7
	}FLAG0_STRUCT;
  }ucFLAG0_BYTE;
#endif


extern int iGLOB_completeSysLFactor;



void vSYSACT_showSystat(
		void
		);









/*****************************  CODE STARTS HERE  ****************************/




/***************************  vSYSACT_do_sleep()  ***************************
*
* SLEEP UNTIL SLOT START OR BUTTON PUSH 
* This event already has the wakeup alarm set for it when entered.
*
*****************************************************************************/
void vSYSACT_do_sleep(
		void
		)
	{
	unsigned char ucSleepRetVal;


	/*-----------------  ANNOUNCE THE STATUS  -------------------------------*/
	#if 1
	/* SHOW MESSAGE COUNT */
	vSERIAL_sout("M", 1);					//SRAM msg count
	vSERIAL_UIV16out(uiL2SRAM_getMsgCount());

	/* SHOW FLASH MESSAGE COUNT */
	vSERIAL_sout(" F", 2);
//	vSERIAL_UIV24out((usl)lL2FRAM_getFlashUnreadMsgCount());

	/* SHOW LOAD FACTOR */
	vSERIAL_sout(" Ld", 3);
	vSERIAL_IV16out(iGLOB_completeSysLFactor);
	#endif

	#if 1
	vSERIAL_sout(" ", 1);
	vComm_showSOMandROMcounts(NO_CRLF);
	#endif

	/* SHOW THE TEST BYTE DATA  */
	#if 1
	if(ucGLOB_testByte)				//if test is running
		{
		vSERIAL_bout('*');
		vSERIAL_UIV8out(ucGLOB_testByte);
		vSERIAL_bout('*');
		}
	#endif

	#if 1
	if(ucGLOB_testByte2)			//if test is running
		{
		vSERIAL_bout('-');
		vSERIAL_UIV8out(ucGLOB_testByte2);
		vSERIAL_bout('-');
		}
	#endif

	vSERIAL_crlf();



	/*---------------  DO SYSTAT STATUS REPORT HERE  -------------------------*/

	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT))
		{
		if((ucGLOB_lastAwakeSlot % 11) == 3)	//status time?
			{
			vSYSACT_showSystat();

			}/* END: if() */

		}/* END: if() */


	/*-----------------  DO THE ACTUAL SLEEP FUNCTION  ----------------------*/

	ucSleepRetVal = ucMISC_sleep_until_button_or_clk(SLEEP_MODE);

	switch(ucSleepRetVal)
		{
		case 0:								/* NORMAL WAKEUP */
			break;

		case 1:								/* HANDLE A BUTTON PUSH */
			#if  0
			vSERIAL_sout("Bttn", 4);
			vSERIAL_HB8out(ucGLOB_lastAwakeSlot);
			vSERIAL_crlf();
			#endif

//			ucKEY_doKeyboardCmdInput();		//lint !e534
			break;

		case 2:							/* CHECK FOR PAST DUE ALARM RET */
			vSERIAL_sout("AlrmOverrun\r\n", 13);

			if(ucGLOB_testByte2 != 255) ucGLOB_testByte2++;	//inc test count

			#if 0
			vSERIAL_sout("CurTime= ", 9);
			vSERIAL_IV32out(lThisAwakeTime);

			vSERIAL_sout("\r\nAlarm= ", 9);
			vSERIAL_IV32out(lNextAwakeTime);

			vSERIAL_sout("\r\nlOpUpTimeInSec= ", 18);
			vSERIAL_IV32out(lOpUpTimeInSec);

			vSERIAL_sout("\r\nlThisLinearSlot= ", 19);
			vSERIAL_IV32out(lThisLinearSlot);

			vSERIAL_sout("\r\nlSecsUsedInThisSlot= ", 23);
			vSERIAL_IV32out(lSecsUsedInThisSlot);
			#endif

			vSERIAL_crlf();
			break;

		default:
			break;

		}/* END: switch() */

	return;

	} /* END: vSYSACT_do_sleep() */






/************************  vSYSACT_do_move_SRAM_to_FLASH()  ***********************
*
* This action already has the wakeup alarm set for it when entered.
*
* This action copies the SRAM store and forward to FLASH. Check in ACTION.h
* to for actual frequency but current frequency is 1 time per frame.
*
* Usually this routines is only run on hubs, but if the startup bit is
* set it will run for spokes also.
*
****************************************************************************/
void vSYSACT_do_move_SRAM_to_FLASH(
		void
		)
	{

	return;

	}/* END: vSYSACT_do_move_SRAM_to_FLASH() */






#if 0

/***************************  vSYSACT_do_EvaluateSts()  **********************
*
* Evaluate status and adjust load
*
* This routine calls a potpourri of evaluation functions
*
*****************************************************************************/
void vSYSACT_do_EvaluateSts(
		void
		)
	{

	return;

	} /* END: vSYSACT_do_EvaluateSts() */

#endif










/*******************  vSYSACT_showSystat()  *********************************
*
* Show the system status information
*
*****************************************************************************/
void vSYSACT_showSystat(
		void
		)
	{

	/*---------------  DO SYSTAT STATUS REPORT HERE  -------------------------*/



	/*-------------------- SHOW THE HEADER LINE -----------------------------*/
	vSERIAL_crlf();
	vSERIAL_dash(15);
	vSERIAL_sout("  SYSTAT  ", 10);
	vSERIAL_dash(25);
	vSERIAL_crlf();




	/*-----------------  SHOW STATS ON LFACTORS  ----------------------------*/
	if(ucMODOPT_readSingleRamOptionBit(OPTPAIR_SHOW_STS_ON_LFACTRS))
		{
		vTask_showStatsOnLFactor();
//		vSERIAL_dash(5);
		vSERIAL_crlf();

		}/* END: if(OPTPAIR_SHOW_STS_ON_LFACTRS) */


	/*-----------  SHOW THE CURRENT NST & STBLS  ----------------------------*/
	#if 1
	vRTS_showAllNSTentrys(ucGLOB_lastAwakeNSTtblNum,YES_SHOW_TCB);
	vSERIAL_dash(5);
	vSERIAL_crlf();
	#endif


	/*------------  SHOW THE LNKBLK REQ SCHEDULE  ---------------------------*/
	vLNKBLK_showAllLnkBlkTblEntrys();
	vSERIAL_dash(5);
	vSERIAL_crlf();


	/*--------------------  SHOW THE SSP  -----------------------------------*/
	//vPICK_showEntireSSPtblFromSRAM(YES_SHOW_ALL_ENTRYS);
	vPICK_showEntireSSPtblFromSRAM( NO_SHOW_ALL_ENTRYS);

	vSERIAL_dash(5);
	vSERIAL_crlf();




	/*-----------  SHOW THE STATISTICS FOR THE RADIO LINKS  -----------------*/

	#if 1
	vLNKBLK_showLnkStats();
	#endif

	vSERIAL_dash(50);	//give a finishing line 
	vSERIAL_crlf();


	return;

	} /* END: vSYSACT_showSystat() */





/*-------------------------------  MODULE END  ------------------------------*/
