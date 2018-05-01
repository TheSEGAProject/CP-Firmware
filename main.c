// version 4.1 	Added SD card check on start up.
// version 4.2 	Reduced dropped links.  Increased diagnostic reporting.
// version 4.3 	Fixed scheduling bugs
// version 4.4 	Added security layer to FRAM.
// 							Changed SD card task to dispatch ASAP.
//							Fixed SD Card v3.0 bug
//							Added crisis logging (critical messages stored in flash)
// version 4.5 	Added support for the solar charge controller
//							Added the report HID task
//							Replaced vSERIAL_rom_sout with a function that requires a string length parameter
// version 4.6 	Fixed bug with core voltage check function
// version 4.7  Added the burst mode discovery to allow sampling while searching for a network
//							Added the request for maximum communication slots when SRAM is backlogged
// version 4.8  Fixed timing bug with the discovery messages
//							Fixed bug preventing commands from going out to rings > 1
//							Fixed bug in routing tables preventing nodes from deleting nodes two rings away.
// version 4.9	Added expiration time to commands
//							Cleaned up fram.c.  It contained several lines of unused debug code
//							Added RSSI request parameter to the wait_for_message function
//							Added watchdog timout during diagnostic mode
//							Added flag field to the CP SP communication protocol
//							Stopped orphaning  children for partial discovery mode
//							Fixed bug in runtime radio diag task, return length was one byte short
//							Fixed bug in GS communication, several sequential bad messages were not being removed from the queue
// version 4.A	Updated SCC communication to contain a flag byte
//							Fixed bug: When parent didn't receive LRQ comm rescheduling failed
//							Added SD card address at the network layer to make reading the SD card easier
//							Allocated seperate local buffer for ack messages over RF to reduce complexity
// 							Added ability to reprogram SP boards over USB
// 							Fixed bug, runtime radio diag report length was wrong
//							Fixed bug: routing list was accumulating non-existent routes.  Dropping children didn't clear table.
// version 4.B	Diagnostic bug fixes
// version 4.C
// version 4.D  Updated SCC and diag menu to include setting SCC HID
// version 4.E  Updated SCC default interval - diagnostic update coming soon

/**************************  MAIN.C  ******************************************
 *
 * Main routine for wizard.
 *
 ******************************************************************************/

#define CP_VERSION 0x4E

#define THIS_VERSION_SUBCODE	' '		//debug codes
//#define THIS_VERSION_SUBCODE	'a'		//debug codes
//#define THIS_VERSION_SUBCODE	'b'		//debug codes
//#define THIS_VERSION_SUBCODE	'c'		//debug codes
//#define THIS_VERSION_SUBCODE	'd'		//debug codes

/*lint -e526 *//* function not defined */
/*lint -e563 *//* label not referencecd */
/*lint -e657 *//* Unusual (nonportable) anonymous struct or union */
/*lint -e714 *//* symbol not referenced */
/*lint -e716 *//* while(1) ... */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
/*lint -e757 *//* global declarator not referenced */
/*lint -e758 *//* global union not referenced */
/*lint -e768 *//* global struct member not referenced */

#include <msp430.h>		//register and ram definition file
#include "std.h"			//std defines
#include "diag.h"			//Diagnostic package header
#include "main.h"			//main defines
#include "delay.h"		//approx delay routine
#include "misc.h"			//homeless routines
#include "rts.h"			//Real Time Sched routines
#include "rand.h"			//random number generator
#include "key.h"			//keyboard handler
#include "serial.h"		//serial port
#include "time.h"			//System Time routines
#include "daytime.h"	//Daytime routines
#include "button.h" 	//Button routines
#include "report.h"		//Reporting routines
#include "comm.h"			//msg helper routines
#include "gid.h"			//Group ID routines
#include "sensor.h"		//sensor routines
#include "pick.h"			//Trigger routines
#include "MODOPT.h"		//Modify Options routines
#include "LNKBLK.h"		//radio link handler routines
#include "config.h"		//system configuration definitions
#include "MPY32.h"
#include "SP.h"				//SP board control
#include "buz.h"			//Buzzer routines
#include "led.h"			//on board LED definitions
#include "adf7020.h"	//ADF7020 definitions
#include "mem_mod.h"	//Level 2 memory routines
#include "flash_mcu.h" 		// Flash memory controller for the MCU
#include "task.h"
#include "gs.h"				// Garden server
#include "PMM.h"
#include "SD_Card.h"
#include "scc.h"
#include "sys.h"
#include "OTA.h" 			// Over the air reprogramming

#ifdef DEBUG_DISPATCH_TIME
#include "t0.h"			//Timer T0 routines
#endif

#include "crc.h"			//CRC calculator routine
#ifdef ESPORT_ENABLED				//defined in diag.h
#include "esport.h"			//external serial port
#endif

/****************************  DEFINES  **************************************/

//#define KILL_ALL_CHECKS TRUE
#ifndef KILL_ALL_CHECKS
#define KILL_ALL_CHECKS FALSE
#endif

//#define DEBUG_DISPATCH_TIME 1

/**********************  VOLATILE  GLOBALS  **********************************/

volatile uchar ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];
volatile ADF7020_Driver_t ADF7020_Driver;

//Time keeping variables

volatile ulong uslALARM_TIME;
volatile ulong uslCLK_TIME;
volatile ulong uslCLK2_TIME;

volatile uint8 ucBUTTON_COUNT[4];

#ifdef INC_ESPORT						//defined on Cmd line
volatile uint8 ucESPORT_TX_Byte; //transmit byte ram
volatile uint8 ucESPORT_RX_Byte;//receive byte ram
volatile uint8 ucESPORT_BitCounter;//
volatile uint8 ucESPORT_TimeCounter;//
#endif /* END: INC_ESPORT */

volatile uint8 ucaBigMinuend[6];
volatile uint8 ucaBigSubtrahend[6];
volatile uint8 ucaBigDiff[6];

volatile uint8 ucRAND_NUM[RAND_NUM_SIZE];

volatile uint8 ucQonIdx_LUL;
volatile uint8 ucQoffIdx_LUL;
volatile uint8 ucQcount;

volatile union //ucFLAG0_BYTE
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

#define FLAG0_INIT_VAL	    0x00		//0000 0000
volatile union //ucFLAG1_BYTE
{
	uint8 byte;
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

#define FLAG1_INIT_VAL	  0x00  	//0000 0000

volatile union //ucFLAG2_BYTE
{
	uint8 byte;
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

#define FLAG2_INIT_VAL	0x00	//00000000

volatile union
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
#define FLAG3_INIT_VAL	0x00	//00000000

volatile union //ucGLOB_diagFlagByte1
{
	uint8 byte;
	struct
	{
		unsigned DIAG_mute_bit :1; //bit 0 ;1=MUTE, 0=SOUND
		unsigned DIAG_test_repeat_looping_bit :1; //bit 1 ;1=LOOPING, 0=NOT LOOPING
		unsigned DIAG_only_a_single_test_bit :1; //bit 2	;1=ONLY A SINGLE, 0=ALL
		unsigned DIAG_only_a_single_group_bit :1; //bit 3 ;1=ONLY A GROUP, 0=ALL
		unsigned DIAG_exit_out_to_top_bit :1; //bit 4 ;1=EXIT OUT, 0=NO EXIT OUT
		unsigned DIAG_exit_out_to_prev_test_bit :1; //bit 5 ;1=GOTO PREV, 0=NO GOTO PREV
		unsigned DIAG_exit_out_to_next_test_bit :1; //bit 6 ;1=GOTO NEXT, 0=NO GOTO NEXT
		unsigned DIAG_quit_out_to_return_bit :1; //bit 7 ;1=TOTO RETURN, 0=NO QUIT
	} diagFlagByte1_STRUCT;
} ucGLOB_diagFlagByte1;

#define DIAG_FLAG_BYTE_1_INIT_VAL 0x00	//00000000
volatile union //ucGLOB_diagFlagByte2
{
	uint8 byte;
	struct
	{
		unsigned DIAG_tell_whats_running_bit :1; //bit 0 ;1=TELL, 0=NO TELL
		unsigned DIAG_change_looping_sts_bit :1; //bit 1 ;1=CHANGE LOOPING, 0=NO CHANGE
		unsigned DIAG_halt_on_error_bit :1; //bit 2 ;1=halt, 0=no halt
		unsigned DIAG_partially_automated_run_bit :1; //bit 3	;1=an 'A' was hit, 0=no 'A' hit
		unsigned DIAG_not_used_4_bit :1; //bit 4 ;
		unsigned DIAG_not_used_5_bit :1; //bit 5 ;
		unsigned DIAG_not_used_6_bit :1; //bit 6 ;
		unsigned DIAG_mode_main_bit :1; //bit 7 ;
	} diagFlagByte2_STRUCT;
} ucGLOB_diagFlagByte2;

#define DIAG_FLAG_BYTE_2_INIT_VAL 0x00	//00000000
/*---------  END of VOLATILES  -----------------------------*/

/***********************  NON-VOLATILE GLOBALS  *****************************/

uint8 ucGLOB_myLevel; //senders level +1

long lGLOB_initialStartupTime; //Time used to compute uptime

long lGLOB_lastAwakeTime; //Nearest thing to slot time
long lGLOB_opUpTimeInSec; //Nearest thing to cur operational up time
long lGLOB_lastAwakeLinearSlot; //Nearest thing to cur linear slot a
long lGLOB_lastAwakeFrame; //Nearest thing to cur frame
uint8 ucGLOB_lastAwakeSlot; //Nearest thing to cur slot
uint8 ucGLOB_lastAwakeNSTtblNum; //Nearest thing to cur NST tbl
uint8 ucGLOB_lastAwakeStblIdx; //Nearest thing to cur sched idx

//! \var g_ucaCurrentTskIndex
//! \brief The task index of the current executing
uchar g_ucaCurrentTskIndex;

long lGLOB_lastScheduledFrame; //last scheduled frame number

long lGLOB_OpMode0_inSec; //Start of Opmode

usl uslGLOB_sramQon_NFL;
usl uslGLOB_sramQoff;
uint uiGLOB_sramQcnt;
uint uiGLOB_curMsgSeqNum;

int iGLOB_Hr0_to_SysTim0_inSec; //dist from SysTim0 to Hr0
//uint uiGLOB_WrldStartHr;			//Starting hour in WorldTime

uint uiGLOB_grpID; //group ID for this group
uint8 ucGLOB_StblIdx_NFL; //next free loc in the sched tables

uint8 ucGLOB_lineCharPosition; //line position for computing tabulation

int iGLOB_completeSysLFactor; //entire Signed LFactor quantity.
ulong ulGLOB_msgSysLFactor;		//Message (byte size) unsigned Load Factor

uint8 ucGLOB_msgSysLnkReq; //Msg Linkup Req

uint8 ucGLOB_radioChannel; //Current radio channel number (0 - 127)

/*----------  DEBUG RAM LOCATIONS  ---------*/
uint uiGLOB_bad_flash_CRC_count; //count of bad CRC's on flash msgs

uint uiGLOB_lostROM2connections; //total lost ROM2's, Zro on startup
uint uiGLOB_lostSOM2connections; //total lost SOM2's, Zro on startup

uint uiGLOB_ROM2attempts; //total ROM2 attempts, Zro on startup
uint uiGLOB_SOM2attempts; //total SOM2 attempts, Zro on startup

uint uiGLOB_TotalSDC4trys; //total SDC4 attempts, Zro on startup
uint uiGLOB_TotalRTJ_attempts; //total RDC4 attempts, Zro on startup

ulong tFactor;
ulong tOffset;

union //ucGLOB_debugBits1
{
	uint8 byte;
	struct
	{
		unsigned DBG_MaxIdxWriteToNST :1; //bit 0 ;set if err, clr'd after reporting
		unsigned DBG_MaxIdxReadFromNST :1; //bit 1 ;;set if err, clr'd after reporting
		unsigned DBG_notUsed2 :1; //bit 2 ;
		unsigned DBG_notUsed3 :1; //bit 3	;
		unsigned DBG_notUsed4 :1; //bit 4 ;
		unsigned DBG_notUsed5 :1; //bit 5 ;
		unsigned DBG_notUsed6 :1; //bit 6 ;
		unsigned DBG_notUsed7 :1; //bit 7 ;
	} debugBits1_STRUCT;
} ucGLOB_debugBits1;

#define DEBUG_BITS_1_INIT_VAL 0x00	//00000000
//Structure that defines the role and the identity of the WiSARD
struct Role G3Role;

/* RAM COPY OF FRAM OPTION BIT ARRAY */
uint8 ucaGLOB_optionBytes[OPTION_BYTE_COUNT];

unsigned char g_ucSP1Ready;
unsigned char g_ucSP2Ready;
unsigned char g_ucSP3Ready;
unsigned char g_ucSP4Ready;

/******************************  DECLARATIONS  *******************************/
void vMain_Tests(void);
void vMAIN_computeDispatchTiming(void);
uchar ucMain_VerifyImage(void);
#ifdef RUN_NOTHING
static void vMAIN_run_nothing(
		void
);
#endif

void vMAIN_showCompilerAnomalies(const char *cMsg);
void vMain_LogRestart(void);
/*******************************  CODE  **************************************/

/*******************************  MAIN  *************************************
 *
 *
 *
 *
 ******************************************************************************/
void main(void){
	//Halt the dog while the system initializes
	WDTCTL = WDTPW + WDTHOLD;

	/**************************************************************************
	 *
	 * Initialize the Ports, Clock, Serial Communication, and GIE
	 *
	 *************************************************************************/

	// SET INITIAL VALUE ON PORTS
	vConfig_InitializePorts();

	//Initialize clocks MCLK=16MHz SMCLK=4MHz
	vUCS_InitializeClock();

	// Start the dog, if not fed for 16 seconds then it will reset
	WDTCTL = WDTPW + WDTSSEL_1 + WDTCNTCL + WDTIS_3;

	//setup the serial port and inform user
	vSERIAL_init();

	// enable global interrupts
	__bis_SR_register(GIE);

	P7DIR |= BIT6;
	P7OUT &= ~BIT6;

	/**************************************************************************
	 *
	 * RAM MEMORY INIT, Initialize the runtime global variables
	 *
	 *************************************************************************/
	//Initialize the Flags to a known state
	ucFLAG0_BYTE.byte = FLAG0_INIT_VAL;
	ucFLAG1_BYTE.byte = FLAG1_INIT_VAL;
	ucFLAG2_BYTE.byte = FLAG2_INIT_VAL;
	ucFLAG3_BYTE.byte = FLAG3_INIT_VAL;
	ucGLOB_diagFlagByte1.byte = DIAG_FLAG_BYTE_1_INIT_VAL;
	ucGLOB_diagFlagByte2.byte = DIAG_FLAG_BYTE_2_INIT_VAL;

	ADF7020_Driver.eDriverState = SHUTDOWN;
	ADF7020_Driver.eRadioState = RADIO_OFF;

	uiGLOB_curMsgSeqNum = 1;
	ucGLOB_lineCharPosition = 0;
	iGLOB_completeSysLFactor = 0;
	ucGLOB_StblIdx_NFL = 0;

	//set the radio channel to an illegal value
	ucGLOB_radioChannel = ILLEGAL_CHANNEL;

	uiGLOB_bad_flash_CRC_count = 0; //count of bad CRC's on flash retrieve

	uiGLOB_lostROM2connections = 0; //counts lost ROM2's
	uiGLOB_lostSOM2connections = 0; //counts lost SOM2's

	uiGLOB_ROM2attempts = 0; //count number of ROM2 attempts
	uiGLOB_SOM2attempts = 0; //count number of SOM2 attempts

	uiGLOB_TotalSDC4trys = 0; //counts number of SDC4 attempts
	uiGLOB_TotalRTJ_attempts = 0; //counts number of RDC4 attempts

	ucGLOB_debugBits1.byte = 0; //debug flags

	// Clear SP board flag bytes
	g_ucSP1Ready = 0;
	g_ucSP2Ready = 0;
	g_ucSP3Ready = 0;
	g_ucSP4Ready = 0;

	// Secure all memory locations in FRAM
	vL2FRAM_LockAllMemory();

	if (uiL2FRAM_chk_for_fram_format())
		// SETUP THE ucaGLOB_roleByte[]	ARRAY, read the role information from FRAM
		vMODOPT_copyAllFramOptionsToRamOptions();
	else
		// otherwise, just format fram
		vL2FRAM_format_fram();

	// If the device has been reprogrammed then format FRAM
//	TODO verify that this is a safe method going forward
//	if(ucMain_VerifyImage() == 1)
//		vL2FRAM_format_fram();

	// Initialize the on-chip RAM queue
	vReport_RAM_QueueInit();
	vL2SRAM_init();

	// Checks to see if the queue is already formatted
	// If not then format, otherwise retain contents
	if (!ucL2SRAM_IsCmdQueueFormatted())
		vL2SRAM_FormatCmd_Q();

	// Check to see if there is a functioning SD card present
	vSD_PowerOn();
	if (ucSD_CheckForCard() == SD_FAILED) {
		vBUZ_tune_bad_news_3();
		vSERIAL_sout("\r\nSD Card Err\r\n,", 15);
	}
	vSD_PowerOff();

	/*****************  END: MEMORY INIT  ********************************/

	/*------------------- START: FUNCTION INIT  -----------------------------*/

	//setup the button
	vBUTTON_init();

	/*------------------ 0 -----------------------*/
	vSERIAL_sout("\r\n0\r\n,", 5);

	//setup the clock
	vTIME_init();

	//If hub then synch with garden server
	if (ucL2FRAM_isHub()) {
		vGS_Init();
		vGS_SynchGardenServer();
	}

	/*****************  Check Battery Voltage  ********************************/
	while (TRUE) //lint !e774
	{
		// If the battery voltage is good or we have external power then break
		if (uiMISC_doCompensatedBattRead() > 3000 || ( ucPMM_chkLowVoltage(BATT_LOW_V300) == 0) )
			break; //above 2.8V is good

		vSERIAL_sout("BattLow= ", 9);
		vTask_showBattReading();
		vSERIAL_crlf();

		vBUZ_tune_bad_news_1(); //Battery Low Sound

		vTIME_setAlarmFromLong(lTIME_getSysTimeAsLong() + 4L);
		ucMISC_sleep_until_button_or_clk(SLEEP_MODE); //lint !e534

	}/* END: while() */

	vSERIAL_sout("Batt= ", 6);
	vTask_showBattReading();
	vSERIAL_crlf();

	/*------------------ 1 ----------------------*/
	vSERIAL_sout("\r\n1,\r\n", 6);

#ifdef DEBUG_DISPATCH_TIME
	//vT0_init();
#endif

	/* SET THE LINK LEVEL */
	ucGLOB_myLevel = LEVEL_MAX_VAL;
	if (ucL2FRAM_isHub()) //Hub init for level & group
	{
		/* INIT THE LEVEL FOR THE HUB */
		ucGLOB_myLevel = 0; //init level

		// INIT GID-SELECTOR
		vGID_init();
	}

#if 0		//DEBUG
	vSERIAL_sout("\r\nTEST ROUTINES\r\n", 17);
	if(ucL2FRAM_isHub()) vSERIAL_sout("IS HUB\r\n", 8);
	if(ucL2FRAM_isSender()) vSERIAL_sout("IS SENDER\r\n", 11);
	if(ucL2FRAM_isSampler()) vSERIAL_sout("IS SAMPLER\r\n", 12);
	if(ucL2FRAM_isReceiver()) vSERIAL_sout("IS RECEIVER\r\n", 13);
#endif		//END: DEBUG


	/* *****************************************************************************
		 *
		 * Initialize the SP boards and set the role accordingly
		 *
		 * ****************************************************************************/
		vSERIAL_sout("2,\r\n", 4);

		/* INIT THE SP BOARDS */
		if (ucMODOPT_readSingleRamOptionBit(OPTPAIR_SPS_ARE_ATTCHD))
		{
			//Start up the SP driver
			ucSP_Init();
			//checks what SP boards are where, sets the Role accordingly
			vSP_SetRole();

		}/* END: if(OPTPAIR_SPS_ARE_ATTCHD) */

		// Check for the solar charge controller and initialize driver if present
		vSCC_Init();

	// Function for conducting a variety of tests from main
	// vMain_Tests();

	/*------------------- 3 ---------------------*/
	vSERIAL_sout("3,", 2);

	/* INCREMENT THE STARUP COUNTER */
	vL2FRAM_incRebootCount();

	/*------------------- 4 ---------------------*/
	vSERIAL_sout("4,", 2);

	/* RUN EARLY DIAGNOSTICS */

	/*------------------- 5 ---------------------*/
	vSERIAL_sout("5,", 2);

	/* TELL THAT SYS IS RUNNING */
	vMAIN_printIntro();

	/* BLINK TO LET THEM KNOW WE ARE ALIVE */
	vLED_RedBlink(100);
	vLED_GrnBlink(100);

	/*-------------------- 6 --------------------*/
	vSERIAL_sout("6,", 2);

	// Initialize the task manager
	if(ucTask_Init() != TASKMNGR_OK){
		vSERIAL_sout("TskMngr Err\r\n", 13);
		vTaskDisplayTaskList();
	}

	/*-------------------- 7 --------------------*/
	vSERIAL_sout("7,", 2);

#if (KILL_ALL_CHECKS == FALSE)

	/******************  SUB-SYSTEM CHECKS  ************************************/

	/***************************************************************************
	 *
	 * Check that the FRAM is working, then check to see if it is formatted
	 *
	 * *************************************************************************/
	vSERIAL_sout("8,", 2);

	// Verify the FRAM check is required by the role
	if (ucMODOPT_readSingleRamOptionBit(OPTPAIR_CHK_FOR_FRAM_ON_STUP))
	{
		// While the FRAM check fails repeat the process
		while (!ucL2FRAM_chk_for_fram(FRAM_CHK_REPORT_MODE))
		{
			vSERIAL_sout("FramFail\r\n", 10);
			vBUZ_morrie(); //Fram Failed
			vSERIAL_sout("Rtry\r\n", 6);

		}/* END: while() */

	}/* END: if(OPTPAIR_CHK_FOR_FRAM_ON_STUP) */

	/* CHECK IF THE FRAM IS FORMATTED */
	if (!uiL2FRAM_chk_for_fram_format())
	{
		long lBegTime;
		uint8 ucChar;

		/* WUPS FRAM IS NOT FORMATTED -- CHECK IF WE SHOULD FORMAT IT*/
		ucChar = 0;
		vBUZ_tune_bad_news_2(); //Fram not formatted
		vSERIAL_sout("\r\nFramNotFmted--LeaveIt (WARNING will clr SD)? [YorN] ", 54);
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

		/* FORMAT THE FRAM */
		if ((ucChar != 'Y') && (ucChar != 'y'))
		{
			vSERIAL_sout("Fmting FRAM, ", 13);
			vL2FRAM_format_fram();

			//Also format the SD card since the block pointer in FRAM was just reset
			vSERIAL_sout("Fmting SD\r\n", 11);
			vSD_PowerOn();
			ucSD_Init();
			SD_Format();
			vSD_PowerOff();

		}

	}/* END: if() */

	/*----------------- 9 ---------------------*/
	vSERIAL_sout("9, ", 3);
	vL2FRAM_showTSBTbl();

	/*------------------ 10 ---------------------*/
	vSERIAL_sout("10, ", 4);

	// Check the state on shutdown byte in FRAM
	if (ucL2FRAM_GetStateOnShutdown())
	{
		// Load the last block from the SD card into the SRAM message buffer
		vReport_LoadSRAMFromSDCard();
		// Reset the state on shutdown byte
		vL2FRAM_SetStateOnShutdown(0x00);
	}

	/*------------------- 11 --------------------*/
	vSERIAL_sout("11 ,", 4);

	// Set up the radio registers and the routing table
	ucRoute_Init(uiL2FRAM_getSnumLo16AsUint());
	unADF7020_Initialize(NULL);

	/* CHECK IF THE RADIO BOARD IS CONNECTED */
	if (ucMODOPT_readSingleRamOptionBit(OPTPAIR_CHK_FOR_RDIO_BD))
	{
		while (!ucADF7020_ChkforRadio())
		{
			vSERIAL_sout("NoRdioBrd\r\n", 11);
			vBUZ_raygun_up(); //No Radio Board
			vSERIAL_sout("Rtry\r\n", 6);
		}/* END: while() */

		/* INT RANDOM NUMBERS */
		uslRAND_getNewSeed(); //lint !e534 //stuff a new seed into rand array

#if 0
	vSERIAL_sout("Rand= ", 6);
	vSERIAL_HB24out(uslRAND_getFullSysSeed());
	vSERIAL_crlf();
#endif

	} /* END: if() */

#endif /* KILL_ALL_CHECKS */

	/****************  START RUNNING THE INITIAL SYSTEM  *************************/

	/*------------------- 12 --------------------*/
	vSERIAL_sout("12, ", 4);

	vMain_LogRestart();

	/* TELL THE WORLD THAT WE ARE ALIVE */
	vMISC_blinkLED(5);

	vBUZ_tune_imperial();

	vMAIN_printIntro();

	/*------------------- 13 --------------------*/
	vSERIAL_sout("13,", 2);

	/* RUN THE DIAGNOSTICS AT THIS POINT */
	vDIAG_run_diagnostics();

	/* SHOW THE CURRENT ROLE PARAMETERS */
	vMODOPT_showAllRamOptionBits();

	/************ OPMODE INIT  ***********/

	/* CLEAR THE LNKBLK TABLE */
	vLNKBLK_zeroEntireLnkBlkTbl();

	/* SETUP THE TRIGGER TABLES */
	vPICK_initSSPtbls();

	/* CLEAR BOTH NST TBLS */
	vRTS_clrNSTtbl(0);
	vRTS_clrNSTtbl(1);

	/* INIT THE CURRENT FRAME COUNTER */
	lGLOB_OpMode0_inSec = 0; //first frame start = 4, 2nd frame st = 260

	lGLOB_lastAwakeTime = lTIME_getSysTimeAsLong();
	lGLOB_opUpTimeInSec = lGLOB_lastAwakeTime - lGLOB_OpMode0_inSec;
	lGLOB_lastAwakeLinearSlot = lGLOB_opUpTimeInSec / SECS_PER_SLOT_L;
	lGLOB_lastAwakeFrame = lGLOB_lastAwakeLinearSlot / SLOTS_PER_FRAME_I;
	ucGLOB_lastAwakeSlot = (uint8) (lGLOB_lastAwakeLinearSlot % SLOTS_PER_FRAME_I);
	ucGLOB_lastAwakeNSTtblNum = (uint8) (lGLOB_lastAwakeFrame % 2);

#if 0
	vSERIAL_sout("lGLOB_OpMode0_inSec= ", 21);
	vSERIAL_IV32out(lGLOB_OpMode0_inSec);
	vSERIAL_crlf();

	vSERIAL_sout("lGLOB_lastAwakeTime= ", 21);
	vSERIAL_IV32out(lGLOB_lastAwakeTime);
	vSERIAL_crlf();

	vSERIAL_sout("lGLOB_opUpTimeInSec= ", 21);
	vSERIAL_IV32out(lGLOB_opUpTimeInSec);
	vSERIAL_crlf();

	vSERIAL_sout("lGLOB_lastAwakeLinearSlot= ", 27);
	vSERIAL_IV32out(lGLOB_lastAwakeLinearSlot);
	vSERIAL_crlf();

	vSERIAL_sout("lGLOB_lastAwakeFrame= ", 22);
	vSERIAL_IV32out(lGLOB_lastAwakeFrame);
	vSERIAL_crlf();

	vSERIAL_sout("ucGLOB_lastAwakeSlot= ", 22);
	vSERIAL_UIV8out(ucGLOB_lastAwakeSlot);
	vSERIAL_crlf();

	vSERIAL_sout("ucGLOB_lastAwakeNSTtblNum= ", 27);
	vSERIAL_UIV8out(ucGLOB_lastAwakeNSTtblNum);
	vSERIAL_crlf();
	vSERIAL_crlf();

#endif

	/* SCHEDULE THE CURRENT NST */
	vRTS_scheduleNSTtbl(lGLOB_lastAwakeFrame);

#if 0
	vRTS_showAllNSTentrys(0, YES_SHOW_STBL);
	vRTS_showAllNSTentrys(1, YES_SHOW_STBL);
#endif

	// Clear the end of slot alarm and wait for the start of the second to roll around
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;
	while(ucTimeCheckForAlarms(GENERAL_ALARM_BIT)==0)
		LPM1;
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;

	// clear flag
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG2_BUTTON_INT_BIT = 0;

	while (1)
	{
		// Feed the dog
		WDTCTL = WDTPW + WDTSSEL_1 + WDTCNTCL + WDTIS_3;

		if(ucGLOB_diagFlagByte2.diagFlagByte2_STRUCT.DIAG_mode_main_bit)
		{
			//Halt the dog while the system initializes
			WDTCTL = WDTPW + WDTHOLD;

			// run diagnostics
			vFULLDIAG_diagnostic_asynchronous_operations();

		}

		else{
			// check for button press
			if(ucFLAG3_BYTE.FLAG3_STRUCT.FLG2_BUTTON_INT_BIT)
			{
				// Halt the dog for diagnostics

				// Change the watchdog timer interval, if not fed for 1 hr seconds then it will reset
				WDTCTL = WDTPW + WDTSSEL_1 + WDTCNTCL + WDTIS_1;

				ucKEY_doKeyboardCmdInput();

				// clear flags (interrupt and clock)
				ucFLAG3_BYTE.FLAG3_STRUCT.FLG2_BUTTON_INT_BIT = 0;
				ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;

				// Enable interrupts on serial RX
				UART1_REG_IFG &= ~ UART1_RX_IFG;
				UART1_REG_IE |= UART1_RX_IE;

				// Set the watchdog timer interval back to 16 seconds
				WDTCTL = WDTPW + WDTSSEL_1 + WDTCNTCL + WDTIS_3;
			}

			//heart beat
			P7OUT ^= BIT6;

			// Dispatch to task
			vTask_Dispatch(ucGLOB_lastAwakeNSTtblNum, ucGLOB_lastAwakeSlot);

			// Check to see if the SPs have data
			vRTS_CheckSPDataPending();

			//Compute the time after dispatch
			vMAIN_computeDispatchTiming();
		}
	}/* END: while(TRUE) */

} // End main

/***********************  vMAIN_showVersionNum()  *****************************
 *
 *
 *
 *
 ******************************************************************************/

void vMAIN_showVersionNum(void)
{

	vSERIAL_bout('V');
	vSERIAL_HB4out(CP_VERSION>>4);
	vSERIAL_bout('.');
	vSERIAL_HB4out(CP_VERSION);
#if (THIS_VERSION_SUBCODE != ' ')
	vSERIAL_bout(THIS_VERSION_SUBCODE);
#endif

	return;

}/* END: vMAIN_showVersionNum() */

/************************  vMAIN_printIntro()  ************************************
 *
 * Print out our introduction message
 *
 ******************************************************************************/

void vMAIN_printIntro(void)
{
	signed char cIndex;
	uint uiHID[4];

	/* ISSUE THE INTRO MESSAGE */
	vSERIAL_crlf();
	vDAYTIME_convertSysTimeToShowDateAndTime(TEXT_FORM);

	vSERIAL_sout("\r\nNetID ", 8);
	vSERIAL_bout(':');
	vL2FRAM_showSysID();
	vSERIAL_bout(' ');
	vSERIAL_sout("\r\nHID ", 6);
	vSERIAL_bout(':');

	vFlash_GetHID(uiHID);
	for (cIndex = 3; cIndex >= 0; cIndex--)
		vSERIAL_HB16out(uiHID[(uchar)cIndex]);

	vSERIAL_bout(' ');
	vMAIN_showVersionNum();
	vSERIAL_bout(' ');

	vMODOPT_showCurIdentity(); //shows role and SP types attached

	vSERIAL_sout("Radio HID ", 10);
	vSERIAL_bout(':');
	vFlash_GetRadioHID(uiHID);
	for (cIndex = 3; cIndex >= 0; cIndex--)
		vSERIAL_HB16out(uiHID[(uchar)cIndex]);

	if (ucMODOPT_readSingleRamOptionBit(OPTPAIR_SPS_ARE_ATTCHD))
	{
		vSP_GetSPMsgVersions();
	}/* END: if(OPTPAIR_SPS_ARE_ATTCHD) */
	else
	{
		vSERIAL_sout("SP-OFF)", 7);
	}/* END: else (OPTPAIR_SPS_ARE_ATTCHD) */

	vSERIAL_crlf();
	/* TELL ABOUT THE FRAM */
	vSERIAL_sout("(FRAM-", 6);

	if (ucMODOPT_readSingleRamOptionBit(OPTPAIR_CHK_FOR_FRAM_ON_STUP))
	{

		uint uiVal;

		uiVal = uiL2FRAM_get_version_num();
		vSERIAL_sout("V", 1);
		vSERIAL_UIV8out((uint8) (uiVal >> 8)); //Version Num
		vSERIAL_bout('.');
		vSERIAL_HB8out((uint8) (uiVal & 0xFF)); //Sub Version Num
		vSERIAL_bout(')');
	}
	else
	{
		vSERIAL_sout("OFF)", 4);

	}/* END: else(OPTPAIR_CHK_FOR_FRAM_ON_STUP) */

	vSERIAL_crlf();

	/* SHOW MESSAGE COUNT */
	vSERIAL_sout("M", 1); //SRAM msg count
	vSERIAL_UIV16out(uiL2SRAM_getMsgCount());

	vSERIAL_crlf();
	vSERIAL_crlf();
	return;

} /* END: vMAIN_printIntro() */

/******************  vMAIN_computeDispatchTiming()  **************************
 *
 * This routine computes the dispatcher timing and then returns.
 *
 * Many of the time keeping responsibilities have been moved to the dispatcher
 * due to the structure of the slots in the GIII WiSARD.  Therefore some of
 * the alarm setting functions have been removed from this function.  Additionally,
 * due to the fact that the slots are now 1 second long the lThisSlotRemainder is
 * no longer kept in seconds it is kept in ticks from TA1R.
 *
 ******************************************************************************/

void vMAIN_computeDispatchTiming(void)
{
	long lThisTime;
	long lThisSlotEndTime;
	long lOpUpTimeInSec;
	long lThisLinearSlot;
	uchar ucMsgIndex;
	long lThisFrameNum;

	/* CHECK IF WE HAVE HAD A TIME RESET */
	if (ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT)
	{
		// If the reset time is greater than 1 second then report it
		if (lTIME_getClk2AsLong() != lTIME_getSysTimeAsLong())
		{
			// Build the report data element header
			vComm_DE_BuildReportHdr(CP_ID, 6, ucMAIN_GetVersion());
			ucMsgIndex = DE_IDX_RPT_PAYLOAD;
			ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SET_TIME;
			ucaMSG_BUFF[ucMsgIndex++] = 4; // data length
			vMISC_copyUlongIntoBytes( lTIME_getSysTimeAsLong(), (uchar*)&ucaMSG_BUFF[ucMsgIndex], NO_NOINT);
			// Store DE
			vReport_LogDataElement(RPT_PRTY_SET_TIME);
		}

		// Update the system clock
		vTIME_setSysTimeFromClk2();

		// Update globals
		lGLOB_lastAwakeTime = lTIME_getSysTimeAsLong();
		lOpUpTimeInSec = lGLOB_lastAwakeTime - lGLOB_OpMode0_inSec;
		lGLOB_lastAwakeLinearSlot = lOpUpTimeInSec / SECS_PER_SLOT_L;
		lThisSlotEndTime = ((lGLOB_lastAwakeLinearSlot + 1) * SECS_PER_SLOT_L) + lGLOB_OpMode0_inSec;
		lGLOB_lastAwakeFrame = lGLOB_lastAwakeLinearSlot / SLOTS_PER_FRAME_I;
		ucGLOB_lastAwakeSlot = (uint8) (lGLOB_lastAwakeLinearSlot % SLOTS_PER_FRAME_I);
		ucGLOB_lastAwakeNSTtblNum = (uint8) (lGLOB_lastAwakeFrame % 2);
		lGLOB_lastScheduledFrame = lGLOB_lastAwakeFrame; //= this frame

		// If the scheduler hasn't run for this frame then run it
		if(ucRTS_getNSTSubSlotentry(ucGLOB_lastAwakeNSTtblNum,59, 4) !=  ucTask_FetchTaskIndex(TASK_ID_SCHED))
			vRTS_schedule_Scheduler_slot(ucTask_FetchTaskIndex(TASK_ID_SCHED), lGLOB_lastAwakeFrame);

		/* CLR THE RESET TIME BIT */
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT = 0;
	} //END: If time was reset

	/* WAIT TIL TIME IS POSITIVE */
	if (lGLOB_lastAwakeTime > lTIME_getSysTimeAsLong())
	{
		vSERIAL_sout("WaitingForTimePositive\r\n", 24);
		while (lGLOB_lastAwakeTime > lTIME_getSysTimeAsLong()); //lint !e722
	} //END: WAIT TIL TIME IS POSITIVE

	/* COMPUTE THE OPERATIONAL TIME IN LINEAR SLOTS */
	lThisTime = lTIME_getSysTimeAsLong();
	lOpUpTimeInSec = lThisTime - lGLOB_OpMode0_inSec;
	lThisLinearSlot = lOpUpTimeInSec / SECS_PER_SLOT_L;
	lThisSlotEndTime = ((lThisLinearSlot + 1) * SECS_PER_SLOT_L) + lGLOB_OpMode0_inSec;
	lThisFrameNum = lThisLinearSlot / SLOTS_PER_FRAME_I; //nxt frame num


	/* IF ITS IN THE LAST_SLOT+1 -- GO START IT */
	if (lThisLinearSlot == lGLOB_lastAwakeLinearSlot + 1L)
		goto Update_and_leave;

	/*------ IF WE ARE HERE THEN TIME SKIPPED FORWARD MORE THAN A SLOT -------*/

	// If we skipped ahead frames then run the scheduler for this frame
	if (lThisFrameNum > lGLOB_lastScheduledFrame)
	{
		vRTS_scheduleNSTtbl(lThisFrameNum );
	}

	Update_and_leave:

	/* WE ARE CAUGHT UP -- SO UPDATE THE CURRENT FRAME AND SLOT */
	lGLOB_lastAwakeTime = lTIME_getSysTimeAsLong();
	lOpUpTimeInSec = lGLOB_lastAwakeTime - lGLOB_OpMode0_inSec;
	lGLOB_lastAwakeLinearSlot = lOpUpTimeInSec / SECS_PER_SLOT_L;
	lThisSlotEndTime = ((lGLOB_lastAwakeLinearSlot + 1) * SECS_PER_SLOT_L) + lGLOB_OpMode0_inSec;
	lGLOB_lastAwakeFrame = lGLOB_lastAwakeLinearSlot / SLOTS_PER_FRAME_I;
	ucGLOB_lastAwakeSlot = (uint8) (lGLOB_lastAwakeLinearSlot % SLOTS_PER_FRAME_I);
	ucGLOB_lastAwakeNSTtblNum = (uint8) (lGLOB_lastAwakeFrame % 2);

	while(ucTimeCheckForAlarms(GENERAL_ALARM_BIT)==0)
		LPM1;

	/* SET SYSTIME ALARM TO NEXT SLOT START */
	vTIME_setAlarmFromLong(lThisSlotEndTime);
	vSERIAL_crlf();
	return;

}/* END: vMAIN_computeDispatchTiming() */

////////////////////////////////////////////////////////////////////////////
//! \fn ucMAIN_GetVersion()
//! \brief Returns the software version of the CP board
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
uchar ucMAIN_GetVersion(void)
{
	return CP_VERSION;
}


/////////////////////////////////////////////////////////////////////////////
//! \fn vMain_LogRestart
//! \brief Logs start up information
/////////////////////////////////////////////////////////////////////////////
void vMain_LogRestart(void)
{
	uchar ucMsgIndex;
	uint uiSysRstRsn;

	// Log the restart
	vComm_DE_BuildReportHdr(CP_ID, 3, ucMAIN_GetVersion());
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;
	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_SYSTEM_RESTART;
	ucaMSG_BUFF[ucMsgIndex++] = 1; // data length
	ucaMSG_BUFF[ucMsgIndex++] = 0; // data length
	vReport_LogDataElement(RPT_PRTY_SYSTEM_RESTART);

	// Determine the reason for the restart
	vSYS_ReadRestartVector(&uiSysRstRsn);

	// Log the restart reason
	vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
	ucMsgIndex = DE_IDX_RPT_PAYLOAD;
	ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_RST_VECTOR;
	ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiSysRstRsn >> 8);
	ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiSysRstRsn;
	vReport_LogDataElement(RPT_PRTY_RST_VECTOR);

	// Report the hardware IDs on start up
	vTask_ReportHID();

}

///////////////////////////////////////////////////////////////////////////
//!\fn ucMain_VerifyImage
//!\brief Verifies that the image on the MCU is the same as the last restart
//!\return 1 if device has been reprogrammed, 0 if the image is the same
///////////////////////////////////////////////////////////////////////////
uchar ucMain_VerifyImage(void){
	uint uiCRC1;
	uint uiCRC2;
	ulong *ulAddress;

	// disable global interrupts
	__bic_SR_register(GIE);

	// Get the CRC for the last known image
	vFlash_GetImageCRC(&uiCRC1);

	// Compute the CRC for bank C
	ulAddress = (unsigned long *)0x20000L;
	uiCRC2 = uiCRC16_CRC_on_memory(ulAddress, 0xFFFF);

	// If CRC is not equal then assume the WiSARD has been reprogrammed and update with the new CRC
	if (uiCRC1 != uiCRC2) {
		vFlash_SetImageCRC(uiCRC2);
		vSERIAL_sout("Reprogramming Detected: Formatting FRAM\r\n", 41);
		vSERIAL_HB16out(uiCRC1);
		vSERIAL_sout(" : ",3);
		vSERIAL_HB16out(uiCRC2);
		vSERIAL_crlf();
		return 1;
	}

	//Enable interrupts;
	__bis_SR_register(GIE);

	return 0;
}

///////////////////////////////////////////////////////////////////////////
//!
//! \brief Random tests
//!
//!
///////////////////////////////////////////////////////////////////////////
void vMain_SoilSensorTests(void){


	//Halt the dog
	WDTCTL = WDTPW + WDTHOLD;

//	vSP_OnboardProgramming();

	uchar ucByteCount;
	uchar ucSPIndex;
	union SP_DataMessage S_DataMsg[4];
	char * cpaSPText[4] = {"SP1", "SP2", "SP3", "SP4"};
	uchar ucSPFlags[4];
	const uchar ucFLAG_Initialized = 0x01;
	const uchar ucFLAG_CommandConfirmed = 0x02;
	const uchar ucFLAG_ReportReceived = 0x04;
	uchar ucPayload[10];
	uchar ucStreamNumber;
	uchar ucSensorNumber;
	uchar ucDataLength;
	ulong ulSensorReading;

	// Stuff the command payload
	ucPayload[0] = 0x01;
	ucPayload[1] = 0x00;
	ucPayload[2] = 0x02;
	ucPayload[3] = 0x00;
	ucPayload[4] = 0x03;
	ucPayload[5] = 0x00;
	ucPayload[6] = 0x04;
	ucPayload[7] = 0x00;

	while (1)
	{

		vSERIAL_sout("Running... \r\n", 13);

		// Clear flags and the contents of the data message structures
		for (ucSPIndex = 0; ucSPIndex < 4; ucSPIndex++) {
			ucSPFlags[ucSPIndex] = 0;
			for (ucByteCount = 0; ucByteCount < 64; ucByteCount++)
				S_DataMsg[ucSPIndex].ucByteStream[ucByteCount] = 0;
		}

		// Start up the SP boards and send the command
		for (ucSPIndex = 0; ucSPIndex < 4; ucSPIndex++) {
			if (!ucSP_Start(ucSPIndex)) {
				ucSPFlags[ucSPIndex] |= ucFLAG_Initialized;
				if (!ucSP_SendCommand(ucSPIndex, 8, ucPayload)) {
					ucSPFlags[ucSPIndex] |= ucFLAG_CommandConfirmed;
				}
			}
		}

		// Verify that all SPs are initialized and commands were confirmed
		for (ucSPIndex = 0; ucSPIndex < 4; ucSPIndex++) {

			// First check to see if the init worked if not report it and continue to the next SP
			if((ucSPFlags[ucSPIndex] & ucFLAG_Initialized) == 0){
				vSERIAL_sout("Startup Failed: ", 16);
				vSERIAL_sout(cpaSPText[ucSPIndex], 3);
				vSERIAL_crlf();
				continue;
			}
			if((ucSPFlags[ucSPIndex] & ucFLAG_CommandConfirmed) == 0){
				vSERIAL_sout("Command Failed: ", 16);
				vSERIAL_sout(cpaSPText[ucSPIndex], 3);
				vSERIAL_crlf();
			}

		}

		// Wait for a second
		vDELAY_LPMWait1us(1000000, 0);

		// Loop through all SPs and gather reports
		for (ucSPIndex = 0; ucSPIndex < 4; ucSPIndex++) {
			// If we get a report then write the SP buffer to the the data message structure
			if (!ucSP_RequestData(ucSPIndex)) {
				ucSPFlags[ucSPIndex] |= ucFLAG_ReportReceived;
				ucSP_GrabMessageFromBuffer(&S_DataMsg[ucSPIndex]);
			}
		}

		// Loop through all SPs parse messages and report errors
		for (ucSPIndex = 0; ucSPIndex < 4; ucSPIndex++) {

			// Print SP number header
			vSERIAL_sout("\r\n ----------- ", 15);
			vSERIAL_sout(cpaSPText[ucSPIndex], 3);
			vSERIAL_sout(" ----------- \r\n", 15);

			// If we recieved data then parse the packet otherwise report the error
			if((ucSPFlags[ucSPIndex] & ucFLAG_ReportReceived) != 0){

				// Loop through the message
				ucByteCount = 0;
				while(ucByteCount < (S_DataMsg[ucSPIndex].fields.ucMsgSize - 4)){

					ulSensorReading = 0;

					// Read a stream number, data length, and the sensor reading from the packet
					ucStreamNumber = S_DataMsg[ucSPIndex].fields.ucaData[ucByteCount++];
					ucDataLength = S_DataMsg[ucSPIndex].fields.ucaData[ucByteCount++];
					while(ucDataLength-- > 0){
						ulSensorReading = ulSensorReading << 8;
						ulSensorReading = S_DataMsg[ucSPIndex].fields.ucaData[ucByteCount++];
					}

					// Only run this block once since each sensor generates 2 data streams and we only need one to determine if there was a failure
					if ((ucStreamNumber % 2 == 0)) {

						// Map stream to sensor number
						switch (ucStreamNumber)
						{
							case 1:
							case 2:
								ucSensorNumber = 1;
							break;

							case 3:
							case 4:
								ucSensorNumber = 2;
							break;

							case 5:
							case 6:
								ucSensorNumber = 3;
							break;

							case 7:
							case 8:
								ucSensorNumber = 4;
							break;

						}

						if (ulSensorReading == 2) {
							vSERIAL_sout("Connectivity Fail: ", 19);
							vSERIAL_UI8out(ucSensorNumber);
							vSERIAL_crlf();
						}
						else if(ulSensorReading == 1) {
							vSERIAL_sout("Checksum Fail: ", 15);
							vSERIAL_UI8out(ucSensorNumber);
							vSERIAL_crlf();
						}
						else {
							vSERIAL_sout("Success: ", 9);
							vSERIAL_UI8out(ucSensorNumber);
							vSERIAL_crlf();
						}
					}
				}

			}
			else{
				vSERIAL_sout("Report Failed: ", 15);
				vSERIAL_sout(cpaSPText[ucSPIndex], 3);
				vSERIAL_crlf();
			}
		}


		vSP_TurnOff(SP1);
		vSP_TurnOff(SP2);
		vSP_TurnOff(SP3);
		vSP_TurnOff(SP4);

		__delay_cycles(16000000);

		// Disable RX interrupts from keyboard and wait for a key press to repeat the test
		UCA1IE &= ~UCRXIE;
		UCA1IFG &= ~UCRXIFG;
		vSERIAL_sout("Press any key to repeat test...", 31);
		while((UCA1IFG & UCRXIFG) ==0);
		vSERIAL_crlf();
		vSERIAL_crlf();

	}
}

///////////////////////////////////////////////////////////////////////////
//!
//! \brief Random tests
//!
//!
///////////////////////////////////////////////////////////////////////////
void vMain_Tests(void){


	//Halt the dog
	WDTCTL = WDTPW + WDTHOLD;


//	vOTA_init();
	//vOTA_UpdateOverSerial();
//	vOTA_ReprogramSP();
//	vSP_OnboardProgramming();

//	uchar ucByteCount;
//	union SP_DataMessage S_DataMsg;
//	uchar ucPayload[10];
//	uchar ucIndex;

//	vMain_SoilSensorTests();

//	int i;
//	while (1)
//	{
//		ucPayload[0] = 0x01;
//		ucPayload[1] = 0x00;
//		ucPayload[2] = 0x02;
//		ucPayload[3] = 0x00;
//		ucPayload[4] = 0x03;
//		ucPayload[5] = 0x00;
//		ucPayload[6] = 0x04;
//		ucPayload[7] = 0x00;
//
//		union SP_DataMessage S_DataMsg;
//
//		for(ucIndex = 0; ucIndex <64; ucIndex++)
//			S_DataMsg.ucByteStream[ucIndex]=0;


//		if (!ucSP_Start(SP1))
//				vSERIAL_sout("SP1 Ready...\r\n", 14);
//		if (!ucSP_Start(SP2))
//			vSERIAL_sout("SP2 Ready...\r\n", 14);
//		if (!ucSP_Start(SP3))
//			vSERIAL_sout("SP3 Ready...\r\n", 14);
//		if (!ucSP_Start(SP4))
//			vSERIAL_sout("SP4 Ready...\r\n", 14);

//		vSERIAL_sout(" Sending Command\r\n", 18);
//		if (!ucSP_SendCommand(SP1, 8, ucPayload))
//			vSERIAL_sout("Command Confirmed SP1\r\n", 23);
//		if (!ucSP_SendCommand(SP2, 8, ucPayload))
//			vSERIAL_sout("Command Confirmed SP2\r\n", 23);
//		if (!ucSP_SendCommand(SP3, 8, ucPayload))
//			vSERIAL_sout("Command Confirmed SP3\r\n", 23);
//		if (!ucSP_SendCommand(SP4, 8, ucPayload))
//			vSERIAL_sout("Command Confirmed SP4\r\n", 23);

//		vSERIAL_sout(" Waiting\r\n\n", 11);
//		i = 2;
//		while (i>0)
//		{
//			i--;
//			__delay_cycles(16000000);
//		}

//		if (!ucSP_RequestData(SP1))
//			vSERIAL_sout("Received Report SP1\r\n", 21);
//		if (!ucSP_RequestData(SP2))
//			vSERIAL_sout("Received Report SP2\r\n", 21);
//		if (!ucSP_RequestData(SP3))
//			vSERIAL_sout("Received Report SP3\r\n", 21);
//		if (!ucSP_RequestData(SP4))
//			vSERIAL_sout("Received Report SP4\r\n", 21);


//		// Read the SP buffer into the the data message structure
//		ucSP_GrabMessageFromBuffer(&S_DataMsg);


//		vSP_TurnOff(SP1);
//		vSP_TurnOff(SP2);
//		vSP_TurnOff(SP3);
//		vSP_TurnOff(SP4);

//		__delay_cycles(16000000);
//	}


	/** Test Battery pack **/
//	while (1) {
//		if (uiMISC_doCompensatedBattRead() < 4500) {
//			LED_PORT |= RED_LED_BIT;
//			LED_PORT &= ~GRN_LED_BIT;
//		}
//		else {
//			LED_PORT |= GRN_LED_BIT;
//			LED_PORT &= ~RED_LED_BIT;
//		}
//	}

//		vADF7020_RunTimeDiagnostic();
//	}
}

/* -----------------------  END OF MODULE  ------------------------------- */
