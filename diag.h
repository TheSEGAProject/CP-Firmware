
/***************************  DIAG.H  ****************************************
*
* DIAG header file
*
*
* V1.00 12/23/2003 wzr
*	started
*
******************************************************************************/

#ifndef DIAG_H_INCLUDED
	#define DIAG_H_INCLUDED

//#define COMPILE_DEFAULT_WIZ			YES //main wizard code

//#define COMPILE_FOR_WIZ2_DUKETEST		YES //main wizard code
//#define COMPILE_TEST_YES_V_TREE_HUMBOLDT YES //main wizard code yes vaisala
//#define COMPILE_FOR_MARK1234			YES //main wizard code

//#define BRAINX_DIAG_ENA 			YES //gross component checkout for brain
//#define RADIOX1_BOARD_DIAG_ENA  	YES //PART-1 RADIO gross component chk for one being tested
//#define RADIOX2_BOARD_DIAG_ENA  	YES //PART-2 RADIO gross component chk for one that is good

//#define FULLDIAG_ENA				YES //full blown system diagnostic

//#define KEY_ROUTINE_ENA 			YES //keybrd routine functions

//#define NOTHING_SO_SD_CAN_RUN			YES //just brings up the brain so SD can work


/*---------------------------------------*/


//#define DISPATCHER_TIMING_DIAG_ENA  YES  //checks out the dispatcher

//#define AD0_DIAG_ENA  YES		//toggle batt sense bit
//#define AD1_DIAG_ENA  YES		//continuous read of A/D (SD board diag)
//#define AD2_DIAG_ENA  YES		//diff of single A/D read and multiple
//#define AD3_DIAG_ENA  YES		//test soil moisture probe

//#define AMP1_REC_CUR_DIAG_ENA		YES
//#define AMP2_XMIT_CUR_DIAG_ENA	YES
//#define AMP3_RUN_CUR_DIAG_ENA		YES
//#define AMP4_SLEEP_CUR_DIAG_ENA	YES	//pure sleep without clk tick
//#define AMP5_DIAG_ENA	   			YES	//sleep function with clk tick
//#define AMP6_DIAG_ENA	   			YES	//hibernate function

//#define BATT1_DIAG_ENA  			YES //Continuous test of vACTION_do_Batt_Sense()

//#define BIGSUB_DIAG_ENA  YES 	//make sure bigsub works

//#define BUTTON0_DIAG_ENA YES	//show button in LED
//#define BUTTON1_DIAG_ENA YES	//MEASURE A BUTTON PUSH TIME
//#define BUTTON2_DIAG_ENA YES	//Debounce the button
//#define BUTTON3_DIAG_ENA YES	//Button Wakeup Diagnostic
//#define BUTTON4_DIAG_ENA YES	//Button spurious push counter
//#define BUTTON5_DIAG_ENA YES	//Button interrupt counter
//#define BUTTON6_DIAG_ENA YES	//test clk button interaction

//#define BUZ1_DIAG_ENA YES
//#define BUZ2_DIAG_ENA YES
//#define BUZ3_DIAG_ENA YES		//plays a tune
//#define BUZ4_DIAG_ENA YES		//Tests the Tune playing routine

//#define CLK1_TIC_DIAG_ENA YES	//check if the clk rolls over properly
//#define CLK2_TIC_DIAG_ENA YES	//check if the clk is ticking
//#define CLK3_TIC_DIAG_ENA YES	//check system time routines
//#define CLK4_TIC_DIAG_ENA YES	//T1 tic check
//#define CLK5_DIAG_ENA		YES	//Check if T1 tics while radio is receiving
//#define CLK6_DIAG_ENA   	YES	//Check if T1 tics while radio is transmitting
//#define CLK7_DIAG_ENA   	YES	//Check T0 timer routines
//#define CLK8_DIAG_ENA   	YES	//Check Date routines

//#define COMM1_DIAG_ENA	YES
//#define COMM2_DIAG_ENA	YES
//#define COMM3_DIAG_ENA	YES
//#define COMM4_DIAG_ENA	YES //test the Serial int routines
//#define COMM5_DIAG_ENA	YES //output the entire char set 0-255
//#define COMM6_DIAG_ENA	YES //test the rad40 conversion routines

//#define CRC1_DIAG_ENA		YES //check crc for snd and receive

//#define ESPORT0_DIAG_ENA YES	//ESPORT Tx a continuous char
//#define ESPORT1_DIAG_ENA YES	//input ESPORT port output serial port
//#define ESPORT2_DIAG_ENA YES	//input line from ESPORT, output line later
//#define ESPORT3_DIAG_ENA YES	//Test a vaisala command
//#define ESPORT4_DIAG_ENA YES	//Test ESPORT bin
//#define ESPORT5_DIAG_ENA YES	//Test ESPORT vaisala bin

//#define FLASH1_DIAG_ENA   YES	//reads the status byte
//#define FLASH2_DIAG_ENA   YES	//dumps first 20 byte of page
//#define FLASH3_DIAG_ENA   YES	//writes a page then reads page and dumps it
//#define FLASH4_DIAG_ENA   YES	//write a page using word writes and dump it
//#define FLASH5_DIAG_ENA   YES	//auto rewite the entire disk
//#define FLASH6_DIAG_ENA 	YES	//flash buffer-only page content test
//#define FLASH7_DIAG_ENA 	YES	//dumps out 10 stored messages
//#define FLASH8_DIAG_ENA 	YES	//power test of flash
//#define FLASH9_DIAG_ENA 	YES	//write 10 messages to flash, set a port before, reset after

//#define FRAM1_DIAG_ENA	YES	//test of lo level functions
//#define FRAM2_DIAG_ENA	YES //full mem test of FRAM
//#define FRAM3_DIAG_ENA	YES //test of a single location
//#define FRAM4_DIAG_ENA	YES //test of L2FRAM functions
//#define FRAM5_DIAG_ENA	YES //hunts for the First byte bug
//#define FRAM6_DIAG_ENA	YES //looks for transfer bugs
//#define FRAM7_DIAG_ENA	YES //looks for rollover bugs

//#define KEY1_DIAG_ENA		YES	//check the cmd module routines
//#define KEY2_DIAG_ENA		YES	//check timing on SERIAL_timedBin()
//#define KEY3_DIAG_ENA		YES	//check upload routines

//#define LED1_DIAG_ENA		YES	//test the LED using raw calls
//#define LED2_DIAG_ENA		YES	//test the LED using sys calls

//#define LIGHT1_DIAG_ENA	YES


#ifdef LNKBLK1_DIAG_ENA
 #undef  LNKBLK1_DIAG_ENA
 #define LNKBLK1_DIAG_ENA	YES
#else
// #define LNKBLK1_DIAG_ENA	YES //test the base routines for LnkBlk Tbl
#endif

#ifdef LNKBLK2_DIAG_ENA
 #undef  LNKBLK2_DIAG_ENA
 #define LNKBLK2_DIAG_ENA	YES
#else
// #define LNKBLK2_DIAG_ENA	YES //test the higher routines
#endif

#ifdef LNKBLK3_DIAG_ENA
 #undef  LNKBLK3_DIAG_ENA
 #define LNKBLK3_DIAG_ENA	YES
#else
// #define LNKBLK3_DIAG_ENA	YES //test compute Lnk Req from Sys Load
#endif

#ifdef LNKBLK4_DIAG_ENA
 #undef  LNKBLK4_DIAG_ENA
 #define LNKBLK4_DIAG_ENA	YES
#else
// #define LNKBLK4_DIAG_ENA	YES //test compute Lnk Req from Sys Load
#endif






//#define LINKUP_DCNT_1_ENABLED YES	//test the linkup downcounter calculator

//#define MISC1_DIAG_ENA	YES		//Test the MISC functions

//#define MSG1_DIAG_ENA	YES	//Test the SHOW msg buffer routine
//#define MSG2_DIAG_ENA	YES	//Store 10 msgs in flash and SRAM, set LED before, clr LED after
//#define MSG3_DIAG_ENA	YES	//Test report routines

//#define NUMCMD1_DIAG_ENA	YES //Test the NUMCMD routines

//#define NUMERIC1_OUTPUT_DIAG_ENA YES

//#define OWIRE1_DIAG_ENA	YES //onewire port detection diagnostic
//#define OWIRE2_DIAG_ENA	YES //ECLK read check
//#define OWIRE3_DIAG_ENA	YES //Look for ECLK bug
//#define OWIRE4_DIAG_ENA	YES //ECLK read check 2
//#define OWIRE5_DIAG_ENA 	YES //ECLK measure current usage.
//#define OWIRE6_DIAG_ENA 	YES //ECLK debug button switch to output

//#define PICK1_DIAG_ENA  	YES //Check the PICK module show routines

//#define PWR1_DIAG_ENA		YES	//continuously read battery voltage
//#define PWR2_DIAG_ENA		YES	//Poll the PWR SOURCE bit and report any changes
//#define PWR3_DIAG_ENA		YES	//Routine to flip the SD power on and off

//#define RADIO0_DIAG_ENA	YES	//bit wiggles
//#define RADIO1_DIAG_ENA   YES	//terminal test
//#define RADIO2_DIAG_ENA   YES //transmit the startup msg repeatedly
//#define RADIO3_DIAG_ENA   YES //transmit power test  
//#define RADIO4_DIAG_ENA   YES //radio board detection
//#define RADIO5_DIAG_ENA   YES //put out radio pulse to measure RSSI on a recieive
//#define RADIO6_DIAG_ENA   YES //measure radio5 pulse RSSI
//#define RADIO7_DIAG_ENA   YES //check radio for xmit/rec switching
//#define RADIO8_DIAG_ENA   YES //check random number generated by radio
//#define RADIO9_DIAG_ENA 	YES //check ping radio
//#define RADIO10_DIAG_ENA 	YES //check the frequency calculator
//#define RADIO11_DIAG_ENA 	YES //check transmit frequency
//#define RADIO12_DIAG_ENA 	YES //send a continuous 1 bit out the radio

//#define RDC4_DIAG_ENA		YES //Test RDC4
//#define ROM2_DIAG_ENA		YES //Test ROM2

//#define ROLE1_DIAG_ENA	YES //Test ROLE OPTION ROUTINES

//#define SDC4_DIAG_ENA		YES //Test SDC4
//#define SOM2_DIAG_ENA		YES //Test SOM2

//#define SHOW_DEFINES_DIAG_ENA  YES

//#define SD1_DIAG_ENA		YES	//read the sensor data
//#define SD2_DIAG_ENA		YES	//read the sensor data using a single call
//#define SD3_DIAG_ENA		YES	//read light sensors 1,2,3,4
//#define SD4_DIAG_ENA		YES	//test the SD wakeup function

//#define SDLOD1_DIAG_ENA 	YES //Test SDLOD read routines
//#define SDLOD2_DIAG_ENA 	YES //Test SDLOD erase & write routines
//#define SDLOD3_DIAG_ENA 	YES //Test SDLOD Higher functions
//#define SDLOD4_DIAG_ENA 	YES //Test SDLOD burn power usage
//#define SDLOD5_DIAG_ENA 	YES //Test SDLOD Config write

//#define SLEEP1_DIAG_ENA YES //check button push during sleep
//#define SLEEP2_DIAG_ENA YES //check hibernation
//#define SLEEP3_DIAG_ENA YES //check T3 timer sleep wakeup

//#define SOIL1_DIAG_ENA YES //continuous read of soil moisture sensor

//#define SRAM0_DIAG_ENA YES //low level data check
//#define SRAM1_DIAG_ENA YES //sram memory diagnostic
//#define SRAM2_DIAG_ENA YES //sram 2nd memory diagnostic
//#define SRAM3_DIAG_ENA YES //msg Q routine diagnostic
//#define SRAM4_DIAG_ENA YES //sram 3rd memory diagnostic
//#define SRAM5_DIAG_ENA YES //write 10 messages to SRAM, set a port before, reset after

//#define STREAM1_DIAG_ENA YES //send a stream to the console

//#define TBL1_DIAG_ENA YES	//RTS table diagnostic

//#define TC1_DIAG_ENA YES
//#define TC2_DIAG_ENA YES	//Mimic TC_12 thermo read
//#define TC3_DIAG_ENA YES	//Bit follower for SPI_ready
//#define TC4_DIAG_ENA YES	//test the show_temp routine

//#define T3TIMER1_DIAG_ENA YES	//Test the Slot end timer

//#define VOFFSET1_DIAG_ENA YES
//#define VOFFSET2_DIAG_ENA YES

//#define VS1_DIAG_ENA			YES	//Loop through all Vaisala reads
//#define VS2_DIAG_ENA			YES	//Test wind dir on Vaisala


#ifdef COMPILE_DEFAULT_WIZ
	#define RUN_CODE_NAME							"Wiz"

	#define USE_ESPORT_OUTPUT						NO

//	#define ENTER_SCHED_REPORT						YES	
//	#define ENTER_SCHED_BATT_SENSE					YES	
//	#define ENTER_SCHED_TC_12						YES	
//	#define ENTER_SCHED_TC_34						YES	
//	#define ENTER_SCHED_LT_12						NO
//	#define ENTER_SCHED_LT_34						NO
//	#define ENTER_SCHED_LT_1234						YES
//	#define ENTER_SCHED_SL_12						YES	
//	#define ENTER_SCHED_VS_AVE_WSPEED_DIR			NO
//	#define ENTER_SCHED_VS_AIR_PRESS_TEMP			NO
//	#define ENTER_SCHED_VS_REL_HUMID_RAIN			NO
//	#define ENTER_SCHED_DO_SDC4						YES
//	#define ENTER_SCHED_DO_RDC4						YES
//	#define ENTER_SCHED_SAP_12						NO
//	#define ENTER_SCHED_SAP_34						NO	

#endif /* END: COMPILE_DEFAULT_WIZ */





/*--------------------  DIAGNOSTICS FROM HERE DOWN  --------------------------
* NOTE: These diagnostics are roughly in alphabetic order.  If you find one
* 		out of order or you are adding one Please spend a moment to order them
*----------------------------------------------------------------------------*/

#ifdef AD0_DIAG_ENA 

	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES
	#define DIAG_FIX_SOIL_RDINGS				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif		
#ifdef AD1_DIAG_ENA 
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES
	#define DIAG_FIX_SOIL_RDINGS				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef AD2_DIAG_ENA 
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES
	#define DIAG_FIX_SOIL_RDINGS				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef AD3_DIAG_ENA 
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES
	#define DIAG_FIX_SOIL_RDINGS				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif



#ifdef AMP1_REC_CUR_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef AMP2_XMIT_CUR_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef AMP3_RUN_CUR_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef AMP4_SLEEP_CUR_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef AMP5_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef AMP6_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif


#ifdef BATT1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_RPT_BATT_VOLTS_TO_RDIO			NO
	#define DIAG_RPT_BATT_VOLTS_TO_FLSH			YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS				YES

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  						NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP			NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif //BATT1_DIAG_ENA



#ifdef BIGSUB_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif




#ifdef BRAINX_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO

	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: BRAINX_DIAG_ENA */



#ifdef BUTTON0_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef BUTTON1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif		
#ifdef BUTTON2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef BUTTON3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef BUTTON4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS					YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef BUTTON5_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif		
#ifdef BUTTON6_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif




#ifdef BUZ1_DIAG_ENA 
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef BUZ2_DIAG_ENA 
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef BUZ3_DIAG_ENA 
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef BUZ4_DIAG_ENA 
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif






#ifdef DISPATCHER_TIMING_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif



/*
#ifdef ESPORT0_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //ESPORT0_DIAG_ENA
#ifdef ESPORT1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //ESPORT1_DIAG_ENA
#ifdef ESPORT2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //ESPORT2_DIAG_ENA
#ifdef ESPORT3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //ESPORT3_DIAG_ENA
*/

#ifdef KEY_ROUTINE_ENA						//keyboard routine functions
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME							"Keyboard"


	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif //KEY_ROUTINE_ENA


#ifdef OWIRE1_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif
#ifdef OWIRE2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif
#ifdef OWIRE3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif  /* END: OWIRE3_DIAG_ENA */
#ifdef OWIRE4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif /* END: OWIRE4_DIAG_ENA */
#ifdef OWIRE5_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif /* END: OWIRE5_DIAG_ENA */
#ifdef OWIRE6_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif /* END: OWIRE6_DIAG_ENA */


#ifdef PICK1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif /* END: PICK1_DIAG_ENA */


#ifdef PWR1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif
#ifdef PWR2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef PWR3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* PWR3_DIAG_ENA */





#ifdef CLK1_TIC_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* CLK1_TIC_DIAG_ENA */
#ifdef CLK2_TIC_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* CLK2_TIC_DIAG_ENA */
#ifdef CLK3_TIC_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef CLK4_TIC_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef CLK5_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef CLK6_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef CLK7_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: CLK7_DIAG_ENA */
#ifdef CLK8_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: CLK8_DIAG_ENA */



#ifdef COMM1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef COMM2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef COMM3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef COMM4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //COMM4_DIAG_ENA
#ifdef COMM5_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif //COMM5_DIAG_ENA
#ifdef COMM6_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif //COMM6_DIAG_ENA



#ifdef CRC1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif



#ifdef NOTHING_SO_SD_CAN_RUN
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_NOTHING							YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif




#ifdef LED1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef LED2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif



#ifdef SHOW_DEFINES_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif



#ifdef SD1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SD2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SD3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP					YES

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif //SD3_DIAG_ENA
#ifdef SD4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: SD4_DIAG_ENA */


#ifdef SDLOD1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME						"Sdlod1"



	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: SDLOD1_DIAG_ENA */

#ifdef SDLOD2_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME						"Sdlod2"

	#define USE_ESPORT_OUTPUT					YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: SDLOD2_DIAG_ENA */
#ifdef SDLOD3_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME						"SDLOD3"

	#define USE_ESPORT_OUTPUT					YES

	#define ENABLE_DIAG_SDLOD_SUBROUTINES		YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: SDLOD3_DIAG_ENA */
#ifdef SDLOD4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME						"SDLOD4"

	#define USE_ESPORT_OUTPUT					NO
	#define ENABLE_DIAG_SDLOD_SUBROUTINES		NO

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: SDLOD4_DIAG_ENA */
#ifdef SDLOD5_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME						"SDLOD5"

	#define USE_ESPORT_OUTPUT					NO
	#define ENABLE_DIAG_SDLOD_SUBROUTINES		NO

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: SDLOD5_DIAG_ENA */



#ifdef SLEEP1_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SLEEP2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SLEEP3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif	//SLEEP3_DIAG_ENA



#ifdef SOIL1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO
	#define DIAG_FIX_SOIL_RDINGS				YES

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  						NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP			NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif	//SOIL1_DIAG_ENA



#ifdef SRAM0_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SRAM1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SRAM2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SRAM3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SRAM4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SRAM5_DIAG_ENA 	//Write 10 messages to SRAM, set a port before reset after
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: SRAM5_DIAG_ENA */


#ifdef STREAM1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif


#ifdef FLASH1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FLASH2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FLASH3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FLASH4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FLASH5_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FLASH6_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: FLASH6_DIAG_ENA */
#ifdef FLASH7_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif  /* END: FLASH7_DIAG_ENA */
#ifdef FLASH8_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif  /* END: FLASH8_DIAG_ENA */
#ifdef FLASH9_DIAG_ENA 	//Write 10 messages to flash, set a port before reset after
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO

	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: FLASH9_DIAG_ENA */




#ifdef FRAM1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FRAM2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FRAM3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FRAM4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FRAM5_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FRAM6_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef FRAM7_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif


#ifdef KEY1_DIAG_ENA						//check CMD module functions
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME							"CMD module diag"

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif

#ifdef KEY2_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME							"Timed input test"

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP					NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif  //KEY2_DIAG_ENA

#ifdef KEY3_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME						"Key3"

	#define FAKE_UPLOAD_MSGS					YES

	#define DIAG_RPT_SOM2_LNKS_TO_RDIO			YES
	#define DIAG_RPT_SOM2_LNKS_TO_FLSH			YES

	#define DIAG_RPT_SOM2_FAILS_TO_RDIO			YES
	#define DIAG_RPT_SOM2_FAILS_TO_FLSH			YES

	#define DIAG_RPT_ROM2_LNKS_TO_RDIO			YES
	#define DIAG_RPT_ROM2_LNKS_TO_FLSH			YES

	#define DIAG_RPT_ROM2_FAILS_TO_RDIO			YES
	#define DIAG_RPT_ROM2_FAILS_TO_FLSH			YES

	#define DIAG_RPT_SYS_RSTRTS_TO_RDIO			YES
	#define DIAG_RPT_SYS_RSTRTS_TO_FLSH			YES

	#define DIAG_RPT_BATT_DEAD_RSTRTS_TO_RDIO	NO
	#define DIAG_RPT_BATT_DEAD_RSTRTS_TO_FLSH	YES

	#define DIAG_RPT_BATT_VOLTS_TO_RDIO			YES
	#define DIAG_RPT_BATT_VOLTS_TO_FLSH			YES

	#define DIAG_RPT_TIME_CHGS_TO_RDIO			NO
	#define DIAG_RPT_TIME_CHGS_TO_FLSH			YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED			YES
	#define DIAG_SND_RDIO_STUP_MSG				YES
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		YES

	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK					YES

	#define DIAG_BUZ_ON							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			YES
	#define DIAG_CHK_FRAM_FMT_ON_STUP			YES
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO

	#define DIAG_SHOW_STS_ON_LFACTRS			YES
	#define DIAG_SHOW_LNK_SCHED					YES
#endif  //KEY3_DIAG_ENA


#ifdef LIGHT1_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  						NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP			NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif



#ifdef LINKUP_DCNT_1_ENABLED
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define RUN_CODE_NAME						"Linkup Dcnt Diag"

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  						NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP			NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif //END: LINKUP_DCNT_1_ENABLED


#ifdef MISC1_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  						NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP			NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif

#ifdef MSG1_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  						NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP			NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif /* END: MSG1_DIAG_ENA */
#ifdef MSG2_DIAG_ENA 	//Write 10 msgs to flash & SRAM, set LED before, clr LED after
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK					NO

	#define DIAG_BUZ_ON  						YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP			NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif /* END: MSG2_DIAG_ENA */
#ifdef MSG3_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK					NO

	#define DIAG_BUZ_ON  						YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP			NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif /* END: MSG3_DIAG_ENA */


#ifdef NUMCMD1_DIAG_ENA
	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO

	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif /* END: NUMCMD1_DIAG_ENA */


#ifdef NUMERIC1_OUTPUT_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif



#ifdef RADIO0_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED 		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		YES
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		YES
	#define DIAG_SND_RDIO_STUP_MSG				YES
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP		NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO5_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD					NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO6_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO7_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO8_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO9_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO10_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO11_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef RADIO12_DIAG_ENA		//send a continuous 1 bit out the radio
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif  //RADIO12_DIAG_ENA





#ifdef RADIOX1_BOARD_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif  //END: RADIOX1_BOARD_DIAG_ENA
#ifdef RADIOX2_BOARD_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO

	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //END: RADIOX2_BOARD_DIAG_ENA



#ifdef RDC4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				YES

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO

	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO

#endif  //END: RDC4_DIAG_ENA

#ifdef ROM2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				YES
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //END: ROM2_DIAG_ENA
#ifdef ROLE1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO

	#define DIAG_FIX_SOIL_RDINGS				NO

	#define DIAG_SHOW_STS_ON_LFACTRS				YES
	#define DIAG_SHOW_LNK_SCHED				YES

#endif //END: ROLE1_DIAG_ENA




#ifdef SDC4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				YES
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef SOM2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				YES
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				YES
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							YES

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif



#ifdef TBL1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif





#ifdef TC1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef TC2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef TC3_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef TC4_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //TC4_DIAG_ENA



#ifdef T3TIMER1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				NO
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif





#ifdef VOFFSET1_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif
#ifdef VOFFSET2_DIAG_ENA
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO
	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				NO

	#define DIAG_DO_HBRNAT_CHK				NO
	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif


#ifdef VS1_DIAG_ENA 	//Loop through all Vaisala reads
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD					NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED			NO
	#define DIAG_SND_RDIO_STUP_MSG					NO
	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT			NO

	#define DIAG_SD_BD_IS_ATTCHD					YES
	#define DIAG_CHK_FOR_SD_ON_STUP					YES

	#define DIAG_DO_HBRNAT_CHK					NO
	#define DIAG_BUZ_ON  								NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP				NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP		NO
	#define DIAG_CHK_FOR_FLSH_ON_STUP			NO
#endif //VS1_DIAG_ENA
#ifdef VS2_DIAG_ENA 	//Test wind direction
 	#define THIS_IS_A_DIAGNOSTIC				YES

	#define DIAG_CHK_FOR_RDIO_BD				NO
	#define DIAG_USE_RDIO_FOR_RAND_SEED		NO
	#define DIAG_SND_RDIO_STUP_MSG				NO

	#define DIAG_ALLOW_BRAIN_STUP_HBRNAT		NO

	#define DIAG_SD_BD_IS_ATTCHD				YES
	#define DIAG_CHK_FOR_SD_ON_STUP				YES

	#define DIAG_BUZ_ON  							NO

	#define DIAG_CHK_FOR_FRAM_ON_STUP			NO
	#define DIAG_CHK_FRAM_FMT_ON_STUP	NO

	#define DIAG_CHK_FOR_FLSH_ON_STUP		NO
#endif //VS2_DIAG_ENA



//#ifndef RUN_CODE_NAME
// #define RUN_CODE_NAME							"No Name"
//#endif


#ifdef INC_ESPORT				//defined on Cmd line
#ifdef USE_ESPORT_OUTPUT		//defined in specific requirement above
#if (USE_ESPORT_OUTPUT == YES)
  #define ESPORT_ENABLED 1
#endif
#endif
#endif



/* ROUTINES */

void vDIAG_run_diagnostics(
		void
		);




#endif /* DIAG_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
