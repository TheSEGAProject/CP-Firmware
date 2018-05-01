/***************************  TIME.H  ****************************************
 *
 * Header for TIME routine pkg
 *
 * V1.00 3/3/2004 wzr
 *	started
 *
 ******************************************************************************/

/* NOTE: Read overview in TIME.c */

#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

#include "std.h"

#define FRAME_SLOT_TIME		1
#define HEX_SEC_TIME		2
#define INT_SEC_TIME		3

#define FILL_LEAD_WITH_SPACES	TRUE
#define FILL_LEAD_WITH_ZEROS	FALSE

#define YES_RESET_TIME 1
#define NO_RESET_TIME  0

#define YES_SKP_ECLK_RAM_FLAG	1
#define NO_SKP_ECLK_RAM_FLAG	0

#define DEFAULT_HR_OFFSET_IN_SEC 0			//default hour offset
//Slot structure definitions
#define ENDOFSLOT						0x7FFF
#define SUBSLOT_ONE_END				0x2000
#define SUBSLOT_ONE_BUFFER_SIZE		0xC80	//100 ms
#define SUBSLOT_TWO_END				0x7000
#define SUBSLOT_TWO_BUFFER_SIZE		0x666 	//50 ms
#define SUBSLOT_THREE_END				0x7900
#define SUBSLOT_THREE_BUFFER_SIZE		0x666	//50 ms


//! \def T_START
//! \def T_STOP
//! \brief Start and stop timer definitions
#define T_START		0x01
#define T_STOP		0x00

//! \def GENERAL_ALARM_BIT
//! \def SUBSLOT_WARNING_ALARM_BIT
//! \def SUBSLOT_END_ALARM_BIT
//! \brief These bits are used to tell the timer module what flags to check
#define GENERAL_ALARM_BIT						0x01
#define SUBSLOT_WARNING_ALARM_BIT		0x02
#define SUBSLOT_END_ALARM_BIT				0x04
#define LPM_DELAY_ALARM_BIT					0x08
#define SUBSLOT_ALARMS 	(SUBSLOT_WARNING_ALARM_BIT | SUBSLOT_END_ALARM_BIT)
#define ALLSLOT_ALARMS	(GENERAL_ALARM_BIT | SUBSLOT_WARNING_ALARM_BIT | SUBSLOT_END_ALARM_BIT)
#define ALL_ALARMS 			(GENERAL_ALARM_BIT | SUBSLOT_WARNING_ALARM_BIT | SUBSLOT_END_ALARM_BIT | LPM_DELAY_ALARM_BIT)

/* ROUTINE DEFINITIONS */

void vTIME_init(void);

long lTIME_getSysTimeAsLong(void);

void vTIME_copySysTimeToBytes(uchar *ucpToPtr //4 bytes
    );

void vTIME_setSysTimeFromLong(ulong lNewSysTime);

void vTIME_setSysTimeFromClk2(void);

/*---------------------------------*/

uchar ucTIME_getECLKsts( //0=noPing, 1=OfflineFlg, 2=notTicking, 3=OK
    uchar ucSkpRamFlag //YES_SKP_ECLK_RAM_FLAG, NO_SKP_ECLK_RAM_FLAG
    );

long lTIME_getECLKtimeAsLong(void);

void vTIME_setECLKtimeFromLong(long lNewECLKtime);

/*---------------------------------*/

uint uiTIME_getSubSecAsUint(void);

void vTIME_copySubSecToBytes(uchar *ucpToPtr);

void vTIME_setSubSecFromUint(uint uiNewSubSec);

void vTIME_setSubSecFromBytes(uchar *ucpFromPtr);


uint uiTime_GetSlotCompareReg(void);
void vTime_SetSlotCompareReg(uint uiNewCompareVal);

/*---------------------------------*/

void vTIME_copyWholeSysTimeToBytesInDtics(uchar *ucpToPtr //6 bytes (Ret = Disjoint Tics)
    );

void vTIME_copyWholeSysTimeToBytesInLtics(uchar *ucpToPtr //6 bytes (Ret = Linear Tics)
    );

void vTIME_setWholeSysTimeFromBytesInDtics(uchar *ucpFromPtr //6 bytes ptr (Src = Disjoint Tics)
    );

void vTIME_showWholeSysTimeInDuS( //shown in decimal uS form
    uchar ucCRLF_termFlag //YES_CRLF, NO_CRLF
    );

/*---------------------------------*/

void vTIME_showWholeTimeInDuS( //shown in Decimal uS form
    uchar *ucpTimeArray, //6 bytes (in Disjoint time format)
    uchar ucCRLF_termFlag //YES_CRLF, NO_CRLF
    );

void vTIME_showWholeTimeDiffInDuS( //shown in Decimal uS form
    uchar *ucpBegTimeArray, //6 bytes ptr (Disjoint time format)
    uchar *ucpEndTimeArray, //6 bytes ptr (Disjoint time format)
    uchar ucCRLF_termFlag //YES_CRLF, NO_CRLF
    );

ulong ulTIME_computeTimeDiffInLtics(uchar *ucpBegTime_inDtics, //6 bytes (Disjoint time format)
    uchar *ucpEndTime_inDtics, //6 bytes (Disjoint time format)
    uchar *ucpDiffTime_inLtics //6 bytes (Linear Time format)
    );

void vTIME_convertDticsToLtics(uchar *ucpSrcTimeArray, //6 bytes (in Disjoint time format)
    uchar *ucpLinearTimeArray //6 bytes (in Linear time format)
    );

/*---------------------------------*/

long lTIME_getAlarmAsLong(void);

// void vTIME_copyAlarmToBytes(
//		uchar *ucpToPtr
//		);

void vTIME_setAlarmFromLong(long lNewAlarm);

void vTIME_setAlarmFromBytes(uchar *ucpFromPtr);

uchar ucTime_SetSubslotAlarm(uint uiSubSlotEnd, uint uiBufferSize);

uchar ucTimeCheckForAlarms(uchar ucAlarmBits);

/*---------------------------------*/

int iTIME_getHr0_to_sysTim0_asInt(void);

void vTIME_copyHr0_to_sysTim0_toBytes(uchar *ucpToPtr);

/*---------------------------------*/

long lTIME_get_Hr_From_SysTim_InSecs(long lUpTimeSecs);

int iTIME_get_HrOffset_From_SysTim_InSecs(long lUpTimeSecs);

/*---------------------------------*/

long lTIME_getClk2AsLong(void);

// void vTIME_copyClk2ToBytes(
//		uchar *ucpToPtr
//		);

void vTIME_setClk2FromLong(long lNewClk2);

void vTIME_setClk2FromBytes(uchar *ucpFromPtr);

/*---------------------------------*/

void vTIME_copyWholeClk2ToBytes(uchar *ucpToPtr //6 bytes (Ret = Disjoint time format)
    );

void vTIME_setWholeClk2FromBytes(uchar *ucpFromPtr //6 bytes ptr (Src = Disjoint time format)
    );

void vTIME_showWholeClk2( //shown in uS decimal form
    uchar ucCRLF_termFlag //YES_CRLF, NO_CRLF
    );

/*---------------------------------*/

uint uiTIME_getT3AsUint(void);

uchar ucTIME_setT3AlarmToSecMinus200ms(
// RET:0=OK, 1=Too Late, 2=Too Close, 3=Too Early
    long lSlotEndTime);

/*---------------------------------*/

long lTIME_convertTimeToLinearSlotNum(long lTime);

long lTIME_getFrameNumFromTime(long lTime);

long lTIME_getSlotNumFromTime(long lTime);

void vTIME_showTime(long lTime, uchar ucTimeFormFlag, //FRAME_SLOT_TIME, HEX_SEC_TIME, INT_SEC_TIME
    uchar ucCRLF_flag //NO_CRLF,YES_CRLF
    );

/*---------------------------------*/

void vTime_LatencyTimer(uchar ucState);
void vTime_SetLinkSlotAlarm(unsigned char ucState);
void vTime_SetLPM_DelayAlarm(unsigned char ucState, unsigned long ulDelay);
#endif /* TIME_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
