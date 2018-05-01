
/***************************  L2FRAM.H  **************************************
 *
 * Header for L2FRAM routine pkg
 *
 *
 * V2.07 12/20/2006 wzr
 *	Added SENSE_ACT bytes to FRAM STARTUP BLKS.
 *
 * V2.06 04/28/2006 wzr
 *	Renamed FRAM_SYS_FLAGS_ADDR to be FRAM_CONFIG_BYTE_ADDR to reflect the
 *	fact that this location contains a copy of the loaded (ROM) config byte.
 *	Added FRAM_SYS_FLAGS_ADDR to save value of data streaming.
 *
 * V1.00 12/28/2004 wzr
 *	started
 *
 ******************************************************************************/

#ifndef L2FRAM_H_INCLUDED
#define L2FRAM_H_INCLUDED

#define YES_ABBREVIATE		1
#define  NO_ABBREVIATE		0

#include "std.h"			//standard definitions
#include "SD_Card.h"

#define FRAM_VERSION_HI		0x02
#define FRAM_VERSION_LO		0x09
#define FRAM_VERSION (((uint)FRAM_VERSION_HI<<8) | ((uint)FRAM_VERSION_LO))

#define FRAM_TEST_ADDR 			6	//4 bytes
#define FRAM_TEST_ADDR_XI		6
#define FRAM_TEST_ADDR_HI		7
#define FRAM_TEST_ADDR_MD		8
#define FRAM_TEST_ADDR_LO		9

#define FRAM_SDCARD_BLOCK_NUM_ADDR			10	//4 bytes
#define FRAM_SDCARD_BUFF_START_PTR_ADDR		14	//2 bytes
#define FRAM_SDCARD_BUFF_END_PTR_ADDR		16	//2 bytes
#define FRAM_TIME_SAVE_AREA_ADDR			18	//4 bytes
#define FRAM_REBOOT_COUNT_ADDR				22	//2 bytes
#define FRAM_USER_ID_ADDR					24	//2 bytes

#define FRAM_OPTION_BYTE_0_ADDR			26	// Option byte array
#define FRAM_OPTION_BYTE_1_ADDR			27
#define FRAM_OPTION_BYTE_2_ADDR			28
#define FRAM_OPTION_BYTE_3_ADDR			29
#define FRAM_OPTION_BYTE_4_ADDR			30
#define FRAM_OPTION_IDX_ADDR				31

//! \def FRAM_STATE_ON_SHUTDOWN_ADDR
//! \brief This location provides information on how the WiSARD shutdown
#define FRAM_STATE_ON_SHUTDOWN_ADDR	32	// 1-byte

//! \def FRAM_RPT_PRTY_ADDR
//! \brief Reporting Priority address
#define FRAM_RPT_PRTY_ADDR			33 // 1-byte
#define FRAM_ST_BLK_COUNT_ADDR		34	//1 byte
#define FRAM_ST_BLK_0_ADDR				35	//Blk = 22 bytes long
#define FRAM_ST_BLK_SIZE					22
#define FRAM_MAX_TSB_COUNT 				MAXNUMTASKS // (defined in task manager)
#define FRAM_ST_BLK_TASK_IDX			0	//index in task manager (1 byte)
#define FRAM_ST_BLK_TASK_ID				1	//(2 bytes)
#define FRAM_ST_BLK_FLAGS					3	//(2 bytes)
#define FRAM_ST_BLK_TASK_STATE		5	//(1 bytes)
#define FRAM_ST_BLK_PARAM1				6	//(4 bytes)
#define FRAM_ST_BLK_PARAM2				10	//(4 bytes)
#define FRAM_ST_BLK_PARAM3				14	//(4 bytes)
#define FRAM_ST_BLK_PARAM4				18	//(4 bytes)

#define FRAM_LAST_ST_BLK_ADDR			(FRAM_ST_BLK_0_ADDR + FRAM_MAX_TSB_COUNT*FRAM_ST_BLK_SIZE)//1574

/*************************  NOTE  ********************************************
 *
 * The trigger areas is indexed by device number.  At the time of this writing
 * there were 76 devices.  The trigger area is allocated for 80 devices.
 * Each device takes 2 bytes of storage.
 *
 ******************************************************************************/

#define FRAM_Y_TRIG_AREA_BEG_ADDR			(FRAM_LAST_ST_BLK_ADDR + 1)//1575

#define FRAM_Y_TRIG_AREA_END_ADDR			(FRAM_Y_TRIG_AREA_BEG_ADDR + 2*80)//1735

/**
 * FRAM is used to temporarily store blocks that will eventually be
 * written to the SD card.  This is required since minimal block
 * lengths of SD cards are typically 512 bytes.
 *
 **/

//!	\def FRAM_SD_CARD_BUFF_SIZE
//! \brief Size of SD card buffer
#define FRAM_SD_CARD_BUFF_SIZE				SD_CARD_BLOCKLEN

//!	\def FRAM_SD_CARD_BUFF_BEG_ADDR
//! \brief Starting address of the SD card buffer
#define	FRAM_SD_CARD_BUFF_BEG_ADDR		(FRAM_Y_TRIG_AREA_END_ADDR	+ 1)//1737

//!	\def FRAM_SD_CARD_BUFF_END_ADDR
//! \brief Ending address of the SD card buffer
#define FRAM_SD_CARD_BUFF_END_ADDR		(FRAM_SD_CARD_BUFF_BEG_ADDR + FRAM_SD_CARD_BUFF_SIZE)//2249

//!	\def FRAM_SD_CARD_OVRFLO_SIZE
//! \brief Size of the overflow buffer
#define FRAM_SD_CARD_OVRFLO_SIZE 			448

//!	\def FRAM_SD_CARD_OVRFLO_END_ADDR
//! \brief Starting address of the overflow buffer
#define FRAM_SD_CARD_OVRFLO_BEG_ADDR 	(FRAM_SD_CARD_BUFF_END_ADDR + 1) //2250

//!	\def FRAM_SD_CARD_BUFF_END_ADDR
//! \brief Ending address of the overflow buffer
#define FRAM_SD_CARD_OVRFLO_END_ADDR	(FRAM_SD_CARD_OVRFLO_BEG_ADDR +	FRAM_SD_CARD_OVRFLO_SIZE)//2762

#define FRAM_CHK_REPORT_MODE	1
#define FRAM_CHK_SILENT_MODE	0

#define FRAM_LOCK						1
#define FRAM_UNLOCK					0

/* ROUTINE DEFINITIONS */

/*--------------------------------*/
void vL2FRAM_LockAllMemory(void);
void vL2FRAM_test(void);

uchar ucL2FRAM_chk_for_fram(uchar ucReportMode //FRAM_CHK_SILENT_MODE, FRAM_CHK_REPORT_MODE
    );

void vL2FRAM_format_fram(void);

uint uiL2FRAM_chk_for_fram_format(void);

void vL2FRAM_force_fram_unformat(void);

void vL2FRAM_writeDefaultStartBlkList(void);

/*--------------------------------*/

uchar ucL2FRAM_GetStateOnShutdown(void);
void vL2FRAM_SetStateOnShutdown(uchar ucState);

/*--------------------------------*/

uint uiL2FRAM_get_version_num(void);

/*--------------------------------*/

uchar ucL2FRAM_WriteReportToSDCardBuff(volatile uchar * p_ucReport, uchar ucLength);
void vL2FRAM_ReadSDCardBuffer(uchar * p_ucBlock);
ulong ulL2FRAM_GetSDCardBlockNum(void);
void vL2FRAM_SetSDCardBlockNum(ulong ulBlockNum);
void vL2FRAM_SDCardOvrflowToBuff(void);
void vL2FRAM_IncrementSDCardBlockNum(void);
ulong ulL2FRAM_GetLastSDCardBlockNum(void);

/*--------------------------------*/

void vL2FRAM_stuffSavedTime(ulong ulSavedTimeVal);

ulong ulL2FRAM_getSavedTime(void);

/*--------------------------------*/

void vL2FRAM_writeRebootCount(uint uiNewRebootCountVal);

uint uiL2FRAM_getRebootCount(void);

void vL2FRAM_incRebootCount(void);

/*--------------------------------*/

uchar ucL2FRAM_isHub(void);

uchar ucL2FRAM_isSender(void);

uchar ucL2FRAM_isSampler(void);

uchar ucL2FRAM_isReceiver(void);

uchar ucL2FRAM_GetOptionByte(uchar ucOptionByteIdx, uchar *ucByteVal);
uchar ucL2FRAM_SetOptionByte(uchar ucOptionByteIdx, uchar ucByteVal);
/*--------------------------------*/

uint uiL2FRAM_getSnumLo16AsUint(void);

void vL2FRAM_copySnumLo16ToBytes(uchar *ucpToPtr);

uchar ucL2FRAM_getSnumMd8(void);

uchar ucL2FRAM_getSnumLo8(void);

void vL2FRAM_setSysID(uint uiSysID);

void vL2FRAM_showSysID(void);

/*-------------- Task State Block ------------------*/

uchar ucL2FRAM_getTSBTblCount(void);

ulong ulL2FRAM_getTSBEntryVal( //RET: TSB Entry Val
    uchar ucTSBNum, //blk number
    uchar ucTSBEntryIdx //index into the blk
    );

void vL2FRAM_putTSBEntryVal( //Stuff the Value int the TSB
    uchar ucTSBNum, //blk number
    uchar ucTSBEntryIdx, //index into the blk
    ulong ulVal //value to put
    );

uchar ucL2FRAM_findTSB( //Ret: TSB num,  255 if none
		uint uiTaskID);

uchar ucL2FRAM_findTSB_SN( //Ret: TSB num,  255 if none
		uint uiSerialNum);

void vL2FRAM_deleteTSB( //remove an entry from StBLk
    uchar ucTSBNum //TSB Num
    );

signed char cL2FRAM_addTSB( //RET: TSB num of new entry
		uchar ucTskIndex //task number to search for
		);

void vL2FRAM_showTSBTbl(void);

/*--------------------------------*/

void vL2FRAM_putYtriggerVal( //Stuff Val into FRAM trigger Area
    uchar ucSensorNum, uint uiYtriggerVal);

uint uiL2FRAM_getYtriggerVal( //ret trigger Val from FRAM
    uchar ucSensorNum);

void vL2FRAM_showYtriggerTbl(void);

void vL2FRAM_SetReportingPriority(uchar ucReportingPriority);
uchar ucL2FRAM_GetReportingPriority(void);
/*--------------------------------*/

#endif /* L2FRAM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
