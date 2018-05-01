

/**************************  DIAG.C  ******************************************
*
* Diagnostic package used as a code include in main
*
*
* V1.00 12/23/2003 wzr
*		Started
*
******************************************************************************/


/*----------------------------------------------------------------------------
*
* NOTE1: These diagnostics are roughly in alphabetic order.  If you find one
* 		out of order or you are adding one Please spend a moment to order them
*
* NOTE2: We have tried to stylize the diagnostic form, when adding a diagnostic
*		please attempt to match the style in use here.
*
*----------------------------------------------------------------------------*/


//lint -e526 		// function not defined 
//lint -e563 		// label not referencecd 
//lint -e657 		// Unusual (nonportable) anonymous struct or union 
//lint -e714 		// symbol not referenced 
//lint -e716 		// while(1) ... 
//lint -e750 		// local macro not referenced 
//lint -e752 		// local delcarator not referenced   //****  special  ***/
//lint -e754 		// local structure member not referenced 
//lint -e755 		// global macro not referenced 
//lint -e757 		// global declarator not referenced 
//lint -e758 		// global union not referenced 
//lint -e768 		// global struct member not referenced 



#include <msp430x54x.h>		//register and ram definition file
#include "std.h"		//standard include
#include "hal/config.h" 	//system configuration definitions
#include "serial.h"		//comm port routines
#include "comm.h"		//msg defines
#include "diag.h"		//diagnostic defines


extern usl uslGLOB_sramQon_NFL;
extern usl uslGLOB_sramQoff;
extern uint uiGLOB_sramQcnt;


extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

extern volatile uchar ucCLK_TIME[4];
extern volatile uchar ucCLK2_TIME[4];

extern volatile uchar ucaBigMinuend[6];
extern volatile uchar ucaBigSubtrahend[6];
extern volatile uchar ucaBigDiff[6];


extern const uchar ucaRandTblNum[2];




extern volatile union				//ucFLAG0_BYTE
  {
  uchar byte;
  struct
    {
	unsigned FLG0_BIGSUB_CARRY_BIT:1;	 		//bit 0 ;1=CARRY, 0=NO-CARRY
	unsigned FLG0_BIGSUB_6_BYTE_Z_BIT:1;		//bit 1 ;1=all diff 0, 0=otherwise
	unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT:1;	//bit 2 ;1=top 4 bytes 0, 0=otherwise
	unsigned FLG0_REDIRECT_COMM_TO_ESPORT_BIT:1;//bit 3 ;1=REDIRECT, 0=COMM1
	unsigned FLG0_RESET_ALL_TIME_BIT:1;			//bit 4 ;1=do time  reset, 0=dont
												//SET:	when RDC4 gets finds first
												//		SOM2.
												//		or
												//		In a Hub when it is reset.
												//
												//CLR: 	when vMAIN_computeDispatchTiming()
												//		runs next.
	unsigned FLG0_SERIAL_BINARY_MODE_BIT:1;		//bit 5 1=binary mode, 0=text mode
	unsigned FLG0_HAVE_WIZ_GROUP_TIME_BIT:1;	//bit 6 1=Wizard group time has
												//        been aquired from a DC4
												//      0=We are using startup time
	unsigned FLG0_ECLK_OFFLINE_BIT:1;			//bit 7 1=ECLK is not being used
												//      0=ECLK is being used
	}FLAG0_STRUCT;
  }ucFLAG0_BYTE;


extern volatile union							//ucFLAG1_BYTE
  	{
  	uchar byte;

  	struct
  		{
  		unsigned FLG1_X_DONE_BIT:		1;	//bit 0
  		unsigned FLG1_X_LAST_BIT_BIT:	1;	//bit 1
  		unsigned FLG1_X_FLAG_BIT:		1;	//bit 2 ;1=XMIT, 0=RECEIVE
  		unsigned FLG1_R_HAVE_MSG_BIT:	1;	//bit 3	;1=REC has a msg, 0=no msg
  		unsigned FLG1_R_CODE_PHASE_BIT: 1;	//bit 4 ;1=MSG PHASE, 0=BARKER PHASE
  		unsigned FLG1_R_ABORT_BIT:		1;	//bit 5
  		unsigned FLG1_X_NXT_LEVEL_BIT:	1;	//bit 6
  		unsigned FLG1_R_SAMPLE_BIT: 	1;	//bit 7
  		}FLAG1_STRUCT;

  	}ucFLAG1_BYTE;

#define FLAG1_INIT_VAL	    0x00		//0000 0000


extern volatile union						//ucFLAG2_BYTE
  	{
  	uchar byte;

  	struct
  	 {
  	 unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=Alarm, 0=no alarm
  	 unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1
  	 unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
	 unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=clk ticked, 0=no tic
  	 unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
  	 unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
  	 unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
  	 unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
  	 }FLAG2_STRUCT;

  	}ucFLAG2_BYTE;



//extern uchar ucGLOB_curMsgSeqNum;

extern long lGLOB_lastAwakeTime;		//Nearest thing to slot time
extern long lGLOB_lastAwakeLinearSlot;	//Nearest thing to cur linear slot
extern long lGLOB_lastAwakeFrame;	 	//Nearest thing to cur frame
extern UCHAR ucGLOB_lastAwakeSlot;		//Nearest thing to cur slot
extern UCHAR ucGLOB_lastAwakeNSTtblNum;	//Nearest thing to current NST tbl

extern long lGLOB_OpMode0_inSec;			//Start of OP mode -- set by discovery

extern UCHAR ucGLOB_myLevel;				//senders level +1

extern const USL uslaNST_baseAddr[2];





#ifdef KEY1_DIAG_ENA
const char *cpaCmdName[12] = 
	{
	"A",			// 0 
	"AA",			// 1 
	"B",			// 2 
	"B0",			// 3 
	"B01",			// 4 
	"CCC",			// 5 
	"CCCCD",		// 6 
	"CCCCE",		// 7 
	"DOG",			// 8 
	"DOG1",			// 9 
	"DOG2",			//10 
	"DOG3"			//11 
	}; // END: cpaCmdName[] 

#endif //#ifdef KEY1_DIAG_ENA








//************************  DECLARATIONS  ************************************


void vMAIN_computeDispatchTiming(
		void
		);


#ifdef SRAM3_DIAG_ENA
void vDIAG_showSramVals(
		void
		);
#endif // END: SRAN3_DIAG_ENA 








//***************************  CODE STARTS  **********************************





#ifdef THIS_IS_A_DIAGNOSTIC
#if (THIS_IS_A_DIAGNOSTIC == YES)

#include "MODOPT.h"

/*********** vDIAG_setupRuntimeOptionBitsForDiagnostic() **********************
*
* This routine sets only the RAM copies of the option bits 
* for diagnostic purposes.  It does not change the option bits
* permanently just for running the included tests 
*
******************************************************************************/

void vDIAG_setupRuntimeOptionBitsForDiagnostic(
		void
		)
	{

	#ifdef DIAG_STRMING
	#if (DIAG_STRMING == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_STRMING, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_STRMING, 0);
	#endif //DIAG_STRMING
	#endif //DIAG_STRMING


	#ifdef DIAG_SHOW_SYSTAT
	#if (DIAG_SHOW_SYSTAT == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT, 0);
	#endif //DIAG_SHOW_SYSTAT
	#endif //DIAG_SHOW_SYSTAT


	#ifdef DIAG_CHK_FOR_RDIO_BD
	#if (DIAG_CHK_FOR_RDIO_BD == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FOR_RDIO_BD, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FOR_RDIO_BD, 0);
	#endif //DIAG_CHK_FOR_RDIO_BD
	#endif //DIAG_CHK_FOR_RDIO_BD


	#ifdef DIAG_USE_RDIO_FOR_RAND_SEED
	#if (DIAG_USE_RDIO_FOR_RAND_SEED == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_USE_RDIO_FOR_RAND_SEED, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_USE_RDIO_FOR_RAND_SEED, 0);
	#endif //DIAG_USE_RDIO_FOR_RAND_SEED
	#endif //DIAG_USE_RDIO_FOR_RAND_SEED


	#ifdef DIAG_SND_RDIO_STUP_MSG
	#if (DIAG_SND_RDIO_STUP_MSG == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SND_RDIO_STUP_MSG, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SND_RDIO_STUP_MSG, 0);
	#endif //DIAG_SND_RDIO_STUP_MSG
	#endif //DIAG_SND_RDIO_STUP_MSG


	#ifdef DIAG_ALLOW_BRAIN_STUP_HBRNAT
	#if (DIAG_ALLOW_BRAIN_STUP_HBRNAT == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_ALLOW_BRAIN_STUP_HBRNAT, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_ALLOW_BRAIN_STUP_HBRNAT, 0);
	#endif //DIAG_ALLOW_BRAIN_STUP_HBRNAT
	#endif //DIAG_ALLOW_BRAIN_STUP_HBRNAT


	#ifdef DIAG_SD_BD_IS_ATTCHD
	#if (DIAG_SD_BD_IS_ATTCHD == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SD_BD_IS_ATTCHD, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SD_BD_IS_ATTCHD, 0);
	#endif //DIAG_SD_BD_IS_ATTCHD
	#endif //DIAG_SD_BD_IS_ATTCHD


	#ifdef DIAG_CHK_FOR_SD_ON_STUP
	#if (DIAG_CHK_FOR_SD_ON_STUP == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FOR_SPS_ON_STUP, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FOR_SPS_ON_STUP, 0);
	#endif //DIAG_CHK_FOR_SD_ON_STUP
	#endif //DIAG_CHK_FOR_SD_ON_STUP


	#ifdef DIAG_FIX_SOIL_RDING
	#if (DIAG_FIX_SOIL_RDING == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_FIX_SOIL_RDING, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_FIX_SOIL_RDING, 0);
	#endif //DIAG_FIX_SOIL_RDING
	#endif //DIAG_FIX_SOIL_RDING


	#ifdef DIAG_DO_HBRNAT_CHK
	#if (DIAG_DO_HBRNAT_CHK == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_DO_HBRNAT_CHK, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_DO_HBRNAT_CHK, 0);
	#endif //DIAG_DO_HBRNAT_CHK
	#endif //DIAG_DO_HBRNAT_CHK


	#ifdef DIAG_BUZ_ON
	#if (DIAG_BUZ_ON == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_BUZ_ON, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_BUZ_ON, 0);
	#endif //DIAG_BUZ_ON
	#endif //DIAG_BUZ_ON


	#ifdef DIAG_CHK_FOR_FRAM_ON_STUP
	#if (DIAG_CHK_FOR_FRAM_ON_STUP == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FOR_FRAM_ON_STUP, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FOR_FRAM_ON_STUP, 0);
	#endif //DIAG_CHK_FOR_FRAM_ON_STUP
	#endif //DIAG_CHK_FOR_FRAM_ON_STUP


	#ifdef DIAG_CHK_FRAM_FMT_ON_STUP
	#if (DIAG_CHK_FRAM_FMT_ON_STUP == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FRAM_FMT_ON_STUP, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FRAM_FMT_ON_STUP, 0);
	#endif //DIAG_CHK_FRAM_FMT_ON_STUP
	#endif //DIAG_CHK_FRAM_FMT_ON_STUP


	#ifdef DIAG_CHK_FOR_FLSH_ON_STUP
	#if (DIAG_CHK_FOR_FLSH_ON_STUP == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FOR_FLSH_ON_STUP, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_CHK_FOR_FLSH_ON_STUP, 0);
	#endif //DIAG_CHK_FOR_FLSH_ON_STUP
	#endif //DIAG_CHK_FOR_FLSH_ON_STUP


	#ifdef DIAG_SHOW_STS_ON_LFACTRS
	#if (DIAG_SHOW_STS_ON_LFACTRS == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SHOW_STS_ON_LFACTRS, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SHOW_STS_ON_LFACTRS, 0);
	#endif //DIAG_SHOW_STS_ON_LFACTRS
	#endif //DIAG_SHOW_STS_ON_LFACTRS


	#ifdef DIAG_SHOW_LNK_SCHED
	#if (DIAG_SHOW_LNK_SCHED == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SHOW_LNK_SCHED, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SHOW_LNK_SCHED, 0);
	#endif //DIAG_SHOW_LNK_SCHED
	#endif //DIAG_SHOW_LNK_SCHED



	#ifdef DIAG_RPT_SOM2_LNKS_TO_RDIO
	#if (DIAG_RPT_SOM2_LNKS_TO_RDIO == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SOM2_LNKS_TO_RDIO, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SOM2_LNKS_TO_RDIO, 0);
	#endif //DIAG_RPT_SOM2_LNKS_TO_RDIO
	#endif //DIAG_RPT_SOM2_LNKS_TO_RDIO

	#ifdef DIAG_RPT_SOM2_LNKS_TO_FLSH
	#if (DIAG_RPT_SOM2_LNKS_TO_FLSH == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SOM2_LNKS_TO_FLSH, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SOM2_LNKS_TO_FLSH, 0);
	#endif //DIAG_RPT_SOM2_LNKS_TO_FLSH
	#endif //DIAG_RPT_SOM2_LNKS_TO_FLSH



	#ifdef DIAG_RPT_SOM2_FAILS_TO_RDIO
	#if (DIAG_RPT_SOM2_FAILS_TO_RDIO == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SOM2_FAILS_TO_RDIO, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SOM2_FAILS_TO_RDIO, 0);
	#endif //DIAG_RPT_SOM2_FAILS_TO_RDIO
	#endif //DIAG_RPT_SOM2_FAILS_TO_RDIO

	#ifdef DIAG_RPT_SOM2_FAILS_TO_FLSH
	#if (DIAG_RPT_SOM2_FAILS_TO_FLSH == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SOM2_FAILS_TO_FLSH, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SOM2_FAILS_TO_FLSH, 0);
	#endif //DIAG_RPT_SOM2_FAILS_TO_FLSH
	#endif //DIAG_RPT_SOM2_FAILS_TO_FLSH



	#ifdef DIAG_RPT_ROM2_LNKS_TO_RDIO
	#if (DIAG_RPT_ROM2_LNKS_TO_RDIO == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_ROM2_LNKS_TO_RDIO, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_ROM2_LNKS_TO_RDIO, 0);
	#endif //DIAG_RPT_ROM2_LNKS_TO_RDIO
	#endif //DIAG_RPT_ROM2_LNKS_TO_RDIO

	#ifdef DIAG_RPT_ROM2_LNKS_TO_FLSH
	#if (DIAG_RPT_ROM2_LNKS_TO_FLSH == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_ROM2_LNKS_TO_FLSH, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_ROM2_LNKS_TO_FLSH, 0);
	#endif //DIAG_RPT_ROM2_LNKS_TO_FLSH
	#endif //DIAG_RPT_ROM2_LNKS_TO_FLSH



	#ifdef DIAG_RPT_ROM2_FAILS_TO_RDIO
	#if (DIAG_RPT_ROM2_FAILS_TO_RDIO == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_ROM2_FAILS_TO_RDIO, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_ROM2_FAILS_TO_RDIO, 0);
	#endif //DIAG_RPT_ROM2_FAILS_TO_RDIO
	#endif //DIAG_RPT_ROM2_FAILS_TO_RDIO

	#ifdef DIAG_RPT_ROM2_FAILS_TO_FLSH
	#if (DIAG_RPT_ROM2_FAILS_TO_FLSH == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_ROM2_FAILS_TO_FLSH, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_ROM2_FAILS_TO_FLSH, 0);
	#endif //DIAG_RPT_ROM2_FAILS_TO_FLSH
	#endif //DIAG_RPT_ROM2_FAILS_TO_FLSH

	

	#ifdef DIAG_RPT_SYS_RSTRTS_TO_RDIO
	#if (DIAG_RPT_SYS_RSTRTS_TO_RDIO == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SYS_RSTRTS_TO_RDIO, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SYS_RSTRTS_TO_RDIO, 0);
	#endif //DIAG_RPT_SYS_RSTRTS_TO_RDIO
	#endif //DIAG_RPT_SYS_RSTRTS_TO_RDIO

	#ifdef DIAG_RPT_SYS_RSTRTS_TO_FLSH
	#if (DIAG_RPT_SYS_RSTRTS_TO_FLSH == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SYS_RSTRTS_TO_FLSH, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_SYS_RSTRTS_TO_FLSH, 0);
	#endif //DIAG_RPT_SYS_RSTRTS_TO_FLSH
	#endif //DIAG_RPT_SYS_RSTRTS_TO_FLSH



	#ifdef DIAG_RPT_BATT_DEAD_RSTRTS_TO_RDIO
	#if (DIAG_RPT_BATT_DEAD_RSTRTS_TO_RDIO == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_BATT_DEAD_RSTRTS_TO_RDIO, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_BATT_DEAD_RSTRTS_TO_RDIO, 0);
	#endif //DIAG_RPT_BATT_DEAD_RSTRTS_TO_RDIO
	#endif //DIAG_RPT_BATT_DEAD_RSTRTS_TO_RDIO

	#ifdef DIAG_RPT_BATT_DEAD_RSTRTS_TO_FLSH
	#if (DIAG_RPT_BATT_DEAD_RSTRTS_TO_FLSH == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_BATT_DEAD_RSTRTS_TO_FLSH, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_BATT_DEAD_RSTRTS_TO_FLSH, 0);
	#endif //DIAG_RPT_BATT_DEAD_RSTRTS_TO_FLSH
	#endif //DIAG_RPT_BATT_DEAD_RSTRTS_TO_FLSH



	#ifdef DIAG_RPT_BATT_VOLTS_TO_RDIO
	#if (DIAG_RPT_BATT_VOLTS_TO_RDIO == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_BATT_VOLTS_TO_RDIO, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_BATT_VOLTS_TO_RDIO, 0);
	#endif //DIAG_RPT_BATT_VOLTS_TO_RDIO
	#endif //DIAG_RPT_BATT_VOLTS_TO_RDIO

	#ifdef DIAG_RPT_BATT_VOLTS_TO_FLSH
	#if (DIAG_RPT_BATT_VOLTS_TO_FLSH == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_BATT_VOLTS_TO_FLSH, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_BATT_VOLTS_TO_FLSH, 0);
	#endif //DIAG_RPT_BATT_VOLTS_TO_FLSH
	#endif //DIAG_RPT_BATT_VOLTS_TO_FLSH



	#ifdef DIAG_RPT_TIME_CHGS_TO_RDIO
	#if (DIAG_RPT_TIME_CHGS_TO_RDIO == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_TIME_CHGS_TO_RDIO, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_TIME_CHGS_TO_RDIO, 0);
	#endif //DIAG_RPT_TIME_CHGS_TO_RDIO
	#endif //DIAG_RPT_TIME_CHGS_TO_RDIO

	#ifdef DIAG_RPT_TIME_CHGS_TO_FLSH
	#if (DIAG_RPT_TIME_CHGS_TO_FLSH == YES)
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_TIME_CHGS_TO_FLSH, 1);
	#else
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_RPT_TIME_CHGS_TO_FLSH, 0);
	#endif //DIAG_RPT_TIME_CHGS_TO_FLSH
	#endif //DIAG_RPT_TIME_CHGS_TO_FLSH

	return;

	}// END: vDIAG_setupRuntimeOptionBitsForDiagnostic() 

#endif //THIS_IS_A_DIAGNOSTIC
#endif //THIS_IS_A_DIAGNOSTIC







#ifdef FLASH8_DIAG_ENA
#include "buz.h"

/*****************  ucDIAG_check_flash_buffer_for_val()  *********************
*
* This routine confirms that the flash buffer is filled with the desired
* data constant.
*
* RET:	1 = data is desired constant value
*		0 = data is not desired constant value
*
******************************************************************************/

uchar ucDIAG_check_flash_buffer_for_val(	//1 = OK, 0= not OK
		uint uiPage,				//page number of data
		uchar ucBufferNum,			//buffer number being used (1 or 2)
		uchar ucVal					//constant value expected
		)
	{
	uint uii;
	uchar ucData;
	uchar ucFlashOKflg;

	ucFlashOKflg = YES;

	// CHECK BUFFER TO MAKE SURE ITS SET == TEST DATA 
	for(uii=0; uii<528;  uii++)
		{
		// CHECK THE BYTE IN BUFFER 
		ucData = ucFLASH_read_byte_from_buff(ucBufferNum, uii);
		if(ucData != ucVal)
			{
			ucFlashOKflg = NOPE;
			}// END: if() 
		}// for(uii) 
		
	return(ucFlashOKflg);

	} // END: ucDIAG_check_flash_buffer_for_val() 

#endif // FLASH8_DIAG_ENA 



#ifdef ENABLE_DIAG_SDLOD_SUBROUTINES
#if (ENABLE_DIAG_SDLOD_SUBROUTINES == YES)
#include "sdlod.h"
/*************************  vDIAG_show_SD_data_blk()  ************************
*
* Show a 64 byte data blk from the SD memory
*
******************************************************************************/

void vDIAG_show_SD_data_blk(
		usl uslAddr
		)
	{
	uchar ucc;
	uchar ucVal;


	//*********  SHOW THE 64 BYTE ROW  ********************
	vSDLOD_load_tblptr(uslAddr);
	for(ucc=0;  ucc<64;  ucc++)
		{
		if((ucc % 16) == 0)
			{
			vSERIAL_crlf();
			vSERIAL_HB24out(uslAddr + ucc);
			vSERIAL_sout(": ", 2);
			}

		ucVal = ucSDLOD_XCT_single_read_cmd(PROG_TBL_RD_POST_INC);

		vSERIAL_HB8out(ucVal);
		vSERIAL_bout(',');

		}// END: for() 

	vSERIAL_crlf();

	return;

	}// END: vDIAG_show_SD_data_blk() 

#endif // END: ENABLE_DIAG_SDLOD_SUBROUTINES 
#endif // END: ENABLE_DIAG_SDLOD_SUBROUTINES 






#ifdef ENABLE_DIAG_SDLOD_SUBROUTINES
#if (ENABLE_DIAG_SDLOD_SUBROUTINES == YES)
/*********************  vDIAG_show_SD_ucaMSG_BUFF()  ****************************
*
* Show a 64 byte data blk from the ucaMSG_BUFF
*
******************************************************************************/

void vDIAG_show_SD_ucaMSG_BUFF(
		void
		)
	{
	uchar ucc;

	for(ucc=0;  ucc<64;  ucc++)
		{
		if((ucc % 16) == 0)
			{
			vSERIAL_crlf();
			vSERIAL_HB24out((usl)ucc+4);
			vSERIAL_sout(": ", 2);
			}

		vSERIAL_HB8out(ucaMSG_BUFF[ucc+4]);
		vSERIAL_bout(',');

		}// END: for() 

	vSERIAL_crlf();

	return;

	}// END: vDIAG_show_SD_ucaMSG_BUFF() 

#endif // END: ENABLE_DIAG_SDLOD_SUBROUTINES 
#endif // END: ENABLE_DIAG_SDLOD_SUBROUTINES 






#ifdef SRAM3_DIAG_ENA
/*********************  vDIAG_showSramVals() *****************************
*
* Sram Diagnostic subroutine
*
*************************************************************************/
void vDIAG_showSramVals(
		void
		)
	{
	ulong ulii;

	vSERIAL_sout("Base=", 4);
	vSERIAL_UIV32out(L2SRAM_BASE_ADDR_UL);
	vSERIAL_sout("  Bufsiz=", 9);
	vSERIAL_UIV32out(SRAM_TEST_MSG_BUFF);
	vSERIAL_crlf();

	vSERIAL_sout("Qbase=", 6);
	vSERIAL_UIV24out((usl)L2SRAM_MSG_Q_BEG_UL);
	vSERIAL_sout("  Qend=", 7);
	vSERIAL_UIV24out((usl)L2SRAM_MSG_Q_END_UL);
	vSERIAL_sout("  Qon=", 6);
	vSERIAL_UIV24out(uslGLOB_sramQon_NFL);
	vSERIAL_sout("  Qoff=", 7);
	vSERIAL_UIV24out(uslGLOB_sramQoff);
	vSERIAL_sout("  Qcnt=", 7);
	vSERIAL_UIV16out(uiGLOB_sramQcnt);
	vSERIAL_sout("  Qvacancy=", 11);
	vSERIAL_UIV16out(uiL2SRAM_getVacantMsgCount());
	vSERIAL_crlf();


	// SHOW THE MSG BUFFER 
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);


	// SHOW THE SRAM BUFFER 
	for(ulii=L2SRAM_MSG_Q_BEG_UL;
		ulii<L2SRAM_MSG_Q_END_UL;
		ulii++
		)
		{
		if(((ulii % 16) == 0) || (ulii == 0))
			{
			vSERIAL_crlf();
			vSERIAL_sout("[", 1);
			vSERIAL_HBV24out(ulii);
			vSERIAL_sout("] ", 2);
			}

		vSERIAL_HB8out(ucSRAM_read_B8(ulii));

		if(uslGLOB_sramQon_NFL == ulii)
			{
			vSERIAL_sout("ON(", 4);
			vSERIAL_UIV16out(uiGLOB_sramQcnt);
			vSERIAL_sout(")", 1);
			}
		if(uslGLOB_sramQoff == ulii)
			{
			vSERIAL_sout("OFF", 4);
			}
		vSERIAL_bout(',');
		}
	vSERIAL_crlf();

	return;

	}// END: vDIAG_showSramVals() 

#endif // SRAM3_DIAG_ENA 






/***********************  vDIAG_run_diagnostics()  ***************************
*
* Routines to test all parts of the wiz
*
******************************************************************************/

void vDIAG_run_diagnostics(
		void
		)
	{

	#ifdef THIS_IS_A_DIAGNOSTIC
	#if (THIS_IS_A_DIAGNOSTIC == YES)

	/******************************************************************
	*
	* This routine sets only the RAM copies of the option bits 
	* for diagnostic purposes.  It does not change the option bits
	* permanently just for running the included tests 
	*
	*******************************************************************/

	vDIAG_setupRuntimeOptionBitsForDiagnostic();

	#endif //THIS_IS_A_DIAGNOSTIC
	#endif //THIS_IS_A_DIAGNOSTIC









	/****************  START OF DIAGNOSTIC SNIPPETS  *************************/

	#ifdef KEY_ROUTINE_ENA
	{
	#include "key.h"		//keyboard routines

	/***********************  KEY ROUTINES  *********************************
	*
	* Routines to test the key commands
	*
	*************************************************************************/

	while(TRUE)		//lint !e774 !e722
		{
		vSERIAL_sout("KEY: Key routine direct\r\n", 25);

		ucKEY_doKeyboardCmdInput();				//lint !e534

		vSERIAL_sout("Xit\r\n", 5);

		}// END: while() 
	}
	#endif // KEY_ROUTINE_ENA 





	#ifdef DISPATCHER_TIMING_DIAG_ENA
	{
	/*****************  DISPATCHER_TIMING_DIAG_ENA  **********************
	*
	* Routines to test all parts of the wiz
	*
	*************************************************************************/
	UCHAR ucc;


	vSERIAL_sout("DPT: Dispatch Timing Test\r\n", 27);


	// INIT THE SCHEDULER TABLES 
	vSTBL_setupInitialSchedTbls();

	vRTS_scheduleNSTtbl(0L);



	// INIT MY_LEVEL TO UNKNOWN 
	ucGLOB_myLevel = LEVEL_MAX_VAL;
								//setup default level = illegal level
					 			//so we can tell if we have been assigned
								//a level


	/************  OPERATIONAL_MODE SECTION  ****************/

	{

	// SHOW THE DEBUG HEADER 
	vSERIAL_printDashIntro("OP MODE");	

	// INIT THE CURRENT FRAME COUNTER 
	lGLOB_OpMode0_inSec = 21;
	lGLOB_lastAwakeTime = lGLOB_OpMode0_inSec - 1L;
	lGLOB_lastAwakeLinearSlot = -1;
	lGLOB_lastAwakeFrame = -1;
	ucGLOB_lastAwakeSlot = 63;


	// SET THE ALARM TIME TO WAKE UP WHEN OP-MODE STARTS 
	vTIME_setAlarmFromLong((unsigned long)lGLOB_OpMode0_inSec);
	vSERIAL_sout("Sleeping til OP..\r\n", 19);
	ucMISC_sleep_until_button_or_clk(SLEEP_MODE);			//lint !e534

	vSERIAL_printDashIntro("OP BEG");	



	/**********************  MAIN OP MODE LOOP *******************************/
	// THIS IS A SLOT LOOP 
	for(ucc=0;  ucc<27;  ucc++)
		{
		uchar uci;

		UCHAR ucaInitTime[6];
		UCHAR ucaSetTime[6];
		UCHAR ucaCaptureTime[6];
		uchar ucaDispatchBegTime[6];
		uchar ucaDispatchEndTime[6];


		vSERIAL_printDashIntro("ST");

		vTIME_copyWholeSysTimeToBytesInDtics(&ucaInitTime[0]); //get the time before
		vMAIN_computeDispatchTiming();					//dispatch
		vTIME_copyWholeSysTimeToBytesInDtics(&ucaCaptureTime[0]); //get the time after
	

		vSERIAL_sout("\r\nEnter Compute Dispatch = ", 27);
		for(uci=0;  uci<6;  uci++)
			{
			vSERIAL_HB8out(ucaInitTime[uci]);
			}
		vSERIAL_crlf();

		vSERIAL_sout("Leave Compute Dispatch = ", 25);
		for(uci=0;  uci<6;  uci++)
			{
			vSERIAL_HB8out(ucaCaptureTime[uci]);
			}
		vSERIAL_crlf();


		vSERIAL_sout("Time run during Compute Dispatch = ", 35);
		vTIME_showWholeTimeDiffInDuS(&ucaInitTime[0], &ucaCaptureTime[0], YES_CRLF);

		vSERIAL_sout("lGLOB_lastAwakeTime= ", 21);
		vSERIAL_HBV32out((ULONG)lGLOB_lastAwakeTime);
		vSERIAL_crlf();

		vSERIAL_sout("lGLOB_lastAwakeLinearSlot= ", 27);
		vSERIAL_HBV32out((ULONG)lGLOB_lastAwakeLinearSlot);
		vSERIAL_crlf();

		vSERIAL_sout("lGLOB_lastAwakeFrame= ", 22);
		vSERIAL_HBV32out((ULONG)lGLOB_lastAwakeFrame);
		vSERIAL_crlf();

		vSERIAL_sout("lGLOB_lastAwakeSlot= ", 21);
		vSERIAL_HB8out(ucGLOB_lastAwakeSlot);
		vSERIAL_crlf();

		vSERIAL_sout("lGLOB_lastAwakeNSTtblNum= ", 26);
		vSERIAL_HB4out(ucGLOB_lastAwakeNSTtblNum);
		vSERIAL_crlf();

		vSERIAL_sout("lGLOB_lastScheduledFrame= ", 26);
		vSERIAL_HBV32out((ULONG)lGLOB_lastScheduledFrame);
		vSERIAL_crlf();

		vSERIAL_sout("lGLOB_lastScheduledSchedSlot= ", 30);
		vSERIAL_HB8out(ucGLOB_lastScheduledSchedSlot);
		vSERIAL_crlf();


		#if 0
		// FAKE AN OVERRUN ON SLOT 1 
		if(lGLOB_lastAwakeTime == 0x19)
			{
			vSERIAL_printDashIntro("FAKE A SLOT OVERRUN");

			#if 0					//overrun by FF tics
			ucaSetTime[0] = 0;
			ucaSetTime[1] = 0;
			ucaSetTime[2] = 0;
			ucaSetTime[3] = 0x1D;
			ucaSetTime[4] = 0x80;
			ucaSetTime[5] = 0xFF;
			#endif


			#if 0					//overrun by 3sec & FF63 tics
			ucaSetTime[0] = 0;
			ucaSetTime[1] = 0;
			ucaSetTime[2] = 0;
			ucaSetTime[3] = 0x20;
			ucaSetTime[4] = 0xFF;
			ucaSetTime[5] = 0x63;
			#endif


			#if 1					//overrun by 3sec & FF63 tics
			ucaSetTime[0] = 0;
			ucaSetTime[1] = 0;
			ucaSetTime[2] = 0;
			ucaSetTime[3] = 0x20;
			ucaSetTime[4] = 0xFE;
			ucaSetTime[5] = 0x00;
			#endif

			vTIME_setWholeSysTimeFromBytesInDtics(&ucaSetTime[0]);
			}
		#endif



		#if 1
		// FAKE AN OVERRUN ON SCHEDULER SLOT 22 
		if(lGLOB_lastAwakeLinearSlot == 22)
			{
			vSERIAL_printDashIntro("OVERRUN OVER SCHEDULER SLOT");

			#if 1
			ucaSetTime[0] = 0;
			ucaSetTime[1] = 0;
			ucaSetTime[2] = 0;
			ucaSetTime[3] = (21 + (23 * 4) + 3);	//Set time to end of slot 23
			ucaSetTime[4] = 0xFF;
			ucaSetTime[5] = 0x63;
			#endif

			vTIME_setWholeSysTimeFromBytesInDtics(&ucaSetTime[0]);
			}
		#endif


		// DISPATCH TO THE EVENT 
		vTIME_copyWholeSysTimeToBytesInDtics(&ucaDispatchBegTime[0]); //get the time before
		vACTION_dispatch_to_event(
							ucGLOB_lastAwakeNSTtblNum,
							ucGLOB_lastAwakeSlot
							);
		vTIME_copyWholeSysTimeToBytesInDtics(&ucaDispatchEndTime[0]); //get the time after




		vSERIAL_sout("\r\nDispatch Beg Time = ", 22);
		for(uci=0;  uci<6;  uci++)
			{
			vSERIAL_HB8out(ucaDispatchBegTime[uci]);
			}
		vSERIAL_crlf();

		vSERIAL_sout("Dispatch End Time = ", 20);
		for(uci=0;  uci<6;  uci++)
			{
			vSERIAL_HB8out(ucaDispatchEndTime[uci]);
			}
		vSERIAL_crlf();


		vSERIAL_sout("Run Time during Execution = ", 28);
		vTIME_showWholeTimeDiffInDuS(&ucaDispatchBegTime[0], &ucaDispatchEndTime[0], YES_CRLF);


		}// END: for(ucc) 

	}// END: OPERATIONAL MODE 

	vSERIAL_sout("EXITING\r\n", 9);

	}
	#endif // DISPATCHER_TIMING_DIAG_ENA 




	#ifdef PICK1_DIAG_ENA
	{
	#include "PICK.h"
	#include "SENSOR.h"
	#include "OPMODE.h"
//	#include "time.h"
	#include "L2FRAM.h"
	#include "L2SRAM.h"
	#include "THERMO.h"

	/***********************  PICK1 diagnostic  *****************************
	*
	* Debug the pick code
	*
	**************************************************************************/
	uchar ucSensorNum[4];
	uint uiSensorData[4];
	long lVal;
	long lTime;
	uchar ucMsgIdx;
	uchar ucArrayIdx;
	uchar ucFlag;

	vSERIAL_sout("\r\nPICK1: Debug uniform compare\r\n", 32);

	vPICK_initSSPtbls();

	lVal = (long)iTHERMO_computeTfromTC(0,378);


	vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, SENSOR_TC_1,(ulong)lVal);
	vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, SENSOR_ONEWIRE_0,378UL);
	vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, SENSOR_LIGHT_1, 300UL);
	vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, SENSOR_LIGHT_2, 300UL);
	vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, SENSOR_LIGHT_3, 300UL);
	vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, SENSOR_LIGHT_4, 300UL);

	vPICK_putSSP_tblEntry(SSP_T_TBL_NUM, SENSOR_TC_1,    500000UL);
	vPICK_putSSP_tblEntry(SSP_T_TBL_NUM, SENSOR_LIGHT_1, 500000UL);
	vPICK_putSSP_tblEntry(SSP_T_TBL_NUM, SENSOR_LIGHT_2, 500000UL);
	vPICK_putSSP_tblEntry(SSP_T_TBL_NUM, SENSOR_LIGHT_3, 500000UL);
	vPICK_putSSP_tblEntry(SSP_T_TBL_NUM, SENSOR_LIGHT_4, 500000UL);

	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, SENSOR_TC_1,  0x30UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, SENSOR_ONEWIRE_0,1UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, SENSOR_LIGHT_1, 75UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, SENSOR_LIGHT_2, 75UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, SENSOR_LIGHT_3, 75UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, SENSOR_LIGHT_4, 75UL);

	vPICK_putSSP_tblEntry(SSP_DELTA_T_TRIG_TBL_NUM, SENSOR_TC_1,    100UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_T_TRIG_TBL_NUM, SENSOR_LIGHT_1, 100UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_T_TRIG_TBL_NUM, SENSOR_LIGHT_2, 100UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_T_TRIG_TBL_NUM, SENSOR_LIGHT_3, 100UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_T_TRIG_TBL_NUM, SENSOR_LIGHT_4, 100UL);

	// SHOW THE SSP ENTRYS
	vPICK_showEntireSSPtblFromSRAM(NO_SHOW_ALL_ENTRYS);




	ucSensorNum[0] = SENSOR_LIGHT_1;
	uiSensorData[0] = 374;

	ucSensorNum[1] = SENSOR_LIGHT_2;
	uiSensorData[1] = 305;

	ucSensorNum[2] = SENSOR_TC_1;
	uiSensorData[2] = 0;

	ucSensorNum[3] = SENSOR_ONEWIRE_0;
	uiSensorData[3] = 378;

//	lTime = 500000;
//	lTime = 500048;
//	lTime = 500049;
	lTime = 500100;
//	lTime = 500101;

	ucMsgIdx = 22;
	ucArrayIdx = 2;

	for(uiSensorData[2]=0;  ;  uiSensorData[2]++)
		{
		// BUILD AN OM2
	  	vOPMODE_buildMsg_OM2(
				0,								//LAST_PKT_BIT & NO_DATA_PKT_BIT
				0,								//dest SN
				ucMSG_incMsgSeqNum(),			//msg seq num
				0,								//Load Factor
				uiL2FRAM_getSnumLo16AsUint(),	//Agent SN
				lTime,							//Collection Time
				4,								//Data count
				ucSensorNum[0],					//Sensor 0
				uiSensorData[0],				//Sensor 0 data
				ucSensorNum[1],					//Sensor 1
				uiSensorData[1],				//Sensor 1 data
				ucSensorNum[2],					//Sensor 2
				uiSensorData[2],				//Sensor 2 data
				ucSensorNum[3],					//Sensor 3
				uiSensorData[3]					//Sensor 3 data
				);


		lVal = lPICK_buildUniformCompareEntryFromOM2(ucMsgIdx);

		vSERIAL_sout("DIAG:SensorID=", 14);
		vSERIAL_UIV8out(ucSensorNum[ucArrayIdx]);
		vSERIAL_bout('(');
		vSENSOR_showSensorName(ucSensorNum[ucArrayIdx], R_JUSTIFY);

		vSERIAL_sout(") Put in=", 9);
		vSERIAL_UIV16out(uiSensorData[ucArrayIdx]);
		vSERIAL_bout('(');
		vSERIAL_HB16out(uiSensorData[ucArrayIdx]);

		vSERIAL_sout(") Got out=", 10);
		vSERIAL_IV32out(lVal);
		vSERIAL_bout('(');
		vSERIAL_HB32out((ulong)lVal);
		vSERIAL_sout(")\r\n", 3);

		ucFlag = ucPICK_putTriggeredDataIntoSSP();

		vSERIAL_sout("FLAG = ", 7);
		vSERIAL_UIV8out(ucFlag);
		vSERIAL_crlf();

		if(ucFlag) break;

		}// END: for()

	// SHOW THE SSP ENTRYS
	vPICK_showEntireSSPtblFromSRAM(NO_SHOW_ALL_ENTRYS);

	}
	#endif // PICK1_DIAG_ENA






	#ifdef PWR1_DIAG_ENA
	{
	#include "ad.h"		//onboard 10 bit AD routines
	#include "delay.h" 	//time delay routines
	#include "misc.h"	//Misc routines

	/***********************  PWR1 diagnostic  *******************************
	*
	* Continuously read the Battery A/D
	*
	**************************************************************************/
	ulong ulii;
	uint uiBatt_in_mV;

	vSERIAL_sout("PWR1: Continuous Batt reading\r\n", 31);

	for(ulii=0;  ;  ulii++)
		{
		if((ulii % 16) == 0)
			{
			vSERIAL_sout("\r\n   NUM  milliV\r\n", 18);
			}

		uiBatt_in_mV = uiMISC_doCompensatedBattRead();

		vSERIAL_UI16out((uint)ulii);
		vSERIAL_sout(": ", 2);
		vSERIAL_UIV16out(uiBatt_in_mV);
		vSERIAL_crlf();

		// ADD A WAIT
		vDELAY_wait100usTic(5000);		//500ms

		}// END: for()

	}
	#endif // PWR1_DIAG_ENA







	#ifdef PWR2_DIAG_ENA
	{
	/***********************  PWR2 diagnostic  *******************************
	*
	* Poll the PWR SOURCE bit and report any changes
	*
	*
	**************************************************************************/

	vSERIAL_sout("PWR2: Poll the PWR SOURCE bit and report any changes\r\n", 54);

	if(POWER_SRC_BIT)
		{
		vSERIAL_sout("PWR2: Pwr Src = EXTERNAL\r\n", 26);
		LED_PORT &= ~LED_BIT;
		}

	if(!POWER_SRC_BIT)
		{
		vSERIAL_sout("PWR2: Pwr Src = BATTERY \r\n", 26);
		LED_PORT |= LED_BIT;
		}

	while(TRUE) //lint !e716 !e774
		{
		if(POWER_SRC_BIT)
			{
			LED_PORT |= LED_BIT;
			while(POWER_SRC_BIT);	//lint !e722
			vSERIAL_sout("Pwr Src switched to BATTERY\r\n", 29);
			}
		else
			{
			LED_PORT &= ~LED_BIT;
			while(!POWER_SRC_BIT);	//lint !e722
			vSERIAL_sout("Pwr Src switched to EXTERNAL\r\n", 30);
			}

		vDELAY_wait100usTic(1000);			//wait 100ms

		}// END: while()

	}
	#endif // PWR2_DIAG_ENA






	#ifdef PWR3_DIAG_ENA
	{
	/***********************  PWR3 SEND diagnostic  *************************
	*
	* Routine to flip the SD power on and off
	*
	* SD DIAG 	so this needs change!
	*
	*************************************************************************/

	vSERIAL_sout("PWR3: SD power ON/OFF test \r\n", 29);

	while(TRUE)							//lint !e716 !e774
		{
		vSERIAL_sout("SD power ON\r\n", 13);

		// POWER UP THE SD BOARD

		// PUT THE SD BOARD IN RESET
		PORTFbits.SD_RESET_BIT = 0;		//SD RESET = reset

		// POWER ON TO SD BOARD
		PORTBbits.SD_POWER_BIT = 0;		//SD power on

		// MUST WAIT AFTER POWER ON TO PREVENT LOSS OF DATA BITS
		vDELAY_wait4us();			   			//SD wait -- prevent losing bits

		// LET THE SD BOARD RUN
		PORTFbits.SD_RESET_BIT = 1;		//SD run

		// BLOCK THE BOOT LOADER FROM COMING UP
		PORTFbits.SD_BT_LDR_DISABLE_BIT = 1;	//SD loader = BLOCKED
		vSERIAL_any_char_to_cont();




		vSERIAL_sout("SD power OFF\r\n", 14);

		// POWER DOWN THE SD BOARD

		// PUT THE SD BOARD IN RESET
		PORTFbits.SD_BT_LDR_DISABLE_BIT = 0;	//make sure no power from this pin
		PORTFbits.SD_RESET_BIT = 0;		//SD RESET = reset

		// POWER ON TO SD BOARD
		PORTBbits.SD_POWER_BIT = 1;		//SD power off

		vSERIAL_any_char_to_cont();




		}// END: while()

	}
	#endif // PWR3_RECEIVE_DIAG_ENA






	#ifdef FLASH1_DIAG_ENA
	{
	/***********************  FLASH1 diagnostic  *******************************
	*
	*
	*
	*
	**************************************************************************/

	uchar uc;
	uchar ucc;
	uchar ucData;

	vSERIAL_sout("FLASH1: READ STATUS REG AND SHOW IT\r\n", 37);

//	for(uc=0;  ;uc++)
	for(uc=3;  ;)
		{

		for(ucc=0; ucc<3;  ucc++)
			{
			uc &= 0x03;
			vFLASH_init(uc);						//init FLASH

			ucData = ucFLASH_read_sts_reg();		//read the status reg

			// SHOW THE STATUS
			vSERIAL_sout("FLASH STS = ", 12);
			vSERIAL_HB8out(ucData);
			vSERIAL_sout("     (Mode= ", 12);
			vSERIAL_HB8out(uc);
			vSERIAL_bout(')');
			vSERIAL_crlf();

			vFLASH_quit();

			vDELAY_wait100usTic(5000);

			}// END: for(ucc)

		}// END: for(uc)


	}
	#endif // FLASH1_DIAG_ENA




	#ifdef FLASH2_DIAG_ENA
	{
	/***********************  FLASH2 diagnostic  *******************************
	*
	*
	*
	*
	**************************************************************************/

	int ii;
	uchar ucData;

	vSERIAL_sout("FLASH2: Dump a buffer\r\n", 23);

	while(TRUE)							//lint !e716 !e774
		{
		// INIT THE FLASH
		vFLASH_init(0);						//init FLASH to mode 0

		// LOAD THE BUFFER FROM PAGE
		vFLASH_read_mem_to_buff(0,4095);

		// SHOW A GROUP OF BYTES FROM BUFFER
		vSERIAL_sout("FLASH Data= ", 12);
		for(ii=0; ii<20;  ii++)
			{
			// READ THE DATA
			ucData = ucFLASH_read_byte_from_buff(0, ii);

			// SHOW THE DATA
			vSERIAL_HB8out(ucData);
			vSERIAL_bout(',');

			}// END: for(ucc)
		vSERIAL_crlf();

		// TURN OFF THE FLASH
		vFLASH_quit();

		// GIVE IT A REST
		vDELAY_wait100usTic(5000);

		}// END: while()


	}
	#endif // FLASH2_DIAG_ENA







	#ifdef FLASH3_DIAG_ENA
	{
	/***********************  FLASH3 diagnostic  *******************************
	*
	*
	*
	*
	**************************************************************************/

	int ii;
	uchar ucData;
	int iPage;

	vSERIAL_sout("FLASH3: Write a buffer & dump it\r\n", 34);

	iPage = 30;
	while(TRUE)							//lint !e716 !e774
		{
		// INIT THE FLASH
		vFLASH_init(0);						//init FLASH to mode 0

		// WRITE THE BUFFER
		for(ii=0; ii<528;  ii++)
			{
			vFLASH_write_byte_to_buff(0,ii,((uchar) (ii&0xFF)));
			}

		// WRITE BUFFER TO MEM
		vFLASH_write_buff_to_mem(0,iPage);

		// LOAD THE BUFFER FROM PAGE
		vFLASH_read_mem_to_buff(0,iPage);

		// SHOW A GROUP OF BYTES FROM BUFFER
		vSERIAL_sout("FLASH Data= ", 12);
		for(ii=0; ii<20;  ii++)
			{
			// READ THE DATA
			ucData = ucFLASH_read_byte_from_buff(0, ii);

			// SHOW THE DATA
			vSERIAL_HB8out(ucData);
			vSERIAL_bout(',');

			}// END: for(ucc)
		vSERIAL_crlf();

		// TURN OFF THE FLASH
		vFLASH_quit();

		// GIVE IT A REST
		vDELAY_wait100usTic(5000);

		}// END: while()


	}
	#endif // FLASH3_DIAG_ENA






	#ifdef FLASH4_DIAG_ENA
	{
	/***********************  FLASH4 diagnostic  *******************************
	*
	*
	*
	*
	**************************************************************************/

	int ii;
	int jj;
	uint uiData;
	int iPage;

	vSERIAL_sout("FLASH4: Write an INT buffer & dump it\r\n", 39);

	iPage = 30;
	while(TRUE)							//lint !e716 !e774
		{
		// INIT THE FLASH
		vFLASH_init(0);						//init FLASH to mode 0

		// WRITE THE BUFFER
		for(ii=0,jj=0; ii<528;  ii+=2,jj++)
			{
			vFLASH_write_word_to_buff(0,ii,(uint)jj);
			}

		// WRITE BUFFER TO MEM
		vFLASH_write_buff_to_mem(0,iPage);

		// LOAD THE BUFFER FROM PAGE
		vFLASH_read_mem_to_buff(0,iPage);

		// SHOW A GROUP OF BYTES FROM BUFFER
		vSERIAL_sout("FLASH Data= ", 12);
		for(ii=0; ii<20;  ii+=2)
			{
			// READ THE DATA
			uiData = uiFLASH_read_word_from_buff(0, ii);

			// SHOW THE DATA
			vSERIAL_HB16out(uiData);
			vSERIAL_bout(',');

			}// END: for(ucc)
		vSERIAL_crlf();

		// TURN OFF THE FLASH
		vFLASH_quit();

		// GIVE IT A REST
		vDELAY_wait100usTic(5000);

		}// END: while()


	}
	#endif // FLASH4_DIAG_ENA







	#ifdef FLASH5_DIAG_ENA
	{
	/***********************  FLASH5 diagnostic  *******************************
	*
	*
	*
	*
	**************************************************************************/
	int ii;

	vSERIAL_sout("FLASH5: auto-rewrite the disk\r\n", 31);

	// INIT THE FLASH
	vFLASH_init(0);						//init FLASH to mode 0

	vSERIAL_sout("Starting....\r\n", 13);

	for(ii=30;  ii<4096;  ii++)
		{
		vSERIAL_bout('\r');
		vSERIAL_I16out(ii);
		vFLASH_auto_rewrite_single_page(0,ii);
		}// END: for()

	vFLASH_quit();						//init FLASH to mode 0

	vSERIAL_sout("done\r\n", 6);

	}
	#endif // FLASH5_DIAG_ENA









	#ifdef FLASH6_DIAG_ENA
	{
	/***********************  FLASH6 diagnostic  *******************************
	*
	*
	*
	*
	**************************************************************************/
	uint uii;
	uint ujj;
	uchar ucData;
	uint uiPage;
	uchar ucFlashOK;

//	#define POWER_SD_FOR_THIS_TEST YES
	#ifndef POWER_SD_FOR_THIS_TEST
	  #define POWER_SD_FOR_THIS_TEST NOPE
	#endif


	vSERIAL_sout("FLASH6: flash buffer & mem content chk\r\n", 40);


	uiPage = 0x30;
	while(TRUE)							//lint !e716 !e774
		{
		ucFlashOK = TRUE;

		#if (POWER_SD_FOR_THIS_TEST == YES)
		vSERIAL_sout("SD powered UP for brain SPI work\r\n", 34);
		if(!ucSDCOM_power_up_SD_for_brain_SPI_work())
			{
			vSERIAL_sout("FLASH: powerup SD for brain work failed\r\n", 41);
			}
		#else
		vSERIAL_sout("SD not powered for this test\r\n", 30);
		#endif

		// INIT THE FLASH
		ucFLASH_init(0);			//lint !e534			//init FLASH to mode 0

		// CLR BUFFER AND DEST MEM PAGE
		for(uii=0; uii<528;  uii++)
			{
			vFLASH_write_byte_to_buff(FLASH_BUFFER_1,uii,0x55);
			}
		vFLASH_write_buff_to_mem(FLASH_BUFFER_1,uiPage);


		// CLR USER TO BUFFER
		for(uii=0; uii<528;  uii++)
			{
			vFLASH_write_byte_to_buff(FLASH_BUFFER_1,uii,0);
			}



		// WRITE TEST DATA TO BUFFER
		for(uii=0; uii<528;  uii++)
			{
			vFLASH_write_byte_to_buff(FLASH_BUFFER_1,uii,((uchar) (uii&0xFF)));
			}

		// CHK THE TEST DATA IN THE BUFFER
		for(uii=0; uii<528;  uii++)
			{
			// CHECK THE BYTE IN MEM
			ucData = ucFLASH_read_byte_from_buff(FLASH_BUFFER_1, uii);
			if(ucData != (uchar) (uii&0xFF))
				{
				vSERIAL_sout("FLASH Fail (BUFF WRITE) at Page ", 32);
				vSERIAL_HB16out((uint)uiPage);
				vSERIAL_sout(" loc ", 5);
				vSERIAL_HB16out((uint)uii);
				vSERIAL_sout(" wrote ", 7);
				vSERIAL_HB8out((uchar)(uii&0xFF));
				vSERIAL_sout(" was ", 5);
				vSERIAL_HB8out(ucData);
				vSERIAL_crlf();

				vBUZ_raygun_down();			//bad

				ucFlashOK = FALSE;

				}// END: if()

			}// for(uii)


		// WRITE TEST BUFFER TO MEM
		vFLASH_write_buff_to_mem(FLASH_BUFFER_1,uiPage);

		// CLR BUFFER AREA OF RESIDUAL DATA
		for(uii=0; uii<528;  uii++)
			{
			vFLASH_write_byte_to_buff(FLASH_BUFFER_1, uii, 0);
			}

		// READ TEST DATA BACK TO BUFFER
		vFLASH_read_mem_to_buff(FLASH_BUFFER_1,  uiPage);

		// CHECK THE TEST DATA
		for(uii=0; uii<528;  uii++)
			{
			// CHECK THE BYTE IN MEM
			ucData = ucFLASH_read_byte_from_buff(FLASH_BUFFER_1, uii);
			if(ucData != (uchar) (uii&0xFF))
				{
				vSERIAL_sout("FLASH Fail (MEM WRITE) at Page ", 31);
				vSERIAL_HB16out((uint)uiPage);
				vSERIAL_sout(" loc ", 5);
				vSERIAL_HB16out((uint)uii);
				vSERIAL_sout(" wrote ", 7);
				vSERIAL_HB8out((uchar)(uii&0xFF));
				vSERIAL_sout(" was ", 5);
				vSERIAL_HB8out(ucData);
				vSERIAL_crlf();

				vBUZ_raspberry();			//bad

				ucFlashOK = FALSE;

				// SHOW THE FIRST 20 LOCATION
				vSERIAL_sout("Page Dump=", 10);
				for(ujj=0; ujj<20;  ujj++)
					{
					ucData = ucFLASH_read_byte_from_buff(FLASH_BUFFER_1, ujj);
					vSERIAL_HB8out(ucData);

					}// END: for(jj)
				vSERIAL_crlf();


				}// END: if()

			}// for(uii)


		// TURN OFF THE FLASH
		vFLASH_quit();

		#if (POWER_SD_FOR_THIS_TEST == YES)
		vSDCOM_power_down_SD_from_brain_SPI_work();
		#endif

		// REPORT IF THE DIAGNOSTIC PASSED
		if(ucFlashOK)
			{
			vSERIAL_sout("Sector passed test\r\n", 20);
			vBUZ_test_successful();
			}


		}// END: while()

	}
	#endif // END: FLASH6_DIAG_ENA






	#ifdef FLASH7_DIAG_ENA
	{
	/***********************  FLASH7 diagnostic  *******************************
	*
	* Dump messages from the flash
	*
	*
	**************************************************************************/

	uchar ucii;
	uchar ucjj;
	ulong ulSavedFlashReadPtr;

	vSERIAL_sout("FLASH7: Dump msgs from flash\r\n", 30);


	// SAVE THE LINEAR FLASH POINTER
	ulSavedFlashReadPtr = (ulong)lL2FRAM_getLinearFlashPtr(
							FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR
							);

	while(TRUE)							//lint !e716 !e774
		{

		for(ucii=0;  ucii<20;  ucii++)
			{
			// LOAD A MSG FROM FLASH
			vL2FLASH_getNextMsgFromFlash();

			// SHOW THE PACKET
//			vSERIAL_HB8out(ucii);
			vSERIAL_bout(179);
			for(ucjj=0;
				ucjj<=(ucaMSG_BUFF[GMH_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK);
				ucjj++
			   )
				{
				if(ucaMSG_BUFF[ucjj] == 0x78) vSERIAL_bout('?');
				vSERIAL_HB8out(ucaMSG_BUFF[ucjj]);
				vSERIAL_bout(',');

				}// END: for(ucjj)

			vSERIAL_crlf();

			}// END: for(ucii)

		// RESTORE THE READ POINTER
		vL2FRAM_putLinearFlashPtr(
					FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR,
					(long)ulSavedFlashReadPtr
					);

		vSERIAL_any_char_to_cont();

		}// END: while()

	}
	#endif // END: FLASH7_DIAG_ENA







	#ifdef FLASH8_DIAG_ENA
	{
	#include "sdctl.h"		//SD board control routines

	/***********************  FLASH8 diagnostic  *******************************
	*
	* Routine to test the effects of variable power on flash writes
	*
	**************************************************************************/
	uint uii;
	uint ujj;
	uchar ucData;
	uint uiPage;
	uchar ucFlashOK;
	uchar ucTmp;
	ulong ulii;

	vSERIAL_sout("FLASH8: tst flash at different power\r\n", 38);

	while(TRUE)	//lint !e774
		{
		ucTmp = ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_DISABLED);
		if(ucTmp != 1)
			{
			vSERIAL_sout("Err on SD startup\r\n", 19);
			continue;
			}

		// INIT THE FLASH
		ucTmp = ucFLASH_init();

		break;

		}// END: while()

	uiPage = 0x30;
	for(ulii=0;  ;  ulii++)
		{
		ucFlashOK = TRUE;

		// SET BUFFER AND MEMORY TO FIXED VALUE
		for(uii=0; uii<528;  uii++)
			{
			vFLASH_write_byte_to_buff(FLASH_BUFFER_1,uii,0x55);
			}
		vFLASH_write_buff_to_mem(FLASH_BUFFER_1,uiPage);

		// CHECK THE BUFFER TO MAKE SURE IT HAPPENED
		if(!ucDIAG_check_flash_buffer_for_val(uiPage, FLASH_BUFFER_1, 0x55))
			{
			vSERIAL_sout("Bad write of buffer (Test 1)\r\n", 30);
			vBUZ_raygun_down();			//bad
			continue;
			}

		// CLR BUFFER
		for(uii=0; uii<528;  uii++)
			{
			vFLASH_write_byte_to_buff(FLASH_BUFFER_1,uii,0);
			}
		// CHK CLR TO MAKE SURE IT HAPPENED
		if(!ucDIAG_check_flash_buffer_for_val(uiPage, FLASH_BUFFER_1, 0x00))
			{
			vSERIAL_sout("Bad write of buffer (Test 2)\r\n", 30);
			vBUZ_raygun_down();			//bad
			continue;
			}


		// WRITE TEST DATA TO BUFFER
		for(uii=0; uii<528;  uii++)
			{
			vFLASH_write_byte_to_buff(FLASH_BUFFER_1,uii,((uchar) (uii&0xFF)));
			}

		// CHECK BUFFER TO MAKE SURE ITS SET == TEST DATA
		for(uii=0; uii<528;  uii++)
			{
			// CHECK THE BYTE IN MEM
			ucData = ucFLASH_read_byte_from_buff(FLASH_BUFFER_1, uii);
			if(ucData != (uchar) (uii&0xFF))
				{
				vSERIAL_sout("FLASH Fail (BUFF WRITE) at Page ", 32);
				vSERIAL_HB16out((uint)uiPage);
				vSERIAL_sout(" loc ", 5);
				vSERIAL_HB16out((uint)uii);
				vSERIAL_sout(" wrote ", 7);
				vSERIAL_HB8out((uchar)(uii&0xFF));
				vSERIAL_sout(" was ", 5);
				vSERIAL_HB8out(ucData);
				vSERIAL_crlf();

				vBUZ_raygun_down();			//bad

				ucFlashOK = FALSE;

				}// END: if()

			}// for(uii)


		// WRITE TEST BUFFER TO MEM
		vFLASH_write_buff_to_mem(FLASH_BUFFER_1,uiPage);


		// CLR BUFFER AREA OF RESIDUAL DATA
		for(uii=0; uii<528;  uii++)
			{
			vFLASH_write_byte_to_buff(FLASH_BUFFER_1, uii, 0);
			}
		// CHK CLR TO MAKE SURE IT HAPPENED
		if(!ucDIAG_check_flash_buffer_for_val(uiPage, FLASH_BUFFER_1, 0x00))
			{
			vSERIAL_sout("Bad write of buffer (Test 3)\r\n", 30);
			vBUZ_raygun_down();			//bad
			continue;
			}



		// READ TEST DATA BACK TO BUFFER
		vFLASH_read_mem_to_buff(FLASH_BUFFER_1,  uiPage);

		// CHECK THE TEST DATA
		for(uii=0; uii<528;  uii++)
			{
			// CHECK THE BYTE IN MEM
			ucData = ucFLASH_read_byte_from_buff(FLASH_BUFFER_1, uii);
			if(ucData != (uchar) (uii&0xFF))
				{
				vSERIAL_sout("FLASH Fail (MEM WRITE) at Page ", 31);
				vSERIAL_HB16out((uint)uiPage);
				vSERIAL_sout(" loc ", 5);
				vSERIAL_HB16out(uii);
				vSERIAL_sout(" wrote ", 7);
				vSERIAL_HB8out((uchar)(uii&0xFF));
				vSERIAL_sout(" was ", 5);
				vSERIAL_HB8out(ucData);
				vSERIAL_crlf();

				vBUZ_raspberry();			//bad

				ucFlashOK = FALSE;

				// SHOW THE FIRST 20 LOCATION
				vSERIAL_sout("Page Dump=", 10);
				for(ujj=0; ujj<20;  ujj++)
					{
					ucData = ucFLASH_read_byte_from_buff(FLASH_BUFFER_1, ujj);
					vSERIAL_HB8out(ucData);

					}// END: for(jj)
				vSERIAL_crlf();


				}// END: if()

			}// for(uii)


		// REPORT IF THE DIAGNOSTIC PASSED
		if(ucFlashOK)
			{
			vSERIAL_UIV32out(ulii);
			vSERIAL_sout(": Sec WR OK\r\n", 13);
			}


		}// END: while()

	}
	#endif // END: FLASH8_DIAG_ENA








	#ifdef FRAM1_DIAG_ENA
	{
	/***********************  FRAM1 diagnostic  *******************************
	*
	*
	*
	*
	**************************************************************************/

	uchar ucData;

	vSERIAL_sout("FRAM1: Test simple transaction\r\n", 32);

//	PORTFbits.RF6 = 0;						//debug

	vFRAM_init();							//init fram

//	PORTFbits.RF6 = 1;						//debug

	while(TRUE)
		{
		if(!ucSDCOM_power_up_SD_for_brain_SPI_work())
			{
			vSERIAL_sout("Error powering up SD for SPI work\r\n", 35);
			continue;
			}
		break;
		}

	/********  READ THE STATUS REGISTER  *************************************/
	ucData = ucFRAM_read_sts_reg();

	// SHOW THE STATUS
	vSERIAL_sout("STATUS = ", 9);
	vSERIAL_HB8out(ucData);
	vSERIAL_crlf();

	/********  SEND A FRAM WRITE ENABLE COMMAND *******************************/
	vFRAM_send_WE_cmd();

	/********  READ THE STATUS REGISTER  *************************************/
	ucData = ucFRAM_read_sts_reg();

	// SHOW THE STATUS
	vSERIAL_sout("STATUS = ", 9);
	vSERIAL_HB8out(ucData);
	vSERIAL_crlf();

	/********  SEND A FRAM WRITE CMD  *****************************************/
	vFRAM_write_data(0x0020, 0x55);


	/********  SEND A FRAM READ CMD  *****************************************/
	ucData = ucFRAM_read_data(0x0020);

	// SHOW THE DATA
	vSERIAL_sout("Data Read = ", 12);
	vSERIAL_HB8out(ucData);
	vSERIAL_crlf();

	vSDCOM_power_down_SD_from_brain_SPI_work();

	}
	#endif // FRAM1_DIAG_ENA






	#ifdef FRAM2_DIAG_ENA

	/*************  FRAM2 diagnostic  ****************************************
	*
	* TEST THE FRAM ROUTINES AND HARDWARE
	*
	* FULL MEMORY TEST
	*
	**************************************************************************/
	{
	uint uiAddr;
	uchar ucWriteData;
	uchar ucReadData;
	uchar ucShouldBe;
	uchar uc;

	#define STARTER_CODE 0x80

//	#define MAX_MEM_SIZE 256
	#define MAX_MEM_SIZE 512
//	#define MAX_MEM_SIZE 4096
//	#define MAX_MEM_SIZE 8192



	// SHOW HEADER MESSAGE
	vSERIAL_sout("FRAM2: Full mem test of FRAM\r\n", 30);

#if 1
	vSERIAL_sout("FRAM2: SD is powered up for brain SPI work\r\n", 44);
	while(TRUE)	//lint !e774
		{
		if(!ucSDCOM_power_up_SD_for_brain_SPI_work())
			{
			vSERIAL_sout("Error powering up SD for SPI work\r\n", 35);
			continue;
			}
		break;
		}
#else
	vSERIAL_sout("FRAM2: SD is powered down\r\n", 27);
#endif

	vSERIAL_sout("Running memory test\r\n", 21);


	// SETUP THE FRAM MEM
	vFRAM_init();

	// LOOP WRITING AND THEN READING THE FRAM MEMORY
	for(uc=0;  ;  uc++)
		{

		// FILL THE MEMORY
		vSERIAL_sout("\r\nWriting FRAM\r\n", 16);
		ucWriteData = STARTER_CODE;

		for(uiAddr=0;  uiAddr<MAX_MEM_SIZE;  uiAddr++)
			{
			// CLEAR THE DATA FIRST*/
			vFRAM_write_B8(uiAddr, 0x00);

			// WRITE THE DATA
			vFRAM_write_B8(uiAddr, ucWriteData);

			// PUT OUT A PERIODIC NOTIFICATION TO USER
			if((uiAddr & 0x00FF) == 0)
				{
				vSERIAL_HB16out(uiAddr);
				vSERIAL_bout('\r');
				}

			// BUMP THE DATA VALUE
			ucWriteData++;

			}// for()

		vSERIAL_crlf();



		// CHECK THE MEMORY
		vSERIAL_sout("Reading FRAM\r\n", 15);
		ucShouldBe = STARTER_CODE;
		for(uiAddr=0;  uiAddr<MAX_MEM_SIZE;  uiAddr++)
			{
			// PUT OUT A PERIODIC NOTIFICATION TO USER
			if((uiAddr & 0x00FF) == 0)
				{
				vSERIAL_HB16out(uiAddr);
				vSERIAL_bout('\r');
				}

			// CHECK BYTE IN MEMORY
			ucReadData = ucFRAM_read_B8(uiAddr);
			if(ucReadData != ucShouldBe)
				{
				vSERIAL_sout("FRAM Mem Fail at ", 17);
				vSERIAL_HB16out(uiAddr);
				vSERIAL_sout(" wrote ", 7);
				vSERIAL_HB8out(ucShouldBe);
				vSERIAL_sout(" was ", 5);
				vSERIAL_HB8out(ucReadData);
				vSERIAL_crlf();

				vSERIAL_sout("Any char to continue...", 23);
				ucSERIAL_bin();								//lint !e534
				vSERIAL_crlf();
				}


			ucShouldBe++;

			}// for()

		vSERIAL_crlf();

		}// END: for(;;)

	vSDCOM_power_down_SD_from_brain_SPI_work();

	}
	#endif // END: FRAM2_DIAG_ENA








	#ifdef FRAM3_DIAG_ENA

	/*************  FRAM3 diagnostic  ****************************************
	*
	* TEST THE FRAM ROUTINES AND HARDWARE
	*
	* TEST A SINGLE MEMORY LOCATION
	*
	**************************************************************************/
	{
//	uint uiAddr;
//	uchar ucWriteData;
	uchar ucReadData;
//	uchar ucShouldBe;
	uchar uc;

	#define SINGLE_LOCATION_ADDR 0x100
	#define STARTER_CODE 0x80

//	#define MAX_MEM_SIZE 256
	#define MAX_MEM_SIZE 512
//	#define MAX_MEM_SIZE 4096
//	#define MAX_MEM_SIZE 8192



	// SHOW HEADER MESSAGE
	vSERIAL_sout("FRAM3: Single location test of FRAM\r\n", 37);

#if 0
	vSERIAL_sout("SD is powered up for brain SPI work\r\n", 37);
	while(TRUE)	//lint !e774
		{
		if(!ucSDCOM_power_up_SD_for_brain_SPI_work())
			{
			vSERIAL_sout("Error powering up SD for SPI work\r\n", 35);
			continue;
			}
		break;
		}
#else
	vSERIAL_sout("SD is powered down\r\n", 20);
#endif

	vSERIAL_sout("Running memory test\r\n", 21);


	// SETUP THE FRAM MEM
	vFRAM_init();

	// LOOP WRITING AND THEN READING THE FRAM MEMORY
	for(uc=0;  ;  uc++)
		{
		// GIVE A PERIODIC INDICATOR
		if(uc == 0) vSERIAL_bout('.');

		// WRITE THE DATA NOT
		vFRAM_write_B8(SINGLE_LOCATION_ADDR, ~STARTER_CODE);

		// WRITE THE DATA
		vFRAM_write_B8(SINGLE_LOCATION_ADDR, STARTER_CODE);

		// READ THE DATA
		ucReadData = ucFRAM_read_B8(SINGLE_LOCATION_ADDR);

		// CHECK BYTE IN MEMORY
		if(ucReadData != STARTER_CODE)
			{
			vSERIAL_sout("FRAM Mem Fail at ", 17);
			vSERIAL_HB16out(SINGLE_LOCATION_ADDR);
			vSERIAL_sout(" wrote= ", 8);
			vSERIAL_HB8out(STARTER_CODE);
			vSERIAL_sout(" was= ", 6);
			vSERIAL_HB8out(ucReadData);
			vSERIAL_sout(" xor= ", 6);
			vSERIAL_HB8out(STARTER_CODE ^ ucReadData);
			vSERIAL_crlf();


//			vSERIAL_sout("Any char to continue...", 23);
//			ucSERIAL_bin();								//lint !e534
//			vSERIAL_crlf();
			}

		}// END: for(;;)

	vSDCOM_power_down_SD_from_brain_SPI_work();

	}
	#endif // END: FRAM3_DIAG_ENA







	#ifdef FRAM4_DIAG_ENA

	/*************  FRAM4 diagnostic  ****************************************
	*
	* TEST THE L2FRAM routines
	*
	**************************************************************************/
	{
//	uchar ucVal;
	uint uiVal;
	USL uslVal;
	unsigned long ulVal;


	// SHOW HEADER MESSAGE
	vSERIAL_sout("FRAM4: Test L2FRAM routines\r\n", 29);



	// SETUP THE FRAM MEM
	vFRAM_init();


#if 1
	vSERIAL_sout("SD is powered up for brain SPI work\r\n", 37);
	while(TRUE)	//lint !e774
		{
		if(!ucSDCOM_power_up_SD_for_brain_SPI_work())
			{
			vSERIAL_sout("Error powering up SD for SPI work\r\n", 35);
			continue;
			}
		break;
		}
#else
	vSERIAL_sout("SD is powered down\r\n", 20);
#endif


	// LOOP WRITING AND THEN READING THE FRAM MEMORY
	while(TRUE)	//lint !e774
		{
		vSERIAL_sout("Starting ......  initial layout follows\r\n", 41);

		// SHOW THE FRAM LAYOUT FIRST
		vSERIAL_sout("Clearing FRAM to 0x55\r\n", 23);
		vL2FRAM_set_fram_blk(0,24,0x55);
		vL2FRAM_show_fram(0, 24);
		vL2FRAM_chk_fram_blk(0,24,0x55);
		vSERIAL_any_char_to_cont();	//wait


		vSERIAL_sout("Settomg 4 bytes at Addr=6 Val=0x66666666\r\n", 42);
		vL2FRAM_set_fram_blk(6,4,0x66);
		vL2FRAM_show_fram(0,24);
		vL2FRAM_chk_fram_blk(6,4,0x66);
		vSERIAL_any_char_to_cont();	//wait


		vSERIAL_sout("Writing B16 Addr=6 Val=1122\r\n", 49);
		vFRAM_write_B16(6,0x1122);
		vL2FRAM_show_fram(0,24);
		uiVal = uiFRAM_read_B16(6);
		if(uiVal != 0x1122)
			{
			vSERIAL_sout("FRAM B16 fail ", 14);
			vSERIAL_sout(" wrote= 0x", 10);
			vSERIAL_HB16out(0x1122);
			vSERIAL_sout(" was= 0x", 8);
			vSERIAL_HB16out(uiVal);
			vSERIAL_sout(" xor= 0x", 8);
			vSERIAL_HB16out(0x1122 ^ uiVal);
			vSERIAL_crlf();
			// SHOW IT A 2ND TIME
			vL2FRAM_show_fram(0,24);
			}
		else
			{
			vSERIAL_sout("B16 test passed\r\n", 17);
			}
		vSERIAL_any_char_to_cont();	//wait


		vSERIAL_sout("Clearing B32 Addr=6 Val=0x55555555\r\n", 36);
		vL2FRAM_set_fram_blk(6,4,0x55);
		vL2FRAM_show_fram(0,24);
		vSERIAL_any_char_to_cont();	//wait

		vSERIAL_sout("Writing B32 Addr=6 Val=44332211\r\n", 33);
		vFRAM_write_B32(6,0x44332211);
		vL2FRAM_show_fram(0,24);
		ulVal = ulFRAM_read_B32(6);
		if(ulVal != 0x44332211)
			{
			vSERIAL_sout("FRAM B32 fail ", 14);
			vSERIAL_sout(" wrote= 0x", 10);
			vSERIAL_HB32out(0x44332211);
			vSERIAL_sout(" read= 0x", 8);
			vSERIAL_HB32out(ulVal);
			vSERIAL_sout(" xor= 0x", 8);
			vSERIAL_HB32out(0x44332211 ^ uslVal);
			vSERIAL_crlf();
			// SHOW IT A 2ND TIME
			vL2FRAM_show_fram(0,24);
			}
		else
			{
			vSERIAL_sout("B32 test passed\r\n", 17);
			}
		vSERIAL_any_char_to_cont();	//wait





		#if 0
		// TEST THE ROUTINE THAT CHECKS FOR FUNCTIONING FRAM
		vSERIAL_sout("checking test_for_fram()\r\n", 26);
		if(ucL2FRAM_chk_for_fram(FRAM_CHK_REPORT_MODE))
			{
			vSERIAL_sout("FRAM tests good\r\n", 17);
			}
		else
			{
			vSERIAL_sout("FRAM tests bad\r\n", 16);
			}

		// SHOW THE FRAM LAYOUT
		vL2FRAM_show_fram(0, 24);
		vSERIAL_any_char_to_cont();	//wait

		// DO AN UNFORMAT
		vSERIAL_sout("Unformatting FRAM\r\n", 19);

		// SHOW THE FRAM LAYOUT
		vL2FRAM_show_fram(0, 24);
		vSERIAL_any_char_to_cont();	//wait


		// CHECK THAT WE ARE UNFORMATTED
		vSERIAL_sout("Checking FRAM format\r\n");
		uiVal = uiL2FRAM_chk_for_fram_format();
		if(uiVal == 0)
			{
			vSERIAL_sout("FRAM is unformatted -- this is correct\r\n", 40);
			}
		else
			{
			vSERIAL_sout("FRAM is formatted -- this is an error\r\n", 39);
			}
		vSERIAL_any_char_to_cont();	//wait



		// NOW FORMAT THE FRAM
		vSERIAL_sout("Formatting FRAM \r\n", 18);
		// SHOW THE FRAM LAYOUT
		vL2FRAM_show_fram(0, 24);
		vSERIAL_any_char_to_cont();	//wait
		vL2FRAM_format_fram();
		vL2FRAM_show_fram(0, 24);
		vSERIAL_any_char_to_cont();	//wait








		// CHECK THAT WE ARE FORMATTED
		vSERIAL_sout("Checking FRAM format\r\n", 22);
		uiVal = uiL2FRAM_chk_for_fram_format();
		if(uiVal == 0)
			{
			vSERIAL_sout("FRAM is unformatted -- this is an error\r\n", 41);
			}
		else
			{
			vSERIAL_sout("FRAM is formatted -- this is good\r\n", 35);
			}
		vSERIAL_any_char_to_cont();	//wait



		// INCREMENT THE FLASH REFRESH PTR
		vSERIAL_sout("Incrementing refresh ptr\r\n", 26);
		vL2FRAM_show_fram(0, 24);
		vL2FRAM_inc_flash_refresh_ptr();
		vL2FRAM_show_fram(0, 24);
		vSERIAL_any_char_to_cont();	//wait


		vSERIAL_sout("Incrementing flash ON ptr +32\r\n", 31);
		vL2FRAM_show_fram(0, 24);
		vL2FRAM_inc_flash_ptr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE,32);
		vL2FRAM_show_fram(0, 24);
		vSERIAL_any_char_to_cont();	//wait

		vSERIAL_sout("Incrementing flash OFF ptr +32\r\n", 32);
		vL2FRAM_show_fram(0, 24);
		vL2FRAM_inc_flash_ptr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE,32);
		vL2FRAM_show_fram(0, 24);
		vSERIAL_any_char_to_cont();	//wait


		vSERIAL_sout("Incrementing flash READOUT ptr +32\r\n", 36);
		vL2FRAM_show_fram(0, 24);
		vL2FRAM_inc_flash_ptr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE, 32);
		vL2FRAM_show_fram(0, 24);
		vSERIAL_any_char_to_cont();	//wait

		#endif


		}// END: while(TRUE)

	vFRAM_quit();		//lint !e527

	}
	#endif // END: FRAM4_DIAG_ENA








	#ifdef FRAM5_DIAG_ENA

	/*************  FRAM5 diagnostic  ****************************************
	*
	* Special test to find the first bad byte problem
	*
	**************************************************************************/
	{

	// SHOW HEADER MESSAGE
	vSERIAL_sout("FRAM5: Find first bad byte \r\n", 29);
	vSERIAL_any_char_to_cont();	//wait

	// SETUP THE FRAM MEM
	vFRAM_init();

	// SHOW THE FRAM LAYOUT FIRST
	vSERIAL_sout("Clearing FRAM\r\n", 15);
	vL2FRAM_set_fram_blk(0,10,0x11);
	vL2FRAM_show_fram(0, 10);
	vSERIAL_any_char_to_cont();	//wait

	vL2FRAM_set_fram_blk(3,5,0x22);
	vL2FRAM_show_fram(0, 10);

	ucSERIAL_bin();					//lint !e534 //this call causes failure

	vL2FRAM_set_fram_blk(3,5,0x22);
	vL2FRAM_show_fram(0, 10);
	vSERIAL_any_char_to_cont();	//wait

	vL2FRAM_set_fram_blk(3,5,0x33);
	vL2FRAM_show_fram(0, 10);
	vSERIAL_any_char_to_cont();	//wait

	vFRAM_quit();		//lint !e527

	vSERIAL_sout("Exit\r\n", 6);

Fred:
	goto Fred;

	}
	#endif // END: FRAM5_DIAG_ENA







	#ifdef FRAM6_DIAG_ENA

	/*************  FRAM6 diagnostic  ****************************************
	*
	* Special test to find the first bad byte problem
	*
	**************************************************************************/
	{
	unsigned long ulii;
	unsigned long ulMsgCount;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("FRAM6: Hunt for upload counter bug \r\n", 37);


	while(TRUE)		//lint !e774
		{
		vSERIAL_any_char_to_cont();	//wait

		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();

		// ERASE ALL READ MESSAGES
		vSERIAL_sout("Erasing all read messages\r\n", 27);
		vL2FRAM_expungeAllUploadedFlashMsgs();

		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();

		// STUFF SOME MESSAGES
		vSERIAL_sout("Stuffing 3 msgs\r\n", 17);
		vL2FRAM_incFlashOnPtr();
		vL2FRAM_incFlashOnPtr();
		vL2FRAM_incFlashOnPtr();

		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();

		// FAKE AN UPLOAD
		vSERIAL_sout("Unloading all messages\r\n"24);
		ulMsgCount = (unsigned long)lL2FRAM_getFlashUnreadMsgCount();
		for(ulii=0; ulii<ulMsgCount; ulii++)
			{
			vL2FRAM_incFlashReadPtr();
			}

		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();

		}// END: while(TRUE)

	vSERIAL_sout("Exit\r\n", 6);

Fred:
	goto Fred;

	}
	#endif // END: FRAM6_DIAG_ENA







	#ifdef FRAM7_DIAG_ENA

	/*************  FRAM7 diagnostic  ****************************************
	*
	* Special test to find the first bad byte problem
	*
	**************************************************************************/
	{
	unsigned long ulii;
	unsigned long ulMsgCount;

	#define FLASH_TEST_LINEAR_ADDR_L ((4095L << 9) | 0x1C0L)

	// SHOW HEADER MESSAGE
	vSERIAL_sout("FRAM7: Hunt for upload counter bug \r\n", 37);


	while(TRUE)		//lint !e774
		{
		vSERIAL_any_char_to_cont();	//wait

		// ERASE ALL READ MESSAGES
		vSERIAL_sout("Erasing all read messages\r\n", 27);
		vL2FRAM_expungeAllUploadedFlashMsgs();

		vSERIAL_sout("FlashOnPtr = ", 13);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashReadoutPtr = ", 18);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashOffPtr = ", 14);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE));
		vSERIAL_crlf();

		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();

		// SETUP ALL THE PTRS
		vSERIAL_sout("\r\nSetting up pointers\r\n", 23);

		vL2FRAM_putLinearFlashPtr(
				FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE,
				FLASH_TEST_LINEAR_ADDR_L
				);

		vL2FRAM_putLinearFlashPtr(
				FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE,
				FLASH_TEST_LINEAR_ADDR_L
				);

		vL2FRAM_putLinearFlashPtr(
				FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE,
				FLASH_TEST_LINEAR_ADDR_L
				);

		vSERIAL_sout("FlashOnPtr = ", 13);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashReadoutPtr = ", 18);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashOffPtr = ", 14);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE));
		vSERIAL_crlf();


		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();

		// STUFF 3 MESSAGES
 		vSERIAL_sout("Stuffing msg 1\r\n", 16);
		vL2FRAM_incFlashOnPtr();
		// SHOW THE POINTERS
		vSERIAL_sout("FlashOnPtr = ", 13);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashReadoutPtr = ", 18);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashOffPtr = ", 14);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE));
		vSERIAL_crlf();
		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();



 		vSERIAL_sout("Stuffing msg 2\r\n", 16);
		vL2FRAM_incFlashOnPtr();
		// SHOW THE POINTERS
		vSERIAL_sout("FlashOnPtr = ", 13);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashReadoutPtr = ", 18);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashOffPtr = ", 14);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE));
		vSERIAL_crlf();
		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();



 		vSERIAL_sout("Stuffing msg 3\r\n", 16);
		vL2FRAM_incFlashOnPtr();
		// SHOW THE POINTERS
		vSERIAL_sout("FlashOnPtr = ", 13);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashReadoutPtr = ", 18);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashOffPtr = ", 14);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE));
		vSERIAL_crlf();


		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();

		// FAKE AN UPLOAD
		vSERIAL_sout("Unloading all messages\r\n", 24);

		vL2FRAM_incFlashReadPtr();

		vSERIAL_sout("FlashOnPtr = ", 13);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashReadoutPtr = ", 18);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashOffPtr = ", 14);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE));
		vSERIAL_crlf();


		vL2FRAM_incFlashReadPtr();

		vSERIAL_sout("FlashOnPtr = ", 13);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashReadoutPtr = ", 18);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashOffPtr = ", 14);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE));
		vSERIAL_crlf();


		vL2FRAM_incFlashReadPtr();

		vSERIAL_sout("FlashOnPtr = ", 13);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_ON_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashReadoutPtr = ", 18);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_READOUT_PTR_ADDR_BASE));
		vSERIAL_crlf();
		vSERIAL_sout("FlashOffPtr = ", 14);
		vL2FRAM_showLinearFlashPtr(lL2FRAM_getLinearFlashPtr(FRAM_FLSH_LINEAR_NFL_OFF_PTR_ADDR_BASE));
		vSERIAL_crlf();


		// SHOW THE COUNTS
		vSERIAL_sout("Old msgs= ", 10);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashReadMsgCount());
		vSERIAL_sout(" new msgs= ", 11);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashUnreadMsgCount());
		vSERIAL_sout(" free msgs= ", 12);
		vSERIAL_HBV32out((unsigned long)lL2FRAM_getFlashFreeMsgCount());
		vSERIAL_crlf();

		}// END: while(TRUE)

	vSERIAL_sout("Exit\r\n", 6);

Fred:
	goto Fred;

	}
	#endif // END: FRAM7_DIAG_ENA





	#ifdef KEY1_DIAG_ENA

	/************************  KEY1 diagnostic  **************************
	*
	*	KEY1 Test the CMD module routines
	*
	************************************************************************/
	{
	uchar cMchRet;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("KEY1: Test the CMD module routines\r\n", 36);


	while(TRUE) //lint !e774
		{
		vSERIAL_sout("\r\nCmd Input: ", 13);

		ucMchRet = ucNUMCMD_getCmdNum(
								&cpaCmdName[0],
								12,
								CASE_INSENSITIVE
								);

		vSERIAL_sout("Match ret = ", 12);
		vSERIAL_IV8out(cMchRet);
		vSERIAL_crlf();

		}// END: while() 

	}// END: TEST

	#endif			//KEY1_DIAG_ENA





	#ifdef KEY2_DIAG_ENA
	{
	#include "time.h"		//Time handling routines
	#include "key.h"		//keyboard routines

	/************************  KEY2 diagnostic  ******************************
	*
	*	KEY2 Test Timed input
	*
	**************************************************************************/

	uchar ucaDispatchBegTime[6];
	uchar ucaDispatchEndTime[6];
	uint uiChar;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("KEY2: Test Timed input\r\n", 24);


	while(TRUE) //lint !e774
		{
		vSERIAL_sout("Starting...\r\n", 13);

		// CAPTURE THE START TIME
		vTIME_copyWholeSysTimeToBytesInDtics( &ucaDispatchBegTime[0]);


		uiChar = uiSERIAL_timedBin(7000);
		if(uiChar < 256)
			{
			vSERIAL_sout("C=", 2);
			vSERIAL_bout((uchar)uiChar);
			vSERIAL_bout(',');
			continue;
			}

		vTIME_copyWholeSysTimeToBytesInDtics( &ucaDispatchEndTime[0]);

		vSERIAL_sout("Tout=", 5);
		vTIME_showWholeTimeDiffInDuS(
								&ucaDispatchBegTime[0],
								&ucaDispatchEndTime[0],
								YES_CRLF
								);


		vSERIAL_any_char_to_cont();

		}// END: while()

	}// END: TEST

	#endif			//KEY2_DIAG_ENA










	#ifdef BUZ2_DIAG_ENA
	{
	/***********************  BUZ2 diagnostic ********************************
	*
	* Routine to test the buzzer
	*
	**************************************************************************/

	vSERIAL_sout("BUZ2: BUZZER Test\r\n", 18);

	while(TRUE)
		{
		vBUZ(2100,5000);
		vDELAY_wait100usTic(5000);
		}

	}
	#endif // BUZ2_DIAG_ENA







	#ifdef BUZ3_DIAG_ENA
	{
	/***********************  BUZ3 diagnostic ********************************
	*
	* Routine to test the buzzer
	*
	**************************************************************************/

	vSERIAL_sout("BUZ3: BUZZER Test\r\n", 19);

	while(TRUE)	//lint !e716 !e774
		{
		#if 1
		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);
		vBUZ(NOTE_D,		WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);
		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);

		vDELAY_wait100usTic(WHOLE_NOTE);

		vBUZ(NOTE_E,		WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);

		vDELAY_wait100usTic(WHOLE_NOTE);

		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_A,	WHOLE_NOTE);
		vBUZ(NOTE_A,	WHOLE_NOTE);

		vDELAY_wait100usTic(WHOLE_NOTE);

		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);
		vBUZ(NOTE_D,		WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);
		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);

		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);

		vBUZ(NOTE_F_SHARP,	WHOLE_NOTE);
		vBUZ(NOTE_E,		WHOLE_NOTE);
		vBUZ(NOTE_D,		WHOLE_NOTE);

		vDELAY_wait100usTic(WHOLE_NOTE);
		#endif

		vSERIAL_sout("Any char to continue...", 23);
		ucSERIAL_bin();								//lint !e534
		vSERIAL_crlf();

		}

	}
	#endif // BUZ3_DIAG_ENA







	#ifdef BUZ4_DIAG_ENA
	{
	/***********************  BUZ4 diagnostic ********************************
	*
	* Routine to test the buzzer
	*
	**************************************************************************/

	vSERIAL_sout("BUZ4: Tune Routine Test\r\n", 25);
	vSERIAL_sout("HIT Key to move on, HIT button to repeat sound\r\n", 48);

	vDELAY_wait100usTic(10000);

	while(TRUE)			//lint !e774
		{

		vSERIAL_sout("BUZ4: buggs\r\n", 13);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_buggs();
			}


		vSERIAL_sout("BUZ4: test_successful\r\n", 23);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_test_successful();
			}


		vSERIAL_sout("BUZ4: scale_up_and_down\r\n", 25);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_scale_up_and_down();
			}


		vSERIAL_sout("BUZ4: scale_down\r\n", 18);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_scale_down();
			}


		vSERIAL_sout("BUZ4: raygun_down\r\n", 19);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_raygun_down();
			}


		vSERIAL_sout("BUZ4: raygun_up\r\n", 17);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_raygun_up();
			}


		vSERIAL_sout("BUZ4: raspberry\r\n", 17);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_raspberry();
			}


		vSERIAL_sout("BUZ4: morrie\r\n", 14);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_morrie();
			}

		vSERIAL_sout("BUZ4: entire note scale\r\n", 25);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_entire_note_scale();
			}

		vSERIAL_sout("BUZ4: bad news 3\r\n", 18);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_tune_bad_news_3();
			}

		vSERIAL_sout("BUZ4: bad news 2\r\n", 18);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_tune_bad_news_2();
			}

		vSERIAL_sout("BUZ4: bad news 1\r\n", 18);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_tune_bad_news_1();
			}

		vSERIAL_sout("BUZ4: Ta-Dah  Ta-Dah\r\n", 22);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_tune_TaDah_TaDah();
			}

		vSERIAL_sout("BUZ4: tune_imperial\r\n", 21);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_tune_imperial();
			}

		vSERIAL_sout("BUZ4: Mary had a little lamb\r\n", 20);
		while(TRUE)		//lint !e774
			{
			if(ucSERIAL_kbhit())
				{
				ucSERIAL_bin();		//lint !e534
				vSERIAL_crlf();
				break;
				}
			if(!ucBUTTON_buttonHit()) continue;
			vBUZ_tune_mary();
			}

		}

	}
	#endif // BUZ4_DIAG_ENA













	#ifdef COMM1_DIAG_ENA
	{
	/***********************  COMM1 diagnostic  *****************************
	*
	* Routine to test the ON/OFF switching on the com port
	*
	*
	*************************************************************************/

	vSERIAL_sout("COMM1: ON/OFF switching Test\r\n", 30);

	while(!ucBUTTON_buttonHit());		//lint !e722


	while(TRUE)							//lint !e716 !e774
		{
		vSERIAL_init();
		vDELAY_wait100usTic(10);
		vSERIAL_bout('[');

		vSERIAL_quit();
		vDELAY_wait100usTic(10);

		vSERIAL_init();
		vDELAY_wait100usTic(10);
		vSERIAL_bout(']');

		}

	}
	#endif // END: COMM1_DIAG_ENA





	#ifdef COMM2_DIAG_ENA
	{
	/***********************  COMM2 diagnostic  *****************************
	*
	* Routine to echo what it receives
	*
	*
	*************************************************************************/
	uchar ucChar;


	vSERIAL_sout("COMM2: ECHO TEST\r\n", 18);


	while(TRUE)							//lint !e716 !e774
		{
		ucChar = ucSERIAL_bin();
		vSERIAL_bout(ucChar);
		}


	}
	#endif // END: COMM2_DIAG_ENA








	#ifdef COMM4_DIAG_ENA

	/*********** TEST INTERRUPT OUTPUT ROUTINES ***************
	*
	*
	*****************************************************************/
	{
	uchar uc;
	uchar ucCount;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("COMM4: Test Serial Interrupts\r\n", 31);

	while(TRUE)
		{
		#if 0
		vSERIAL_bout('B');
		ucSERIAL_bin();
		vSERIAL_bout('I');
		ucSERIAL_bin();
		vSERIAL_bout('L');
		ucSERIAL_bin();
		vSERIAL_bout('L');
		#endif

		ucSERIAL_bin();


		vSERIAL_sout("123456789*123456789&123456789^123456789%123456789$123456789#\r\n", 62);
//		ucSERIAL_bin();
		vSERIAL_sout("abcdefghijklmnopqrstuvwxyz1234567890\r\n", 38);
//		ucSERIAL_bin();
		vSERIAL_sout("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890\r\n", 38);
//		ucSERIAL_bin();
		}

	}
	#endif // COMM4_DIAG_ENA





	#ifdef COMM5_DIAG_ENA

	/*********** TEST INTERRUPT OUTPUT ROUTINES ***************
	*
	*
	*****************************************************************/
	{
	uchar ucii;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("COMM5: Output all chars 0-255\r\n", 31);
	vSERIAL_setBinaryMode(BINARY_MODE_ON);
	vSERIAL_any_char_to_cont();

	while(TRUE) //lint !e774
		{
		for(ucii=0;  ;  ucii++)
			{
			vSERIAL_bout(ucii);
			if(ucii == 255) break;

			}// END: for(ucii)

		vSERIAL_any_char_to_cont();

		}// END: while()

	}
	#endif // COMM5_DIAG_ENA






	#ifdef COMM6_DIAG_ENA

	/**************** TEST THE RAD40 ROUTINES  *******************************
	*
	* Test the RAD40 conversions to string and back
	*
	*************************************************************************/
	{
	char cStr[6];
	uint uiRad40Val;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("COMM6: Test of Rad40 conversion\r\n", 33);

	while(TRUE)		//lint !e774
		{
		vSERIAL_sout("Input a 3 char str: ", 20);
		while(ucSERIAL_getEditedInput((uchar *)cStr,4)); //ling !e722
		vSERIAL_crlf();

		uiRad40Val = uiRAD40_ram_convertStrToRad40(cStr);

		vSERIAL_sout("Rad40 conversion in Hex= ", 25);
		vSERIAL_HB16out(uiRad40Val);
		vSERIAL_crlf();

		vSERIAL_sout("Rad40 to string= ", 17);
		vRAD40_showRad40(uiRad40Val);
		vSERIAL_crlf();

		}// END: while() 


	}
	#endif // COMM6_DIAG_ENA











	#ifdef AD1_DIAG_ENA
	{

	/*******  AD1 TEST  ****************************
	*
	* Routine to continuously read an A/D channel
	*
	*
	***********************************************/
	uint uiVal;
	uchar ucChan;
	uchar ucInputChanNum;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("AD1: Continuously read A/D\r\n", 28);
	vSERIAL_sout("Type a new channel number to change the channel\r\n", 49);
	vSERIAL_sout("(0=BATT, 1=RSSI, 2=PHOTO1, 3=PHOTO2, 4=TC0, 5=TC1, 6=TC2, 7=GENPURP)\r\n\n", 70);

	// INIT THE A/D
	vAD_init();

	ucChan = BATT_AD_CHAN;
//	ucChan = RSSI_AD_CHAN;
//	ucChan = PHOTO_1_AD_CHAN;
//	ucChan = PHOTO_2_AD_CHAN;
//	ucChan = TC_0_AD_CHAN;
//	ucChan = TC_1_AD_CHAN;
//	ucChan = TC_2_AD_CHAN;

	while(TRUE)					//lint !e716 !e774
		{
		uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_1);


		vSERIAL_sout("AD[ ", 4);

		// TELL WHICH CHAN IS BEING READ
		switch(ucChan)
			{

			case BATT_AD_CHAN:
 				vSERIAL_sout("batt", 4);
				break;

			case RSSI_AD_CHAN:
 				vSERIAL_sout("rssi", 4);
				break;

			case PHOTO_1_AD_CHAN:
 				vSERIAL_sout("photo1", 6);
				break;

			case PHOTO_2_AD_CHAN:
 				vSERIAL_sout("photo2", 6);
				break;

			case TC_0_AD_CHAN:
				vSERIAL_sout("tc0", 3);
				break;

			case TC_1_AD_CHAN:
 				vSERIAL_sout("tc1", 3);
				break;

			case TC_2_AD_CHAN:
 				vSERIAL_sout("tc2", 3);
				break;

			case GEN_PURPOSE_AD_CHAN:
 				vSERIAL_sout("genpurp", 7);
				break;

			default:
 				vSERIAL_sout("unk", 3);
				break;

			}// END: switch()

		vSERIAL_sout(" ] = ", 5);
		vSERIAL_UI16out(uiVal);
		vSERIAL_crlf();

		// ADD A WAIT
		vDELAY_wait100usTic(1000);		//100ms

		// CHECK IF USER WANTS TO CHANGE A/D
		if(ucSERIAL_kbhit())
			{
			// GET A NEW CHAN NUMBER
			vSERIAL_sout("Chan (0=BATT, 1=RSSI, 2=PHOTO1, 3=PHOTO2, 4=TC0, 5=TC1, 6=TC2, 7=GENPURP) = ", 76);
			ucInputChanNum = ucSERIAL_bin();
			vSERIAL_bout(ucInputChanNum);
			vSERIAL_crlf();

			// CONVERT INPUT NUMBER TO A CHANNEL NUM
			switch(ucInputChanNum)
				{
				case '0':
					vSERIAL_sout("Channel changed to BATT\r\n", 25);
					ucChan = BATT_AD_CHAN;
					break;

				case '1':
					vSERIAL_sout("Channel changed to RSSI\r\n", 25);
					ucChan = RSSI_AD_CHAN;
					break;

				case '2':
					vSERIAL_sout("Channel changed to PHOTO1\r\n", 27);
					ucChan = PHOTO_1_AD_CHAN;
					break;

				case '3':
					vSERIAL_sout("Channel changed to PHOTO2\r\n", 25);
					ucChan = PHOTO_2_AD_CHAN;
					break;

				case '4':
					vSERIAL_sout("Channel changed to TC0\r\n", 24);
					ucChan = TC_0_AD_CHAN;
					break;

				case '5':
					vSERIAL_sout("Channel changed to TC1\r\n", 24);
					ucChan = TC_1_AD_CHAN;
					break;

				case '6':
					vSERIAL_sout("Channel changed to TC2\r\n", 24);
					ucChan = TC_2_AD_CHAN;
					break;

				case '7':
					vSERIAL_sout("Channel changed to GENPURP\r\n", 28);
					ucChan = GEN_PURPOSE_AD_CHAN;
					break;

				case '?':
					vSERIAL_sout("HELP:  0=BATT, 1=RSSI, 2=PHOTO1, 3=PHOTO2, 4=TC0, 5=TC1, 6=TC2, 7=GENPURP\r\n", 75);
					break;

				default:
					vSERIAL_sout("WUPS:  0=BATT, 1=RSSI, 2=PHOTO1, 3=PHOTO2, 4=TC0, 5=TC1, 6=TC2, 7=GENPURP\r\n", 75);
					break;

				}// END: switch()

			}// END: if()

		}// END: while()

	vAD_quit();

	}
	#endif				//AD1_DIAG_ENA








	#ifdef AD2_DIAG_ENA

	/*******  SECOND AD TEST  *******
	*
	*
	*
	*
	************************************/
	{
	uint uiVal;
	uint uiMIN = 0;
	uint uiMAX;
	uint uiTmp;
	uint uiDiff;
	uint uiSum;

	uchar uci;
	uchar ucj;
	uchar uck;

	uchar ucChan;

	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Test the A/D\r\n", 16);

	vSERIAL_sout("Entering A/D test\r\n", 19);		//lint!e64


	// INIT THE A/D
	vAD_init();

//	ucChan = RSSI_AD_CHAN;
	ucChan = BATT_AD_CHAN;
//	ucChan = PHOTO_1_AD_CHAN;
//	ucChan = PHOTO_2_AD_CHAN;
//	ucChan = TC_0_AD_CHAN;
//	ucChan = TC_1_AD_CHAN;
//	ucChan = TC_2_AD_CHAN;


//	for(uci=0;  uci<4;  uci++)
	for(uci=0;  ;  uci++)
		{
		uiTmp = uiMIN;

		uiMIN = 0x400;
		uiMAX = 0;
		uiSum = 0;

		for(ucj=0; ucj<16;  ucj++)
			{
			uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_1);
//			uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_1);
//			uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_1);
//			uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_1);
//			uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_1);
//			uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_1);
//			uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_1);

			uiSum += uiVal;

			// CHECK FOR A NEW MAX
			if(uiVal > uiMAX)   uiMAX = uiVal;

			// CHECK FOR A NEW MIN
			if(uiVal < uiMIN)   uiMIN = uiVal;

			// CHECK FOR A NEW FLOOR FOR GRAPH
			if(uiTmp > uiMIN)   uiTmp = uiMIN;	//adjust for going down

			uiDiff = uiVal - uiTmp;

			if(uiDiff > 40) uiDiff = 20;

#if  0
			// SHOW THE VALUE
			vSERIAL_HB16out((uchar) uiVal);
			vSERIAL_bout(' ');

			// DO GRAPH
			for(uck=0; uck<uiDiff; uck++)
				{
				vSERIAL_bout(' ');
				}
			vSERIAL_bout('*');
			vSERIAL_crlf();
#endif


			// ADD A WAIT
//			vDELAY_wait100usTic(1000);
			vDELAY_wait100usTic(120);

			}// END: for(ucj)

#if  1
		// SHOW THE MINIMUM
		vSERIAL_sout("     ", 5);
		vSERIAL_HB16out(uiMIN);

		// show the maximum
		vSERIAL_bout(',');
		vSERIAL_HB16out(uiMAX);

		// SHOW THE DIFFERENCE
		vSERIAL_sout(" = ", 3);
		uiTmp = uiMAX - uiMIN;
		vSERIAL_HB16out(uiTmp);

		// SHOW LOCAL AVERAGE
		vSERIAL_sout("     ", 5);
		uiSum = (uiSum >> 4);
		vSERIAL_HB16out(uiSum);

		// SHOW SUBROUTINE AVE
		vSERIAL_sout(" - ", 3);
		uiVal = uiAD_read(ucChan, AD_READ_AVE_OF_16);
		vSERIAL_HB16out(uiVal);

		// SHOW THE DIFFERENCE
		vSERIAL_sout(" = ", 3);
		uiDiff = uiSum - uiVal;
		vSERIAL_HB16out(uiDiff);

		// SHOW A FLAG IF DIFFERENCE IS OUT OF LIMITS

		if((uiDiff >= 2 ) && (uiDiff <= 0xFFFE)) vSERIAL_sout("     #", 6);

		vSERIAL_crlf();
#endif

		}// END: for(;;)

	vAD_quit();

	}// END for(;;)

	#endif	//AD2_DIAG_ENA






	#ifdef AD3_DIAG_ENA

	/*******  THIRD AD TEST  *******
	*
	* Routine to test the soil moisture probe
	*
	*
	************************************/
	{
	int iVal;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout(": Test the A/D on the SOIL PROBE \r\n", 35);

	while(TRUE)				//lint !e716 !e774
		{

		iVal = iReadEchoDirtProbe();

		// WAIT
		vDELAY_wait100usTic(120);			// 12ms

		vSERIAL_I16out(iVal);
		vSERIAL_crlf();

		}// END: while()

	vAD_quit();

	}

	#endif //AD3_DIAG_ENA








	#ifdef AMP3_RUN_CUR_DIAG_ENA
	{
	/***********************  AMP3 diagnostic  *****************************
	*
	* Routine to measure the current for running processor
	*
	*
	*************************************************************************/

	vSERIAL_sout("AMP3: Measure RUN CURRENT TEST (no radio)\r\n", 43);

Endless_loop:
	goto Endless_loop;

	}
	#endif // AMP3_RUN_CUR_DIAG_ENA






	#ifdef AMP4_SLEEP_CUR_DIAG_ENA
	{
	/***********************  AMP4 diagnostic  *****************************
	*
	* Routine to measure the sleep current during sleep instruction.
	*
	* Needs to change! to match sleep choices
	*
	*************************************************************************/

	vSERIAL_sout("AMP4: SLEEP current Test\r\n", 26);

Endless_loop:

	_asm
		//;BCF PORTC,3,0							//debug
		SLEEP
		NOP									//lint !e40 !e522 !e10
		//;BSF PORTC,3,0							//debug
	_endasm;								//lint !e40 !e522 !e10


	goto Endless_loop;

	}
	#endif	//AMP4_SLEEP_CUR_DIAG_ENA






	#ifdef AMP5_DIAG_ENA
	{
	#include "time.h"		//Time handling routines
	#include "misc.h"		//miscellaneous routines

	/***********************  AMP5 diagnostic  *****************************
	*
	* Measure the sleep current when the actual sleep function is called
	*
	* Needs to change! to match sleep choices
	*
	*************************************************************************/
	uchar ucTmp;

	vSERIAL_sout("AMP5: Test Sleep Function\r\n", 27);

	while(TRUE)								//lint !e716 !e774
		{
		vSERIAL_sout("Going to Sleep...", 17);

		// SLEEP UNTIL BUTTON PUSH
		vTIME_setAlarmFromLong(0x7FFFFFFF);
		ucTmp = ucMISC_sleep_until_button_or_clk(SLEEP_MODE);

		// HANDLE A BUTTON PUSH
		vSERIAL_sout("BTTN\r\n", 6);

		vMISC_blinkLED(5);

		}

	} //lint !e550
	#endif // AMP5_DIAG_ENA





	#ifdef AMP6_DIAG_ENA
	{
	#include "time.h"		//Time handling routines
	#include "misc.h"		//miscellaneous routines

	/***********************  AMP6 diagnostic  *****************************
	*
	* Measure Hibernation current when the actual Hibernate function is called
	*
	* Needs to change! to match sleep choices
	*
	*************************************************************************/

	vSERIAL_sout("AMP6: Test Hibernate Function\r\n", 31);

	while(TRUE)								//lint !e716 !e774
		{
		vSERIAL_sout("Going to Hibernate...\r\n", 23);

		// SLEEP UNTIL BUTTON PUSH
		vTIME_setAlarmFromLong(0x7FFFFFFF);
		ucMISC_sleep_until_button_or_clk(HIBERNATE_MODE);	//lint !e534

		// HANDLE A BUTTON PUSH
		vSERIAL_sout("BTTN\r\n", 6);

		vMISC_blinkLED(5);

		}

	}
	#endif // AMP6_DIAG_ENA





	#ifdef BATT1_DIAG_ENA
	{
	#include "delay.h" 		//time delay routines
	#include "Action.h"		//Action routines
	#include "Key.h"		//Key routines
	#include "L2FLASH.h"	//Level 2 flash routines
	#include "L2FRAM.h"		//Level 2 Fram routines

	/***********************  BATT1 diagnostic  *******************************
	*
	* Test the DO_BATT_SENSE routine
	*
	**************************************************************************/
	ulong ulMsgCnt;
	ulong ulii;
//	uint uiBatt_in_mV;

	vSERIAL_sout("BATT1: Test Batt read routines \r\n", 33);

	while(TRUE) 			//lint !e774
		{

		// EMPTY OUT THE FLASH FIRST
		ulMsgCnt = (ulong)lL2FRAM_getFlashUnreadMsgCount();
		for(ulii=0;  ulii<ulMsgCnt; ulii++)
			{
			vL2FLASH_getNextMsgFromFlash();
			}// END: for()

		// PUT MSGS INTO FLASH
		for(ulii=0;  ulii<20UL; ulii++)
			{
			vSERIAL_dash(15);

			// DO THE BATT READ AND BUILD AN OM2
			vACTION_do_Batt_Sense();
			vKEY_checkAndRepairFlashMsgs();
			vMSG_showMsgBuffer(YES_CRLF, SHOW_MSG_COOKED);

			vSERIAL_dash(15);

			// ADD A WAIT
			vDELAY_wait100usTic(5000);		//500ms

			}// END: for()


		// NOW GO GET THE DATA
		ucKEY_doKeyboardCmdInput();	//lint !e534

		}// END: while()

	}
	#endif // BATT1_DIAG_ENA







	#ifdef BIGSUB_DIAG_ENA
	{
	/***********************  BIGSUB diagnostic  *****************************
	*
	* Routine to test the big subtract
	*
	*
	*************************************************************************/
	uchar ucc;

	vSERIAL_sout("BIGSUB: Test\r\n", 14);

	ucaBigMinuend[0] = 0;
	ucaBigMinuend[1] = 0;
	ucaBigMinuend[2] = 0;
	ucaBigMinuend[3] = 5;
	ucaBigMinuend[4] = 0x80;
	ucaBigMinuend[5] = 00;

	ucaBigSubtrahend[0] = 0;
	ucaBigSubtrahend[1] = 0;
	ucaBigSubtrahend[2] = 0;
	ucaBigSubtrahend[3] = 1;
	ucaBigSubtrahend[4] = 0x80;
	ucaBigSubtrahend[5] = 25;			//0x19

	vBIGSUB_doBigDiff();
	vSERIAL_sout("BigDiff = ", 10);

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT)
		vSERIAL_bout('Z');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT)
		vSERIAL_bout('4');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_CARRY_BIT)
		vSERIAL_bout('C');
	else
		vSERIAL_bout(' ');
	for(ucc=0;  ucc<6;  ucc++)
		{
		vSERIAL_HB8out(ucaBigDiff[ucc]);
		}
	vSERIAL_crlf();




	ucaBigMinuend[0] = 0;
	ucaBigMinuend[1] = 0;
	ucaBigMinuend[2] = 0;
	ucaBigMinuend[3] = 4;
	ucaBigMinuend[4] = 0x80;
	ucaBigMinuend[5] = 35;			//0x23

	ucaBigSubtrahend[0] = 0;
	ucaBigSubtrahend[1] = 0;
	ucaBigSubtrahend[2] = 0;
	ucaBigSubtrahend[3] = 4;
	ucaBigSubtrahend[4] = 0x80;
	ucaBigSubtrahend[5] = 25;		//0x19

	vBIGSUB_doBigDiff();
	vSERIAL_sout("BigDiff = ", 10);

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT)
		vSERIAL_bout('Z');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT)
		vSERIAL_bout('4');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_CARRY_BIT)
		vSERIAL_bout('C');
	else
		vSERIAL_bout(' ');
	for(ucc=0;  ucc<6;  ucc++)
		{
		vSERIAL_HB8out(ucaBigDiff[ucc]);
		}
	vSERIAL_crlf();




	ucaBigMinuend[0] = 0;
	ucaBigMinuend[1] = 0;
	ucaBigMinuend[2] = 0;
	ucaBigMinuend[3] = 2;
	ucaBigMinuend[4] = 0x80;
	ucaBigMinuend[5] = 25;			//0x19

	ucaBigSubtrahend[0] = 0;
	ucaBigSubtrahend[1] = 0;
	ucaBigSubtrahend[2] = 0;
	ucaBigSubtrahend[3] = 4;
	ucaBigSubtrahend[4] = 0x80;
	ucaBigSubtrahend[5] = 25;		//0x19

	vBIGSUB_doBigDiff();
	vSERIAL_sout("BigDiff = ", 10);

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT)
		vSERIAL_bout('Z');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT)
		vSERIAL_bout('4');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_CARRY_BIT)
		vSERIAL_bout('C');
	else
		vSERIAL_bout(' ');
	for(ucc=0;  ucc<6;  ucc++)
		{
		vSERIAL_HB8out(ucaBigDiff[ucc]);
		}
	vSERIAL_crlf();




	ucaBigMinuend[0] = 1;
	ucaBigMinuend[1] = 1;
	ucaBigMinuend[2] = 1;
	ucaBigMinuend[3] = 2;
	ucaBigMinuend[4] = 0x80;
	ucaBigMinuend[5] = 25;

	ucaBigSubtrahend[0] = 1;
	ucaBigSubtrahend[1] = 1;
	ucaBigSubtrahend[2] = 1;
	ucaBigSubtrahend[3] = 2;
	ucaBigSubtrahend[4] = 0x80;
	ucaBigSubtrahend[5] = 25;

	vBIGSUB_doBigDiff();
	vSERIAL_sout("BigDiff = ", 10);

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT)
		vSERIAL_bout('Z');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT)
		vSERIAL_bout('4');
	else
		vSERIAL_bout(' ');

	if(ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_CARRY_BIT)
		vSERIAL_bout('C');
	else
		vSERIAL_bout(' ');
	for(ucc=0;  ucc<6;  ucc++)
		{
		vSERIAL_HB8out(ucaBigDiff[ucc]);
		}
	vSERIAL_crlf();



	}
	#endif // BIGSUB_DIAG_ENA





   #ifdef DSK3_DIAG_ENA
	{
	/***********************  DSK3 diagnostic  *****************************
	*
	* Catch disk interaction diagnostic
	*
	*
	*************************************************************************/

	// CATCH SRAM INTERACTION WITH RADIO
	uint uii;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("SRAM RADIO INTERACTION TEST\r\n", 29);

	// RADIO IS ON
	vRADIO_start_receiver();

	// CLK IT TICKING

	// NOW CONTINUOUSLY READ LOCATION 40H AN 41H
	for(uii=0; ;uii++)
		{
		uint uiLOC40a41;

		uiLOC40a41 = uiSRAM_lo_read_B16(0x40);
		if(uiLOC40a41 != 0x0001)
			{
			vSERIAL_sout("B", 1);
			vSRAM_lo_write_B16(0x40, 0x0001);	//write it back
			}
		else
			{
			vSERIAL_bout('.');
			}
		}
	}
   #endif // END: DSK3_DIAG_ENA




	#if 0
	// CHECK THE EDITED INPUT
	{
	/***********************  xxxx diagnostic  *****************************
	*
	* Check the edited input routine
	*
	*
	*************************************************************************/
	long lValue;

	vSERIAL_sout("TEST EDITED INPUT\r\n", 19);


	while(TRUE)							//lint !e716 !e774
		{
		vSERIAL_sout("TYPE in a str: ", 15);

		lValue = lSERIAL_L32in();

		vSERIAL_sout("Value = ", 8);
 		vSERIAL_I32out(lValue);
		vSERIAL_crlf();
		vSERIAL_crlf();

		}// END: while()
	}
	#endif // END: CHECK THE EDITED INPUT



	#if  0
	{
	/***********************  FAKE read diagnostic  *****************************
	*
	* Fake Read test
	*
	*
	*************************************************************************/

	// setup a fake reading for test purposes
	vStuffFakeMsgToSRAM();
	vStuffFakeMsgToSRAM();
	}
	#endif



	#ifdef SLEEP1_DIAG_ENA
	{
	/***********************  SLEEP1 diagnostic  *****************************
	*
	* Sleep test
	*
	*
	*************************************************************************/
	uchar ucVal;
	uchar ucTmp;

	vSERIAL_sout("SLEEP1: Test Sleep Function\r\n", 29);

	while(TRUE)								//lint !e716 !e774
		{
		vSERIAL_sout("Going to Sleep..."17);

		// SLEEP UNTIL BUTTON PUSH
		vSetAlarmWithAbs(0x7FFFFFFF);
		ucTmp = ucDO_sleep_until_button_or_clk(SLEEP_MODE);

		// HANDLE A BUTTON PUSH
		vSERIAL_sout("BTTN\r\n", 6);

		vBlinkLED(5);

		}

	}
	#endif // SLEEP1_DIAG_ENA




	#ifdef SLEEP2_DIAG_ENA
	{
	/***********************  SLEEP2 diagnostic  *****************************
	*
	* Hibernate test
	*
	*
	*************************************************************************/
	uchar ucVal;
	uchar ucTmp;

	vSERIAL_sout("SLEEP2: Test Hibernate Function\r\n", 33);

	while(TRUE)								//lint !e716 !e774
		{
		vSERIAL_sout("Going to Hibernate...\r\n", 23);

		// SLEEP UNTIL BUTTON PUSH
		vSetAlarmWithAbs(0x7FFFFFFF);
		ucTmp = ucDO_sleep_until_button_or_clk(HIBERNATE_MODE);

		// HANDLE A BUTTON PUSH
		vSERIAL_sout("BTTN\r\n", 6);

		vBlinkLED(5);

		}

	}
	#endif // END: SLEEP2_DIAG_ENA





	#ifdef SLEEP3_DIAG_ENA
	{
	/***********************  SLEEP3 diagnostic  *****************************
	*
	* Sleep test
	*
	*
	*************************************************************************/
	uchar ucTmp;

	while(TRUE)								//lint !e716 !e774
		{
		vSERIAL_sout("SLEEP3: T3 wakeup function\r\n", 28);

		vTIME_setSysTimeFromLong(10L);
		vTIME_setAlarmFromLong(0x7FFFFFFF);
		ucTmp = ucTIME_setT3AlarmToSecMinus200ms(12L);
		if(ucTmp != 0)
			{
			vSERIAL_sout("Set T3 return = ", 16);
			vSERIAL_UIV8out(ucTmp);
			vSERIAL_crlf();
			}

		vSERIAL_sout("Going to Sleep...", 17);

		// SLEEP UNTIL BUTTON PUSH
		ucTmp = ucMISC_sleep_until_button_or_clk(SLEEP_MODE);

		vSERIAL_sout("Wake up at: ", 12);
		vTIME_ShowWholeSysTimeInDuS(YES_CRLF);

		} // END: while(TRUE)

	}
	#endif // SLEEP3_DIAG_ENA





	#ifdef SOIL1_DIAG_ENA
	{
	/***********************  SOIL1 diagnostic  *****************************
	*
	* Soil moisture test
	*
	*
	*************************************************************************/
	uchar ucSampleCount;
	uchar ucSensorVal1;
	uchar ucSensorVal2;
	uchar ucSensorVal3;
	uchar ucSensorVal4;
	uchar ucSensorVal5;

	vSERIAL_sout("SOIL1: continuous read function\r\n", 33);

	ucSampleCount = 1;
	ucSensorVal1 = SENSOR_SOIL_MOISTURE_1;
	ucSensorVal2 = 0;
	ucSensorVal3 = 0;
	ucSensorVal4 = 0;
	ucSensorVal5 = 0;

	while(TRUE)								//lint !e716 !e774
		{
		uchar ucc;
		uchar ucSensorNum[OM2_MAX_DATA_ENTRY_COUNT];
		uint uiSensorReading[OM2_MAX_DATA_ENTRY_COUNT];


		// ZRO ALL THE SENSOR NUMBERS AND READINGS
		for(ucc=0;  ucc<OM2_MAX_DATA_ENTRY_COUNT;  ucc++)
			{
			ucSensorNum[ucc] = 0;
			uiSensorReading[ucc] = 0;
			}

		// FILL THE SENSOR READINGS AS NEEDED
		ucSensorNum[0] = ucSensorVal1;
		ucSensorNum[1] = ucSensorVal2;;
		ucSensorNum[2] = ucSensorVal3;;
		ucSensorNum[3] = ucSensorVal4;;
		ucSensorNum[4] = ucSensorVal5;;


		// POWER UP THE SD BOARD
		if(ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_DISABLED) == 0)
			{
			vSERIAL_sout("ACTN: Err booting SD GSUS\r\n", 27);
			}

		// READ ALL THE REQUIRED SENSORS
		for(ucc=0;  ucc<ucSampleCount;  ucc++)
			{
			// REQUEST THE SENSOR DATA
			if(ucSDCOM_awaken_SD_and_do_full_msg_transfer(ucSensorNum[ucc]) == 0)
				{
				vSERIAL_sout("ACTN: SD msg err Sensor ", 24);
				vSERIAL_UIV8out(ucSensorNum[ucc]);
				vSERIAL_crlf();
				}

			// SAVE THE RESULTS
			uiSensorReading[ucc] = uiMISC_buildUintFromBytes(
									(uchar *)&ucaSDSPI_BUFF[3],
									NO_NOINT
									);

			#if 0
			vSERIAL_sout("GSUS ", 5);
			vSERIAL_UIV8out(ucSensorNum[ucc]);
			vSERIAL_sout(" = ", 3);
			vSERIAL_UIV16out(uiSensorReading[ucc]);
			vSERIAL_crlf();
			#endif

			// SHOW THE READING
			vSERIAL_UIV16out(uiSensorReading[ucc]);
			vSERIAL_crlf();

			}// END: for(ucc)

		// POWER DOWN THE SD BOARD
//		vSDCTL_halt_SD_and_power_down();

		#if 0
		vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);
		#endif

		// WAIT FOR USER TO CONTINUE
//		vSERIAL_any_char_to_cont();

		} // END: while(TRUE)

	}
	#endif // SOIL1_DIAG_ENA








	#ifdef COMPILE_FOR_STREAMING_HUB

	/************  STREAMING HUB CODE  **********
	*
	* THIS DIAGNOSTIC HAS A RECEIVE ONLY COMPONENT
	*
	****************************************/

	{
	uchar ucc;
	uint uiAgentNum;
	uint uiDataVal;
	uchar ucLimit;
	uchar ucIntegrityRetVal;

	vSERIAL_sout("STREAMING HUB: Listen to Wiz Radio Activity\r\n", 45);


	/********************  RECEIVE  ************************/

	while(TRUE)			//lint !e716 !e774
		{
		// STARTUP THE RADIO RECEIVER
		vRADIO_start_receiver();					//startup the receiver

		// WAIT FOR MESSAGE COMPLETE
		while(!ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT);	//lint !e722

		ucLimit = (ucaMSG_BUFF[GMH_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK);


		/********************  STREAMING HUB LINE 1  *********************/

		ucIntegrityRetVal = ucMSG_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
			CHKBIT_CRC+CHKBIT_MSG_TYPE,
			0,
			MSG_TYPE_OM2,					//msg type
			0,								//src SN
			0								//Dst SN
			);

		// THROW OUT ANY BAD MSGS
		if(ucIntegrityRetVal != 0) continue;


		// SHOW THE AGENT SN
		uiAgentNum = uiMISC_buildUintFromBytes(
						(uchar *)&ucaMSG_BUFF[OM2_IDX_AGENT_NUM_HI],
						NO_NOINT
						);
		if(uiAgentNum > 200) continue;		//throw out bad agents
		vSERIAL_sout("#", 1);
		vSERIAL_UIV16out(uiAgentNum);
		vSERIAL_sout(" ", 1);


		// SHOW SENSOR AND DATA
		for(ucc=OM2_IDX_DATA_0_SENSOR_NUM;  ucc<ucLimit-2;  ucc+=3)
			{
			switch(ucaMSG_BUFF[ucc+0])
				{
				case SENSOR_LIGHT_1:
				case SENSOR_LIGHT_2:
				case SENSOR_LIGHT_3:
				case SENSOR_LIGHT_4:
					vSERIAL_sout("\tLIGHT", 6);
					vSERIAL_UIV8out(ucaMSG_BUFF[ucc+0] - 10);
					vSERIAL_sout("= ", 2);
					uiDataVal = uiMISC_buildUintFromBytes(
									(uchar *)&ucaMSG_BUFF[ucc+1],
									NO_NOINT
									);
					vSERIAL_UIV16out(uiDataVal);
					vSERIAL_sout(",  ", 3);
					break;

				case SENSOR_TC_1:
				case SENSOR_TC_2:
				case SENSOR_TC_3:
				case SENSOR_TC_4:
					{
					int iData;
					long lData;
					long lT_10000;
					long lTFullDeg;
					long lTDecDeg;

					uiDataVal = uiMISC_buildUintFromBytes(
									(uchar *)&ucaMSG_BUFF[ucc+1],
									NO_NOINT
									);
					iData = (int)uiDataVal;
					lData = (long)iData;

					#if 0
					if(iData == 0)
						{
						vSERIAL_sout("\t\t\t", 3);
						break;
						}
					#endif

					vSERIAL_sout("\tTC", 3);
					vSERIAL_UIV8out(ucaMSG_BUFF[ucc+0] - 6);
					vSERIAL_sout("=", 1);

					lT_10000 = (595 * lData) + 250000L;
					lTFullDeg = lT_10000/10000L;
					lTDecDeg = lT_10000 - (lTFullDeg * 10000L);
					vSERIAL_IV32out(lTFullDeg);
					vSERIAL_bout('.');

					vSERIAL_sout("  ", 2);
					}

					break;

				default:
					break;

				}// END: switch()

			}// END: for()

		vSERIAL_crlf();
		vSERIAL_crlf();

		}// END: while()
	}
	#endif // END: COMPILE_FOR_STREAMING_HUB







	#ifdef COMPILE_FOR_STREAMING_SLAVE
	{
	/***********************  STREAMING SPOKE  *******************************
	*
	* Routine to stream data to the hub (hub in sniffer mode)
	*
	*
	*************************************************************************/
	uchar ucSensor1Num;
	uint uiSensor1Data;
	uchar ucSensor2Num;
	uint uiSensor2Data;
	ulong ulRandSubSlot;

	ulong ulSlotStartTime_sec;
	ulong ulMsgXmitOffset_sec;
	ulong ulMsgXmitOffset_us;
	ulong ulMsgXmitOffset_subsec_us;
	ulong ulMsgXmitTime_sec;

	uint uiMsgXmitTime_tics;
	uint uiMsgXmitTime_clks;

	uchar ucaDC5sndTime[6];
	uchar ucXmitRetVal;


	vSERIAL_sout("STREAMING SPOKE: routine\r\n", 26);

	while(TRUE)							//lint !e716 !e774
		{

		// READ LIGHT_1 DATA
		ucSensor1Num = SENSOR_LIGHT_1;

		if(ucSDCOM_awaken_SD_and_do_full_msg_transfer(ucSensor1Num) == 0)
			{
			vSERIAL_sout("ACTN: SD msg err Sensor ", 24);
			vSERIAL_UIV8out(ucSensor1Num);
			vSERIAL_crlf();
			}

		uiSensor1Data = uiMISC_buildUintFromBytes(
							(uchar *)&ucaSDSPI_BUFF[3],
							NO_NOINT
							);


		// READ TC_1 DATA
		ucSensor2Num = SENSOR_TC_1;

		if(ucSDCOM_awaken_SD_and_do_full_msg_transfer(ucSensor2Num) == 0)
			{
			vSERIAL_sout("ACTN: SD msg err Sensor ", 24);
			vSERIAL_UIV8out(ucSensor1Num);
			vSERIAL_crlf();
			}

		uiSensor2Data = uiMISC_buildUintFromBytes(
							(uchar *)&ucaSDSPI_BUFF[3],
							NO_NOINT
							);



		// BUILD THE OM2 MESSAGE
	  	vOPMODE_buildMsg_OM2(
					0,							//LAST_PKT_BIT & NO_DATA_PKT_BIT
					0,							//dest SN
					ucMSG_incMsgSeqNum(),		//msg seq num
					0,							//Load Factor
					uiROM_getSnumLo16AsUint(),	//Agent SN
					lTIME_getSysTimeAsLong(),	//Collection Time
					2,							//Data count
					ucSensor1Num,				//Sensor 0
					uiSensor1Data,				//Sensor 0 data
					ucSensor2Num,				//Sensor 1
					uiSensor2Data,				//Sensor 1 data
					0,							//Sensor 2
					0,							//Sensor 2 data
					0,							//Sensor 3
					0							//Sensor 3 data
					);


//		// POWER DOWN THE SD BOARD
//		vSDCTL_halt_SD_and_power_down();


		// NOW PICK A RANDOM ANSWER SLOT
		ulRandSubSlot = (ulong)(ucRAND_getRolledMidSysSeed() % 5);

		// COMPUTE THE SLOT START TIME
		ulSlotStartTime_sec = (ulong)lTIME_getSysTimeAsLong()+1;

		// COMPUTE THE TRANSMIT OFFSET
		ulMsgXmitOffset_us = (ulRandSubSlot * 200000UL);

		// CONVERT TRANSMIT TIME TO SEC AND OFFSET
		ulMsgXmitOffset_sec = ulMsgXmitOffset_us / 1000000UL;
		ulMsgXmitOffset_subsec_us = ulMsgXmitOffset_us % 1000000UL;

		// COMPUTE THE TRANSMIT TIME
		ulMsgXmitTime_sec = ulSlotStartTime_sec + ulMsgXmitOffset_sec;

		// CONVERT THE SUBSEC-TIME-IN-SEC TO SUBSEC-TIME-IN-TICS
		uiMsgXmitTime_tics = (uint)(ulMsgXmitOffset_subsec_us / CLK_uS_PER_TIC_UL);
		uiMsgXmitTime_clks = uiMsgXmitTime_tics | 0x8000;

		// PACK THE WHOLE TIME
		vMISC_copyUlongIntoBytes(					//pack FULL sec part
					ulMsgXmitTime_sec,
					&ucaDC5sndTime[0],
					NO_NOINT
					);
		vMISC_copyUintIntoBytes(
					uiMsgXmitTime_clks,				//pack SUBSEC part
					&ucaDC5sndTime[4],
					NO_NOINT
					);

		// SEND THE MSG
		ucXmitRetVal = ucMSG_doSubSecXmit(
							&ucaDC5sndTime[0],
							USE_CLK1,
							NO_RECEIVER_START
							);


		}// END: while()

	}
	#endif // END: COMPILE_FOR_STREAMING_SLAVE








	#ifdef CLK1_TIC_DIAG_ENA

	/*********  CHECK THE CLOCK TIC  **********
	*
	*
	*************************************************/
	{
	uchar ucc;
	uchar ucTime1Array[6];
	uchar ucTime2Array[6];
	long lClk1Time_first;
	long lClk2Time_first;
	long lClk1Time;
	long lClk2Time;

	uint uiSubSec_first;
	uint uiSubSec;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("CLK1: Clock Rollover Diagnostic\r\n", 33);

	// SETUP TIME AT TEST VALUE
	lClk1Time_first = 0x70FFFF80L;
	lClk2Time_first = 0x70FFFF80L;

	lClk1Time = 0x11223344L;
	lClk2Time = 0x55667788L;





	// CHECK THE FIRST 4 SYSTIME ROUTINES
	vSERIAL_sout("1 - ", 4);

	vTIME_setSysTimeFromLong(lClk1Time_first);
	vSERIAL_HB32out((ulong)lTIME_getSysTimeAsLong());
	vSERIAL_sout("  ", 2);

	vMISC_copyUlongIntoBytes((ulong)lClk1Time, &ucTime1Array[0], NO_NOINT);
	vTIME_setSysTimeFromBytes(&ucTime1Array[0]);
	vTIME_copySysTimeToBytes(&ucTime2Array[0]);
	for(ucc=0;  ucc<4;  ucc++)
		{
		vSERIAL_HB8out(ucTime2Array[ucc]);
		}
	vSERIAL_crlf();





	// CHECK THE SECOND 4 SUBSEC TIME ROUTINES
	vSERIAL_sout("2 - ", 4);
	uiSubSec_first = 0x1122;
	uiSubSec = 0x5566;

	vTIME_setSubSecFromUint(uiSubSec_first);
	vSERIAL_HB16out(uiTIME_getSubSecAsUint());
	vSERIAL_sout("  ", 2);

	vMISC_copyUintIntoBytes(uiSubSec, &ucTime1Array[0], NO_NOINT);
	vTIME_setSubSecFromBytes(&ucTime1Array[0]);
	vTIME_copySubSecToBytes(&ucTime2Array[0]);
	for(ucc=0;  ucc<2;  ucc++)
		{
		vSERIAL_HB8out(ucTime2Array[ucc]);
		}
	vSERIAL_crlf();




	// CHECK THE THIRD GROUP OF TIME ROUTINES
	vSERIAL_sout("3 - ", 4);

	ucTime1Array[0] = 0x11;
	ucTime1Array[1] = 0x22;
	ucTime1Array[2] = 0x33;
	ucTime1Array[3] = 0x44;
	ucTime1Array[4] = 0x55;
	ucTime1Array[5] = 0x66;

	vTIME_setWholeSysTimeFromBytesInDtics(&ucTime1Array[0]);
	vTIME_copyWholeSysTimeToBytesInDtics(&ucTime2Array[0]);
	for(ucc=0;  ucc<6;  ucc++)
		{
		vSERIAL_HB8out(ucTime2Array[ucc]);
		}
	vSERIAL_sout("  ", 2);
	vTIME_setWholeSysTimeFromBytesInDtics(&ucTime1Array[0]);
	vTIME_ShowWholeSysTimeInDuS(YES_CRLF);






	// CHECK WHOLE TIME DIFF
	vSERIAL_sout("4 - ", 4);

	ucTime1Array[0] = 0x11;
	ucTime1Array[1] = 0x22;
	ucTime1Array[2] = 0x33;
	ucTime1Array[3] = 0x44;
	ucTime1Array[4] = 0x55;
	ucTime1Array[5] = 0x66;

	vSERIAL_HB16out(uiMISC_buildUintFromBytes(&ucTime1Array[4], YES_NOINT));
	vSERIAL_sout("  ", 2);
	vTIME_showWholeTimeInDuS(&ucTime1Array[0], NO_CRLF);
	vSERIAL_sout("  ", 2);
	vTIME_showWholeTimeInDuSDiff(&ucTime1Array[0], &ucTime2Array[0], YES_CRLF);




	// CHECK THE NEXT 4 ALARM ROUTINES
	vSERIAL_sout("5 - ", 4);
	vTIME_setSysTimeFromLong(0L);

	vTIME_setAlarmFromLong(0x33445566L);
	vSERIAL_HB32out((ulong)lTIME_getAlarmAsLong());
	vSERIAL_sout("  ", 2);

	vMISC_copyUlongIntoBytes(0x77665544UL, &ucTime1Array[0], NO_NOINT);
	vTIME_setAlarmFromBytes(&ucTime1Array[0]);
	vTIME_copyAlarmToBytes(&ucTime2Array[0]);
	for(ucc=0;  ucc<4;  ucc++)
		{
		vSERIAL_HB8out(ucTime2Array[ucc]);
		}
	vSERIAL_crlf();





	// CHECK THE CLK2 ROUTINES
	vSERIAL_sout("6 - ", 4);

	vTIME_setClk2FromLong(lClk1Time_first);
	vSERIAL_HB32out((ulong)lTIME_getClk2AsLong());
	vSERIAL_sout("  ", 2);

	vMISC_copyUlongIntoBytes((ulong)lClk1Time, &ucTime1Array[0], NO_NOINT);
	vTIME_setClk2FromBytes(&ucTime1Array[0]);
	vTIME_copyClk2ToBytes(&ucTime2Array[0]);
	for(ucc=0;  ucc<4;  ucc++)
		{
		vSERIAL_HB8out(ucTime2Array[ucc]);
		}
	vSERIAL_crlf();



	// CHECK THE CLK2 COPY ROUTINE
	vSERIAL_sout("7 - ", 4);

	vTIME_setClk2FromLong(0x22334455L);
	vTIME_copyClk2ToSysTime();
	vSERIAL_HB32out((ulong)lTIME_getSysTimeAsLong());
	vSERIAL_crlf();
	vSERIAL_crlf();


	// CHECK WHOLE SYSTIME  OUPUT FOR A PARTICULAR VALUE
	vSERIAL_sout("8 - ", 4);

	ucTime1Array[0] = 0x11;
	ucTime1Array[1] = 0x22;
	ucTime1Array[2] = 0x33;
	ucTime1Array[3] = 0x44;
	ucTime1Array[4] = 0x63;
	ucTime1Array[5] = 0x36;

	vTIME_setSysTimeFromBytes(&ucTime1Array[0]);
	vTIME_ShowWholeSysTimeInDuS(NO_CRLF);
	vSERIAL_crlf();



	// CHECK ALARM EARLY
	vSERIAL_sout("9 - ", 4);

	ucTime1Array[0] = 0x00;
	ucTime1Array[1] = 0x00;
	ucTime1Array[2] = 0x00;
	ucTime1Array[3] = 0x10;
	ucTime1Array[4] = 0x00;
	ucTime1Array[5] = 0x00;

	vTIME_setWholeSysTimeFromBytesInDtics(&ucTime1Array[0]);
	vTIME_setAlarmFromLong(0x00000017L);		//7 sec alarm

	while(!ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT);		//lint !e722
	vTIME_copyWholeSysTimeToBytesInDtics(&ucTime2Array[0]);

	vSERIAL_sout("ALARM went off at ", 18);
	vTIME_showWholeTimeInDuS(&ucTime2Array[0], YES_CRLF);



	// CHECK ALARM EQUAL TO
	vTIME_setWholeSysTimeFromBytesInDtics(&ucTime1Array[0]);
	vTIME_setAlarmFromLong(0x00000010L);

	while(!ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT);			//lint !e722
	vTIME_copyWholeSysTimeToBytesInDtics(&ucTime2Array[0]);

	vSERIAL_sout("ALARM went off at ", 18);
	vTIME_showWholeTimeInDuS(&ucTime2Array[0], YES_CRLF);



	// CHECK ALARM LESS THAN
	vTIME_setWholeSysTimeFromBytesInDtics(&ucTime1Array[0]);
	vTIME_setAlarmFromLong(0x00000008L);

	while(!ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT);			//lint !e722
	vTIME_copyWholeSysTimeToBytesInDtics(&ucTime2Array[0]);

	vSERIAL_sout("ALARM went off at ", 18);
	vTIME_showWholeTimeInDuS(&ucTime2Array[0], YES_CRLF);





	// CHECK THE CLK TIC
	vTIME_setSysTimeFromLong(0x11223344L);

	while(TRUE)						//lint !e774
		{
		vSERIAL_HB32out((ulong)lTIME_getSysTimeAsLong());
		vSERIAL_sout("  ", 2);

		vSERIAL_HB32out((ulong)lTIME_getClk2AsLong());
		vSERIAL_sout("  ", 2);

		vTIME_ShowWholeSysTimeInDuS(NO_CRLF);
		vSERIAL_sout("  ", 2);

		vSERIAL_crlf();

		vDELAY_wait100usTic(5000);
		};

	}
	#endif







	#ifdef CLK2_TIC_DIAG_ENA

	/*********  CHECK THE WAKE UP TIC  **********
	*
	*
	*************************************************/
	{
	long lLongCurTime;
	unsigned long ull;
	uchar ucTIMER_OK;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("CLK2: Clock Tic Diagnostic\r\n", 28);


	while(TRUE)					//lint !e774
		{
		// SETUP TIME AT TEST VALUE
		ucCLK_TIME[0] = 0x00;
		ucCLK_TIME[1] = 0xFF;
		ucCLK_TIME[2] = 0xFF;
		ucCLK_TIME[3] = 0xFD;


		ucTIMER_OK = FALSE;
		for(ull=0; ull<11000;  ull++)
			{
			lLongCurTime = lTIME_getSysTimeAsLong();

			if(lLongCurTime >= 0x01000000)
				{
				ucTIMER_OK = TRUE;
				break;
				}

			} // END: for()

		if(ucTIMER_OK)
			{
			vSERIAL_sout("Timer is OK ------- counter = ", 30);
			vSERIAL_UI32out(ull);
			vSERIAL_crlf();
			}
		else	// (ucTIMER_OK == FALSE)
			{
			vSERIAL_sout("Timer is BAD --------- Time = ", 31);
			vSERIAL_HB32out((unsigned long)lLongCurTime);
			vSERIAL_crlf();
			}

		}// END: while() 

	}
	#endif  // END: CLK2_TIC_DIAG_ENA






	#ifdef CLK3_TIC_DIAG_ENA

	/*********  CHECK THE BYTE TO LONG AND LONG TO BYTE ROUTINES  **********
	*
	*
	*************************************************/
	{
	uchar ucc;
	unsigned long ulTestTime;
	unsigned long ulReadBackTime;
	uchar ucByteArray[6];
	uchar ucByteArray2[6];
	unsigned long ulLongResult;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("CLK3: check byte to long & long to byte conversion\n", 51);


	// CHECK THE BUILD ROUTINE
	ucByteArray[0] = 0x12;
	ucByteArray[1] = 0x34;
	ucByteArray[2] = 0x56;
	ucByteArray[3] = 0x78;
	ulLongResult = ulMISC_buildLongFromBytes((uchar *)&ucByteArray[0]);
	vSERIAL_sout("BuildLongFromBytes = "19);
	vSERIAL_HB32Fout(ulLongResult);
	vSERIAL_sout(" Should be 0x12345678\r\n", 23);


	// CHECK THE COPYLONGINTOBYTES ROUTINE
	ulLongResult = 0x87654321;
	vMISC_copyLongIntoBytes(ulLongResult,
							(uchar *)&ucByteArray[0]);
	vSERIAL_sout("CopyLongIntoBytes = ", 20);
	vSERIAL_Hformat();
	for(ucc=0;  ucc<4; ucc++)
		{
		vSERIAL_HB8out(ucByteArray[ucc]);
		}
	vSERIAL_sout("  should be 0x87654321\r\n", 24);


	// CHECK THE BYTESTOBYTES ROUTINE
	ucByteArray[0] = 0x12;
	ucByteArray[1] = 0x34;
	ucByteArray[2] = 0x56;
	ucByteArray[3] = 0x78;
	vMISC_copyBytesToBytes((uchar *) &ucByteArray[0],
							(uchar *) &ucByteArray2[0],
							4
							);
	vSERIAL_sout("CopyBytesToBytes = ", 19);
	vSERIAL_Hformat();
	for(ucc=0;  ucc<4; ucc++)
		{
		vSERIAL_HB8out(ucByteArray2[ucc]);
		}
	vSERIAL_sout("  should be 0x12345678\r\n", 22);



	// check the setSysTimeFromLong routine
	ulTestTime = 0x12345678;
	vTIME_setSysTimeFromLong(ulTestTime);
	vSERIAL_sout("Test time = ", 12);
	vSERIAL_HB32out(ulTestTime);
	ulReadBackTime = (unsigned long)lTIME_getSysTimeAsLong();
	vSERIAL_sout("  ReadBack time = ", 18);
	vSERIAL_HB32out(ulReadBackTime);
	vSERIAL_crlf();


	vTIME_copySysTimeToBytes((uchar *) &ucByteArray[0]);
	vSERIAL_sout("Time as byte array = ", 21);
	for(ucc=0;  ucc<4; ucc++)
		{
		vSERIAL_HB8out(ucByteArray[ucc]);
		}
	vSERIAL_sout(" Should be= 0x12345678\r\n", 24);


	ucByteArray[0] = 0x76;
	ucByteArray[1] = 0x54;
	ucByteArray[2] = 0x32;
	ucByteArray[3] = 0x10;
	vTIME_setSysTimeFromBytes((uchar *) &ucByteArray[0]);
	vSERIAL_sout("SetSysTimeFromBytes= ", 21);
	vSERIAL_HB32out((unsigned long)lTIME_getSysTimeAsLong());
	vSERIAL_sout("  Should be= 0x76543210\r\n", 25);


	// check copyWholeSysTimeToBytesInDtics

	ucByteArray[0] = 0x11;
	ucByteArray[1] = 0x22;
	ucByteArray[2] = 0x33;
	ucByteArray[3] = 0x44;
	ucByteArray[4] = 0x55;
	ucByteArray[5] = 0x66;
	vTIME_setWholeSysTimeFromBytesInDtics((uchar *)&ucByteArray[0]);
	vTIME_copyWholeSysTimeToBytesInDtics((uchar *) &ucByteArray2[0]);

	vSERIAL_sout("WholeSysTimeToBytes set= ", 25);
	vSERIAL_Hformat();
	for(ucc=0;  ucc<6; ucc++)
		{
		vSERIAL_HB8out(ucByteArray[ucc]);
		}
	vSERIAL_sout("  WholeSysTimeToBytes readback= ", 32);
	vSERIAL_Hformat();
	for(ucc=0;  ucc<6; ucc++)
		{
		vSERIAL_HB8out(ucByteArray2[ucc]);
		}
	vSERIAL_crlf();



	// check setAlarmFromLong and getAlarmAsLong

	vTIME_setAlarmFromLong(0x22334455);
	ulLongResult = (unsigned long)lTIME_getAlarmAsLong();

	vSERIAL_sout("SetAlarmFromLong= 0x22334455  readback= ", 40);
	vSERIAL_HB32Fout(ulLongResult);
	vSERIAL_crlf();


	// check CopyAlarmFromBytes and CopyAlarmToBytes


	ucByteArray[0] = 0x11;
	ucByteArray[1] = 0x22;
	ucByteArray[2] = 0x33;
	ucByteArray[3] = 0x44;
	vTIME_setAlarmFromBytes((uchar *) &ucByteArray[0]);
	vTIME_copyAlarmToBytes((uchar *) &ucByteArray2[0]);

	vSERIAL_sout("SetAlarmFromBytes SET= ", 23);
	vSERIAL_Hformat();
	for(ucc=0;  ucc<4; ucc++)
		{
		vSERIAL_HB8out(ucByteArray[ucc]);
		}
	vSERIAL_sout("  GetAlarmFromBytes GET= ", 25);
	for(ucc=0;  ucc<4; ucc++)
		{
		vSERIAL_HB8out(ucByteArray2[ucc]);
		}
	vSERIAL_crlf();

	}
	#endif  // END: CLK3_TIC_DIAG_ENA






	#ifdef CLK4_TIC_DIAG_ENA

	/*********  CHECK TIC  **********
	*
	*
	*************************************************/
	{
	long lLongCurTime;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("CLK4: Clock Tic Diagnostic\r\n", 28);

	ucCLK_TIME[0] = 0x00;
	ucCLK_TIME[1] = 0xFF;
	ucCLK_TIME[2] = 0xFF;
	ucCLK_TIME[3] = 0xFD;

	lLongCurTime = lTIME_getSysTimeAsLong();

	while(TRUE)					//lint !e774
		{
		if(lLongCurTime != lTIME_getSysTimeAsLong())
			{
			vSERIAL_sout("T1=", 3);
			vSERIAL_UIV32out((ulong)lLongCurTime);
			vSERIAL_crlf();

			lLongCurTime = lTIME_getSysTimeAsLong();

			}

		}// END: while()

	}
	#endif  // END: CLK4_TIC_DIAG_ENA











	#ifdef CLK7_DIAG_ENA

	/*********  CLK DIAGNOSTIC   ********************
	*
	* Check T0 timer routines
	*
	*************************************************/
	{
	long lLongCurTime;
	uchar ucWholeTime[6];

	// SETUP TIMER 0
	vT0_init();

	while(TRUE)	//lint !e774
		{
		// SHOW HEADER MESSAGE
		vSERIAL_sout("CLK7: Chk T0 routines\r\n", 23);

		// SETUP THE SYSTEM TIMER
		ucWholeTime[0] = 0;
		ucWholeTime[1] = 0;
		ucWholeTime[2] = 0;
		ucWholeTime[3] = 0;
		ucWholeTime[4] = 0x80;
		ucWholeTime[5] = 0;
		vTIME_setWholeSysTimeFromBytesInDtics(&ucWholeTime[0]);
		lLongCurTime = lTIME_getSysTimeAsLong();

		vT0_start_T0_timer();

		while(TRUE)					//lint !e774
			{
			if(lLongCurTime != lTIME_getSysTimeAsLong())
				{
				vSERIAL_sout("T1=", 3);
				vSERIAL_UIV32out((ulong)lLongCurTime);
				vSERIAL_crlf();

				lLongCurTime = lTIME_getSysTimeAsLong();

				}

			if(lLongCurTime >= 5L) break;

			}// END: while()

		Stop_T0_timer;

		vTIME_ShowWholeSysTimeInDuS(NO_CRLF);
		vSERIAL_sout("  ", 2);
		vT0_show_T0_in_uS();
		vSERIAL_crlf();


		vSERIAL_any_char_to_cont();


		}// END: while() 

	}
	#endif  // END: CLK7_TIC_DIAG_ENA





	#ifdef CLK8_DIAG_ENA

	/*********  CLK8 DIAGNOSTIC   ********************
	*
	* Check the time and date routines
	*
	*************************************************/
	{
	uchar ucaStr[10];
	DATE_STRUCT sDate;
	TIME_STRUCT sTime;
	char cTmp;
	long lDaySec;
	long lHourSec;
	long lTotSec;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("\r\nCLK8: Chk the Date and time routines\r\n", 40);

	while(TRUE)	//lint !e774
		{
		while(TRUE)	//lint !e774
			{
			vSERIAL_sout("Enter Date (MM/DD/YY): ", 23);

			if(ucSERIAL_getEditedInput(ucaStr, 10))
				{
				vSERIAL_sout("Err ret from getEdited input\r\n\n", 31);
				continue;
				}

			if((cTmp = cDAYTIME_convertDateStrToDstruct(ucaStr, &sDate)) < 0)
				{
				vSERIAL_sout("Conversion err ret= ", 20);
				vSERIAL_IV8out(cTmp);
				vSERIAL_crlf();
				vSERIAL_crlf();
				continue;
				}

			break;

			}// END: while()

		vDAYTIME_convertDstructToShowAsNumericLine(&sDate);
		vSERIAL_sout(" or ", 4);
		vDAYTIME_convertDstructToShowAsTextLine(&sDate);
		vSERIAL_crlf();
		lDaySec = lDAYTIME_convertDstructToSec(&sDate);
		vSERIAL_sout("DaySec=", 7);
		vSERIAL_IV32out(lDaySec);
		vSERIAL_crlf();
		vSERIAL_crlf();




		while(TRUE)	//lint !e774
			{
			vSERIAL_sout("Enter Time (HH:MM:SS): ", 23);

			if(ucSERIAL_getEditedInput(ucaStr, 10))
				{
				vSERIAL_sout("Err ret from getEdited input\r\n\n", 31);
				continue;
				}

			if((cTmp = cDAYTIME_convertTstrToTstruct(ucaStr, &sTime)) < 0)
				{
				vSERIAL_sout("Conversion err ret= ", 20);
				vSERIAL_IV8out(cTmp);
				vSERIAL_crlf();
				vSERIAL_crlf();
				continue;
				}

			break;

			}// END: while()

		vDAYTIME_showTimeAsNumericLine(&sTime);
		vSERIAL_crlf();
		lHourSec = lDAYTIME_convertTstructToSec(&sTime);
		vSERIAL_sout("HourSec=", 8);
		vSERIAL_IV32out(lHourSec);
		vSERIAL_crlf();
		vSERIAL_crlf();


		// COMPUTE THE TOTAL SEC
		lTotSec = lDaySec+lHourSec;

		vSERIAL_sout("Total Date sec= ", 16);
		vSERIAL_IV32out(lTotSec);
		vSERIAL_crlf();


		// CONVERT THIS TO A STRUCT
		vDAYTIME_convertSecsToDstructAndTstruct(
			lTotSec,
			&sDate,
			&sTime
			);

		vSERIAL_sout("raw date struct= ", 17);
		vSERIAL_UIV8out(sDate.ucMonth);
		vSERIAL_bout('-');
		vSERIAL_UIV8out(sDate.ucDay);
		vSERIAL_bout('-');
		vSERIAL_UIV8out(sDate.ucYear);
		vSERIAL_crlf();


		vSERIAL_sout("Total Secs as a date: ", 22);
		vDAYTIME_convertDstructToShowAsTextLine(&sDate);
		vSERIAL_sout(" ", 1);
		vDAYTIME_showTimeAsNumericLine(&sTime);
		vSERIAL_crlf();
		vSERIAL_crlf();

		vSERIAL_dash(20);

		vSERIAL_crlf();

		}// END: while() 

	}
	#endif  // END: CLK8_TIC_DIAG_ENA



   #ifdef LED1_DIAG_ENA

	/********  BLINK THE LED USING RAW CALLS *********
	*
	*
	**************************************************/
	{
	uchar ucCount;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("LED1: Blink the LED (using raw calls)\r\n", 39);

	LED_PORT &= ~LED_BIT;

	// FLASH THE LED
	for(ucCount=0; ; ucCount++)
		{
		if(ucCount&0x01)
			{
			LED_PORT &= ~LED_BIT;
			vSERIAL_bout('a');
			}
		else
			{
			LED_PORT |= LED_BIT;
			vSERIAL_bout('a');
			}

		vDELAY_wait100usTic(10000);

		}// END: for()

	}

   #endif  // END: LED1_DIAG_ENA






   #ifdef LED2_DIAG_ENA

	/********  BLINK THE LED USING SYS CALLS *********
	*
	*
	**************************************************/
	{
	// SHOW HEADER MESSAGE 
	vSERIAL_sout("LED2: Blink the LED (using sys calls)\r\n", 39);

	// FLASH THE LED
	while(TRUE)				//lint !e774
		{
		vBlinkLED(5);
		vDELAY_wait100usTic(5000);
		}

	}

   #endif  // END: LED2_DIAG_ENA









	#ifdef BUTTON0_DIAG_ENA

	/********  SHOW BUTTON IN LED ***
	*
	*
	*********************************/
	{
	// SHOW HEADER MESSAGE 
	vSERIAL_sout("BUTTON0: Show Button by blinking the LED\r\n", 42);

	while(TRUE)							//lint !e716 !e774
		{
		if(ucBUTTON_buttonHit())
			{
			vSERIAL_sout("ON ", 3);
			LED_PORT |= LED_BIT;
			while(ucBUTTON_buttonHit()); //lint !e722
			}
		else
			{
			vSERIAL_sout("OFF ", 4);
			LED_PORT &= ~LED_BIT;
			while(!ucBUTTON_buttonHit()); //lint !e722
			}

		}// END: while() 
	}
	#endif // BUTTON0_DIAG_ENA







	#ifdef BUTTON1_DIAG_ENA

	/********  MEASURE A BUTTON PUSH TIME ***
	*
	*
	*****************************************/
	{
	long lStartTime;
	long lEndTime;
	long lDiffTime;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("BUTTON1: Button Measurement Diagnostic\r\n", 40);


	while(TRUE)		//lint !e716 !e774
		{
		if(ucMISC_buttonHit() == 1)
			{
			lStartTime = TA1R;

			while(ucMISC_buttonHit() == 1);	//lint !e722

			lEndTime = TA1R;

			lDiffTime = lEndTime - lStartTime;

			vSERIAL_I32out(lEndTime);
			vSERIAL_bout('-');
			vSERIAL_I32out(lStartTime);
			vSERIAL_bout('=');
			vSERIAL_I32out(lDiffTime);
			vSERIAL_crlf();

			}

		}// END: for(;;)


	}  // END: BUTTON1_DIAG_ENA

	#endif // BUTTON1_DIAG_ENA






 	#ifdef BUTTON2_DIAG_ENA

	/********  Debounce the button ***
	*
	*
	*****************************************/
	{
	// SHOW HEADER MESSAGE
	vSERIAL_sout("BUTTON2: Debounce the button\r\n", 30);

	while(TRUE)		//lint !e716 !e774
		{
		if(ucMISC_buttonHit() == 1)
			{
			vDELAY_wait100usTic(600);

			if(ucMISC_buttonHit() == 1)
				{
				vSERIAL_sout("YES\r\n", 5);
				while(ucMISC_buttonHit() == 1);
				}
			else
				{
				vSERIAL_sout("NO\r\n", 4);
				}

			vSERIAL_crlf();

			}

		}// END: while() 

	}

	#endif // END: BUTTON2_DIAG_ENA






 	#ifdef BUTTON3_DIAG_ENA
	{
	/***********************  BUTTON3 diagnostic  *****************************
	*
	* Wakeup test
	*
	*
	*************************************************************************/
	uchar ucTmp;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("BUTTON3: Button Wakeup Diagnostic\r\n", 35);


	while(TRUE)								//lint !e716 !e774
		{
		// SLEEP UNTIL BUTTON PUSH
		vSetAlarmWithAbs(0x7FFFFFFF);
		vSERIAL_sout("Going to Sleep...", 17);
//		ucTmp = ucDO_sleep_until_button_or_clk(HIBERNATE_MODE);
		ucTmp = ucDO_sleep_until_button_or_clk(SLEEP_MODE);

		// HANDLE A BUTTON PUSH
		vSERIAL_sout("BTTN\r\n", 6);

		vBlinkLED(5);

		}

	}
	#endif // BUTTON3_DIAG_ENA







 	#ifdef BUTTON4_DIAG_ENA
	{
	/***********************  BUTTON4 diagnostic  *****************************
	*
	* Count the number of spurious pushes on the button
	*
	*************************************************************************/
	uint uiCount;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("BUTTON4: Count number of spurious button pushes\r\n", 49);

	uiCount=0;
	while(TRUE)				//lint !e716 !e774
		{
		// COUNT THE NUMBER OF SPURIOUS BUTTON PUSHES
		if(BUTTON_IN_PORT & BUTTON_BIT)
			{
			uiCount++;
			vSERIAL_sout("Bttn=", 5);
			vSERIAL_UI16out(uiCount);
			vSERIAL_crlf();
			}
		}

	}
	#endif // BUTTON4_DIAG_ENA




 	#ifdef BUTTON5_DIAG_ENA
	{
	/***********************  BUTTON5 diagnostic  *****************************
	*
	* Count int button pushes
	*
	*************************************************************************/
	ulong ulButtonCount;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("BUTTON5: Count number int button pushes\r\n", 41);

	vBUTTON_init();

	while(TRUE)				//lint !e716 !e774
		{
		if(ucBUTTON_isButtonFlgSet())
			{
			ulButtonCount = ulBUTTON_readAndClrButtonCount();
			vSERIAL_sout("Bttn= ", 6);
			vSERIAL_UIV32out(ulButtonCount);
			vSERIAL_sout("=", 1);
			vSERIAL_HB32out(ulButtonCount);
			vSERIAL_crlf();
			}

		}// END: while()

	}
	#endif // BUTTON5_DIAG_ENA






 	#ifdef BUTTON6_DIAG_ENA
	{
	/***********************  BUTTON6 diagnostic  *****************************
	*
	* Test button/clk interaction
	*
	*************************************************************************/
//	ulong ulButtonCount;
//	long lLongCurTime;
	uchar ucWholeTime[6];

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("BUTTON6: Test button-clk interaction\r\n", 38);

	vBUTTON_init();
	vT0_init();

	while(TRUE)	//lint !e774
		{
		// SETUP THE SYSTEM TIMER
		ucWholeTime[0] = 0;
		ucWholeTime[1] = 0;
		ucWholeTime[2] = 0;
		ucWholeTime[3] = 0;
		ucWholeTime[4] = 0x80;
		ucWholeTime[5] = 0;
		vTIME_setWholeSysTimeFromBytesInDtics(&ucWholeTime[0]);
		vT0_start_T0_timer();

//		lLongCurTime = lTIME_getSysTimeAsLong();

		vSERIAL_bout('S');

		while(TRUE)					//lint !e774
			{

			if(uiT0_get_val_in_tics() >= 48828) break;
//			if(ucCLK_TIME[3] >= 5) break;	//this is direct since it must be fast

			}// END: while()


		vTIME_copyWholeSysTimeToBytesInDtics(&ucWholeTime[0]);
		vT0_stop_T0_timer();

		vSERIAL_bout('X');

		vTIME_showWholeTimeInDuS(&ucWholeTime[0],NO_CRLF);
		vSERIAL_sout("  ", 2);
		vT0_show_T0_in_uS();
		vSERIAL_crlf();


		vSERIAL_any_char_to_cont();


		}// END: while()

	}
	#endif // BUTTON6_DIAG_ENA





	#if 0
	/****************  TEST THE SERIAL OUTPUT ROUTINES ***************
	*
	*
	* Serial chars output should be:
	*
	*		"AAAAA---GHIJKLMN--GHIJKLMN-1F4"
	*
	*****************************************************************/
	{
	uchar ucCount;
	static char cStr[] = "-GHIJKLMN-";
	char *cPtr;


	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Serial Output Test Routine\r\n", 30);


	while(TRUE)
		{

		// SEND FIVE 'A'S OUT
		for(ucCount=0;  ucCount<5;  ucCount++)
			{
			vSERIAL_bout('A');

			}// END: for(ucCount)

		vSERIAL_bout(cStr[0]);

		cPtr = cStr;
		vSERIAL_bout(*cPtr);

		for(ucCount=0;  ucCount<10;  ucCount++)
			{
			vSERIAL_bout(cStr[ucCount]);
			}


		vSERIAL_sout(cStr, 10);

		vSERIAL_HB8out(0x1F);

		vSERIAL_HB4out(0x54);

		vSERIAL_crlf();

		}// END: while()


	}
	#endif






	#if 0
	/************* TEST THE TIMER ALARM & BUTTON PUSH INT ROUTINES  ***********
	*
	*
	**************************************************************************/
	{

	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Test Timer alarm and button push routines\r\n", 45);


	for(;;)
		{
		vSERIAL_sout("SETTING ALARM FOR 5 SEC\r\n", 25);

		vSetAlarmWithDelta(0,0,0,5);
		if(ucDO_sleep_until_button_or_clk(SLEEP_MODE))
			{
			vSERIAL_sout("BUTTON, BUTTON\r\n\n", 17);
			}
		else
			{
			vSERIAL_sout("RINGI, DINGI\r\n\n", 15);
			}

		}

	}
	#endif






	#ifdef SRAM0_DIAG_ENA

	/*************  SRAM0 diagnostic  ****************************************
	*
	* FLOP THE DATA LINES UP AND DOWN
	*
	*
	**************************************************************************/
	{
	uchar ucWriteData;
	uchar ucReadData;
	uchar ucShouldBe;
	uchar uc;

//	#define MAX_MEM_SIZE 3
//	#define MAX_MEM_SIZE 10
//	#define MAX_MEM_SIZE 256
//	#define MAX_MEM_SIZE 4096
	#define MAX_MEM_SIZE 65535

	uchar ucAddrXI;			//hi 2 bits
//	uchar ucAddrHI;
//	uchar ucAddrLO;
	unsigned int uiAddrTwoByte;


	// SHOW HEADER MESSAGE
	vSERIAL_sout("SRAM0: Test SRAM hardware and routines\r\n", 40);

	ucAddrLO = 0;
//	ucAddrHI = 0;
//	ucAddrXI = 0;
	uiAddrTwoByte = 0;

	uc = 0;


	__bic_SR_register(GIE);					// No interrupts

	// LATCH XI ADDR BITS
	SRAM_ADDR_HI_PORT &= ~SRAM_ADDR_HI_2BIT;	//clr XI addr bits
	SRAM_ADDR_HI_PORT |= SRAM_ADDR_HI_2BIT;		//set XI addr bits

	// LATCH IN THE LO ADDR 2 BYTES
	SRAM_ADDR_LO_PORT = uiAddrTwoByte;

	// TURN ON WRITE ENABLE
	SRAM_SELRW_PORT &= ~SRAM_READWRITE;			//Turn on write

	// FORCE CORRECT DIRECTION
	SRAM_DATA_DIR_PORT = 0xFF;					//restore the latch to an output

	while(TRUE)
		{
		// STUFF THE DATA
		SRAM_DATA_OUT_PORT = uc;					//stuff data
		uc = ~uc;

		SRAM_SELRW_PORT |= SRAM_CHIP_SELECT;		//clock in the data
		vDELAY_wait4us();
		SRAM_SELRW_PORT &= ~SRAM_CHIP_SELECT;

		}

	}
	#endif	// END: SRAM0_DIAG_ENA







	#ifdef SRAM1_DIAG_ENA

	/*************  SRAM1 diagnostic  ****************************************
	*
	* TEST THE SRAM ROUTINES AND HARDWARE
	*
	* FULL MEMORY TEST
	*
	**************************************************************************/
	{
	USL uslAddr;
	uchar ucWriteData;
	uchar ucReadData;
	uchar ucShouldBe;
	uchar uc;

//	#define MAX_MEM_SIZE 3
//	#define MAX_MEM_SIZE 10
	#define MAX_MEM_SIZE 256
//	#define MAX_MEM_SIZE 4096
//	#define MAX_MEM_SIZE 65535
//	#define MAX_MEM_SIZE 262144


	#define STARTER_CODE 0x80;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("SRAM1: Test SRAM 256 passes\r\n", 29);


	vSERIAL_sout("Running mem test\r\n", 18);

	for(uc=0;  ;  uc++)
		{

		// FILL THE MEMORY
		vSERIAL_sout("\r\nWriting SRAM\r\n", 16);
		ucWriteData = STARTER_CODE;

		for(uslAddr=0;  uslAddr<MAX_MEM_SIZE;  uslAddr++)
			{

		  #if 1
			// SMASH A LOCATION ON PURPOSE TO PROVE TEST WORKS
			if(uslAddr == 0x1000)
				{
				vSRAM_write_B8(uslAddr, 0x25);
				ucWriteData++;
				continue;
				}
		  #endif

			// PUT OUT A PERIODIC NOTIFICATION TO USER
			if((uslAddr & 0x00FF) == 0)
				{
				vSERIAL_HB24out(uslAddr);
				vSERIAL_bout('\r');
				}

			vSRAM_write_B8(uslAddr, ucWriteData);

			ucWriteData++;

			}// for()


		vSERIAL_crlf();



		// CHECK THE MEMORY
		vSERIAL_sout("Reading SRAM\r\n", 14);
		ucShouldBe = STARTER_CODE;
		for(uslAddr=0;  uslAddr<MAX_MEM_SIZE;  uslAddr++)
			{
			// PUT OUT A PERIODIC NOTIFICATION TO USER
			if((uslAddr & 0x00FF) == 0)
				{
				vSERIAL_HB24out(uslAddr);
				vSERIAL_bout('\r');
				}

			// CHECK THE BYTE IN MEM
			ucReadData = ucSRAM_read_B8(uslAddr);
			if(ucReadData != ucShouldBe)
				{
				vSERIAL_sout("Mem Fail at ", 12);
				vSERIAL_HB24out(uslAddr);
				vSERIAL_sout(" wrote ", 7);
				vSERIAL_HB8out(ucShouldBe);
				vSERIAL_sout(" was ", 5);
				vSERIAL_HB8out(ucReadData);
				vSERIAL_crlf();

				vSERIAL_sout("Any char to continue...", 23);
				ucSERIAL_bin();								//lint !e534
				vSERIAL_crlf();
				}

			ucShouldBe++;

			}// for()

		vSERIAL_crlf();

		}// END: for(;;)

	}
	#endif // END: SRAM1_DIAG_ENA





	#ifdef SRAM2_DIAG_ENA

	/*************  SRAM2 DIAGNOSTIC  **************************************
	*
	* SEE DATA OUT AND IN SRAM HARDWARE
	*
	* This is test is used in case of a bad SRAM
	*
	************************************************************************/
	{
	uchar ucWriteData;
	uchar ucReadData;
	uchar ucShouldBe;
//	uchar uc;
	uint uiAddr;

	#define MAX_MEM_SIZE 10

	// SHOW HEADER MESSAGE
	vSERIAL_sout("SRAM2: Test Data in and out of SRAM\r\n", 37);


	ucWriteData = 0x55;

	for(uiAddr=0; uiAddr<MAX_MEM_SIZE;  uiAddr++)
		{

		vSERIAL_sout("Writing loc ", 12);
		vSERIAL_HB16out(uiAddr);
		vSERIAL_sout(" = ", 3);
		vSERIAL_HB8out(ucWriteData);
		vSERIAL_crlf();

		vSRAM_lo_write_byte(uiAddr, ucWriteData);

		ucWriteData++;

		}// END: for()


	vSERIAL_crlf();

	ucShouldBe = 0x55;

	for(uiAddr=0; uiAddr<MAX_MEM_SIZE;  uiAddr++)
		{

		ucReadData = ucSRAM_lo_read_byte(uiAddr);

		vSERIAL_sout("Reading loc ", 12);
		vSERIAL_HB16out(uiAddr);
		vSERIAL_sout(" = ", 3);
		vSERIAL_HB8out(ucReadData);
		vSERIAL_crlf();

		ucShouldBe++;

		}// END: for(;;)

	}
	#endif // END: SRAM2_DIAG_ENA








	#ifdef SRAM3_DIAG_ENA
	{
	/************  TEST SRAM3  ***********
	*
	* Check the L2sram Q routines
	*
	***************************************/

	uchar ucTmp;
	uchar uc;
	ulong uliii;


	// SHOW HEADER MESSAGE
	vSERIAL_sout("SRAM3: test of L2SRAM Q routines\r\n", 34);

	vL2SRAM_init(); //init

	// CLR THE RAM MEMORY
	for(uliii=L2SRAM_MSG_Q_BEG_UL;
		uliii<L2SRAM_MSG_Q_END_UL;
		uliii++
		)
		{
		vSRAM_write_B8((usl)uliii,0);
		}

	vSERIAL_sout("After INIT\r\n", 12);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	/*------------------- STORE #1 ---------------------------*/

	// INIT THE SD MSG BUFFER
	ucaMSG_BUFF[0]  = 4;
	ucaMSG_BUFF[1]  = 1;
	ucaMSG_BUFF[2]  = 2;
	ucaMSG_BUFF[3]  = 3;
	ucaMSG_BUFF[4]  = 4;
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	vL2SRAM_storeMsgToSram();		//stuff the new message

	vSERIAL_sout("After STORE 1\r\n", 15);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0; // clr msg buff 

	/*-------------------  STORE #2 ---------------------------*/

	ucaMSG_BUFF[0]  = 4;
	ucaMSG_BUFF[1]  = 5;
	ucaMSG_BUFF[2]  = 6;
	ucaMSG_BUFF[3]  = 7;
	ucaMSG_BUFF[4]  = 8;
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	vL2SRAM_storeMsgToSram();		//stuff the new message

	// SHOW THE POINTERS 
	vSERIAL_sout("After STORE 2\r\n", 15);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER 
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0; // clr msg buff 


	/*---------------------  COPY #1 ---------------------*/


	ucTmp = ucL2SRAM_getCopyOfCurMsg();				// copy the message to buff

	vSERIAL_sout("After COPY #1\r\n", 15);
	vSERIAL_sout("Ret=", 4);
	vSERIAL_HB8out(ucTmp);
	vSERIAL_crlf();
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0;




	/*---------------  DELETE #1 ---------------*/


	vL2SRAM_delCurMsg();

	vSERIAL_sout("After DELETE #1\r\n", 17);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();


	/*---------------  COPY #2 ---------------*/


	ucTmp = ucL2SRAM_getCopyOfCurMsg();

	vSERIAL_sout("After COPY 2\r\n", 14);
	vSERIAL_sout("Ret=", 4);
	vSERIAL_HB8out(ucTmp);
	vSERIAL_crlf();
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0;


	/*---------------  DELETE #2 ---------------*/

	vL2SRAM_delCurMsg();

	vSERIAL_sout("After DELETE #2\r\n", 17);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();


	/*-------------------  STORE #3 ---------------------------*/

	ucaMSG_BUFF[0]  = 0x6;
	ucaMSG_BUFF[1]  = 0x9;
	ucaMSG_BUFF[2]  = 0xA;
	ucaMSG_BUFF[3]  = 0xB;
	ucaMSG_BUFF[4]  = 0xC;
	ucaMSG_BUFF[5]  = 0xD;
	ucaMSG_BUFF[6]  = 0xE;

	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	vL2SRAM_storeMsgToSram();		//stuff the new message

	// SHOW THE POINTERS
	vSERIAL_sout("After STORE #3\r\n", 15);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0; // clr msg buff



	/*-------------------  STORE #4 ---------------------------*/

	ucaMSG_BUFF[0]  = 0x6;
	ucaMSG_BUFF[1]  = 0xF;
	ucaMSG_BUFF[2]  = 0x10;
	ucaMSG_BUFF[3]  = 0x11;
	ucaMSG_BUFF[4]  = 0x12;
	ucaMSG_BUFF[5]  = 0x13;
	ucaMSG_BUFF[6]  = 0x14;

	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	vL2SRAM_storeMsgToSram();		//stuff the new message

	// SHOW THE POINTERS
	vSERIAL_sout("After STORE #4\r\n", 15);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0; // clr msg buff


	/*-------------------  STORE #5 ---------------------------*/

	ucaMSG_BUFF[0]  = 0x6;
	ucaMSG_BUFF[1]  = 0x15;
	ucaMSG_BUFF[2]  = 0x16;
	ucaMSG_BUFF[3]  = 0x17;
	ucaMSG_BUFF[4]  = 0x18;
	ucaMSG_BUFF[5]  = 0x19;
	ucaMSG_BUFF[6]  = 0x20;

	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	vL2SRAM_storeMsgToSram();		//stuff the new message

	// SHOW THE POINTERS
	vSERIAL_sout("After STORE #5\r\n", 15);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0; // clr msg buff



	/*-------------------  STORE #6 ---------------------------*/

	ucaMSG_BUFF[0]  = 0x5;
	ucaMSG_BUFF[1]  = 0x21;
	ucaMSG_BUFF[2]  = 0x22;
	ucaMSG_BUFF[3]  = 0x23;
	ucaMSG_BUFF[4]  = 0x24;
	ucaMSG_BUFF[5]  = 0x25;
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	vL2SRAM_storeMsgToSram();		//stuff the new message

	// SHOW THE POINTERS
	vSERIAL_sout("After STORE #6\r\n", 15);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0; // clr msg buff


	/*-------------------  STORE #7 ---------------------------*/

	ucaMSG_BUFF[0]  = 0x5;
	ucaMSG_BUFF[1]  = 0x26;
	ucaMSG_BUFF[2]  = 0x27;
	ucaMSG_BUFF[3]  = 0x28;
	ucaMSG_BUFF[4]  = 0x29;
	ucaMSG_BUFF[5]  = 0x30;
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	vL2SRAM_storeMsgToSram();		//stuff the new message

	// SHOW THE POINTERS
	vSERIAL_sout("After STORE #7\r\n", 15);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0; // clr msg buff



	/*-------------------  STORE #8 ---------------------------*/

	ucaMSG_BUFF[0]  = 0x7;
	ucaMSG_BUFF[1]  = 0x31;
	ucaMSG_BUFF[2]  = 0x32;
	ucaMSG_BUFF[3]  = 0x33;
	ucaMSG_BUFF[4]  = 0x34;
	ucaMSG_BUFF[5]  = 0x35;
	ucaMSG_BUFF[6]  = 0x36;
	ucaMSG_BUFF[7]  = 0x37;
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);

	vL2SRAM_storeMsgToSram();		//stuff the new message

	// SHOW THE POINTERS
	vSERIAL_sout("After STORE #8\r\n", 15);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	// CLR THE MSG BUFFER
	for(uc=0; uc<MAX_MSG_SIZE;  uc++) ucaMSG_BUFF[uc] = 0; // clr msg buff


	/*---------------  DELETE #3 ---------------*/

	vL2SRAM_delCurMsg();

	vSERIAL_sout("After DELETE #3\r\n", 17);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	/*---------------  DELETE #4 ---------------*/

	vL2SRAM_delCurMsg();

	vSERIAL_sout("After DELETE #4\r\n", 17);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();

	/*---------------  DELETE #5 ---------------*/

	vL2SRAM_delCurMsg();

	vSERIAL_sout("After DELETE #5\r\n", 17);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();


	/*---------------  DELETE #6 ---------------*/

	vL2SRAM_delCurMsg();

	vSERIAL_sout("After DELETE #6\r\n", 17);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();


	/*---------------  DELETE #7 ---------------*/

	vL2SRAM_delCurMsg();

	vSERIAL_sout("After DELETE #7\r\n", 17);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();


	/*---------------  DELETE #8 ---------------*/

	vL2SRAM_delCurMsg();

	vSERIAL_sout("After DELETE #8\r\n", 17);
	vDIAG_showSramVals();
	vSERIAL_any_char_to_cont();


	/*---------------  END  ---------------*/

	vSERIAL_sout("Done\r\n", 6);

	}
	#endif // END: SRAM3_DIAG_ENA




	#ifdef SRAM4_DIAG_ENA

	/*************  SRAM4 diagnostic  ****************************************
	*
	* TEST THE SRAM ROUTINES AND HARDWARE
	*
	**************************************************************************/
	{
	uint uiAddr;
	uchar ucWriteData;
	uchar ucReadData;
	uchar ucShouldBe;
	uchar uc;

//	#define MAX_MEM_SIZE 3
//	#define MAX_MEM_SIZE 10
//	#define MAX_MEM_SIZE 256
//	#define MAX_MEM_SIZE 4096
	#define MAX_MEM_SIZE 65535


	#define STARTER_CODE 0x55;
	#define STARTER_ADDR 0x100;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("SRAM4: Test SRAM hardware and routines\r\n", 40);

	for(uc=0;  ;  uc++)
		{

		// FILL THE MEMORY
		vSERIAL_sout("\r\nWriting SRAM\r\n", 16);
		ucWriteData = STARTER_CODE;
		uiAddr = STARTER_ADDR;
		vSRAM_lo_write_B8(uiAddr, ucWriteData);

		// CHECK THE LO BYTE IN LO MEMORY
		ucReadData = ucSRAM_lo_read_B8(uiAddr);


		if(ucReadData != ucWriteData)
			{
			vSERIAL_sout("LO Mem Fail at ", 15);
			vSERIAL_HB16out(uiAddr);

			vSERIAL_sout(" wrote ", 7);
			vSERIAL_HB8out(ucWriteData);

			vSERIAL_sout(" read back ", 11);
			vSERIAL_HB8out(ucReadData);
			vSERIAL_crlf();

			vSERIAL_sout("Any char to continue...", 23);
			ucSERIAL_bin();								//lint !e534
			vSERIAL_crlf();
			}

		}// END: for(;;)

	}
	#endif // END: SRAM4_DIAG_ENA



	#ifdef STREAM1_DIAG_ENA
	{

	/*************  STREAM1 diagnostic  ****************************************
	*
	* sent a fake stream to the console
	*
	**************************************************************************/
	// SHOW HEADER MESSAGE 

	vSERIAL_sout("STREAM1: send a test stream msg to console\r\n", 44);

	vBUTTON_init();

	ucaMSG_BUFF[0]  = 0x1D; //size

	ucaMSG_BUFF[1]  = 0x06; //Msg Type

	ucaMSG_BUFF[2]  = 0x17; //Signature
	ucaMSG_BUFF[3]  = 0x95;

	ucaMSG_BUFF[4]  = 0xB4; //SRC ID
	ucaMSG_BUFF[5]  = 0x74;

	ucaMSG_BUFF[6]  = 0xB4; //Dest ID
	ucaMSG_BUFF[7]  = 0x93;

	ucaMSG_BUFF[8]  = 0x9B; //Seq Number

	ucaMSG_BUFF[9]  = 0x30; //load 30

	ucaMSG_BUFF[10] = 0xb4; //Agent
	ucaMSG_BUFF[11] = 0x74;

	ucaMSG_BUFF[12] = 0x0D; //Time
	ucaMSG_BUFF[13] = 0x3B;
	ucaMSG_BUFF[14] = 0xE0;
	ucaMSG_BUFF[15] = 0xf3;

	ucaMSG_BUFF[16] = 0x0b; //sensor 1 = LIGHT-1

	ucaMSG_BUFF[17] = 0x00;
	ucaMSG_BUFF[18] = 0x00;

	ucaMSG_BUFF[19] = 0x03; //sensor 2 = SD CODE VERSION

	ucaMSG_BUFF[20] = 0x02;
	ucaMSG_BUFF[21] = 0x07;

	ucaMSG_BUFF[22] = 0x0a;	//sensor 3 = TC-4

	ucaMSG_BUFF[23] = 0x03;
	ucaMSG_BUFF[24] = 0xda;

	ucaMSG_BUFF[25] = 0x13; //sensor 4 = ONEWIRE

	ucaMSG_BUFF[26] = 0x84;
	ucaMSG_BUFF[27] = 0x00;

	ucaMSG_BUFF[28] = 0x34; //Chksum
	ucaMSG_BUFF[29] = 0x42;


	while(TRUE)		//lint !e774
		{
		uchar ucii;
		uchar ucMsgCount;

		ucMsgCount = ucaMSG_BUFF[0] & MAX_MSG_SIZE_MASK;

		vSERIAL_bout('^');				//header char announcing stream
		for(ucii=0;  ucii<=ucMsgCount; ucii++)
			{
			vSERIAL_HB8out(ucaMSG_BUFF[ucii]);

			}// END: for(ucii)

		vSERIAL_crlf();

		while(!ucBUTTON_buttonHit());	//lint !e722

		}// END: while()

	}
	#endif // END: STREAM1_DIAG_ENA




	#if 0
	/************  TEST THE RADIO XMIT TIMER TIC (TIMER 2) **************
	*
	* This requires the port toggle of PORTB,2 be turned on in radioa.asm
	*
	* This routine toggles the portb,2 flag of each clock tic of timer 2.
	* so that it can be seen on the scope.
	*
	* Radio has changed so this may change! too
	*
	********************************************************************/
	{
	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Test the radio xmit timer tic on timer 2\r\n", 44);


	while(TRUE)
		{
		vSERIAL_sout("Starting the radio transmission now\r\n", 37);

		vRADIO_xmit_msg();					//startup the transmitter

		}

	}
	#endif




	#if 0
	/************  TEST THE RADIO REC TIMER TIC (TIMER 2) **************
	*
	* This requires the port toggle of PORTB,2 be turned on in radioa.asm
	*
	* This routine toggles the portb,2 flag of each clock tic of timer 2.
	* so that it can be seen on the scope.
	*
	* Radio has changed so this may change! too
	*
	*
	********************************************************************/
	{
	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Test the radio receive timer tic (TIMER 2)\r\n", 46);

	while(TRUE)
		{
		vSERIAL_sout("Starting the radio receive now\r\n", 32);

		vRADIO_start_receiver();

		}

	}
	#endif



	#if 0
	/************  TEST THE XOR COUNTER TABLE **************************
	*
	*
	* XOR results are 0,3,7
	*
	* Radio has changed so this may change! too
	* Will at least need asm code change!
	*
	********************************************************************/
	{
	extern void BYTE_1_CNT(void);
	uint ui;

	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Test the radio XOR function table\r\n", 37);


	vSERIAL_sout("Entered BYTE_1_CNT test code\r\n", 30);


	for(ui=0;  ui<=255;  ui++)
		{
		ucaMSG_BUFF[GMH_IDX_EOM_IDX] = (uchar) ui;

		_asm
			MOVLB 4

			MOVLW 1
			MOVWF TBLPTRH,0

			MOVFF ucaMSG_BUFF+0,WREG

			CALL BYTE_1_CNT,0
			MOVFF TABLAT,ucaMSG_BUFF+1
		_endasm

		vSERIAL_HB8out(ucaMSG_BUFF[GMH_IDX_EOM_IDX]);
		vSERIAL_sout(" = ", 3);
		vSERIAL_HB8out(ucaMSG_BUFF[GMH_IDX_MSG_TYPE]);
		vSERIAL_crlf();
		}

	}
	#endif







	#if 0
	/************* TEST THE A/D ROUTINES  ************
	*
	*
	**************************************************/
	{
	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Test the A/D routines\r\n", 25);

	vAD_init();

	uiAD_read(TC_0_AD_CHAN, AD_READ_AVE_OF_1);

	}
	#endif





	#ifdef VOFFSET1_DIAG_ENA

	/************************  VOFFSET1 diagnostic  **************************
	*
	* Continuous read of V reference (offset) voltage
	*
	*
	*
	*
	************************************************************************/
	{

	int iAD_V_ref;
	uint uci;


	// SHOW HEADER MESSAGE 
	vSERIAL_sout("VOFFSET1: Continuous read of V ref\r\n", 36);

	vAD_init();					//turn on A/D

	for(uci=0;  ;  uci++)
		{

		iAD_V_ref = (int)uiAD_read(TC_0_AD_CHAN, AD_READ_AVE_OF_1);

		vSERIAL_I16out(iAD_V_ref);
		vSERIAL_bout('(');
		vSERIAL_HB16out((uint)iAD_V_ref);
		vSERIAL_bout(')');
		vSERIAL_crlf();

		vDELAY_wait100usTic(3000);		//delay so operator can read it


		}// END: for(;;)

	vAD_quit();

	}// END for(;;)

	#endif		//VOFFSET1_DIAG_ENA






	#ifdef TBL1_DIAG_ENA

	/************************  TBL1 diagnostic  **************************
	*
	* TBL test
	*
	*
	************************************************************************/
	{
	usl uslCheckBaseAddr1;
	usl uslCheckBaseAddr2;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("TBL1: Test of tbl indexing\r\n", 28);

	uslCheckBaseAddr1 = uslaNST_baseAddr[0];  //lint !e661

	vRADIO_init();

	vRADIO_start_receiver();

	while(TRUE) //lint !e774
		{
		vSERIAL_sout("-2-", 3);

		uslCheckBaseAddr2 = uslaNST_baseAddr[0];  //lint !e661
		if(uslCheckBaseAddr1 != uslCheckBaseAddr2)
			{

			vSERIAL_bout('*');
#if 0
			vSERIAL_sout("NST_tblNum= ", 12);
			vSERIAL_UIV8out(0);
			vSERIAL_sout("  NST_BaseAddr1= ", 17);
			vSERIAL_UIV24out(uslCheckBaseAddr1);
			vSERIAL_sout("  NST_BaseAddr2= ", 17);
			vSERIAL_UIV24out(uslCheckBaseAddr2);
			vSERIAL_crlf();
#endif
			}
		else
			{
//			vSERIAL_bout('.');
			}

		// IF THE RADIO STOPS -- RESTART IT
		if(ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT)
			{
			vRADIO_start_receiver();
			}

		}// END: while() 

	}

	#endif	//TBL1_DIAG_ENA





	#ifdef TC1_DIAG_ENA

	/************************  TC1 diagnostic  **************************
	*
	*	THERMOCOUPLE READ TEST
	*
	*
	************************************************************************/
	{

	int iAD_raw;
	int iAD_V_offset;
	int iAD_actual;
	uint uiColdJctTempInC_x16;
    int iColdJctTempInC_x16;
	int iWholePart;
	uint uiFractionalPart;
	uint uci;


	// SHOW HEADER MESSAGE 
	vSERIAL_sout("TC1: Thermocouple read test\r\n", 29);


	for(uci=0;  ;  uci++)
		{

		// GET THE READING
		vAD_init();					//turn on A/D

		iAD_raw = (int)uiAD_read(TC_1_AD_CHAN, AD_READ_AVE_OF_1);

		iAD_V_offset = (int)uiAD_read(TC_0_AD_CHAN, AD_READ_AVE_OF_1);

		iAD_actual = iAD_raw - iAD_V_offset;

		vAD_quit();					//turn off A/D

		// READ THE COLD JUNCTION TEMPERATURE
		uiColdJctTempInC_x16 = uiReadTemp();
	    iColdJctTempInC_x16 = (int)uiColdJctTempInC_x16;

		if(((uiColdJctTempInC_x16 & 0x8000) == 0) &&
			(uiColdJctTempInC_x16 >= 0x07D0))
			{
			vSERIAL_sout(" Invalid Read\r\n", 15);
			continue;
			}

		if(((uiColdJctTempInC_x16 & 0x8000) == 1) &&
			(uiColdJctTempInC_x16 <= 0xFC90))
			{
			vSERIAL_sout(" Invalid Read\r\n", 15);
			continue;
			}

		// SHOW THE DECIMAL TEMPERATURE VALUE

		iWholePart = iColdJctTempInC_x16/16;

		uiFractionalPart = iColdJctTempInC_x16 & 0x0F;
		uiFractionalPart *= 100;
		uiFractionalPart /= 16;

		vSERIAL_I16out(iAD_raw);
		vSERIAL_sout(" (", 2);
		vSERIAL_HB16out((uint)iAD_raw);
		vSERIAL_bout(')');
		vSERIAL_sout(" - ", 3);
		vSERIAL_I16out(iAD_V_offset);
		vSERIAL_sout(" (", 2);
		vSERIAL_HB16out((uint)iAD_V_offset);
		vSERIAL_sout(") = ", 4);
		vSERIAL_I16out(iAD_actual);
		vSERIAL_sout(" @", 2);
		vSERIAL_I16out(iWholePart);
		vSERIAL_bout('.');
		vSERIAL_UIV16out(uiFractionalPart);
		vSERIAL_crlf();

		}// END: for(;;)

	vAD_quit();

	}// END for(;;)

	#endif			//TC1_DIAG_ENA





	#ifdef TC2_DIAG_ENA

	/************************  TC2 DIAGNOSTIC  ******************************
	*
	* Thermocouple test that mimics the DUKE trial run
	*
	* may change! with change to mini SD
	*
	************************************************************************/
	{

	uint uiVal;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("TC2: Mimic TC_12\n\r", 18);

	while(TRUE)				//lint !e774
		{

		LED_PORT &= ~LED_BIT;

		// BUILD THE MESSGE
		ucaMSG_BUFF[OM2_IDX_MSG_TYPE] = MSG_TYPE_OPMODE_2;

		// STUFF THE SOURCE NUMBER
		ucaMSG_BUFF[OM2_IDX_SRC_SN_HI] = ucGetSerialNumMd(); //SRC num HI
		ucaMSG_BUFF[OM2_IDX_SRC_SN_LO] = ucGetSerialNumLo(); //SRC num LO

		// STUFF A BLANK DESTINATION
		ucaMSG_BUFF[OM2_IDX_DEST_SN_HI] = 0; //DEST addr HI
		ucaMSG_BUFF[OM2_IDX_DEST_SN_LO] = 0; //DEST addr LO

		// STUFF THE MSG SEQ NUMBER
		ucaMSG_BUFF[OM2_IDX_MSG_SEQ_NUM] = ucMSG_incMsgSeqNum();

		// STUFF THE COLLECTING AGENT NUMBER
		ucaMSG_BUFF[OM2_IDX_AGENT_NUM_HI] = ucGetSerialNumMd(); //AGENT num HI
		ucaMSG_BUFF[OM2_IDX_AGENT_NUM_LO] = ucGetSerialNumLo(); //AGENT num LO

		// INSERT THE COLLECTION TIME
		__bic_SR_register(GIE);					// No interrupts
		ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_XI] = ucCLK_TIME[0];
		ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_HI] = ucCLK_TIME[1];
		ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_MD] = ucCLK_TIME[2];
		ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_LO] = ucCLK_TIME[3];
		__bis_SR_register(GIE);					// Yes interrupts


		// POWER UP THE SD BOARD
		if(!ucSDCOM_power_up_SD_for_messaging())
			{
			vSERIAL_sout("ACTN: SD pwr up err\r\n", 21);
			}

		// READ TC_1 DATA
		ucGLOB_testByte++;
		if(ucSDCOM_do_complete_poweron_read_poweroff(SENSOR_TC_1, NO_POWER_CONTROL))
			{
			vSERIAL_sout("ACTN: SD msg err\r\n", 18);
			}
		// STUFF THE DATA
		ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM+0] = SENSOR_TC_1;
		ucaMSG_BUFF[OM2_IDX_DATA_0_HI+0] = ucaSDSPI_BUFF[3];
		ucaMSG_BUFF[OM2_IDX_DATA_0_LO+0] = ucaSDSPI_BUFF[4];
		ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM+0+3] = SENSOR_ONEWIRE_0;
		ucaMSG_BUFF[OM2_IDX_DATA_0_HI+0+3] = ucaSDSPI_BUFF[5];
		ucaMSG_BUFF[OM2_IDX_DATA_0_LO+0+3] = ucaSDSPI_BUFF[6];
		#if 1
		if((ucaSDSPI_BUFF[5] == 0) && (ucaSDSPI_BUFF[6] == 0))
			{
			if(ucGLOB_testByte != 255) ucGLOB_testByte++;
			}
		#endif

		#if 1
		vSERIAL_sout("TC_1=", 5);		//debug
		uiVal = ucaSDSPI_BUFF[3];  uiVal<<=8;  uiVal |= (ucaSDSPI_BUFF[4] & 0xFF);
		vSERIAL_IV16out((int)uiVal);
		vSERIAL_bout(',');
		uiVal = ucaSDSPI_BUFF[5];  uiVal<<=8;  uiVal |= (ucaSDSPI_BUFF[6] & 0xFF);
		vSERIAL_UI16out(uiVal);
		//vSERIAL_crlf();
		#endif


		// READ TC_2 DATA
		if(ucSDCOM_do_complete_poweron_read_poweroff(SENSOR_TC_2, NO_POWER_CONTROL))
			{
			vSERIAL_sout("ACTN: SD msg err\r\n", 18);
			}
		// STUFF THE DATA
		ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM+6] = SENSOR_TC_2;
		ucaMSG_BUFF[OM2_IDX_DATA_0_HI+6] = ucaSDSPI_BUFF[3];
		ucaMSG_BUFF[OM2_IDX_DATA_0_LO+6] = ucaSDSPI_BUFF[4];
		ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM+6+3] = SENSOR_ONEWIRE_0;
		ucaMSG_BUFF[OM2_IDX_DATA_0_HI+6+3] = ucaSDSPI_BUFF[5];
		ucaMSG_BUFF[OM2_IDX_DATA_0_LO+6+3] = ucaSDSPI_BUFF[6];

		#if 1
		if((ucaSDSPI_BUFF[5] == 0) && (ucaSDSPI_BUFF[6] == 0))
			{
			if(ucGLOB_testByte != 255) ucGLOB_testByte++;
			}
		#endif

		#if 1
		vSERIAL_sout("    TC_2=", 9);		//debug
		uiVal = ucaSDSPI_BUFF[3];  uiVal<<=8;  uiVal |= (ucaSDSPI_BUFF[4] & 0xFF);
		vSERIAL_IV16out((int)uiVal);
		vSERIAL_bout(',');
		uiVal = ucaSDSPI_BUFF[5];  uiVal<<=8;  uiVal |= (ucaSDSPI_BUFF[6] & 0xFF);
		vSERIAL_UI16out(uiVal);
		vSERIAL_crlf();
		#endif


		// STUFF THE MESSAGE SIZE-1 HERE
		ucaMSG_BUFF[OM2_IDX_EOM_IDX] = OM2_IDX_DATA_0_LO+6+3+2;

		// POWER DOWN THE SD BOARD
		vSDCOM_power_down_SD_from_messaging();

		PORTBbits.SD_SPI_SLAVE_READY_BIT = 0;
		TRISBbits.SD_SPI_SLAVE_READY_BIT = 0;

		PORTFbits.RF0 = 0;						//debug


		// COMPUTE THE CRC
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

		// STUFF THE MSG TO THE DISK
		//vL2SRAM_storeMsgToSram();


		#if 0
		vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);
		#endif

		vDELAY_wait100usTic(5000);		//500 ms
		vDELAY_wait100usTic(5000);		//500 ms
		//vDELAY_wait100usTic(10000);		//1000 ms


		}// END: while()

	}
	#endif			//TC2_DIAG_ENA






	#ifdef TC3_DIAG_ENA

	/************************  TC3 DIAGNOSTIC  ******************************
	*
	* Thermocouple test to follow SPI_ready line on power up of SD
	*
	* may change! with change to mini SD
	*
	************************************************************************/
	{

	uint uiVal;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("TC3: SPI ready line follower\n\r", 30);


//	vSERIAL_any_char_to_cont();


	while(TRUE)				//lint !e774
		{

		PORTFbits.RF0 = 1;						//debug

		// POWER UP THE SD BOARD
		if(!ucSDCOM_power_up_SD_for_messaging())
			{
			vSERIAL_sout("ACTN: SD pwr up err\r\n", 21);
			}

		_asm

st:		BTFSC	PORTB,2,0		//lint !e40 !e522 !e10
   		BSF		PORTG,4,0		//lint !e40 !e522 !e10
   		BTFSS	PORTB,2,0		//lint !e40 !e522 !e10
   		BCF		PORTG,4,0		//lint !e40 !e522 !e10
   		GOTO	st				//lint !e40 !e522 !e10

   		_endasm;


		// POWER DOWN THE SD BOARD
		vSDCOM_power_down_SD_from_messaging();

		PORTFbits.RF0 = 0;						//debug



		vDELAY_wait100usTic(5000);		//500 ms
		vDELAY_wait100usTic(5000);		//500 ms
		//vDELAY_wait100usTic(10000);		//1000 ms


		}// END: while()

	}
	#endif			//TC3_DIAG_ENA






	#ifdef TC4_DIAG_ENA

	/************************  TC4 DIAGNOSTIC  ******************************
	*
	* Test the show temp routine
	*
	************************************************************************/
	{

	int iVal;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("TC4: test the showOwireValue routine\r\n\n", 39);


	iVal = 561;					//= (35*16)+1
	vTHERMO_showOwireValue(iVal);
	vSERIAL_crlf();
	vSERIAL_any_char_to_cont();

	iVal = -561;
	vTHERMO_showOwireValue(iVal);
	vSERIAL_crlf();
	vSERIAL_any_char_to_cont();

	iVal = 562;
	vTHERMO_showOwireValue(iVal);
	vSERIAL_crlf();
	vSERIAL_any_char_to_cont();

	iVal = 563;
	vTHERMO_showOwireValue(iVal);
	vSERIAL_crlf();
	vSERIAL_any_char_to_cont();

	iVal = 564;
	vTHERMO_showOwireValue(iVal);
	vSERIAL_crlf();
	vSERIAL_any_char_to_cont();

	iVal = 575;
	vTHERMO_showOwireValue(iVal);
	vSERIAL_crlf();
	vSERIAL_any_char_to_cont();

	}
	#endif			//TC4_DIAG_ENA







	#ifdef T3TIMER1_DIAG_ENA

	/************************  T3TIMER1 DIAGNOSTIC  ******************************
	*
	* Check the T3 TIMER
	*
	*
	************************************************************************/
	{
//	uchar ucc;
	uchar ucStartTime[6];
	uchar ucEndTime[6];
	uint uiFirstT3read;
	uint uiNextT3read;
	uchar ucTmp;
	long lTime;


	// SHOW HEADER MESSAGE
	vSERIAL_sout("T3TIMER1: Test the Timer routine\r\n", 34);


	vTIME_copyWholeSysTimeToBytesInDtics(&ucStartTime[0]);

	lTime = lTIME_getSysTimeAsLong();
	ucTmp = ucTIME_setT3AlarmToSecMinus200ms(lTime+1L);

	if(ucTmp == 0)
		{
		vSERIAL_sout("Timer Ret= RUN\r\n", 16);
		}
	else
		{
		vSERIAL_sout("Timer Ret= BAD Ret = ", 21);
		vSERIAL_UIV8out(ucTmp);
		vSERIAL_crlf();
		}

	// CHECK FOR T3 TIMER TIC
	uiFirstT3read = uiTIME_getT3AsUint();
	vSERIAL_sout("uiFirstT3read= ", 15);
	vSERIAL_HB16out(  uiFirstT3read);
	vSERIAL_crlf();

	while(TRUE)							//lint !e774
		{
		uiNextT3read = uiTIME_getT3AsUint();
		if(uiFirstT3read == uiNextT3read)
			{
			vSERIAL_bout('.');
			continue;
			}

		break;

		}

	vSERIAL_sout("NextT3read= ", 12);
	vSERIAL_HB16out(uiNextT3read);
	vSERIAL_crlf();


	vSERIAL_sout("Waiting for T3 alarm...", 23);

	while(!ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT); //lint !e722
	vTIME_copyWholeSysTimeToBytesInDtics(&ucEndTime[0]);

	vSERIAL_sout("\r\nStart Time= ", 14);
	vSERIAL_HB6ByteOut(&ucStartTime[0]);
	vSERIAL_sout("  End Time= ", 12);
	vSERIAL_HB6ByteOut(&ucEndTime[0]);
	vSERIAL_sout("  Diff= ", 8);
	vTIME_showWholeTimeDiffInDuS(&ucStartTime[0], &ucEndTime[0], YES_CRLF);

	}
	#endif			//T3TIMER1_DIAG_ENA






	#ifdef LIGHT1_DIAG_ENA

	/************************  LIGHT1 diagnostic  **************************
	*
	*	LIGHT1 READ TEST
	*
	************************************************************************/
	{

	int iAD_raw;


	// SHOW HEADER MESSAGE
	vSERIAL_sout("LIGHT1: Light sensor read test\r\n", 32);


	vAD_init();					//turn on A/D

	for(;;)
		{

		// GET THE READING
		iAD_raw = (int)uiAD_read(PHOTO_1_AD_CHAN, AD_READ_AVE_OF_1);

		vSERIAL_I16out(iAD_raw);
		vSERIAL_sout(" (", 2);
		vSERIAL_HB16out((uint)iAD_raw);
		vSERIAL_bout(')');
		vSERIAL_crlf();

		}// END: for(;;)

	vAD_quit();

	}// END for(;;)

	#endif			//LIGHT1_DIAG_ENA





	#ifdef LNKBLK1_DIAG_ENA

	#include "LNKBLK.h"
	#include "L2SRAM.h"
	#include "SRAM.h"
	#include "TIME.h"


	/*******************  LNKBLK1 diagnostic  ******************************
	*
	* Test the base routines for LnkBlk Tbl
	*
	************************************************************************/
	{
	uchar ucFakeStblIdx;
	uchar ucFakeDesiredLink;
	usl uslFakeRandNum;
	long lNextMsgTime;




	// SHOW HEADER MESSAGE
	vSERIAL_sout("LnkBlk Diag\r\n", 13);

	vSERIAL_sout("LNKBLK_TBL_BASE_ADDR= ", 22);
	vSERIAL_IV32out((long)LNKBLK_TBL_BASE_ADDR);
	vSERIAL_crlf();

	vSERIAL_sout("ENTRYS_PER_LNKBLK_BLK_L= ", 25);
	vSERIAL_IV32out(ENTRYS_PER_LNKBLK_BLK_L);
	vSERIAL_crlf();

	vSERIAL_sout("BYTES_PER_LNKBLK_ENTRY_L= ", 26);
	vSERIAL_IV32out(BYTES_PER_LNKBLK_ENTRY_L);
	vSERIAL_crlf();

	vSERIAL_sout("LNKBLK_TBL_END_ADDR_PLUS_1= ", 28);
	vSERIAL_IV32out((long)LNKBLK_TBL_END_ADDR_PLUS_1);
	vSERIAL_crlf();

	vSERIAL_sout("GENERIC_SCHED_MAX_IDX= ", 23);
	vSERIAL_IV32out((long)GENERIC_SCHED_MAX_IDX);
	vSERIAL_crlf();




	vLNKBLK_clrSingleLnkBlk(0);

	vLNKBLK_zeroEntireLnkBlkTbl();

	// CHECK IF WRITING IN THE RIGHT SPOT IN SRAM
	vLNKBLK_writeSingleLnkBlkEntry(0,2,0x11223344);


	{
	long lii;
	ulong ulVal;
	uchar ucc;

	for(lii=LNKBLK_TBL_BASE_ADDR-4;  lii<LNKBLK_TBL_BASE_ADDR+(9*4); lii+=4)
		{
		ulVal = ulSRAM_read_B32((usl)lii);
		vSERIAL_I32out(lii);
		vSERIAL_sout(" = ", 3);
		vSERIAL_HB32out(ulVal);
		vSERIAL_crlf();
		}

	vSERIAL_crlf();



	for(ucc=0;  ucc<10;  ucc++)
		{
		ulVal = (ulong)lLNKBLK_readSingleLnkBlkEntry(0,ucc);
		vSERIAL_UI8out(ucc);
		vSERIAL_sout(" = ", 3);
		vSERIAL_HB32out(ulVal);
		vSERIAL_crlf();
		}

	vSERIAL_crlf();

	vLNKBLK_showSingleLnkBlk(0);
	vSERIAL_crlf();

	}


	ucFakeStblIdx = 24;
	ucFakeDesiredLink = (1<<3)+1;
	uslFakeRandNum = 0xD6A387;

	vSERIAL_sout("LinearSlot * 4 = ", 17);
	vSERIAL_HB32out((ulong)(lGLOB_lastAwakeLinearSlot * 4L));
	vSERIAL_sout("  =  ", 5);
	vTIME_showTime((lGLOB_lastAwakeLinearSlot * 4L));
	vSERIAL_crlf();

	// PUT THE FAKE RANDOM NUMBER INTO THE TABLE
	vL2SRAM_putStblEntry(	ucaRandTblNum[0],
							ucFakeStblIdx,
							uslFakeRandNum
							);
	vL2SRAM_putStblEntry(	ucaRandTblNum[1],
							ucFakeStblIdx,
							uslFakeRandNum
							);

	lNextMsgTime = lLNKBLK_retTimeForNewLnk(ucFakeStblIdx, ucFakeDesiredLink);
	vSERIAL_sout("Next Msg Time= ", 15);
	vSERIAL_HB32out((ulong)lNextMsgTime);
	vSERIAL_crlf();



	/******************************************************/
	ucFakeDesiredLink = (1<<3)+0;
	lNextMsgTime = lLNKBLK_retTimeForNewLnk(ucFakeStblIdx, ucFakeDesiredLink);
	vSERIAL_sout("Next Msg Time= ", 15);
	vSERIAL_HB32out((ulong)lNextMsgTime);
	vSERIAL_crlf();


	/******************************************************/
	ucFakeDesiredLink = (1<<3)+2;
	lNextMsgTime = lLNKBLK_retTimeForNewLnk(ucFakeStblIdx, ucFakeDesiredLink);
	vSERIAL_sout("Next Msg Time= ", 15);
	vSERIAL_HB32out((ulong)lNextMsgTime);
	vSERIAL_crlf();


	/******************************************************/
	ucFakeDesiredLink = (2<<3)+1;
	lNextMsgTime = lLNKBLK_retTimeForNewLnk(ucFakeStblIdx, ucFakeDesiredLink);
	vSERIAL_sout("Next Msg Time= ", 15);
	vSERIAL_HB32out((ulong)lNextMsgTime);
	vSERIAL_crlf();







Hang:
	goto Hang;

	}
	#endif			//LNKBLK1_DIAG_ENA




	#ifdef LNKBLK2_DIAG_ENA

	#include "LNKBLK.h"
	#include "L2SRAM.h"
//	#include "SRAM.h"
//	#include "TIME.h"


	/*******************  LNKBLK2 diagnostic  ******************************
	*
	* Test the base routines for LnkBlk Tbl
	*
	************************************************************************/
	{
	uchar ucFakeStblIdx;
	uchar ucFakeDesiredLnk;
	usl uslFakeRandNum;

	ucFakeStblIdx = 24;
	ucFakeDesiredLnk = (1<<3)+1;
	uslFakeRandNum = 0xD6A387;

	// PUT THE FAKE RANDOM NUMBER INTO THE TABLE
	vL2SRAM_putStblEntry(	ucaRandTblNum[0],
							ucFakeStblIdx,
							uslFakeRandNum
							);
	vL2SRAM_putStblEntry(	ucaRandTblNum[1],
							ucFakeStblIdx,
							uslFakeRandNum
							);

	vLNKBLK_showSingleLnkBlk(ucFakeStblIdx);
	vSERIAL_crlf();
	vLNKBLK_fillLnkBlkFromMultipleLnkReq(ucFakeStblIdx, ucFakeDesiredLnk);
	vLNKBLK_showSingleLnkBlk(ucFakeStblIdx);
	vSERIAL_crlf();


	ucFakeDesiredLnk = (1<<3)+4;
	vLNKBLK_showSingleLnkBlk(ucFakeStblIdx);
	vSERIAL_crlf();
	vLNKBLK_fillLnkBlkFromMultipleLnkReq(ucFakeStblIdx, ucFakeDesiredLnk);
	vLNKBLK_showSingleLnkBlk(ucFakeStblIdx);
	vSERIAL_crlf();

Hang:
	goto Hang;

	}
	#endif			//LNKBLK2_DIAG_ENA






	#ifdef LNKBLK3_DIAG_ENA

	#include "LNKBLK.h"
//	#include "L2SRAM.h"
//	#include "SRAM.h"
//	#include "TIME.h"

	/*******************  LNKBLK3 diagnostic  ******************************
	*
	* Test compute of Lnk Req from Sys Load
	*
	************************************************************************/
	{
	int iLoadParam;

//	for(iLoadParam=  -3;  iLoadParam <  500;  iLoadParam++)
//	for(iLoadParam= 500;  iLoadParam < 1000;  iLoadParam++)
	for(iLoadParam=1000;  iLoadParam < 1500;  iLoadParam++)
		{

		ucLNKBLK_computeMultipleLnkReqFromSysLoad(iLoadParam);	//lint !e534

		}// END: for(iLoadParam)

Hang:
	goto Hang;

	}
	#endif			//LNKBLK3_DIAG_ENA






	#ifdef LNKBLK4_DIAG_ENA

	#include "LNKBLK.h"
//	#include "L2SRAM.h"
//	#include "SRAM.h"
//	#include "TIME.h"

	/*******************  LNKBLK4 diagnostic  ******************************
	*
	* Test the write and read of lnkblk table
	*
	************************************************************************/
	{

	// CLR THE ENTIRE TBL
	vLNKBLK_zeroEntireLnkBlkTbl();

	// SHOW THE TABLE
	vSERIAL_crlf();
	vLNKBLK_showAllLnkBlkTblEntrys();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	// NOW WRITE A SINGLE ENTRY
	vLNKBLK_writeSingleLnkBlkEntry(6,0, 0x11111111);
	vLNKBLK_writeSingleLnkBlkEntry(6,3, 0x12222222);
	vLNKBLK_writeSingleLnkBlkEntry(7,0, 0x13333333);


	// NOW SHOW THE TABLE AGAIN
	vLNKBLK_showAllLnkBlkTblEntrys();
	vSERIAL_dash(20);
	vSERIAL_crlf();

Hang:
	goto Hang;

	}
	#endif			//LNKBLK4_DIAG_ENA







	#ifdef LINKUP_DCNT_1_ENABLED

	/*******************  LINKUPDCNT1 diagnostic  **************************
	*
	*	Diagnose the LinkupDcnt routine
	*
	*
	*
	*
	************************************************************************/
	{
	uchar ucc;
	uchar ucLinkupDcnt;
	const char *cStrPtr;
	uint uiTest;


	// SHOW HEADER MESSAGE 
	vSERIAL_sout("Linkup Dcnt Diag\r\n", 18);

	// CREATE TABLE HEADER
	vSERIAL_sout("Sender   Linkup\r\n", 17);
	vSERIAL_sout("LFactor   Dcnt \r\n", 17);

	for(ucc=0; ucc<=255;  ucc++)
		{
		ucLinkupDcnt = ucOPMODE_computeLinkupDcnt(ucc);
//		uiTest = uiOPMODE_computeLinkupDcnt(ucc);

		vSERIAL_colTab(3);
		vSERIAL_UIV8out(ucc);
		vSERIAL_colTab(11);

		cStrPtr = "Mult ";
		if(ucLinkupDcnt & 0x80) cStrPtr = "Skip ";
		vSERIAL_sout(cStrPtr, 5);

		vSERIAL_UIV8out(ucLinkupDcnt & 0x3F);
//		vSERIAL_UIV16out(uiTest);
		vSERIAL_crlf();

		if(ucc==255) break;
		}

	}// END for(;;)

LinkupDcnt1_hang:
	goto LinkupDcnt1_hang;

	#endif			//LINKUP_DCNT_1_ENABLED






	#ifdef MISC1_DIAG_ENA

	/************************  MISC1 diagnostic  **************************
	*
	*	MISC1 READ TEST
	*
	************************************************************************/
	{
	uchar ucc;
	uchar ucTestArray[4];
	ulong ulVal;
	usl uslVal;
	uint uiVal;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("MISC1: Test Misc Routines\r\n", 17);


	ucTestArray[0] = 0x11;
	ucTestArray[1] = 0x22;
	ucTestArray[2] = 0x33;
	ucTestArray[3] = 0x44;

	ulVal = ulMISC_buildLongFromBytes((uchar *)&ucTestArray[0]);
	vSERIAL_sout("LONG = ", 7);
	vSERIAL_HBV32out(ulVal);
	vSERIAL_crlf();

	uslVal = ulMISC_buildUlongFromBytes((uchar *)&ucTestArray[0]);
	vSERIAL_sout("USL = ", 6);
	vSERIAL_HBV24out(uslVal);
	vSERIAL_crlf();

	uiVal = uiMISC_buildIntFromBytes((uchar *)&ucTestArray[0]);
	vSERIAL_sout("UINT = ", 7);
	vSERIAL_HBV16out(uiVal);
	vSERIAL_crlf();


	ulVal = 0x44332211;
	vMISC_copyLongIntoBytes(ulVal, (uchar *)&ucTestArray[0]);
	for(ucc=0;  ucc<4;  ucc++)
		{
		vSERIAL_HB8out(ucTestArray[ucc]);
		}
	vSERIAL_crlf();


	uslVal = 0x332211;
	vMISC_copyUslIntoBytes(uslVal, (uchar *)&ucTestArray[0]);
	for(ucc=0;  ucc<4;  ucc++)
		{
		vSERIAL_HB8out(ucTestArray[ucc]);
		}
	vSERIAL_crlf();

	uiVal = 0x2211;
	vMISC_copyIntIntoBytes(uiVal, (uchar *)&ucTestArray[0]);
	for(ucc=0;  ucc<4;  ucc++)
		{
		vSERIAL_HB8out(ucTestArray[ucc]);
		}
	vSERIAL_crlf();




Exit_this:
	goto Exit_this;

	}
	#endif			//MISC1_DIAG_ENA






	#ifdef MSG1_DIAG_ENA
	{

	/*************  MSG1 diagnostic  ****************************************
	*
	* Test the SHOWmsgBuffer routine
	*
	**************************************************************************/
	// SHOW HEADER MESSAGE

	vSERIAL_sout("MSG1: Showing test message\r\n", 28);

	ucaMSG_BUFF[0]  = 0x1D; //size

	ucaMSG_BUFF[1]  = 0x06; //Msg Type

	ucaMSG_BUFF[2]  = 0x17; //Signature
	ucaMSG_BUFF[3]  = 0x95;

	ucaMSG_BUFF[4]  = 0xB4; //SRC ID
	ucaMSG_BUFF[5]  = 0x74;

	ucaMSG_BUFF[6]  = 0xB4; //Dest ID
	ucaMSG_BUFF[7]  = 0x93;

	ucaMSG_BUFF[8]  = 0x9B; //Seq Number

	ucaMSG_BUFF[9]  = 0x30; //load 30

	ucaMSG_BUFF[10] = 0xb4; //Agent
	ucaMSG_BUFF[11] = 0x74;

	ucaMSG_BUFF[12] = 0x0D; //Time
	ucaMSG_BUFF[13] = 0x3B;
	ucaMSG_BUFF[14] = 0xE0;
	ucaMSG_BUFF[15] = 0xf3;

	ucaMSG_BUFF[16] = 0x0b; //sensor 1 = LIGHT-1

	ucaMSG_BUFF[17] = 0x00;
	ucaMSG_BUFF[18] = 0x00;

	ucaMSG_BUFF[19] = 0x03; //sensor 2 = SD CODE VERSION

	ucaMSG_BUFF[20] = 0x02;
	ucaMSG_BUFF[21] = 0x07;

	ucaMSG_BUFF[22] = 0x0a;	//sensor 3 = TC-4

	ucaMSG_BUFF[23] = 0x03;
	ucaMSG_BUFF[24] = 0xda;

	ucaMSG_BUFF[25] = 0x13; //sensor 4 = ONEWIRE

	ucaMSG_BUFF[26] = 0x84;
	ucaMSG_BUFF[27] = 0x00;

	ucaMSG_BUFF[28] = 0x34; //Chksum
	ucaMSG_BUFF[29] = 0x42;

	while(TRUE)		//lint !e774
		{
		vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_COOKED);
		vSERIAL_any_char_to_cont();
		vSERIAL_crlf();
		}// END: while()

	}
	#endif // END: MSG1_DIAG_ENA




	#ifdef MSG2_DIAG_ENA
	{
//	#include "l2sram.h"
	#include "opmode.h"
	#include "l2fram.h"
	#include "time.h"
	#include "sensor.h"
	#include "misc.h"
	#include "delay.h"

	/*************  MSG2 diagnostic  ****************************************
	*
	* Test msg to flash & sram storage time
	*
	**************************************************************************/
	int ii;

	// SHOW HEADER MESSAGE 
	vSERIAL_sout("MSG2: Test msg to flash & sram storage time\r\n", 45);

	// BLINK THE LED SO WE KNOW THAT IT'S TIME TO GO
	vMISC_blinkLED(63);
	vMISC_setLED(0);

	// GIVE THE USER TIME TO HIT SINGLE ON THE SCOPE
	vDELAY_wait100usTic(50000);

	// TURN ON THE LED (OUR MEASUREMENT TOOL)
	vMISC_setLED(1);

	// STORE THE MSG 10 TIMES, SO THE SIGNAL WILL BE THERE FOR A WHILE
	for(ii = 0; ii < 10; ++ii)
		{
		// MAKE A MESSAGE AND STORE IT
		vOPMODE_buildMsg_OM2(
				0,
				0,
				ucMSG_incMsgSeqNum(),
				0,
				uiL2FRAM_getSnumLo16AsUint(),
				lTIME_getSysTimeAsLong(),
				2,
				SENSOR_ROM2_LINK_INFO,
				32145,
				SENSOR_ROM2_INFO_REASON,
				OM2_MISSED,
				0,
				0,
				0,
				0);

		vL2FLASH_storeMsgToFlash();
		vL2SRAM_storeMsgToSram();

		}// END: for(ii)

	// SHUT OFF THE LED, END OF THE INTERVAL
	vMISC_setLED(0);

	}
	#endif // END: MSG2_DIAG_ENA





	#ifdef MSG3_DIAG_ENA
	{
	#include "opmode.h" 	//Operational mode routines
	#include "l2fram.h"		//Level 2 Ferro RAM
	#include "time.h"		//Time handling routines
	#include "sensor.h"		//sensor definitions
	#include "report.h"		//internal reporting functions

	extern uchar ucGLOB_curMsgSeqNum;

	/*************  MSG3 diagnostic  ****************************************
	*
	* Test the SHOWmsgBuffer routine
	*
	**************************************************************************/
	// SHOW HEADER MESSAGE
	uchar ucc;
	uchar ucRet;

	vSERIAL_sout("MSG3: Test report routines\r\n", 28);

#if 0
	vOPMODE_buildMsg_OM2(
			0,
			0,
			25,
			0,
			uiL2FRAM_getSnumLo16AsUint(),
			lTIME_getSysTimeAsLong(),
			2,
			0x25,
			0x2145,
			0x26,
			0x2,
			0,
			0,
			0,
			0
			);

//	vREPORT_logReport(0,0);		//none
//	vREPORT_logReport(1,0);		//radio only
//	vREPORT_logReport(0,1);		//flash only
//	vREPORT_logReport(1,1);		//radio and flash

	// CLR THE MSG BUFFER
	for(ucc=0;  ucc<MAX_MSG_SIZE;  ucc++) { ucaMSG_BUFF[ucc] = 0; }

	// SHOW MSG BUFFER
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_RAW);

#endif

#if 1
	// CLR THE MSG BUFFER
	for(ucc=0;  ucc<MAX_MSG_SIZE;  ucc++) { ucaMSG_BUFF[ucc] = 0; }

//	// SHOW MSG BUFFER
//	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_RAW);

	// FILL MSG BUFFER
	vOPMODE_buildMsg_OM2(
			0,								//LAST_PKT_BIT & NO_DATA_PKT_BIT
			0,								//Dest Serial num
			25,								//Msg Seq Num
			0,								//Load Factor
			uiL2FRAM_getSnumLo16AsUint(),	//Agent SN
			lTIME_getSysTimeAsLong(),		//Collection Time
			4,								//Data Count
			0x01,							//Sensor 0 number
			0x2145,							//Sensor 0 data
			0x02,							//Sensor 1 number
			0x0303,							//Sensor 1 data
			0x04,							//Sensor 2 number
			0x0505,							//Sensor 2 data
			0x06,							//Sensor 3 number
			0x0707							//Sensor 3 data
			);

	// SHOW MSG BUFFER
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_RAW);

	// CLR THE MSG BUFFER
	for(ucc=0;  ucc<MAX_MSG_SIZE;  ucc++) { ucaMSG_BUFF[ucc] = 0; }


#endif

#if 1

	ucGLOB_curMsgSeqNum = 24;

	vREPORT_buildReportAndLogIt(
			0x01,							//Sensor 0 number
			0x2145,							//Sensor 0 data
			0x02,							//Sensor 1 number
			0x0303,							//Sensor 1 data
			0,								//report to radio
			0								//report to flash
			);

	// SHOW MSG BUFFER
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_RAW);

	// CLR THE MSG BUFFER
	for(ucc=0;  ucc<MAX_MSG_SIZE;  ucc++) { ucaMSG_BUFF[ucc] = 0; }


#endif

#if 0
	ucGLOB_curMsgSeqNum = 24;

	vREPORT_buildEmptyReportHull();

	// SHOW MSG BUFFER
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_RAW);

	// CLR THE MSG BUFFER
	for(ucc=0;  ucc<MAX_MSG_SIZE;  ucc++) { ucaMSG_BUFF[ucc] = 0; }

#endif


#if 1
	// CLR THE MSG BUFFER
	for(ucc=0;  ucc<MAX_MSG_SIZE;  ucc++) { ucaMSG_BUFF[ucc] = 0; }

	ucGLOB_curMsgSeqNum = 24;
	vREPORT_buildEmptyReportHull();

	ucRet = ucREPORT_addSensorDataToExistingReport(
			0x01,							//Sensor 0 number
			0x2145,							//Sensor 0 data
			0x02,							//Sensor 1 number
			0x0303							//Sensor 1 data
			);

	vSERIAL_sout("ucRet= ", 7);
	vSERIAL_HB8out(ucRet);
	vSERIAL_crlf();
	vSERIAL_crlf();

	// SHOW MSG BUFFER
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_RAW);

	ucRet = ucREPORT_addSensorDataToExistingReport(
			0x04,							//Sensor 2 number
			0x0505,							//Sensor 2 data
			0x06,							//Sensor 3 number
			0x0707							//Sensor 3 data
			);

	vSERIAL_sout("ucRet= ", 7);
	vSERIAL_HB8out(ucRet);
	vSERIAL_crlf();
	vSERIAL_crlf();

	// SHOW MSG BUFFER
	vMSG_showMsgBuffer(YES_CRLF,SHOW_MSG_RAW);

	// CLR THE MSG BUFFER
	for(ucc=0;  ucc<MAX_MSG_SIZE;  ucc++) { ucaMSG_BUFF[ucc] = 0; }

#endif

	}
	#endif // END: MSG3_DIAG_ENA





	#ifdef NUMCMD1_DIAG_ENA

	#include "opmode.h" 	//Operational mode routines
	#include "l2fram.h"		//Level 2 Ferro RAM
	#include "time.h"		//Time handling routines
	#include "sensor.h"		//sensor definitions
	#include "report.h"		//internal reporting functions
	#include "numcmd.h"		//numeric command routines

	/*************  NUMCMD1 diagnostic  ****************************************
	*
	* Test the showCmdList routine
	*
	**************************************************************************/
	{
	uchar ucc;
	uchar ucRet;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("NUMCMD1: Test NUMCMD routines\r\n", 31);

	}
	#endif // END: NUMCMD1_DIAG_ENA






	#ifdef NUMERIC1_OUTPUT_DIAG_ENA
	{
	/****************  TEST NUMERIC OUTPUT ROUTINES ***************
	*
	* Numeric output test
	*
	*
	*****************************************************************/

	uchar uc;
	uint ui;
	unsigned long ull;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("NUMERIC1: Numeric Output Test Routine\r\n", 39);

	/********************  HB8out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" HB8out() ", 10);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("HB8out(0xFF)              FF = ", 31);
	vSERIAL_HB8out(0xFF);
	vSERIAL_crlf();

	vSERIAL_sout("HB8out(0x1F)              1F = ", 31);
	vSERIAL_HB8out(0x1F);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534


	/********************  UI8out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" UI8out() ", 10);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 23);
	for(uc=126;  uc!=131;  uc++)
		{
		vSERIAL_sout("UI8out()               ", 23);

		vSERIAL_bout('(');
		vSERIAL_HB8out(uc);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UI8out(uc);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(uc=254;  uc!=3;  uc++)
		{
		vSERIAL_sout("UI8out()               ", 23);

		vSERIAL_bout('(');
		vSERIAL_HB8out(uc);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UI8out(uc);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();



	/********************  UIV8out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" UIV8out() ", 11);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("IV8out: 1 to 5 digits:\r\n", 34);
	uc = 1;
	vSERIAL_sout("IV8out() 1 digit        ", 24);
	vSERIAL_I8out(uc);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	uc = 11;
	vSERIAL_sout("IV8out() 2 digit        ", 24);
	vSERIAL_I8out(uc);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	uc = 111;
	vSERIAL_sout("IV8out() 3 digit        ", 24);
	vSERIAL_I8out(uc);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 23);
	for(uc=126;  uc!=131;  uc++)
		{
		vSERIAL_sout("UIV8out()              ", 23);

		vSERIAL_bout('(');
		vSERIAL_HB8out(uc);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UIV8out(uc);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(uc=254;  uc!=3;  uc++)
		{
		vSERIAL_sout("UIV8out()              ", 23);

		vSERIAL_bout('(');
		vSERIAL_HB8out(uc);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UIV8out(uc);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();



	/********************  I8out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" I8out() ", 9);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("Sign rollover from BOTTOM test:\r\n", 33);
	for(uc=126;  uc!=131;  uc++)
		{
		vSERIAL_sout("I8out()             ", 20);

		vSERIAL_bout('(');
		vSERIAL_HB8out(uc);
		vSERIAL_sout(") ", 2);

		vSERIAL_UI8out(uc);								//lint !e713
		vSERIAL_sout(" = ", 3);
		vSERIAL_I8out(uc);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_sout("Sign rollover at TOP test:\r\n", 28);
	for(uc=254;  uc!=3;  uc++)
		{
		vSERIAL_sout("I8out()             ", 20);

		vSERIAL_bout('(');
		vSERIAL_HB8out(uc);
		vSERIAL_sout(") ", 2);

		vSERIAL_UI8out(uc);								//lint !e713
		vSERIAL_sout(" = ", 3);
		vSERIAL_I8out(uc);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();


	/********************  IV8out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" IV8out() ", 10);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("IV8out: 1 to 3 digits:\r\n", 24);
	uc = 1;
	vSERIAL_sout("IV8out() 1 digit        ", 24);
	vSERIAL_I8out(uc);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	uc = 11;
	vSERIAL_sout("IV8out() 2 digit        ", 24);
	vSERIAL_I8out(uc);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	uc = 111;
	vSERIAL_sout("IV8out() 3 digit        ", 24);
	vSERIAL_I8out(uc);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	uc = (uchar) -1;
	vSERIAL_sout("IV8out() 1 digit (-)    ", 24);
	vSERIAL_I8out(uc);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	uc = (uchar) -11;
	vSERIAL_sout("IV8out() 2 digit (-)    ", 24);
	vSERIAL_I8out(uc);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	uc = (uchar) -111;
	vSERIAL_sout("IV8out() 3 digit (-)    ", 24);
	vSERIAL_I8out(uc);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV8out(uc);								//lint !e713
	vSERIAL_crlf();

	vSERIAL_sout("Sign rollover from BOTTOM test:\r\n", 33);
	for(uc=126;  uc!=131;  uc++)
		{
		vSERIAL_sout("IV8out()           ", 19);

		vSERIAL_bout('(');
		vSERIAL_HB8out(uc);
		vSERIAL_sout(") ", 2);

		vSERIAL_I8out(uc);								//lint !e713
		vSERIAL_sout(" = ", 3);
		vSERIAL_IV8out(uc);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_sout("Sign rollover at TOP test:\r\n", 28);
	for(uc=254;  uc!=3;  uc++)
		{
		vSERIAL_sout("IV8out()           ", 19);

		vSERIAL_bout('(');
		vSERIAL_HB8out(uc);
		vSERIAL_sout(") ", 2);

		vSERIAL_I8out(uc);								//lint !e713
		vSERIAL_sout(" = ", 3);
		vSERIAL_IV8out(uc);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();


	/********************  HB16out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" HB16out() ", 11);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	ui = 0x0000;
	vSERIAL_sout("HB16out(0x0000)         ", 24);
	vSERIAL_HB16out(ui);								//
	vSERIAL_sout(" = ", 3);
	vSERIAL_HB16out(ui);
	vSERIAL_crlf();

	ui = 0x0001;
	vSERIAL_sout("HB16out(0x0001)         ", 24);
	vSERIAL_HB16out(ui);								//
	vSERIAL_sout(" = ", 3);
	vSERIAL_HB16out(ui);
	vSERIAL_crlf();

	ui = 0xFFFF;
	vSERIAL_sout("HB16out(0xFFFF)         ", 24);
	vSERIAL_HB16out(ui);								//
	vSERIAL_sout(" = ", 3);
	vSERIAL_HB16out(ui);
	vSERIAL_crlf();

	ui = 0x7FFF;
	vSERIAL_sout("HB16out(0x7FFF)         ", 24);
	vSERIAL_HB16out(ui);								//
	vSERIAL_sout(" = ", 3);
	vSERIAL_HB16out(ui);
	vSERIAL_crlf();

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();


	/********************  UI16out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" UI16out() ", 11);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 23);
	for(ui=32766;  ui!=32770;  ui++)
		{
		vSERIAL_sout("UI16out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB16out(ui);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UI16out(ui);							//
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(ui=65534;  ui!=3;  ui++)
		{
		vSERIAL_sout("UI16out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB16out(ui);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UI16out(ui);								//
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();

	/********************  UIV16out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" UIV16out() ", 12);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("UIV16out: 1 to 5 digits:\r\n", 28);
	ui = 1;
	vSERIAL_sout("UIV16out() 1 digit          ", 28);
	vSERIAL_UI16out(ui);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = 11;
	vSERIAL_sout("UIV16out() 2 digit          ", 28);
	vSERIAL_UI16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = 111;
	vSERIAL_sout("UIV16out() 3 digit          ", 28);
	vSERIAL_UI16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = 1111;
	vSERIAL_sout("UIV16out() 4 digit          ", 28);
	vSERIAL_UI16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = 11111;
	vSERIAL_sout("UIV16out() 5 digit          ", 28);
	vSERIAL_UI16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV16out(ui);								//lint !e713
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 13);
	for(ui=32766;  ui!=32770;  ui++)
		{
		vSERIAL_sout("UIV16out()           ", 21);

		vSERIAL_bout('(');
		vSERIAL_HB16out(ui);
		vSERIAL_sout(") ", 2);

		vSERIAL_UI16out(ui);								//lint !e713
		vSERIAL_sout(" = ", 3);
		vSERIAL_UIV16out(ui);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_sout("TOP rollover test:\r\n");
	for(ui=65534;  ui!=3;  ui++)
		{
		vSERIAL_sout("UIV16out()           ", 21);

		vSERIAL_bout('(');
		vSERIAL_HB16out(ui);
		vSERIAL_sout(") ", 2);

		vSERIAL_UI16out(ui);								//lint !e713
		vSERIAL_sout(" = ", 3);
		vSERIAL_UIV16out(ui);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();

	/********************  I16out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" I16out() ", 10);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 23);
	for(ui=32766;  ui!=32770;  ui++)
		{
		vSERIAL_sout("I16out()               ", 23);

		vSERIAL_bout('(');
		vSERIAL_HB16out(ui);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_I16out(ui);							//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(ui=65534;  ui!=3;  ui++)
		{
		vSERIAL_sout("I16out()               ", 23);

		vSERIAL_bout('(');
		vSERIAL_HB16out(ui);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_I16out(ui);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();

	/********************  IV16out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" IV16out() ", 11);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("IV16out: 1 to 5 digits:\r\n", 25);
	ui = 1;
	vSERIAL_sout("IV16out() 1 digit          ", 27);
	vSERIAL_UI16out(ui);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = 11;
	vSERIAL_sout("IV16out() 2 digit          ", 27);
	vSERIAL_UI16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = 111;
	vSERIAL_sout("IV16out() 3 digit          ", 27);
	vSERIAL_UI16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = 1111;
	vSERIAL_sout("IV16out() 4 digit          ", 27);
	vSERIAL_UI16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = 11111;
	vSERIAL_sout("IV16out() 5 digit          ", 27);
	vSERIAL_UI16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();


	ui = (uint)-1;
	vSERIAL_sout("IV16out() 1 digit         ", 27);
	vSERIAL_I16out(ui);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = (uint)-11;
	vSERIAL_sout("IV16out() 2 digit         ", 27);
	vSERIAL_I16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = (uint)-111;
	vSERIAL_sout("IV16out() 3 digit         ", 27);
	vSERIAL_I16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = (uint)-1111;
	vSERIAL_sout("IV16out() 4 digit         ", 27);
	vSERIAL_I16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();

	ui = (uint)-11111;
	vSERIAL_sout("IV16out() 5 digit         ", 27);
	vSERIAL_I16out(ui);					  			//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV16out(ui);								//lint !e713
	vSERIAL_crlf();


	vSERIAL_sout("BOTTOM rollover test:\r\n", 23);
	for(ui=32766;  ui!=32770;  ui++)
		{
		vSERIAL_sout("IV16out()           ", 20);

		vSERIAL_bout('(');
		vSERIAL_HB16out(ui);
		vSERIAL_sout(") ", 2);

		vSERIAL_UI16out(ui);								//lint !e713
		vSERIAL_sout(" = ", 3);
		vSERIAL_IV16out(ui);								//lint !e713
		vSERIAL_crlf();

		}// END: for()

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(ui=65534;  ui!=3;  ui++)
		{
		vSERIAL_sout("UIV16out()           ", 21);

		vSERIAL_bout('(');
		vSERIAL_HB16out(ui);
		vSERIAL_sout(") ", 2);

		vSERIAL_UI16out(ui);								//lint !e713
		vSERIAL_sout(" = ", 3);
		vSERIAL_IV16out(ui);								//lint !e713
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();


	/********************  HB32out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" HB32out() ", 11);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(ull=0xFFFFFFFE;  ull!=0x00000003;  ull++)
		{
		vSERIAL_sout("HB32out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_HB32out(ull);								//
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();


	/********************  UI32out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" UI32out() ", 11);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 23);
	for(ull=0x7FFFFFFE;  ull!=0x80000002;  ull++)
		{
		vSERIAL_sout("UI32out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UI32out(ull);							//
		vSERIAL_crlf();

		}// END: for()

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(ull=0xFFFFFFFE;  ull!=0x00000003;  ull++)
		{
		vSERIAL_sout("UI32out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UI32out(ull);								//
		vSERIAL_crlf();

		}// END: for()

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();

	/********************  UIV32out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" UIV32out() ", 12);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("UIV32out: 1 to 10 digits:\r\n", 27);
	ull = 1;
	vSERIAL_sout("UIV32out() 1 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 11;
	vSERIAL_sout("UIV32out() 2 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 111;
	vSERIAL_sout("UIV32out() 3 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 1111;
	vSERIAL_sout("UIV32out() 4 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 11111;
	vSERIAL_sout("UIV32out() 5 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 111111;
	vSERIAL_sout("UIV32out() 6 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 1111111;
	vSERIAL_sout("UIV32out() 7 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 11111111;
	vSERIAL_sout("UIV32out() 8 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 111111111;
	vSERIAL_sout("UIV32out() 9 digit          ", 28);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 1111111111;
	vSERIAL_sout("UIV32out() 10 digit         ", 29);
	vSERIAL_UI32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_UIV32out(ull);								//lint !e713
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 23);
	for(ull=0x7FFFFFFE;  ull!=0x80000003;  ull++)
		{
		vSERIAL_sout("UI32out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UI32out(ull);							//
		vSERIAL_crlf();

		}// END: for()

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(ull=0xFFFFFFFE;  ull!=0x00000003;  ull++)
		{
		vSERIAL_sout("UI32out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_UI32out(ull);								//
		vSERIAL_crlf();

		}// END: for() 

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();

	/********************  I32out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" I32out() ", 11);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 23);
	for(ull=0x7FFFFFFE;  ull!=0x80000003;  ull++)
		{
		vSERIAL_sout("I32out()               ", 23);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_I32out(ull);							//lint !e713
		vSERIAL_crlf();

		}// END: for()

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(ull=0xFFFFFFFE;  ull!=0x00000003;  ull++)
		{
		vSERIAL_sout("I32out()               ", 23);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_I32out(ull);							//lint !e713
		vSERIAL_crlf();

		}// END: for()

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();

	/********************  IV32out tests  ************************************/

	vSERIAL_dash(20);
	vSERIAL_sout(" IV32out() ", 11);
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("IV32out: 1 to 10 digits:\r\n", 26);
	ull = 1;
	vSERIAL_sout("IV32out() 1 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 11;
	vSERIAL_sout("IV32out() 2 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 111;
	vSERIAL_sout("IV32out() 3 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 1111;
	vSERIAL_sout("IV32out() 4 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 11111;
	vSERIAL_sout("IV32out() 5 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 111111;
	vSERIAL_sout("IV32out() 6 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 1111111;
	vSERIAL_sout("IV32out() 7 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 11111111;
	vSERIAL_sout("IV32out() 8 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 111111111;
	vSERIAL_sout("IV32out() 9 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = 1111111111;
	vSERIAL_sout("IV32out() 10 digit         ", 28);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	 				/******  NEGATIVES  ********/
	ull = (unsigned long)-1L;											//lint !e713 !e640
	vSERIAL_sout("IV32out() 1 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-11L;											//lint !e713 !e640
	vSERIAL_sout("IV32out() 2 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-111L;											//lint !e713  !e640
	vSERIAL_sout("IV32out() 3 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-1111L;										//lint !e713  !e640
	vSERIAL_sout("IV32out() 4 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-11111L;										//lint !e713 !e640
	vSERIAL_sout("IV32out() 5 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-111111L;										//lint !e713 !e640
	vSERIAL_sout("IV32out() 6 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-1111111L;										//lint !e713 !e640
	vSERIAL_sout("IV32out() 7 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-11111111L;									//lint !e713 !e640
	vSERIAL_sout("IV32out() 8 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-111111111L;									//lint !e713 !e640
	vSERIAL_sout("IV32out() 9 digit          ", 27);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	ull = (unsigned long)-1111111111L;									//lint !e713 !e640
	vSERIAL_sout("IV32out() 10 digit         ", 28);
	vSERIAL_I32out(ull);								//lint !e713
	vSERIAL_sout(" = ", 3);
	vSERIAL_IV32out(ull);								//lint !e713
	vSERIAL_crlf();

	vSERIAL_sout("BOTTOM rollover test:\r\n", 27);
	for(ull=0x7FFFFFFE;  ull!=0x80000003;  ull++)
		{
		vSERIAL_sout("IV32out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_IV32out((long)ull);							//
		vSERIAL_crlf();

		}// END: for()

	vSERIAL_sout("TOP rollover test:\r\n", 20);
	for(ull=0xFFFFFFFE;  ull!=0x00000003;  ull++)
		{
		vSERIAL_sout("IV32out()               ", 24);

		vSERIAL_bout('(');
		vSERIAL_HB32out(ull);
		vSERIAL_sout(") ", 2);

		vSERIAL_sout(" = ", 3);
		vSERIAL_IV32out((long)ull);								//
		vSERIAL_crlf();

		}// END: for()

	vSERIAL_crlf();
	vSERIAL_sout("MORE....", 8);
	ucSERIAL_bin();											//lint !e534
	vSERIAL_crlf();


	vSERIAL_sout("TEST EXIT\r\n", 11);

Hang: goto Hang;

	}
	#endif  // NUMERIC1_OUTPUT_DIAG_ENA






	#ifdef CRC1_DIAG_ENA
	{
	#include "crc.h"		//calculate the crc


	/***************  TEST THE CRC ROUTINE  *******
	*
	* This test checks the CRC already calculated at the bottom of the
	* msg.  it it is not correct then the routine is not working.
	*
	*
	*************************************************/

	uchar uc;
	uchar ucLimit;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("CRC1: CRC calculation test\r\n", 28);


	ucaMSG_BUFF[GMH_IDX_EOM_IDX] = 11;			//msg last used location ptr
	ucaMSG_BUFF[1] = 02;			//msg type
	ucaMSG_BUFF[2] = 00;
	ucaMSG_BUFF[3] = 02;
	ucaMSG_BUFF[4] = 00;
	ucaMSG_BUFF[5] = 01;
	ucaMSG_BUFF[6] = 00;
	ucaMSG_BUFF[7] = 00;
	ucaMSG_BUFF[8] = 00;
	ucaMSG_BUFF[9] = 00;
	ucaMSG_BUFF[10] = 0xBB;
	ucaMSG_BUFF[11] = 0xCC;

AGAIN:

	// DUMP THE MSG
	vSERIAL_sout("Dumping contents of msg before call:\r\n", 38);

	ucLimit = (ucaMSG_BUFF[GMH_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK);

	vSERIAL_bout('B');
	vSERIAL_bout('=');
	for(uc=0;  uc<=ucLimit;  uc++)
		{
		vSERIAL_HB8out(ucaMSG_BUFF[uc]);
		if(uc != ucLimit)
			{
			vSERIAL_bout(',');
			}
		else
			{
			vSERIAL_crlf();
			}
		}


	vSERIAL_sout("ABOUT TO CALL SEND CRC..... ", 28);
	if(!ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND))
		{
		vSERIAL_sout("SEND CRC RET = BAD\r\n", 20);
		}
	else
		{
		vSERIAL_sout("SEND CRC RET = GOOD\r\n", 21);
		}


	vSERIAL_sout("CRC= ", 5);
	vSERIAL_HB8out(ucaMSG_BUFF[10]);
	vSERIAL_HB8out(ucaMSG_BUFF[11]);
	vSERIAL_crlf();


	// DUMP THE MSG
	vSERIAL_sout("Dumping contents of msg after call:\r\n", 37);
	ucLimit = (ucaMSG_BUFF[GMH_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK);

	vSERIAL_bout('A');
	vSERIAL_bout('=');
	for(uc=0;  uc<=ucLimit;  uc++)
		{
		vSERIAL_HB8out(ucaMSG_BUFF[uc]);
		if(uc != ucLimit)
			{
			vSERIAL_bout(',');
			}
		else
			{
			vSERIAL_crlf();
			}
		}

	vSERIAL_sout("any chr to continue  ", 21);
	ucSERIAL_bin();							//lint !e534
	vSERIAL_crlf();

//	goto AGAIN;


	// NOW CHECK THE MSG FOR RECEIVE
	vSERIAL_sout("ABOUT TO CALL RECEIVE CRC..... ", 31);
	if(!ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_REC))
		{
		vSERIAL_sout("WUPS2\r\n", 7);
		}
	else
		{
		vSERIAL_sout("RECEIVE CRC is GOOD!\r\n", 22);
		}


	// DUMP THE MSG
	vSERIAL_sout("Dumping contents of msg after calling receive:\r\n", 48);
	ucLimit = (ucaMSG_BUFF[GMH_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK);

	vSERIAL_bout('R');
	vSERIAL_bout('=');
	for(uc=0;  uc<=ucLimit;  uc++)
		{
		vSERIAL_HB8out(ucaMSG_BUFF[uc]);
		if(uc != ucLimit)
			{
			vSERIAL_bout(',');
			}
		else
			{
			vSERIAL_crlf();
			}
		}

	vSERIAL_sout("CRC = ", 6);
	vSERIAL_HB8out(ucaMSG_BUFF[10]);
	vSERIAL_HB8out(ucaMSG_BUFF[11]);
	vSERIAL_crlf();

	vSERIAL_sout("any chr to continue  ", 21);
	ucSERIAL_bin();							//lint !e534
	vSERIAL_crlf();

	goto AGAIN;

	}
	#endif // END: CRC1_DIAG_ENA




	#if 0
	/***********  TEST THE RANDOM NUMBER GENERATOR  *********
	*
	*
	*
	********************************************************/
	{
	//NOTE: First few values of RAND_HI when using a seed of FF,FF,FF
	//FF,82,00,7F,9A,7D,E7,5D,82,1D,45


	// SHOW HEADER MESSAGE 
	vSERIAL_sout(": Random number generator test\r\n", 32);


	ucRAND_NUM[RAND_HI] = 0xFF;
	ucRAND_NUM[RAND_MD] = 0xFF;
	ucRAND_NUM[RAND_LO] = 0xFF;

	for(;;)
		{
		vSERIAL_HB8out(ucRAND_getRolledMidSysSeed());
		vSERIAL_crlf();

		vSERIAL_sout("any chr to continue  ", 21);
		ucSERIAL_bin();		//lint !e534
		vSERIAL_crlf();

		}// END: for()

	}
	#endif






	#if 0
	/******  XMIT ONLY / RECEIVER ONLY  ****
	*
	* This diagnostic has a send only component and a receive only component
	*
	* There is no chksum used in this diagnostic
	*
	****************************************/

	{
	uchar ucCount;
	uint uiCount;


	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Radio xmit and rec test\r\n", 27);


	/********************  SEND  **************************/

	// IF BUTTON IS PUSHED -- CALL THE TRANSMIT ROUTINE
	if(ucMISC_buttonHit())
		{
		vMISC_setLED(1);							//tell we are transmitting

		vSERIAL_sout("XMIT\r\n", 6);				//tell we are transmitting

		for(ucCount=0; ;  ucCount++)
			{
			// SETUP THE MESSAGE TO SEND
			ucaMSG_BUFF[GMH_IDX_EOM_IDX] = 1;						//last msg entry
			ucaMSG_BUFF[1] = 'B';					//msg = the letter 'B'

			// SEND THE MESSAGE
			vRADIO_xmit_msg();

			vDELAY_wait100usTic(4000);					//wait

			}
		}


	/********************  RECEIVE  ************************/

	vSERIAL_sout("REC\r\n", 5);					//tell we are receiving
	vSetLED(0);

	while(TRUE)
		{
		vRADIO_start_receiver();					//startup the receiver

		for(uiCount=0;  ;  uiCount++)
			{
			if(!ucRADIO_chk_rec_busy()) break;		//wait for a msg

			// GIVE A PERIODIC POKE TO SHOW WORK
			if(uiCount & 0x0400)
				{
				vSERIAL_bout('.');
				}

			}// END: for()

		if(ucaMSG_BUFF[1] >= 0x20)
			{
			vSERIAL_bout(ucaMSG_BUFF[1]);			//show the received char
			}
		else
			{
			vSERIAL_bout('-');
			}

		}// END: while()
	}
	#endif







	#ifdef RADIO0_DIAG_ENA

	/********  WIGGLE RADIO BITS ********
	*
	*
	* change! to radio may mean change here
	*
	**************************************/
	{
	uchar ucBitToTest;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("RADIO0: Wiggle radio bits\r\n", 27);

	#define IDX_RADIO_STANDBY_BIT			0
	#define IDX_RADIO_TRANSMIT_DATA_BIT		1
	#define IDX_RADIO_ANTENNA_BIT			2
	#define IDX_RADIO_MODE_BIT				3


//	ucBitToTest = IDX_RADIO_STANDBY_BIT;
//	ucBitToTest = IDX_RADIO_TRANSMIT_DATA_BIT;
	ucBitToTest = IDX_RADIO_ANTENNA_BIT;
//	ucBitToTest = IDX_RADIO_MODE_BIT;


	while(TRUE)							//lint !e716 !e774
		{
		switch(ucBitToTest)
			{
			case 0:
				PORTJbits.RADIO_STANDBY_BIT = 0;
				break;
			case 1:
				PORTGbits.RADIO_TRANSMIT_BIT = 0;
				break;
			case 2:
				PORTGbits.RADIO_ANTENNA_BIT = 0;
				break;
			case 3:
				PORTJbits.RADIO_MODE_BIT = 0;
				break;
			default:
				break;

			}// END: switch()


		vDELAY_wait4us();


		switch(ucBitToTest)
			{
			case 0:
				PORTJbits.RADIO_STANDBY_BIT = 1;
				break;
			case 1:
				PORTGbits.RADIO_TRANSMIT_BIT = 1;
				break;
			case 2:
				PORTGbits.RADIO_ANTENNA_BIT = 1;
				break;
			case 3:
				PORTJbits.RADIO_MODE_BIT = 1;
				break;
			default:
				break;

			}// END: switch()


		vDELAY_wait4us();

		}// END: while() 

	}
	#endif 	// RADIO0_DIAG_ENA







	#ifdef RADIO1_DIAG_ENA

	/************  RADIO TERMINAL TEST LOOP  *************
	*
	* This is the main loop which listens for
	* radio transmission and listens for
	* keyboard input.
	*
	* change! to radio may mean change here
	*************************************************/
	{
	uchar ucTmp;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("RADIO1: Radio transmit and receive test\r\n", 41);


	// STARTUP THE RADIO RECEIVER
	vRADIO_start_receiver();

	while(TRUE)							//lint !e716 !e774
		{
		// CHECK IF RECEIVER IS IDLE
		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_R_BUSY_BIT == 0)
			{
			// CHECK FOR ABORT
			if((ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_ABORT_BIT == 1) ||
			   (ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT == 0))
				{
				// WAS ABORTED OR NO MSG -- RESTART THE RECEIVER -- AND LEAVE
				//vSERIAL_bout('N');
				//vSERIAL_bout(ucaMSG_BUFF[1]);
				vRADIO_start_receiver();
				continue;
				}
		  #if 0
			vSERIAL_bout(' ');
			vSERIAL_bout('R');
			vSERIAL_HB8out(ucaMSG_BUFF[GMH_IDX_EOM_IDX]);
			vSERIAL_bout(',');
			vSERIAL_HB8out(ucaMSG_BUFF[1]);
			vSERIAL_bout(',');
			vSERIAL_HB8out(ucaMSG_BUFF[2]);
		  #endif

		  #if  1
			// WE HAVE A MSG
			// ECHO THE MESSAGE
			ucaMSG_BUFF[GMH_IDX_EOM_IDX] &= MSG_SIZE_MASK;					//scrub spurious bits
			for(ucTmp = 1;  ucTmp <= ucaMSG_BUFF[GMH_IDX_EOM_IDX];  ucTmp++)
				{
				vSERIAL_bout(ucaMSG_BUFF[ucTmp]);	//show the char
				if(ucaMSG_BUFF[ucTmp] == 13)
					vSERIAL_bout(10);
				}
		  #endif

			// MARK MSG AS GONE
			ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 0;

			// RESTART THE RECEIVER
//			PORTAbits.RA4 = 1;
			vRADIO_start_receiver();
//			PORTAbits.RA4 = 0;
			}

#if  1

		// CHECK FOR A KEYBOARD HIT
		if(ucSERIAL_kbhit())
			{
			// GET THE CHAR
			ucaMSG_BUFF[GMH_IDX_EOM_IDX] = 1;					//setup the buffer idx
			ucaMSG_BUFF[1] = ucSERIAL_bin();

			// ABORT THE RECEIVER SO WE CAN SEND
			if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_R_BUSY_BIT == 1)
				vRADIO_abort_receiver();

			// CHECK FOR A 2ND CHAR
			if(ucSERIAL_kbhit())
				{
				ucaMSG_BUFF[2] = ucSERIAL_bin();
				ucaMSG_BUFF[GMH_IDX_EOM_IDX] = 2;
				}

#if  1
			// CHECK FOR A 3RD CHAR (uart can have 3)*/
			if(ucSERIAL_kbhit())
				{
				ucaMSG_BUFF[3] = ucSERIAL_bin();
				ucaMSG_BUFF[GMH_IDX_EOM_IDX] = 3;
				}
#endif

			// SEND THE PACKET TO RADIO LINK
			vRADIO_xmit_msg();					//send the packet

			// ECHO TO OUR TERMINAL
			for(ucTmp = 1;  ucTmp <= ucaMSG_BUFF[GMH_IDX_EOM_IDX];  ucTmp++)
				vSERIAL_bout(ucaMSG_BUFF[ucTmp]);

			}// END: if()
#endif

		}// END: while(1)
	}
	#endif // RADIO1_DIAG_ENA






	#if 0
	/********  STARTUP RECEIVER ONE TIME ********
	*
	*
	*
	*
	*********************************************/
	{
	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Radio receiver startup one time test\r\n", 40);

	vRADIO_start_receiver();

	vSERIAL_sout("-R-", 3);
	vSERIAL_HB8out(ucFLAG1_BYTE.byte);
	vSERIAL_bout(',');
	vSERIAL_HB8out(ucFLAG2_BYTE.byte);
	vSERIAL_crlf();

	for(;;);						//hang

	}
	#endif




	#ifdef RADIO2_DIAG_ENA

	/********  STARTUP TRANSMITTER ********
	*
	*
	*
	*
	*********************************************/
	{
	// SHOW HEADER MESSAGE
	vSERIAL_sout("RADIO2: Radio xmit startup messages should be sent\r\n", 52);

	vRADIO_init();

	vMSG_buildMsgHdr_GENERIC( //HDR= Len,Type,Group,Src,Dest
			ST1_MSG_LAST_BYTE_NUM_UC,		//Len
			MSG_TYPE_STARTUP,				//Type
			0								//Dest (All)
			);

	// COMPUTE THE CRC
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

	while(TRUE)								//lint !e774
		{
		vSERIAL_sout("S", 1);

		// SHIP IT
//		PORTFbits.RF0 = 1;			//debug
		vRADIO_xmit_msg();
//		PORTFbits.RF0 = 0;			//debug

		// DELAY
		vDELAY_wait100usTic(5000);

		}// END: while(TRUE)

	}
	#endif 	// RADIO2_DIAG_ENA







	#ifdef RADIO3_DIAG_ENA
	{
	/***********************  RADIO3 diagnostic  *****************************
	*
	* This routine turns on the radio in XMIT mode and hangs there.
	* This is so the radio XMIT current can be measured.
	*
	* change! to radio may mean change here
	*
	*************************************************************************/

	#define PWR_M00 3			//highest
	#define PWR_M20 2			//-20 db
	#define PWR_M10 1			//-10 db
	#define PWR_OFF 0			//shut off

	uchar ucXmitPwr;


	vSERIAL_sout("RADIO3: Measure Radio XMIT current\r\n", 36);
	vSERIAL_sout("Push any Key to cycle to a new transmit power\r\n", 47);

	vRADIO_init();

	// FORCE THE RADIO TO HIGHEST TRANSMIT POWER
	vRADIO_write_setup_word(
						D_WORD,
						DWORD_HI_BYTE,
						DWORD_MID_BYTE_HI,			//lint !e778
						DWORD_LO_BYTE
						);
	ucXmitPwr = PWR_M00;
	vSERIAL_sout("POWER = HI\r\n", 12);


	// TURN RADIO ON
	PORTJbits.RADIO_MODE_BIT = 0;			//radio mode = XMIT
	PORTJbits.RADIO_STANDBY_BIT = 1;		//turn radio on
	PORTGbits.RADIO_ANTENNA_BIT = 1;		//XMIT antenna

	// FLOP THE TRANSMIT BIT
	while(TRUE)								//lint !e716 !e774
		{
		PORTGbits.RADIO_TRANSMIT_BIT = 1;
//		vDELAY_wait100usTic(1);

//		PORTGbits.RADIO_TRANSMIT_BIT = 0;
//		vDELAY_wait100usTic(1);

		if(ucSERIAL_kbhit())
			{
			switch(ucXmitPwr)
				{
				case PWR_M00:
					vRADIO_write_setup_word(
										D_WORD,
										DWORD_HI_BYTE,
										DWORD_MID_BYTE_MD,	//lint !e778
										DWORD_LO_BYTE
										);
					ucXmitPwr = PWR_M10;
					vSERIAL_sout("POWER = -10\r\n", 13);
					ucSERIAL_bin();		//lint !e534 //eat the char
					break;

				case PWR_M10:
					vRADIO_write_setup_word(
										D_WORD,
										DWORD_HI_BYTE,
										DWORD_MID_BYTE_LO,	//lint !e778
										DWORD_LO_BYTE
										);
					vSERIAL_sout("POWER = -20\r\n", 13);
					ucXmitPwr = PWR_M20;
					ucSERIAL_bin();		//lint !e534 //eat the char
					break;

				case PWR_M20:
					vRADIO_write_setup_word(
										D_WORD,
										DWORD_HI_BYTE,
										DWORD_MID_BYTE_OFF,	//lint !e778
										DWORD_LO_BYTE
										);
					vSERIAL_sout("POWER = OFF\r\n", 13);
					ucXmitPwr = PWR_OFF;
					ucSERIAL_bin();		//lint !e534 //eat the char
					break;

				case PWR_OFF:
				default:
					vRADIO_write_setup_word(
										D_WORD,
										DWORD_HI_BYTE,
										DWORD_MID_BYTE_HI,	//lint !e778
										DWORD_LO_BYTE
										);
					vSERIAL_sout("POWER = HI\r\n", 12);
					ucXmitPwr = PWR_M00;
					ucSERIAL_bin();		//lint !e534 //eat the char
					break;

				} // END: switch()

			} // END: if()

		}// END: while() 

	}
	#endif // RADIO3_DIAG_ENA






	#ifdef RADIO4_DIAG_ENA

	/********  RADIO BOARD DETECTION ************
	*
	*
	*
	*
	*********************************************/
	{
	// SHOW HEADER MESSAGE
	vSERIAL_sout("RADIO4: RADIO board detection\r\n", 31);

	while(TRUE)				//lint !e716 !e774 !e722 hang here
		{
		if(ucRADIO_is_board_there())
			{
			vSERIAL_sout("RADIO Board is IN \r\n", 20);
			}
		else
			{
			vSERIAL_sout("RADIO Board is OUT\r\n", 20);
			}

		vDELAY_wait100usTic(5000);
		}

	}
	#endif 	// RADIO4_DIAG_ENA







	#ifdef RADIO5_DIAG_ENA
	{
	/***********************  RADIO5 diagnostic  *****************************
	*
	* This routine sends a pulse so that the receiver strength can be tested
	*
	* change! to radio may mean change here
	*************************************************************************/

	#define PWR_M00 3			//highest
	#define PWR_M20 2			//-20 db
	#define PWR_M10 1			//-10 db
	#define PWR_OFF 0			//shut off

	uchar ucXmitPwr;


	vSERIAL_sout("RADIO5: Measure Receiver strength\r\n", 35);
	vSERIAL_sout("Push any Key to cycle to a new transmit power\r\n", 47);

	vRADIO_init();

	// FORCE THE RADIO TO HIGHEST TRANSMIT POWER
	vRADIO_write_setup_word(
						D_WORD,
						DWORD_HI_BYTE,
						DWORD_MID_BYTE_HI,			//lint !e778
						DWORD_LO_BYTE
						);
	ucXmitPwr = PWR_M00;
	vSERIAL_sout("POWER = HI\r\n", 12);


	// TURN RADIO ON
	PORTJbits.RADIO_MODE_BIT = 0;			//radio mode = XMIT
	PORTJbits.RADIO_STANDBY_BIT = 1;		//turn radio on
	PORTGbits.RADIO_ANTENNA_BIT = 1;		//XMIT antenna

	// FLOP THE TRANSMIT BIT
	while(TRUE)								//lint !e716 !e774
		{
		PORTGbits.RADIO_TRANSMIT_BIT = 1;
		vDELAY_wait100usTic(1);

		PORTGbits.RADIO_TRANSMIT_BIT = 0;
		vDELAY_wait100usTic(1);

		if(ucSERIAL_kbhit())
			{
			switch(ucXmitPwr)
				{
				case PWR_M00:
					vRADIO_write_setup_word(
										D_WORD,
										DWORD_HI_BYTE,
										DWORD_MID_BYTE_MD,	//lint !e778
										DWORD_LO_BYTE
										);
					ucXmitPwr = PWR_M10;
					vSERIAL_sout("POWER = -10\r\n", 13);
					ucSERIAL_bin();		//lint !e534 //eat the char
					break;

				case PWR_M10:
					vRADIO_write_setup_word(
										D_WORD,
										DWORD_HI_BYTE,
										DWORD_MID_BYTE_LO,	//lint !e778
										DWORD_LO_BYTE
										);
					vSERIAL_sout("POWER = -20\r\n", 13);
					ucXmitPwr = PWR_M20;
					ucSERIAL_bin();		//lint !e534 //eat the char
					break;

				case PWR_M20:
					vRADIO_write_setup_word(
										D_WORD,
										DWORD_HI_BYTE,
										DWORD_MID_BYTE_OFF,	//lint !e778
										DWORD_LO_BYTE
										);
					vSERIAL_sout("POWER = OFF\r\n", 13);
					ucXmitPwr = PWR_OFF;
					ucSERIAL_bin();		//lint !e534 //eat the char
					break;

				case PWR_OFF:
				default:
					vRADIO_write_setup_word(
										D_WORD,
										DWORD_HI_BYTE,
										DWORD_MID_BYTE_HI,	//lint !e778
										DWORD_LO_BYTE
										);
					vSERIAL_sout("POWER = HI\r\n", 12);
					ucXmitPwr = PWR_M00;
					ucSERIAL_bin();		//lint !e534 //eat the char
					break;

				} // END: switch()

			} // END: if()

		}// END: while() 

	}
	#endif // RADIO5_DIAG_ENA






	#ifdef RADIO6_DIAG_ENA
	{
	/*********  CHECK THE RSSI MEASUREMENT **************
	*
	*
	* change! to radio may mean change here
	*
	******************************************************/

	uchar ucc;
	long lSUM256;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("RADIO6: Measure RSSI\r\n", 22);
	vSERIAL_sout("This assumes that there is a transmitter pulsing RADIO5\r\n", 57);


	// TURN ON RADIO TO GET RSSI MEASUREMENT
	PORTJbits.RADIO_MODE_BIT = 1;			//radio mode = receive
	PORTJbits.RADIO_STANDBY_BIT = 1;		//turn radio on
	PORTGbits.RADIO_ANTENNA_BIT = 0;		//connect antenna
	vDELAY_wait100usTic(150);				//wait for radio to come up


	// SETUP THE A/D
	vAD_init();
	vAD_setup_analog_bits(RSSI_AD_CHAN);
	vAD_select_chan(RSSI_AD_CHAN, NO_WAIT);


	// LOOP READING THE RSSI
	lSUM256 = 0;						//init value
	while(TRUE)							//lint !e716 !e774
		{
		// COMPUTE A ROLLING AVERAGE
		for(ucc=0; ; ucc++)
			{
			// take out an ave reading
			lSUM256 -= ((lSUM256 & 0x00FFFF00) >> 8);
			// add on the new reading
			lSUM256 += (long)uiAD_read(RSSI_AD_CHAN, NO_SELECT_CHAN);

			// if time to see the value then break
			if(ucc == 0) break;
			}


		vSERIAL_UIV32out(((lSUM256 & 0x00FFFF00) >> 8));
		vSERIAL_crlf();

		}

	}
	#endif // RADIO6_DIAG_ENA





	#ifdef RADIO7_DIAG_ENA
	{
	/***********************  RADIO7 diagnostic  *****************************
	*
	* This test the transmit and receive states in the radio
	*
	* change! to radio may mean change here
	*************************************************************************/

	long ll;
	long lTmp1;
	long lTmp2;

	vSERIAL_sout("RADIO7: Test XMIT and REC radio states\r\n", 40);

	vRADIO_init();

	vSERIAL_sout("JUST after radio init\r\n", 23);
	vSERIAL_any_char_to_cont();

	// TURN RADIO ON
	PORTJbits.RADIO_STANDBY_BIT  = 1;	// Standby = awake
	PORTGbits.RADIO_TRANSMIT_BIT = 0;	// DATA bit = 0
	PORTGbits.RADIO_ANTENNA_BIT  = 1; 	// Antenna = XMIT
	PORTJbits.RADIO_MODE_BIT     = 0;	// Mode = xmit

	vSERIAL_sout("JUST after radio transmit startup\r\n", 35);
	vSERIAL_any_char_to_cont();

	// TURN RADIO OFF
	PORTGbits.RADIO_TRANSMIT_BIT = 0;	// DATA bit = 0
	PORTGbits.RADIO_ANTENNA_BIT  = 0; 	// Antenna = RECEIVE
	PORTJbits.RADIO_MODE_BIT     = 1;	// Mode = receive
	PORTJbits.RADIO_STANDBY_BIT  = 0;	// Standby = sleep

	vSERIAL_sout("JUST after radio transmit shutoff\r\n", 35);
	vSERIAL_any_char_to_cont();

	vSERIAL_sout("Radio test exit\r\n", 17);

Loop:
	goto Loop;


	}
	#endif // RADIO7_DIAG_ENA




	#ifdef RADIO8_DIAG_ENA

	/***********  TEST THE RANDOM NUMBER INITIALIZATION  *********
	*
	* Random number initializes using the radio
	*
	********************************************************/
	{
	usl uslRandVal;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("RADIO8: Random number radio init test\r\n", 39);


	vSERIAL_sout("Starting random init\r\n", 22);

	for(;;)
		{

		uslRandVal = uslRADIO_getRandomNoise();

		vSERIAL_sout("RandVal= ", 9);
		vSERIAL_UIV24out(uslRandVal);
		vSERIAL_crlf();


		}// END: for(;;)

	}
	#endif // RADIO8_DIAG_ENA






	#ifdef RADIO9_DIAG_ENA
	{
	/***********************  RADIO9 diagnostic  *****************************
	*
	* Test the radio Ping
	*
	* change! to radio may mean change here
	*************************************************************************/

	vSERIAL_sout("RADIO9: Test radio Ping\r\n", 25);

	vRADIO_init();

	while(TRUE)			//lint !e774
		{
		// TURN RADIO ON
		PORTJbits.RADIO_STANDBY_BIT  = 1;	// Standby = awake
		PORTGbits.RADIO_TRANSMIT_BIT = 1;	// DATA bit = 1
		PORTGbits.RADIO_ANTENNA_BIT  = 1; 	// Antenna = XMIT
		PORTJbits.RADIO_MODE_BIT     = 0;	// Mode = xmit

		vSERIAL_bout('1');
 		vDELAY_wait100usTic(10000);

		// TURN RADIO OFF
		PORTGbits.RADIO_TRANSMIT_BIT = 0;	// DATA bit = 0
		PORTGbits.RADIO_ANTENNA_BIT  = 0; 	// Antenna = RECEIVE
		PORTJbits.RADIO_MODE_BIT     = 1;	// Mode = receive
		PORTJbits.RADIO_STANDBY_BIT  = 0;	// Standby = sleep

		vSERIAL_bout('0');
 		vDELAY_wait100usTic(10000);

		}// END: while()

	}
	#endif // RADIO9_DIAG_ENA




	#ifdef RADIO10_DIAG_ENA
	{
	#include "radio.h"		//radio routines

	/***********************  RADIO10 diagnostic  *****************************
	*
	* Test the frequency calculator
	*
	*************************************************************************/
	uchar ucChanNum;
	usl uslSetupWord;


	vSERIAL_sout("RADIO10: Test Radio freq calculator\r\n", 37);

	vRADIO_init();

	ucChanNum = 0;
	while(TRUE)			//lint !e774
		{
		uslSetupWord = uslRADIO_computeFreqSetupWord(ucChanNum, RADIO_XMIT_FACTOR);
		vSERIAL_sout("      XMIT setup word chan(", 27);
		vSERIAL_UIV8out(ucChanNum);
		vSERIAL_sout(")= ", 3);
		vSERIAL_HB24out(uslSetupWord);
		vSERIAL_sout("        REC setup word chan(", 28);
		vSERIAL_UIV8out(ucChanNum);
		vSERIAL_sout(")= ", 3);
		uslSetupWord = uslRADIO_computeFreqSetupWord(ucChanNum, RADIO_REC_FACTOR);
		vSERIAL_HB24out(uslSetupWord);
		vSERIAL_crlf();

		if(ucChanNum == 0)
			{
			uslSetupWord = FULL_DDS0_FREQ_WORD;
			vSERIAL_sout("FIXED XMIT setup word chan(", 27);
			vSERIAL_UIV8out(ucChanNum);
			vSERIAL_sout(")= ", 3);
			vSERIAL_HB24out(uslSetupWord);
			vSERIAL_sout("  Fixed REC setup word chan(", 28);
			vSERIAL_UIV8out(ucChanNum);
			vSERIAL_sout(")= ", 3);
			uslSetupWord = FULL_DDS1_FREQ_WORD;
			vSERIAL_HB24out(uslSetupWord);
			vSERIAL_crlf();
			}

		if(ucChanNum == 1)
			{
			uslSetupWord = FULL_DDS0_FREQ_WORD_1;
			vSERIAL_sout("FIXED XMIT setup word chan(", 27);
			vSERIAL_UIV8out(ucChanNum);
			vSERIAL_sout(")= ", 3);
			vSERIAL_HB24out(uslSetupWord);
			vSERIAL_sout("  Fixed REC setup word chan(", 28);
			vSERIAL_UIV8out(ucChanNum);
			vSERIAL_sout(")= ", 3);
			uslSetupWord = FULL_DDS1_FREQ_WORD_1;
			vSERIAL_HB24out(uslSetupWord);
			vSERIAL_crlf();
			}

		ucChanNum++;
		ucChanNum &= 0x7F;

		vSERIAL_any_char_to_cont();

		}// END: while()

	}
	#endif // RADIO10_DIAG_ENA







	#ifdef RADIO11_DIAG_ENA
	{
	#include "radio.h"		//radio routines

	/***********************  RADIO11 diagnostic  *****************************
	*
	* Test the transmit frequency
	*
	* change! to radio may mean change here
	*************************************************************************/
	uchar ucChanNum;

	vSERIAL_sout("RADIO11: Test Radio transmit frequency\r\n", 40);

	vRADIO_init();

	ucChanNum = 0;
	while(TRUE)			//lint !e774
		{
		vRADIO_setBothFrequencys(ucChanNum,YES_FORCE_FREQ);

		// TRANSMIT A ONE
		PORTJbits.RADIO_STANDBY_BIT  = 1;	// StandbyBit = awake
		PORTGbits.RADIO_TRANSMIT_BIT = 1;	// DATABit = 1
		PORTGbits.RADIO_ANTENNA_BIT  = 1; 	// AntennaBit = XMIT
		PORTJbits.RADIO_MODE_BIT     = 0;	// ModeBIT = xmit

		vSERIAL_sout("Transmitting a 1 on channel ", 28);
		vSERIAL_UIV8out(ucChanNum);
		vSERIAL_crlf();
		vSERIAL_any_char_to_cont();

		// TRANSMIT A ZERO
		PORTGbits.RADIO_TRANSMIT_BIT = 0;	// DATA bit = 0
		PORTGbits.RADIO_ANTENNA_BIT  = 1; 	// AntennaBit = XMIT
		PORTJbits.RADIO_MODE_BIT     = 0;	// ModeBIT = xmit

		vSERIAL_sout("Transmitting a 0 on channel ", 28);
		vSERIAL_UIV8out(ucChanNum);
		vSERIAL_crlf();
		vSERIAL_any_char_to_cont();

		vRADIO_quit();

		ucChanNum++;
		ucChanNum &= 0x7F;

		}// END: while() 

	}
	#endif // RADIO11_DIAG_ENA





	#ifdef RADIO12_DIAG_ENA
	{
	#include "radio.h"		//radio routines

	/***********************  RADIO12 diagnostic  *****************************
	*
	* Send out a continuous radio signal 1
	*
	* change! to radio may mean change here
	*************************************************************************/
	uchar ucChanNum;

	vSERIAL_sout("RADIO12: Test Radio transmit frequency\r\n", 40);

	vRADIO_init();

	ucChanNum = TEST_CHANNEL;

	vRADIO_setBothFrequencys(ucChanNum,YES_FORCE_FREQ);

	// TRANSMIT A ONE
	PORTJbits.RADIO_STANDBY_BIT  = 1;	// StandbyBit = awake
	PORTGbits.RADIO_TRANSMIT_BIT = 1;	// DATABit = 1
	PORTGbits.RADIO_ANTENNA_BIT  = 1; 	// AntennaBit = XMIT
	PORTJbits.RADIO_MODE_BIT     = 0;	// ModeBIT = xmit

	while(TRUE);			//lint !e774 !e722

	}
	#endif // RADIO12_DIAG_ENA







	#ifdef RADIOX1_BOARD_DIAG_ENA

	#include "radio.h"		//radio routines
	#include "buz.h"		//buzzer routines
	#include "delay.h"		//time delay routines
	

	/********************************************
	*
	* RADIO gross component checkout
	*
	* NOTE: This routine has 2 parts:
	*		PART 1 loads into the radio that you are testing.
	*		PART 2 loads into a working radio that simply replies to the
	*		test radio.
	*
	*********************************************/
	{
	#define WAITCOUNT_MAX 100000
	long lWaitCount;

	#define NOTE_C		  768
	#define NOTE_E_FLAT   977

	#define QUARTER_NOTE 1000
	#define WHOLE_NOTE	 4000


	#define ERROR_BAD_CRC		1
	#define ERROR_BAD_MSG_TYPE	2


	uint uiTotMsgCount;
	uint uiTotErrCount;
	uint uiTotRecErrors;
	uint uiTotXmtErrors;
	uint uiPercentGood;
	const char *cStrPtr;
	uint uiStrLength;


	// INIT
	uiTotMsgCount = 0L;
	uiTotErrCount = 0L;
	uiTotRecErrors = 0L;
	uiTotXmtErrors = 0L;


	// SHOW HEADER MESSAGE
	vSERIAL_sout("RADIOX1: Gross Component Chk PART1\r\n", 36);

	// SETUP THE RADIO
	vRADIO_init();

	// SETUP THE RADIO FREQUENCY
	vRADIO_setBothFrequencys(TEST_CHANNEL,NO_FORCE_FREQ);

	//  MAKE SURE THE BOARD IS PLUGGED IN
	while(TRUE)							//lint !e716 !e774
		{
		if(!ucRADIO_chk_for_radio_board())
			{
			vSERIAL_sout("RADIO Not Plugged In\r\n", 22);
			vBUZ_raygun_up();
			continue;
			}
		vSERIAL_sout("RADIO plugged OK!\r\n", 19);
		break;
		}// END: while()


	while(TRUE)					//lint !e774
		{

		// NOW PERFORM THE TRANSMIT / RECEIVE CHECKOUT

		// FIRST TRANSMIT TEST-PACKET TO CHECKER

		// BUILD THE MSG
		ucaMSG_BUFF[ST1_IDX_EOM_IDX] = ST1_MSG_LAST_BYTE_NUM_UC;
		ucaMSG_BUFF[ST1_IDX_MSG_TYPE] = MSG_TYPE_TS1;

		ucaMSG_BUFF[ST1_IDX_GID_HI] = 0; //Group ID
		ucaMSG_BUFF[ST1_IDX_GID_LO] = 0; //Group ID

		ucaMSG_BUFF[ST1_IDX_SRC_SN_HI] = ucL2FRAM_getSnumMd8(); //SRC num MD
		ucaMSG_BUFF[ST1_IDX_SRC_SN_LO] = ucL2FRAM_getSnumLo8(); //SRC num LO

		ucaMSG_BUFF[ST1_IDX_DEST_SN_HI] = 0; //DEST addr HI
		ucaMSG_BUFF[ST1_IDX_DEST_SN_LO] = 0; //DEST addr LO

		// COMPUTE THE CRC
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

		// WAIT FOR OTHER MACHINE TO SWITCH TO RECEIVE
		vDELAY_wait100usTic(6000);	//60ms

		// SHIP IT
		vRADIO_xmit_msg();


		// SWITCH TO RECEIVE
		vRADIO_start_receiver();

		uiTotMsgCount++;

		// WAIT FOR MESSAGE COMPLETE
		for(lWaitCount = 0;  lWaitCount<WAITCOUNT_MAX;  lWaitCount++)
			{
			if(ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT) break;
			}

		// CHECK FOR REPLY TIMEOUT
		if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_R_BUSY_BIT == 1)
			{
			// WUPS WE STILL HAVE RECEIVER ON

			// ABORT THE RECEIVER OPERATION
			vRADIO_abort_receiver();

			cStrPtr = "R - NO Ans";
			uiStrLength = 10;
			vBUZ(NOTE_C, WHOLE_NOTE);		//NO answer
			uiTotRecErrors++;
			uiTotErrCount++;
			goto Loop_End;
			}

		// CHECK BAD MSG TYPE
		if(ucaMSG_BUFF[GMH_IDX_MSG_TYPE] != MSG_TYPE_TS2)
			{
			cStrPtr = "R - Bad Msg Typ";
			uiStrLength = 15;
			vBUZ_tune_bad_news_2();
			uiTotRecErrors++;
			uiTotErrCount++;
			goto Loop_End;
			}

		// CHECK FOR BAD CRC
		if(!ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_REC))
			{
			cStrPtr = "R - Bad CRC";
			uiStrLength = 11;
			vBUZ_tune_bad_news_1();
			uiTotRecErrors++;
			uiTotErrCount++;
			goto Loop_End;
			}

		// CHECK FOR XMIT BAD MSG TYPE
		if(ucaMSG_BUFF[ST1_IDX_GID_LO] == ERROR_BAD_MSG_TYPE)
			{
			cStrPtr = "X - Bd Msg Typ";
			uiStrLength = 14;
			vBUZ_raspberry();
			uiTotXmtErrors++;
			uiTotErrCount++;
			goto Loop_End;
			}

		// CHECK FOR XMIT BAD CRC
		if(ucaMSG_BUFF[ST1_IDX_GID_LO] == ERROR_BAD_CRC)
			{
			cStrPtr = "X - Bad CRC";
			uiStrLength = 11;
			vBUZ_tune_bad_news_3();
			uiTotXmtErrors++;
			uiTotErrCount++;
			goto Loop_End;
			}

		cStrPtr = "Good";
		uiStrLength = 4;
		vBUZ_test_successful();

Loop_End:

		{// COMPUTE THE PERCENT GOOD
		ulong ulTmp1;
		ulong ulTmp2;

		ulTmp1 = (uiTotMsgCount - uiTotErrCount) * 100UL;
		ulTmp2 = ulTmp1 / uiTotMsgCount;

		uiPercentGood = (uint)ulTmp2;
		}

			if((uiTotMsgCount % 10) == 0)
			{
				vSERIAL_sout("TotMsg  X-Err TotErr     OK%\r\n", 30);
			}

		vSERIAL_sout(" ", 1);
		vSERIAL_UI16out(uiTotMsgCount);
		vSERIAL_sout("  ", 2);
		vSERIAL_UI16out(uiTotXmtErrors);
		vSERIAL_sout("  ", 2);
		vSERIAL_UI16out(uiTotErrCount);
		vSERIAL_sout("  ", 2);
		vSERIAL_UI16out(uiPercentGood);
		vSERIAL_sout("%  ", 3);
		vSERIAL_sout(cStrPtr, uiStrLength);
		vSERIAL_crlf();


		}// END: while()

	}
	#endif 	// RADIOX1_DIAG_ENA






	#ifdef RADIOX2_BOARD_DIAG_ENA

	#include "radio.h"		//radio routines
	#include "buz.h"		//buzzer routines
	#include "delay.h"		//time delay routines

	/********************************************
	*
	* RADIO gross component checkout
	* NOTE: This routine has 2 parts.  PART 1 goes into the
	*		radio that you are testing.  PART 2 goes into
	*		a working radio that simply replies to the
	*		test radio.
	*
	*
	*********************************************/
	{

	#define ERROR_BAD_CRC		1
	#define ERROR_BAD_MSG_TYPE	2



	// SHOW HEADER MESSAGE
	vSERIAL_sout("RADIOX2: Gross Component Chk PART2\r\n", 36);

	// SETUP THE RADIO
	vRADIO_init();

	// SETUP THE RADIO FREQUENCY
	vRADIO_setBothFrequencys(TEST_CHANNEL,NO_FORCE_FREQ);

	//  MAKE SURE THE BOARD IS PLUGGED IN
	while(TRUE)							//lint !e716 !e774
		{
		if(!ucRADIO_chk_for_radio_board())
			{
			vSERIAL_sout("RADIO Not Plugged In\r\n", 21);
			vBUZ_raygun_up();
			continue;
			}
		vSERIAL_sout("RADIO Plugged OK!\r\n", 19);
		break;
		}// END: while()


	// STARTUP THE RECEIVER
	vRADIO_start_receiver();


	while(TRUE)					//lint !e774
		{

		// WAIT FOR MESSAGE COMPLETE
		while(!ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT);	//lint !e722

		// CHECK THE MSG TYPE
		if(ucaMSG_BUFF[GMH_IDX_MSG_TYPE] != MSG_TYPE_TS1)
			{
			vSERIAL_sout("Bad Msg Type\r\n", 14);
			ucaMSG_BUFF[ST1_IDX_GID_HI] = 0;
			ucaMSG_BUFF[ST1_IDX_GID_LO] = ERROR_BAD_MSG_TYPE;
			goto Do_Reply;
			}

		// CHECK THE CRC -- IF ITS BAD -- LOOP BACK
		if(!ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_REC))
			{
			vSERIAL_sout("Rec Bad CRC\r\n", 13);
			ucaMSG_BUFF[ST1_IDX_GID_HI] = 0;
			ucaMSG_BUFF[ST1_IDX_GID_LO] = ERROR_BAD_CRC;
			goto Do_Reply;
			}

		vSERIAL_sout("Rec OK\r\n", 8);


Do_Reply:
		// CHANGE THE MESSAGE TYPE
		ucaMSG_BUFF[ST1_IDX_MSG_TYPE] = MSG_TYPE_TS2;

		// COMPUTE THE CRC
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534 //compute the CRC

		// WAIT FOR OTHER MACHINE TO SWITCH TO RECEIVE
		vDELAY_wait100usTic(3000);	//30ms

		// SHIP IT
		vRADIO_xmit_msg();

		// SWITCH TO RECEIVE
		vRADIO_start_receiver();

		vSERIAL_sout("Rply Snt\r\n\r\n\r\n", 14);

		}// END: while()


	}
	#endif 	// RADIOX2_BOARD_DIAG_ENA







	#ifdef ROLE1_DIAG_ENA

	#include "MODOPT.h"

	/********  TEST ROLE ROUTINES ********
	*
	*
	***************************************/
	{
	uchar ucBitVal;

	// SHOW HEADER MESSAGE
	vSERIAL_sout("ROLE1: Test SHOW routine\r\n", 26);

	// SHOW THE ROM ROLE VERSIONS
	vMODOPT_showAllRomOptionTblsAllBits();
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vMODOPT_showAllRomOptionTblNames();
	vSERIAL_dash(20);
	vSERIAL_crlf();
	vMODOPT_showSingleRomOptionTblAllBits(DEFAULT_ROLE_IDX);
	vSERIAL_dash(20);
	vSERIAL_crlf();

//	goto STOP;

	// COPY THE DEFAULT AND SHOW IT
	vMODOPT_copyRomOptionsToFramOptions(DEFAULT_ROLE_IDX);
	vMODOPT_showAllFramOptionBits();
	vSERIAL_dash(20);
	vSERIAL_crlf();

//	goto STOP;

	// TEST THE FRAM BIT READ/WRITE
	ucBitVal = ucMODOPT_readSingleFramOptionBit(OPTPAIR_SHOW_SYSTAT);
	vSERIAL_sout("FRAM Show-systat bit = ", 23);
	vSERIAL_HB8out(ucBitVal);
	vSERIAL_crlf();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vSERIAL_sout("Setting FRAM Show-systat bit\r\n", 30);
	vMODOPT_writeSingleFramOptionBit(OPTPAIR_SHOW_SYSTAT, 1);
	ucBitVal = ucMODOPT_readSingleFramOptionBit(OPTPAIR_SHOW_SYSTAT);
	vSERIAL_sout("FRAM Show-systat bit = ", 23);
	vSERIAL_HB8out(ucBitVal);
	vSERIAL_crlf();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vMODOPT_showAllFramOptionBits();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vSERIAL_sout("Clring FRAM Show-systat bit\r\n", 29);
	vMODOPT_writeSingleFramOptionBit(OPTPAIR_SHOW_SYSTAT, 0);
	ucBitVal = ucMODOPT_readSingleFramOptionBit(OPTPAIR_SHOW_SYSTAT);
	vSERIAL_sout("FRAM Show-systat bit = ", 23);
	vSERIAL_HB8out(ucBitVal);
	vSERIAL_crlf();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vMODOPT_showAllFramOptionBits();
	vSERIAL_crlf();
	vSERIAL_dash(20);
	vSERIAL_crlf();

//	goto STOP;

	// COPY THE FRAM ROLE INTO THE RAM ROLE
	vMODOPT_copyAllFramOptionsToRamOptions();
	vMODOPT_showAllRamOptionBits();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	// TEST THE RAM BIT READ/WRITE
	ucBitVal = ucMODOPT_readSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT);
	vSERIAL_sout("RAM Show-systat bit = ", 22);
	vSERIAL_HB8out(ucBitVal);
	vSERIAL_crlf();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vSERIAL_sout("Setting RAM Show-systat bit\r\n", 29);
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT, 1);
	ucBitVal = ucMODOPT_readSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT);
	vSERIAL_sout("RAM Show-systat bit = ", 22);
	vSERIAL_HB8out(ucBitVal);
	vSERIAL_crlf();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vMODOPT_showAllRamOptionBits();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vSERIAL_sout("Clring RAM Show-systat bit\r\n", 28);
	vMODOPT_writeSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT, 0);
	ucBitVal = ucMODOPT_readSingleRamOptionBit(OPTPAIR_SHOW_SYSTAT);
	vSERIAL_sout("RAM Show-systat bit = ", 25);
	vSERIAL_HB8out(ucBitVal);
	vSERIAL_crlf();
	vSERIAL_dash(20);
	vSERIAL_crlf();

	vMODOPT_showAllRamOptionBits();
	vSERIAL_crlf();

	}

	#endif //END: ROLE1_DIAG_ENA






	#ifdef SHOW_DEFINES_DIAG_ENA

	/*********  DUMP THE MAIN.H DEFINES TO CHECK THEM  *****
	*
	*
	*
	******************************************************/
	{

	// SHOW HEADER MESSAGE
	vSERIAL_sout(": Show Define values diagnostic\r\n", 53);


	#if  1			// SHOW THE DC1 DEFINES
	  {
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_MSG_SIZE_IN_BYTES_UL 15= ", 29);
	  vSERIAL_HB32out( DC1_MSG_SIZE_IN_BYTES_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_MSG_IN_nS_UL    802C800= ", 29);
	  vSERIAL_HB32out( DC1_MSG_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_PKT_IN_TICS_UL     161E= ", 29);
	  vSERIAL_HB32out( DC1_PKT_IN_TICS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_PKT_IN_nS_UL    A4CB800= ", 29);
	  vSERIAL_HB32out( DC1_PKT_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("SUB_MINUS_PKT_IN_nS_UL 017922ED= ", 33);
	  vSERIAL_HB32out( SUB_MINUS_PKT_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_SUB_IN_nS_UL    0BC5DAED= ", 30);
	  vSERIAL_HB32out( DC1_SUB_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_MAX_SUBS_PER_SEC_UL   5= ", 29);
	  vSERIAL_HB32out( DC1_MAX_SUBS_PER_SEC_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_SUBS_PER_SEC_UL       5= ", 29);
	  vSERIAL_HB32out( DC1_SUBS_PER_SEC_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_TOT_SUB_COUNT_UL     80= ", 29);
	  vSERIAL_HB32out( DC1_TOT_SUB_COUNT_UL);
	  vSERIAL_crlf();
	  vSERIAL_sout("DC1_TOT_SUB_COUNT_UI     80= ", 29);
	  vSERIAL_HB16out( DC1_TOT_SUB_COUNT_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_SLOT_IN_SEC_UL       1A= ", 29);
	  vSERIAL_HB32out( DC1_SLOT_IN_SEC_UL);
	  vSERIAL_crlf();

   	vSERIAL_sout("DC1_SUB_IN_TICS_UI     1999= ", 29);
	  vSERIAL_HB16out( DC1_SUB_IN_TICS_UI);
	  vSERIAL_crlf();


	  vSERIAL_sout("DC1_REAL_ST_SUB0_UI    0000= ", 29);
	  vSERIAL_HB16out( DC1_REAL_ST_SUB0_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_REAL_ST_SUB1_UI    1999= ", 29);
	  vSERIAL_HB16out( DC1_REAL_ST_SUB1_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_REAL_ST_SUB2_UI    3332= ", 29);
	  vSERIAL_HB16out( DC1_REAL_ST_SUB2_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_REAL_ST_SUB3_UI    4CCB= ", 29);
	  vSERIAL_HB16out( DC1_REAL_ST_SUB3_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC1_REAL_ST_SUB4_UI    6664= ", 29);
	  vSERIAL_HB16out( DC1_REAL_ST_SUB4_UI);
	  vSERIAL_crlf();


	  vSERIAL_crlf();
	  vSERIAL_crlf();

	  }

	#endif


	#if  0
	  {
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_MSG_SIZE_IN_BYTES_UL  C= ", 29);
	  vSERIAL_HB32out( DC2_MSG_SIZE_IN_BYTES_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_MSG_IN_nS_UL    493E000= ", 29);
	  vSERIAL_HB32out( DC2_MSG_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_PKT_IN_TICS_UL      EBE= ", 29);
	  vSERIAL_HB32out( DC2_PKT_IN_TICS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_PKT_IN_nS_UL    6DDD000= ", 29);
	  vSERIAL_HB32out( DC2_PKT_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("SUB_MINUS_PKT_IN_nS_UL 12CD7AD= ", 32);
	  vSERIAL_HB32out( SUB_MINUS_PKT_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_SUB_IN_nS_UL    80AA7AD= ", 29);
	  vSERIAL_HB32out( DC2_SUB_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_MAX_SUBS_PER_SEC_UL   7= ", 29);
	  vSERIAL_HB32out( DC2_MAX_SUBS_PER_SEC_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_SUBS_PER_SEC_UL       7= ", 29);
	  vSERIAL_HB32out( DC2_SUBS_PER_SEC_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_TOT_SUB_COUNT_UL     80= ", 29);
	  vSERIAL_HB32out( DC2_TOT_SUB_COUNT_UL);
	  vSERIAL_crlf();
	  vSERIAL_sout("DC2_TOT_SUB_COUNT_UI     80= ", 29);
	  vSERIAL_HB16out( DC2_TOT_SUB_COUNT_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_SLOT_IN_SEC_UL       13= ", 29);
	  vSERIAL_HB32out( DC2_SLOT_IN_SEC_UL);
	  vSERIAL_crlf();

   	  vSERIAL_sout("DC2_SUB_IN_TICS_UI     1249= ", 31);
	  vSERIAL_HB16out( DC2_SUB_IN_TICS_UI);
	  vSERIAL_crlf();
	  vSERIAL_crlf();


	  vSERIAL_sout("DC2_REAL_ST_SUB0_UI    0000= ", 29);
	  vSERIAL_HB16out( DC2_REAL_ST_SUB0_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_REAL_ST_SUB1_UI    1249= ", 29);
	  vSERIAL_HB16out( DC2_REAL_ST_SUB1_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_REAL_ST_SUB2_UI    2492= ", 29);
	  vSERIAL_HB16out( DC2_REAL_ST_SUB2_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_REAL_ST_SUB3_UI    36DB= ", 29);
	  vSERIAL_HB16out( DC2_REAL_ST_SUB3_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_REAL_ST_SUB4_UI    4924= ", 29);
	  vSERIAL_HB16out( DC2_REAL_ST_SUB4_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_REAL_ST_SUB5_UI    5B6D= ", 29);
	  vSERIAL_HB16out( DC2_REAL_ST_SUB5_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC2_REAL_ST_SUB6_UI    6DB6= ", 29);
	  vSERIAL_HB16out( DC2_REAL_ST_SUB6_UI);
	  vSERIAL_crlf();


	  vSERIAL_crlf();
	  vSERIAL_crlf();

	  }

	#endif




	#if  0
	  {
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_MSG_SIZE_IN_BYTES_UL  F= ", 29);
	  vSERIAL_HB32out( DC3_MSG_SIZE_IN_BYTES_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_MSG_IN_nS_UL    5B8D800= ", 29);
	  vSERIAL_HB32out( DC3_MSG_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_PKT_IN_TICS_UL     1134= ", 29);
	  vSERIAL_HB32out( DC3_PKT_IN_TICS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_PKT_IN_nS_UL    802C800= ", 29);
	  vSERIAL_HB32out( DC3_PKT_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("SUB_MINUS_PKT_IN_nS_UL 12CD7AD= ", 33);
	  vSERIAL_HB32out( SUB_MINUS_PKT_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_SUB_IN_nS_UL    92F9FAD= ", 29);
	  vSERIAL_HB32out( DC3_SUB_IN_nS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_MAX_SUBS_PER_SEC_UL   6= ", 29);
	  vSERIAL_HB32out( DC3_MAX_SUBS_PER_SEC_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_SUBS_PER_SEC_UL       6= ", 29);
	  vSERIAL_HB32out( DC3_SUBS_PER_SEC_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_TOT_SUB_COUNT_UL     80= ", 29);
	  vSERIAL_HB32out( DC3_TOT_SUB_COUNT_UL);
	  vSERIAL_crlf();
	  vSERIAL_sout("DC3_TOT_SUB_COUNT_UI     80= ", 29);
	  vSERIAL_HB16out( DC3_TOT_SUB_COUNT_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_SLOT_IN_SEC_UL       16= ", 29);
	  vSERIAL_HB32out( DC3_SLOT_IN_SEC_UL);
	  vSERIAL_crlf();

   	vSERIAL_sout("DC3_SUB_IN_TICS_UI     1555= ", 29);
	  vSERIAL_HB16out( DC3_SUB_IN_TICS_UI);
	  vSERIAL_crlf();
	  vSERIAL_crlf();


	  vSERIAL_sout("DC3_REAL_ST_SUB0_UI    0000= ", 29);
	  vSERIAL_HB16out( DC3_REAL_ST_SUB0_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_REAL_ST_SUB1_UI    1555= ", 29);
	  vSERIAL_HB16out( DC3_REAL_ST_SUB1_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_REAL_ST_SUB2_UI    2AAA= ", 29);
	  vSERIAL_HB16out( DC3_REAL_ST_SUB2_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_REAL_ST_SUB3_UI    3FFF= ", 29);
	  vSERIAL_HB16out( DC3_REAL_ST_SUB3_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_REAL_ST_SUB4_UI    5554= ", 29);
	  vSERIAL_HB16out( DC3_REAL_ST_SUB4_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_REAL_ST_SUB5_UI    6AA9= ", 29);
	  vSERIAL_HB16out( DC3_REAL_ST_SUB5_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("DC3_REAL_ST_SUB6_UI    7FFE= ", 29);
	  vSERIAL_HB16out( DC3_REAL_ST_SUB6_UI);
	  vSERIAL_crlf();


	  vSERIAL_crlf();
	  vSERIAL_crlf();

	  }

	#endif





	#if  0
	  {
	  vSERIAL_crlf();

	  vSERIAL_sout("OM1_REAL_ST_DELAY_IN_TICS_UL      6553= ", 40);
	  vSERIAL_UI32out(  OM1_REAL_ST_DELAY_IN_TICS_UL);
	  vSERIAL_crlf();
	  vSERIAL_sout("OM1_ST_DELAY_IN_TICS_UI      9999= ", 35);
	  vSERIAL_HB16out(  OM1_ST_DELAY_IN_TICS_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("WARMUP_IN_TICS_UI            28F= ", 34);
	  vSERIAL_HB16out(  WARMUP_IN_TICS_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("OM1_MSG_SIZE_IN_BYTES_UL      10= ", 34);
	  vSERIAL_HB32out(  OM1_MSG_SIZE_IN_BYTES_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("OM1_PKT_IN_TICS_UL          1206= ", 34);
	  vSERIAL_HB32out( OM1_PKT_IN_TICS_UL);
	  vSERIAL_crlf();

	  vSERIAL_sout("OM1_DIGEST_IN_TICS_UI         41= ", 34);
	  vSERIAL_HB16out(  OM1_DIGEST_IN_TICS_UI);
	  vSERIAL_crlf();

	  vSERIAL_sout("OM1_REAL_SYNC_IN_TICS_UI    21A4= ", 34);
	  vSERIAL_HB16out(  OM1_REAL_SYNC_IN_TICS_UI);
	  vSERIAL_crlf();

	  vSERIAL_crlf();
	  vSERIAL_crlf();

	  }

	#endif



	}
	#endif





	#ifdef BRAINX_DIAG_ENA
	{
	#include "delay.h"		//time delay routines
	#include "buz.h"		//buzzer routines
	#include "fram.h"		//Level 1 Ferro RAM

	/*************  BRAINX diagnostic  ****************************************
	*
	* GROSS COMPONENT TEST FOR THE BRAIN BOARD
	*
	* This code is old (pre-buzzer) and should not be used
	*
	**************************************************************************/

	USL uslAddr;
	uchar ucWriteData;
	uchar ucReadData;
	uchar ucShouldBe;
//	uchar uc;
	uchar ucSRAM_OK;

//	#define MAX_MEM_SIZE 3
//	#define MAX_MEM_SIZE 10
//	#define MAX_MEM_SIZE 256
	#define MAX_MEM_SIZE 4096
//	#define MAX_MEM_SIZE 65535
//	#define MAX_MEM_SIZE 262144


//	#define FORCE_AN_SRAM_ERROR	YES		//comment this out for real test
//	#define FORCE_A_FLASH_ERROR	YES		//comment this out for real test
//	#define FORCE_AN_FRAM_ERROR	YES		//comment this out for real test

	#define STARTER_CODE 0x80;

	#define NOTE_C       768
	#define NOTE_E_FLAT  977
	#define QUARTER_NOTE 1000



	PORTF = 0;						//clr all the LED's
	TRISF = 0;

	while(TRUE) //lint !e774

		{/*******************  FULL TEST LOOP ********************************/

		// SHOW HEADER MESSAGE
		vSERIAL_sout("\r\n\r\n\r\n", 6);
		vSERIAL_dash(7);
		vSERIAL_sout("  BRAINX: Gross component test for Brain  ", 42);
		vSERIAL_dash(7);
		vSERIAL_crlf();


		while(TRUE) //lint !e774

			{/**************  TEST THE SRAM  **********************/
			ucSRAM_OK = TRUE;

			// CLR ALL LED'S
			PORTF = 0;

			vSERIAL_sout("Running SRAM test\r\n", 19);

			// FILL THE SRAM MEMORY
			ucWriteData = STARTER_CODE;
			for(uslAddr =0xFFFFFE;		//floating 0 addr line chk
				uslAddr!=0xFBFFFF;
				uslAddr = ((uslAddr << 1) | 0x000001)
			   )
				{
				#if 0
				vSERIAL_sout("Writing addr= ", 14);
				vSERIAL_HB24out(uslAddr);
				vSERIAL_sout(" Value= ", 8);
				vSERIAL_UI8out(ucWriteData);
				vSERIAL_crlf();
				#endif

				#ifdef FORCE_AN_SRAM_ERROR
				// SMASH A LOCATION ON PURPOSE TO PROVE TEST WORKS
				if(uslAddr == 0xFFFFEF)
					{
					vSRAM_write_B8(uslAddr, 0x25);
					ucWriteData++;
					continue;
					}
				#endif

				vSRAM_write_B8(uslAddr, ucWriteData);

				ucWriteData++;

				}// for()


			// CHECK THE SRAM MEMORY
			ucShouldBe = STARTER_CODE;
			for(uslAddr=0xFFFFFE;
				uslAddr!=0xFBFFFF;
				uslAddr = ((uslAddr << 1) | 0x000001)
			   )
				{
				// CHECK THE BYTE IN MEM
				ucReadData = ucSRAM_read_B8(uslAddr);

				#if 0
				vSERIAL_sout("Reading addr= ", 14);
				vSERIAL_HB24out(uslAddr);
				vSERIAL_sout(" Value= ", 8);
				vSERIAL_UI8out(ucReadData);
				vSERIAL_crlf();
				#endif

				if(ucReadData != ucShouldBe)
					{
					// SHOW AN SRAM ERROR IN LOW 2 LED'S
					PORTFbits.RF1 = 1;
					PORTFbits.RF3 = 1;

					vSERIAL_sout("Mem Fail at ", 12);
					vSERIAL_HB24out(uslAddr);
					vSERIAL_sout(" wrote ", 7);
					vSERIAL_HB8out(ucShouldBe);
					vSERIAL_sout(" was ", 5);
					vSERIAL_HB8out(ucReadData);
					vSERIAL_crlf();

					ucSRAM_OK = FALSE;

					vDELAY_wait100usTic(800);

					PORTFbits.RF1 = 0;
					PORTFbits.RF3 = 0;

					}

				ucShouldBe++;

				}// for()

			if(ucSRAM_OK) break;

			}/**************  END: TEST THE SRAM  **********************/




		{ // lint !e539
		/************  CHECK THE FLASH CHIP  ********************/

		uint uii;
		uchar ucData;
		int iPage;
		uchar ucFlashOK;

		vSERIAL_sout("Running FLASH test\r\n", 20);

		iPage = 0x30;
		ucFlashOK = TRUE;
		while(TRUE)							//lint !e716 !e774
			{

			// CLEAR ALL THE LED'S
			PORTF = 0;

			// INIT THE FLASH
			ucFLASH_init();			//lint !e534			//init FLASH to mode 0

			// WRITE USER TO BUFFER
			for(uii=0; uii<528;  uii++)
				{
				// WRITE THE DATA
				vFLASH_write_byte_to_buff(0,uii,((uchar) (uii&0xFF)));

				#ifdef FORCE_A_FLASH_ERROR
				// SMASH A LOCATION ON PURPOSE TO PROVE TEST WORKS
				if(uii == 0x6)
					{
					vFLASH_write_byte_to_buff(0,uii,0x55);
					}
				#endif

				}

			// CHECK THE FLASH WRITTEN DATA
			for(uii=0; uii<528;  uii++)
				{
				// CHECK THE BYTE IN MEM
				ucData = ucFLASH_read_byte_from_buff(0, uii);
				if(ucData != (uchar) (uii&0xFF))
					{
					// SHOW AN SRAM ERROR IN HI 2 LED'S
					PORTFbits.RF5 = 1;
					PORTFbits.RF7 = 1;

					vSERIAL_sout("FLASH Fail (BUFF WRITE) at Page ", 32);
					vSERIAL_HB16out((uint)iPage);
					vSERIAL_sout(" loc ", 5);
					vSERIAL_HB16out((uint)uii);
					vSERIAL_sout(" wrote ", 7);
					vSERIAL_HB8out((uchar)(uii&0xFF));
					vSERIAL_sout(" was ", 5);
					vSERIAL_HB8out(ucData);
					vSERIAL_crlf();

					vBUZ(NOTE_C,3000);			//bad

					ucFlashOK = FALSE;

					vDELAY_wait100usTic(5000);

					}// END: if()

				}// for(uii)

			// TURN OFF THE FLASH
			vFLASH_quit();

			if(ucFlashOK)	break;

			}// END: while()

		}  /****************  END: CHECK THE FLASH CHIP  *******************/





		{ /**************  CHK THE CLK CHIP TO SEE IF IT TICS  *************/
		uchar ucTIMER_OK;

		// SHOW HEADER MESSAGE
		vSERIAL_sout("Running CLOCK test\r\n", 20);


		while(TRUE)					//lint !e774
			{
			uchar ucTime1HI;
			uchar ucTime1LO;
			uchar ucTime2HI;
			uchar ucTime2LO;

			ucTIMER_OK = TRUE;

			// READ THE CLK
			T1CONbits.RD16 = 1;
			ucTime1LO = TMR1L;
			ucTime1HI = TMR1H;

			// WAIT FOR A GUARANTEED TIC
			vDELAY_wait4us();
			vDELAY_wait4us();
			vDELAY_wait4us();
			vDELAY_wait4us();
			vDELAY_wait4us();
			vDELAY_wait4us();
			vDELAY_wait4us();
			vDELAY_wait4us();
			vDELAY_wait4us();
			vDELAY_wait4us();


			// READ THE CLK AGAIN
			T1CONbits.RD16 = 1;
			ucTime2LO = TMR1L;
			ucTime2HI = TMR1H;

			if(ucTime2LO == ucTime1LO)
				{
				ucTIMER_OK = FALSE;

				// SHOW AN SRAM ERROR IN HI 2 LED'S
				PORTFbits.RF5 = 1;
				PORTFbits.RF7 = 1;

				vSERIAL_sout("Timer is BAD\r\n", 14);

				vDELAY_wait100usTic(800);

				PORTFbits.RF5 = 0;
				PORTFbits.RF7 = 0;

				vDELAY_wait100usTic(800);
				}


			// BREAK OUT IF THE TIMER WORKS
			if(ucTIMER_OK)	break;

			}// END: while()

		}/************  END: CLK TEST  ********************/





		{/*************  CHECK THE FRAM  ******************/
		uchar ucFramData;
		#define FRAM_TEST_CODE 0xCC

		// SHOW HEADER MESSAGE
		vSERIAL_sout("Running FRAM test\r\n", 19);


		// SETUP THE FRAM MEM
		vFRAM_init();

		// LOOP WRITING AND THEN READING THE FRAM MEMORY
		while(TRUE)		//lint !e774
			{
			// CLEAR ALL THE LED'S
			PORTF = 0;

			// WRITE THE DATA NOT
			vFRAM_write_B8(FRAM_TEST_ADDR_XI, ~FRAM_TEST_CODE);

			// WRITE THE DATA
			vFRAM_write_B8(FRAM_TEST_ADDR_XI, FRAM_TEST_CODE);

			// READ THE DATA
			ucFramData = ucFRAM_read_B8(FRAM_TEST_ADDR_XI);

			#ifdef FORCE_AN_FRAM_ERROR
			ucFramData = ~FRAM_TEST_CODE;
			#endif


			// CHECK BYTE IN MEMORY
			if(ucFramData == FRAM_TEST_CODE) break;

			// SHOW AN SRAM ERROR IN HI 2 LED'S
			PORTFbits.RF5 = 1;
			PORTFbits.RF7 = 1;

			vBUZ(NOTE_C,3000);			//bad

			vSERIAL_sout("FRAM Mem Fail at ", 17);
			vSERIAL_HB16out(FRAM_TEST_ADDR_XI);
			vSERIAL_sout(" wrote= ", 8);
			vSERIAL_HB8out(FRAM_TEST_CODE);
			vSERIAL_sout(" was= ", 6);
			vSERIAL_HB8out(ucFramData);
			vSERIAL_sout(" xor= ", 6);
			vSERIAL_HB8out(FRAM_TEST_CODE ^ ucFramData);
			vSERIAL_crlf();

			vDELAY_wait100usTic(5000);

			}// END: while()

		}/*************  END: CHECK THE FRAM  ******************/





		// IF IT PASSED THE GIVE A SUCCESS MESSAGE

		// SHOW A SUCCESS IN ALL 4 LED'S
		PORTFbits.RF1 = 1;
		PORTFbits.RF3 = 1;
		PORTFbits.RF5 = 1;
		PORTFbits.RF7 = 1;

		vSERIAL_sout("\r\nBRAIN board passed gross component test\r\n", 43);

		vBUZ(NOTE_C,		QUARTER_NOTE);
		vDELAY_wait100usTic(QUARTER_NOTE);
		vBUZ(NOTE_C,		QUARTER_NOTE);
		vDELAY_wait100usTic(QUARTER_NOTE);
		vBUZ(NOTE_C,		QUARTER_NOTE);
		vDELAY_wait100usTic(QUARTER_NOTE);
		vBUZ(NOTE_E_FLAT,	QUARTER_NOTE);

		vDELAY_wait100usTic(5000);

		}// END: while(TRUE)

	}/*************  FULL TEST LOOP ********************/
	#endif // END: BRAINX_DIAG_ENA


	#ifdef FLASH9_DIAG_ENA
	// See how long it takes to store a message to flash
	{
		#include "l2flash.h"
		#include "opmode.h"
		#include "msg.h"
		#include "l2fram.h"
		#include "time.h"
		#include "sensor.h"
		#include "misc.h"
		#include "delay.h"
		int i;

		// Blink the led so we know that it's time to go
		vMISC_blinkLED(63);
		vMISC_setLED(0);

		// Give the user time to hit single on the scope
		vDELAY_wait100usTic(50000);

		// Turn on the LED (our measurement tool)
		vMISC_setLED(1);

		// Store the message 10 times, so the signal will be there for a while
		for(i = 0; i < 10; ++i)
		{
			// Make a message and store it
			vOPMODE_buildMsg_OM2(
					0,
					0,
					ucMSG_incMsgSeqNum(),
					0,
					uiL2FRAM_getSnumLo16AsUint(),
					lTIME_getSysTimeAsLong(),
					2,
					SENSOR_ROM2_FAILURE_DEST_SN,
					32145,
					SENSOR_ROM2_FAILURE_TYPE,
					SENSOR_DATA_OM2_MISSED,
					0,
					0,
					0,
					0);

			vL2FLASH_storeMsgToFlash();
		}

		// Shut off the LED, end of the interval
		vMISC_setLED(0);
	}
	#endif





	// SEE HOW LONG IT TAKES TO STORE A MESSAGE TO SRAM
	#ifdef SRAM5_DIAG_ENA
	{
		#include "l2sram.h"
		#include "opmode.h"
		#include "msg.h"
		#include "l2fram.h"
		#include "time.h"
		#include "sensor.h"
		#include "misc.h"
		#include "delay.h"
		int i;

		// Blink the led so we know that it's time to go
		vMISC_blinkLED(63);
		vMISC_setLED(0);

		// Give the user time to hit single on the scope
		vDELAY_wait100usTic(50000);

		// Turn on the LED (our measurement tool)
		vMISC_setLED(1);

		// Store the message 10 times, so the signal will be there for a while
		for(i = 0; i < 10; ++i)
			{
			// Make a message and store it
			vOPMODE_buildMsg_OM2(
					0,
					0,
					ucMSG_incMsgSeqNum(),
					0,
					uiL2FRAM_getSnumLo16AsUint(),
					lTIME_getSysTimeAsLong(),
					2,
					SENSOR_ROM2_FAILURE_DEST_SN,
					32145,
					SENSOR_ROM2_FAILURE_TYPE,
					SENSOR_DATA_OM2_MISSED,
					0,
					0,
					0,
					0);

			vL2SRAM_storeMsgToSram();
			}

		// SHUT OFF THE LED, END OF THE INTERVAL
		vMISC_setLED(0);
	}
	#endif


	// -------------  END OF IFDEF DIAGNOSTICS  ------------------------------

	#ifdef THIS_IS_A_DIAGNOSTIC
	#if (THIS_IS_A_DIAGNOSTIC == YES)

	vSERIAL_sout("HALT\r\n", 6);

STOP:
	goto STOP;

	#endif //THIS_IS_A_DIAGNOSTIC
	#endif //THIS_IS_A_DIAGNOSTIC

	return;

	}// END: vDIAG_run_diagnostics() 




// -----------------------  END OF MODULE  --------------------------------- 
