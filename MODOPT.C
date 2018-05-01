

/**************************  MODOPT.C  ****************************************
*
* MODOPT deals with the option bits that determine the "persona" that
* the wisard can execute.  The user can choose the options that he wishes the
* wisard to have, and those options are then copied to FRAM, where they are
* loaded on wisard startup.
*
* NOTE: Each option set is simply a set of runtime parameters that determine
*		the	wisard runtime behavior.
*
* NOTE: This module has 3 functional parts:
*			The first  are the routines to access the ROM  option bits.
*			The second are the routines to access the FRAM option bits.
*			The third  are the routines to access the RAM  option bits.
*
* NOTE:	RAM option bits are simply a copy of the FRAM option bits.  They
*		exist because accessing ram avoids tying up the SPI bus to get
*		the FRAM option bits.
*
* V1.00 12/01/2007 wzr
*		Started
*
******************************************************************************/


/*lint -e526 */  	/* function not defined */
/*lint -e657 */  	/* unusual (nonportable) anonymous struct or union */
//*lint -e714 */ 	/* symbol not referenced */
/*lint -e750 */  	/* local macro not referenced */
/*lint -e754 */  	/* local structure member not referenced */
/*lint -e755 */  	/* global macro not referenced */
//*lint -e757 */  	/* global declarator not referenced */
//*lint -e752 */  	/* local declarator not referenced */
//*lint -e758 */  	/* global union not referenced */
//*lint -e768 */  	/* global struct member not referenced */


#include "std.h"			//standard defines
#include "serial.h" 		//serial IO port stuff
#include "key.h"			//top level key command routines
#include "numcmd.h"			//numeric command routines
#include "MODOPT.h"			//Modify Options routines
#include "hal/config.h" 	//system configuration description file
#include "drivers/SP.h"
#include "mem_mod/l2fram.h"			//Level 2 Ferro Ram routines


/*----------------------  EXTERNS  ------------------------------------------*/

extern uchar ucaGLOB_optionBytes[OPTION_BYTE_COUNT];




/*----------------------  DEFINES HERE  -------------------------------------*/


/* OPTION BIT VALUES BYTE 0 OF THE OPTION ARRAY */

#define WIZ_SENDS_YES							0x01	// 0000 0001
#define WIZ_SENDS_NO							0x00	// 0000 0000
#define WIZ_SENDS_MSK							0x01	// 0000 0001
													
#define WIZ_RECEIVES_YES						0x02	// 0000 0010
#define WIZ_RECEIVES_NO							0x00	// 0000 0000
#define WIZ_RECEIVES_MSK						0x02	// 0000 0010
													
#define WIZ_SAMPLES_YES							0x04	// 0000 0100
#define WIZ_SAMPLES_NO							0x00	// 0000 0000
#define WIZ_SAMPLES_MSK							0x04	// 0000 0100
													
#define WIZ_NIY_YES								0x08	// 0000 1000 //not implemented yet
#define WIZ_NIY_NO 								0x00	// 0000 0000
#define WIZ_NIY_MSK								0x08	// 0000 1000

#define STRMING_YES								0x10	// 0001 0000
#define STRMING_NO								0x00	// 0000 0000
#define STRMING_MSK								0x10	// 0001 0000
													
#define SHOW_SYSTAT_YES							0x20	// 0010 0000
#define SHOW_SYSTAT_NO							0x00	// 0000 0000
#define SHOW_SYSTAT_MSK							0x20	// 0010 0000
													
#define CHK_FOR_RDIO_BD_YES						0x40	// 0100 0000
#define CHK_FOR_RDIO_BD_NO 						0x00	// 0000 0000
#define CHK_FOR_RDIO_BD_MSK						0x40	// 0100 0000
													
#define USE_RDIO_FOR_RAND_SEED_YES				0x80	// 1000 0000
#define USE_RDIO_FOR_RAND_SEED_NO 				0x00	// 0000 0000
#define USE_RDIO_FOR_RAND_SEED_MSK				0x80	// 1000 0000



/* OPTION BIT VALUES BYTE 1 OF THE OPTION ARRAY */
													
#define SND_RDIO_STUP_MSG_YES					0x01	// 0000 0001
#define SND_RDIO_STUP_MSG_NO 					0x00	// 0000 0000
#define SND_RDIO_STUP_MSG_MSK					0x01	// 0000 0001
													
#define ALLOW_BRAIN_STUP_HBRNAT_YES				0x02	// 0000 0010
#define ALLOW_BRAIN_STUP_HBRNAT_NO 				0x00	// 0000 0000
#define ALLOW_BRAIN_STUP_HBRNAT_MSK				0x02	// 0000 0010
												 
#define SP_BDS_ARE_ATTCHD_YES					0x04	// 0000 0100
#define SP_BDS_ARE_ATTCHD_NO 					0x00	// 0000 0000
#define SP_BDS_ARE_ATTCHD_MSK					0x04	// 0000 0100
													
#define CHK_FOR_SPS_ON_STUP_YES					0x08	// 0000 1000
#define CHK_FOR_SPS_ON_STUP_NO					0x00	// 0000 0000
#define CHK_FOR_SPS_ON_STUP_MSK					0x08	// 0000 1000
												
#define FIX_SOIL_RDING_YES						0x10	// 0001 0000
#define FIX_SOIL_RDING_NO						0x00	// 0000 0000
#define FIX_SOIL_RDING_MSK						0x10	// 0001 0000
													
#define DO_HBRNAT_CHK_YES						0x20	// 0010 0000
#define DO_HBRNAT_CHK_NO						0x00	// 0000 0000
#define DO_HBRNAT_CHK_MSK						0x20	// 0010 0000
													
#define BUZ_ON_YES								0x40	// 0100 0000
#define BUZ_ON_NO 								0x00	// 0000 0000
#define BUZ_ON_MSK								0x40	// 0100 0000
													
#define CHK_FOR_FRAM_ON_STUP_YES				0x80	// 1000 0000
#define CHK_FOR_FRAM_ON_STUP_NO 				0x00	// 0000 0000
#define CHK_FOR_FRAM_ON_STUP_MSK				0x80	// 1000 0000



/* OPTION BIT VALUES BYTE 2 OF THE OPTION ARRAY */
													
#define CHK_FRAM_FMT_ON_STUP_YES				0x01	// 0000 0001
#define CHK_FRAM_FMT_ON_STUP_NO 				0x00	// 0000 0000
#define CHK_FRAM_FMT_ON_STUP_MSK				0x01	// 0000 0001
													
#define CHK_FOR_FLSH_ON_STUP_YES				0x02	// 0000 0010
#define CHK_FOR_FLSH_ON_STUP_NO 				0x00	// 0000 0000
#define CHK_FOR_FLSH_ON_STUP_MSK				0x02	// 0000 0010
													
#define SHOW_STS_ON_LFACTRS_YES					0x04	// 0000 0100
#define SHOW_STS_ON_LFACTRS_NO 					0x00	// 0000 0000
#define SHOW_STS_ON_LFACTRS_MSK					0x04	// 0000 0100
													
#define SHOW_LNK_SCHED_YES						0x08	// 0000 1000
#define SHOW_LNK_SCHED_NO						0x00	// 0000 0000
#define SHOW_LNK_SCHED_MSK						0x08	// 0000 1000
												
#define UNUSED_1_YES							0x10	// 0001 0000
#define UNUSED_1_NO								0x00	// 0000 0000
#define UNUSED_1_MSK							0x10	// 0001 0000
													
#define UNUSED_2_YES							0x20	// 0010 0000
#define UNUSED_2_NO								0x00	// 0000 0000
#define UNUSED_2_MSK							0x20	// 0010 0000
													
#define UNUSED_3_YES							0x40	// 0100 0000
#define UNUSED_3_NO 							0x00	// 0000 0000
#define UNUSED_3_MSK							0x40	// 0100 0000
													
#define UNUSED_4_YES							0x80	// 1000 0000
#define UNUSED_4_NO 							0x00	// 0000 0000
#define UNUSED_4_MSK							0x80	// 1000 0000




/* OPTION BIT VALUES FOR BYTE 3 OF THE OPTION ARRAY */

#define RPT_SOM2_LNKS_TO_RDIO_YES			0x01	// 0000 0001
#define RPT_SOM2_LNKS_TO_RDIO_NO			0x00	// 0000 0000
#define RPT_SOM2_LNKS_TO_RDIO_MSK			0x01	// 0000 0001

#define RPT_SOM2_LNKS_TO_FLSH_YES			0x02	// 0000 0010
#define RPT_SOM2_LNKS_TO_FLSH_NO			0x00	// 0000 0000
#define RPT_SOM2_LNKS_TO_FLSH_MSK			0x02	// 0000 0010

#define RPT_SOM2_FAILS_TO_RDIO_YES			0x04	// 0000 0100
#define RPT_SOM2_FAILS_TO_RDIO_NO			0x00	// 0000 0000
#define RPT_SOM2_FAILS_TO_RDIO_MSK			0x04	// 0000 0100

#define RPT_SOM2_FAILS_TO_FLSH_YES			0x08	// 0000 1000
#define RPT_SOM2_FAILS_TO_FLSH_NO			0x00	// 0000 0000
#define RPT_SOM2_FAILS_TO_FLSH_MSK			0x08	// 0000 1000

#define RPT_ROM2_LNKS_TO_RDIO_YES			0x10	// 0001 0000
#define RPT_ROM2_LNKS_TO_RDIO_NO			0x00	// 0000 0000
#define RPT_ROM2_LNKS_TO_RDIO_MSK			0x10	// 0001 0000

#define RPT_ROM2_LNKS_TO_FLSH_YES			0x20	// 0010 0000
#define RPT_ROM2_LNKS_TO_FLSH_NO			0x00	// 0000 0000
#define RPT_ROM2_LNKS_TO_FLSH_MSK			0x20	// 0010 0000

#define RPT_ROM2_FAILS_TO_RDIO_YES			0x40	// 0100 0000
#define RPT_ROM2_FAILS_TO_RDIO_NO			0x00	// 0000 0000
#define RPT_ROM2_FAILS_TO_RDIO_MSK			0x40	// 0100 0000

#define RPT_ROM2_FAILS_TO_FLSH_YES			0x80	// 1000 0000
#define RPT_ROM2_FAILS_TO_FLSH_NO			0x00	// 0000 0000
#define RPT_ROM2_FAILS_TO_FLSH_MSK			0x80	// 1000 0000




/* OPTION BIT VALUES FOR BYTE 4 OF THE OPTION ARRAY */

#define RPT_SYS_RSTRTS_TO_RDIO_YES			0x01	// 0000 0001
#define RPT_SYS_RSTRTS_TO_RDIO_NO			0x00	// 0000 0000
#define RPT_SYS_RSTRTS_TO_RDIO_MSK			0x01	// 0000 0001

#define RPT_SYS_RSTRTS_TO_FLSH_YES			0x02	// 0000 0010
#define RPT_SYS_RSTRTS_TO_FLSH_NO			0x00	// 0000 0000
#define RPT_SYS_RSTRTS_TO_FLSH_MSK			0x02	// 0000 0010

#define RPT_BATT_DEAD_RSTRT_TO_RDIO_YES		0x04	// 0000 0100
#define RPT_BATT_DEAD_RSTRT_TO_RDIO_NO		0x00	// 0000 0000
#define RPT_BATT_DEAD_RSTRT_TO_RDIO_MSK		0x04	// 0000 0100

#define RPT_BATT_DEAD_RSTRT_TO_FLSH_YES		0x08	// 0000 1000
#define RPT_BATT_DEAD_RSTRT_TO_FLSH_NO		0x00	// 0000 0000
#define RPT_BATT_DEAD_RSTRT_TO_FLSH_MSK		0x08	// 0000 1000

#define RPT_BATT_VOLTS_TO_RDIO_YES			0x10	// 0001 0000
#define RPT_BATT_VOLTS_TO_RDIO_NO			0x00	// 0000 0000
#define RPT_BATT_VOLTS_TO_RDIO_MSK			0x10	// 0001 0000

#define RPT_BATT_VOLTS_TO_FLSH_YES			0x20	// 0010 0000
#define RPT_BATT_VOLTS_TO_FLSH_NO			0x00	// 0000 0000
#define RPT_BATT_VOLTS_TO_FLSH_MSK			0x20	// 0010 0000

#define RPT_TIM_CHGS_TO_RDIO_YES			0x40	// 0100 0000
#define RPT_TIM_CHGS_TO_RDIO_NO				0x00	// 0000 0000
#define RPT_TIM_CHGS_TO_RDIO_MSK			0x40	// 0100 0000

#define RPT_TIM_CHGS_TO_FLSH_YES			0x80	// 1000 0000
#define RPT_TIM_CHGS_TO_FLSH_NO				0x00	// 0000 0000
#define RPT_TIM_CHGS_TO_FLSH_MSK			0x80	// 1000 0000






/*-----------------------  TABLES HERE  -------------------------------------*/

const uchar ucaBitMask[8] =
{
	0x01,			// 0000 0001	
	0x02,			// 0000 0010,	
	0x04,			// 0000 0100,	
	0x08,			// 0000 1000,	
	0x10,			// 0001 0000,	
	0x20,			// 0010 0000,	
	0x40,			// 0100 0000,	
	0x80			// 1000 0000

}; /* END: ucaBitMask[] */



const uchar ucaRevBitMask[8] =
{
	0x80,		// 1000 0000,
	0x40,		// 0100 0000,	
	0x20,		// 0010 0000,	
	0x10,		// 0001 0000,	
	0x08,		// 0000 1000,	
	0x04,		// 0000 0100,	
	0x02,		// 0000 0010,	
	0x01		// 0000 0001

}; /* END: ucaRevBitMask[] */



T_Text S_RoleName[NUMBER_OF_ROLES] =
{
		{"None", 4},					//---
		{"SendOnly", 8},			//--x
		{"Hub", 3},						//-r-
		{"Relay", 5},					//-rx
		{"StandAlone", 10},		//s--
		{"Terminus", 8},			//s-x
		{"SamplingHub", 11},	//sr-
		{"Spoke", 5},				//srx
};

const uchar ucaRoleOptionDefaults[NUMBER_OF_ROLES][OPTION_BYTE_COUNT] =
{
  {
	//ROLE 0 = BLOB (NO SEND, NO RECEIVE, NO SAMPLE)

    0,				//OPTION BYTE 0
	0,				//OPTION BYTE 1
	0,				//OPTION BYTE 2
	0,				//OPTION BYTE 3
	0				//OPTION BYTE 4
	//END: ROLE 0 = BLOB (NO SEND, NO RECEIVE, NO SAMPLE)
  },

  {
	//ROLE 1 = SEND_ONLY (YES SEND, NO RECEIVE, NO SAMPLE)

    1,				//OPTION BYTE 0
	0,				//OPTION BYTE 1
	0,				//OPTION BYTE 2
	0,				//OPTION BYTE 3
	0				//OPTION BYTE 4

	//END: ROLE 1 = SEND_ONLY (YES SEND, NO RECEIVE, NO SAMPLE)
  },


  {
	//ROLE 2 = HUB (NO SEND, YES RECEIVE, NO SAMPLE)

										//OPTION BYTE 0
	WIZ_SENDS_NO|					//0: Wisard sends radio msgs
	WIZ_RECEIVES_YES|				//1: Wisard receives radio msgs
	WIZ_SAMPLES_NO|					//2: Wisard samples data from SD
	WIZ_NIY_NO|						//3: Wisard not implemented bit
	STRMING_NO|						//4: Streaming (On/Off)
	SHOW_SYSTAT_NO|					//5: Show Systat (On/Off)
	CHK_FOR_RDIO_BD_YES|			//6: Check for Radio Board on startup (On/Off)
	USE_RDIO_FOR_RAND_SEED_YES,		//7: Use Radio to Get Random Seed (On/Off)

									//OPTION BYTE 1
	SND_RDIO_STUP_MSG_YES|			//0: Send Radio Startup msg
	ALLOW_BRAIN_STUP_HBRNAT_YES|	//1: Allow Startup Brain Hibernation
	SP_BDS_ARE_ATTCHD_YES|			//2: SP boards are attached
	CHK_FOR_SPS_ON_STUP_YES|		//3: Check for SP boards on startup
	FIX_SOIL_RDING_NO|				//4: Correct the soil readings
	DO_HBRNAT_CHK_YES|				//5: Do Hibernation Check on Startup
	BUZ_ON_YES|						//6: Buzzer on
	CHK_FOR_FRAM_ON_STUP_YES,		//7: Check for Fram on Startup

									//OPTION BYTE 2
	CHK_FRAM_FMT_ON_STUP_YES|		//0: Check Fram format on Startup
	CHK_FOR_FLSH_ON_STUP_NO|		//1: Check for Flash on Startup
	SHOW_STS_ON_LFACTRS_YES|		//2: Show Status on LFactors
	SHOW_LNK_SCHED_YES,				//3: Show Link Schedules
									//4: 
									//5: 
									//6: 
									//7: 

									//OPTION BYTE 3
	RPT_SOM2_LNKS_TO_RDIO_NO|		//0: SOM2 LINKUP       report over radio
	RPT_SOM2_LNKS_TO_FLSH_NO|		//1: SOM2 LINKUP	   report to flash
	RPT_SOM2_FAILS_TO_RDIO_NO|		//2: SOM2 FAILED LINK  report over radio
	RPT_SOM2_FAILS_TO_FLSH_NO|		//3: SOM2 FAILED LINK  report to flash
	RPT_ROM2_LNKS_TO_RDIO_NO|		//4: ROM2 LINKUP	   report over radio
	RPT_ROM2_LNKS_TO_FLSH_YES|		//5: ROM2 LINKUP	   report to flash
	RPT_ROM2_FAILS_TO_RDIO_NO|		//6: ROM2 FAILED LINK  report over radio
	RPT_ROM2_FAILS_TO_FLSH_YES,		//7: ROM2 FAILED LINK  report to flash

									 //OPTION BYTE 4
	RPT_SYS_RSTRTS_TO_RDIO_NO|		 //0: SYS RESTART	   report over radio
	RPT_SYS_RSTRTS_TO_FLSH_YES|		 //1: SYS RESTART	   report to flash
	RPT_BATT_DEAD_RSTRT_TO_RDIO_NO|	 //2: BATT DEAD		   report over radio
	RPT_BATT_DEAD_RSTRT_TO_FLSH_YES| //3: BATT DEAD		   report to flash
	RPT_BATT_VOLTS_TO_RDIO_NO|		 //4: BATT VOLTAGE 	   report over radio
	RPT_BATT_VOLTS_TO_FLSH_YES|		 //5: BATT VOLTATGE	   report to flash
	RPT_TIM_CHGS_TO_RDIO_NO|		 //6: TIME CHANGES	   report over radio
	RPT_TIM_CHGS_TO_FLSH_YES		 //7: TIME CHANGES	   report to flash

	//END: ROLE 2 = HUB (NO SEND, YES RECEIVE, NO SAMPLE)
  },

  {
	//ROLE 3 = RELAY (YES SEND, YES RECEIVE, NO SAMPLE)

									//OPTION BYTE 0
	WIZ_SENDS_YES|					//0: Wisard sends radio msgs
	WIZ_RECEIVES_YES|				//1: Wisard receives radio msgs
	WIZ_SAMPLES_NO|					//2: Wisard samples data from SD
	WIZ_NIY_NO|						//3: Wisard not implemented bit
	STRMING_NO|						//4: Streaming (On/Off)
	SHOW_SYSTAT_NO|					//5: Show Systat (On/Off)
	CHK_FOR_RDIO_BD_YES|			//6: Check for Radio Board on startup (On/Off)
	USE_RDIO_FOR_RAND_SEED_YES,		//7: Use Radio to Get Random Seed (On/Off)

									//OPTION BYTE 1
	SND_RDIO_STUP_MSG_YES|			//0: Send Radio Startup msg
	ALLOW_BRAIN_STUP_HBRNAT_YES|	//1: Allow Startup Brain Hibernation
	SP_BDS_ARE_ATTCHD_NO|			//2: SP boards are attached
	CHK_FOR_SPS_ON_STUP_NO|			//3: Check for SP boards on startup
	FIX_SOIL_RDING_NO|				//4: Correct the soil readings
	DO_HBRNAT_CHK_YES|				//5: Do Hibernation Check on Startup
	BUZ_ON_YES|						//6: Buzzer on
	CHK_FOR_FRAM_ON_STUP_YES,		//7: Check for Fram on Startup

									//OPTION BYTE 2
	CHK_FRAM_FMT_ON_STUP_YES|		//0: Check Fram format on Startup
	CHK_FOR_FLSH_ON_STUP_NO|		//1: Check for Flash on Startup
	SHOW_STS_ON_LFACTRS_YES|		//2: Show Status on LFactors
	SHOW_LNK_SCHED_YES,				//3: Show Link Schedules
									//4: 
									//5: 
									//6: 
									//7: 

									//OPTION BYTE 3
	RPT_SOM2_LNKS_TO_RDIO_YES|		//0: SOM2 LINKUP       report over radio
	RPT_SOM2_LNKS_TO_FLSH_YES|		//1: SOM2 LINKUP	   report to flash
	RPT_SOM2_FAILS_TO_RDIO_YES|		//2: SOM2 FAILED LINK  report over radio
	RPT_SOM2_FAILS_TO_FLSH_YES|		//3: SOM2 FAILED LINK  report to flash
	RPT_ROM2_LNKS_TO_RDIO_YES|		//4: ROM2 LINKUP	   report over radio
	RPT_ROM2_LNKS_TO_FLSH_YES|		//5: ROM2 LINKUP	   report to flash
	RPT_ROM2_FAILS_TO_RDIO_YES|		//6: ROM2 FAILED LINK  report over radio
	RPT_ROM2_FAILS_TO_FLSH_YES,		//7: ROM2 FAILED LINK  report to flash

									 //OPTION BYTE 4
	RPT_SYS_RSTRTS_TO_RDIO_YES|		 //0: SYS RESTART	   report over radio
	RPT_SYS_RSTRTS_TO_FLSH_YES|		 //1: SYS RESTART	   report to flash
	RPT_BATT_DEAD_RSTRT_TO_RDIO_NO|	 //2: BATT DEAD		   report over radio
	RPT_BATT_DEAD_RSTRT_TO_FLSH_YES| //3: BATT DEAD		   report to flash
	RPT_BATT_VOLTS_TO_RDIO_YES|		 //4: BATT VOLTAGE 	   report over radio
	RPT_BATT_VOLTS_TO_FLSH_YES|		 //5: BATT VOLTATGE	   report to flash
	RPT_TIM_CHGS_TO_RDIO_NO|		 //6: TIME CHANGES	   report over radio
	RPT_TIM_CHGS_TO_FLSH_YES		 //7: TIME CHANGES	   report to flash

	//END: ROLE 3 = RELAY (YES SEND, YES RECEIVE, NO SAMPLE)
  },

  {
	//ROLE 4 = STANDALONE (NO SEND, NO RECEIVE, YES SAMPLE)

									//OPTION BYTE 0
	WIZ_SENDS_NO|					//0: Wisard sends radio msgs
	WIZ_RECEIVES_NO|				//1: Wisard receives radio msgs
	WIZ_SAMPLES_YES|				//2: Wisard samples data from SD
	WIZ_NIY_NO|						//3: Wisard not implemented bit
	STRMING_NO|						//4: Streaming (On/Off)
	SHOW_SYSTAT_NO|					//5: Show Systat (On/Off)
	CHK_FOR_RDIO_BD_NO|				//6: Check for Radio Board on startup (On/Off)
	USE_RDIO_FOR_RAND_SEED_NO,		//7: Use Radio to Get Random Seed (On/Off)

									//OPTION BYTE 1
	SND_RDIO_STUP_MSG_NO|			//0: Send Radio Startup msg
	ALLOW_BRAIN_STUP_HBRNAT_YES|	//1: Allow Startup Brain Hibernation
	SP_BDS_ARE_ATTCHD_YES|  		//2: SP boards are attached
	CHK_FOR_SPS_ON_STUP_YES|		//3: Check for SP boards on startup
	FIX_SOIL_RDING_YES|				//4: Correct the soil readings
	DO_HBRNAT_CHK_YES|				//5: Do Hibernation Check on Startup
	BUZ_ON_YES|						//6: Buzzer on
	CHK_FOR_FRAM_ON_STUP_YES,		//7: Check for Fram on Startup

									//OPTION BYTE 2
	CHK_FRAM_FMT_ON_STUP_YES|		//0: Check Fram format on Startup
	CHK_FOR_FLSH_ON_STUP_NO|		//1: Check for Flash on Startup
	SHOW_STS_ON_LFACTRS_NO|			//2: Show Status on LFactors
	SHOW_LNK_SCHED_NO,				//3: Show Link Schedules
									//4: 
									//5: 
									//6: 
									//7: 

									//OPTION BYTE 3
	RPT_SOM2_LNKS_TO_RDIO_NO|		//0: SOM2 LINKUP       report over radio
	RPT_SOM2_LNKS_TO_FLSH_YES|		//1: SOM2 LINKUP	NKUP	   report to flash
	RPT_SOM2_FAILS_TO_RDIO_NO|		//2: SOM2 FAILED LINK  report over radio
	RPT_SOM2_FAILS_TO_FLSH_YES|		//3: SOM2 FAILED LINK  report to flash
	RPT_ROM2_LNKS_TO_RDIO_NO|		//4: ROM2 LINKUP	   report over radio
	RPT_ROM2_LNKS_TO_FLSH_YES|		//5: ROM2 LINKUP	   report to flash
	RPT_ROM2_FAILS_TO_RDIO_NO|		//6: ROM2 FAILED LINK  report over radio
	RPT_ROM2_FAILS_TO_FLSH_YES,		//7: ROM2 FAILED LINK  report to flash

									 //OPTION BYTE 4
	RPT_SYS_RSTRTS_TO_RDIO_NO|		 //0: SYS RESTART	   report over radio
	RPT_SYS_RSTRTS_TO_FLSH_YES|		 //1: SYS RESTART	   report to flash
	RPT_BATT_DEAD_RSTRT_TO_RDIO_NO|	 //2: BATT DEAD		   report over radio
	RPT_BATT_DEAD_RSTRT_TO_FLSH_YES| //3: BATT DEAD		   report to flash
	RPT_BATT_VOLTS_TO_RDIO_NO|		 //4: BATT VOLTAGE 	   report over radio
	RPT_BATT_VOLTS_TO_FLSH_YES|		 //5: BATT VOLTATGE	   report to flash
	RPT_TIM_CHGS_TO_RDIO_NO|		 //6: TIME CHANGES	   report over radio
	RPT_TIM_CHGS_TO_FLSH_YES		 //7: TIME CHANGES	   report to flash

	//END: ROLE 4 = STANDALONE (NO SEND, NO RECEIVE, YES SAMPLE)
  },

  {
	//ROLE 5 = TERMINUS (YES SEND, NO RECEIVE, YES SAMPLE)

											//OPTION BYTE 0
	WIZ_SENDS_YES|					//0: Wisard sends radio msgs
	WIZ_RECEIVES_NO|				//1: Wisard receives radio msgs
	WIZ_SAMPLES_YES|				//2: Wisard samples data from SD
	WIZ_NIY_NO|						//3: Wisard not implemented bit
	STRMING_NO|						//4: Streaming (On/Off)
	SHOW_SYSTAT_NO|					//5: Show Systat (On/Off)
	CHK_FOR_RDIO_BD_YES|	  		//6: Check for Radio Board on startup (On/Off)
	USE_RDIO_FOR_RAND_SEED_YES,		//7: Use Radio to Get Random Seed (On/Off)

									//OPTION BYTE 1
	SND_RDIO_STUP_MSG_YES|			//0: Send Radio Startup msg
	ALLOW_BRAIN_STUP_HBRNAT_YES|	//1: Allow Startup Brain Hibernation
	SP_BDS_ARE_ATTCHD_YES|  		//2: SP boards are attached
	CHK_FOR_SPS_ON_STUP_YES|		//3: Check for SP boards on startup
	FIX_SOIL_RDING_YES|				//4: Correct the soil readings
	DO_HBRNAT_CHK_YES|				//5: Do Hibernation Check on Startup
	BUZ_ON_YES|						//6: Buzzer on
	CHK_FOR_FRAM_ON_STUP_YES,		//7: Check for Fram on Startup

									//OPTION BYTE 2
	CHK_FRAM_FMT_ON_STUP_YES|		//0: Check Fram format on Startup
	CHK_FOR_FLSH_ON_STUP_NO|		//1: Check for Flash on Startup
	SHOW_STS_ON_LFACTRS_YES|		//2: Show Status on LFactors
	SHOW_LNK_SCHED_YES,				//3: Show Link Schedules
									//4: 
									//5: 
									//6: 
									//7: 

									//OPTION BYTE 3
	RPT_SOM2_LNKS_TO_RDIO_YES|		//0: SOM2 LINKUP       report over radio
	RPT_SOM2_LNKS_TO_FLSH_YES|		//1: SOM2 LINKUP	   report to flash
	RPT_SOM2_FAILS_TO_RDIO_YES|		//2: SOM2 FAILED LINK  report over radio
	RPT_SOM2_FAILS_TO_FLSH_YES|		//3: SOM2 FAILED LINK  report to flash
	RPT_ROM2_LNKS_TO_RDIO_NO|		//4: ROM2 LINKUP	   report over radio
	RPT_ROM2_LNKS_TO_FLSH_NO|		//5: ROM2 LINKUP	   report to flash
	RPT_ROM2_FAILS_TO_RDIO_NO|		//6: ROM2 FAILED LINK  report over radio
	RPT_ROM2_FAILS_TO_FLSH_NO,		//7: ROM2 FAILED LINK  report to flash

									 //OPTION BYTE 4
	RPT_SYS_RSTRTS_TO_RDIO_YES|		 //0: SYS RESTART	   report over radio
	RPT_SYS_RSTRTS_TO_FLSH_YES|		 //1: SYS RESTART	   report to flash
	RPT_BATT_DEAD_RSTRT_TO_RDIO_NO|	 //2: BATT DEAD		   report over radio
	RPT_BATT_DEAD_RSTRT_TO_FLSH_YES| //3: BATT DEAD		   report to flash
	RPT_BATT_VOLTS_TO_RDIO_YES|		 //4: BATT VOLTAGE 	   report over radio
	RPT_BATT_VOLTS_TO_FLSH_YES|		 //5: BATT VOLTATGE	   report to flash
	RPT_TIM_CHGS_TO_RDIO_YES|		 //6: TIME CHANGES	   report over radio
	RPT_TIM_CHGS_TO_FLSH_YES		 //7: TIME CHANGES	   report to flash

	//END: ROLE 5 = TERMINUS (YES SEND, NO RECEIVE, YES SAMPLE)
  },


  {
  	//ROLE 6 = SAMPLING HUB (NO SEND, YES RECEIVE, YES SAMPLE)

											//OPTION BYTE 0
	WIZ_SENDS_NO|					//0: Wisard sends radio msgs
	WIZ_RECEIVES_YES|				//1: Wisard receives radio msgs
	WIZ_SAMPLES_YES|				//2: Wisard samples data from SD
	WIZ_NIY_NO|						//3: Wisard not implemented bit
	STRMING_NO|						//4: Streaming (On/Off)
	SHOW_SYSTAT_NO|					//5: Show Systat (On/Off)
	CHK_FOR_RDIO_BD_YES|			//6: Check for Radio Board on startup (On/Off)
	USE_RDIO_FOR_RAND_SEED_YES,		//7: Use Radio to Get Random Seed (On/Off)

									//OPTION BYTE 1
	SND_RDIO_STUP_MSG_YES|			//0: Send Radio Startup msg
	ALLOW_BRAIN_STUP_HBRNAT_YES|	//1: Allow Startup Brain Hibernation
	SP_BDS_ARE_ATTCHD_YES|			//2: SP boards are attached
	CHK_FOR_SPS_ON_STUP_YES|		//3: Check for SP boards on startup
	FIX_SOIL_RDING_YES|				//4: Correct the soil readings
	DO_HBRNAT_CHK_YES|				//5: Do Hibernation Check on Startup
	BUZ_ON_YES|						//6: Buzzer on
	CHK_FOR_FRAM_ON_STUP_YES,		//7: Check for Fram on Startup

									//OPTION BYTE 2
	CHK_FRAM_FMT_ON_STUP_YES|		//0: Check Fram format on Startup
	CHK_FOR_FLSH_ON_STUP_NO|		//1: Check for Flash on Startup
	SHOW_STS_ON_LFACTRS_YES|		//2: Show Status on LFactors
	SHOW_LNK_SCHED_YES,				//3: Show Link Schedules
									//4: 
									//5: 
									//6: 
									//7: 

									//OPTION BYTE 3
	RPT_SOM2_LNKS_TO_RDIO_NO|		//0: SOM2 LINKUP       report over radio
	RPT_SOM2_LNKS_TO_FLSH_YES|		//1: SOM2 LINKUP	   report to flash
	RPT_SOM2_FAILS_TO_RDIO_NO|		//2: SOM2 FAILED LINK  report over radio
	RPT_SOM2_FAILS_TO_FLSH_YES|		//3: SOM2 FAILED LINK  report to flash
	RPT_ROM2_LNKS_TO_RDIO_NO|		//4: ROM2 LINKUP	   report over radio
	RPT_ROM2_LNKS_TO_FLSH_YES|		//5: ROM2 LINKUP	   report to flash
	RPT_ROM2_FAILS_TO_RDIO_NO|		//6: ROM2 FAILED LINK  report over radio
	RPT_ROM2_FAILS_TO_FLSH_YES,		//7: ROM2 FAILED LINK  report to flash

									 //OPTION BYTE 4
	RPT_SYS_RSTRTS_TO_RDIO_NO|		 //0: SYS RESTART	   report over radio
	RPT_SYS_RSTRTS_TO_FLSH_YES|		 //1: SYS RESTART	   report to flash
	RPT_BATT_DEAD_RSTRT_TO_RDIO_NO|	 //2: BATT DEAD		   report over radio
	RPT_BATT_DEAD_RSTRT_TO_FLSH_YES| //3: BATT DEAD		   report to flash
	RPT_BATT_VOLTS_TO_RDIO_NO|		 //4: BATT VOLTAGE 	   report over radio
	RPT_BATT_VOLTS_TO_FLSH_YES|		 //5: BATT VOLTATGE	   report to flash
	RPT_TIM_CHGS_TO_RDIO_NO|		 //6: TIME CHANGES	   report over radio
	RPT_TIM_CHGS_TO_FLSH_YES		 //7: TIME CHANGES	   report to flash

  	//END: ROLE 6 = SAMPLING HUB (NO SEND, YES RECEIVE, YES SAMPLE)
  },


  {
  	//ROLE 7 = SPOKE (YES SEND, YES RECEIVE, YES SAMPLE) (DEFAULT)

									//OPTION BYTE 0
	WIZ_SENDS_YES|					//0: Wisard sends radio msgs
	WIZ_RECEIVES_YES|				//1: Wisard receives radio msgs
	WIZ_SAMPLES_YES|				//2: Wisard samples data from SD
	WIZ_NIY_NO|						//3: Wisard not implemented bit
	STRMING_NO|						//4: Streaming (On/Off)
	SHOW_SYSTAT_NO|					//5: Show Systat (On/Off)
	CHK_FOR_RDIO_BD_YES|			//6: Check for Radio Board on startup (On/Off)
	USE_RDIO_FOR_RAND_SEED_YES,		//7: Use Radio to Get Random Seed (On/Off)

									//OPTION BYTE 1
	SND_RDIO_STUP_MSG_YES|			//0: Send Radio Startup msg
	ALLOW_BRAIN_STUP_HBRNAT_YES|	//1: Allow Startup Brain Hibernation
	SP_BDS_ARE_ATTCHD_YES|			//2: SP boards are attached
	CHK_FOR_SPS_ON_STUP_NO|			//3: Check for SP boards on startup
	FIX_SOIL_RDING_YES|				//4: Correct the soil readings
	DO_HBRNAT_CHK_YES|				//5: Do Hibernation Check on Startup
	BUZ_ON_YES|						//6: Buzzer on
	CHK_FOR_FRAM_ON_STUP_YES,		//7: Check for Fram on Startup

									//OPTION BYTE 2
	CHK_FRAM_FMT_ON_STUP_YES|		//0: Check Fram format on Startup
	CHK_FOR_FLSH_ON_STUP_NO|		//1: Check for Flash on Startup
	SHOW_STS_ON_LFACTRS_YES|		//2: Show Status on LFactors
	SHOW_LNK_SCHED_YES,				//3: Show Link Schedules
									//4: 
									//5: 
									//6: 
									//7: 

									//OPTION BYTE 3
	RPT_SOM2_LNKS_TO_RDIO_YES|		//0: SOM2 LINKUP       report over radio
	RPT_SOM2_LNKS_TO_FLSH_YES|		//1: SOM2 LINKUP	   report to flash
	RPT_SOM2_FAILS_TO_RDIO_YES|		//2: SOM2 FAILED LINK  report over radio
	RPT_SOM2_FAILS_TO_FLSH_YES|		//3: SOM2 FAILED LINK  report to flash
	RPT_ROM2_LNKS_TO_RDIO_YES|		//4: ROM2 LINKUP	   report over radio
	RPT_ROM2_LNKS_TO_FLSH_YES|		//5: ROM2 LINKUP	   report to flash
	RPT_ROM2_FAILS_TO_RDIO_YES|		//6: ROM2 FAILED LINK  report over radio
	RPT_ROM2_FAILS_TO_FLSH_YES,		//7: ROM2 FAILED LINK  report to flash

									 //OPTION BYTE 4
	RPT_SYS_RSTRTS_TO_RDIO_YES|		 //0: SYS RESTART	   report over radio
	RPT_SYS_RSTRTS_TO_FLSH_YES|		 //1: SYS RESTART	   report to flash
	RPT_BATT_DEAD_RSTRT_TO_RDIO_NO|	 //2: BATT DEAD		   report over radio
	RPT_BATT_DEAD_RSTRT_TO_FLSH_YES| //3: BATT DEAD		   report to flash
	RPT_BATT_VOLTS_TO_RDIO_YES|		 //4: BATT VOLTAGE 	   report over radio
	RPT_BATT_VOLTS_TO_FLSH_YES|		 //5: BATT VOLTATGE	   report to flash
	RPT_TIM_CHGS_TO_RDIO_NO|		 //6: TIME CHANGES	   report over radio
	RPT_TIM_CHGS_TO_FLSH_YES		 //7: TIME CHANGES	   report to flash

  	//END: ROLE 7 = SPOKE (SEND, RECEIVE, YES SAMPLE) (DEFAULT)
  },


}; /* END: const uchar ucaRoleOptionBytes[][] */



T_Text S_BitName[OPTION_BYTE_COUNT][8] =
{

	{						//OPTION BYTE 0

			{"RdioSnd", 7},				//0: Wisard sends radio msgs
			{"RdioRec",	7},			//1: Wisard receives radio msgs
			{"SampDat",	7},		//2: Wisard samples data from SD
			{" ", 1},					//3: Wisard not implemented bit
			{"Stream", 6},				//4: Streaming (On/Off)
			{"Systat",6},				//5: Show Systat (On/Off)
			{"CkRdioBd", 8},				//6: Check for Radio Board on startup (On/Off)
			{"RdioRnd", 7},				//7: Use Radio to Get Random Seed (On/Off)
	},

	{						//OPTION BYTE 1

			{"RdioMsg", 7},				//0: Send Radio Startup msg
			{"BrHbrnt", 7},				//1: Allow Startup Brain Hibernation
			{"CkSdAtch",8 },			//2: SD board is attached
			{"CkSDStup", 8},			//3: Check for SD on startup
			{"FxSoil", 6},				//4: Correct the soil readings
			{"HbrnatCk", 8},			//5: Do Hibernation Check on Startup
			{"BuzOn", 5},					//6: Buzzer on
			{"CkForFrm", 8},			//7: Check for Fram on Startup
	},

	{						//OPTION BYTE 2

			{"CkFrmFmt", 8},			//0: Check Fram format on Startup
			{"CkForFsh", 8},			//1: Check for Flash on Startup
			{"ShwStsLf", 8},			//2: Show Status on LFactors
			{"ShowLnks", 8},			//3: Show Link Schedules
			{"", 0},							//4:
			{"", 0},							//5:
			{"", 0},							//6:
			{"", 0},							//7:
	},

	{						//OPTION BYTE 3

			{"RdioSLk", 7},				//0: SOM2 LINKUP       report over radio
			{"FlshSLk", 7},				//1: SOM2 LINKUP	   report to flash
			{"RdioFSLk", 8},			//2: SOM2 FAILED LINK  report over radio
			{"FlshFSLk", 8},			//3: SOM2 FAILED LINK  report to flash
			{"RdioRLk", 7},				//4: ROM2 LINKUP	   report over radio
			{"FlshRLk", 7},				//5: ROM2 LINKUP	   report to flash
			{"RdioFRLk", 8},			//6: ROM2 FAILED LINK  report over radio
			{"FlshFRLk", },				//7: ROM2 FAILED LINK  report to flash
	},

	{						//OPTION BYTE 4

			{"RdioRsts", 8},			//0: SYS RESTART	   report over radio
			{"FlshRsts", 8},			//1: SYS RESTART	   report to flash
			{"RdioBtDd", 8},			//2: BATT DEAD		   report over radio
			{"FlshBtDd", 8},			//3: BATT DEAD		   report to flash
			{"RdioBtV", 7},				//4: BATT VOLTAGE 	   report over radio
			{"FlshBtV", 7},				//5: BATT VOLTATGE	   report to flash
			{"RdioTime", 8},			//6: TIME CHANGES	   report over radio
			{"FlshTime", 8},			//7: TIME CHANGES	   report to flash
	}

};

extern struct Role G3Role;

/*----------------------------------------------------------------*/


/* FUNCTION DELCARATIONS */
//     void vKEY_exitFuncPlaceHolder(void);		// 0 Quit
static void vMODOPT_cmdShowHelp(void);			// 1 Help
static void vMODOPT_cmdShowOptions(void);		// 2 Show Option bits
static void vMODOPT_cmdSetOption(void);			// 3 Set an option bit
static void vMODOPT_cmdClrOption(void);			// 4 Clr an option bit
static void vMODOPT_cmdSetRole(void);			// 5 Set a new role

#define CMD_ARRAY_SIZE 6

/* NOTE: This array is parallel to the Cmd array */
T_Text S_MODOPTcmdText[CMD_ARRAY_SIZE] =
	{
			{"Quit", 4},									// 0 Quit
			{"Help", 4},									// 1 Help
			{"showOptions", 11},							// 2 Show Option bits
			{"setOption", 9},							// 3 Set an Option bit
			{"clrOption", 9},							// 4 Clr an Option bit
			{"setRole", 7},								// 5 Set a new role
	}; /* END: cpaModOptCmdText[] */


/* NOTE: This array is parallel to the Cmd array */
const GENERIC_CMD_FUNC vMODOPTfunctionTbl[CMD_ARRAY_SIZE] =
 {
 vKEY_exitFuncPlaceHolder,			// 0 Quit
 vMODOPT_cmdShowHelp,				// 1 Help
 vMODOPT_cmdShowOptions,			// 2 Show Option bits
 vMODOPT_cmdSetOption,				// 3 set an option bit
 vMODOPT_cmdClrOption,				// 4 clr an option bit
 vMODOPT_cmdSetRole					// 5 set a new role
 };







/*-------------------  FUNCTION DECLARATIONS HERE  ---------------------------*/

uchar ucMODOPT_readFramOptionByte(
		uchar ucOptionByteIdx		//0 to OPTION_BYTE_COUNT
		);

void vMODOPT_writeFramOptionByte(
		uchar ucOptionByteIdx,		//0 to OPTION_BYTE_COUNT
		uchar ucByteVal				//value to write to FRAM
		);

void vMODOPT_cmdSilentSetOption(
		uchar ucOptionIdxPair
		);

void vMODOPT_cmdSilentClrOption(
		uchar ucOptionIdxPair
		);

void vMODOPT_cmdSilentSetRole(
		char cRoleIdx
		);





/*---------------------- CODE STARTS HERE  -----------------------------------*/


/*----------------------  ROM OPTION ROUTINES  -------------------------------*/



/**********************  vMODOPT_showOptionColHeader()  ***************************
*
* Show the column header
*
******************************************************************************/

void vMODOPT_showOptionColHeader(
		void
		)
	{
	char *cStrPtr;

	/* SHOW COLUMN HEADER */
	cStrPtr = "ID NAME     STS    ";
	vSERIAL_sout(cStrPtr, 19);
	vSERIAL_sout(cStrPtr, 19);
	vSERIAL_sout(cStrPtr, 19);
	vSERIAL_sout(cStrPtr, 19);
	vSERIAL_sout("\r\n\r\n", 4);

	return;

	}/* END: vMODOPT_showOptionColHeader() */








/*****************  vMODOPT_showSingleRomOptionBit()  ************************
*
* Show a single Option entry
*
******************************************************************************/

void vMODOPT_showSingleRomOptionBit(
		uchar ucOptionTblNum,	//ROM Option table idx
		uchar ucOptionIdxPair	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;
	T_Text S_StrPtr;
	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x07;
	ucByteIdx = ucOptionIdxPair >> 3;

	vSERIAL_UI8_2char_out(ucOptionIdxPair,' ');		//show index
	vSERIAL_sout(" ", 1);
	vSERIAL_padded_sout(&S_BitName[ucByteIdx][ucBitIdx], 8);//show name
	vSERIAL_sout(" ", 1);

	S_StrPtr.m_cText = "No";
	S_StrPtr.m_uiLength = 2;

	if(ucaRoleOptionDefaults[ucOptionTblNum][ucByteIdx] & ucaBitMask[ucBitIdx])
		{
		S_StrPtr.m_cText = "Yes";
		S_StrPtr.m_uiLength = 3;
		}

	vSERIAL_padded_sout(&S_StrPtr,-3);	//show Bit state

	#if 0
	vSERIAL_sout(" = ", 3);
	vSERIAL_HB8out(ucaRoleOptionDefaults[ucOptionTblNum][ucByteIdx]);
	#endif

	return;

	}/* END: vMODOPT_showSingleRomOptionBit() */






/**************  vMODOPT_showSingleRomOptionTblAllBits()  ********************
*
* Show a single Rom Option table (show all bits)
*
******************************************************************************/

void vMODOPT_showSingleRomOptionTblAllBits(
		uchar ucOptionTblNum
		)
	{
	uchar ucjj;
	uchar uckk;
	uchar ucOptionIdxPair;

	/* SHOW COLUMN HEADER */
	vMODOPT_showOptionColHeader();

	/* LOOP FOR EACH BYTE IN THE Option TABLE */
	for(ucjj=0;  ucjj< OPTION_BYTE_COUNT;  ucjj++)	//byte idx
		{
		/* LOOP FOR EACH BIT IN THE BYTE */
		for(uckk=0;  uckk<8;  uckk++)	//bit idx
			{
			/* BUILD THE OPTION IDX PAIR */
			ucOptionIdxPair = ucjj;
			ucOptionIdxPair <<= 3;
			ucOptionIdxPair |= uckk;

			vMODOPT_showSingleRomOptionBit(ucOptionTblNum, ucOptionIdxPair);
			vSERIAL_sout("    ", 4);

			if((uckk != 0) && ((uckk % 4) == 3))
				{
				vSERIAL_crlf();
				}

			}/* END: for(uckk) */

		}/* END: for(ucjj) */

	vSERIAL_crlf();

	return;

	}/* END: vMODOPT_showSingleRomOptionTblAllBits() */





/***************  vMODOPT_showAllRomOptionTblsAllBits()  *********************
*
* Show the entire Option table bit collection
*
******************************************************************************/

void vMODOPT_showAllRomOptionTblsAllBits(
		void
		)
	{
	uchar ucii;

	/* LOOP FOR EACH OPTION NUMBER */
	for(ucii=0;  ucii<NUMBER_OF_ROLES;  ucii++)
		{
		vSERIAL_sout("\r\nOPTION TBL ", 13);
		vSERIAL_UIV8out(ucii);
		vSERIAL_sout(" (\"", 3);
		vSERIAL_sout(S_RoleName[ucii].m_cText, S_RoleName[ucii].m_uiLength);
		vSERIAL_sout("\")\r\n", 4);

		vMODOPT_showSingleRomOptionTblAllBits(ucii);

		} /* END: for(ucii) */

	vSERIAL_crlf();

	return;

	}/* END: vMODOPT_showAllRomOptionTblsAllBits() */








/***************  vMODOPT_showAllRomOptionTblNames()  *************************
*
* Show the entire Option table
*
******************************************************************************/

void vMODOPT_showAllRomOptionTblNames(
		void
		)
	{
	uchar ucii;

	/* LOOP FOR EACH Option NUMBER */
	for(ucii=0;  ucii<NUMBER_OF_ROLES;  ucii++)
		{
		vSERIAL_UI8_2char_out(ucii,' ');
		vSERIAL_sout(" ", 1);
		vSERIAL_sout(S_RoleName[ucii].m_cText, S_RoleName[ucii].m_uiLength);
		vSERIAL_crlf();

		} /* END: for(ucii) */

	vSERIAL_crlf();

	return;

	}/* END: vMODOPT_showAllRomOptionTblNames() */


/*--------------------  FRAM OPTION ROUTINES  ---------------------------------*/



/********************  ucMODOPT_readSingleFramOptionBit()  ********************
*
* Returns a single bit flag from the Option bit array.
*
* NOTE: Ret Bit is Right Justified.
*
******************************************************************************/

uchar ucMODOPT_readSingleFramOptionBit(
		uchar ucOptionIdxPair	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;
	uchar ucByteVal;
	uchar ucBitVal;

	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x7;
	ucByteIdx = ucOptionIdxPair >> 3;
	
	/* READ THE OPTION BYTE */
	ucByteVal = ucMODOPT_readFramOptionByte(ucByteIdx);

	/* MASK OUT THE BIT */
	ucBitVal = 0;
	if(ucByteVal & ucaBitMask[ucBitIdx]) ucBitVal = 1;

	return(ucBitVal);

	}/* END: ucMODOPT_readSingleFramOptionBit() */





/****************  vMODOPT_writeSingleFramOptionBit()  *******************
*
* Write a single bit flag from the Option bit array.
*
******************************************************************************/

void vMODOPT_writeSingleFramOptionBit(
		uchar ucOptionIdxPair,	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		uchar ucBitVal		 	//0 = clear bit, non-zro = set bit
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;
	uchar ucByteVal;

	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x7;
	ucByteIdx = ucOptionIdxPair >> 3;
	
	/* READ THE OPTION BYTE */
	ucByteVal = ucMODOPT_readFramOptionByte(ucByteIdx);

	/* CLEAR THE THE BIT */
	ucByteVal &= (~ucaBitMask[ucBitIdx]);

	/* SET THE BIT IF IT NEEDS TO BE SET */
	if(ucBitVal != 0)
		{
		ucByteVal |= ucaBitMask[ucBitIdx];
		}

	/* WRITE THE BYTE BACK */
	vMODOPT_writeFramOptionByte(ucByteIdx, ucByteVal);

	return;

	}/* END: vMODOPT_writeSingleFramOptionBit() */






/*****************  ucMODOPT_readFramOptionByte()  ****************************
*
* RET:	The full 8 bits of the Option Byte specified
* 		0 - if index out of range, or access error
*
*
******************************************************************************/

uchar ucMODOPT_readFramOptionByte(
		uchar ucOptionByteIdx		//0 - OPTION_BYTE_COUNT
		)
	{
	uchar ucByteVal;
	
	if(ucOptionByteIdx > OPTION_BYTE_COUNT) return(0);

	if(ucL2FRAM_GetOptionByte(ucOptionByteIdx, &ucByteVal) != 0)
		return 0;

	return(ucByteVal);

	}/* END: ucMODOPT_readFramOptionByte() */





/*****************  vMODOPT_writeFramOptionByte()  ****************************
*
* NOTE: will not write if byte index if out of range or if access violation
*
******************************************************************************/

void vMODOPT_writeFramOptionByte(
		uchar ucOptionByteIdx,		//0 - OPTION_BYTE_COUNT
		uchar ucByteVal				//value to write to FRAM
		)
	{
	if(ucOptionByteIdx > OPTION_BYTE_COUNT) return;

	ucL2FRAM_SetOptionByte(ucOptionByteIdx, ucByteVal);

	return;

	}/* END: vMODOPT_writeFramOptionByte() */





/**************  vMODOPT_copyAllRomOptionsToFramOptions()  *******************
*
* copy a single ROM Option Table into the Fram Option Table
*
******************************************************************************/

void vMODOPT_copyAllRomOptionsToFramOptions(
		uchar ucRomOptionTblNum
		)
	{
	uchar ucjj;
	uchar ucOptionByteVal;

	/* LOOP FOR EACH BYTE IN THE OPTION TABLE */
	for(ucjj=0;  ucjj< OPTION_BYTE_COUNT;  ucjj++)	//byte idx
		{
		ucOptionByteVal = ucaRoleOptionDefaults[ucRomOptionTblNum][ucjj];
		vMODOPT_writeFramOptionByte(ucjj, ucOptionByteVal);

		}/* END: for(ucjj) */

	return;

	}/* END: vMODOPT_copyAllRomOptionsToFramOptions() */








/*****************  vMODOPT_showSingleFramOptionBit()  ************************
*
* Show a single Option entry
*
******************************************************************************/

void vMODOPT_showSingleFramOptionBit(
		uchar ucOptionIdxPair	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;
	T_Text S_StrPtr;

	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x07;
	ucByteIdx = ucOptionIdxPair >> 3;

	vSERIAL_UI8_2char_out(ucOptionIdxPair,' ');		//show index
	vSERIAL_sout(" ", 1);
	vSERIAL_padded_sout(&S_BitName[ucByteIdx][ucBitIdx], 8);//show name
	vSERIAL_sout(" ", 1);

	S_StrPtr.m_cText = "No";
	S_StrPtr.m_uiLength = 2;			//assume bit off

	if(ucMODOPT_readFramOptionByte(ucByteIdx) & ucaBitMask[ucBitIdx])
		{
		S_StrPtr.m_cText = "Yes";
		S_StrPtr.m_uiLength = 3;		//correct assumption
		}

	vSERIAL_padded_sout(&S_StrPtr,-3);	//show Bit state

	return;

	}/* END: vMODOPT_showSingleFramOptionBit() */






/*****************  vMODOPT_showAllFramOptionBits()  ******************************
*
* Show all Fram Option bits
*
******************************************************************************/

void vMODOPT_showAllFramOptionBits(
		void
		)
	{
	uchar ucjj;
	uchar uckk;
	uchar ucOptionIdxPair;

	/* SHOW THE TITLE HEADER */
	vSERIAL_sout("\r\nOPTION TBL  (\"FRAM VERSION\")\r\n", 32);

	/* SHOW COLUMN HEADER */
	vMODOPT_showOptionColHeader();

	/* LOOP FOR EACH BYTE IN THE OPTION TABLE */
	for(ucjj=0;  ucjj< OPTION_BYTE_COUNT;  ucjj++)	//byte idx
		{
		/* LOOP FOR EACH BIT IN THE BYTE */
		for(uckk=0;  uckk<8;  uckk++)	//bit idx
			{
			/* BUILD THE OPTION IDX PAIR */
			ucOptionIdxPair = ucjj;
			ucOptionIdxPair <<= 3;
			ucOptionIdxPair |= uckk;

			vMODOPT_showSingleFramOptionBit(ucOptionIdxPair);
			vSERIAL_sout("    ", 4);

			if((uckk != 0) && ((uckk % 4) == 3))
				{
				vSERIAL_crlf();
				}

			}/* END: for(uckk) */

		}/* END: for(ucjj) */

	vSERIAL_crlf();

	return;

	}/* END: vMODOPT_showAllFramOptionBits() */







#if 0
/*****************  ucMODOPT_buildTwoBitReportVal()  **************************
*
* Build a 2 bit report value from the Ram Option bits
*
******************************************************************************/

uchar ucMODOPT_buildTwoBitReportVal( 
		uchar ucRadioReportBitIdxPair,
		uchar ucFlashReportBitIdxPair
		)
	{
	uchar ucReportVal;

	ucReportVal =  ucMODOPT_readSingleRamOptionBit(ucRadioReportBitIdxPair);
	ucReportVal <<= 1;
	ucReportVal |= ucMODOPT_readSingleRamOptionBit(ucFlashReportBitIdxPair);

	return(ucReportVal);

	}/* END: ucMODOPT_buildTwoBitReportVal() */
#endif







/*--------------------  RAM OPTION ROUTINES  ---------------------------------*/



/*****************  vMODOPT_copyAllFramOptionsToRamOptions()  **************************
*
* copy a single ROM Option Table into the Fram Option Table
*
******************************************************************************/

void vMODOPT_copyAllFramOptionsToRamOptions(
		void
		)
	{
	uchar ucjj;

	/* LOOP FOR EACH BYTE IN THE Option TABLE */
	for(ucjj=0;  ucjj< OPTION_BYTE_COUNT;  ucjj++)	//byte idx
		{
		ucaGLOB_optionBytes[ucjj] = ucMODOPT_readFramOptionByte(ucjj);

		}/* END: for(ucjj) */

	return;

	}/* END: vMODOPT_copyAllFramOptionsToRamOptions() */






/********************  ucMODOPT_readSingleRamOptionBit()  ********************
*
* Returns a single bit flag from the Option bit array.
*
* NOTE: Ret Bit is Right Justified.
*
******************************************************************************/

uchar ucMODOPT_readSingleRamOptionBit(
		uchar ucOptionIdxPair	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;
	uchar ucBitVal;

	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x7;
	ucByteIdx = ucOptionIdxPair >> 3;
	
	/* MASK OUT THE BIT */
	ucBitVal = 0;
	if(ucaGLOB_optionBytes[ucByteIdx] & ucaBitMask[ucBitIdx]) ucBitVal = 1;

	return(ucBitVal);

	}/* END: ucMODOPT_readSingleRamOptionBit() */





/****************  vMODOPT_writeSingleRamOptionBit()  *************************
*
* Write a single bit flag from the Option bit array.
*
******************************************************************************/

void vMODOPT_writeSingleRamOptionBit(
		uchar ucOptionIdxPair,	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		uchar ucBitVal			//0 = clr-bit, non-zro = set-bit
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;

	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x7;
	ucByteIdx = ucOptionIdxPair >> 3;
	
	/* CLEAR THE THE BIT */
	ucaGLOB_optionBytes[ucByteIdx] &= (~ucaBitMask[ucBitIdx]);

	/* SET THE BIT IF IT NEEDS TO BE SET */
	if(ucBitVal != 0)
		{
		ucaGLOB_optionBytes[ucByteIdx] |= ucaBitMask[ucBitIdx];
		}

	return;

	}/* END: vMODOPT_writeSingleRamOptionBit() */







/*****************  vMODOPT_showSingleRamOptionBit()  ***************************
*
* Show a single Option entry
*
******************************************************************************/

void vMODOPT_showSingleRamOptionBit(
		uchar ucOptionIdxPair	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;
	T_Text S_StrPtr;

	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x07;
	ucByteIdx = ucOptionIdxPair >> 3;

	vSERIAL_UI8_2char_out(ucOptionIdxPair,' ');		//show index
	vSERIAL_sout(" ", 1);
	vSERIAL_padded_sout(&S_BitName[ucByteIdx][ucBitIdx], 8);//show name
	vSERIAL_sout(" ", 1);

	S_StrPtr.m_cText = "No";			//assume bit off
	S_StrPtr.m_uiLength = 2;
	if(ucaGLOB_optionBytes[ucByteIdx] & ucaBitMask[ucBitIdx])
		{
		S_StrPtr.m_cText = "Yes";		//correct assumption
		S_StrPtr.m_uiLength =3;
		}
	vSERIAL_padded_sout(&S_StrPtr,-3);	//show Bit state

	return;

	}/* END: vMODOPT_showSingleRamOptionBit() */






/*****************  vMODOPT_showAllRamOptionBits()  **************************
*
* Show the entire ram option table
*
******************************************************************************/

void vMODOPT_showAllRamOptionBits(
		void
		)
	{
	uchar ucjj;
	uchar uckk;
	uchar ucOptionIdxPair;

	/* SHOW THE RAM TITLE HEADER */
	vSERIAL_sout("\r\nOPTION TBL  (\"RAM COPY\")\r\n", 28);

	/* SHOW COLUMN HEADER */
	vMODOPT_showOptionColHeader();

	/* LOOP FOR EACH BYTE IN THE OPTION TABLE */
	for(ucjj=0;  ucjj< OPTION_BYTE_COUNT;  ucjj++)	//byte idx
		{
		/* LOOP FOR EACH BIT IN THE BYTE */
		for(uckk=0;  uckk<8;  uckk++)	//bit idx
			{
			/* BUILD THE OPTION IDX PAIR */
			ucOptionIdxPair = ucjj;
			ucOptionIdxPair <<= 3;
			ucOptionIdxPair |= uckk;

			vMODOPT_showSingleRamOptionBit(ucOptionIdxPair);
			vSERIAL_sout("    ", 4);

			if((uckk != 0) && ((uckk % 4) == 3))
				{
				vSERIAL_crlf();
				}

			}/* END: for(uckk) */

		}/* END: for(ucjj) */

	vSERIAL_crlf();

	return;

	}/* END: vMODOPT_showAllRamOptionBits() */



/****************  vMODOPT_writeBothFramAndRamBit()  *************************
*
* Write a single bit flag from the Option bit array.
*
******************************************************************************/

void vMODOPT_writeBothFramAndRamBit(
		uchar ucOptionIdxPair,	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		uchar ucBitVal			//0 = clr-bit, non-zro = set-bit
		)
	{

	vMODOPT_writeSingleFramOptionBit(ucOptionIdxPair, ucBitVal);
	vMODOPT_writeSingleRamOptionBit(ucOptionIdxPair, ucBitVal);

	return;

	}/* END: vMODOPT_writeBothFramAndRamBit() */




/****************  ucMODOPT_getCurRole()  ************************************
*
* return the current role index
*
******************************************************************************/

uchar ucMODOPT_getCurRole(
		void
		)
	{
	return(ucaGLOB_optionBytes[0] & 0x7);

	}/* END: vMODOPT_getCurRole() */




/****************  vMODOPT_showCurRole()  ************************************
*
* Print only the Role name (no CRLF) (no leader)
*
******************************************************************************/

void vMODOPT_showCurRole(
		void
		)
	{
	uchar ucRoleIdx;

	ucRoleIdx = ucMODOPT_getCurRole();
	vSERIAL_sout(S_RoleName[ucRoleIdx].m_cText, S_RoleName[ucRoleIdx].m_uiLength);

	return;

	}/* END: vMODOPT_showCurRole() */
	

//////////////////////////////////////////////////////////////////////////////
//! \fn ucMODOPT_isRelay
//! \brief Checks if the node's role is relay
//!	\return 1 if relay, else 0
//////////////////////////////////////////////////////////////////////////////
uchar ucMODOPT_isRelay(void)
{
	uchar ucRoleBits;

	ucRoleBits = ucMODOPT_getCurRole();

	if((ucRoleBits & ROLE_RELAY_MSK) == 3)
		return 1;

	return 0;
}

/****************  vMODOPT_showCurRole()  ************************************
*
* Print the Role name and types of SP boards attached (no CRLF) (no leader)
*
******************************************************************************/

void vMODOPT_showCurIdentity(
		void
		)
	{

	G3Role.ucRole = ucMODOPT_getCurRole();

	vMODOPT_showCurRole();


	vSERIAL_crlf();
	return;

	}/* END: vMODOPT_showCurIdentity() */





/*------------------------  CMD FUNCTIONS  ----------------------------------*/




/****************  vMODOPT_cmdModifyOptionTbls()  *******************************
*
* Write a single bit flag from the Option bit array.
*
******************************************************************************/

void vMODOPT_cmdModifyOptionTbls(
		void
		)
	{
	vSERIAL_sout("Cmds:\r\n", 7);
	vMODOPT_cmdShowHelp();

	vNUMCMD_doCmdInputAndDispatch(
				'*',						//prompt
				S_MODOPTcmdText,			//command array
				vMODOPTfunctionTbl,			//dispatch array ptrs
				CMD_ARRAY_SIZE,				//dispatch array size
				YES_SHOW_INITIAL_PROMPT		//prompt flag
				);

	return;

	}/* END: vMODOPT_cmdModifyOptionTbls() */





/*******************  vMODOPT_cmdShowHelp()  ************************************
*
*
*
******************************************************************************/
void vMODOPT_cmdShowHelp(
		void
		)
	{
	vNUMCMD_showCmdList(S_MODOPTcmdText, CMD_ARRAY_SIZE);
	vSERIAL_crlf();

	return;
	}/* END: vMODOPT_cmdShowHelp() */




/*******************  vMODOPT_cmdShowOptions()  ******************************
*
*
*
******************************************************************************/
void vMODOPT_cmdShowOptions(
		void
		)
	{

	vSERIAL_sout("Role= ", 6);
	vMODOPT_showCurRole();
	vSERIAL_crlf();

	vMODOPT_showAllRamOptionBits();

	return;

	}/* END: vMODOPT_cmdShowOptions() */






/*****************  vMODOPT_cmdSetOption()  *************************************
*
*
*
******************************************************************************/
void vMODOPT_cmdSetOption(
		void
		)
	{
	signed char cOptionNum;

	/* SHOW WHATS OUT THERE */
	vMODOPT_showAllRamOptionBits();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_sout("OptID: ", 7);
	cOptionNum = cNUMCMD_getCmdNum(MAX_OPTPAIR_VALUE);
	if(cOptionNum < 0)
		{
		goto setOptionExit;
		}

	/* ITS VALID -- SET THE OPTION */
	vMODOPT_cmdSilentSetOption((uchar)cOptionNum);

	/* SHOW THE NEW OPTION */
	vMODOPT_showAllRamOptionBits();
	vSERIAL_dash(10);
	vSERIAL_crlf();


setOptionExit:
	return;

	}/* END: vMODOPT_cmdSetOption() */






/*****************  vMODOPT_cmdSilentSetOption()  ****************************
*
*
*
******************************************************************************/
void vMODOPT_cmdSilentSetOption(
		uchar ucOptionIdxPair
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;

	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x7;
	ucByteIdx = ucOptionIdxPair >> 3;

	/* RANGE CHECK THE INDEX */
	if(ucBitIdx >= 8) goto silentSetOptionExit;
	if(ucByteIdx >= OPTION_BYTE_COUNT) goto silentSetOptionExit;

	/* SET THE OPTION BIT */
	vMODOPT_writeBothFramAndRamBit(ucOptionIdxPair, 1);


silentSetOptionExit:
	return;

	}/* END: vMODOPT_cmdSilentSetOption() */







/*****************  vMODOPT_cmdClrOption()  **********************************
*
*
*
******************************************************************************/
void vMODOPT_cmdClrOption(
		void
		)
	{
	signed char cOptionNum;

	/* SHOW WHATS OUT THERE */
	vMODOPT_showAllRamOptionBits();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_sout("OptID: ", 7);
	cOptionNum = cNUMCMD_getCmdNum(MAX_OPTPAIR_VALUE);
	if(cOptionNum < 0)
		{
		goto clrOptionExit;
		}

	/* ITS VALID -- SET THE OPTION */
	vMODOPT_cmdSilentClrOption((uchar)cOptionNum);

	/* SHOW THE NEW OPTION STS */
	vMODOPT_showAllRamOptionBits();
	vSERIAL_dash(10);
	vSERIAL_crlf();


clrOptionExit:
	return;

	}/* END: vMODOPT_cmdClrOption() */






/***************  vMODOPT_cmdSilentClrOption()  ******************************
*
*
*
******************************************************************************/

void vMODOPT_cmdSilentClrOption(
		uchar ucOptionIdxPair
		)
	{
	uchar ucByteIdx;
	uchar ucBitIdx;

	/* UNPACK THE OPTION IDX PAIR */
	ucBitIdx  = ucOptionIdxPair & 0x7;
	ucByteIdx = ucOptionIdxPair >> 3;

	#if 0
	vSERIAL_sout("\r\nByte= ", 8);
	vSERIAL_HB8out(ucByteIdx);
	vSERIAL_sout(",  Bit= ", 8);
	vSERIAL_HB8out(ucBitIdx);
	vSERIAL_crlf();
	#endif

	/* RANGE CHECK THE INDEX */
	if(ucBitIdx >= 8) goto silentClrOptionExit;
	if(ucByteIdx >= OPTION_BYTE_COUNT) goto silentClrOptionExit;

	/* SET THE OPTION BIT */
	vMODOPT_writeBothFramAndRamBit(ucOptionIdxPair, 0);

silentClrOptionExit:
	return;

	}/* END: vMODOPT_cmdSilentClrOption() */






/*****************  vMODOPT_cmdSetRole()  ************************************
*
*
*
******************************************************************************/
void vMODOPT_cmdSetRole(
		void
		)
	{
	char cRoleIdx;

	/* SHOW WHATS OUT THERE */
	vSERIAL_sout("CurRole= ", 9);
	vMODOPT_showCurRole();
	vSERIAL_sout("\r\nRoles are:\r\n\nID NAME\r\n", 24);
	vMODOPT_showAllRomOptionTblNames();
//	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_sout("RoleID: ", 8);
	cRoleIdx = cNUMCMD_getCmdNum(NUMBER_OF_ROLES);
	if(cRoleIdx <= 0)
		{
		vSERIAL_sout("Abrt\r\n", 6);
		goto setRoleExit;
		}

	/* ITS VALID -- ADD IT TO THE SRAM TABLE */
	vMODOPT_cmdSilentSetRole(cRoleIdx);

	/* SHOW THE NEW OPTION */
	vSERIAL_sout("NewRole= ", 9);
	vMODOPT_showCurRole();
	vSERIAL_crlf();


setRoleExit:
	return;

	}/* END: vMODOPT_cmdSetRole() */




/*****************  vMODOPT_cmdSilentSetRole()  ******************************
*
*
*
******************************************************************************/
void vMODOPT_cmdSilentSetRole(
		char cRoleIdx
		)
	{
	/* DO RANGE CHECK ON ROLE NUMBER */
	if(cRoleIdx <= 0) goto silentSetRoleExit;
	if(cRoleIdx >= NUMBER_OF_ROLES) goto silentSetRoleExit;

	/* SET FRAM OPTION FLAGS */
	vMODOPT_copyAllRomOptionsToFramOptions((uchar)cRoleIdx);

	/* SET RAM OPTION FLAGS */
	vMODOPT_copyAllFramOptionsToRamOptions();

//	/* OVERWRITE THE STBLKS IN FRAM WITH THE NEW ROLE DEFAULTS */
//	vL2FRAM_writeDefaultStartBlkList();
	ucTask_Init();

silentSetRoleExit:
	return;

	}/* END: vMODOPT_cmdSilentSetRole() */




/*-------------------------------  MODULE END  ------------------------------*/
