

/**************************  FULLDIAG.C  *************************************
*
* Routines to fully diagnose the system
*
*
* V1.00	01/19.2005 wzr
*		started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1) ... */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include <msp430x54x.h>		//register and ram definition file
#include "std.h"			//standard defines
#include "diag.h"			//diagnostic header
#include "config.h" 		//configuration parameters 
#include "serial.h"			//serial port routines
#include "main.h"			//
#include "misc.h"			//homeless functions
#include "fram.h"			//Ferro ram memory functions
#include "l2fram.h"			//Level 2 FRAM routines
#include "fulldiag.h"		//full system diagnostic functions
#include "l2flash.h"		//level 2 flash routines
#include "delay.h"			//delay timer routines
#include "buz.h"			//Buzzer routines
#include "dradio.h"			//Diag Radio routines
#include "adf7020.h"			//Radio Channel Defines
#include "sensor.h"			//Sensor name routine
#include "comm.h"			//comm protocol routines
#include "task.h"			//task manager routines
#include "mem_mod.h"		//Memory module routines
#include "modact.h"
#include "led.h"
#include "SP.h"
#include "gs.h" // to access report to garden server functionality
#include "crc.h"					//CRC calculation module
#include "report.h" // access to message writing functionality
#include "time.h"
#include "SD_Card_Testing.h"

extern struct S_Attached_SP_Fields S_AttachedSP;

extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];

//extern volatile uchar ucaSDSPI_BUFF[ MAX_SDSPI_SIZE ];

extern volatile union						//ucGLOB_diagFlagByte1
  {
	uchar ucByte;
	struct
	 {
	 unsigned DIAG_mute_bit:1;					//bit 0 ;1=MUTE, 0=SOUND
	 unsigned DIAG_test_repeat_looping_bit:1;	//bit 1 ;1=LOOPING, 0=NOT LOOPING
	 unsigned DIAG_only_a_single_test_bit:1;	//bit 2	;1=ONLY A SINGLE, 0=ALL
	 unsigned DIAG_only_a_single_group_bit:1;	//bit 3 ;1=ONLY A GROUP, 0=ALL
	 unsigned DIAG_exit_out_to_top_bit:1;		//bit 4 ;1=EXIT OUT, 0=NO EXIT OUT
	 unsigned DIAG_exit_out_to_prev_test_bit:1;	//bit 5 ;1=GOTO PREV, 0=NO GOTO PREV
	 unsigned DIAG_exit_out_to_next_test_bit:1;	//bit 6 ;1=GOTO NEXT, 0=NO GOTO NEXT
	 unsigned DIAG_quit_out_to_return_bit:1;	//bit 7 ;1=TOTO RETURN, 0=NO QUIT
	 }diagFlagByte1_STRUCT;

  }ucGLOB_diagFlagByte1;

#define DIAG_FLAG_BYTE_1_INIT_VAL 0b00000000

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
  		unsigned FLG3_UNUSED_BIT6 :1;
  		unsigned FLG3_GSV_COM_BIT :1; // 1=GSV com is active 0= GSV com is inactive
  	} FLAG3_STRUCT;
  } ucFLAG3_BYTE;

extern volatile union						//ucGLOB_diagFlagByte2
  {
	uchar ucByte;
	struct
	 {
	 unsigned DIAG_tell_whats_running_bit:1;		//bit 0 ;1=TELL, 0=NO TELL
	 unsigned DIAG_change_looping_sts_bit:1;		//bit 1 ;1=CHANGE LOOPING, 0=NO CHANGE
	 unsigned DIAG_halt_on_error_bit:1;				//bit 2 ;1=halt, 0=no halt
	 unsigned DIAG_partially_automated_run_bit:1;	//bit 3	;1=an 'A' was hit, 0=no 'A' hit
	 unsigned DIAG_start_bit:1;				//bit 4 ;
	 unsigned DIAG_not_used_5_bit:1;				//bit 5 ;
	 unsigned DIAG_not_used_6_bit:1;				//bit 6 ;
	 unsigned DIAG_mode_main_bit:1;				//bit 7 ;
	 }diagFlagByte2_STRUCT;

  }ucGLOB_diagFlagByte2;

#define DIAG_FLAG_BYTE_2_INIT_VAL 0b00000000

// test global var for diagnostics

#define DUR_STATE_START 0x00
#define DUR_STATE_CMD 0x01
#define DUR_STATE_RQST 0x02
#define DUR_STATE_RPT 0x03
#define DUR_STATE_IDLE 0x04

uchar Global_DUR_count = 0;
extern uint  uiGLOB_sramQcnt;

// struct which CP will use to return diagnostic results
typedef struct{
	uchar SP_slot1[8];
	uchar SP_slot2[8];
	uchar SP_slot3[8];
	uchar SP_slot4[8];
	uchar LED_test;
	uchar radio_disc_test;
	uchar radio_op_test;
	uchar buzzer_test;
	uchar valve_test;
	uchar SD_test;
	uchar FRAM_test;
	uchar SP_slot1_types[4];
	uchar SP_slot2_types[4];
	uchar SP_slot3_types[4];
	uchar SP_slot4_types[4];
	uchar diagMode;
} results;

#define SP_EMPTY 0x00
#define SP_STM_PRESENT 0x01
#define SP_CM_STM_PRESENT 0x02
#define SP_ST 0x03
#define SP_SL 0x04

#define DIAG_TEST_PASS 0x01
#define DIAG_TEST_FAIL 0x00

T_Text S_AutoTestExitStr = { "AutoTstXit\r\n", 12 };

/* DECLARATIONS */
uchar ucFULLDIAG_exit_next_repeat(
		void
		);


/* DECLARE A FUNCTION TYPE FIRST */
typedef uchar (*GENERIC_DIAG_FUNC)(void);


/************************  FRAM tables here  *********************************/

#define FRAM_DIAG_ARRAY_SIZE 3

#define FRAM_TEST_0_REPEAT_COUNT 10
#define FRAM_TEST_1_REPEAT_COUNT 10
#define FRAM_TEST_2_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE FRAM ARRAY */
T_Text S_FRAM_FuncDesc[FRAM_DIAG_ARRAY_SIZE] =
 {
		 {"TstLoLevelFramCalls", 19},
		 {"TstLevel2FramCalls", 18},
		 {"TstToCatch1stByteBadProblm", 26},
 };


/* FUNCTION DECLARATIONS HERE */
//static uchar ucFULLDIAG_FRAM_0(void);
static uchar ucFULLDIAG_FRAM_1(void);
static uchar ucFULLDIAG_FRAM_2(void);


const GENERIC_DIAG_FUNC ucFRAM_DIAG_PtrArray[FRAM_DIAG_ARRAY_SIZE] =
 {
 //ucFULLDIAG_FRAM_0,
 ucFULLDIAG_FRAM_1,
 ucFULLDIAG_FRAM_2
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaFRAM_testRepeatCountArray[FRAM_DIAG_ARRAY_SIZE] =
 {
 FRAM_TEST_0_REPEAT_COUNT,
 FRAM_TEST_1_REPEAT_COUNT,
 FRAM_TEST_2_REPEAT_COUNT
 };


/**********************  FLASH tables here  **********************************/

#define FLASH_DIAG_ARRAY_SIZE 1

#define FLASH_TEST_0_REPEAT_COUNT 10
#define FLASH_TEST_1_REPEAT_COUNT 10
#define FLASH_TEST_2_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE FLASH ARRAY */
T_Text S_FLASH_FuncDesc [FLASH_DIAG_ARRAY_SIZE]= { {"NIY", 3}, };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_FLASH_0(void);


const GENERIC_DIAG_FUNC ucFLASH_DIAG_PtrArray[FLASH_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_FLASH_0
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaFLASH_testRepeatCountArray[FLASH_DIAG_ARRAY_SIZE] =
 {
 FLASH_TEST_0_REPEAT_COUNT
 };


/*********************  SDBOARD tables here  *********************************/

#define SDBOARD_DIAG_ARRAY_SIZE 2

#define SDBOARD_TEST_0_REPEAT_COUNT 10
#define SDBOARD_TEST_1_REPEAT_COUNT 10
#define SDBOARD_TEST_2_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE SDBOARD ARRAY */
T_Text S_SDBOARD_FuncDesc[SDBOARD_DIAG_ARRAY_SIZE] =
 {
		 {"BrkOutOfSleepBitFollowProg", 26},
		 {"ChkIfSdBbdVernumXmitsOK", 23},
 };


// TODO these aren't used
/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_SDBOARD_0(void){return 0;}
static uchar ucFULLDIAG_SDBOARD_1(void){return 0;}


const GENERIC_DIAG_FUNC ucSDBOARD_DIAG_PtrArray[SDBOARD_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_SDBOARD_0,
 ucFULLDIAG_SDBOARD_1
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaSDBOARD_testRepeatCountArray[SDBOARD_DIAG_ARRAY_SIZE] =
 {
 SDBOARD_TEST_0_REPEAT_COUNT,
 SDBOARD_TEST_1_REPEAT_COUNT
 };


/************************  RADIO tables here  *********************************/

#define RADIO_DIAG_ARRAY_SIZE 4

#define RADIO_TEST_0_REPEAT_COUNT 10
#define RADIO_TEST_1_REPEAT_COUNT 10
#define RADIO_TEST_2_REPEAT_COUNT 10
#define RADIO_TEST_3_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE RADIO ARRAY */
T_Text S_RADIO_FuncDesc[RADIO_DIAG_ARRAY_SIZE] =
 {
		 {"TstRdioXmitPwr", 14},
		 {"TstRdioPath", 11},
		 {"ChkSndRecSwtchDIODE", 19},
		 {"Sniffer", 7},
 };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_RADIO_0(void);
static uchar ucFULLDIAG_RADIO_1(void);
static uchar ucFULLDIAG_RADIO_2(void);
//static uchar ucFULLDIAG_RADIO_3(void);


const GENERIC_DIAG_FUNC ucRADIO_DIAG_PtrArray[RADIO_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_RADIO_0,
 ucFULLDIAG_RADIO_1,
 ucFULLDIAG_RADIO_2,
 //ucFULLDIAG_RADIO_3
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaRADIO_testRepeatCountArray[RADIO_DIAG_ARRAY_SIZE] =
 {
 RADIO_TEST_0_REPEAT_COUNT,
 RADIO_TEST_1_REPEAT_COUNT,
 RADIO_TEST_2_REPEAT_COUNT,
 RADIO_TEST_3_REPEAT_COUNT
 };


/********************* SENSOR DIAGNOSTIC TABLES HERE  ***********************/

#define SENSOR_DIAG_ARRAY_SIZE 6

#define SENSOR_TEST_0_REPEAT_COUNT 10
#define SENSOR_TEST_1_REPEAT_COUNT 10
#define SENSOR_TEST_2_REPEAT_COUNT 10
#define SENSOR_TEST_3_REPEAT_COUNT 10
#define SENSOR_TEST_4_REPEAT_COUNT 10
#define SENSOR_TEST_5_REPEAT_COUNT 10


/* THIS IS A PARALLEL ARRAY TO THE SENSOR ARRAY */
T_Text S_SENSOR_FuncDesc[SENSOR_DIAG_ARRAY_SIZE] =
 {
		 {"ReadLight", 9},
		 {"ReadSoilMoists", 14},
		 {"ReadThermos", 11},
		 {"VaisalaAveWindSp&Dir", 20},
		 {"VaisalaAirPress&AirTemp", 23},
		 {"VaisalaRelHum&RainAccum", 23},
 };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_SENSOR_0(void);
static uchar ucFULLDIAG_SENSOR_1(void);
static uchar ucFULLDIAG_SENSOR_2(void);
static uchar ucFULLDIAG_SENSOR_3(void);
static uchar ucFULLDIAG_SENSOR_4(void);
static uchar ucFULLDIAG_SENSOR_5(void);


/* ARRAY OF POINTERS TO THE DIAGNOSTIC FUNCTIONS */
const GENERIC_DIAG_FUNC ucSENSOR_DIAG_PtrArray[SENSOR_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_SENSOR_0,			//array of functions
 ucFULLDIAG_SENSOR_1,
 ucFULLDIAG_SENSOR_2,
 ucFULLDIAG_SENSOR_3,
 ucFULLDIAG_SENSOR_4,
 ucFULLDIAG_SENSOR_5
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaSENSOR_testRepeatCountArray[SENSOR_DIAG_ARRAY_SIZE] =
 {
 SENSOR_TEST_0_REPEAT_COUNT,
 SENSOR_TEST_1_REPEAT_COUNT,
 SENSOR_TEST_2_REPEAT_COUNT,
 SENSOR_TEST_3_REPEAT_COUNT,
 SENSOR_TEST_4_REPEAT_COUNT,
 SENSOR_TEST_5_REPEAT_COUNT
 };


/************************  BUZ tables here  *********************************/

#define BUZ_DIAG_ARRAY_SIZE 1

#define BUZ_TEST_0_REPEAT_COUNT 1


/* THIS IS A PARALLEL ARRAY TO THE BUZ ARRAY */
T_Text S_BUZ_FuncDesc[BUZ_DIAG_ARRAY_SIZE] =
 {
		 {"AllBuzSounds", 12},
 };


/* FUNCTION DECLARATIONS HERE */
static uchar ucFULLDIAG_BUZ_0(void);


/* ARRAY OF POINTERS TO THE DIAGNOSTIC FUNCTIONS */
const GENERIC_DIAG_FUNC ucBUZ_DIAG_PtrArray[BUZ_DIAG_ARRAY_SIZE] =
 {
 ucFULLDIAG_BUZ_0,
 };


/* PARALLEL ARRAY CONTAINING THE REPEAT LOOP COUNTS FOR EACH TEST */
const uint uiaBUZ_testRepeatCountArray[BUZ_DIAG_ARRAY_SIZE] =
 {
 BUZ_TEST_0_REPEAT_COUNT
 };


/***************  TOP (GROUP) ARRAY STUFF HERE  ******************************/
// was 6, changed to 7 to introduce new option
#define NUMBER_OF_DIAG_GROUPS 6


const GENERIC_DIAG_FUNC *pfpGroupPtrArray[NUMBER_OF_DIAG_GROUPS] =
 {
 ucFRAM_DIAG_PtrArray,
 ucFLASH_DIAG_PtrArray,
 ucSDBOARD_DIAG_PtrArray,
 ucRADIO_DIAG_PtrArray,
 ucSENSOR_DIAG_PtrArray,
 ucBUZ_DIAG_PtrArray
 };


/* THIS IS A PARALLEL ARRAY to GroupPtrArray OF ARRAY SIZES */
const uchar ucaDiagCountPerGroup[NUMBER_OF_DIAG_GROUPS] =
 {
 FRAM_DIAG_ARRAY_SIZE,
 FLASH_DIAG_ARRAY_SIZE,
 SDBOARD_DIAG_ARRAY_SIZE,
 RADIO_DIAG_ARRAY_SIZE,
 SENSOR_DIAG_ARRAY_SIZE,
 BUZ_DIAG_ARRAY_SIZE
 };


/* THIS IS PARALLEL ARRY TO GroupPtrArray OF GROUP NAMES */
T_Text S_GroupName[NUMBER_OF_DIAG_GROUPS] =
 {
		 {"FramTest", 8},
		 {"FlashTest", 9},
		 {"SD-Test", 7},
		 {"RadioTest", 9},
		 {"SensorTest", 10},
		 {"BuzTest", 7},
 //"ChangeParam" // new option to support alteration of runtime parameters
 };


/* THIS IS A PARALLEL ARRAY TO GroupPtrArray of FUNCTION DESCRIPTORS */
T_Text *S_pGroupsFuncDesc[NUMBER_OF_DIAG_GROUPS] =
 {
 S_FRAM_FuncDesc,
 S_FLASH_FuncDesc,
 S_SDBOARD_FuncDesc,
 S_RADIO_FuncDesc,
 S_SENSOR_FuncDesc,
 S_BUZ_FuncDesc
 };


/* PARALLEL ARRAY that tells what the repeat loop count is for each test */
const uint *uipaGROUPS_testRepCntPtr[NUMBER_OF_DIAG_GROUPS] =
  {
  uiaFRAM_testRepeatCountArray,
  uiaFLASH_testRepeatCountArray,
  uiaSDBOARD_testRepeatCountArray,
  uiaRADIO_testRepeatCountArray,
  uiaSENSOR_testRepeatCountArray,
  uiaBUZ_testRepeatCountArray
  };


#if 0

///********************  ucFULLDIAG_getSingleCharCmd()  ************************
//*
//* This routine accepts a sting of acceptable characters and
//* returns the position in the string that matches.
//*
//* RET:	char position of matched char
//*		if no match is found -- it does not exit
//*
//*****************************************************************************/
//
//uchar ucFULLDIAG_getSingleCharCmd(
//		const rom char *cpMsgOutStr,			//lint !e125
//		const rom char *cpAcceptableCharSet	//lint !e125
//		)
//	{
//	uchar ucChar;
//	uchar ucMatchChar;
//	uchar uci;
//	uchar ucForBreakFlg;
//
//	while(TRUE)		//lint !e774
//		{
//		/* SETUP, GIVE THE LEADING MSG, AND GET AN ANSWER */
//		vSERIAL_rom_sout(cpMsgOutStr);
//		ucChar = ucSERIAL_bin();
//		vSERIAL_bout(ucChar);					//echo the char
//
//		/* SEARCH THE ACCEPTABLE STRING FOR A MATCH */
//		ucForBreakFlg = 0;
//		for(uci=0; ;  uci++)
//			{
//			ucMatchChar = (uchar)cpAcceptableCharSet[uci];
//
//			if(ucMatchChar == 0) break;
//
//			if(ucMatchChar == ucChar)
//				{
//				ucForBreakFlg = 1;
//				break;
//				}
//
//			if((ucMatchChar >= 'a') && (ucMatchChar <= 'z'))
//				{
//				uchar ucRaisedMatchChar = ucMatchChar & ~0b00100000;
//				if(ucRaisedMatchChar == ucChar)
//					{
//					ucForBreakFlg = 1;
//					break;
//					}
//				}
//
//			}/* END: for() */
//
//		/* IF A MATCH WAS FOUND LEAVE */
//		if(ucForBreakFlg == 1)
//			{
//			vSERIAL_crlf();
//			break;
//			}
//
//		vSERIAL_showXXXmsg();
//
//		}/* END: while(TRUE) */
//
//	return(uci);
//
//	}/* END: ucFULLDIAG_getSingleCharCmd() */

#endif


/********************  ucFULLDIAG_getChoiceOrAll()  ************************
*
* This routine accepts a number or the letter 'A' or 'X'.
*   returns the number if it was a number
*   returns the letter 'A' if it was an 'A'
*   returns the letter 'X' if it was an 'X' or 'Q'
*
* RET:	char position of matched char
*		if no match is found -- it does not exit
*
*****************************************************************************/

uchar ucFULLDIAG_getChoiceOrAll(
		char *cpMsgOutStr,			//lint !e125
		uint uiStrLen,
		uchar ucMaxNumPlusOne
		)
	{
	unsigned long ulValue;
	uchar ucStr[13];

	while(TRUE)		//lint !e774
		{
		/* SETUP, GIVE THE LEADING MSG, AND GET AN ANSWER */
		vSERIAL_sout(cpMsgOutStr, uiStrLen);

		/* GET THE STRING REPLY */
		if(ucSERIAL_getEditedInput(ucStr, 13))
			{
			vSERIAL_showXXXmsg();
			return('X');
			}

		/* NOW CHECK IF ITS A LEGAL NUMBER */
		if((ucStr[0] >= '0') && (ucStr[0] <= '9'))	//was it numeric?
			{
			/* IT WAS NUMERIC TEXT -- CONVERT IT TO A NUMBER */
			ulValue = (unsigned long)lSERIAL_AsciiToNum(ucStr, UNSIGNED, DECIMAL);
			/* RANGE CHECK THE NUMBER */
			if(ulValue < ucMaxNumPlusOne)
				{
				/* IT WAS A GOOD NUMBER  -- RETURN IT */
				return((uchar) ulValue);
				}
			}

		/* NOW CHECK IF THE STRING ENTERED WAS AN 'A' */
		if((ucStr[0] == 'a') || (ucStr[0] == 'A'))
			{
			return('A');
			}

		/* NOW CHECK IF THE STRING ENTERED WAS AN 'A' */
		if((ucStr[0] == 'x') || (ucStr[0] == 'X') ||
		   (ucStr[0] == 'q') || (ucStr[0] == 'Q'))
			{
			return('X');
			}


		/* IF WE GOT HERE THE ENTRY WAS INVALID */
		vSERIAL_showXXXmsg();

		}/* END: while(TRUE) */

	}/* END: ucFULLDIAG_getChoiceOrAll() */


/****************  ucFULLDIAG_doRunTimeKeyHitChk()  **************************
*
* RunTime Keys are:
*	 none - RET= 0  none 
*		  - RET= 1  no action key
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*
* RET:	0 if not key
*		n if an action key was hit
*			(flag byte was also set)
*
*****************************************************************************/

uchar ucFULLDIAG_doRunTimeKeyHitChk(
		void
		)
	{
	uchar ucChar;
	uchar ucRetVal;

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;	//assume none

	if(ucSERIAL_kbhit())
		{
		ucChar = ucSERIAL_bin();
		switch(ucChar)
			{
			case 'h':		//Toggle Halt on Error flag
			case 'H':
				if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit)
					{
					ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit = 0;
					vSERIAL_sout("\r\nNoHaltOnErrs\r\n", 16);
					}
				else
					{
					ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit = 1;
					vSERIAL_sout("\r\nHaltOnErrs\r\n", 14);
					}

				ucRetVal = DIAG_ACTION_KEY_TOGGLE_HALT_ON_ERR;
				break;

			case 'l':		//toggle looping
			case 'L':
				ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_change_looping_sts_bit = 1;
				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit)
					vSERIAL_sout("\r\nDiagIsUnlooped\r\n", 18);
				else
					vSERIAL_sout("\r\nDiagIsLooped\r\n", 16);

				ucRetVal = DIAG_ACTION_KEY_TOGGLE_LOOP_FLAG;
				break;

			case 'm':		//toggle the mute bit
			case 'M':
				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_mute_bit)
					{
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_mute_bit = 0;
					vSERIAL_sout("\r\nBuzIsUnmute\r\n", 15);
					}
				else
					{
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_mute_bit = 1;
					vSERIAL_sout("\r\nBuzIsMute\r\n", 13);
					}

				ucRetVal = DIAG_ACTION_KEY_TOGGLE_MUTE_FLAG;
				break;


			case 'n':		//go to next test
			case 'N':
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit = 1;
				vSERIAL_sout("\r\nGoNxtTst\r\n", 12);

				ucRetVal = DIAG_ACTION_KEY_NEXT_TEST_FLAG;
				break;

			case 'p':		//go to previous test
			case 'P':
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit = 1;
				vSERIAL_sout("\r\nGoPrevTst\r\n", 13);

				ucRetVal = DIAG_ACTION_KEY_PREV_TEST_FLAG;
				break;

			case 'q':		//quit out of all tests and exit routine
			case 'Q':
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit = 1;
				vSERIAL_sout("\r\nQuitTsts\r\n", 12);

				ucRetVal = DIAG_ACTION_KEY_QUIT;
				break;

			case 's':		//Stop running
			case 'S':
				vSERIAL_sout("\r\nPrgmStop(S)toGo...", 20);
				while(TRUE)	//lint !e774
					{
					ucChar = ucSERIAL_bin();
					if((ucChar == 's') || (ucChar == 'S')) break;
					}
				ucRetVal = DIAG_ACTION_KEY_PLAIN_KEY;
				break;

			case 't':		//Tell what this diag is
			case 'T':
				ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_tell_whats_running_bit = 1;
				ucRetVal = DIAG_ACTION_KEY_TELL;
				break;

			case 'x':		//exit out of test and start over 
			case 'X':
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit = 1;
				vSERIAL_sout("\r\nXitToTopDiag\r\n", 16);
				ucRetVal = DIAG_ACTION_KEY_EXIT;
				break;

			default:
				ucRetVal = DIAG_ACTION_KEY_PLAIN_KEY;
				break;

			}/* END: switch(ucChar) */

		}/* END: if() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_doRunTimeKeyHitChk() */


/**********************  vFULLDIAG_run_module()  *****************************
*
* This is the full diagnostic main control routine.
*
*****************************************************************************/

void vFULLDIAG_run_module(
		void
		)
	{
	uchar ucChar;

	uchar ucGroupAnsIdx;
	uchar ucGroupIdx;

	uchar ucTestAnsIdx;
	uchar ucTestNumIdx;

	uchar ucc;

	uchar ucGroupLpStart;
	uchar ucGroupLpEnd;
	uchar ucGroupLpInc;
	uchar ucGidx;

	uchar ucTestLpStart;
	uchar ucTestLpEnd;
	uchar ucTestLpInc;
	uchar ucTidx;
	uchar ucTestRet;

	uint uiRepLpStart;
	uint uiRepLpEnd;
	uint uiRepLpInc;
	uint uiRidx;


	while(TRUE)	//lint !e716 !e774
		{
		/* INITIALIZE */
		//ucGLOB_diagFlagByte1.ucByte = DIAG_FLAG_BYTE_1_INIT_VAL;
		//ucGLOB_diagFlagByte2.ucByte = DIAG_FLAG_BYTE_2_INIT_VAL;

		ucGLOB_diagFlagByte1.ucByte = 0x00;

		ucGLOB_diagFlagByte2.ucByte = 0x00;

		/***********  Select Desired Option  ****************/
		//vSERIAL_sout("Run Diagnostics or Modify Runtime Parameters?", 45);
		// get selection from the user



		/***********  PICK OUT A GROUP FIRST  ****************/

		vSERIAL_sout("DiagGrpsAre:\r\n", 14);
		for(ucc=0;  ucc<NUMBER_OF_DIAG_GROUPS;  ucc++)	//list the groups
			{
			vSERIAL_UI8_2char_out(ucc,' ');
			vSERIAL_sout(": ", 2);
			vSERIAL_sout(S_GroupName[ucc].m_cText, S_GroupName[ucc].m_uiLength);
			vSERIAL_crlf();
			}/* END: for(ucc) */

		vSERIAL_crlf();

		ucGroupAnsIdx = ucFULLDIAG_getChoiceOrAll(
							"Pick one or A for all: ",
							23,
							NUMBER_OF_DIAG_GROUPS
							);

		ucGroupIdx = ucGroupAnsIdx;
 		ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit = 1;

		if(ucGroupAnsIdx == 'A')
			{
			ucGroupIdx = 0;
			ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit = 0;
			ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit = 1;
			}
		if(ucGroupAnsIdx == 'X')
			{
			break;
			}

//		// option to change runtime parameters
//		if(ucGroupAnsIdx == 'C')
//			{
//			// perform the modification of the interval
//			vMODACT_modifyInterval();
//			//vMODACT_modifyActionTbl();
//			break;
//			}




		/******  NOW PICK OUT A TEST TO RUN  -- IF NOT RUNNING ALL *********/
		ucTestAnsIdx = 0;						//default
		ucTestNumIdx = 0;						//default
		if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit)
			{
			vSERIAL_sout("DiagsAre:\r\n", 11);

			for(ucc=0;  ucc<ucaDiagCountPerGroup[ucGroupIdx];  ucc++)	//list the groups
				{
				vSERIAL_UI8_2char_out(ucc,' ');
				vSERIAL_sout(": ", 2);
				vSERIAL_sout((S_pGroupsFuncDesc[ucGroupIdx])[ucc].m_cText, (S_pGroupsFuncDesc[ucGroupIdx])[ucc].m_uiLength);
				vSERIAL_crlf();
				}/* END: for(ucc) */

			vSERIAL_crlf();

			ucTestAnsIdx = ucFULLDIAG_getChoiceOrAll(
								"Pick one or A for all: ",
								23,
								ucaDiagCountPerGroup[ucGroupIdx]
								);

			ucTestNumIdx = ucTestAnsIdx;
			ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit = 1;


			if(ucTestAnsIdx == 'A')	// all tests
				{
				ucTestNumIdx = 0;
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit = 0;
				ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit = 1;
				}

			if(ucTestAnsIdx == 'X')
				{
				continue;
				}

			}/* END: if() */



//		vSERIAL_sout("FellIntoTsts\r\n", 14);



		/************ AT THIS POINT WE ARE READY TO RUN *******************/
		vSERIAL_sout("RunTsts...\r\n", 12);

		/* GROUP LOOP SETUP */
		ucGroupLpStart = 0;
		ucGroupLpEnd   = NUMBER_OF_DIAG_GROUPS;
		ucGroupLpInc   = 1;
		if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit) //change setup if only one group
			{
			ucGroupLpStart = ucGroupIdx;
			ucGroupLpInc = 0;
			}

		/* GROUP LOOP */
		for(ucGidx=ucGroupLpStart; ucGidx<ucGroupLpEnd;	ucGidx+=ucGroupLpInc)
			{

			/* TEST LOOP SETUP */
			ucTestLpStart = 0;
			ucTestLpEnd   = ucaDiagCountPerGroup[ucGidx];
			ucTestLpInc   = 1;
			if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit) //change setup if only one group
				{
				ucTestLpStart = ucTestNumIdx;
				ucTestLpInc = 0;
				}

			/* TEST LOOP */
			for(ucTidx=ucTestLpStart; ucTidx<ucTestLpEnd;  ucTidx+=ucTestLpInc)
				{

				/* REPEAT LOOP SETUP */
				uiRepLpStart = 0;
				uiRepLpEnd   = (uipaGROUPS_testRepCntPtr[ucGidx])[ucTidx];
				uiRepLpInc   = 1;
				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit) //change setup if only one group
					{
					uiRepLpInc = 0;
					}

				/* REPEAT LOOP */
				for(uiRidx=uiRepLpStart; uiRidx<uiRepLpEnd; uiRidx+=uiRepLpInc)
					{
					/* RUN THE TEST AND TEST THE RESULTING FLAG */
					ucTestRet = (pfpGroupPtrArray[ucGidx])[ucTidx]();
					switch(ucTestRet)
						{
						case DIAG_ACTION_KEY_ERR:
							if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit)
								{
								vSERIAL_sout("\r\nHltOnErr\'H\'GotoNxtHlt,\'G\'=GoNoHlt...", 38);
								while(TRUE) //lint !e774
									{
									ucChar = ucSERIAL_bin();
									if((ucChar == 'g') || (ucChar == 'G'))
										{
										ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_halt_on_error_bit = 0;
										vSERIAL_bout(ucChar);
										vSERIAL_crlf();
										break;
										}
									if((ucChar == 'h') || (ucChar == 'H'))
										{
										vSERIAL_bout(ucChar);
										vSERIAL_crlf();
										break;
										}
									}/* END: while() */

								}/* END: if() */
							break;
							
						case DIAG_ACTION_KEY_EXIT:
							ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit = 1;
							break;	

						default:
							break;

						}/* END: switch() */


					/* CHECK FOR ANY KEY HITS */
					if((ucFULLDIAG_doRunTimeKeyHitChk() > DIAG_ACTION_KEY_PLAIN_KEY) ||
					   (ucTestRet > DIAG_ACTION_KEY_ERR))
						{
						if((ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit) ||
						   (ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit) ||
						   (ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit) ||
						   (ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit)
						   )
							{
							break;
							}
						
						if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_change_looping_sts_bit)
							{
							ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_change_looping_sts_bit = 0;

							if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit)
								{
								ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit = 0;
								ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit = 0;
								uiRepLpInc = 1;
								ucTestLpInc = 1;
								break;
								}
							else
								{
								ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_test_repeat_looping_bit = 1;
								uiRepLpInc = 0;
								continue;
								}
							}

						if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_tell_whats_running_bit)
							{
							vSERIAL_crlf();
							vSERIAL_sout((S_pGroupsFuncDesc[ucGidx])[ucTidx].m_cText, (S_pGroupsFuncDesc[ucGidx])[ucTidx].m_uiLength);
							vSERIAL_crlf();
							ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_tell_whats_running_bit = 0;

							#if 0
							vSERIAL_sout("ucGidx= ", 8);
							vSERIAL_UI8out(ucGidx);
							vSERIAL_crlf();
							vSERIAL_sout("ucTidx= ", 8);
							vSERIAL_UI8out(ucTidx);
							vSERIAL_crlf();
							#endif

							continue;
							}

						}/* END: if() */

					}/* END: for(uiRidx) */	/* END: of REPEAT LOOP */



				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit)
					//ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit;

					{
					break;
					}

				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit)
					{
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit = 0;
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit = 0;
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit = 0;
					break;
					}

				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit)
					{
					if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit)
						{
						ucTestLpInc = 0;
						if(ucTidx != 0) ucTidx--;
						}
					else
						{
						ucTestLpInc = 1;
						if(ucTidx != 0) ucTidx--;
						ucTidx--;
						}
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_prev_test_bit = 0;
					continue;
					}

				if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit)
					{
					ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_next_test_bit = 0;
					if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_test_bit)
						{
						ucTestLpInc = 0;
						if(ucTidx < (ucaDiagCountPerGroup[ucGidx])-1) ucTidx++;
						}
					else
						{
						ucTestLpInc = 1;
						}
					continue;
					}

				}/* END: for(ucTidx) */  /* END of TEST LOOP */




			if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit)
				{
				break;
				}

			if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit)
				{
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_only_a_single_group_bit = 0;
				ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_exit_out_to_top_bit = 0;
				break;
				}

			}/* END: for(ucGidx) */  /* END of GROUP LOOP */




		if(ucGLOB_diagFlagByte1.diagFlagByte1_STRUCT.DIAG_quit_out_to_return_bit)
			{
			break;
			}

		}/* END: while() */  /* END of DIAG LOOP */

	vSERIAL_sout("XitDiag\r\n", 9);

	return;

	}/* END: vFULLDIAG_run_module() */


/*******************  FRAM DIAGNOSTICS HERE  ********************************/


/**********************  ucFULLDIAG_FRAM_0()  ********************************
*
* Routine to test the low level FRAM calls
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

/*
uchar ucFULLDIAG_FRAM_0(
		void
		)
	{
	uchar ucRetVal;
	uchar ucVal;
	uint uiVal;
	USL uslVal;		
	unsigned long ulVal;

	 SHOW THE INTRO
	vSERIAL_printDashIntro(cpFRAM_FuncDesc[0]);

	 ASSUME NO ERRORS
	ucRetVal = DIAG_ACTION_KEY_NONE;

	vFRAM_init();

	 TEST THE B8 WRITE AND READ ROUTINES
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B8(FRAM_TEST_ADDR, 0x12);
	ucVal = ucFRAM_read_B8(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(ucVal != 0x12)
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B8 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)0x12,
			(unsigned long)ucVal
			);

		vBUZ_morrie();
		}
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B8(FRAM_TEST_ADDR, (~0x12 & 0xFF));
	ucVal = ucFRAM_read_B8(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(ucVal != (~0x12 & 0xFF))
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B8 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)(~0x12 & 0xFF),
			(unsigned long)ucVal
			);

		vBUZ_morrie();
		}




	 TEST THE B16 WRITE AND READ ROUTINES
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B16(FRAM_TEST_ADDR, 0x1234);
	uiVal = uiFRAM_read_B16(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(uiVal != 0x1234)
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B16 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)0x1234,
			(unsigned long)uiVal
			);

		vBUZ_morrie();
		}
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B16(FRAM_TEST_ADDR, (~0x1234 & 0xFFFF));
 	uiVal = uiFRAM_read_B16(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(uiVal != (~0x1234 & 0xFFFF))
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B16 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)(~0x1234 & 0xFFFF),
			(unsigned long)uiVal
			);

		vBUZ_morrie();
		}



	 TEST THE B24 WRITE AND READ ROUTINES
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	//vFRAM_write_B24(FRAM_TEST_ADDR, 0x123456);
	//uslVal = uslFRAM_read_B24(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(uslVal != 0x123456)
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B24 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)0x123456,
			(unsigned long)uslVal
			);

		vBUZ_morrie();
		}
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B24(FRAM_TEST_ADDR, (~0x123456 & 0xFFFFFF));
	uslVal = uslFRAM_read_B24(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(uslVal != (~0x123456 & 0xFFFFFF))
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B24 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)(~0x123456 & 0xFFFFFF),
			(unsigned long)uslVal
			);

		vBUZ_morrie();
		}




	 TEST THE B32 WRITE AND READ ROUTINES
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B32(FRAM_TEST_ADDR, 0x12345678);
	ulVal = ulFRAM_read_B32(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(ulVal != 0x12345678)
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B32 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)0x12345678,
			(unsigned long)ulVal
			);

		vBUZ_morrie();
		}
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_write_B32(FRAM_TEST_ADDR, (~0x12345678 & 0xFFFFFFFF));
	ulVal = ulFRAM_read_B32(FRAM_TEST_ADDR);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	if(ulVal != (~0x12345678 & 0xFFFFFFFF))
		{
		ucRetVal = 1;						//indicate and error
		vMSG_showStorageErr(
			"FRAM B32 write failure",
			(unsigned long)FRAM_TEST_ADDR,
			(unsigned long)(~0x12345678 & 0xFFFFFFFF),
			(unsigned long)ulVal
			);

		vBUZ_morrie();
		}


	vFRAM_quit();

	return(ucRetVal);

	} END: ucFULLDIAG_FRAM_0()
*/







/**********************  ucFULLDIAG_FRAM_1()  ********************************
*
* Routine to test the level 2 FRAM calls
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_FRAM_1(
		void
		)
	{
	uchar ucRetVal;
	uint uiVersionNum;
	
	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_FRAM_FuncDesc[1]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* CHECK IF THE FRAM IS THERE */
	if(ucL2FRAM_chk_for_fram(FRAM_CHK_REPORT_MODE) == 0)
		{
		vSERIAL_sout("ChkFramFail\r\n", 13);
		vBUZ_morrie();
		ucRetVal = 1;
		return(ucRetVal);
		}
	vSERIAL_sout("FramDetected\r\n", 14);



	/* WE ARE ABOUT TO CHANGE THE FRAM MEMORY -- CHECK IF IT IS FORMATTED */
	vSERIAL_sout("ChkingForFramFmt\r\n", 18);
	if(uiL2FRAM_chk_for_fram_format())		//are we formatted?
 		{
		/* FRAM IS FORMATED */
		vSERIAL_sout("FramIsFmtted\r\n", 14);


		/* IF THIS IS AN Auto TEST -- LEAVE */
		if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
			{
			vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
			return(ucRetVal);
			}

		/* NO THIS IS A MANUALLY RUN TEST -- ASK FOR AN OVERRIDE */
		vSERIAL_sout("OverwriteFram?(YorN)...", 23);

		/* GET THE CONFIRMATION */
		{
		uchar ucStr[4];
		if(ucSERIAL_getEditedInput(ucStr, 2)) return(DIAG_ACTION_KEY_EXIT);
		if((ucStr[0] != 'Y') && (ucStr[0] != 'y')) return(DIAG_ACTION_KEY_EXIT);
		}

		}/* END: if() */

	vSERIAL_sout("Tsting...\r\n", 11);




	/* IF YOU ARE HERE, FRAM IS EITHER UNFORMATTED, OR HAS AN OPERATOR OVERRIDE */

	/* WIPE THE FRAM  & CHECK THE WIPE */
	vSERIAL_sout("SetFramTo0x99\r\n", 15);
	vFRAM_fillFramBlk(0, 512, 0x99);
	vFRAM_show_fram(0, 24);
	/* CHECK THE FRAM WIPE */
	vSERIAL_sout("ChkingFramSet\r\n", 15);
	if(ucFRAM_chk_fram_blk(0,512,0x99) == 0)
		{
		vSERIAL_sout("FramSetBd\r\n", 11);
		vBUZ_morrie();
		ucRetVal = 1;
		return(ucRetVal);
		}
	else
		{
		vSERIAL_sout("FramSetOK\r\n", 11);
		}


	/* NOW FORMAT THE FRAM */
	vSERIAL_sout("FmtOfFram\r\n", 11);
	vL2FRAM_format_fram();
	vFRAM_show_fram(0, 24);
	/* CHECK THE FRAM FORMAT */
	vSERIAL_sout("ChkingFramFmt\r\n", 15);
	if(uiL2FRAM_chk_for_fram_format() == 0)
		{
		vSERIAL_sout("FramFmtBd\r\n", 11);
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_sout("FramFmtOK\r\n", 11);
		}
		
	/* CHECK THE FORMAT VERSION NUMBER */
	vSERIAL_sout("ChkingFramVer\r\n", 15);
	uiVersionNum = uiL2FRAM_get_version_num();
	vSERIAL_sout("FramVerChk: RdVer= ", 19);
	vSERIAL_HB16out(uiVersionNum);
	vSERIAL_sout(" ShouldBeVer= ", 14);
	vSERIAL_HB16out(FRAM_VERSION);
	vSERIAL_crlf();
	if(uiVersionNum == FRAM_VERSION)
		{
		vSERIAL_sout("FramVerOK\r\n", 11);
		}
	else
		{
		vSERIAL_sout("FramVerBd\r\n", 11);
		vBUZ_morrie();
		ucRetVal = 1;
		}


	/* NOW UNFORMAT THE FRAM */
	vSERIAL_sout("UnfmtOfFram\r\n", 13);
	vL2FRAM_force_fram_unformat();
	vFRAM_show_fram(0, 24);
	/* NOW CHECK THE UNFORMAT */
	vSERIAL_sout("ChkingFramUNfmt\r\n", 17);
	if(uiL2FRAM_chk_for_fram_format() != 0)
		{
		vSERIAL_sout("FramUNfmtBd\r\n", 13);
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_sout("FramUNfmtOK\r\n", 13);
		}


	/* NOW RE-FORMAT THE FRAM */
	vSERIAL_sout("RefmtOfFram\r\n", 13);
	vL2FRAM_format_fram();
	vFRAM_show_fram(0, 24);
	/* CHECK THE FRAM RE-FORMAT */
	vSERIAL_sout("ChkingFramRefmt\r\n", 17);
	if(uiL2FRAM_chk_for_fram_format() == 0)
		{
		vSERIAL_sout("FramRefmtBd\r\n", 13);
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_sout("FramRefmtOK\r\n", 13);
		}

	return(ucRetVal);

	}/* END: ucFULLDIAG_FRAM_1() */


/**********************  ucFULLDIAG_FRAM_2()  ********************************
*
* "FRAM2: Special test to catch the first byte is bad problem"
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_FRAM_2(
		void
		)
	{
	uchar ucRetVal;
//	uchar ucTmp;
//	uint uii;
	

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_FRAM_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* TURN ON THE SD BOARD */
	vSERIAL_sout("1stTstIsWithSDpwred\r\n", 21);
//	if(ucSDCTL_start_SD_and_specify_boot(SD_BOOT_LOADER_DISABLED) == 0)
//		{
//		vSERIAL_sout("BdSDstart\r\n", 11);
//		vBUZ_morrie();
//		ucRetVal = 1;
//		return(ucRetVal);
//		}

	/* CHECK IF THE FRAM IS THERE */
	if(ucL2FRAM_chk_for_fram(FRAM_CHK_REPORT_MODE) == 0)
		{
		vSERIAL_sout("ChkFramFailRet\r\n", 16);
		vBUZ_morrie();
		ucRetVal = 1;
//		vSDCTL_halt_SD_and_power_down();
		return(ucRetVal);
		}
	vSERIAL_sout("FramDetectd\r\n", 13);



	/* WIPE THE FRAM  & CHECK THE WIPE */
	vSERIAL_sout("FramTstLocSetTo0x99\r\n", 21);
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	/* CHECK THE FRAM WIPE */
	vSERIAL_sout("ChkingFramTstArea\r\n", 19);
	if(ucFRAM_chk_fram_blk(FRAM_TEST_ADDR, 4, 0x99) == 0)
		{
		vSERIAL_sout("FramSetBd\r\n", 11);
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_sout("FramSetOK\r\n", 11);
		}


	/* NOW WE ARE GOING TO CLEAR THE FRAM TEST AREA BUT WITH SERIAL PORT INPUT */
	vSERIAL_sout("FramTstLocSetTo0x66AfterCommInput\r\n", 35);

	#if 1
	vDELAY_wait100usTic(20000);				//this causes fails too
	#endif

	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x66);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	/* CHECK THE FRAM WIPE */
	vSERIAL_sout("ChkOfFramSet\r\n", 14);
	if(ucFRAM_chk_fram_blk(FRAM_TEST_ADDR, 4, 0x66) == 0)
		{
		vSERIAL_sout("FramSetBd\r\n", 11);
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_sout("FramSetOK\r\n", 11);
		}


	/* POWER DOWN THE SD BOARD */
//	vSDCTL_halt_SD_and_power_down();


	/* LEAVE SD BOARD POWERED DOWN */
	vSERIAL_sout("2ndPartOfTstWithSDPwredOff\r\n", 29);
	vSERIAL_sout("TstShouldFail,IfNot,SDmayBeBd\r\n", 31);

	/* WIPE THE FRAM  & CHECK THE WIPE */
	vSERIAL_sout("FramTstLocSetTo0x99\r\n", 21);
	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x99);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);

	/* NOW WE ARE GOING TO CLEAR THE FRAM TEST AREA BUT WITH SERIAL PORT INPUT */
	vSERIAL_sout("FramTstLocSetTo0x66AfterSerialInput\r\n", 37);

	#if 1
	vDELAY_wait100usTic(20000);				//this causes fails too
	#endif

	vFRAM_fillFramBlk(FRAM_TEST_ADDR, 4, 0x66);
	vFRAM_show_fram(FRAM_TEST_ADDR, 4);
	/* CHECK THE FRAM WIPE */
	vSERIAL_sout("ChkingFramLocForBdSet\r\n", 23);
	if(ucFRAM_chk_fram_blk(FRAM_TEST_ADDR, 4, 0x66) == 1)
		{
		vSERIAL_sout("FramSetBdFail\r\n", 15);
		vBUZ_morrie();
		ucRetVal = 1;
		}
	else
		{
		vSERIAL_sout("FramSetBdOK\r\n", 13);
		}

	return(ucRetVal);

	}/* END: vFULLDIAG_FRAM_2() */


/*********************  FLASH DIAGNOSTICS HERE  *****************************/


/**********************  FULLDIAG_FLASH_0()  ********************************
*
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_FLASH_0(
		void
		)
	{

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_FLASH_FuncDesc[0]);

	return(0);

	}/* END: ucFULLDIAG_FLASH_0() */


/*****************  RADIO DIAGNOSTICS HERE  ******************************/


/**********************  ucFULLDIAG_RADIO_0()  ****************************
*
* "  RADIO_0: Measure the Transmit power of the radio  ",
*
*
* This routine turns on the radio in XMIT mode and hangs there.
* This is so the radio XMIT current can be measured.
* The signal is steady (not pulsed) so that the full power can be measured.
*
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_RADIO_0(
		void
		)
	{
  /*
	uchar ucRetVal;
	uchar ucChar;

	// SHOW THE INTRO
	vSERIAL_printDashIntro(cpRADIO_FuncDesc[0]);

	// ASSUME NO ERRORS
	ucRetVal = DIAG_ACTION_KEY_NONE;

	// DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC
	if(ucGLOB_diagFlagByte2.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	// LOAD THE RADIO
	vRADIO_init();

	// FORCE THE RADIO TO HIGHEST TRANSMIT POWER
	vRADIO_setTransmitPower(XMIT_PWR_HI);

	// TURN RADIO ON
	PORTJbits.RADIO_MODE_BIT = 0;			//radio mode = XMIT
	PORTJbits.RADIO_STANDBY_BIT = 1;		//turn radio on
	PORTGbits.RADIO_ANTENNA_BIT = 1;		//XMIT antenna

	// TURN ON THE TRANSMIT BIT
	PORTGbits.RADIO_TRANSMIT_BIT = 0;
	vSERIAL_sout("InXmit0Mode\r\n", 13);
	vSERIAL_sout("SpaceCharSwitchsMode,or\'X\'toXit\r\n", 33);
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 'x':
		case 'X':
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			break;

		default:
			break;

		}// END: switch()

	if(ucRetVal != DIAG_ACTION_KEY_EXIT)
		{
		PORTGbits.RADIO_TRANSMIT_BIT = 1;
		vSERIAL_sout("Xmit1Mode\r\n", 11);
		vSERIAL_sout("SpaceChrToSwtchMode,or\'X\'toXit\r\n", 32);
		while(!ucSERIAL_kbhit());			//lint !e722
		}

	// MAKE SURE RADIO IS SHUT OFF
	vRADIO_quit();

	vSERIAL_sout("Xit\r\n", 5);
	return(ucRetVal);
  */
  return(DIAG_ACTION_KEY_NONE);

	}// END: ucFULLDIAG_RADIO_0()






/**********************  ucFULLDIAG_RADIO_1()  ****************************
*
* "RADIO_1: Characterize the messaging error",
*
* This routine has a sender section and a receiver section.  The sender
* sends out a known message and the receiver receives it and checks it
* for an exact bit for bit match.  If it matches it goes ahead and waits
* for the next message.  If not it prints out an error message and then
* goes ahead and waits for the next message.
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_RADIO_1(
		void
		)
	{
	uchar ucRetVal;
	uchar ucChar;
	uchar ucChanIdx;


	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_RADIO_FuncDesc[1]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	/* CHOOSE CHANNEL FREQUENCY */
	vSERIAL_sout("PickFreq\'O\'=Op,\'D\'=Discvr,\'T\'=Tst...", 36);
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 'd':
		case 'D':
			vSERIAL_sout("Discvr\r\n", 8);
			//ucChanIdx = DISCOVERY_CHANNEL_INDEX;
			ucChanIdx = 2;
			break;

		case 'o':
		case 'O':
			vSERIAL_sout("Op\r\n", 4);
			//ucChanIdx = DATA_CHANNEL_INDEX;
			ucChanIdx = 1;
			break;

		case 't':
		case 'T':
			vSERIAL_sout("Tst\r\n", 5);
			//ucChanIdx = TEST_CHANNEL_INDEX;
			ucChanIdx = 3;
			break;


		default:
			vSERIAL_showXXXmsg();
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			goto Radio_1_exit;

		}/* END: switch() */


/*

	 CHOOSE TRANSMITTER OR RECEIVER
	vSERIAL_sout("Choose\'T\'=Xmit,\'R\'=Rec,\'X\'=Xit...", 33);
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 't':
		case 'T':
			vSERIAL_sout("Xmit\r\n", 6);
			ucRetVal = ucDRADIO_continuousXmitTestMsg(ucChanIdx);
			break;

		case 'r':
		case 'R':
			vSERIAL_sout("Rec\r\n", 5);
			ucRetVal = ucDRADIO_continuousReceiveTestMsg(ucChanIdx);
			break;

		default:
			vSERIAL_showXXXmsg();
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			break;

		} END: switch()
*/


Radio_1_exit:

	return(ucRetVal);

	}/* END: ucFULLDIAG_RADIO_1() */






/**********************  ucFULLDIAG_RADIO_2()  ****************************
*
* "RADIO_2: Check the Send/Receive switching DIODE"
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/

uchar ucFULLDIAG_RADIO_2(
		void
		)
	{
  /*
	uchar ucRetVal;
	uchar ucChar;

	// SHOW THE INTRO
	vSERIAL_printDashIntro(cpRADIO_FuncDesc[2]);

	// ASSUME NO ERRORS
	ucRetVal = DIAG_ACTION_KEY_NONE;

	// DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC
	if(ucGLOB_diagFlagByte2.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	// LOAD THE RADIO
	vRADIO_init();

	// TURN RADIO ON
	PORTJbits.RADIO_MODE_BIT = 0;			//radio mode = XMIT
	PORTJbits.RADIO_STANDBY_BIT = 1;		//turn radio on
	PORTGbits.RADIO_ANTENNA_BIT = 1;		//XMIT antenna


	// NOW SWITCH THE MODE BIT BETWEEN SEND AND RECEIVE
	vSERIAL_sout("SwchBitBetweenXmt&Rec\r\n", 23);
	vSERIAL_sout("\'X\'toXit\r\n", 10);
	vSERIAL_sout("Start\r\n", 7);
	while(!ucSERIAL_kbhit())
		{
		if(ucSERIAL_kbhit()) break;		//chk for key hit

		vSERIAL_bout('T');
		PORTJbits.RADIO_MODE_BIT = 0;	//radio mode = XMIT
		vDELAY_wait100usTic(10000);		//switch delay

		vSERIAL_bout('R');
		PORTJbits.RADIO_MODE_BIT = 1;	//radio mode = RECEIVE
		vDELAY_wait100usTic(10000);		//switch delay

		}


	// HANDLE THE KEY HIT THAT BROKE OUT OF THE WHILE
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 'x':
		case 'X':
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			break;

		default:
			break;

		}

	// MAKE SURE RADIO IS SHUT OFF
	vRADIO_quit();

	vSERIAL_sout("Xit\r\n", 5);
	return(ucRetVal);
  */
  return(DIAG_ACTION_KEY_NONE);

	}






/**********************  ucFULLDIAG_RADIO_3()  ****************************
*
* "RADIO_3: Radio Message Sniffer",
*
*
* RET:
*	 none - RET= 0  no errors 
*		  - RET= 1  found errors
*		H - RET= 2  Toggle halt on error flag
*		L - RET= 3  Toggle looping flag
*		M - RET= 4  Toggle Mute flag
*		N - RET= 5  go to next test (flag)
*		P - RET= 6  go to previous test (flag)
*		Q - RET= 7  quit out to return from Diagnostic
*		S - RET= 8  Stop running
*		T - RET= 9  Tell what is running bit
*		X - RET=10  exit out of this test
*
*****************************************************************************/
/*

uchar ucFULLDIAG_RADIO_3(
		void
		)
	{
	uchar ucRetVal;
	uchar ucChar;
	uchar ucChanIdx;


	 SHOW THE INTRO
	vSERIAL_printDashIntro(cpRADIO_FuncDesc[3]);

	 ASSUME NO ERRORS
	ucRetVal = DIAG_ACTION_KEY_NONE;

	 DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	 CHOOSE CHANNEL FREQUENCY
	vSERIAL_sout("PickFreq\'O\'=Op,\'D\'=Discvr,\'T\'=Tst...", 36);
	ucChar = ucSERIAL_bin();
	switch(ucChar)
		{
		case 'd':
		case 'D':
			vSERIAL_sout("Discvr\r\n", 8);
			//ucChanIdx = DISCOVERY_CHANNEL_INDEX;
			ucChanIdx = 2;
			break;

		case 'o':
		case 'O':
			vSERIAL_sout("Op\r\n", 4);
			//ucChanIdx = DATA_CHANNEL_INDEX;
			ucChanIdx = 1;
			break;

		case 't':
		case 'T':
			vSERIAL_sout("Tst\r\n", 5);
			//ucChanIdx = TEST_CHANNEL_INDEX;
			ucChanIdx = 3;
			break;

		default:
			vSERIAL_showXXXmsg();
			ucRetVal = DIAG_ACTION_KEY_EXIT;
			goto Radio_3_exit;

		} END: switch()

	vSERIAL_sout("Sniff On\r\n", 9);
	ucRetVal = ucDRADIO_run_sniffer(ucChanIdx);

Radio_3_exit:
	return(ucRetVal);

	} END: ucFULLDIAG_RADIO_3()

*/


/*****************  SENSOR DIAGNOSTICS HERE  ******************************/


/**********************  ucFULLDIAG_SENSOR_0()  ****************************
*
* "SENSOR_0: Light sensor readings from all light sensors",
*
* This routine reads Light 1,2,3,4 repeatedly.
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_0(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucjj;
	uint uiVal;
	uchar ucSensorID[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_SENSOR_FuncDesc[0]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	for(ucii=0; ucii<4;  ucii++)
		{
		ucSensorID[ucii] = SENSOR_LIGHT_1 + ucii;
		}/* END: for() */

	for(ucii=0;  ;  ucii++)
		{
		/* SHOW HEADER LINE */
		if((ucii %16) == 0)
			{
			vSERIAL_sout("\r\n   L1     L2     L3     L4\r\n", 30);
			}

	    //vACTION_read_sensors_into_buffer(ucSensorID);

	    for(ucjj=0; ucjj<4; ucjj++)
		   	{			
			/* SHOW THE LIGHT SENSOR VALUES */
			uiVal = uiMISC_buildUintFromBytes(
				(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(3 * ucjj)],
				NO_NOINT
				);
			vSERIAL_UI16out(uiVal);
			vSERIAL_sout("  ", 2);

	        }// END: for(ucjj = 0)

		vSERIAL_crlf(); //print out a new line of records

		if(ucSERIAL_kbhit()) break;

        }/* END: while() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_0() */






/**********************  ucFULLDIAG_SENSOR_1()  ****************************
*
* "SENSOR_1: Soil Moisture readings from all moisture sensors"
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_1(
		void
		)
	{
	uchar ucRetVal;
	uchar ucc;
	uchar ucii;
	uint uiVal;
	uchar ucSensorID[4];
//	long lVal;
//	long lReading[2];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_SENSOR_FuncDesc[1]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	ucSensorID[0] = SENSOR_SOIL_MOISTURE_1;
	ucSensorID[1] = SENSOR_SOIL_MOISTURE_2;
	ucSensorID[2] = 0;
	ucSensorID[3] = 0;

	for(ucc=0;  ;  ucc++)
		{
		/* SHOW HEADER LINE */
		if((ucc %16) == 0)
			{
			vSERIAL_sout("\r\nSOIL1  SOIL2\r\n", 16);
			}

	    //vACTION_read_sensors_into_buffer(ucSensorID);

		/* SHOW THE SENSOR VALUES */
		for(ucii=0; ucii<2;  ucii++)
			{
			uiVal = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(3 * ucii)],
				 	NO_NOINT
				 	);

			//lReading[ucii] = (long)uiVal;

			vSERIAL_UI16out(uiVal);
			vSERIAL_sout("  ", 2);

			}/* END: for(ucii) */

		vSERIAL_crlf();

		if(ucSERIAL_kbhit()) break;

		vDELAY_wait100usTic(5000);				//wait 1/2 sec

		}/* END: for(ucc) */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_1() */





/********************  vFULLDIAG_showTCgroupOf3()  **************************
*
* This routine shows the Vactual, OwireColdJctTemp, and the Kun Converted value
*
*****************************************************************************/

/*void vFULLDIAG_showTCgroupOf3(
		int iV_actual,
		int iT_coldJct_in_C
		)
	{
	int iT_in_C;
	int iT_in_F;

	vSERIAL_I16outFormat(iV_actual,4);
	vSERIAL_bout(' ');

	vSERIAL_I16outFormat(iT_coldJct_in_C, 4);
	vSERIAL_bout(' ');

	iT_in_C = iTHERMO_computeTfromTC(iV_actual, iT_coldJct_in_C);
	iT_in_F = iTHERMO_convertOwire_C_to_F(iT_in_C);
	vTHERMO_showOwireValue(iT_in_F);
	vSERIAL_bout(' ');

	return;

	} END: vFULLDIAG_showTCgroupOf3() */





/**********************  ucFULLDIAG_SENSOR_2()  ****************************
*
* "SENSOR_2: Thermocouple reading from all thermocouples"
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_2(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucjj;
	int iV_actual;
	int iT_coldJct_in_C;
	uchar ucSensorID1[4];
	uchar ucSensorID2[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_SENSOR_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	ucSensorID1[0] = SENSOR_TC_1;
	ucSensorID1[1] = SENSOR_ONEWIRE_0;
	ucSensorID1[2] = SENSOR_TC_2;
	ucSensorID1[3] = SENSOR_ONEWIRE_0;

	ucSensorID2[0] = SENSOR_TC_3;
	ucSensorID2[1] = SENSOR_ONEWIRE_0;
	ucSensorID2[2] = SENSOR_TC_4;
	ucSensorID2[3] = SENSOR_ONEWIRE_0;

	for(ucii=0; ; ucii++)
		{

	    //vACTION_read_sensors_into_buffer(ucSensorID1);

		/* SHOW THE HEADER PERIODICALLY */
		if((ucii % 24) == 0)
			vSERIAL_sout("\r\n TC1 OWIRE Deg_F    TC2 OWIRE Deg_F    TC3 OWIRE Deg_F    TC4 OWIRE Deg_F\r\n", 77);

#if 0
 TC1 OWIRE Deg_F    TC2 OWIRE Deg_F    TC3 OWIRE Deg_F    TC4 OWIRE Deg_F\r\n")";
xxxx +xxx -xx.yy   xxxx +xxx -xx.yy   xxxx +xxx -xx.yy   xxxx +xxx -xx.yy;
#endif
		/* SHOW THE SENSOR VALUES */
		for(ucjj=0; ucjj<2;  ucjj++)
			{
			iV_actual = (int)uiMISC_buildUintFromBytes(
						(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucjj)],
						NO_NOINT
						);
			iT_coldJct_in_C = (int)uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucjj)],
				 	NO_NOINT
				 	);
			//vFULLDIAG_showTCgroupOf3(iV_actual, iT_coldJct_in_C);

			vSERIAL_sout("  ", 2);

			}/* END: for(ucjj) */

		/* GET DATA FOR TC 3&4  */
	    //vACTION_read_sensors_into_buffer(ucSensorID2);

		/* SHOW THE SENSOR VALUES */
		for(ucjj=0; ucjj<2;  ucjj++)
			{

			iV_actual = (int)uiMISC_buildUintFromBytes(
						(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucjj)],
						NO_NOINT
						);
			iT_coldJct_in_C = (int)uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucjj)],
				 	NO_NOINT
				 	);
			//vFULLDIAG_showTCgroupOf3(iV_actual, iT_coldJct_in_C);

			vSERIAL_sout("  ", 2);

			}/* END: for(ucjj) */

		vSERIAL_crlf();

		if(ucSERIAL_kbhit()) break;

		}/* END: for(ucii) */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_2() */





/**********************  ucFULLDIAG_SENSOR_3()  ****************************
*
* "SENSOR_3: Vaisala Ave Wind Speed & Direction",
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_3(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucSensorNum;
	uint uiVal1;
	uint uiVal2;
	uchar ucSensorID[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_SENSOR_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	ucSensorID[0] = SENSOR_WIND_SPEED_AVE_WHOLE_VS;
	ucSensorID[1] = SENSOR_WIND_SPEED_AVE_FRAC_VS;
	ucSensorID[2] = SENSOR_WIND_DIR_AVE_WHOLE_VS;
	ucSensorID[3] = SENSOR_WIND_DIR_AVE_FRAC_VS;

	while(TRUE) //lint !e774
		{
	    //vACTION_read_sensors_into_buffer(ucSensorID);

		/* SHOW THE SENSOR VALUES */
		for(ucii=0; ucii<2;  ucii++)
			{
			ucSensorNum = ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM +(6 * ucii)];
			uiVal1 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucii)],
				 	NO_NOINT
				 	);
			uiVal2 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucii)],
				 	NO_NOINT
				 	);
			vSENSOR_showSensorName(ucSensorNum, L_JUSTIFY);
			vSERIAL_sout(" = ", 3);
			vSERIAL_HBV16out(uiVal1);
			vSERIAL_bout('.');
			vSERIAL_HB16out(uiVal2);
			vSERIAL_crlf();

			}/* END: for(ucii) */

		if(ucSERIAL_kbhit()) break;

		}/* END: while() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_3() */







/**********************  ucFULLDIAG_SENSOR_4()  ****************************
*
* "SENSOR_4: Vaisala Air Pressure & Air Temperature",
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_4(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucSensorNum;
	uint uiVal1;
	uint uiVal2;
	uchar ucSensorID[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_SENSOR_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	ucSensorID[0] = SENSOR_AIR_PRESS_WHOLE_VS;
	ucSensorID[1] = SENSOR_AIR_PRESS_FRAC_VS;
	ucSensorID[2] = SENSOR_AIR_TEMP_WHOLE_VS;
	ucSensorID[3] = SENSOR_AIR_TEMP_FRAC_VS;

	while(TRUE) //lint !e774
		{
	    //vACTION_read_sensors_into_buffer(ucSensorID);

		/* SHOW THE SENSOR VALUES */
		for(ucii=0; ucii<2;  ucii++)
			{
			ucSensorNum = ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM +(6 * ucii)];
			uiVal1 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucii)],
				 	NO_NOINT
				 	);
			uiVal2 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucii)],
				 	NO_NOINT
				 	);
			vSENSOR_showSensorName(ucSensorNum, L_JUSTIFY);
			vSERIAL_sout(" = ", 3);
			vSERIAL_HBV16out(uiVal1);
			vSERIAL_bout('.');
			vSERIAL_HB16out(uiVal2);
			vSERIAL_crlf();

			}/* END: for(ucii) */

		if(ucSERIAL_kbhit()) break;

		}/* END: while() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_4() */







/**********************  ucFULLDIAG_SENSOR_5()  ****************************
*
* "SENSOR_5: Vaisala Relative Humidity & Rain Accumulation"
*
*****************************************************************************/

uchar ucFULLDIAG_SENSOR_5(
		void
		)
	{
	uchar ucRetVal;
	uchar ucii;
	uchar ucSensorNum;
	uint uiVal1;
	uint uiVal2;
	uchar ucSensorID[4];

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_SENSOR_FuncDesc[2]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	ucSensorID[0] = SENSOR_REL_HUMID_WHOLE_VS;
	ucSensorID[1] = SENSOR_REL_HUMID_FRAC_VS;
	ucSensorID[2] = SENSOR_RAIN_ACC_WHOLE_VS;
	ucSensorID[3] = SENSOR_RAIN_ACC_FRAC_VS;

	while(TRUE) //lint !e774
		{
	    //vACTION_read_sensors_into_buffer(ucSensorID);

		/* SHOW THE SENSOR VALUES */
		for(ucii=0; ucii<2;  ucii++)
			{
			ucSensorNum = ucaMSG_BUFF[OM2_IDX_DATA_0_SENSOR_NUM +(6 * ucii)];
			uiVal1 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI +(6 * ucii)],
				 	NO_NOINT
				 	);
			uiVal2 = uiMISC_buildUintFromBytes(
				 	(uchar *)&ucaMSG_BUFF[OM2_IDX_DATA_0_HI+3 +(6 * ucii)],
				 	NO_NOINT
				 	);
			vSENSOR_showSensorName(ucSensorNum, L_JUSTIFY);
			vSERIAL_sout(" = ", 3);
			vSERIAL_HBV16out(uiVal1);
			vSERIAL_bout('.');
			vSERIAL_HB16out(uiVal2);
			vSERIAL_crlf();

			}/* END: for(ucii) */

		if(ucSERIAL_kbhit()) break;

		}/* END: while() */

	return(ucRetVal);

	}/* END: ucFULLDIAG_SENSOR_5() */







/**********************  ucFULLDIAG_BUZ_0()  *********************************
*
* "BUZ_0: Let operator hear sounds
*
*****************************************************************************/

uchar ucFULLDIAG_BUZ_0(
		void
		)
	{
	uchar ucRetVal;
	uchar ucReply;

	/* SHOW THE INTRO */
	vSERIAL_printDashIntro(S_BUZ_FuncDesc[0]);

	/* ASSUME NO ERRORS */
	ucRetVal = DIAG_ACTION_KEY_NONE;

	/* DON'T ALLOW THIS DIAGNOSTIC TO RUN IF AUTOMATIC */
	if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_partially_automated_run_bit)
		{
		vSERIAL_sout(S_AutoTestExitStr.m_cText, S_AutoTestExitStr.m_uiLength);
		return(ucRetVal);
		}

	ucRetVal = DIAG_ACTION_KEY_EXIT;


Buz_0_1:
	vSERIAL_sout("SysStart\r\n", 10);
	vBUZ_tune_imperial();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_1;		//repeat

Buz_0_2:
	vSERIAL_sout("NewRdioLnk\r\n", 12);
	vBUZ_tune_TaDah_TaDah();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_2;		//repeat

Buz_0_3:
	vSERIAL_sout("BattLo\r\n", 8);
	vBUZ_tune_bad_news_1();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_3;		//repeat

Buz_0_4:
	vSERIAL_sout("NoFramFmt\r\n", 11);
	vBUZ_tune_bad_news_2();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_4;		//repeat

Buz_0_5:
	vSERIAL_sout("FramFail\r\n", 10);
	vBUZ_morrie();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_5;		//repeat

Buz_0_6:
	vSERIAL_sout("SlntRdio\r\n", 10);
	vBUZ_raspberry();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_6;		//repeat

Buz_0_7:
	vSERIAL_sout("NoRdioBrd\r\n", 11);
	vBUZ_raygun_up();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_7;		//repeat

Buz_0_8:
	vSERIAL_sout("FlshFail\r\n", 10);
	vBUZ_raygun_down();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_8;		//repeat

Buz_0_9:
	vSERIAL_sout("SD-Fail\r\n", 9);
	vBUZ_scale_down();
	ucReply = ucFULLDIAG_exit_next_repeat();
	if(ucReply == 0) goto Buz_0_x;		//exit
	if(ucReply == 2) goto Buz_0_9;		//repeat


Buz_0_x:
	return(ucRetVal);

	}/* END: ucFULLDIAG_BUZ_0() */







/**************** ucFULLDIAG_exit_next_repeat() ******************************
*
* RET: 	0 = exit
*		1 = next
*		2 = repeat
*		everything else = next
*
******************************************************************************/
uchar ucFULLDIAG_exit_next_repeat(
		void
		)
	{
	uchar ucChar;

	vSERIAL_sout("X-it,N-ext,R-epeat...", 21);
	ucChar = ucSERIAL_bin();
	vSERIAL_bout(ucChar);
	vSERIAL_crlf();
	ucChar = ucSERIAL_toupper(ucChar);
	if(ucChar == 'X') return(0);
	if(ucChar == 'Q') return(0);
	if(ucChar == 'R') return(2);

	return(1);

	}/*END: ucFULLDIAG_exit_next_repeat() */

/**************** vFULLDIAG_test_batteries() **********************************
* will get a battery voltage reading from the wisard
******************************************************************************/
void vFULLDIAG_test_batteries(void)
{
	// test batteries
	vSERIAL_sout("\r\nTesting Battery Voltage\r\n", 27);
	vTask_showBattReading();
	vSERIAL_sout("\r\nBattery Test Complete\r\n", 25);
} // end vFULLDIAG_test_batteries

/**************** vFULLDIAG_test_temp_sensor() **********************************
* will test internal temperature sensor
******************************************************************************/
void vFULLDIAG_test_temp_sensor(void){
	// get averaged mcu temp data element
	// this is asynchronous, could cause problems
	vTask_MCUTemp();
} // end vFULLDIAG_test_temp_sensor

// Report Payload structure
typedef struct
{
	uchar ucProcID; // processor ID
	ulong ulTimeStamp; // UNIX timestamp

}payloadHeader_type;

typedef struct
{
	uchar ucTransID; // transducer ID
	uchar ucReportLength; // Report Length
}payload_type;


swap_endian(uchar* pucBuffer, int ilen)
{
	unsigned char tmp;
	uchar i;

	for(i=0; i<ilen; i += 2)
	{
		tmp = pucBuffer[i];
		pucBuffer[i] = pucBuffer[i+1];
		pucBuffer[i+1] = tmp;
	}
}


/**************** vFULLDIAG_test_single_SP() ********************************
* will command sampling, request data, store DEs, and store message
******************************************************************************/
void vFULLDIAG_test_single_SP(uchar ucSPNumber){
	//uchar ucSPNumber;
	uchar ucByteCount;
	uchar ucMsgIndex;
	union SP_DataMessage S_DataMsg;

	// Build transducers 1 - 4 request data payload
	uchar ucPayload[10];
	ucPayload[0] = 1;
	ucPayload[1] = 0;
	ucPayload[2] = 2;
	ucPayload[3] = 0;
	ucPayload[4] = 3;
	ucPayload[5] = 0;
	ucPayload[6] = 4;
	ucPayload[7] = 0;

	// if CMSTM, only request transducer 1 and 2
	/*
	vSP_FetchSPName(ucSPNumber, cSPName);
	if(cSPName[0] == 0x43){
		ucPayload[0] = 1;
		ucPayload[1] = 0;
		ucPayload[2] = 2;
		ucPayload[3] = 0;
		ucPayload[4] = 0;
		ucPayload[5] = 0;
		ucPayload[6] = 0;
		ucPayload[7] = 0;
	}*/

	// Wake up the SP
	if(!ucSP_Start(ucSPNumber)){

		// Set the state of the SP board to active
		vSP_SetSPState(ucSPNumber, SP_STATE_ACTIVE);

		// give ample time for SPs to sample
		__delay_cycles(16000000);

		// Send the command
		ucSP_SendCommand(ucSPNumber, 8, ucPayload);

		// give ample time for SPs to sample
		__delay_cycles(16000000);

		// If we have data from the SP board then format it as a Data Element and store it in SRAM
		uchar ret = ucSP_RequestData(ucSPNumber);
		if (ret == 0) // Request data from the SP, if returns 0 then error
		{
			// Read the SP buffer into the the data message structure
			ucSP_GrabMessageFromBuffer(&S_DataMsg);

			// Build the message header
			vComm_DE_BuildReportHdr((ucSPNumber+1), (S_DataMsg.fields.ucMsgSize - SP_HEADERSIZE), S_DataMsg.fields.ucMsgVersion);
			ucMsgIndex = DE_IDX_RPT_PAYLOAD;

			//stuff the report into the data element
			for (ucByteCount = 0; ucByteCount < (S_DataMsg.fields.ucMsgSize - SP_HEADERSIZE); ucByteCount++)
			{
				ucaMSG_BUFF[ucMsgIndex++] = S_DataMsg.fields.ucaData[ucByteCount];
			}

			// Store DE
			vReport_LogDataElement(DEFAULTREPORTINGPRIORITY);
		}

		else{
			// comm errpor
			if(ret == 0x01)	vSERIAL_sout("\r\nComm Error\r\n", 14);
			// bad state
			else if(ret == 0x02) vSERIAL_sout("\r\nBadState\r\n", 12);
			// sp not attached
			else if(ret == 0x04) vSERIAL_sout("\r\nSP Not Attached\r\n", 19);
			else{
				vSERIAL_sout("\r\nUnknownError\r\n", 16);
				vSERIAL_HB8out(ret);
				vSERIAL_crlf();
			}
		}

		// Set SP state to inactive
		vSP_SetSPState(ucSPNumber, SP_STATE_INACTIVE);
	}

} // end vFULLDIAG_test_single_SP

/**************** vFULLDIAG_test_SP_channels() ********************************
*
******************************************************************************/
void vFULLDIAG_test_SP_channels(void)
{

	uchar ucPacketSize;

	// test all SP channels
	vSERIAL_sout("\r\nTesting SP Sensors\r\n", 22);

	// assign struct elements to buffer loacations
	//payloadHeader = (payloadHeader_type*)g_ucaSP_RXBuffer; // for header info

	// display SP info
	//vSP_Display();

	char* names[SP_NAME_LENGTH]; // declare array to hold names
	uchar* SPname; // pointer to hold a particular name
	uchar ucSPNumber;

	// loop through all 4 SP locations
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
		vSP_FetchSPName(ucSPNumber, SPname); // get the name of the SP
		names[ucSPNumber] = (char*)SPname; // place that name into the SP name array
		vSERIAL_sout(names[ucSPNumber], SP_NAME_LENGTH); // display SP name
		vSERIAL_crlf();
		uchar numTrans = ucSP_FetchNumTransducers(ucSPNumber); // display number of transducers
		vSERIAL_UI8out(numTrans);
		vSERIAL_crlf();
	}

	// Build transducers 1 - 4 request data payload
	uchar ucPayload[10];
	ucPayload[0] = 1;
	ucPayload[1] = 0;
	ucPayload[2] = 2;
	ucPayload[3] = 0;
	ucPayload[4] = 3;
	ucPayload[5] = 0;
	ucPayload[6] = 4;
	ucPayload[7] = 0;

	//while(1){ // comment this out later
	uchar i;
	for(i = 0; i < 10; i++){

	// Display SP Readiness
	if (!ucSP_Start(SP1)){
		vSERIAL_sout("\r\nSP1 Ready...\r\n", 16);
	}
	if (!ucSP_Start(SP2)){
		vSERIAL_sout("\r\nSP2 Ready...\r\n", 16);
	}
	if (!ucSP_Start(SP3)){
		vSERIAL_sout("\r\nSP3 Ready...\r\n", 16);
	}
	if (!ucSP_Start(SP4)){
		vSERIAL_sout("\r\nSP4 Ready...\r\n", 16);
	}

	// Send commands for data sampling across all transducers
	if (!ucSP_SendCommand(SP1, 8, ucPayload))
		vSERIAL_sout("Command Confirmed SP1\r\n", 23); // send confirmation
	if (!ucSP_SendCommand(SP2, 8, ucPayload))
		vSERIAL_sout("Command Confirmed SP2\r\n", 23); // send confirmation
	if (!ucSP_SendCommand(SP3, 8, ucPayload))
		vSERIAL_sout("Command Confirmed SP3\r\n", 23); // send confirmation
	if (!ucSP_SendCommand(SP4, 8, ucPayload))
		vSERIAL_sout("Command Confirmed SP4\r\n", 23); // send confirmation

	// wait for SPs to sample transducers
	__delay_cycles(16000000);

	// Request data from SP1
	if (!ucSP_RequestData(SP1))
	{
		vSERIAL_sout("Received Report SP1\r\n", 21); // confirm reception

		// get data from SP message buffer and store in data element
		uchar ucSPNumber;
		uchar ucByteCount;
		uchar ucMsgIndex;
		union SP_DataMessage S_DataMsg;

		// Read the SP buffer into the the data message structure
		ucSP_GrabMessageFromBuffer(&S_DataMsg);

		// Build the message header
		vComm_DE_BuildReportHdr(SP1, (S_DataMsg.fields.ucMsgSize - 3), S_DataMsg.fields.ucMsgVersion);
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		//stuff the report into the data element
		for (ucByteCount = 0; ucByteCount < (S_DataMsg.fields.ucMsgSize - 3); ucByteCount++)
		{
			ucaMSG_BUFF[ucMsgIndex++] = S_DataMsg.fields.ucaData[ucByteCount];
		}

		// Store DE
		vReport_LogDataElement(DEFAULTREPORTINGPRIORITY);

		// Set SP state to inactive
		vSP_SetSPState(SP1, SP_STATE_INACTIVE);
	}

	// Request data from SP2
	if (!ucSP_RequestData(SP2))
	{
		vSERIAL_sout("Received Report SP2\r\n", 21); // confirm reception
		// get data from SP message buffer and store in data element
		uchar ucByteCount;
		uchar ucMsgIndex;
		union SP_DataMessage S_DataMsg;

		ucSPNumber = 1;

		// Read the SP buffer into the the data message structure
		ucSP_GrabMessageFromBuffer(&S_DataMsg);

		// Build the message header
		vComm_DE_BuildReportHdr(SP2, (S_DataMsg.fields.ucMsgSize - 3), S_DataMsg.fields.ucMsgVersion);
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		//stuff the report into the data element
		for (ucByteCount = 0; ucByteCount < (S_DataMsg.fields.ucMsgSize - 3); ucByteCount++)
		{
			ucaMSG_BUFF[ucMsgIndex++] = S_DataMsg.fields.ucaData[ucByteCount];
		}

		// Store DE
		vReport_LogDataElement(DEFAULTREPORTINGPRIORITY);

		// Set SP state to inactive
		vSP_SetSPState(SP2, SP_STATE_INACTIVE);
	}

	// Request data from SP3
	if (!ucSP_RequestData(SP3))
	{

		// Display Report Header Info

		vSERIAL_sout("Received Report SP3\r\n", 21); // confirm reception


		// get data from SP message buffer and store in data element
		uchar ucByteCount;
		uchar ucMsgIndex;
		union SP_DataMessage S_DataMsg;

		// Read the SP buffer into the the data message structure
		ucSP_GrabMessageFromBuffer(&S_DataMsg);

		// Build the message header
		vComm_DE_BuildReportHdr(SP3, (S_DataMsg.fields.ucMsgSize - 3), S_DataMsg.fields.ucMsgVersion);
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		//stuff the report into the data element
		for (ucByteCount = 0; ucByteCount < (S_DataMsg.fields.ucMsgSize - 3); ucByteCount++)
		{
			ucaMSG_BUFF[ucMsgIndex++] = S_DataMsg.fields.ucaData[ucByteCount];
		}

		// Store DE
		vReport_LogDataElement(DEFAULTREPORTINGPRIORITY);

		// Set SP state to inactive
		vSP_SetSPState(SP3, SP_STATE_INACTIVE);
	}

	// Request data from SP4
	if (!ucSP_RequestData(SP4))
	{
		vSERIAL_sout("Received Report SP4\r\n", 21); // confirm reception

		// get data from SP message buffer and store in data element
		uchar ucByteCount;
		uchar ucMsgIndex;
		union SP_DataMessage S_DataMsg;

		// Read the SP buffer into the the data message structure
		ucSP_GrabMessageFromBuffer(&S_DataMsg);

		// Build the message header
		vComm_DE_BuildReportHdr(SP4, (S_DataMsg.fields.ucMsgSize - 3), S_DataMsg.fields.ucMsgVersion);
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		//stuff the report into the data element
		for (ucByteCount = 0; ucByteCount < (S_DataMsg.fields.ucMsgSize - 3); ucByteCount++)
		{
			ucaMSG_BUFF[ucMsgIndex++] = S_DataMsg.fields.ucaData[ucByteCount];
		}

		// Store DE
		vReport_LogDataElement(DEFAULTREPORTINGPRIORITY);

		// Set SP state to inactive
		vSP_SetSPState(SP4, SP_STATE_INACTIVE);
	}

	// Build Messages from DEs
	vReport_BuildMsgsFromDEs();

	// Store Msg to Sram
	vL2SRAM_storeMsgToSram();

	// Send Msg over serial
	uchar ucII;
	uint uiMySN;

	if (ucL2SRAM_getCopyOfCurMsg()) // if there is a message
	{
		ucPacketSize = NET_HDR_SZ + MSG_HDR_SZ + CRC_SZ;
		vComm_NetPkg_buildHdr(0xFEFE);
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND,ucaMSG_BUFF,ucPacketSize);
		for (ucII = 0; ucII < ucaMSG_BUFF[MSG_IDX_LEN] + 1; ucII++)
		{
			vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
		}
		vSERIAL_crlf();
	}
	else //no messages in SRAM send empty packet
	{
		vSERIAL_sout("NoNewData", 9);
		vSERIAL_crlf();
	}

	// Delete the transmitted message
	vL2SRAM_delCurMsg();

	}

	//ucFULLDIAG_SENSOR_1();
}

/**************** vFULLDIAG_test_radio_discovery() **************************************
* tests the radio in opmode
******************************************************************************/
void vFULLDIAG_test_radio_discovery(void){

	//Set the channel
	unADF7020_SetChannel(DISCOVERY_CHANNEL);

	//Power up and initialize the radio
	vADF7020_WakeUp();

	// Set state to TX mode
	vADF7020_TXRXSwitch(RADIO_TX_MODE);

	uchar kHit = 0; // set state bit

	vSERIAL_sout("\r\nBroadcasting in discovery mode. Press any key to stop transmitting\r\n", 70);
	while (kHit == 0) // while no keypresses, run the radio test
	{
		ucSERIAL_bin();
		kHit = 1;
	} //end while

	//Shutdown the radio
	vADF7020_Quit();

} // end vFULLDIAG_test_radio_discovery

/**************** vFULLDIAG_test_radio_opmode() **************************************
* tests the radio in discovery mode
******************************************************************************/
void vFULLDIAG_test_radio_opmode(){
	//Set the channel
		unADF7020_SetChannel(TEST_CHANNEL);

		//Power up and initialize the radio
		vADF7020_WakeUp();

	vADF7020_TXRXSwitch(RADIO_TX_MODE);

	uchar kHit = 0; // set state bit

	vSERIAL_sout("\r\nBroadcasting in opmode. Press any key to stop transmitting\r\n", 62);

	while (kHit == 0) // while no keypresses, run the radio test
	{
		ucSERIAL_bin();
		kHit = 1;
	}

	//Shutdown the radio
	vADF7020_Quit();

} // end vFULLDIAG_test_radio_opmode


/**************** vFULLDIAG_test_actuators() **********************************
*
******************************************************************************/
void vFULLDIAG_test_actuators(uchar ucSPNumber){
	// Build transducers 1 - 4 request data payload

	// payload loaded with off commands
	uchar ucPayload_off[10];
	ucPayload_off[0] = 3;
	ucPayload_off[1] = 2;
	ucPayload_off[2] = 0;
	ucPayload_off[3] = 0;
	ucPayload_off[4] = 0;
	ucPayload_off[5] = 0;
	ucPayload_off[6] = 0;
	ucPayload_off[7] = 0;

	// payload loaded with on commands
	uchar ucPayload_on[10];
	ucPayload_on[0] = 3;
	ucPayload_on[1] = 2;
	ucPayload_on[2] = 0x5B;
    ucPayload_on[3] = 0;
    ucPayload_on[4] = 0;
	ucPayload_on[5] = 0;
	ucPayload_on[6] = 0;
	ucPayload_on[7] = 0;

	// if sp is in the slot passed, send command
	if(ucSP_IsAttached(ucSPNumber)){
		ucSP_Start(ucSPNumber); // start SP
		// Set the state of the SP board to active
		vSP_SetSPState(ucSPNumber, SP_STATE_ACTIVE);
		ucSP_SendCommand(ucSPNumber, 8, ucPayload_off); // send command to SP
		__delay_cycles(32000000);
		ucSP_SendCommand(ucSPNumber, 8, ucPayload_on); // send command to SP
		__delay_cycles(32000000);
		ucSP_SendCommand(ucSPNumber, 8, ucPayload_off); // send command to SP
		__delay_cycles(32000000);
		ucSP_SendCommand(ucSPNumber, 8, ucPayload_on); // send command to SP
		// Set the state of the SP board to active
		vSP_SetSPState(ucSPNumber, SP_STATE_INACTIVE);

	} // end if
	else{
		vSERIAL_sout("No SP attached in this slot\r\n", 29);
	} // end else

} // end vFULLDIAG_test_actuators

/**************** vFULLDIAG_test_buzzer() *************************************
* This function will test the buzzer on the CP board
******************************************************************************/
void vFULLDIAG_test_buzzer(void){
	// play buzzer noise
	vBUZ_test_successful();

} // end vFULLDIAG_test_buzzer

/**************** vFULLDIAG_test_SDCard() *************************************
* This function will test the SD card on the CP board
******************************************************************************/
void vFULLDIAG_test_SDCard(void){
	// read/write to/from device

} // end vFULLDIAG_teset_SDCard

/**************** vFULLDIAG_test_LED() ****************************************
* This function will test the buzzer on the CP board
******************************************************************************/
void vFULLDIAG_test_LED(void){
	// test led
	uchar ucCount;
	for(ucCount = 0; ucCount < 10; ucCount++){
		vLED_RedBlink(1000);
		vLED_GrnBlink(1000);
	} // end for

} // end vFULLDIAG_test_LED

/**************** vFULLDIAG_display_diag_status() ****************************************
* This function will display diagnostic status information
*****************************************************************************************/
void vFULLDIAG_display_diag_status(){
	/*
	// Display diagnostic test status
	vSERIAL_sout("\r\n---------------------------------------------------------\r\n", 61);
	vSERIAL_sout("Diagnostic Status: \r\n", 21);
	switch(Global_DUR_count){
		case DUR_STATE_START:
			vSERIAL_sout("State: DUR_STATE_START\r\n", 24);
			vSERIAL_sout("Description: Scheduling SP reporting\r\n", 38);
			break;
		case DUR_STATE_CMD:
			vSERIAL_sout("State: DUR_STATE_CMD\r\n", 22);
			vSERIAL_sout("Description: Performing Sampling\r\n", 34);
			break;
		case DUR_STATE_RQST:
			vSERIAL_sout("State: DUR_STATE_RQST\r\n", 23);
			vSERIAL_sout("Description: Requesting Data from Satellite Boards\r\n", 52);
			break;
		case DUR_STATE_RPT:
			vSERIAL_sout("State: DUR_STATE_RPT\r\n", 22);
			vSERIAL_sout("Description: Gathering Reports\r\n", 32);
			break;
		case DUR_STATE_IDLE:
			vSERIAL_sout("State: DUR_STATE_IDLE\r\n", 23);
			vSERIAL_sout("Description: Diagnostic Task is Currently Inactive\r\n", 52);
			break;
		default:
			vSERIAL_sout("State: DUR_STATE_IDLE\r\n", 23);
			vSERIAL_sout("Description: Diagnostic Task is Currently Inactive\r\n", 52);
			break;
	} // end switch

	//vSERIAL_UI8out(Global_DUR_count);
	vSERIAL_sout("\r\n---------------------------------------------------------\r\n", 61);
	*/
}


/**************** vFULLDIAG_report_DUR() ****************************************
* This function will report back to the diagnostic utility
********************************************************************************/
void vFULLDIAG_report_DUR(){
	/*
	uchar ucII;
	uchar ucAckRet;
	uint uiNodeID;
	uint uiNextHopID;
	S_Cmd S_CommandData;
	uchar ucDELength;
	uchar ucDEVersion;
	uchar ucProcID;
	uchar ucTransID;
	uchar ucDEID;
	uchar ucCount, ucCmdParamCount;
	uint uiMySN;
	uchar ucMsgIndex;
	uchar ucSPNumber; // count for each SP
	uchar ucPacketSize;

	// Build transducers 1 - 4 request data payload
	uchar ucPayload[10];
	ucPayload[0] = 1;
	ucPayload[1] = 0;
	ucPayload[2] = 2;
	ucPayload[3] = 0;
	ucPayload[4] = 3;
	ucPayload[5] = 0;
	ucPayload[6] = 4;
	ucPayload[7] = 0;

	// print status to the screen
	vFULLDIAG_display_diag_status();

	// perform the appropriate actions for whichever state the task is in
	switch(Global_DUR_count){

		/////////////////////////////
		// START STATE
		/////////////////////////////
		case DUR_STATE_START: // diagnostics are in starting state
			Global_DUR_count = DUR_STATE_CMD; // transition to cmd state
			break;

		/////////////////////////////
		// COMMAND STATE
		/////////////////////////////
		case DUR_STATE_CMD: // diagnostics are in cmd state

			// run SP test for each SP board
			for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
				// if sp is attached...
				if(ucSP_IsAttached(ucSPNumber)){
					ucSP_Start(ucSPNumber); // start SP
					ucSP_SendCommand(ucSPNumber, 8, ucPayload); // send command to SP
				} // end if
				else{
					//vSERIAL_sout("No SP in slot\r\n", 15);
				} // end else
			} // end for

			Global_DUR_count = DUR_STATE_RQST; // transition to request data state
			break;

		/////////////////////////////
		// REQUEST STATE
		/////////////////////////////
		case DUR_STATE_RQST: // request data state
			//ucSP_RequestData(ucSPNumber); 	// Request data from SP

			//uchar ucSPNumber; // count for each SP

			// run SP test for each SP board
			for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){

				// get data from SP message buffer and store in data element

				//uchar ucSPNumber;
				uchar ucByteCount;
				uchar ucMsgIndex;
				union SP_DataMessage S_DataMsg;

				// ensure message is clear
				//memset(&S_DataMsg,0,sizeof(S_DataMsg));

				// Read the SP buffer into the the data message structure
				ucSP_GrabMessageFromBuffer(&S_DataMsg);

				// Build the message header
				vComm_DE_BuildReportHdr((ucSPNumber+1), (S_DataMsg.fields.ucMsgSize - 3), S_DataMsg.fields.ucMsgVersion);
				ucMsgIndex = DE_IDX_RPT_PAYLOAD;

				//stuff the report into the data element
				for (ucByteCount = 0; ucByteCount < (S_DataMsg.fields.ucMsgSize - 3); ucByteCount++)
				{
					ucaMSG_BUFF[ucMsgIndex++] = S_DataMsg.fields.ucaData[ucByteCount];
				}

				// Store DE
				vReport_LogDataElement(DEFAULTREPORTINGPRIORITY);

				// Set SP state to inactive
				vSP_SetSPState(ucSPNumber, SP_STATE_INACTIVE);

				// Build Messages from DEs
				vReport_BuildMsgsFromDEs();

				// Store Msg to Sram
				vL2SRAM_storeMsgToSram();

				// Once the DE is written to the MSG_BUFF then remove it from RAM
				vReport_RemoveDEFromRAM();

			} // end for

			Global_DUR_count = DUR_STATE_RPT; // transition to report data state
			break;

		/////////////////////////////
		// REPORT STATE
		/////////////////////////////
		case DUR_STATE_RPT: // report to diagnostic utility

			// garden server com is now active over usci, keyboard commands disabled
			ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 1;

			uiMySN = uiL2FRAM_getSnumLo16AsUint();

			while (ucTimeCheckForAlarms(SUBSLOT_ALARMS) == 0)
			{
				if (ucL2SRAM_getCopyOfCurMsg()) // if there is a message
				{
					vComm_NetPkg_buildHdr(0xDEDE); // denotes diagnostic utility destination
					// Get the size of the packet
					ucPacketSize = NET_HDR_SZ + MSG_HDR_SZ + CRC_SZ;
					ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucPacketSize);
					for (ucII = 0; ucII < ucaMSG_BUFF[MSG_IDX_LEN] + 1; ucII++)
					{
						vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
					} // end for
					vSERIAL_crlf();
				} // end if

				else //no messages in SRAM send empty packet
				{
					/*
					vComm_Msg_buildOperational((MSG_FLG_SINGLE | MSG_FLG_ACKRQST), 1, uiMySN, MSG_ID_OPERATIONAL);
					vComm_NetPkg_buildHdr(0xDEDE);
					ucaMSG_BUFF[MSG_IDX_LEN] = 12;
					ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);

					for (ucII = 0; ucII < ucaMSG_BUFF[MSG_IDX_LEN] + 1; ucII++)
					{
						vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
					} // end for
					vSERIAL_crlf();

				} // end else

				// Delete the transmitted message
				vL2SRAM_delCurMsg();
			} // end while

			// garden server com is no longer active, clear flag so keyboard commands can be used again
			ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 0;

			// set task to idle so that it won't be scheduled again
			//vTask_SetState(ucTask_FetchTaskIndex(TASK_ID_DUR), TASK_STATE_IDLE);
			ucTask_SetField(ucTask_FetchTaskIndex(TASK_ID_DUR), TSK_STATE, (ulong) TASK_STATE_IDLE);
			Global_DUR_count = DUR_STATE_IDLE; // transition to idle state
			break;

		/////////////////////////////
		// IDLE STATE
		/////////////////////////////
		case DUR_STATE_IDLE:
			// essentially this task will do nothing at the moment
			// ToDo: Ensure the task no longer gets scheduled pased this state
			break;

		/////////////////////////////
		// DEFAULT STATE
		/////////////////////////////
		default:
			// Basically just the idle state
			break;
	} // end switch





		// Wait for the response
		// ucAckRet = ucSerial_Wait_for_Ack();

		/*
		switch (ucAckRet)
		{
			case TIMEOUT: // No response from GS
			break;

			case ACK:
				vL2SRAM_delCurMsg();
			break;

			case NACK: // Nack, so do not delete the message and retry
			break;

			case CMD: // Command
				vSERIAL_ReadBuffer(ucaMSG_BUFF);

				// GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN
				if ((ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
				CHKBIT_CRC + CHKBIT_MSG_TYPE, //chk flags
					CHKBIT_CRC + CHKBIT_MSG_TYPE, //report flags
					MSG_ID_OPERATIONAL, //msg type
					0, //src SN
					0 //Dst SN
					)))
				{
					// If we are here then the message failed the integrity check
					// Send a nack message
					vGS_SendNack();
				}
				else // The command is good
				{
					uiNodeID = (ucaMSG_BUFF[MSG_IDX_ADDR_HI] << 8) | ucaMSG_BUFF[MSG_IDX_ADDR_LO];

					// If the command is for the hub
					if(uiNodeID == uiMySN)
					{
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
								ucTransID = ucaMSG_BUFF[ucCount++];
								S_CommandData.m_uiTaskId = (uint)((ucProcID<<8) | ucTransID);
								S_CommandData.m_ucCmdLength = ucDELength - 5;

								// Get the command parameters from the message
								for (ucCmdParamCount = 0; ucCmdParamCount < S_CommandData.m_ucCmdLength; ucCmdParamCount++)
								{
									S_CommandData.m_ucCmdParam[ucCmdParamCount] = ucaMSG_BUFF[ucCount++];
								}

								// Set up the command
								ucComm_Handle_Command(S_CommandData);
							break;

							case PROGRAM_CODE:
								// hmm...
							break;

							case REQUEST_DATA:
							break;

						} // END switch (DEID)
					}
					// Otherwise the command is for a descendant node
					else
					{
					uiNextHopID = uiRoute_GetNextHop(uiNodeID);

					// If the node is not in the edge list then generate a report stating the destination was not reached
					if (uiNextHopID == 0)
					{
						// If the report is destined for a node that is not in the network then generate the report DE and store it

						// Build the DE header
						vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
						ucMsgIndex = DE_IDX_RPT_PAYLOAD;

						// Load the rest of the DE
						ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_ROUTING_FAIL; // Src ID
						ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
						ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiNodeID>> 8);
						ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiNodeID;

						// Store DE
						vReport_LogDataElement(DEFAULTREPORTINGPRIORITY);
						}
						else
						{
							// Store the command in the queue
							ucL2SRAM_PutCMD_inQueue(uiNextHopID);
						}
					}
					// Delete the transmitted message
					vL2SRAM_delCurMsg();
				}
			break;

			default:
			break;
		}

	}*/
} // end vFULLDIAG_report_DUR

/**************** getResponse() *************************************************************
* captures an input from the user
*********************************************************************************************/
char getResponse(void){
	char input;
	scanf("%c", &input);
	return input;
} // end getResponse


/**************** vFULLDIAG__diagnostic_asynchronous_operations() ***************************
* runs through all of the diagnostic tests that don't require tasks
*********************************************************************************************/
void vFULLDIAG_diagnostic_asynchronous_operations(void){
	char cKey;
	uchar ucSPNumber;
	uchar ucPacketSize;
	results rpt;
	uchar ucDiagMode; // Deployment mode: 0x00, Field mode: 0x01
	char* tempName;

	// ensure no errroneous data in struct
	rpt.LED_test = 0x00;
	rpt.radio_disc_test = 0x00;
	rpt.radio_op_test = 0x00;
	rpt.buzzer_test = 0x00;
	rpt.valve_test = 0x00;
	rpt.SD_test = 0x00;
	rpt.FRAM_test = 0x00;

	// initialize all sensor types to 0
	unsigned int j;
	for(j=0; j<4; j++){
		rpt.SP_slot1_types[j] = 0;
		rpt.SP_slot2_types[j] = 0;
		rpt.SP_slot3_types[j] = 0;
		rpt.SP_slot4_types[j] = 0;
	}

	// TEST if requesting sensor types will actually work
	uint8 SP1_types[4], SP2_types[4], SP3_types[4], SP4_types[4];

	for(j=0; j<4; j++){
		SP1_types[j] = 0;
		SP2_types[j] = 0;
		SP3_types[j] = 0;
		SP4_types[j] = 0;
	}

	// loop through and wake up SPs
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
		// Wake up the SP
		if(!ucSP_Start(ucSPNumber)){
			// Set the state of the SP board to active
			vSP_SetSPState(ucSPNumber, SP_STATE_ACTIVE);
		}
	}

	// give ample time for SPs to awake
	__delay_cycles(16000000);

	// fetch names and place into report
	uchar* ucSPName;
	vSP_FetchSPName(0x00, rpt.SP_slot1);
	vSP_FetchSPName(0x01, rpt.SP_slot2);
	vSP_FetchSPName(0x02, rpt.SP_slot3);
	vSP_FetchSPName(0x03, rpt.SP_slot4);

	// give ample time for SPs to awake
	__delay_cycles(16000000);

	// loop through SPs, get their types, and populate arrays
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
		switch(ucSPNumber){
			case 0:
				if( rpt.SP_slot1[0] == 'S' && rpt.SP_slot1[1] == 'T' && rpt.SP_slot1[2] == 'M' )
				{
					vSP_Command_SensorTypes(ucSPNumber);
					__delay_cycles(16000000);
					ucSP_Request_SensorTypes(ucSPNumber, SP1_types);
				}
				else if( rpt.SP_slot1[0] == 'C' && rpt.SP_slot1[1] == 'M' && rpt.SP_slot1[2] == 'S' && rpt.SP_slot1[3] == 'T' && rpt.SP_slot1[4] == 'M')
				{
					vSP_Command_SensorTypes(ucSPNumber);
					__delay_cycles(16000000);
					ucSP_Request_SensorTypes(ucSPNumber, SP1_types);
				}
				else{
					SP1_types[0] = 0;
					SP1_types[1] = 0;
					SP1_types[2] = 0;
					SP1_types[3] = 0;
				}
				break;
			case 1:
				if( rpt.SP_slot2[0] == 'S' && rpt.SP_slot2[1] == 'T' && rpt.SP_slot2[2] == 'M' )
				{
					vSP_Command_SensorTypes(ucSPNumber);
					__delay_cycles(16000000);
					ucSP_Request_SensorTypes(ucSPNumber, SP2_types);
				}
				else if( rpt.SP_slot2[0] == 'C' && rpt.SP_slot2[1] == 'M' && rpt.SP_slot2[2] == 'S' && rpt.SP_slot2[3] == 'T' && rpt.SP_slot2[4] == 'M')
				{
					vSP_Command_SensorTypes(ucSPNumber);
					__delay_cycles(16000000);
					ucSP_Request_SensorTypes(ucSPNumber, SP2_types);
				}
				else{
					SP2_types[0] = 0;
					SP2_types[1] = 0;
					SP2_types[2] = 0;
					SP2_types[3] = 0;
				}
				break;
			case 2:
				if( rpt.SP_slot3[0] == 'S' && rpt.SP_slot3[1] == 'T' && rpt.SP_slot3[2] == 'M' )
				{
					vSP_Command_SensorTypes(ucSPNumber);
					__delay_cycles(16000000);
					ucSP_Request_SensorTypes(ucSPNumber, SP3_types);
				}
				else if( rpt.SP_slot3[0] == 'C' && rpt.SP_slot3[1] == 'M' && rpt.SP_slot3[2] == 'S' && rpt.SP_slot3[3] == 'T' && rpt.SP_slot3[4] == 'M')
				{
					vSP_Command_SensorTypes(ucSPNumber);
					__delay_cycles(16000000);
					ucSP_Request_SensorTypes(ucSPNumber, SP3_types);
				}
				else{
					SP3_types[0] = 0;
					SP3_types[1] = 0;
					SP3_types[2] = 0;
					SP3_types[3] = 0;
				}
				break;
			case 3:
				if( rpt.SP_slot4[0] == 'S' && rpt.SP_slot4[1] == 'T' && rpt.SP_slot4[2] == 'M' )
				{
					vSP_Command_SensorTypes(ucSPNumber);
					__delay_cycles(16000000);
					ucSP_Request_SensorTypes(ucSPNumber, SP4_types);
				}
				else if( rpt.SP_slot4[0] == 'C' && rpt.SP_slot4[1] == 'M' && rpt.SP_slot4[2] == 'S' && rpt.SP_slot4[3] == 'T' && rpt.SP_slot4[4] == 'M')
				{
					vSP_Command_SensorTypes(ucSPNumber);
					__delay_cycles(16000000);
					ucSP_Request_SensorTypes(ucSPNumber, SP4_types);
				}
				else{
					SP4_types[0] = 0;
					SP4_types[1] = 0;
					SP4_types[2] = 0;
					SP4_types[3] = 0;
				}
				break;

			default:
				break;
		} // end switch(ucSPNumber)
	} // end for(SPs)

	vSERIAL_sout("\r\nTransducer types\r\n", 20);

	// display SP1 types
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
		vSERIAL_HB8out(SP1_types[ucSPNumber]);
		vSERIAL_crlf();
	}

	// display SP2 types
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
		vSERIAL_HB8out(SP2_types[ucSPNumber]);
		vSERIAL_crlf();
	}

	// display SP3 types
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
		vSERIAL_HB8out(SP3_types[ucSPNumber]);
		vSERIAL_crlf();
	}

	// display SP4 types
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
		vSERIAL_HB8out(SP4_types[ucSPNumber]);
		vSERIAL_crlf();
	}


	// prompt user for diagnostic mode
	vSERIAL_sout("\r\nDeployment or Field Diagnostic? (1 - for deployment, 2 - for field)\r\n", 71);
	while(1){
		vSERIAL_sout("\r\n>", 3);
		cKey = ucSERIAL_bin();
		vSERIAL_bout(cKey);
		if(cKey == '1'){
			ucDiagMode = 0x00;
			break;
		}
		else if(cKey == '2'){
			ucDiagMode = 0x01;
			break;
		}
		else
			continue;
	}

	// assign mode to report struct
	rpt.diagMode = ucDiagMode;

	// Clean out the message buffer
	vSERIAL_CleanBuffer((uchar *)ucaMSG_BUFF);

	//////////////////////
	// Batt Voltage
	//////////////////////
	vSERIAL_sout("\r\nCalculating Battery Voltage...\r\n", 34);
	vTask_Batt_Sense(); // do a battery reading

	//////////////////////
	// MCU Temp
	//////////////////////
	vSERIAL_sout("\r\nCalculating MCU Temperature...\r\n", 34);
	vTask_MCUTemp(); // do a temperature reading

	// send all messages
	uchar ucCount;
	uint uiMySN;
	uchar ucII;
	uiMySN = uiL2FRAM_getSnumLo16AsUint();

	for(ucCount=0; ucCount < 20; ucCount++)
	{
		if (ucL2SRAM_getCopyOfCurMsg()) // if there is a message
		{
			vComm_NetPkg_buildHdr(0xDEDE);
			// Get the size of the packet
			ucPacketSize = NET_HDR_SZ + ucaMSG_BUFF[MSG_IDX_LEN] + CRC_SZ;
			ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucPacketSize);
			for (ucII = 0; ucII < ucPacketSize; ucII++)
			{
				vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
			}
			vSERIAL_crlf();
			vL2SRAM_delCurMsg();
		}
		else //no messages in SRAM
		{
			// do nothing
		}
	}

	//////////////////////
	// TEST SENSORS
	//////////////////////
	vSERIAL_sout("\r\nTesting Sensors...\r\n", 22);
	uchar ucTimes;

	// loop through each SP, send commands, get reports
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){

		// generalize names for easy iteration
		if(ucSPNumber == 0)
			tempName = rpt.SP_slot1;
		else if(ucSPNumber == 1)
			tempName = rpt.SP_slot2;
		else if(ucSPNumber == 2)
			tempName = rpt.SP_slot3;
		else if(ucSPNumber == 3)
			tempName = rpt.SP_slot4;

		// if no SP attached in slot, go to next slot
		if (!ucSP_IsAttached(ucSPNumber))
			continue;

		// display name of SP to test
		vSERIAL_sout("\r\nTesting SP", 12);
		vSERIAL_HB8out(ucSPNumber + 1);
		vSERIAL_crlf(); // new line

		// perform context sensitive tests on the STM sensors
		if( tempName[0] == 'S' && tempName[1] == 'T' && tempName[2] == 'M' )
		{
			vSERIAL_sout("\r\nPlease place all sensors for this SP in water\r\nPress any key to proceed...\r\n", 80);
			cKey = ucSERIAL_bin(); // get keypress from user
			vSERIAL_crlf(); // new line
			vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors

			vSERIAL_sout("\r\nNow, remove the sensors from water and place in hand\r\nPress any key to proceed...\r\n", 87);
			cKey = ucSERIAL_bin(); // get keypress from user
			vSERIAL_crlf(); // new line
			vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors
		}

		// perform context sensitive tests on the CMSTM sensors
		else if( tempName[0] == 'C' && tempName[1] == 'M' && tempName[2] == 'S' && tempName[3] == 'T' && tempName[4] == 'M')
		{
			vSERIAL_sout("\r\nPlease place all sensors (NOT VALVES) for this SP in water\r\nPress any key to proceed...\r\n", 93);
			cKey = ucSERIAL_bin(); // get keypress from user
			vSERIAL_crlf(); // new line
			vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors

			vSERIAL_sout("\r\nNow, remove the sensors from water and place in hand\r\nPress any key to proceed...\r\n", 87);
			cKey = ucSERIAL_bin(); // get keypress from user
			vSERIAL_crlf(); // new line
			vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors
		}

		// perform context sensitive tests on the ST sensors
		else if( tempName[0] == 'S' && tempName[1] == 'T')
		{
			vSERIAL_sout("\r\nPlease place all sensors in open air\r\nPress any key to proceed...\r\n", 71);
			cKey = ucSERIAL_bin(); // get keypress from user
			vSERIAL_crlf(); // new line
			vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors

			vSERIAL_sout("\r\nNow, hold the ends of each sensor in your hands\r\nPress any key to proceed...\r\n", 82);
			cKey = ucSERIAL_bin(); // get keypress from user
			vSERIAL_crlf(); // new line
			vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors
		}

		// Build Messages from DEs
		vReport_BuildMsgsFromDEs();

	} // end for each SP

	for(ucCount=0; ucCount < 20; ucCount++)
	{
		if (ucL2SRAM_getCopyOfCurMsg()) // if there is a message
		{
			// Designate packet as a diag packet
			vComm_NetPkg_buildHdr(0xDEDE);

			// Get the size of the packet
			ucPacketSize = NET_HDR_SZ + ucaMSG_BUFF[MSG_IDX_LEN] + CRC_SZ;
			ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucPacketSize);
			for (ucII = 0; ucII < ucPacketSize; ucII++)
			{
				vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
			}
			vSERIAL_crlf();
			vL2SRAM_delCurMsg();
		}
		else //no messages in SRAM
		{
			// nothing
		}
	}

	//////////////////////
	// SP-ST Calibration
	//////////////////////
	vSERIAL_sout("\r\nChecking if any calibration necessary...\r\n", 44);
	//uchar ucTimes;

	// loop through each SP, send commands, get reports
	for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){

		// generalize names for easy iteration
		if(ucSPNumber == 0)
			tempName = rpt.SP_slot1;
		else if(ucSPNumber == 1)
			tempName = rpt.SP_slot2;
		else if(ucSPNumber == 2)
			tempName = rpt.SP_slot3;
		else if(ucSPNumber == 3)
			tempName = rpt.SP_slot4;

		// if no SP attached in slot, go to next slot
		if (!ucSP_IsAttached(ucSPNumber))
			continue;

		// perform context sensitive tests on the STM sensors
		if( tempName[0] == 'S' && tempName[1] == 'T' && tempName[2] == 'M' )
		{
			// don't need to do any calibration for STMs
			continue;
		}

		// perform context sensitive tests on the CMSTM sensors
		else if( tempName[0] == 'C' && tempName[1] == 'M' && tempName[2] == 'S' && tempName[3] == 'T' && tempName[4] == 'M')
		{
			continue;
		}

		// perform context sensitive tests on the ST sensors
		else if( tempName[0] == 'S' && tempName[1] == 'T')
		{
			// display name of SP to test
			vSERIAL_sout("\r\nGathering calibration info for SP", 32);
			vSERIAL_HB8out(ucSPNumber + 1);
			vSERIAL_crlf(); // new line

			vSERIAL_sout("\r\nPlease plug the fluke into channel 1 and source 0C\r\nPress any key to proceed...\r\n", 85);
			cKey = ucSERIAL_bin(); // get keypress from user
			vSERIAL_crlf(); // new line

			// take 10 samples
			int tempLoopCounter;
			for(tempLoopCounter = 0; tempLoopCounter < 5; tempLoopCounter++){
				vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors
			}
			// Build Messages from DEs
			vReport_BuildMsgsFromDEs();

			for(tempLoopCounter = 0; tempLoopCounter < 5; tempLoopCounter++){
				vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors
			}
			// Build Messages from DEs
			vReport_BuildMsgsFromDEs();

			vSERIAL_sout("\r\nNow, source 25C from the same channel\r\nPress any key to proceed...\r\n", 70);
			cKey = ucSERIAL_bin(); // get keypress from user
			vSERIAL_crlf(); // new line

			// take 10 more samples
			for(tempLoopCounter = 0; tempLoopCounter < 5; tempLoopCounter++){
				vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors
			}
			// Build Messages from DEs
			vReport_BuildMsgsFromDEs();

			// take 10 more samples
			for(tempLoopCounter = 0; tempLoopCounter < 5; tempLoopCounter++){
				vFULLDIAG_test_single_SP(ucSPNumber); // sample sensors
			}
			// Build Messages from DEs
			vReport_BuildMsgsFromDEs();


			//while (ucTimeCheckForAlarms(SUBSLOT_ALARMS) == 0)
			for(ucCount=0; ucCount <= 20; ucCount++)
			{
				if (ucL2SRAM_getCopyOfCurMsg()) // if there is a message
				{
					vComm_NetPkg_buildHdr(0xDEDE);
					// Get the size of the packet
					//ucPacketSize = NET_HDR_SZ + MSG_HDR_SZ + CRC_SZ;
					// Get the size of the packet
					ucPacketSize = NET_HDR_SZ + ucaMSG_BUFF[MSG_IDX_LEN] + CRC_SZ;
					ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucPacketSize);
					for (ucII = 0; ucII < ucPacketSize; ucII++)
					{
						vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
					}
					vSERIAL_crlf();
					vL2SRAM_delCurMsg();
				}
				else //no messages in SRAM
				{
					// nothing
				}
			}
		}

	} // end for each SP


	//////////////////////
	// TEST CP LEDs
	//////////////////////
	vSERIAL_sout("\r\n Testing CP LEDs...\r\n", 23);
	vSERIAL_sout("Do you see red and green flashing? (press y or n, or r to repeat test)\r\n", 72);
	while(1){
		vFULLDIAG_test_LED(); // run the LED test
		vSERIAL_sout("\r\n>", 3);
		cKey = ucSERIAL_bin();
		vSERIAL_bout(cKey);

		if(cKey == 'y' || cKey == 'Y'){
			rpt.LED_test = DIAG_TEST_PASS;
			break;
		}

		else if(cKey == 'n' || cKey == 'N'){
			rpt.LED_test = DIAG_TEST_FAIL;
			break;
		}
		else if(cKey != 'r' && cKey != 'R') // allow for tests to be halted
			return;
	}// end while

	//////////////////////
	// TEST Radio Discovery
	//////////////////////
	vSERIAL_sout("\r\n\r\nPlease plug in spectrum analyzer to test\r\n", 46);
		while(1){
			vFULLDIAG_test_radio_discovery(); // run the radio in discovery mode
			vSERIAL_sout("\r\nDid the radio transmit at 10dBm or greater? (press y or n, or r to repeat test)\r\n", 83);
					vSERIAL_sout("\r\n>", 3);
					cKey = ucSERIAL_bin();
					vSERIAL_bout(cKey);

				if(cKey == 'y' || cKey == 'Y'){
					rpt.radio_disc_test = DIAG_TEST_PASS;
					break;
				}

				else if(cKey == 'n' || cKey == 'N'){
					rpt.radio_disc_test = DIAG_TEST_FAIL;
					break;
				}
				else if(cKey != 'r' && cKey != 'R') // allow for tests to be halted
					return;
		}// end while(1)

	//////////////////////
	// TEST Radio OpMode
	//////////////////////
		vSERIAL_sout("\r\n\r\nPlease plug in spectrum analyzer to test\r\n", 46);
			while(1){
				vFULLDIAG_test_radio_opmode(); // run the radio in opmode
				vSERIAL_sout("\r\nDid the radio transmit at 10dBm or greater? (press y or n, or r to repeat test)\r\n", 83);
						vSERIAL_sout("\r\n>", 3);
						cKey = ucSERIAL_bin();
						vSERIAL_bout(cKey);

					if(cKey == 'y' || cKey == 'Y'){
						rpt.radio_op_test = DIAG_TEST_PASS;
						break;
					}

					else if(cKey == 'n' || cKey == 'N'){
						rpt.radio_op_test = DIAG_TEST_FAIL;
						break;
					}
					else if(cKey != 'r' && cKey != 'R') // allow for tests to be halted
						return;
			}

	//////////////////////
	// TEST Buzzer
	//////////////////////
	vSERIAL_sout("\r\nPlease listen for the buzzer\r\n", 32);
	__delay_cycles(16000000);
	while(1){
		vFULLDIAG_test_buzzer(); // run the buzzer test
		vSERIAL_sout("\r\nDid you hear the buzzer? (press y or n, or r to repeat test)\r\n", 64);

		vSERIAL_sout("\r\n>", 3);
		cKey = ucSERIAL_bin();
		vSERIAL_bout(cKey);

		//cKey = getResponse(); // capture response from user
		if(cKey == 'y' || cKey == 'Y'){
			rpt.buzzer_test = DIAG_TEST_PASS;
			break;
		}

		else if(cKey == 'n' || cKey == 'N'){
			rpt.buzzer_test = DIAG_TEST_FAIL;
			break;
		}
		else if(cKey != 'r' && cKey != 'R') // allow for tests to be halted
			return;
	} // end while

	//////////////////////
	// TEST Valves
	//////////////////////
	uchar errCount = 0;

		//vFULLDIAG_test_actuators(uchar ucSPNumber)
		for(ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++){
			uchar cSPName[5];
			vSP_FetchSPName(ucSPNumber, cSPName);
			// if name of current SP is CM-STM, do a valve test
			if(cSPName[0] == 0x43){
			//if(vSP_FetchSPName(ucSPNumber, cSPName) == "CMSTM"){
				vSERIAL_sout("\r\n\r\nTesting valve on SP", 19);
				vSERIAL_UI8out(ucSPNumber + 1);
				vSERIAL_sout("...\r\n\r\nPlease listen for the valve\r\n", 36);
				__delay_cycles(16000000); // delay to give users adequate time to read
				//while(1){
					vFULLDIAG_test_actuators(ucSPNumber); // run the valve test
					vSERIAL_sout("\r\nDid the valve open and close? (press y or n, or r to repeat test)\r\n", 69);
					//cKey = getResponse(); // capture response from user
					//if(ucSERIAL_kbhit()){
						cKey = ucSERIAL_bin();
						vSERIAL_bout(cKey);
						//break;
					//}
					if(cKey == 'y' || cKey == 'Y'){
						//rpt.valve_test = DIAG_TEST_PASS;
						//break;
						continue;
					}

					else if(cKey == 'n' || cKey == 'N'){
						//rpt.valve_test = DIAG_TEST_FAIL;
						//break;
						errCount++;
					}
					else if(cKey != 'r' && cKey != 'R') // allow for tests to be halted
						return;
				//} // end while
			}
			else
				continue;
		} // end for

		if(errCount == 0)
			rpt.valve_test = DIAG_TEST_PASS;
		else
			rpt.valve_test = DIAG_TEST_FAIL;


	//////////////////////
	// TEST SD Card
	//////////////////////
	vSERIAL_sout("\r\nTesting SD Card...", 20);

	if(!SD_Testing_Basic()){
		// success
		rpt.SD_test = DIAG_TEST_PASS;
	}
	else{
		// fail
		rpt.SD_test = DIAG_TEST_FAIL;
	}

	vSERIAL_sout("\r\nSD Card Test Complete\r\n", 25);


	//////////////////////
	// TEST FRAM
	//////////////////////
	vSERIAL_sout("\r\nTesting FRAM...", 17);
	__delay_cycles(16000000);
	if(uiL2FRAM_chk_for_fram_format() != 0){
		// if fram is formatted, then pass
		rpt.FRAM_test = DIAG_TEST_PASS;
	}
	else rpt.FRAM_test = DIAG_TEST_FAIL;

	vSERIAL_sout("\r\nFRAM Test Complete\r\n", 22);

	//////////////////////
	// Show Connections
	//////////////////////

	vSERIAL_sout("\r\nCompiling Results into Report\r\n\r\n", 35);
	__delay_cycles(16000000);

	// zeros out the buffer
	vSERIAL_CleanBuffer((uchar *)ucaMSG_BUFF);

	// build results report
	ucaMSG_BUFF[0] = 0xDD; // denotes diagnostic results report
	ucaMSG_BUFF[1] = 0xDD;

	// SP1 name
	ucaMSG_BUFF[2] = rpt.SP_slot1[0];
	ucaMSG_BUFF[3] = rpt.SP_slot1[1];
	ucaMSG_BUFF[4] = rpt.SP_slot1[2];
	ucaMSG_BUFF[5] = rpt.SP_slot1[3];
	ucaMSG_BUFF[6] = rpt.SP_slot1[4];
	ucaMSG_BUFF[7] = rpt.SP_slot1[5];
	ucaMSG_BUFF[8] = rpt.SP_slot1[6];
	ucaMSG_BUFF[9] = rpt.SP_slot1[7];

	// SP2 name
	ucaMSG_BUFF[10] = rpt.SP_slot2[0];
	ucaMSG_BUFF[11] = rpt.SP_slot2[1];
	ucaMSG_BUFF[12] = rpt.SP_slot2[2];
	ucaMSG_BUFF[13] = rpt.SP_slot2[3];
	ucaMSG_BUFF[14] = rpt.SP_slot2[4];
	ucaMSG_BUFF[15] = rpt.SP_slot2[5];
	ucaMSG_BUFF[16] = rpt.SP_slot2[6];
	ucaMSG_BUFF[17] = rpt.SP_slot2[7];

	// SP3 name
	ucaMSG_BUFF[18] = rpt.SP_slot3[0];
	ucaMSG_BUFF[19] = rpt.SP_slot3[1];
	ucaMSG_BUFF[20] = rpt.SP_slot3[2];
	ucaMSG_BUFF[21] = rpt.SP_slot3[3];
	ucaMSG_BUFF[22] = rpt.SP_slot3[4];
	ucaMSG_BUFF[23] = rpt.SP_slot3[5];
	ucaMSG_BUFF[24] = rpt.SP_slot3[6];
	ucaMSG_BUFF[25] = rpt.SP_slot3[7];

	// SP4 name
	ucaMSG_BUFF[26] = rpt.SP_slot4[0];
	ucaMSG_BUFF[27] = rpt.SP_slot4[1];
	ucaMSG_BUFF[28] = rpt.SP_slot4[2];
	ucaMSG_BUFF[29] = rpt.SP_slot4[3];
	ucaMSG_BUFF[30] = rpt.SP_slot4[4];
	ucaMSG_BUFF[31] = rpt.SP_slot4[5];
	ucaMSG_BUFF[32] = rpt.SP_slot4[6];
	ucaMSG_BUFF[33] = rpt.SP_slot4[7];

	// test results
	ucaMSG_BUFF[34] = rpt.LED_test;
	ucaMSG_BUFF[35] = rpt.radio_disc_test;
	ucaMSG_BUFF[36] = rpt.radio_op_test;
	ucaMSG_BUFF[37] = rpt.buzzer_test;
	ucaMSG_BUFF[38] = rpt.valve_test;
	ucaMSG_BUFF[39] = rpt.SD_test;
	ucaMSG_BUFF[40] = rpt.FRAM_test;

	// added types and mode
	ucaMSG_BUFF[41] = rpt.diagMode; // deployment or field mode

	ucaMSG_BUFF[42] = SP1_types[0];
	ucaMSG_BUFF[43] = SP1_types[1];
	ucaMSG_BUFF[44] = SP1_types[2];
	ucaMSG_BUFF[45] = SP1_types[3];

	ucaMSG_BUFF[46] = SP2_types[0];
	ucaMSG_BUFF[47] = SP2_types[1];
	ucaMSG_BUFF[48] = SP2_types[2];
	ucaMSG_BUFF[49] = SP2_types[3];

	ucaMSG_BUFF[50] = SP3_types[0];
	ucaMSG_BUFF[51] = SP3_types[1];
	ucaMSG_BUFF[52] = SP3_types[2];
	ucaMSG_BUFF[53] = SP3_types[3];

	ucaMSG_BUFF[54] = SP4_types[0];
	ucaMSG_BUFF[55] = SP4_types[1];
	ucaMSG_BUFF[56] = SP4_types[2];
	ucaMSG_BUFF[57] = SP4_types[3];

	// send results report to connected PC
	for(ucII = 0; ucII < 58; ucII++){
		vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
	}
	vSERIAL_crlf();

	// zeros out the message buffer
	vSERIAL_CleanBuffer((uchar *)ucaMSG_BUFF);

	// return to normal operation
	ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_mode_main_bit &= 0;
} // end vFULLDIAG_diagnostic_asynchronous_operations

/**************** vFULLDIAG_deployment_diagnostic() *****************************************
* runs through all of the diagnostic tests for G3 wisards
**********************************************************************************************/
void vFULLDIAG_deployment_diagnostic(void)
{
	vFULLDIAG_diagnostic_asynchronous_operations();

} // end vFULLDIAG_deployment_diagnostic



/*-------------------------------  MODULE END  ------------------------------*/

