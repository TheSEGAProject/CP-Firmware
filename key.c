
/**************************  KEY.C  ******************************************
*
* Routines to accept keyboard inputs.
*
*
* V1.00 10/14/2003 wzr
*		Started
*
******************************************************************************/



/*************************  CMD TREE  ****************************************
*
* Button ÄÄÂÄ0 Quit
*  Push    ³
*          ÃÄ 1 Help
*          ³
*          ÃÄ 2 Show Tbls
*          ³
*          ÃÄ 3 Date Set
*          ³
*          ÃÄ 4 Time Set
*          ³
*          ÃÄ 5 Upload to PC
*          ³
*          ÃÄ 6 ModifyID
*          ³                 
*          ³                 
*          ÃÄ 7 ModifyOptions ÄÄÂÄ0 Quit
*          ³   (MODOPT.C)       ³
*          ³                    ÃÄ1 Help
*          ³                    ³
*          ³                    ÃÄ2 Show Option Bits
*          ³                    ³
*          ³                    ÃÄ3 Set Option Bit
*          ³                    ³
*          ³                    ÃÄ4 Clear Option Bit
*          ³                    ³
*          ³                    ÀÄ5 Set Role
*          ³    
*          ³    
*          ÃÄ 8 ModifyStorage ÄÄÂÄ0 Quit
*          ³   (MODSTORE.C)     ³
*          ³                    ÃÄ1 Help
*          ³                    ³
*          ³                    ÃÄ2 Show Flash Tables
*          ³                    ³
*          ³                    ÃÄ3 Delete Old Flash
*          ³                    ³
*          ³                    ÃÄ4 WindBack Flash Ptr
*          ³                    ³
*          ³                    ÀÄ5 Zero Flash
*          ³  
*          ³  
*          ÃÄ 9 ModifyAction  ÄÄÂÄ0 Quit
*          ³   (MODACT.C)       ³
*          ³                    ÃÄ1 Help
*          ³                    ³
*          ³                    ÃÄ2 Show Actn Tables
*          ³                    ³
*          ³                    ÃÄ3 Add Actn
*          ³                    ³
*          ³                    ÃÄ4 Delete Actn
*          ³                    ³
*          ³                    ÃÄ5 Modify Actn Interval
*          ³                    ³
*          ³                    ÃÄ6 Modify Actn Act
*          ³                    ³
*          ³                    ÀÄ7 Modify Actn YTrigger
*          ³  
*          ÃÄ10 Restart
*          ³  
*          ÃÄ11 Diagnostics
*          ³  
*          ÀÄ12 Load SD 
*              
*
**************************  CMD TREE  ****************************************/

/*lint -e526 */     /* function not defined */
/*lint -e657 */    	/* Unusual (nonportable) anonymous struct or union */
///*lint -e714 */ 	/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
///*lint -e758 */ 	/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "diag.h"			//diagnostic defines
#include "std.h"			//standard definitions
#include "key.h"			//Top level Key commands
#include "main.h"			//
#include "misc.h"			//homeless functions
#include "crc.h"			//CRC calculation module
#include "serial.h"			//serial IO port stuff
#include "time.h"			//system time routines
#include "comm.h"			//msg routines
#include "fulldiag.h"		//runtime diag
#include "MODACT.h"			//Keyboard action table commands
#include "daytime.h"		//Daytime routines
#include "MODOPT.h"		//Rom Role Flag Handler Routines
#include "MODSTORE.h"		//Modify Storage tables	
#include "numcmd.h"			//numbered command routines
#include "hal/config.h"		//system configuration description file
#include "drivers/buz.h"			//buzzer control code
#include "mem_mod.h"			//Level 2 memory routines
#include "flash_mcu.h"			// Flash memory of the MCU
#include "SP.h"							// SP board defines/routines
#include "report.h"
#include "OTA.h"
#include "scc.h"

#ifdef FAKE_UPLOAD_MSGS
#if (FAKE_UPLOAD_MSGS == YES)
  #include "opmode.h"			//Operational mode routines
#endif
#endif

#ifdef FAKE_UPLOAD_MSGS
#if (FAKE_UPLOAD_MSGS == YES)
  #include "action.h"			//Action routines
#endif
#endif

#ifdef INC_ESPORT						//defined on Cmd line
#ifdef USE_ESPORT_OUTPUT
#if (USE_ESPORT_OUTPUT == YES)
  #include "esport.h"
#endif
#endif
#endif




#define UPLOAD_FROM_NOBODY	0
#define UPLOAD_FROM_SRAM	1
#define UPLOAD_FROM_FLASH	2
#define UPLOAD_FROM_FAKE	3





/*********************  EXTERNS  *********************************************/

extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

extern uint uiGLOB_bad_flash_CRC_count;	//count of bad CRC's on flash msgs

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

/**********************  TABLES  *********************************************/

#define CMD_ARRAY_SIZE 18

/* FUNCTION DELCARATIONS (in order of numbering )*/
//     void vKEY_exitFuncPlaceHolder(void);		// 0 Quit
static void vKEY_showHelp(void);				// 1 Help 
static void vKEY_showTables(void);				// 2 Show Tables
static void vKEY_setDate(void);					// 3 Set Date
static void vKEY_setTime(void);					// 4 Set Time
static void vKEY_uploadDataToPC(void);			// 5 Upload data to PC		
static void vKEY_IDchange(void);				// 6 Modify the Wizard ID	 
//     void vMODOPT_cmdModifyOptionTbls(void);	// 7 Modify the Wizard Role 
//     void vMODSTORE_modifyStorageTbls(void);	// 8 Modify Storage Tables
//     void vMODACT_modifyActionTbls(void);		// 9 Modify Actions Entrys
static void vKEY_restartSys(void);				//10 Restart the system
static void vKEY_diagnostics(void);             //11 Run Test Routines
static void vKEY_ResetSDLogging(void);			//14 Format the SD card
static void vKEY_DisplaySDBlockNum(void);		//15 Display SD Blk num
static void vKey_CrisisLog(void);				//16 Reads messages stored in the crisis log

#define SETID_ARRAY_SIZE 4
void vKEY_showIDHelp(void);
static void vKEY_Set_HID(void);							// 0 Set the hardware ID
static void vKEY_SetWiSARDNet_ID(void);			// 1 Modify the Wizard network address

#define CRISIS_OPTIONS_ARRAY_SIZE	4
void vKEY_showCrisisHelp(void);
void vKEY_ReadCrisis(void);
void vKEY_EraseCrisis(void);

const GENERIC_CMD_FUNC vCMD_func_ptrArray[CMD_ARRAY_SIZE] =
 {
 vKEY_exitFuncPlaceHolder,			// 0 Quit
 vKEY_showHelp,						// 1 Help 
 vKEY_showTables,					// 2 Show Tables
 vKEY_setDate,						// 3 Set Date
 vKEY_setTime,						// 4 Set Time
 vKEY_uploadDataToPC,				// 5 Upload data to PC			  
 vKEY_IDchange,						// 6 Modify the Wizard ID					  
 vMODOPT_cmdModifyOptionTbls,		// 7 Modify the Wizard Options
 vMODSTORE_modifyStorageTbls,		// 8 Modify Storage Tables
 vMODACT_modifyActionTbl,			// 9 Modify Actions Entrys
 vKEY_restartSys,					//10 Restart the system
 vKEY_diagnostics,					//11 Run Test Routines
 vKEY_restoreFRAM,					//12 Factory Reset
 vRoute_DisplayEdges,				//13 Show connected nodes
 vKEY_ResetSDLogging,				//14 Format and reset the SD card
 vKEY_DisplaySDBlockNum,				//15 Display SD Blk num
 vKey_CrisisLog,							//16 Options for the messages stored in the crisis log
 vOTA													//17 Over the air reprogramming
 };


T_Text S_CmdText[CMD_ARRAY_SIZE] =
{
 {"Quit",	4},								// 0 Quit
 {"Help",	4},								// 1 Help
 {"ShowTbls",	8},						// 2 Show Tables
 {"DateSet", 7},						// 3 Set Date
 {"TimeSet", 7},						// 4 Set Time
 {"UploadDataToPC",	14},		// 5 Upload data to PC
 {"ModifyID", 8},						// 6 Modify the Wizard ID
 {"ModifyOptionTbls",	16},	// 7 Modify the Wizard Role
 {"ModifyStorageTbls", 17},	// 8 Modify Storage Tables
 {"ModifyActionTbls",	16},	// 9 Modify Actions Entrys
 {"RestartSys", 10},				//10 Restart the system
 {"Diagnostics", 11},				//11 Run Test Routines
 {"SystemRestore", 13},			//12 Restore FRAM to default values
 {"DisplayNetEdges", 15},		//13 Show connected nodes
 {"ResetSDLogging", 14},		//14 Format and reset the SD card
 {"DisplaySDBlkNum", 15},		//15 Display SD Blk num
 {"CrisisLog", 9},
 {"OTA", 3},
}; /* END: cpaCmdText[] */

const GENERIC_CMD_FUNC vKey_SetID_func_ptrArray[SETID_ARRAY_SIZE] = {
		vKEY_exitFuncPlaceHolder, // 0 Quit
		vKEY_showIDHelp,						// 1 Help
    vKEY_SetWiSARDNet_ID, 		// 1 Modify the Wizard network address
    vKEY_Set_HID, 						// 0 Set the hardware ID
    };

T_Text S_SetIDtext[SETID_ARRAY_SIZE] =
 {
		 {"Quit", 4},						// 0 Quit
		 {"Help",	4},						// 1 Help
		 {"WisardNet ID", 12},	// 2 Set the hardware ID
		 {"Hardware ID", 11},		// 3 Modify the Wizard network address
 };

const GENERIC_CMD_FUNC vKey_Crisis_func_ptrArray[CRISIS_OPTIONS_ARRAY_SIZE] = {
		vKEY_exitFuncPlaceHolder, 	// 0 Quit
		vKEY_showCrisisHelp,						// 1 Help
		vKEY_ReadCrisis, 						// 2 Print crisis log to terminal
		vKEY_EraseCrisis, 					// 3 Erase crisis log
    };

T_Text S_CrisisLogtext[CRISIS_OPTIONS_ARRAY_SIZE] =
 {
		 {"Quit", 4},								// 0 Quit
		 {"Help", 4},								// 1 Help
		 {"Print Crisis Log", 16},	// 2 Set the hardware ID
		 {"Erase Crisis Log",	16},	// 3 Modify the Wizard network address
 };

/********************  DECLARATIONS  *****************************************/

void vKEY_doUploadToPC(
		uchar ucUploadType	//UPLOAD_FROM_SRAM,  UPLOAD_FROM_FLASH
		);

void vKEY_showCurTime(
		void
		);




/***************************  CODE  ******************************************/



/************************  ucKEY_doKeyboardCmdInput() *************************
*
* Handle the keyboard commands
*
* RET: == 0  restart
*		> 0  continue in operational mode
*
******************************************************************************/

uchar ucKEY_doKeyboardCmdInput(
		void
		)

	{
	long lLimitTime;

	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_KEY_PRESSED = 0;

	/* OUTPUT THE COMMAND PROMPT */
	vSERIAL_sout("\r\nPress any key to continue...", 30);

	/* WAIT FOR A KEY HIT */
	lLimitTime = lTIME_getSysTimeAsLong() + 10L;
	while(TRUE)
		{
		if(ucSERIAL_kbhit()) break;
		if(lTIME_getSysTimeAsLong() > lLimitTime)
			{
			vSERIAL_sout("ToutRetToSys\r\n", 14);
			return(1);
			}
		}

	vSERIAL_sout("\r\n", 2);
	vKEY_showHelp();
	vSERIAL_sout(">", 1);

	// Disable serial rx interrupts
	UART1_REG_IE &= ~UART1_RX_IE;

	vNUMCMD_doCmdInputAndDispatch(
				'>',						//prompt
				S_CmdText,				//command array
				&vCMD_func_ptrArray[0],		//dispatch function ptrs
				CMD_ARRAY_SIZE,				//array size
				NO_SHOW_INITIAL_PROMPT		//initial prompt flag
				);


	// Enable serial rx interrupts
	UART1_REG_IE |= UART1_RX_IE;

	return(1);

	}/* END: uchar ucKEY_doKeyboardCmdInput() */





/*******************  vKEY_exitFuncPlaceHolder()  ****************************
*
*
*
******************************************************************************/
void vKEY_exitFuncPlaceHolder(
		void
		)
	{
	return;

	}/* END: vKEY_exitFuncPlaceHolder() */






/*******************  vKEY_showHelp()  ****************************************
*
*
*
******************************************************************************/
void vKEY_showHelp(
		void
		)
	{
	vSERIAL_crlf();
	vNUMCMD_showCmdList(S_CmdText, CMD_ARRAY_SIZE);
	vSERIAL_crlf();

	return;

	}/* END: vKEY_showHelp() */



/*******************  vKEY_showIDHelp()  ****************************************
*
* Displays menu for setting IDs
*
******************************************************************************/
void vKEY_showIDHelp(
		void
		)
	{
	vSERIAL_crlf();
	vNUMCMD_showCmdList(S_SetIDtext, SETID_ARRAY_SIZE);
	vSERIAL_crlf();

	return;

	}/* END: vKEY_showHelp() */


/*******************  vKEY_showIDHelp()  ****************************************
*
* Displays menu for setting IDs
*
******************************************************************************/
void vKEY_showCrisisHelp(void)
	{
	vSERIAL_crlf();
	vNUMCMD_showCmdList(S_CrisisLogtext, CRISIS_OPTIONS_ARRAY_SIZE);
	vSERIAL_crlf();

	return;

	}/* END: vKEY_showCrisisHelp() */


/*********************  vKEY_restartSys()  ***********************************
*
*
******************************************************************************/
static void vKEY_restartSys(
		void
		)
	{
	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG)) return;

	vTask_Reset();
	}/* END: vKEY_restartSys() */




//#if 0
/********************  vKEY_restoreFRAM()  ***********************************
*
*
*
*****************************************************************************/
void vKEY_restoreFRAM(
		void
		)
	{
	vSERIAL_sout("RsetToOriginalConfig\r\n", 22);

	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG)) return;

	// Wipe fram
	vL2FRAM_force_fram_unformat();
	vSERIAL_sout("FRAM Wiped\r\n", 12);

	// perform a soft reset
	vTask_Reset();

	return;

	}/* END: vKEY_restoreFRAM() */

//#endif

/********************  vKEY_CheckRestoreSD()  ***********************************
*
*
*
*****************************************************************************/
void vKEY_ResetSDLogging(void)
{
	long lBegTime;
	uint8 ucChar;

	/* WUPS FRAM IS NOT FORMATTED -- CHECK IF WE SHOULD FORMAT IT*/
	ucChar = 0;
	vSERIAL_sout("\r\nWARNING this will clr SD of all data? [YorN] ", 47);
	lBegTime = lTIME_getSysTimeAsLong();
	while ((lTIME_getSysTimeAsLong() - lBegTime) < 10)
	{
		if (ucSERIAL_kbhit())
		{
			ucChar = ucSERIAL_bin();
			vSERIAL_bout(ucChar);
			break;
		}
	}/* END: while() */

	vSERIAL_crlf();

	/* FORMAT THE SD */
	if ((ucChar != 'Y') && (ucChar != 'y'))
	{
		//Also format the SD card since the block pointer in FRAM was just reset
		vSERIAL_sout("Fmting SD\r\n", 11);
		vSD_PowerOn();
		ucSD_Init();
		SD_Format();
		vSD_PowerOff();

		// reset SD pointer in FRAM
		vL2FRAM_SetSDCardBlockNum(SD_CARD_START_BLOCK);
	}
}

/********************  vKEY_DisplaySDBlockNum()  ***********************************
* \brief Reads the SD block num from fram and prints to screen. Runs from menu
* \param none
* \return none
*****************************************************************************/
void vKEY_DisplaySDBlockNum(void){
	ulong ulAddress;

	// Get the next free SD card address from FRAM
	ulAddress = ulL2FRAM_GetSDCardBlockNum();

	// Print block location to screen
	vSERIAL_sout("SDBlkNum: \r\n", 12);
	vSERIAL_HB32out(ulAddress);
	vSERIAL_crlf();

	return;

} /* END: vKEY_DisplaySDBlockNum() */

/*******************  vKEY_diagnostics()  ****************************************
*
*
*
******************************************************************************/
void vKEY_diagnostics(
		void
		)
	{
	//vFULLDIAG_run_module();
	vFULLDIAG_deployment_diagnostic();

	return;

	}/* END: vKEY_diagnostics() */





/*******************  vKEY_showTables()  ****************************************
*
*
*
******************************************************************************/
void vKEY_showTables(
		void
		)
	{

	/* SHOW THE NAME AND THE VERSION NUMBER */
	vMAIN_printIntro();

	/* TELL HOW MANY MESSAGES THERE ARE IN SRAM AND FLASH */
	{
	ulong ulMsgCount;
	ulMsgCount = (ulong)uiL2SRAM_getMsgCount();
	vSERIAL_sout("SramMsgs:New= ", 14);
	vSERIAL_UIV32out(ulMsgCount);
	vSERIAL_bout(',');
	/* SHOW THE FLASH MSG COUNTS */
	vKEY_showFlashMsgCounts();
	vSERIAL_crlf();
	}

	/* SHOW THE FLASH MSG COUNTS */
	vKEY_showFlashMsgCounts();
	vSERIAL_crlf();

	/* SHOW THE SCHED TABLE */
	//vSTBL_showAllStblEntrys();

	#if 1
	/* SHOW THE FRAM ST TBL */
	vL2FRAM_showTSBTbl();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	/* SHOW THE FRAM TRIGGER TBL */
	vL2FRAM_showYtriggerTbl();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	#endif


	return;

	}/* END: vKEY_showTables() */


/*******************  vKEY_setDate()  ****************************************
*
*
*
******************************************************************************/
static void vKEY_setDate(
		void
		)
	{
	DATE_STRUCT sDate;
	TIME_STRUCT sTime;
	uchar ucaStr[12];


	/* GET THE CURRENT RAM TIME */
	vDAYTIME_convertSecsToDstructAndTstruct(
		lTIME_getSysTimeAsLong(),
		&sDate,
		&sTime
		);

	vSERIAL_sout("SysDate= ", 9);
	vDAYTIME_convertDstructToShow(&sDate, NUMERIC_FORM);
	vSERIAL_crlf();

	vSERIAL_sout("EntrDat(MM/DD/YY): ", 19);
	if(ucSERIAL_getEditedInput(ucaStr,12))
		{
		vSERIAL_showXXXmsg();
		return;
		}

	if(cDAYTIME_convertDstrToDstruct(ucaStr, &sDate) < 0)
		{
		vSERIAL_showXXXmsg();
		return;
		}

	/* SET THE TIME */
	vDAYTIME_convertDstructAndTstructToSetSysTime(&sDate, &sTime);


	/* GET THE CURRENT RAM TIME */
	vDAYTIME_convertSecsToDstructAndTstruct(
		lTIME_getSysTimeAsLong(),
		&sDate,
		&sTime
		);

	vSERIAL_sout("SysDate= ", 9);
	vDAYTIME_convertDstructToShow(&sDate, NUMERIC_FORM);
	vSERIAL_crlf();

	return;

	}/* END: vKEY_setDate() */







/*******************  vKEY_setTime()  ****************************************
*
*
*
******************************************************************************/
static void vKEY_setTime(
		void
		)
	{
	DATE_STRUCT sDate;
	TIME_STRUCT sTime;
	uchar ucaStr[12];


	vSERIAL_sout("Tim= ", 5);

	vDAYTIME_convertSecsToDstructAndTstruct(
		lTIME_getSysTimeAsLong(),
		&sDate,
		&sTime
		);

	vDAYTIME_convertTstructToShow(&sTime);
	vSERIAL_crlf();

	vSERIAL_sout("EnterTim(HH:MM:SS): ", 20);
	if(ucSERIAL_getEditedInput(ucaStr,12))
		{
		vSERIAL_showXXXmsg();
		return;
		}

	if(cDAYTIME_convertTstrToTstruct(ucaStr, &sTime) < 0)
		{
		vSERIAL_showXXXmsg();
		return;
		}

	vDAYTIME_convertDstructAndTstructToSetSysTime(&sDate, &sTime);

	return;

	}/* END: vKEY_setTime() */





/*******************  vKEY_uploadDataToPC()  *********************************
*
*
*
******************************************************************************/
static void vKEY_uploadDataToPC(
		void
		)
	{
	uchar ucUploadTypeFlag;
	uchar ucaStr[4];

	vKEY_showFlashMsgCounts();

	vSERIAL_sout(" UploadSram(S)orFlsh(F)? ", 25);
	ucSERIAL_getEditedInput(&ucaStr[0], 2);		//lint !e534
	ucUploadTypeFlag = UPLOAD_FROM_NOBODY;
	if((ucaStr[0] == 's') || (ucaStr[0] == 'S')) ucUploadTypeFlag = UPLOAD_FROM_SRAM;
	if((ucaStr[0] == 'f') || (ucaStr[0] == 'F')) ucUploadTypeFlag = UPLOAD_FROM_FLASH;
	if(ucUploadTypeFlag == UPLOAD_FROM_NOBODY)
		{
		vSERIAL_showXXXmsg();
		return;
		}

	vSERIAL_sout("\r\n ", 3);
	vKEY_doUploadToPC(ucUploadTypeFlag);
	return;

	}/* END: vKEY_uploadDataToPC() */







/*******************  vKEY_clrMsgTail()  ************************************
*
* This routine zros the remainder of the msg buffer from the passed IDX to 31
*
*****************************************************************************/
void vKEY_clrMsgTail(
		uchar ucFirstFreeIdx
		)
	{
	uchar ucc;

	if(ucFirstFreeIdx >= 31) return;			//skip if full size

	for(ucc=ucFirstFreeIdx; ucc<=31;  ucc++)
		{
		ucaMSG_BUFF[ucc] = 0;

		}/* END: for(ucc) */

	return;

	}/* END: vKEY_clrMsgTail() */






/**********************  vKEY_doUploadToPC()  *******************************
*
* upload the messages to the PC.
*
*****************************************************************************/

void vKEY_doUploadToPC(
		uchar ucUploadType	//UPLOAD_FROM_SRAM,  UPLOAD_FROM_FLASH
		)
	{
	#define UPLD_BEG_TRANS	0xE1
	#define UPLD_MSG_OK		0xE2
	#define UPLD_MSG_RESEND	0xE3
	#define UPLD_END_TRANS	0xE4

	uint uiChar;
	uchar ucTmp;
	uchar uci;
	ulong ulMsgCount;
	ulong uli;
	uchar ucPacketSize;

	#ifdef FAKE_UPLOAD_MSGS
	#if (FAKE_UPLOAD_MSGS == YES)
	ulong ulRetryCounter;
	#endif
	#endif

	#ifdef FAKE_UPLOAD_MSGS
	#if (FAKE_UPLOAD_MSGS == YES)
	ucUploadType = UPLOAD_FROM_FAKE;
	#endif
	#endif

	#ifdef ESPORT_ENABLED			//defined in DIAG.h
//	vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//	vSERIAL_sout("\r\nE:Uplod\r\n", 11);
//	vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
	#endif

	/* CHECK IF WE HAVE ANY MSGS TO SEND */
	ulMsgCount = 0;
	switch(ucUploadType)
		{
		case UPLOAD_FROM_SRAM:
			ulMsgCount = (ulong)uiL2SRAM_getMsgCount();
			break;

		case UPLOAD_FROM_FLASH:
			//ulMsgCount = (ulong)lL2FRAM_getFlashUnreadMsgCount();
			break;

		#ifdef FAKE_UPLOAD_MSGS
		#if (FAKE_UPLOAD_MSGS == YES)
		case UPLOAD_FROM_FAKE:
			ulMsgCount = 50UL;
			break;
		#endif
		#endif

		default:
			vSERIAL_sout("BdUplodTyp\r\n", 12);
			ulMsgCount = 0;
			break;

		}/* END: switch() */

	if(ulMsgCount == 0)
		{
		vSERIAL_sout("NoMsgsFnd\r\n", 11);
		vSERIAL_sout("PicExitUplod\r\n\r\n", 16);
		return;
		}

	vSERIAL_sout("\r\nMsgCnt= ", 10);
	vSERIAL_UIV32out(ulMsgCount);
	vSERIAL_crlf();



	/*--------------  START THE UPLOAD PROCEDURE  --------------------------*/

	/* SEND A START CODE */
	vSERIAL_setBinaryMode(BINARY_MODE_ON);	//enter binary mode
	vSERIAL_bout(UPLD_BEG_TRANS);		//startup a transmission
	vSERIAL_bout(UPLD_BEG_TRANS);


	/* WAIT FOR A START SEND SIGNAL */
	uiChar = uiSERIAL_timedBin(9000);	
	if(uiChar != UPLD_BEG_TRANS)
		{
		vSERIAL_sout("KEY:ToutOnSt\r\n", 14);
		goto doUpload_xit;
		}


	/* START THE TRANSMISSION */	
	#ifdef FAKE_UPLOAD_MSGS
	#if (FAKE_UPLOAD_MSGS == YES)
	ulRetryCounter = 0;
	#endif
	#endif

	#ifdef ESPORT_ENABLED			//defined in DIAG.h
//	vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//	vSERIAL_sout("ST:Cnt= ", 8);
//	vSERIAL_UIV32out(ulMsgCount);
//	vSERIAL_crlf();
//	vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
	#endif

	for(uli=0;  uli<ulMsgCount; )		//loop for all messages in buffer
		{
		/* LOAD A PACKET INTO MSG BUFFER */
		if(uiChar != UPLD_MSG_RESEND)
			{
			switch(ucUploadType)
				{
				case UPLOAD_FROM_SRAM:
					ucTmp = ucL2SRAM_getCopyOfCurMsg();
					if(ucTmp == 0) goto doUpload_xit;
					ucPacketSize = ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ;
					ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF,ucPacketSize);
					break;

				case UPLOAD_FROM_FLASH:

					break;

				#ifdef FAKE_UPLOAD_MSGS
				#if (FAKE_UPLOAD_MSGS == YES)
				case UPLOAD_FROM_FAKE:
					/* BUILD A FAKE OM2 */
					vOPMODE_buildMsg_OM2(
						0,						//Flags
						uiRAD40_rom_convertStrToRad40("FK1"),//Dest Serial num
						(uchar)uli,				//Msg Seq Num
						0,						//Load Factor
						uiRAD40_rom_convertStrToRad40("FK2"),//Agent SN
						0x00001500L,			//Collection Time
						2,						//Data Count
						SENSOR_LIGHT_1,			//Sensor 0 number
						0x0150,					//Sensor 0 data
						SENSOR_LIGHT_2,			//Sensor 1 number
						0x0250,					//Sensor 1 data
						0,						//Sensor 2 number
						0,						//Sensor 2 data
						0,						//Sensor 3 number
						0						//Sensor 3 data
						);

					/* NOTE: CRC has been computed for this msg by build */

					break;

				#endif
				#endif

				default:
					ulMsgCount = 0; //for loop break;
					break;

				}/* END: switch() */

			}/* END: if() */


		#ifdef FAKE_UPLOAD_MSGS
		#if (FAKE_UPLOAD_MSGS == YES)
		{
		uchar ucSavedByte;

		/* PERFORM SPECIFIC FAULTS TO TEST THE RECOVERY PROCEDURES */
		if(uli == 10)
			{
			if(ulRetryCounter == 0) ucSavedByte = ucaMSG_BUFF[0];

			#if 0	//msg fail size underrun
			ucaMSG_BUFF[0] = ((ucaMSG_BUFF[0] & 0xE0) | ((ucaMSG_BUFF[0] & 0x3F)+1));//lint !e564
			#endif

			#if 1	//msg fail size too small
			ucaMSG_BUFF[0] = (ucaMSG_BUFF[0] & 0xE0) | 13;
			#endif

			#if 0	//msg fail size too big
			ucaMSG_BUFF[0] = (ucaMSG_BUFF[0] & 0xC0) | 32;
			#endif

			if(ulRetryCounter == 3) ucaMSG_BUFF[0] = ucSavedByte; //lint !e644

			}/* END: if() */
		}
		#endif
		#endif



		/* SEND THE PACKET TO THE PC*/
		for(uci=0; uci<=ucaMSG_BUFF[MSG_IDX_LEN] + CRC_SZ + NET_HDR_SZ; uci++){
			vSERIAL_bout(ucaMSG_BUFF[uci]);
			}

		/* WAIT FOR A PACKET OK MSG */
		uiChar = uiSERIAL_timedBin(10000);
		if((uiChar != UPLD_MSG_OK) && (uiChar != UPLD_MSG_RESEND))
			{
			#ifdef ESPORT_ENABLED			//defined in DIAG.h
//			vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//			vSERIAL_sout("PktOk=Bd= ", 10);
//			vSERIAL_HB16out(uiChar);
//			vSERIAL_crlf();
//			vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
			#endif

			goto doUpload_xit;
			}

		/* IF IT WAS A RESEND THEN LOOP BACK WITHOUT A NEW MSG */
		if(uiChar == UPLD_MSG_RESEND)
			{
			#ifdef FAKE_UPLOAD_MSGS
			#if (FAKE_UPLOAD_MSGS == YES)
			ulRetryCounter++;
			#endif
			#endif
			continue;
			}

		/* MSG WAS SENT COMPLETED --  DELETE THIS MSG AND GET THE NEXT ONE */
		if(ucUploadType == UPLOAD_FROM_SRAM) vL2SRAM_delCurMsg();

		/* BUMP THE MSG COUNTER */
		uli++;

		#ifdef FAKE_UPLOAD_MSGS
		#if (FAKE_UPLOAD_MSGS == YES)
		ulRetryCounter = 0;				//reset counter
		#endif
		#endif

		}/* END: for(uli) */


	#ifdef ESPORT_ENABLED			//defined in DIAG.h
//	vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//	vSERIAL_sout("LpXit\r\n", 7);
//	vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
	#endif


doUpload_xit:

	#ifdef ESPORT_ENABLED			//defined in DIAG.h
//	vESPORT_redirectOutput(REDIRECT_TO_ESPORT);
//	vSERIAL_sout("EotSnt\r\n", 8);
//	vSERIAL_sout("XT:MsgCnt= ", 11);
//	vSERIAL_UIV32out(ulMsgCount);
//	vSERIAL_sout("   uli= ", 8);
//	vSERIAL_UIV32out(uli);				//lint !e644
//	vSERIAL_crlf();
//	vESPORT_redirectOutput(REDIRECT_TO_SERIAL);
	#endif

	vSERIAL_bout(UPLD_END_TRANS);	//force an end

	uiChar = uiSERIAL_timedBin(3000);
	if(uiChar != UPLD_END_TRANS)
		{
		vSERIAL_sout("NoEndReply\r\n", 12);
		}

	vSERIAL_setBinaryMode(BINARY_MODE_OFF);
	vSERIAL_sout("PicXitingUplod\r\n\r\n", 18);
	vKEY_showFlashMsgCounts();

	vSERIAL_sout("MrkedFlshMsgs= ", 15);
	vSERIAL_UIV16out(uiGLOB_bad_flash_CRC_count);
	vSERIAL_crlf();

	return;

	}/* END: vKEY_doUploadToPC() */






/***************************  vKEY_showCurTime()  ********************************
*
* Show the current time of day
*
******************************************************************************/

void vKEY_showCurTime(
		void
		)
	{
	long lLongCurTime;
//	long lCurTimeSinceFirstHr;
	long lCurHr;
	long lCurMin;
	long lCurSec;
	long lQuotient;


	/* GRAB THE CURRENT TIME */
	lLongCurTime = lTIME_getSysTimeAsLong();

	lCurSec = lLongCurTime % 60;		//get the seconds
	lQuotient = lLongCurTime / 60;		//quotient units in minutes
	lCurMin = lQuotient % 60;			//get the minutes
	lQuotient = lQuotient / 60;			//quotient units in hours
	lCurHr = lQuotient % 24;			//get the hours

	vSERIAL_sout("UpTim= ", 7);
	vSERIAL_UIV16out((unsigned int)lCurHr);
	vSERIAL_bout(':');
	vSERIAL_UIV8out((uchar) lCurMin);
	vSERIAL_bout(':');
	vSERIAL_UIV8out((uchar) lCurSec);

	vSERIAL_sout("   Sec= ", 8);
	vSERIAL_I32out(lLongCurTime);
	vSERIAL_crlf();

//	lCurTimeSinceFirstHr = lLongCurTime - (long)iGLOB_Hr0_to_SysTim0_inSec;

	return;

	}/* END: vKEY_showCurTime() */




/*****************  vKEY_showFlashMsgCounts()  ********************************
*
*
*
******************************************************************************/
void vKEY_showFlashMsgCounts(
		void
		)
	{
	vSERIAL_sout("FlshMsgs:Old= ", 14);
	//vSERIAL_IV32out(lL2FRAM_getFlashReadMsgCount());
	vSERIAL_sout(",New= ", 6);
	//vSERIAL_UIV32out((ulong)lL2FRAM_getFlashUnreadMsgCount());
	vSERIAL_sout(",Free= ", 7);
	//vSERIAL_IV32out(lL2FRAM_getFlashFreeMsgCount());
	vSERIAL_crlf();

	return;

	}/* END: vKEY_showFlashMsgcounts() */






////////////////////////////////////////////////////////////////////////////
//! \fn vKEY_IDchange()
//!
//!	\brief Receives a new ID from the user and stores it in FRAM
//!
////////////////////////////////////////////////////////////////////////////
void vKEY_IDchange(
		void
		)
	{

	// HID or WiSARDNET address
	vSERIAL_sout("Hardware ID or ", 15);

	vSERIAL_crlf();
	vNUMCMD_showCmdList(S_SetIDtext, SETID_ARRAY_SIZE);
	vSERIAL_sout(">", 1);

	// Disable serial rx interrupts
	UART1_REG_IE &= ~UART1_RX_IE;

	vNUMCMD_doCmdInputAndDispatch(
				'>',												//prompt
				S_SetIDtext,								//command array
				vKey_SetID_func_ptrArray,		//dispatch function ptrs
				SETID_ARRAY_SIZE,													//array size
				NO_SHOW_INITIAL_PROMPT			//initial prompt flag
				);


	// Enable serial rx interrupts
	UART1_REG_IE |= UART1_RX_IE;

	return;

	}/* END: vKEY_IDchange() */


////////////////////////////////////////////////////////////////////////////
//! \fn vKEY_SetWiSARDNET_ID()
//!
//!	\brief Receives a new ID from the user and stores it in FRAM
//!
////////////////////////////////////////////////////////////////////////////
void vKEY_SetWiSARDNet_ID(
		void
		)
	{
	uchar ucStr[5];
	uint uiNewID;

	// Print out the list of valid characters
	vSERIAL_sout("IdChrs=  0-9", 12);
	vSERIAL_crlf();
	vSERIAL_crlf();

	vSERIAL_sout("CurId= ", 7);
	vL2FRAM_showSysID();
	vSERIAL_crlf();

	vSERIAL_sout("NewId(Less than 65535): ", 24);
	if(ucSERIAL_getEditedInput((uchar *)ucStr,5))
		{
		vSERIAL_showXXXmsg();
		goto IDchange_exit;
		}

	// Convert the ascii input to an integer
	uiNewID = (uint) lSERIAL_AsciiToNum(ucStr, UNSIGNED, DECIMAL);
	vL2FRAM_setSysID(uiNewID);

IDchange_exit:
	vSERIAL_sout("Id= ", 4);
	vL2FRAM_showSysID();
	vSERIAL_crlf();

	return;

	}/* END: vKEY_IDchange() */


////////////////////////////////////////////////////////////////////////////
//! \fn vKEY_Set_HID()
//!
//!	\brief Receives a hardware ID from the user and stores it in memory
//!
////////////////////////////////////////////////////////////////////////////
void vKEY_Set_HID(void)
{
	uint uiOldHID[4];
	uint uiNewHID[4];
	uchar ucStr[16];
	signed char cIndex;
	ulong ulNewIDLo,ulNewIDHi;
	signed char cBoardNumber;
	uchar ucAttachedSPBoards[NUMBER_SPBOARDS] = {0};

	vSERIAL_sout("Select a board\r\n", 16);
	vSERIAL_sout("  0: CP\r\n", 9);

	// Check what SPs are attached
	for (cIndex = 0; cIndex < NUMBER_SPBOARDS; cIndex++)
	{
		vSERIAL_UI8out(cIndex + 1);
		vSERIAL_sout(": ", 2);
		vSP_DisplayName(cIndex);
		vSERIAL_crlf();

		if (ucSP_IsAttached(cIndex))
		{
			ucAttachedSPBoards[(uchar)cIndex] = 1;
		}
	}
	vSERIAL_sout("  5: Radio\r\n", 12);

	if(ucSCC_IsAttached()){
		vSERIAL_sout("  6: SCC\r\n", 10);
	}

	vSERIAL_crlf();
	vSERIAL_sout(">", 1);


	if(ucSCC_IsAttached()){
		// Wait for user to select a board
		cBoardNumber = cNUMCMD_getCmdNum(6);
	}
	else{
		cBoardNumber = cNUMCMD_getCmdNum(5);
	}
	// Print out the list of valid characters
	vSERIAL_sout("IdChrs=  0-F", 12);
	vSERIAL_crlf();


	// Get the previous HID from the board
	if (cBoardNumber == 0)
	{
		vFlash_GetHID(uiOldHID);
	}
	else if(cBoardNumber == 5){
		vFlash_GetRadioHID(uiOldHID);
	}
	else if(cBoardNumber == 6){
		ucSCC_GetHID((uchar*)uiOldHID);
	}
	else
	{
		// Check if the board is attached
		if(ucAttachedSPBoards[(uint)cBoardNumber-1]) //decrement board count to index into table
		{
			// Read the HID from the SP driver
			ucSP_GetHID((cBoardNumber - 1) , (uchar*)uiOldHID);
		}
		else
		{
			vSERIAL_sout("Location is Empty\r\n", 19);
			return;
		}
	}


	vSERIAL_sout("HID = ", 6);
	for (cIndex = 3; cIndex >= 0; cIndex--)
		vSERIAL_HB16out(uiOldHID[(uchar)cIndex]);
	vSERIAL_crlf();

	vSERIAL_sout("Enter new 64-bit ID: ", 21);
	if (ucSERIAL_getEditedInput((uchar *) ucStr, 16))
	{
		vSERIAL_showXXXmsg();
		return;
	}

	// Convert the ascii input to a number
	ulNewIDLo = lSERIAL_AsciiToNumber(&ucStr[8], UNSIGNED, HEX, 8);
	ulNewIDHi = lSERIAL_AsciiToNumber(&ucStr[0], UNSIGNED, HEX, 8);

	// Move number to an array
	uiNewHID[0] = (uint) ulNewIDLo;
	uiNewHID[1] = (uint) (ulNewIDLo >> 16);
	uiNewHID[2] = (uint) ulNewIDHi;
	uiNewHID[3] = (uint) (ulNewIDHi >> 16);



	if (cBoardNumber == 0)
	{
	// Write the new ID to flash
	vFlash_SetHID(uiNewHID);

	// Read it back out and make sure that the change took effect
	vFlash_GetHID(uiOldHID);
	}
	else if(cBoardNumber == 5){
		// Write the new ID to flash
		vFlash_SetRadioHID(uiNewHID);

		// Read it back out and make sure that the change took effect
		vFlash_GetRadioHID(uiOldHID);
	}
	else if(cBoardNumber == 6){
		ucSCC_SetHID((uchar*)uiNewHID);
		// Clean out the ID array
		uiNewHID[0] = 0;
		uiNewHID[1] = 0;
		uiNewHID[2] = 0;
		uiNewHID[3] = 0;

		// Read the HID from the SP driver
		ucSCC_GetHID((uchar*)uiNewHID);
	}
	else
	{
		ucSP_Start(cBoardNumber - 1);
		ucSP_SetHID(cBoardNumber - 1, (uchar *)uiNewHID);
		vSP_TurnOff(cBoardNumber - 1);
		// Clean out the ID array
		uiNewHID[0] = 0;
		uiNewHID[1] = 0;
		uiNewHID[2] = 0;
		uiNewHID[3] = 0;

		// Read the HID from the SP driver
		ucSP_GetHID((cBoardNumber - 1) , (uchar*)uiNewHID);
	}

	// Display changed (hopefully) ID to user
	vSERIAL_sout("HID = ", 6);
	for (cIndex = 3; cIndex >= 0; cIndex--)
		vSERIAL_HB16out(uiNewHID[(uchar)cIndex]);
	vSERIAL_crlf();

}


/////////////////////////////////////////////////////////////////////////////
//! \fn vKey_CrisisLog
//! \brief Function reads data elements from flash, assembles messages out of them,
//!	and sends them out over serial
//!
//////////////////////////////////////////////////////////////////////////////
void vKey_CrisisLog(void){

	vSERIAL_crlf();
	vNUMCMD_showCmdList(S_CrisisLogtext, CRISIS_OPTIONS_ARRAY_SIZE);
	vSERIAL_sout(">", 1);

	vNUMCMD_doCmdInputAndDispatch(
				'>',												//prompt
				S_CrisisLogtext,								//command array
				vKey_Crisis_func_ptrArray,		//dispatch function ptrs
				CRISIS_OPTIONS_ARRAY_SIZE,													//array size
				NO_SHOW_INITIAL_PROMPT			//initial prompt flag
				);

}


/////////////////////////////////////////////////////////////////////////////
//! \fn vKey_ReadCrisisLog
//! \brief Function erases all of crisis logging
//!
//////////////////////////////////////////////////////////////////////////////
void vKEY_EraseCrisis(void){

	vReport_EraseCrisisLog();

}

/////////////////////////////////////////////////////////////////////////////
//! \fn vKey_ReadCrisisLog
//! \brief Function reads data elements from flash, assembles messages out of them,
//!	and sends them out over serial
//!
//////////////////////////////////////////////////////////////////////////////
void vKEY_ReadCrisis(void){

	vReport_PrintCrisisLog();

}

/*******************  vKEY_versionNumHeader()  *******************************
*
*
*
******************************************************************************/
void vKEY_versionNumHeader(
		void
		)
	{

	/* SHOW THE NAME AND THE VERSION NUMBER */
	vMAIN_printIntro();

	return;

	}/* END: vKEY_versionNumHeader() */










/*-------------------------------  MODULE END  ------------------------------*/
