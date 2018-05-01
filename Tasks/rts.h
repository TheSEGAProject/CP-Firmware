
/***************************  RTS.H  ****************************************
*
* Header for RTS (REAL TIME SCHEDULER) routine pkg
*
*
* V1.00 03/02/2005 wzr
*	started
*
******************************************************************************/

#ifndef RTS_H_INCLUDED
 #define RTS_H_INCLUDED

#include "std.h"


#define YES_SHOW_TCB	1
#define  NO_SHOW_TCB	0



/* SCHEDULER PRIORITYS, 7 = LOWEST, 0 = HIGHEST */
#define PRIORITY_0	0x00	//00000000
#define PRIORITY_1	0x20	//00100000
#define PRIORITY_2	0x40	//01000000
#define PRIORITY_3	0x60	//01100000
#define PRIORITY_4	0x80	//10000000
#define PRIORITY_5	0xA0	//10100000
#define PRIORITY_6	0xC0	//11000000
#define PRIORITY_7	0xE0	//11100000

#define PRIORITY_MAX_VAL	0xE0		//11100000
#define PRIORITY_MASK		0xE0		//11100000
#define PRIORITY_INC_VAL	0x20		//00100000


#define SCHED_FUNC_DORMANT_SLOT		0
#define SCHED_FUNC_SCHEDULER_SLOT	1
#define SCHED_FUNC_SDC4_SLOT			2
#define SCHED_FUNC_RDC4_SLOT			3
#define SCHED_FUNC_ALL_SLOT				4
#define SCHED_FUNC_OM_SLOT				5
#define SCHED_FUNC_INTERVAL_SLOT	6
#define SCHED_FUNC_LOADBASED_SLOT	7
#define SCHED_CMD_SLOT						8
#define SCHED_DNCNT_SLOT					9 // task scheduling based on a time parameter


#define SCHED_FUNC_MAX_COUNT		10
#define SCHED_FUNC_MASK				0x1F		//00011111

#define MAXNUM_TASKS_PERSLOT		5
#define SUBSLOTZERO					0


#define GENERIC_NST_MAX_IDX	 	(60)
#define GENERIC_NST_LAST_IDX		(GENERIC_NST_MAX_IDX -1)
#define GENERIC_NST_MAX_IDX_MASK	0x3F
#define GENERIC_NST_NOT_USED_VAL	0xCC
#define MAX_NST_TBL_COUNT			2

/*************  NOTE: WHEN ADDING A NEW FUNCTION *****************************

1. Add a new define
1. Be sure to inc the max count
2. Write the new function
3. Add the declaration to the function declarations in RTS.c
4. Add a new entry in fpaSchedFunctArray[] in RTS.c

******************************************************************************/


/*--------------------------------------------------------------------------*/


 /* ROUTINE DEFINITIONS */
void vTestRTS_putNSTentry(uchar ucNST_tblNum,uchar ucNST_slot ,uchar ucNST_val);

void vRTS_putNSTentry(
		uchar ucNST_tblNum,		//NST tbl (0 or 1)
		uchar ucNST_slot		//NST slot number
		);

void vRTS_getNSTentry(
		uchar ucNST_tblNum,		//NST tbl (0 or 1)
		uchar ucNST_slot,		//NST slot number
		uchar *punSlotArray
		);

uchar ucRTS_CheckNSTSlotforEntry(
		uchar ucNST_tblNum,		//NST tbl (0 or 1)
		uchar ucNST_slot,		//NST slot number
		uchar ucStblIdx			//Task to be checked for
		);

uchar ucRTS_getNSTSubSlotentry(
		uchar ucNST_tblNum,		//NST tbl (0 or 1)
		uchar ucNST_slot,		//NST slot number
		uchar ucNST_SubSlot
		);

void vRTS_showAllNSTentrys(
		uchar ucNST_tblNum,		//NST tbl num (0 or 1)
		uchar ucShowStblFlag	//YES_SHOW_STBL, NO_SHOW_STBL
		);

void vRTS_convertAllRTJslotsToSleep(
		void
		);

void vRTS_scheduleNSTtbl(
		long lNextFrameNumber	//Frame we are filling NST for
		);

uint uiRTS_findNearestNSTslot(
		uchar ucNST_tblNum,		//NST table (0 or 1)
		uchar ucDesiredSlot,		//NST slot num
		uchar ucTaskIDxtoSched
		);

void vRTS_clrNSTtbl(
		uchar ucNST_tblNum		//NST table (0 or 1)
		);

void vRTS_runScheduler(
		void
		);

void vRTS_showTaskHdrLine(
		uchar ucCRLF_termFlg	//YES_CRLF, NO_CRLF
		);

uchar ucRTS_thisNSTnum(
		void
		);

uchar ucRTS_nextNSTnum(
		void
		);

#if 0
uchar ucRTS_hasSchedRun(
		void
		);
#endif

uchar ucRTS_lastScheduledNSTnum(
		void
		);

void vRTS_showStatsOnLFactor(
		void
		);

uchar ucRTS_CheckforConflict(
		uchar ucNST_tblNum, //NST table number
		uchar ucSlot, //Slot number
		uint uiTaskFlags  //Task
		);

void vRTS_schedule_Scheduler_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
    long lFrameNumToSched //Frame number to schedule
    );

void vRTS_SchedCommand(uchar ucTskIndex);
void vRTS_schedule_ASAP(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lCurrentTime //Current time
		);
void vRTS_CreateRadioDiagTask(void);
void vRTS_CheckSPDataPending(void);

#endif /* RTS_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

