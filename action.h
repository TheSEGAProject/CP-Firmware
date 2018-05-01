//
///***************************  ACTION.H  ****************************************
//*
//* Header for ACTION routine pkg
//*
//* NOTE: See Main.h for the disk layout map.
//*
//*
//*
//* V1.00 10/04/2003 wzr
//*	started
//*
//******************************************************************************/
//
//#ifndef ACTION_H_INCLUDED
//	#define ACTION_H_INCLUDED
//#include "std.h"
// #define BATT_CHK_REPORT_MODE 1	//do batt chk and report from routine
// #define BATT_CHK_SILENT_MODE 0	//do batt chk and say nothing
//
//
///*----------------------Action Flags--------------------------------*/
//#define RADIOCOMM	0x01
//#define GETSPDATA	0x02
//
//
///*--------------------  EVENT ACTION LIST  ----------------------------------*/
//
////ID for CP board processor
//#define CP_ProcessorID	0
//
///**************************  ACTION 0  ***************************************/
// #define E_ACTN_SLEEP				0
//  #define E_ACTN_SLEEP_UL			((ulong) E_ACTN_SLEEP)
//
//  #define STS_LFACT_SLEEP			0
//
//  #define USE_FLAGS_SLEEP  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2)
//
//  #define SLEEP_WHO_CAN_RUN			RBIT_ALL
//
//	#define SENSE_ID_SLEEP			0x0000
//
///**************************  ACTION 1  ***************************************/
// #define E_ACTN_BATT_SENSE			1	//sample the battery
//
//  #ifdef COMPILE_FOR_WIZ2_DUKETEST
//		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)  256) // 1 frame
//  #endif
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_BATT_SENSE_I
//		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int) 1800)	//30 Min
////		#define SAMPLE_INTERVAL_BATT_SENSE_I	 ((int) 5400)	//1.5 Hrs  //default
//  #endif
//
//  #define STS_LFACT_BATT_SENSE (3600 / SAMPLE_INTERVAL_BATT_SENSE_I)
//
//  #define HR0_TO_SAMPLE0_BATT_SENSE_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_BATT_SENSE_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_BATT_SENSE  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT )
//
//  #define BATT_SENSE_WHO_CAN_RUN	RBIT_ALL
//
//	#define SENSE_ID_BATT_SENSE			0x0001
//
///**************************  ACTION 2  ***************************************/
// #define E_ACTN_RUN_SCHEDULER		2	//Run the scheduler
//
//  #define STS_LFACT_SCHEDULER		0
//
//  #define USE_FLAGS_SCHEDULER  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2| F_USE_START_OF_SLOT )
//
//  #define SCHEDULER_WHO_CAN_RUN	RBIT_ALL
//
//	#define SENSE_ID_SCHEDULER			0x0002
//
//
///**************************  ACTION 3  ***************************************/
// #define E_ACTN_SOM2				3	//Send the OM2 packets
//
//  #define STS_LFACT_SOM2			0
//
//  #define USE_FLAGS_SOM2   (F_USE_EXACT_SLOT | F_USE_THIS_RAND | F_USE_NO_MAKE_OM2 | F_USE_MIDDLE_OF_SLOT)
//
//  #define SENSE_ACT_SOM2_VAL ((ulong)(0))
//
//  #define SOM2_WHO_CAN_RUN	RBIT_SENDERS
//
//	#define	SENSE_ID_SOM2			0x0003
//
///**************************  ACTION 4  ***************************************/
// #define E_ACTN_ROM2				4	//Receive the OM2 packets
//
//  #define STS_LFACT_ROM2			0
//
//  #define USE_FLAGS_ROM2  (F_USE_EXACT_SLOT | F_USE_THIS_RAND | F_USE_NO_MAKE_OM2 | F_USE_MIDDLE_OF_SLOT)
//
//  #define ROM2_WHO_CAN_RUN	RBIT_RECEIVERS
//
//	#define SENSE_ID_ROM2			0x0004
//
///**************************  ACTION 5  *************************************/
// #define E_ACTN_DO_MOVE_SRAM_TO_FLASH		5	//Move_SRAM_to_FLASH
// 												//(usually masters only)
// 												//copies SRAM to FLASH &
//												//deletes SRAM
//
////	#define SAMPLE_INTERVAL_MOVE_SRAM_TO_FLASH_I	((int)128) // 2 times/frame
//	#define SAMPLE_INTERVAL_MOVE_SRAM_TO_FLASH_I	((int)256) // 1 time per frame
//
//	#define STS_LFACT_MOVE_SRAM_TO_FLASH (3600 / SAMPLE_INTERVAL_MOVE_SRAM_TO_FLASH_I)
//
//	#define HR0_TO_SAMPLE0_MOVE_SRAM_TO_FLASH_IN_SEC	 0
//
//  #define USE_FLAGS_MOVE_SRAM_TO_FLASH 		 	(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2)
//
//	#define MOVE_SRAM_TO_FLASH_WHO_CAN_RUN	 	(RBIT_HUB| RBIT_SAMPLEHUB)
//
// 	#define SENSE_ID_MOVE_FRAM_TO_FLASH				0x0005
//
///**************************  ACTION 6  *************************************/
//  #define E_ACTN_SDC4				6	//send DC4 packets
//
//  #define STS_LFACT_SDC4		0
//
//  #define USE_FLAGS_SDC4 		(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 | F_USE_MIDDLE_OF_SLOT)
//
//  #define SDC4_WHO_CAN_RUN	RBIT_RECEIVERS
//
//  #define SENSE_ID_SDC4			0x0006
//
///**************************  ACTION 7  *************************************/
// #define E_ACTN_RDC4				7	//Receive DC4 packets
//
//  #define STS_LFACT_RDC4			0
//
//
//  #define USE_FLAGS_RDC4  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 | F_USE_FULL_SLOT)
//
//  #define RDC4_WHO_CAN_RUN	RBIT_SENDERS
//
//	#define SENSE_ID_RDC4			0x0007
//
//
///**************************  ACTION 8  *************************************/
// #define E_ACTN_REPORT				8	//report is currently disabled as was considered too energy intensive
//
//  #ifdef COMPILE_FOR_WIZ2_DUKETEST
//		#define SAMPLE_INTERVAL_REPORT_I	 ((int)  256)	//1 frame
//  #endif
//
//  /* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_REPORT_I
////		#define SAMPLE_INTERVAL_REPORT_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_REPORT_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_REPORT_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_REPORT_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_REPORT_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_REPORT_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_REPORT (3600 / SAMPLE_INTERVAL_REPORT_I)
//
//  #define HR0_TO_SAMPLE0_REPORT_IN_SEC ( 0 )
//
//
//  #define USE_FLAGS_REPORT  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT )
//
//  #define REPORT_WHO_CAN_RUN	RBIT_ALL
//
//	#define SENSE_ID_REPORT		0x0008
//
///**************************  ACTION 9  ***************************************/
// #define E_ACTN_SP1_TRAN0_SAMPLE	9	//SP board in location 1 transducer 0
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP1_TRAN0_I
//		#define SAMPLE_INTERVAL_SP1_TRAN0_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP1_TRAN0_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN0_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP1_TRAN0_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP1_TRAN0_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN0_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN0_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP1_TRAN0 (3600 / SAMPLE_INTERVAL_SP1_TRAN0_I)
//
//  #define HR0_TO_SAMPLE0_SP1_TRAN0_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP1_TRAN0_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP1_TRAN0  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP1_TRAN0_WHO_CAN_RUN		RBIT_SAMPLERS
//
//	#define SENSE_ID_SP1_TRAN0		0x0101
//
///**************************  ACTION 10  ***************************************/
// #define E_ACTN_SP1_TRAN1_SAMPLE	10	//SP board in location 1 transducer 1
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP1_TRAN1_I
////		#define SAMPLE_INTERVAL_SP1_TRAN1_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP1_TRAN1_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN1_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP1_TRAN1_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP1_TRAN1_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN1_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SP1_TRAN1_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP1_TRAN1 (3600 / SAMPLE_INTERVAL_SP1_TRAN1_I)
//
//  #define HR0_TO_SAMPLE0_SP1_TRAN1_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP1_TRAN1_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP1_TRAN1  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP1_TRAN1_WHO_CAN_RUN		RBIT_SAMPLERS
//
//	#define SENSE_ID_SP1_TRAN1		0x0102
//
///**************************  ACTION 11  ***************************************/
// #define E_ACTN_SP1_TRAN2_SAMPLE	11	//SP board in location 1 transducer 2
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP1_TRAN2_I
////		#define SAMPLE_INTERVAL_SP1_TRAN2_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP1_TRAN2_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN2_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP1_TRAN2_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP1_TRAN2_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN2_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SP1_TRAN2_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP1_TRAN2 (3600 / SAMPLE_INTERVAL_SP1_TRAN2_I)
//
//  #define HR0_TO_SAMPLE0_SP1_TRAN2_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP1_TRAN2_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP1_TRAN2  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP1_TRAN2_WHO_CAN_RUN		RBIT_SAMPLERS
//
//	#define SENSE_ID_SP1_TRAN2		0x0104
//
///**************************  ACTION 12  ***************************************/
// #define E_ACTN_SP1_TRAN3_SAMPLE	12	//SP board in location 1 transducer 3
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP1_TRAN3_I
////		#define SAMPLE_INTERVAL_SP1_TRAN3_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP1_TRAN3_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN3_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP1_TRAN3_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP1_TRAN3_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP1_TRAN3_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SP1_TRAN3_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP1_TRAN3 (3600 / SAMPLE_INTERVAL_SP1_TRAN3_I)
//
//  #define HR0_TO_SAMPLE0_SP1_TRAN3_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP1_TRAN3_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP1_TRAN3  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP1_TRAN3_WHO_CAN_RUN		RBIT_SAMPLERS
//
//	#define SENSE_ID_SP1_TRAN3		0x0108
//
///**************************  ACTION 13  ***************************************/
// #define E_ACTN_SP2_TRAN0_SAMPLE	13	//SP board in location 2 transducer 0
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP2_TRAN0_I
//		#define SAMPLE_INTERVAL_SP2_TRAN0_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP2_TRAN0_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN0_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP2_TRAN0_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP2_TRAN0_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN0_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN0_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP2_TRAN0 (3600 / SAMPLE_INTERVAL_SP2_TRAN0_I)
//
//  #define HR0_TO_SAMPLE0_SP2_TRAN0_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP2_TRAN0_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP2_TRAN0  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP2_TRAN0_WHO_CAN_RUN		RBIT_SAMPLERS
//
//	#define SENSE_ID_SP2_TRAN0		0x0201
//
///**************************  ACTION 14  ***************************************/
// #define E_ACTN_SP2_TRAN1_SAMPLE	14	//SP board in location 2 transducer 1
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP2_TRAN1_I
////		#define SAMPLE_INTERVAL_SP2_TRAN1_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP2_TRAN1_I	 ((int)   60)	// 1 Frame
//		#define SAMPLE_INTERVAL_SP2_TRAN1_I	 ((int)   120)	// 2 Frame
////		#define SAMPLE_INTERVAL_SP2_TRAN1_I	 ((int)  256)	// 6 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN1_I	 ((int)  512)	// 12 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN1_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN1_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN1_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP2_TRAN1 (3600 / SAMPLE_INTERVAL_SP2_TRAN1_I)
//
//  #define HR0_TO_SAMPLE0_SP2_TRAN1_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP2_TRAN1_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP2_TRAN1  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP2_TRAN1_WHO_CAN_RUN		RBIT_SAMPLERS
//
//	#define SENSE_ID_SP2_TRAN1		0x0202
//
///**************************  ACTION 15  ***************************************/
// #define E_ACTN_SP2_TRAN2_SAMPLE	15	//SP board in location 2 transducer 2
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP2_TRAN2_I
////		#define SAMPLE_INTERVAL_SP2_TRAN2_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP2_TRAN2_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN2_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP2_TRAN2_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP2_TRAN2_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN2_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SP2_TRAN2_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP2_TRAN2 (3600 / SAMPLE_INTERVAL_SP2_TRAN2_I)
//
//  #define HR0_TO_SAMPLE0_SP2_TRAN2_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP2_TRAN2_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP2_TRAN2  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP2_TRAN2_WHO_CAN_RUN		RBIT_SAMPLERS
//
//	#define SENSE_ID_SP2_TRAN2		0x0204
//
///**************************  ACTION 16  ***************************************/
// #define E_ACTN_SP2_TRAN3_SAMPLE	16	//SP board in location 2 transducer 3
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP2_TRAN3_I
////		#define SAMPLE_INTERVAL_SP2_TRAN3_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP2_TRAN3_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN3_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP2_TRAN3_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP2_TRAN3_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP2_TRAN3_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SP2_TRAN3_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP2_TRAN3 (3600 / SAMPLE_INTERVAL_SP2_TRAN3_I)
//
//  #define HR0_TO_SAMPLE0_SP2_TRAN3_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP2_TRAN3_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP2_TRAN3  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP2_TRAN3_WHO_CAN_RUN		RBIT_SAMPLERS
//
//	#define SENSE_ID_SP2_TRAN3		0x0208
//
//
///**************************  ACTION 17  ***************************************/
// #define E_ACTN_SP3_TRAN0_SAMPLE	17	//SP board in location 3 transducer 0
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP3_TRAN0_I
//		#define SAMPLE_INTERVAL_SP3_TRAN0_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP3_TRAN0_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN0_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP3_TRAN0_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP3_TRAN0_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN0_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN0_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP3_TRAN0 (3600 / SAMPLE_INTERVAL_SP3_TRAN0_I)
//
//  #define HR0_TO_SAMPLE0_SP3_TRAN0_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP3_TRAN0_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP3_TRAN0  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP3_TRAN0_WHO_CAN_RUN		RBIT_SAMPLERS
//
//  #define SENSE_ID_SP3_TRAN0		0x0301
//
//
///**************************  ACTION 18  ***************************************/
// #define E_ACTN_SP3_TRAN1_SAMPLE	18	//SP board in location 3 transducer 1
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP3_TRAN1_I
////		#define SAMPLE_INTERVAL_SP3_TRAN1_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP3_TRAN1_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN1_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP3_TRAN1_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP3_TRAN1_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN1_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SP3_TRAN1_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP3_TRAN1 (3600 / SAMPLE_INTERVAL_SP3_TRAN1_I)
//
//  #define HR0_TO_SAMPLE0_SP3_TRAN1_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP3_TRAN1_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP3_TRAN1  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP3_TRAN1_WHO_CAN_RUN		RBIT_SAMPLERS
//
//  #define SENSE_ID_SP3_TRAN1		0x0302
//
//
///**************************  ACTION 19  ***************************************/
// #define E_ACTN_SP3_TRAN2_SAMPLE	19	//SP board in location 3 transducer 2
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP3_TRAN2_I
////		#define SAMPLE_INTERVAL_SP3_TRAN2_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP3_TRAN2_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN2_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP3_TRAN2_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP3_TRAN2_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN2_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SP3_TRAN2_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP3_TRAN2 (3600 / SAMPLE_INTERVAL_SP3_TRAN2_I)
//
//  #define HR0_TO_SAMPLE0_SP3_TRAN2_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP3_TRAN2_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP3_TRAN2  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP3_TRAN2_WHO_CAN_RUN		RBIT_SAMPLERS
//
//  #define SENSE_ID_SP3_TRAN2		0x0304
//
///**************************  ACTION 20  ***************************************/
// #define E_ACTN_SP3_TRAN3_SAMPLE	20	//SP board in location 3 transducer 3
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP3_TRAN3_I
////		#define SAMPLE_INTERVAL_SP3_TRAN3_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP3_TRAN3_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN3_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP3_TRAN3_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP3_TRAN3_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP3_TRAN3_I	 ((int)  900)	//15 Min
//		#define SAMPLE_INTERVAL_SP3_TRAN3_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP3_TRAN3 (3600 / SAMPLE_INTERVAL_SP3_TRAN3_I)
//
//  #define HR0_TO_SAMPLE0_SP3_TRAN3_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP3_TRAN3_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP3_TRAN3  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP3_TRAN3_WHO_CAN_RUN		RBIT_SAMPLERS
//
//  #define SENSE_ID_SP3_TRAN3		0x0308
//
///**************************  ACTION 21  ***************************************/
// #define E_ACTN_SP4_TRAN0_SAMPLE	21	//SP board in location 4 transducer 0
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP4_TRAN0_I
//		#define SAMPLE_INTERVAL_SP4_TRAN0_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP4_TRAN0_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN0_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP4_TRAN0_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP4_TRAN0_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN0_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN0_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP4_TRAN0 (3600 / SAMPLE_INTERVAL_SP4_TRAN0_I)
//
//  #define HR0_TO_SAMPLE0_SP4_TRAN0_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP4_TRAN0_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP4_TRAN0  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP4_TRAN0_WHO_CAN_RUN		RBIT_SAMPLERS
//
//  #define SENSE_ID_SP4_TRAN0 0x0401
//
//
///**************************  ACTION 22  ***************************************/
// #define E_ACTN_SP4_TRAN1_SAMPLE	22	//SP board in location 4 transducer 1
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP4_TRAN1_I
//		#define SAMPLE_INTERVAL_SP4_TRAN1_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP4_TRAN1_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN1_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP4_TRAN1_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP4_TRAN1_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN1_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN1_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP4_TRAN1 (3600 / SAMPLE_INTERVAL_SP4_TRAN1_I)
//
//  #define HR0_TO_SAMPLE0_SP4_TRAN1_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP4_TRAN1_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP4_TRAN1  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP4_TRAN1_WHO_CAN_RUN		RBIT_SAMPLERS
//
//  #define SENSE_ID_SP4_TRAN1 0x0402
//
//
///**************************  ACTION 23  ***************************************/
// #define E_ACTN_SP4_TRAN2_SAMPLE	23	//SP board in location 4 transducer 2
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP4_TRAN2_I
//		#define SAMPLE_INTERVAL_SP4_TRAN2_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP4_TRAN2_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN2_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP4_TRAN2_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP4_TRAN2_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN2_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN2_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP4_TRAN2 (3600 / SAMPLE_INTERVAL_SP4_TRAN2_I)
//
//  #define HR0_TO_SAMPLE0_SP4_TRAN2_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP4_TRAN2_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP4_TRAN2  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP4_TRAN2_WHO_CAN_RUN		RBIT_SAMPLERS
//
//  #define SENSE_ID_SP4_TRAN2 0x0404
//
///**************************  ACTION 24  ***************************************/
// #define E_ACTN_SP4_TRAN3_SAMPLE	24	//SP board in location 4 transducer 3
//
//	/* DEFAULT SAMPLE INTERVAL */
//  #ifndef SAMPLE_INTERVAL_SP4_TRAN3_I
//		#define SAMPLE_INTERVAL_SP4_TRAN3_I	 ((int)   30)	//30 Sec
////		#define SAMPLE_INTERVAL_SP4_TRAN3_I	 ((int)   60)	// 1 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN3_I	 ((int)  256)	// 1 Frame
////		#define SAMPLE_INTERVAL_SP4_TRAN3_I	 ((int)  512)	// 2 Frames
////		#define SAMPLE_INTERVAL_SP4_TRAN3_I	 ((int)  540)	// 9 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN3_I	 ((int)  900)	//15 Min
////		#define SAMPLE_INTERVAL_SP4_TRAN3_I	 ((int) 1800)	//30 Min   //default
//  #endif
//
//  #define STS_LFACT_SP4_TRAN3 (3600 / SAMPLE_INTERVAL_SP4_TRAN3_I)
//
//  #define HR0_TO_SAMPLE0_SP4_TRAN3_IN_SEC ( 0 )
////#define HR0_TO_SAMPLE0_SP4_TRAN3_IN_SEC (600)	//10 MIN
//
//  #define USE_FLAGS_SP4_TRAN3  (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//
//  #define SP4_TRAN3_WHO_CAN_RUN		RBIT_SAMPLERS
//
//  #define E_ACTN_MAX_NUM_IDX			(24+1)
//
//  #define SENSE_ID_SP4_TRAN3 0x0408
//
///********************************  NOTE  *************************************
//*
//* NOTE: WHEN ADDING A NEW ACTION, YOU MUST:
//*
//* 1. Write routine to perform the event (this ".C" module).
//* 2. Provide defines above for the action (this ".H" module).
//* 3. Add new entry in ACTION vector tbl vEventPtrArray[] (this ".C" module).
//* 4. Add new entry in RTS table cpActionName[] (this ".C" module).
//* 5. Add new init action in vSTBL_setupInitialSchedTbls() (STBL ".C" module).
//*
//*
//******************************************************************************/
//
//
// /* ROUTINE DEFINITIONS */
//
//void vACTION_dispatch_to_event(
//		unsigned char ucNSTtblNum,		//table num (0 or 1)
//		unsigned char ucNSTslotNum		//slot num in table
//		);
//
//void vUpdateCurrentTaskStructure(
//		uchar ucActionNum);
//
//
//void vACTION_read_sensors_into_buffer(//first 0 sensor num ends read
//		uchar ucSensorNum[4]	//Sensors [0]=first, [3]=last
//		);
//
//void vACTION_showActionName(
//		uchar ucActionNum			//takes an action number
//		);
//
//void vACTION_showStblActionName(
//		uchar ucStblIdx				//takes an Stbl index
//		);
//
//void vACTION_showBattReading(
//		void
//		);
//
//void vACTION_do_Batt_Sense(
//		void
//		);
//
//void vACTION_Wake_SP_and_SEND_CMD(
//		void
//		);
//
//void vACTION_Request_SP_Data(
//		void
//		);
//
//void vACTION_showSlotEntries(
//		uchar *ucSlot
//		);
//
//void vAction_do_Nothing(
//		void
//		);
//
//
//#endif /* ACTION_H_INCLUDED */
//
///* --------------------------  END of MODULE  ------------------------------- */
