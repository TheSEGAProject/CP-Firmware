/**************************  RTS.C  *****************************************
 *
 * Routines for the REAL TIME SCHEDULER.
 *
 *
 * V1.00 03/02/2005 wzr
 *		Started
 *
 ******************************************************************************/

/*lint -e526 *//* function not defined */
/*lint -e657 *//* unusual (nonportable) anonymous struct or union */
///*lint -e714 */		/* symbol not referenced */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
///*lint -e757 */		/* global declarator not referenced */
///*lint -e752 */		/* local declarator not referenced */
///*lint -e758 */		/* global union not referenced */
///*lint -e768 */		/* global struct member not referenced */
#include <msp430.h>		//processor reg description
#include "task.h"
#include "std.h"			//standard defines
#include "main.h"			//
#include "serial.h" 		//serial IO port stuff
#include "daytime.h"		//Daytime routines
#include "rts.h"			//Real Time Scheduler routines
#include "comm.h"			//radio msg helper routines
#include "rand.h"			//Random number generator
#include "time.h"				//Time routines
#include "modopt.h" 		//role flags for wiz routines
#include "lnkblk.h"			//Link byte handler routines
#include "config.h" 		//system configuration description file
#include "l2sram.h"  		//disk storage module
#include "l2fram.h"			//Level 2 Ferro Ram routines
#include "led.h"
#include "buz.h"
#include "report.h"
#include "pmm.h"

extern volatile uint8 ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];

extern uchar ucGLOB_myLevel; //senders level +1
extern const uchar ucaBitMask[8];
extern long lGLOB_initialStartupTime; //Time used to compute uptime
extern long lGLOB_lastAwakeFrame; //Nearest thing to cur frame
extern uchar ucGLOB_lastAwakeSlot; //Nearest thing to cur slot
extern uchar ucGLOB_lastAwakeNSTtblNum; //Nearest thing to cur NST tbl
extern long lGLOB_lastScheduledFrame;

extern unsigned char g_ucSP1Ready;
extern unsigned char g_ucSP2Ready;
extern unsigned char g_ucSP3Ready;
extern unsigned char g_ucSP4Ready;

extern volatile union //ucFLAG0_BYTE
{
	uchar byte;
	struct
	{
		unsigned FLG0_BIGSUB_CARRY_BIT :1; //bit 0 ;1=CARRY, 0=NO-CARRY
		unsigned FLG0_BIGSUB_6_BYTE_Z_BIT :1; //bit 1 ;1=all diff 0, 0=otherwise
		unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT :1; //bit 2 ;1=top 4 bytes 0, 0=otherwise
		unsigned FLG0_NOTUSED_3_BIT :1; //bit 3 ;1=SOM2 link exists, 0=none
		//SET:	when any SOM2 links exist
		//CLR: 	when the SOM2 link is lost
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
		unsigned FLG0_NOTUSED7_BIT :1; //bit 7
	} FLAG0_STRUCT;
} ucFLAG0_BYTE;

extern union //ucGLOB_debugBits1
{
	uchar byte;
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

unsigned char ucaSlot[MAXNUM_TASKS_PERSLOT]; //the slot array used to schedule several functions in one slot
unsigned char ucMiniSlotIdx; //the index referenced in this file used for scheduling minislots in a slot

//! \var g_ucNextSlotTaskTable
//! \brief This contains 2 frames worth of scheduled tasks indices
uchar g_ucNextSlotTaskTable[MAX_NST_TBL_COUNT][GENERIC_NST_MAX_IDX][MAXNUM_TASKS_PERSLOT];

/*******************  FUNCTION DECLARATIONS HERE  ****************************/

static void vRTS_schedule_no_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

static void vRTS_schedule_BCN_slots(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

static void vRTS_schedule_RTJ_slots(uchar ucTaskdx, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

static void vRTS_schedule_all_slots(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

static void vRTS_schedule_OM_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

static void vRTS_schedule_interval_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

static void vRTS_schedule_loadonly_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

static void vRTS_schedule_command_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

static void vRTS_schedule_downcount_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		);

/* DECLARE A VOID RET FUNC WITH 2 PARAMETERS FOR FUNCTIONS ABOVE */
typedef void (*SCHED_FUNC_WITH_2_PARAMS)(uchar ucTskIndex, long lFrameNumToSched);

/*-----------------  fpaSchedFuncArray[]  -----------------------------------
 *
 * DECLARE THE ARRAY OF POINTERS TO FUNCTIONS
 * NOTE: THIS TABLE IS INDEXED BY FUNCTION NUMBER
 *
 *----------------------------------------------------------------------------*/
const SCHED_FUNC_WITH_2_PARAMS fpaSchedFuncArray[SCHED_FUNC_MAX_COUNT] = {
/* INDEXED BY PRIORITY-USE NUMBER */
vRTS_schedule_no_slot, //0 = SCHED_FUNC_DORMANT_SLOT
		vRTS_schedule_Scheduler_slot, //1 = SCHED_FUNC_SCHEDULER_SLOT
		vRTS_schedule_BCN_slots, //2 = SCHED_FUNC_SDC4_SLOT
		vRTS_schedule_RTJ_slots, //3 = SCHED_FUNC_RTJ_SLOT
		vRTS_schedule_all_slots, //4 = SCHED_FUNC_ALL_SLOT
		vRTS_schedule_OM_slot, //5 = SCHED_FUNC_RANDOM_SLOT
		vRTS_schedule_interval_slot, //6 = SCHED_FUNC_INTERVAL_SLOT
		vRTS_schedule_loadonly_slot, //7 = SCHED_FUNC_LOADBASED_SLOT
		vRTS_schedule_command_slot, //8 = SCHED_CMD_SLOT
		vRTS_schedule_downcount_slot //9 = SCHED_DNCNT_SLOT
		};

/*****************************  CODE STARTS HERE  ****************************/

/////////////////////////  vRTS_putNSTSubSlotentry()  ////////////////////////////////
//! \brief This function enters an action into a Sub Slot
//!
//!
//!	\param  ucNST_tblNum, ucNST_Slot, ucNST_SubSlot, ucTaskId
//! \return none
/////////////////////////////////////////////////////////////////////////////////////
static void vRTS_putNSTSubSlotentry(uchar ucNST_tblNum, //NST tbl (0 or 1)
		uchar ucNST_Slot, uchar ucNST_SubSlot, //NST slot number
		uchar ucTskIndex)
{

	// Range check the nst table number as well as the slot number
	if ((ucNST_tblNum >= MAX_NST_TBL_COUNT) || (ucNST_Slot > GENERIC_NST_LAST_IDX)) {
		vSERIAL_sout("RTS:WrtOfNSToutOfRange:\r\n", 25);
		vSERIAL_sout("NSTtblNum= ", 11);
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_sout(" Mx= ", 5);
		vSERIAL_UIV8out(MAX_NST_TBL_COUNT);
		vSERIAL_sout("  Slt#= ", 8);
		vSERIAL_UIV8out(ucNST_Slot);
		vSERIAL_sout(" SltMx= ", 8);
		vSERIAL_UIV8out(GENERIC_NST_LAST_IDX);
		vSERIAL_sout("\r\nAbortRdToSaveSram\r\n", 21);

		ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 1;
	}

	g_ucNextSlotTaskTable[ucNST_tblNum][ucNST_Slot][ucNST_SubSlot] = ucTskIndex;

	return;

}/* END: vRTS_putNSTSubSlotentryRAM() */

/////////////////////////  vRTS_getNSTSubSlotentry()  ////////////////////////////////
//! \brief This function gets a task index from the NST
//!
//!
//!	\param  ucNST_tblNum, ucNST_Slot, ucNST_SubSlot
//! \return none
/////////////////////////////////////////////////////////////////////////////////////
uchar ucRTS_getNSTSubSlotentry(uchar ucNST_tblNum, //NST tbl (0 or 1)
		uchar ucNST_Slot, //NST slot number
		uchar ucNST_SubSlot)
{

#if 0
	vSERIAL_sout("E:L2SRMGetNSTentry()\r\n", 22);
#endif

	/* RANGE CHECK THE NST TBL NUMBER */
	if ((ucNST_tblNum >= MAX_NST_TBL_COUNT) || (ucNST_Slot > GENERIC_NST_LAST_IDX)) {
		vSERIAL_sout("RTS:RdOfSubSlotoutOfRange:\r\n", 28);
		vSERIAL_sout("NSTtblNum= ", 11);
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_sout(" Mx= ", 5);
		vSERIAL_UIV8out(MAX_NST_TBL_COUNT);
		vSERIAL_sout("  Slt#= ", 8);
		vSERIAL_UIV8out(ucNST_Slot);
		vSERIAL_sout(" SltMx= ", 8);
		vSERIAL_UIV8out(GENERIC_NST_LAST_IDX);

		ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxReadFromNST = 1;

		return (0);
	}

	// Return the Task ID within the slot
	return (g_ucNextSlotTaskTable[ucNST_tblNum][ucNST_Slot][ucNST_SubSlot]);
}

/////////////////////////  vRTS_getNSTentry()   ////////////////////////////////
//! \brief This function checks the NST and fills the passed array with
//!        the task indices of the specified slot.
//!
//!
//!	\param  ucNST_tblNum, ucNST_slot, *pucSlotArray
//! \return none
/////////////////////////////////////////////////////////////////////////////////////
void vRTS_getNSTentry(uchar ucNST_tblNum, //NST tbl (0 or 1)
		uchar ucNST_slot, //NST slot number
		uchar *punSlotArray)
{
	uchar ucSubSlotCounter;

#if 0
	vSERIAL_sout("E:L2SRMGetNSTentry()\r\n", 22);
#endif

	// Range check the nst table number as well as the slot number
	if ((ucNST_tblNum >= MAX_NST_TBL_COUNT) || (ucNST_slot > GENERIC_NST_LAST_IDX)) {
		vSERIAL_sout("RTS:RdOfNSToutOfRange:\r\n", 24);
		vSERIAL_sout("NSTtblNum= ", 11);
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_sout(" Mx= ", 5);
		vSERIAL_UIV8out(MAX_NST_TBL_COUNT);
		vSERIAL_sout("  Slt#= ", 8);
		vSERIAL_UIV8out(ucNST_slot);
		vSERIAL_sout(" SltMx= ", 8);
		vSERIAL_UIV8out(GENERIC_NST_LAST_IDX);
		vSERIAL_sout("\r\nAbort\r\n", 9);

		ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxReadFromNST = 1;
	}
	// If we are in range then get the entries in the NST
	else {
		for (ucSubSlotCounter = 0x00; ucSubSlotCounter < MAXNUM_TASKS_PERSLOT; ucSubSlotCounter++)
			*punSlotArray++ = ucRTS_getNSTSubSlotentry(ucNST_tblNum, ucNST_slot, ucSubSlotCounter);
	}

}/* END: vRTS_getNSTentry() */

///////////////////////// ucRTS_CheckNSTSlotforEntry()  ///////////////////////////////
//! \brief This function checks for a task entry in a slot in the NST
//!
//!
//!	\param  ucNST_tblNum, ucNST_slot, ucTskIndex
//! \return 1 if the task has been scheduled, 0 if the task has not been scheduled
/////////////////////////////////////////////////////////////////////////////////////
uchar ucRTS_CheckNSTSlotforEntry(uchar ucNST_tblNum, //NST tbl (0 or 1)
		uchar ucNST_slot, //NST slot number
		uchar ucTaskIndex //Task to be checked for
		)
{
	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];
	uchar ucCounter;
	uchar ucReturnVal;

	//Assume the task is not scheduled in this slot
	ucReturnVal = 0;
	vRTS_getNSTentry(ucNST_tblNum, ucNST_slot, ucaSlotArray);

	// Loop through the slot and check each
	for (ucCounter = 0x00; ucCounter < MAXNUM_TASKS_PERSLOT; ucCounter++) {
		if (ucaSlotArray[ucCounter] == ucTaskIndex){
			ucReturnVal = 1;
			break;
		}
	}

	return ucReturnVal;
}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Checks to make sure a slot is empty.
//!
//! This is useful when scheduling tasks that require the entire slot
//!
//! \param ucSlotNum, ucNST_tblNum
//! \return 1 for conflict 0 for empty slot
///////////////////////////////////////////////////////////////////////////////
uchar ucRTS_isSlotEmpty(uchar ucSlotNum, uchar ucNST_tblNum)
{
	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];
	uchar ucCounter;

	vRTS_getNSTentry(ucNST_tblNum, ucSlotNum, ucaSlotArray);

	// Loop through the slot and check each
	for (ucCounter = 0x00; ucCounter < MAXNUM_TASKS_PERSLOT; ucCounter++) {
		if (ucaSlotArray[ucCounter] != GENERIC_NST_NOT_USED_VAL)
			return FALSE;
	}
	return TRUE;
}

//////////////////////  vRTS_showAllNSTentrys()////////////////////////////////////
//! \brief shows all the values in the NST table formatted in a 3 by 20 array
//!
//! \param ucNST_tblNum, ucShowStblFlag
//! \return none
///////////////////////////////////////////////////////////////////////////////////
void vRTS_showAllNSTentrys(uchar ucNST_tblNum, //NST tbl num (0 or 1)
		uchar ucShowStblFlag //YES_SHOW_TSB, NO_SHOW_TSB
		)
{
	uchar ucNSTidxCnt;
	uchar ucNSTSubidxCnt;

	/* SHOW THE TITLE */
	vSERIAL_dash(35);
	vSERIAL_sout("  NST ", 6);
	vSERIAL_UIV8out(ucNST_tblNum);
	vSERIAL_sout("  ", 2);
	vSERIAL_dash(34);
	vSERIAL_crlf();

//this loop cycles through the NST table by Slot
	for (ucNSTidxCnt = 0; ucNSTidxCnt < GENERIC_NST_MAX_IDX; ucNSTidxCnt++) {
		/* DO SOME COUNTER FORMATTING HERE */
		if (!(ucNSTidxCnt % 2)) {
			vSERIAL_crlf();
		}
		vSERIAL_HB8out(ucNSTidxCnt);
		vSERIAL_sout(": ", 2);

		//This loop cycles through the NST by Sub Slot
		for (ucNSTSubidxCnt = 0x00; ucNSTSubidxCnt < MAXNUM_TASKS_PERSLOT; ucNSTSubidxCnt++) {
			// Display the task name
			vTask_showTaskName(g_ucNextSlotTaskTable[ucNST_tblNum][ucNSTidxCnt][ucNSTSubidxCnt]);
			vSERIAL_sout("  ", 2);
		}
	}
	vSERIAL_crlf();

	if (ucShowStblFlag == YES_SHOW_TCB) {
		vL2FRAM_showTSBTbl();
	}
	return;

}/* END: vRTS_showAllNSTentrys() */

//////////////////////////////  vRTS_clrNSTtbl()  //////////////////////////
//! \brief This function sets all entries in an NST to sleep
//!
//!
//! \param ucNST_tbl_Num
//! \brief none
/////////////////////////////////////////////////////////////////////////////
void vRTS_clrNSTtbl(uchar ucNST_tblNum //0 or 1
		)
{
	uchar ucNST_slotCounter;
	uchar ucNST_subslotCounter;

#if 0
	vSERIAL_sout("E:ClrNSTtbl\r\n", 13);
#endif

	//If the NST number is out of range then display error message and force it to either 1 or 0
	if (ucNST_tblNum > 1) {
		vSERIAL_sout("ClrNST:BdValForNSTidx=", 22);
		vSERIAL_UIV8out(ucNST_tblNum);
		vSERIAL_crlf();
		ucNST_tblNum = ucNST_tblNum % 2; //force it to legal bounds
	}

	//this loop cycles through each Slot
	for (ucNST_slotCounter = 0; ucNST_slotCounter < GENERIC_NST_MAX_IDX; ucNST_slotCounter++) {
		//This loop cycles through each Sub Slot
		for (ucNST_subslotCounter = 0; ucNST_subslotCounter < MAXNUM_TASKS_PERSLOT; ucNST_subslotCounter++)
			g_ucNextSlotTaskTable[ucNST_tblNum][ucNST_slotCounter][ucNST_subslotCounter] = GENERIC_NST_NOT_USED_VAL;
	}/* END: for(ucNST_slotCounter) */

#if 0
	vSERIAL_sout("X:clrNSTtbl\r\n", 13);
#endif

	return;

}/* END: vRTS_clrNSTtbl() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Sets all empty slots to sleep
//!
//! \param lFrameNumber
//! \return ucNST_tblNum
//////////////////////////////////////////////////////////////////////////////
vRTS_SetEmptySlotstoSleep(uchar ucNST_tblNum)
{
	uchar ucSleepIndex;
	uchar ucNST_slotCounter;

	// Find the index of the sleep task
	ucSleepIndex = ucTask_FetchTaskIndex(TASK_ID_SLEEP);

	//this loop cycles through each Slot
	for (ucNST_slotCounter = 0; ucNST_slotCounter < GENERIC_NST_MAX_IDX; ucNST_slotCounter++) {
		// If the slot is empty then set it to sleep
		if (ucRTS_isSlotEmpty(ucNST_slotCounter, ucNST_tblNum) == TRUE)
			g_ucNextSlotTaskTable[ucNST_tblNum][ucNST_slotCounter][0] = ucSleepIndex;

	}/* END: for(ucNST_slotCounter) */
}

/////////////////////// ucRTS_computeNSTfromFrameNum() ///////////////////////
//! \brief Compute the NST table (0 or 1) from the frame number
//!
//! \param lFrameNumber
//! \return ucNST_tblNum
//////////////////////////////////////////////////////////////////////////////
uchar ucRTS_computeNSTfromFrameNum(long lFrameNumber)
{
	uchar ucNST_tblNum;

	/* CHECK THE FRAME NUMBER */
	if (lFrameNumber < 0) {
#if 0
		vSERIAL_sout("RTS:BdFrameNum=", 15);
		vSERIAL_IV32out(lFrameNumber);
		vSERIAL_crlf();
#endif
	}

	/* COMPUTE WHICH NST TABLE WE ARE FILLING */
	ucNST_tblNum = (uchar) (lFrameNumber % 2L); //compute NST tbl from frame

	return (ucNST_tblNum);

}/* END: ucRTS_computeNSTfromFrameNum() */


/////////////////////// vRTS_scheduleNSTtbl()  //////////////////////////////
//! \brief Schedules the NST according to priority
//!
//!
//! \param lFrameNumber
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vRTS_scheduleNSTtbl(long lFrameNumber //Frame number we are scheduling
		)
{
	uchar ucTaskIdxCnt;
	uchar ucPriorityCnt;
	uchar ucPriorityFuncVal;
	uchar ucPriorityOnlyVal;
	uchar ucFunctionOnlyVal;
	uchar ucNST_tblNum;
	uchar ucRole;
	uchar ucRoleMask;
	uchar ucWhoCanRun, ucTaskState;
	ulong ulPriority, ulTaskState, ulWhoCanRun;

	/* GET THE NST THAT WE WILL BE WORKING ON */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumber);

#if 0
	vSERIAL_sout("E:RTS:Sched\r\n", 13);
	vRTS_showAllNSTentrys(ucNST_tblNum, YES_SHOW_TSB);
#endif

	/* FIRST CLEAR THE NST WE ARE GOING TO FILL */
	vRTS_clrNSTtbl(ucNST_tblNum);

	// Clean up the portion of the task list allocated for scheduler created tasks
	vTask_ClearDynSector();

	// Remove unneeded tasks
	vTask_CleanTaskList();

	// Get the role and the role mask to determine what tasks can run
	ucRole = ucMODOPT_getCurRole();
	ucRoleMask = ucaBitMask[ucRole];

#if 0
	vRTS_showAllNSTentrys(0,YES_SHOW_TCB);
	vRTS_showAllNSTentrys(1,YES_SHOW_TCB);
#endif

	/* MULTIPLE PASSES OVER THE SCHED TABLES BY PRIORITY */
	for (ucPriorityCnt = PRIORITY_0;; ucPriorityCnt += PRIORITY_INC_VAL)
	{
#if 0
		vSERIAL_sout("SchdNST:Priority=", 17);
		vSERIAL_HB8out(ucPriorityCnt);
		vSERIAL_crlf();
#endif

		// Search through the list of defined tasks (does not include tasks created by the scheduler)
		for (ucTaskIdxCnt = 0; ucTaskIdxCnt < TASKPARTITION; ucTaskIdxCnt++) {

			// Get the WhoCanRun parameter
			if (ucTask_GetField(ucTaskIdxCnt, TSK_WHOCANRUN, &ulWhoCanRun) != TASKMNGR_OK)
				continue;
			ucWhoCanRun = (uchar) ulWhoCanRun;

			// If the task can be run by this WiSARD
			if ((ucWhoCanRun & ucRoleMask) != 0) {
				// Get the task state if allowed
				if (ucTask_GetField(ucTaskIdxCnt, TSK_STATE, &ulTaskState) != TASKMNGR_OK)
					continue;
				ucTaskState = (uchar) ulTaskState;

				if (ucTaskState == TASK_STATE_ACTIVE) {
					// GET THE PRIORITY-FUNCTION VALUE FOR THIS ENTRY
					if (ucTask_GetField(ucTaskIdxCnt, TSK_PRIORITY, &ulPriority) != TASKMNGR_OK) // get priority from tsk mgr
						continue;
					ucPriorityFuncVal = (uchar) ulPriority;

					// Bit mask to get the priority and scheduler function index
					ucPriorityOnlyVal = (ucPriorityFuncVal & PRIORITY_MASK); //PRIORITY_MASK = 0b11100000
					ucFunctionOnlyVal = (ucPriorityFuncVal & SCHED_FUNC_MASK); //SCHED_FUNC_MASK = 0b00011111

					/* VECTOR TO THE SCHED FUNCTION IF THE PRIORITY MATCHES */
					if (ucPriorityOnlyVal == ucPriorityCnt) {
						/* VECTOR TO THE FUNCTION */
						fpaSchedFuncArray[ucFunctionOnlyVal](ucTaskIdxCnt, lFrameNumber);
#if 0
						vSERIAL_sout("PriorityCnt= ", 13);
						vSERIAL_HB8out(ucPriorityCnt);
						vSERIAL_sout(" Func= ", 7);
						vSTBL_showPFuncName(ucFunctionOnlyVal);
						vSERIAL_sout(" TaskID= ", 9);
						vSERIAL_HB8out(ucTaskIdxCnt);
						vSERIAL_crlf();
						vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_TCB);
#endif

					}/* END: if(ucPriorityOnlyVal) */

				} // END: if task exists
			} //END: if(role)
		}/* END: for(ucTaskIdxCnt) */

		if (ucPriorityCnt == PRIORITY_MAX_VAL)
			break;
	}/* END: for(ucPriorityCnt) */

	/* UPDATE THE GLOBAL TO SHOW THE LAST SCHEDULER PARAMETERS */
	lGLOB_lastScheduledFrame = lFrameNumber;

	// make sure we are sleeping when not busy with tasks
	vRTS_SetEmptySlotstoSleep(ucNST_tblNum);

#if 1
	// note, command tasks prefer slot 0, if YES_SHOW_TSB is passed, potential
	// text delay from writing will cause slot to next execute if scheduled in
	// slot 0
	vRTS_showAllNSTentrys(ucNST_tblNum, YES_SHOW_TCB);
#endif

	return;

}/* END: vRTS_scheduleNSTtbl() */

/////////////////////// vRTS_schedule_no_slot() //////////////////////////////
//! \brief Some tasks (ie. RSSI) run in parallel with the operation of the
//!	WiSARD and do not require a slot.
//!
//! \param ucTskIndex, lFrameNum
//! \return none
/////////////////////////////////////////////////////////////////////////////
static void vRTS_schedule_no_slot(uchar ucTaskIdx, //Tbl Idx of action to schedule
		long lFrameNum //Frame number to schedule
		)
{
	return;
}/*lint !e715 */

/////////////////////// vRTS_schedule_Scheduler_slot()  ////////////////////////
//! \brief This is a special routine to assign the scheduler slot in the last slot
//!        of the NST table
//!
//!
//! \param ucTskIndex, lFrameNum
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vRTS_schedule_Scheduler_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNum //Frame number to schedule
		)
{
	uchar ucNST_tblNum;

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNum);

	/* THE SCHEDULER IS NOW IN A FIXED SPOT (LAST SLOT OF THE NST) */
	vRTS_putNSTSubSlotentry(ucNST_tblNum, 59, 4, ucTskIndex);


	return;

}/* vRTS_schedule_Scheduler_slot() *//*lint !e715 */

/////////////////////// ucRTS_computeBCN_slot_0() ///////////////////////////
//! \brief Computes the start slot of the beacon slot
//!
//!
//! \param ucLevelNum
//! \return ucStartSlot
/////////////////////////////////////////////////////////////////////////////
static uchar ucRTS_computeBCN_slot_0(uchar ucLevelNum)
{
	uchar ucStartSlot;

#if 0
	ucStartSlot = (uchar)((ucLevelNum +1) * 2);
	ucStartSlot %= GENERIC_NST_MAX_IDX; //limit it
#endif

	ucStartSlot = ucLevelNum + 1; //for now assign start slot by level

	return (ucStartSlot);

}/* END: ucRTS_computeBCN_slot_0() */

/////////////////////// vRTS_schedule_BCN_slots() //////////////////////////
//! \brief This function schedules the beacon messages
//!
//! For a hub the beacon is scheduled at 10 equally spaced slots, if it has
//! already established children nodes then the beacon is scheduled once per
//! frame. For a child node the beacon is scheduled once per randomly selected
//! frame once it has joined with a parent node.
//!
//!
//! \param ucTskIndex, lFrameNumToSched
//! \return none
/////////////////////////////////////////////////////////////////////////////
static void vRTS_schedule_BCN_slots(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		)
{
	uchar ucNST_tblNum;
	uchar ucActiveSOMslots;
	uchar ucActiveROMslots;
	uint uiCountTotals;
	uchar ucStartSlot;
	uchar ucReqSlot;
	uint uiFoundSlot;
	uchar ucFoundSlot;
	uchar ucFoundSubSlot;
	uchar ucii;
	uchar ucBigRand;
	uchar ucSmallRand;

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/* COUNT THE ACTIVE SOM2 AND ROM2 SLOTS */
	uiCountTotals = uiTask_countSOM2andROM2entrys();

	//unpack the counts SOM count in the high byte and ROM count in the low byte
	ucActiveSOMslots = (uchar) (uiCountTotals >> 8);
	ucActiveROMslots = (uchar) uiCountTotals;

	uchar ucLnkBlk;
	// If we already have the maximum number of links then do not schedule the beacon
	if(ucLNKBLK_FindEmptyBlk(&ucLnkBlk) != LNKMNGR_OK)
		return;

	/* COMPUTE THE BEGINNING SLOT FOR THIS WIZARD */
	ucStartSlot = ucRTS_computeBCN_slot_0(ucGLOB_myLevel);

	/*----------  SCHEDULE Beacon FOR THE HUB  ---------------------*/

	if (ucL2FRAM_isHub()) {
		/* HUB HAS ESTABLISHED ROM2'S -- DISCOVER IN ONLY 1 SLOT */
		if (ucActiveROMslots != 0) {

			/* SELECT THE FIXED SLOT NUMBER */
			ucReqSlot = ucStartSlot + 8;

			/* SEE IF THE SLOT IS FREE */
			uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, ucReqSlot, ucTskIndex);

			//unpack the slot information
			ucFoundSlot = (uchar) uiFoundSlot;
			ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

			if (uiFoundSlot != 0xFFFF) {
				/* ASSIGN THE SLOT TO BE THE NEXT DISCOVERY SLOT */
				vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, SUBSLOTZERO, ucTskIndex);
			}
			return;
		}/* END: if() */

		/* WE ARE A HUB WITH NO ESTABLISHED ROM2'S */
		/* DISCOVER IN  10 EQUALLY SPACED SLOTS */
		for (ucii = 0; ucii < 10; ucii++) {
			/* COMPUTE A SLOT TO CHOOSE */
			ucReqSlot = (uchar) (ucStartSlot + (ucii * 6));

			/* LIMIT ANY POSSIBLE OVERRUN */
			ucReqSlot %= GENERIC_NST_MAX_IDX;

			/* SEE IF THE SLOT IS FREE */
			uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, ucReqSlot, ucTskIndex);

			//unpack the slot information
			ucFoundSlot = (uchar) uiFoundSlot;
			ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

			if (uiFoundSlot != 0xFFFF) {
				/* ASSIGN THE NEXT DISCOVERY SLOT */
				vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, SUBSLOTZERO, ucTskIndex);
			}
#if 0
			vSERIAL_sout("RTS:MstrNoLnksWantsSlt ", 23);
			vSERIAL_UIV8out(ucReqSlot);
			vSERIAL_sout("  Lvl= ", 7);
			vSERIAL_UIV8out(ucGLOB_myLevel);
			vSERIAL_crlf();
#endif

		}/* END: for(ucii) */

		return;

	}/* END: if(isHub()) */
	else if(ucMODOPT_isRelay()){
		/* SELECT THE FIXED SLOT NUMBER */
		ucReqSlot = ucStartSlot + 8;

		/* SEE IF THE SLOT IS FREE */
		uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, ucReqSlot, ucTskIndex);

		//unpack the slot information
		ucFoundSlot = (uchar) uiFoundSlot;
		ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

		if (uiFoundSlot != 0xFFFF) {
			/* ASSIGN THE SLOT TO BE THE NEXT DISCOVERY SLOT */
			vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, SUBSLOTZERO, ucTskIndex);
		}
		return;
	}

	/*----------  SCHEDULE Beacon FOR THE spoke ------------------*/

	/* WE ARE A SPOKE */
	/* NO DISCOVERY FOR A UNIT WITHOUT SOM2'S */
	if (ucActiveSOMslots == 0)
		return;

	/* WE ARE A SPOKE WITH ESTABLISHED SOM2'S -- TRANSMIT ON 1 SLOT RANDOMLY */

	/* ROLL THE RANDOM NUMBER */
	ucBigRand = ucRAND_getRolledMidSysSeed();
	ucSmallRand = (ucBigRand & 0x03);

	/* CHECK IF THIS RANDOM SLOT'S NUMBER IS UP */
	if (ucSmallRand == 0x02) {
#if 0
		vSERIAL_sout("RTS:RndYesXQA= ", 15);
		vSERIAL_HB8out(ucSmallRand);
		vSERIAL_sout("  BigRnd= ", 10);
		vSERIAL_HB8out(ucBigRand);
		vSERIAL_crlf();
#endif

		/* CHOOSE THE FIXED SLOT NUMBER */
		ucReqSlot = ucStartSlot + 8;

		/* SEE IF THE SLOT IS FREE */

		uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, ucReqSlot, ucTskIndex);

		//unpack the slot information
		ucFoundSlot = (uchar) uiFoundSlot;
		ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

		/* IT ITS FREE THEN ASSIGN IT */
		if (ucReqSlot == ucFoundSlot)
			vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, ucFoundSubSlot, ucTskIndex);

	}/* END: if() */

	return;

}/* END: vRTS_schedule_BCN_slots() */


/////////////////////// vRTS_schedule_RTJ_slots()  /////////////////////////
//! \brief Schedules the Receive Discovery message, if the WiSARD has never
//!		   acquired group time then the entire NST table is filled with RTJ
//!
//! \param ucTskIndex, lFrameNumToSched
//! \return none
/////////////////////////////////////////////////////////////////////////////
static void vRTS_schedule_RTJ_slots(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		)
{
	uchar ucNST_tblNum;
	uchar ucFoundSlot;
	uchar ucReqSlot;
	uchar ucActiveSOM2slots;
	uint uiFoundSlot;
	uint uiCountTotals;
	ulong ulBurstTime;
	T_Discovery S_Disc;

#if 0
	vSERIAL_sout("\r\nE:SCHED:RTJ:\r\n", 16);
#endif

	/* IF NOT SENDING OM2'S THEN LEAVE */
 	if (!ucL2FRAM_isSender()){
		return;
	}

	/* COUNT THE ACTIVE Child AND Parent SLOTS */
	uiCountTotals = uiTask_countSOM2andROM2entrys();
	ucActiveSOM2slots = (uchar) (uiCountTotals >> 8);

	/* IF WE ALREADY HAVE AN ACTIVE SOM2 -- DONT DO RDC4 */
	if (ucActiveSOM2slots != 0){
		return;
	}

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	// Get the current discovery settings
	vCommGetDiscMode(&S_Disc);

	// If for some reason the discovery structure has been corrupted then go to full discovery
	if(S_Disc.m_ucMode != FULLDISCOVERY && S_Disc.m_ucMode != PARTIALDISCOVERY && S_Disc.m_ucMode != BURSTDISCOVERY){
		vCommSetDiscMode(FULLDISCOVERY);
		vCommGetDiscMode(&S_Disc);
	}

	// Cycle through the discovery modes as needed
	if (ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT == 0) {
		// Go to full discovery if we don't have network time and refresh structure
		vCommSetDiscMode(FULLDISCOVERY);
		vCommGetDiscMode(&S_Disc);
	}
	else if (S_Disc.m_ucMode == PARTIALDISCOVERY) {
		if (((ulong)lTIME_getSysTimeAsLong() - S_Disc.m_ulStartTime) >= S_Disc.m_ulMaxDuration){

			// Create the radio diag task if WISARD is entering burst mode
			vRTS_CreateRadioDiagTask();

			// Set discovery to burst and refresh structure
			vCommSetDiscMode(BURSTDISCOVERY);
			vCommGetDiscMode(&S_Disc);
		}
	}
	else if (S_Disc.m_ucMode == BURSTDISCOVERY) {
		if (((ulong) lTIME_getSysTimeAsLong() - S_Disc.m_ulStartTime) >= S_Disc.m_ulMaxDuration) {
			// Set discovery to full and refresh the structure
			vCommSetDiscMode(FULLDISCOVERY);
			vCommGetDiscMode(&S_Disc);
		}
	}

	switch(S_Disc.m_ucMode){
		case FULLDISCOVERY:
			// If we have lost our connection with the parent then we cannot support children
			vTask_OrphanChildren();

			ucTask_SetField(ucTskIndex, TSK_FLAGS, (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 |  F_USE_FULL_SLOT));
			/* WE ARE A SPOKE THAT HAS NEVER ACQUIRED GROUP TIME -- DO A BLANKET SEARCH */
			vRTS_schedule_all_slots(ucTskIndex, lFrameNumToSched);
			break;

		case PARTIALDISCOVERY:
			// If we are scheduling partial RTJ then modify the task flags to execute during middle subslot
			ucTask_SetField(ucTskIndex, TSK_FLAGS, (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 |  F_USE_MIDDLE_OF_SLOT));
			/*----------------  NO ACTIVE SOM2'S  -------------------------------*/

			/* ASSIGN RDC4'S TO A GROUP OF SLOTS 10 WIDE */

			for (ucReqSlot = 7; ucReqSlot < 18; ucReqSlot++) {
				/* SEE IF THE SLOT IS FREE */
				uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, ucReqSlot, ucTskIndex);

				//unpack the slot information
				ucFoundSlot = (uchar) uiFoundSlot;

				if (ucFoundSlot < GENERIC_NST_MAX_IDX) {
					vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, SUBSLOTZERO, ucTskIndex);
				}

			}/* END: for(ucc) */
			break;

		case BURSTDISCOVERY:
			// If we have lost our connection with the parent then we cannot support children
			vTask_OrphanChildren();

			// If we are scheduling partial RTJ then modify the task flags to execute during middle subslot
			ucTask_SetField(ucTskIndex, TSK_FLAGS, (F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 |  F_USE_MIDDLE_OF_SLOT));

			// Determine if we should listen for beacons in this frame
			ulBurstTime = (ulong)lTIME_getSysTimeAsLong() - S_Disc.m_ulStartTime;

			// Mod 2o minutes
			ulBurstTime = ulBurstTime%1200;

			// If we are in one of the first 5 minutes (300 seconds ) in the 20 minute window then we listen
			if (ulBurstTime < 300)
				vRTS_schedule_all_slots(ucTskIndex, lFrameNumToSched);

			break;

		default:
			// Added for redundancy. Must ensure that the discovery module is in a valid state
			vCommSetDiscMode(FULLDISCOVERY);
			break;
	}

	return;

}/* END: vRTS_schedule_RTJ_slots() */

/////////////////////// vRTS_schedule_all_slots() //////////////////////////////
//! \brief This function schedules all the slots in the NST with one function
//!		   unless the slot has already been assigned
//!
//! \param ucTskIndex, lFrameNumToSched
//! \return none
/////////////////////////////////////////////////////////////////////////////
static void vRTS_schedule_all_slots(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		)

{
	uchar ucNST_tblNum;
	uchar ucNSTslotCnt;
	uchar ucNSTSublotCnt;
	uchar ucReadBack_NST_idx;
	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];

#if 0
	vSERIAL_sout("E:AllSlotsOn:\r\n", 15);
//vSTBL_showSingleStblEntry(ucTaskId, NO_HDR, NO_CRLF);
	vSERIAL_crlf();
#endif

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/* WALK THROUGH ENTIRE NST TBL FILLING ENTRYS */
	for (ucNSTslotCnt = 0; ucNSTslotCnt < (GENERIC_NST_MAX_IDX - 1); ucNSTslotCnt++) {
		/* READ THE SLOT AND CHECK IF IT IS ASSIGNED ALREADY */
		if (ucRTS_CheckNSTSlotforEntry(ucNST_tblNum, ucNSTslotCnt, ucTskIndex))
			break;

		vRTS_getNSTentry(ucNST_tblNum, ucNSTslotCnt, ucaSlotArray);
		for (ucNSTSublotCnt = 0; ucNSTSublotCnt < MAXNUM_TASKS_PERSLOT; ucNSTSublotCnt++) {
			//Check the tasks scheduled for that slot
			ucReadBack_NST_idx = ucaSlotArray[ucNSTSublotCnt];

			/* CHECK IF THIS SLOT IS ALREADY ASSIGNED */
			if ((ucReadBack_NST_idx == GENERIC_NST_NOT_USED_VAL) || (ucReadBack_NST_idx == TASK_ID_SLEEP)) {
				/* ASSIGN THIS SLOT */
				vRTS_putNSTSubSlotentry(ucNST_tblNum, ucNSTslotCnt, ucNSTSublotCnt, ucTskIndex);

				/* CHECK FOR MAX INDEX ERROR */
				if (ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST) {
					vSERIAL_sout("RTS:MxNSTidxInSched_all_slots\r\n", 31);
					ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
				}
				break;
			}

		} //END for(ucNSTSublotCnt)
	}/* END: for(ucNSTslotCnt) */

#if 0
	{
		vSERIAL_sout("RTS:SchedAll-  ", 15);
		vACTION_showStblActionName(ucTskIndex);
		vSERIAL_crlf();
		//vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_TCB);
	}
#endif

#if 0
	vSERIAL_sout("X:vRTS_Schedule_all_slots\r\n", 27);
#endif

	return;

}/* END: vRTS_schedule_all_slots() */


/////////////////////// vRTS_schedule_OM2_slot() //////////////////////////////
//! \brief Uses LNKBLK Table to determine how to schedule these functions
//!
//!
//! \param ucTskIndex, lFrameNumToSched
//! \return none
/////////////////////////////////////////////////////////////////////////////
static void vRTS_schedule_OM_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		)
{
	uchar ucc;
	uchar ucNST_tblNum;
	ulong ulPreviousSeed;
	ulong ulNextSeed;
	uint uiFoundSlot;
	uchar ucFoundSubSlot;
	uchar ucFoundSlot;
	uchar ucMsgIndex;
	ulong ulLinkTime;
	long lLinkFrame;
	uchar ucLinkSlot;
	uchar ucLnkNeedsForcing;
	uchar ucSubSlotIndex;
	uint uiTaskID;
	ulong ulTaskID;
	uchar ucTempIdx;
	ulong ulSerialNumber;
	uint uiSerialNumber;
	ulong ulTempSN;
	uint uiTempSN;
	uchar ucLnkIndex;
	uchar ucLinkPriority, ucTempLinkPriority;

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	/*---------------  ROLL THE RANDOM NUMBERS FIRST  -----------------------*/

	// Get the serial number if allowed
	if(ucTask_GetField(ucTskIndex, PARAM_SN, &ulSerialNumber) != TASKMNGR_OK)
		return;
	uiSerialNumber = (uint) ulSerialNumber;

	// Get the random number if allowed
	if (ucLNKBLK_ReadRand(uiSerialNumber, &ulPreviousSeed) != LNKMNGR_OK)
		return;

	/* GET THE NEXT FOREIGN SEED */
	ulNextSeed = uslRAND_getRolledFullForeignSeed(ulPreviousSeed);

	// Store the next seed in the link block
	ucLNKBLK_WriteRand(uiSerialNumber, ulNextSeed);

#if 0
	vSERIAL_sout("PrevRndSeed= ", 13);
	vSERIAL_HBV32out(ulPreviousSeed);
	vSERIAL_sout("  NxtSeed= ", 11);
	vSERIAL_HBV32out(ulNextSeed);
	vSERIAL_crlf();
#endif


	/*--------  CHECK THE FIRST REQ ENTRY FOR LNKBLK ERRS  ------------------*/

	/* ASSUME THAT NO FORCING IS REQUIRED */
	ucLnkNeedsForcing = 0;

	/* LOAD THE FIRST ENTRY OF THE LNKBLK */
	ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, 0, &ulLinkTime);
	lLinkFrame = lTIME_getFrameNumFromTime(ulLinkTime);
	ucLinkSlot = (uchar) lTIME_getSlotNumFromTime(ulLinkTime);

	/* CHECK FOR A ZRO ENTRY */
	if (ulLinkTime == 0) {
#if 1   /* REPORT A ZERO LINKUP ENTRY */
		vSERIAL_dash(6);
		vSERIAL_sout(" Lk ", 4);
		vSERIAL_UI16out(uiSerialNumber);
		vSERIAL_sout(" isZro ", 7);
		vSERIAL_dash(6);
		vSERIAL_crlf();

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_LINK_ZERO;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiSerialNumber >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiSerialNumber;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_LINK_ZERO);
#endif

		ucLnkNeedsForcing = 1; //force a new linkup
		goto Force_static_correction;
	}

	/* CHECK TO SEE IF THE LINK TIME IS TOO OLD */
	if (ulLinkTime < lFrameNumToSched) {
#if 1 /* REPORT A LINKUP TIME MISS */
		vSERIAL_dash(6);
		vSERIAL_sout(" Lk ", 3);
		vSERIAL_UI16out(uiSerialNumber);
		vSERIAL_sout(" Late ", 6);
		vSERIAL_dash(6);
		vSERIAL_crlf();

		vSERIAL_sout("CurTim= ", 8);
		vTIME_showTime(lTIME_getSysTimeAsLong(), FRAME_SLOT_TIME, NO_CRLF);

		vSERIAL_sout("  LkTim= ", 9);
		vTIME_showTime(ulLinkTime, FRAME_SLOT_TIME, YES_CRLF);

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_LINK_LATE;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiSerialNumber >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiSerialNumber;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_LINK_LATE);

#endif

		ucLnkNeedsForcing = 1; //force a new linkup
		goto Force_static_correction;
	}

	/* CHECK TO SEE IF THE LINK TIME IS IN THE FUTURE */
	if (lLinkFrame > lFrameNumToSched) {
#if 1  /* REPORT A FUTURE LINK REQ */
		vSERIAL_dash(6);
		vSERIAL_sout(" Lk ", 4);
		vSERIAL_UI16out(uiSerialNumber);
		vSERIAL_sout(" Future= ", 9);
		vTIME_showTime(ulLinkTime, FRAME_SLOT_TIME, NO_CRLF);
		vSERIAL_dash(6);
		vSERIAL_crlf();
#endif

		goto sos_exit;
	}

	Force_static_correction:

	/* CHECK IF WE NEED TO FORCE A LINK REQ */
	if (ucLnkNeedsForcing) {
		/* SETUP FOR A FORCED NEW LINKUP */
		ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiSerialNumber, LNKREQ_1FRAME_1LNK, lTIME_getSysTimeAsLong());

		/* READ FORCED TIME BACK */
		ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, 0, &ulLinkTime);

#if 1
		vSERIAL_dash(6);
		vSERIAL_sout(" Force Static Lk ", 17);
		vSERIAL_UI16out(uiSerialNumber);
		vSERIAL_sout(" = ", 3);
		vTIME_showTime(ulLinkTime, FRAME_SLOT_TIME, NO_CRLF);
		vSERIAL_dash(6);
		vSERIAL_crlf();
#endif

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_FORCE_STATIC;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiSerialNumber >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiSerialNumber;

		// Store DE
		vReport_LogDataElement(RPT_PRTY_FORCE_STATIC);

	}/* END: if() */

	/*--------  CHECK THE FIRST ENTRY FOR NST SCHEDULING ERRS  --------------*/

	ucLnkNeedsForcing = 0; //assume everything is OK

	/* LOAD THE FIRST ENTRY OF THE LNKBLK */
	ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, 0, &ulLinkTime);
	lLinkFrame = lTIME_getFrameNumFromTime(ulLinkTime);
	ucLinkSlot = (uchar) lTIME_getSlotNumFromTime(ulLinkTime);

	/* FIND THE CLOSEST POSITION TO THIS SLOT */
	uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, ucLinkSlot, ucTskIndex);

	//unpack the slot information
	ucFoundSlot = (uchar) uiFoundSlot;
	ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

	/* CHECK IF THERE ARE ANY SLOTS AVAILABLE */
	if (ucFoundSlot > GENERIC_NST_LAST_IDX) {
#if 1		/* REPORT NO SLOTS AVAILABLE */
		vSERIAL_dash(6);
		vSERIAL_sout(" Lk ", 4);
		vSERIAL_UI16out(uiSerialNumber);
		vSERIAL_sout(" NoSlt ", 7);
		vSERIAL_dash(6);
		vSERIAL_crlf();
//		vRTS_showAllNSTentrys(ucNST_tblNum, YES_SHOW_TCB);
		vSERIAL_crlf();
#endif

		ucLnkNeedsForcing = 1; //force a new linkup
		goto Force_dynamic_correction;

	}

	Force_dynamic_correction:

	if (ucLnkNeedsForcing) {
		/* SETUP FOR A FORCED NEW LINKUP 1 FRAME OUT */
		ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiSerialNumber, LNKREQ_1FRAME_1LNK, lTIME_getSysTimeAsLong());

		/* READ FORCED TIME BACK */
		ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, 0, &ulLinkTime);

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;

		ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_FORCE_DYNAMIC;
		ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiSerialNumber >> 8);
		ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiSerialNumber;

#if 1
		vSERIAL_dash(6);
		vSERIAL_sout(" Forcing Dynamic Lk ", 20);
		vSERIAL_UI16out(uiSerialNumber);
		vSERIAL_sout(" = ", 3);
		vTIME_showTime(ulLinkTime, FRAME_SLOT_TIME, NO_CRLF);
		vSERIAL_dash(6);
		vSERIAL_crlf();
#endif

		goto sos_exit;

	}

	/*-------  IF YOU ARE HERE THE 1ST LNK REQ IS GOOD IN EVERY WAY -----------*/

	/*------------  FILL THE NST FROM THE LINK BLK REQ'S --------------------*/

	/* LOOP FOR ALL ENTRYS IN THE LNK BLK */
	for (ucc = 0; ucc < ENTRYS_PER_LNKBLK_BLK; ucc++) {
		/* LOAD A TABLE LINK TIME ENTRY */
		ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, ucc, &ulLinkTime);
		lLinkFrame = lTIME_getFrameNumFromTime(ulLinkTime);
		ucLinkSlot = (uchar) lTIME_getSlotNumFromTime(ulLinkTime);

		/* EXIT IF BLK EMPTY */
		if (ulLinkTime == 0)
			break;

		/*------  SCHEDULING THE LNK FOR THE NEXT NST -----------------------*/

		/* FIND THE CLOSEST POSITION TO THIS SLOT */
		uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, ucLinkSlot, ucTskIndex);

		//unpack the slot information
		ucFoundSlot = (uchar) uiFoundSlot;
		ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

		/* IF NO SLOTS AVAILABLE -- STOP NOW */
		if (ucFoundSlot >= GENERIC_NST_LAST_IDX) {
#if 1		/* REPORT NO SLOTS AVAILABLE */
			vSERIAL_dash(6);
			vSERIAL_sout(" Lk ", 4);
			vSERIAL_UI16out(uiSerialNumber);
			vSERIAL_sout(" NoSlt ", 7);
			vSERIAL_dash(6);
			vSERIAL_crlf();
//			vRTS_showAllNSTentrys(ucNST_tblNum, YES_SHOW_TCB);
			vSERIAL_crlf();
#endif

			break;
		}

		/* SKIP THIS SLOT IF NOT EXACT */
		if (ucFoundSlot != ucLinkSlot) {

#if 1  /* REPORT A SLOT COLLISION */

			vSERIAL_dash(6);
			vSERIAL_sout(" Lk ", 4);
			vSERIAL_UI16out(uiSerialNumber);
			vSERIAL_sout(" CollideAt ", 11);
			vSERIAL_HB8out(ucLinkSlot);
			vSERIAL_dash(6);
			vSERIAL_crlf();

			// Build the report data element header
			vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
			ucMsgIndex = DE_IDX_RPT_PAYLOAD;

			ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_COLLISION;
			ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
			ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiSerialNumber >> 8);
			ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiSerialNumber;

			// Store DE
			vReport_LogDataElement(RPT_PRTY_COLLISION);
#endif

			// Fetch the link priority of task to be scheduled
			ucLNKBLK_ReadPriority(uiSerialNumber, &ucLinkPriority);

			// Search the slot for a communication task
			for (ucSubSlotIndex = 0; ucSubSlotIndex < MAXNUM_TASKS_PERSLOT; ucSubSlotIndex++) {

				// Temporary copy of the task index in the NST
				ucTempIdx = ucRTS_getNSTSubSlotentry(ucNST_tblNum, ucLinkSlot, ucSubSlotIndex);

				// Get the task ID
				if (ucTask_GetField(ucTempIdx, TSK_ID, &ulTaskID) != TASKMNGR_OK) {
					continue;
				}
				uiTaskID = (uint) ulTaskID;

				if (uiTaskID == TASK_ID_ROM || uiTaskID == TASK_ID_SOM ) {
					break;
				}
			}

			// Make sure the pre-existing task is a communication task before overwriting it
			if (uiTaskID == TASK_ID_ROM || uiTaskID == TASK_ID_SOM ) {

				ucTask_GetField(ucTempIdx, PARAM_SN, &ulTempSN);
				uiTempSN = (uint) ulTempSN;

				// Read the link priority of the scheduled task
				ucLNKBLK_ReadPriority(uiTempSN, &ucTempLinkPriority);

				// Overwrite scheduled task if the link priority is higher
				if (ucLinkPriority > ucTempLinkPriority) {

					// Increment the link priority of the overwritten task
					ucTempLinkPriority++;
					ucLNKBLK_WritePriority(uiTempSN, ucTempLinkPriority);

					// Find the link block index for this entry, then update the link state to reflect a failure to schedule
					ucLNKBLK_FindLinkTime(uiTempSN, ulLinkTime, &ucLnkIndex);
					ucLNKBLK_WriteLnkState(uiTempSN, ucLnkIndex, LINK_FAIL);

					// Check if we are going to overwrite the first entry in the other link block table
					if (ucLNKBLK_AnyGoodLinks(uiTempSN) == 0) {
							ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiTempSN, LNKREQ_1FRAME_1LNK, ulLinkTime);
					}

					// If the slot is in range then stuff it
					if (ucFoundSlot < GENERIC_NST_MAX_IDX) {
						// SLOT WAS FOUND, WAS AVAILABLE, WAS EXACT -- STUFF THE NST
						vRTS_putNSTSubSlotentry(ucNST_tblNum, //NST tbl (0 or 1)
								ucLinkSlot, ucSubSlotIndex, //NST slot number
								ucTskIndex //NST value
								);
					}

					// Build the report data element header
					vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
					ucMsgIndex = DE_IDX_RPT_PAYLOAD;

					ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_OVERWRITE;
					ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
					ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiTempSN >> 8);
					ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiTempSN;

					// Store DE
					vReport_LogDataElement(RPT_PRTY_OVERWRITE);

				}
				else {

					// Increment the link priority of the overwritten task
					ucLinkPriority++;
					ucLNKBLK_WritePriority(uiSerialNumber, ucLinkPriority);

					// Find the link block index for this entry, then update the link state to reflect a failure to schedule
					ucLNKBLK_FindLinkTime(uiSerialNumber, ulLinkTime, &ucLnkIndex);
					ucLNKBLK_WriteLnkState(uiSerialNumber, ucLnkIndex, LINK_FAIL);

					// Build the report data element header
					vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
					ucMsgIndex = DE_IDX_RPT_PAYLOAD;

					ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_NO_OVERWRITE;
					ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
					ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiSerialNumber >> 8);
					ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiSerialNumber;

					// Store DE
					vReport_LogDataElement(RPT_PRTY_NO_OVERWRITE);

					// Check if we are going to overwrite the first entry in the link block table
					if (ucLNKBLK_AnyGoodLinks(uiSerialNumber) == 0) {
							ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiSerialNumber, LNKREQ_1FRAME_1LNK, ulLinkTime);
					}

					// This continue prevents the task from being written to the NST when no overwrite is allowed
					continue;
				} // END: else
			} // END: if (uiTaskID)
		} // END: if (ucFoundSlot)
		else {

			// If the slot is in range then stuff it
			if (ucFoundSlot < GENERIC_NST_MAX_IDX) {
				// SLOT WAS FOUND, WAS AVAILABLE, WAS EXACT -- STUFF THE NST
				vRTS_putNSTSubSlotentry(ucNST_tblNum, //NST tbl (0 or 1)
						ucFoundSlot, ucFoundSubSlot, //NST slot number
						ucTskIndex //NST value
						);
			}
		}
		/* CHECK FOR MAX INDEX ERROR */
		if (ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST) {
			vSERIAL_sout("MxNSTidxInSchedOM2slot\r\n", 24); //report
			ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0; //clr bit
		}

	}/* END: for(ucc) */

	sos_exit:

	// Set the overwrite OK flag
	ucLNKBLK_SetFlag(uiSerialNumber, F_OVERWRITE);

	return;

}/* END: vRTS_schedule_Comm_slot() */


/***************  vRTS_schedule_interval_slot()  *****************************
 *
 * The Sample time terminology is as follows:
 *
 *
 *
 *Hr0                                               BaseTime
 *                          Sample0                             NxtSample
 * ³                          ³                         ³          ³
 * ³  lHr0_to_Sample0_inSec   ³lSample0_to_BaseTime     ³          ³
 * ³<------------------------>³<----------------------->³<-------->³
 * ³                          ³                         ³          ³
 * ³												       ³	      ³
 * ³												   	   ³
 * ³                          SysTim0          		   ³
 * ³			            (System clk startup)    	   ³
 * ³  lHr0_to_SysTim0_inSec      ³      BaseTime        ³
 * ³<--------------------------->³<-------------------->³
 * ³                             ³		  		       ³
 *
 *Hr0		                    0			       BaseTime
 *                             Time
 *
 * Main Equation:
 *
 *lHr0_to_Sample0_inSec + lSample0_to_BaseTime = lHr0_to_SysTim0_inSec + BaseTime
 *
 *
 *
 * SysTim0:	Point on time line that the Wizard was started (Internal Time 0).
 * CurTime:	Current Time in Seconds from SysTim0.
 * OpMode0:	Time in seconds from SysTim0 that opmode was started.
 * BaseTime: Rightmost time of CurTime or OpMode0.
 *
 * Hr0:		First hour mark to the left of SysTim0.
 * Sample0:	First Sample time as specified by user.
 * NxtSample: Time that the next sample should be taken.
 *
 *
 *****************************************************************************/

static void vRTS_schedule_interval_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		)
{
	long lBaseTime_inSec;
	long lEndTime_inSec;
	ulong ulSamplePhase;
	long lSample0_to_BaseTime_inSec;
	ulong ulSampleInterval_inSec;
	long lNumberOfSamples;
	long lSampleCnt;
	long lThisSampleTime;
	long lThisSlotIdx;
	uint uiFoundSlot;
	uchar ucFoundSlot;
	uchar ucFoundSubSlot;
	uint uiFlags;
	uchar ucNST_tblNum;
	uchar ucTaskIdxTemp;
	uchar ucProcID;
	uchar ucTskDuration;
	S_Task_Ctl S_Task;
	long lDispatchTime;
	ulong ulFlags, ulTskDuration, ulProcID;
	uint uiUniqueTaskID;

	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	// Determine the start time of the frame to be scheduled
	lBaseTime_inSec = lFrameNumToSched * SECS_PER_FRAME_L;

	// Determine the end time for the frame to be scheduled
	lEndTime_inSec = lBaseTime_inSec + (SECS_PER_SLOT_L * SLOTS_PER_FRAME_L);

	// Get the sampling interval if allowed
	if (ucTask_GetField(ucTskIndex, PARAM_INTERVAL, &ulSampleInterval_inSec) != TASKMNGR_OK)
		return;

	// Get the sampling phase if allowed
	if (ucTask_GetField(ucTskIndex, PARAM_PHASE, &ulSamplePhase) != TASKMNGR_OK)
		return;

	/* COMPUTE THE DIST FROM SAMPLE0 TO BASETIME */
	lSample0_to_BaseTime_inSec = lBaseTime_inSec - ulSamplePhase;

	/* COMPUTE THE NUM OF SAMPLES UP TO BASETIME */
	lNumberOfSamples = lSample0_to_BaseTime_inSec / ulSampleInterval_inSec;

	/* WE ARE NOW READY TO CALCULATE THE NEXT NST SLOTS USED BY THIS EVENT */
	/* LOOP FOR ALL SAMPLES THAT ARE WITHIN THIS FRAME */
	for (lSampleCnt = lNumberOfSamples;; lSampleCnt++) {

		// The computed sample time is the phase + the number of samples * the sampling interval
		lThisSampleTime = ulSamplePhase + (ulSampleInterval_inSec * lSampleCnt);

		if (lThisSampleTime >= lEndTime_inSec)
			break; //termination condition

		//insist we stay in this frame
		if (lThisSampleTime >= lBaseTime_inSec) {

			/* CONVERT THIS SAMPLE TIME TO A SLOT IDX */
			lThisSlotIdx = ((lThisSampleTime - lBaseTime_inSec) / SECS_PER_SLOT_L) % SLOTS_PER_FRAME_L;

			//subslot is in the high byte, slot is in the low byte
			uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, //NST tbl (0 or 1)
					(uchar) lThisSlotIdx, //Desired slot
					ucTskIndex);

			//unpack the slot information
			ucFoundSlot = (uchar) uiFoundSlot;
			ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

			if (ucTask_GetField(ucTskIndex, TSK_FLAGS, &ulFlags) != TASKMNGR_OK)
				break;
			uiFlags = (uint) ulFlags;

			/* IF THIS EVENT REQUIRES AND EXACT SLOT -- SKIP IT */
			if ((uiFlags & F_USE_EXACT_SLOT) && (uiFoundSlot != (uchar) lThisSlotIdx)) {
#if 0
				vSERIAL_sout("RTS:SchedIntrvl:ExactSltNotAvail\r\n", 34);
#endif
				break;
			}

			/* CHK IF NO SLOTS AVAILABLE */
			if (ucFoundSlot >= GENERIC_NST_MAX_IDX) {
#if 0
				vSERIAL_sout("RTS:SchedIntrvl:NoNSTsltsAvail\r\n", 32);
#endif
				break;
			}

			uchar ucTaskListIndex, ucCompatibleTaskFound;
			ucCompatibleTaskFound = FALSE;
			for(ucTaskListIndex = 0; ucTaskListIndex < MAXNUMTASKS; ucTaskListIndex++)
			{
				// Is the slot occupied with a compatible task
				if (ucTask_CheckComp(ucTskIndex, g_ucNextSlotTaskTable[ucNST_tblNum][ucFoundSlot][ucTaskListIndex])) {

					// A compatible task has been found
					ucCompatibleTaskFound = TRUE;

					// create new task that represents the combined task
					ucTaskIdxTemp = ucTask_CreateCombinedTask(g_ucNextSlotTaskTable[ucNST_tblNum][ucFoundSlot][ucTaskListIndex], ucTskIndex);

					// Fill the slot if the slot is in range
					if (ucFoundSlot < GENERIC_NST_MAX_IDX)
						vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, ucTaskListIndex, ucTaskIdxTemp);

					break;
				}

			}

//			// Is the slot occupied with a compatible task
//			if (ucTask_CheckComp(ucTskIndex, g_ucNextSlotTaskTable[ucNST_tblNum][ucFoundSlot][ucFoundSubSlot])) {
//				// create new task that represents the combined task
//				ucTaskIdxTemp = ucTask_CreateCombinedTask(g_ucNextSlotTaskTable[ucNST_tblNum][ucFoundSlot][ucFoundSubSlot], ucTskIndex);
//
//				// Fill the slot if the slot is in range
//				if (ucFoundSlot < GENERIC_NST_MAX_IDX)
//					vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, ucFoundSubSlot, ucTaskIdxTemp);
//			}
//			else {
			if(ucCompatibleTaskFound == FALSE){
				// Fill the slot if the slot is in range
				if (ucFoundSlot < GENERIC_NST_MAX_IDX)
					vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, ucFoundSubSlot, ucTskIndex);

				// Schedule request data task if needed
				if (ucTask_GetField(ucTskIndex, TSK_PROCESSORID, &ulProcID) != TASKMNGR_OK)
					break;
				ucProcID = (uchar) ulProcID;

				if (ucProcID != 0 && ucProcID != 6) {
					// Fetch the duration required for this task
					if (ucTask_GetField(ucTskIndex, PARAM_TSKDURATION, &ulTskDuration) != TASKMNGR_OK)
						return;
					ucTskDuration = (uchar) ulTskDuration;

					// Compute the dispatch time (in seconds) from the frame number, slot number, and task duration
					lDispatchTime = ucTskDuration + lFrameNumToSched * SECS_PER_FRAME_L + ucFoundSlot * SECS_PER_SLOT_I;

					// We need to guarantee a unique task ID here!!!!!!!!!
					uiUniqueTaskID = uiTask_GetUniqueID((uint) uslRAND_getFullSysSeed());

					// Load the request SP data task parameters
					S_Task.m_uiTask_ID = uiUniqueTaskID;
					S_Task.m_ucPriority = TASK_PRIORITY_RQSTSPDATA;
					S_Task.m_ucProcessorID = ucProcID;
					S_Task.m_ulTransducerID = (TASK_ID_RQSTSPDATA & TASK_TRANSDUCER_ID_MASK);
					S_Task.m_uiFlags = TASK_FLAGS_RQSTSPDATA;
					S_Task.m_ucState = TASK_STATE_ACTIVE;
					S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_RQSTSPDATA;
					S_Task.m_ulParameters[0x00] = lDispatchTime;
					S_Task.m_ulParameters[0x01] = 0;
					S_Task.m_ulParameters[0x02] = 0;
					S_Task.m_ulParameters[0x03] = 0;
					S_Task.m_cName = TASK_NAME_RQSTSPDATA;
					S_Task.ptrTaskHandler = vTask_SP_EndSlot;
					ucTask_CreateTask(S_Task); // create the task
				} // if(ucProcID)
				else if(ucProcID == 6){
					// Fetch the duration required for this task
					if (ucTask_GetField(ucTskIndex, PARAM_TSKDURATION, &ulTskDuration) != TASKMNGR_OK)
						return;
					ucTskDuration = (uchar) ulTskDuration;

					// Compute the dispatch time (in seconds) from the frame number, slot number, and task duration
					lDispatchTime = ucTskDuration + lFrameNumToSched * SECS_PER_FRAME_L + ucFoundSlot * SECS_PER_SLOT_I;

					// We need to guarantee a unique task ID here!!!!!!!!!
					uiUniqueTaskID = uiTask_GetUniqueID((uint) uslRAND_getFullSysSeed());

					// Load the request SP data task parameters
					S_Task.m_uiTask_ID = uiUniqueTaskID;
					S_Task.m_ucPriority = TASK_PRIORITY_RQSTSPDATA;
					S_Task.m_ucProcessorID = ucProcID;
					S_Task.m_ulTransducerID = (TASK_ID_RQSTSPDATA & TASK_TRANSDUCER_ID_MASK);
					S_Task.m_uiFlags = TASK_FLAGS_RQSTSPDATA;
					S_Task.m_ucState = TASK_STATE_ACTIVE;
					S_Task.m_ucWhoCanRun = RBIT_ALL;
					S_Task.m_ulParameters[0x00] = lDispatchTime;
					S_Task.m_ulParameters[0x01] = 0;
					S_Task.m_ulParameters[0x02] = 0;
					S_Task.m_ulParameters[0x03] = 0;
					S_Task.m_cName = "SCCRQ";
					S_Task.ptrTaskHandler = vTask_SCC_EndSlot;
					ucTask_CreateTask(S_Task); // create the task
				}

			}

			/* CHECK FOR MAX INDEX ERROR */
			if (ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST) {
				vSERIAL_sout("RTS:MxNSTidxInSchedule_interval_slot\r\n", 38);
				ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
			}

		}/* END: if(lThisSampleTime) */

	}/* END: for(lSampleCnt) */

	return;

}/* END: vRTS_schedule_interval_slot() */

///////////////////// vRTS_schedule_loadonly_slot() //////////////////////////
//! \brief This schedules evenly spaced NST slots -- count is set by load
//!
//!  While this function is not used in the G3 code, I think the concept may have
//!  potential for future use and I have left it in
//!
//! \param ucTskIndex, lFrameNumToSched
//! \return none
/////////////////////////////////////////////////////////////////////////////
static void vRTS_schedule_loadonly_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		)
{
//	uchar ucNST_tblNum;
//	uchar ucPrevNSTtblNum;
//	uchar ucc;
//	uchar ucReqSlot;
//	uchar ucFoundSubSlot;
//	uchar ucFoundSlot;
//	uint uiFoundSlot;
//	uchar ucLoadValOnly;
//	uint uiPreviousLoad;
//	uint uiNextLoad;
//
//	/* GET THE NST TABLE NUMBER FROM THE FRAME NUMBER */
//	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);
//
//	/* GET THE PREVIOUS NST TABLE NUM */
//	ucPrevNSTtblNum = (ucNST_tblNum ^ 0x01);

	/*------------ COPY THE LOAD NUM OVER TO NEXT LOAD  ------------------*/

//	/* LOAD THE PREVIOUS LOAD VALUE */
//	uiPreviousLoad = (uint) ulL2SRAM_getStblEntry(ucaDcntTblNum[ucPrevNSTtblNum], //Sched tbl num
//	    ucTaskId //Sched tbl idx
//	    );
//
//	/* GET THE NEW LOAD VALUE */
//	uiNextLoad = 0; //uiSTBL_handleLoadDowncnt(uiPreviousLoad);
//
//	/* STASH THE NEW LOAD IN THE NEXT TBL */
//	vL2SRAM_putStblEntry(ucaDcntTblNum[ucNST_tblNum], ucTaskId, (ulong) uiNextLoad);
	/*----------- DECIDE IF THIS ACTION IS TO RUN --------------------------*/
//
//	/* IF NO ALARM SET  -- DON'T SCHEDULE IT */
//	if (!(uiNextLoad & F_DCNT_ALARM))
//		goto LoadOnly_event_exit;
//
//	/* IF DOWNCOUNT = 0  -- DON'T SCHEDULE IT */
//	if ((uiNextLoad & F_DCNT_COUNT_ONLY_MASK) == 0)
//		goto LoadOnly_event_exit;
//
//	/*-------------  FILL THE NST ENTRIES  ----------------------------------*/
//
//	/* TRY TO CAPTURE EQUA-DISTANT LOCATIONS */
////	ucLoadValOnly = (uchar) (uiNextLoad & F_DCNT_COUNT_ONLY_MASK);
//	if (ucLoadValOnly >= GENERIC_NST_MAX_IDX_MASK)
//		ucLoadValOnly = GENERIC_NST_MAX_IDX;
//	for (ucc = 0; ucc < ucLoadValOnly; ucc++)
//	{
//		ucReqSlot = (uchar) (ucc * (GENERIC_NST_MAX_IDX / (ucLoadValOnly + 1)));
//
//		/* LOOK FOR A SLOT */
//		uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, //NST tbl (0 or 1)
//		    ucReqSlot, //Desired slot
//		    ucTskIndex);
//
//		//unpack the slot information
//		ucFoundSlot = (uchar) uiFoundSlot;
//		ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);
//
//		/* IF NO SLOTS AVAILABLE -- STOP NOW */
//		if (ucFoundSlot >= GENERIC_NST_MAX_IDX)
//			break;
//
//		/* YES WE HAVE A SLOT -- STUFF IT */
//		vRTS_putNSTSubSlotentry(ucNST_tblNum, //NST tbl (0 or 1)
//		    ucFoundSlot, ucFoundSubSlot, //NST slot number
//		    ucTskIndex //NST value
//		    );
//
//		/* CHECK FOR MAX INDEX ERROR */
//		if (ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST)
//		{
//			vSERIAL_sout("RTS:MxNSTidxInSchedule_LdOnly_slot\r\n", 36);
//			ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
//		}
//
//	}/* END: for(ucc) */
//
//	LoadOnly_event_exit:
//
//#if 0
//	{
//		vSERIAL_sout("RTS:SchedLdOnly- ", 17);
//		vACTION_showStblActionName(ucTskIndex);
//		vSERIAL_crlf();
//		//vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_TCB);
//	}
//#endif
//
//	return;
}/* END: vRTS_schedule_loadonly_slot() */

///////////////////// vRTS_schedule_downcount_slot() //////////////////////////
//! \brief This schedules a task when required based on the time parameter
//!
//!
//! \param ucTskIndex, lFrameNumToSched
//! \return none
/////////////////////////////////////////////////////////////////////////////
static void vRTS_schedule_downcount_slot(uchar ucTskIndex, long lFrameNumToSched)
{

	ulong ulDispatchTime;
	long lDispatchFrame;
	uchar ucDispatchSlot, ucFoundSlot, ucFoundSubSlot;
	uchar ucNST_tblNum;
	uint uiFoundSlot;

	// Get the NST table number from the frame number
	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	// Get the dispatch time for this task
	if (ucTask_GetField(ucTskIndex, PARAM_DISPATCHTIME, &ulDispatchTime) != TASKMNGR_OK)
		return;

	// Determine the dispatch frame for the task
	lDispatchFrame = lTIME_getFrameNumFromTime(ulDispatchTime);

	// Check if the desired frame is the frame being scheduled
	if (lDispatchFrame == lFrameNumToSched) {
		// Determine the desired slot number in this frame
		ucDispatchSlot = (uchar) lTIME_getSlotNumFromTime(ulDispatchTime);

		// Find the slot
		uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, ucDispatchSlot, ucTskIndex);

		//unpack the slot information
		ucFoundSlot = (uchar) uiFoundSlot;
		ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

//		// Make sure that this task isn't already scheduled
//		ucRTS_CheckNSTSlotforEntry(ucNST_tblNum, ucFoundSlot, ucTskIndex);  // This checks the task ID, we need to check task ID and processor ID

		if (ucFoundSlot < GENERIC_NST_MAX_IDX) {
			// Put task in the NST table
			vRTS_putNSTSubSlotentry(ucNST_tblNum, ucFoundSlot, ucFoundSubSlot, ucTskIndex);
		}
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
////!
////!
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//uchar ucRTS_isSlotAvailable(uchar ucNST_tblNum, uchar ucDesiredSlot, uchar ucTaskIndextoSched)
//{
//	uchar ucSleepTaskIndex;
//	uchar ucTaskCounter;
//	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];
//	uchar ucProcID;
//	ulong ulFlagsTasktoSched, ulProcID, ulFlags;
//	uint uiFlagsTasktoSched, uiFlags;
//
//	// Fetch the index of the sleep task
//	ucSleepTaskIndex = ucTask_FetchTaskIndex(TASK_ID_SLEEP);
//
//	// Get the flags for the task being scheduled
//	if (ucTask_GetField(ucTaskIndextoSched, TSK_FLAGS, &ulFlagsTasktoSched) != TASKMNGR_OK)
//		return FALSE;
//	uiFlagsTasktoSched = (uint) ulFlagsTasktoSched;
//
//	// If task requires entire slot, only return slot number when entire slot is available
//	if (uiFlagsTasktoSched & F_USE_FULL_SLOT && ucRTS_isSlotEmpty(ucDesiredSlot, ucNST_tblNum) == FALSE) {
//			return FALSE;
//	}
//
//	// Evaluate compatibility with all tasks in the slot
//	for (ucTaskCounter = 0; ucTaskCounter < MAXNUM_TASKS_PERSLOT; ucTaskCounter++) {
//
//		// Check if there is no task for this entry of the slot then there is nothing to test so continue
//		if (ucaSlotArray[ucTaskCounter] == GENERIC_NST_NOT_USED_VAL || (ucaSlotArray[ucTaskCounter] == ucSleepTaskIndex))
//			continue;
//
//		// Get flags if allowed.  If unable to get the flags then there is an invalid task in the subslot
//		if (ucTask_GetField(ucaSlotArray[ucTaskCounter], TSK_FLAGS, &ulFlags) != TASKMNGR_OK)
//			break;
//		uiFlags = (uint) ulFlags;
//
//		// If the processor ID of the task is for the CP make sure not to double book the slot
//		if (ucTask_GetField(ucTaskIndextoSched, TSK_PROCESSORID, &ulProcID) != TASKMNGR_OK)
//			return FALSE;
//		ucProcID = (uchar) ulProcID;
//
//		if (ucProcID == 0) {
//
//			// Check against each subslot
//			if((uiFlagsTasktoSched & uiFlags & F_USE_START_OF_SLOT))
//				return FALSE;
//
//			if(uiFlagsTasktoSched & uiFlags & F_USE_MIDDLE_OF_SLOT)
//				return FALSE;
//
//			if(uiFlagsTasktoSched & uiFlags & F_USE_END_OF_SLOT)
//				return FALSE;
//		}
//	} //END: for(ucTaskCounter)
//
//	return TRUE;
//}

//////////////////////////////////////////////////////////////////////////////////
//! \fn ucRTS_isSlotAvailable
//! \brief Determines if a task can be scheduled in the desired slot
//!
//!
//! \param ucNST_tblNum, The next slot table number
//! \param ucDesiredSlot, the desired slot
//! \param ucTaskIndextoSched, The index of the task to be scheduled
//////////////////////////////////////////////////////////////////////////////////
uchar ucRTS_isSlotAvailable(uchar ucNST_tblNum, uchar ucDesiredSlot, uchar ucTaskIndextoSched)
{

	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];
	ulong ulTaskFlagstoSched;
	uint uiTaskFlagstoSched;
	uint uiaFlagArray[MAXNUM_TASKS_PERSLOT] = {0};
	uchar ucProcID;
	ulong ulProcID;
	uchar ucTaskCounter;
	ulong ulFlags;

	// Get the flags for the task being scheduled
	if (ucTask_GetField(ucTaskIndextoSched, TSK_FLAGS, &ulTaskFlagstoSched) != TASKMNGR_OK)
		return FALSE;
	uiTaskFlagstoSched = (uint) ulTaskFlagstoSched;

	// Load the NST entries for this slot
	vRTS_getNSTentry(ucNST_tblNum, //NST tbl (0 or 1)
			(uchar) ucDesiredSlot, //NST slot number
			ucaSlotArray);

//	We want to get all flags for the tasks in the slot, then determine which subslots are occupied.
//	Then we can get rid of the check for conflict function.

	//Get the flags for all tasks in the slot
	for (ucTaskCounter = 0; ucTaskCounter < MAXNUM_TASKS_PERSLOT; ucTaskCounter++) {
		// Get flags if allowed otherwise flags = 0
		if (ucTask_GetField(ucaSlotArray[ucTaskCounter], TSK_FLAGS, &ulFlags) == TASKMNGR_OK) {
			uiaFlagArray[ucTaskCounter] = (uint) ulFlags;
		}
	} //END: for(ucTaskCounter)


	// If the slot contains the sleep task then it is available
	if(ucRTS_CheckNSTSlotforEntry(ucNST_tblNum, ucDesiredSlot, ucTask_FetchTaskIndex(TASK_ID_SLEEP)))
		return TRUE;

	// If a task in the slot uses the full slot then we cannot use it
	if (ucRTS_CheckforConflict(ucNST_tblNum, ucDesiredSlot, F_USE_FULL_SLOT))
		return FALSE;

	// If the processor ID of the task is for the CP make sure not to double book the slot
	if (ucTask_GetField(ucTaskIndextoSched, TSK_PROCESSORID, &ulProcID) != TASKMNGR_OK)
		return FALSE;
	ucProcID = (uchar) ulProcID;

	if (ucProcID == 0) {

		// If task requires entire slot, only return slot number when entire slot is available
		if ((uiTaskFlagstoSched & F_USE_FULL_SLOT) && ucRTS_isSlotEmpty(ucDesiredSlot, ucNST_tblNum) == FALSE) {
				return FALSE;
		}

		if (uiTaskFlagstoSched & F_USE_START_OF_SLOT) {
			if (ucRTS_CheckforConflict(ucNST_tblNum, ucDesiredSlot, F_USE_START_OF_SLOT))
				return FALSE;
		}
		if (uiTaskFlagstoSched & F_USE_MIDDLE_OF_SLOT) {
			if (ucRTS_CheckforConflict(ucNST_tblNum, ucDesiredSlot, F_USE_MIDDLE_OF_SLOT))
				return FALSE;
		}
		if (uiTaskFlagstoSched & F_USE_END_OF_SLOT) {
			if (ucRTS_CheckforConflict(ucNST_tblNum, ucDesiredSlot, F_USE_END_OF_SLOT))
				return FALSE;
		}
		if (uiTaskFlagstoSched & F_USE_FULL_SLOT) {
			if (ucRTS_CheckforConflict(ucNST_tblNum, ucDesiredSlot, F_USE_FULL_SLOT))
				return FALSE;
		}
	}
	// if we are here then the slot is available
	return TRUE;
}

/////////////////////// uiRTS_findnearestslot() //////////////////////////////
//! \brief Searches for an available slot starting at the desired slot and
//! 			 radiating out in both directions
//!
//! \param ucNST_tblNum, ucDesiredSlot, ucTaskIndextoSched
//! \return 0 to the maximum number of possible slots or 0xFFFF if no slot available
/////////////////////////////////////////////////////////////////////////////
uint uiRTS_findNearestNSTslot(uchar ucNST_tblNum, //0 or 1
		uchar ucDesiredSlot, uchar ucTaskIndextoSched)
{
	int iSlotDist;
	int iDesiredSlot;
	int iPosTestSlotNum;
	int iNegTestSlotNum;
	uint uiFoundSlot;
	uint uiSlotIndex;
	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];
	uchar ucSleepTaskIndex;

	// Convert desired slot to int
	iDesiredSlot = (int) ucDesiredSlot;

	// Fetch the index of the sleep task
	ucSleepTaskIndex = ucTask_FetchTaskIndex(TASK_ID_SLEEP);

	/* HUNT STARTS AT THE DESIRED SLOT AND RADIATES OUTWARD */
	for (iSlotDist = 0; iSlotDist < GENERIC_NST_MAX_IDX; iSlotDist++) {
		iPosTestSlotNum = iDesiredSlot + iSlotDist;
		iNegTestSlotNum = iDesiredSlot - iSlotDist;

		/* CHECK FOR LOOP TERMINATION CONDITION */
		//If the slot is outside the range of possible slots then skip
		if ((iPosTestSlotNum <= GENERIC_NST_LAST_IDX) && (iPosTestSlotNum >= 0)) {

			if (ucRTS_isSlotAvailable(ucNST_tblNum, iPosTestSlotNum, ucTaskIndextoSched) == TRUE) {

				// Load the NST entries for this slot
				vRTS_getNSTentry(ucNST_tblNum, (uchar) iPosTestSlotNum, ucaSlotArray);

				// Find an empty index in the slot
				for (uiSlotIndex = 0; uiSlotIndex < MAXNUM_TASKS_PERSLOT; uiSlotIndex++) {
					if (ucaSlotArray[uiSlotIndex] == GENERIC_NST_NOT_USED_VAL || (ucaSlotArray[uiSlotIndex] == ucSleepTaskIndex)) {
						uiFoundSlot = (uiSlotIndex << 8);
						uiFoundSlot |= (uchar) iPosTestSlotNum;
						return (uiFoundSlot);
					}// End: if(ucaSlotArray)
				}// End: for(uiSlotIndex)
			}// End: if(ucRTS_isSlotAvailable
		}// End: if(iPosTestSlotNum)

		/* NOW CHECK THE NEGATIVE */
		/* CHECK FOR LOOP TERMINATION CONDITION */
		//If the slot is outside the range of possible slots then continue
		if ((iNegTestSlotNum <= GENERIC_NST_LAST_IDX) && (iNegTestSlotNum >= 0)) {

			if (ucRTS_isSlotAvailable(ucNST_tblNum, iNegTestSlotNum, ucTaskIndextoSched) == TRUE) {
				// Load the NST entries for this slot
				vRTS_getNSTentry(ucNST_tblNum, (uchar) iNegTestSlotNum, ucaSlotArray);

				// Find an empty index in the slot
				for (uiSlotIndex = 0; uiSlotIndex < MAXNUM_TASKS_PERSLOT; uiSlotIndex++) {
					if (ucaSlotArray[uiSlotIndex] == GENERIC_NST_NOT_USED_VAL || (ucaSlotArray[uiSlotIndex] == ucSleepTaskIndex)) {
						uiFoundSlot = (uiSlotIndex << 8);
						uiFoundSlot |= (uchar) iNegTestSlotNum;
						return (uiFoundSlot);
					}// End: if(ucaSlotArray)
				}// End: for(uiSlotIndex)
			}// End: if(ucRTS_isSlotAvailable)
		}// End: if(iNegTestSlotNum)

	}/* END: for(ucSlotDist) */

	return (0xFFFF);

}/* END: uiRTS_findNearestNSTslot() */

///////////////////////  vRTS_runScheduler()  ////////////////////////////////
//! \brief
//!
//!
//! \param
//! \return
/////////////////////////////////////////////////////////////////////////////
void vRTS_runScheduler(void)
{
	long lNextFrameNum;

	/* GET THE NEXT FRAME NUMBER */
	lNextFrameNum = lGLOB_lastAwakeFrame + 1L;

	/* SAVE THE CURRENT TIME INTO FRAM IN CASE WE CRASH */
	vL2FRAM_stuffSavedTime((ulong) lTIME_getSysTimeAsLong());

#if 0
	/* GO RUN THE EVALUATOR */
	vSYSACT_do_EvaluateSts();
#endif

	/* GO HANDLE THE SCHEDULING */
	vRTS_scheduleNSTtbl(lNextFrameNum);

	/* COMPUTE THE SYSTEM LFACTOR */
	vTask_ComputeSysLFactor();

	/* SHOW THE NST TABLE */
#if 0
	{
		uchar ucNST_tblNum;
		ucNST_tblNum = (uchar)(lNextFrameNum % 2L); //compute NST tbl from frame
		vRTS_showAllNSTentrys(ucNST_tblNum,YES_SHOW_TCB);
	}
#endif

	// Show the link info
#if 0
	vLNKBLK_showAllLnkBlkTblEntrys();
#endif
	return;

}/* END: vRTS_runScheduler() */

/////////////////////// vRTS_showActionHdrLine() //////////////////////////////
//! \brief
//!
//!
//!
//! \param
//! \return
/////////////////////////////////////////////////////////////////////////////
void vRTS_showTaskHdrLine(uchar ucCRLF_termFlg //YES_CRLF, NO_CRLF
		)
{
	uchar ucCounter;
	long lUpTime;
	long lTmp;
	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];

	// Only print if the USB cable is plugged in and USCI is running
	if (UCA1CTL1 & UCSWRST)
		return;

	//Fetch the actions in the slot and store them in the ucaSlotArray
	vRTS_getNSTentry(ucGLOB_lastAwakeNSTtblNum, ucGLOB_lastAwakeSlot, ucaSlotArray);

	vDAYTIME_convertSysTimeToShowDateAndTime(NUMERIC_FORM); //daytime
	vSERIAL_sout("  ", 2);

	vSERIAL_HBV32out((ulong) lGLOB_lastAwakeFrame); //frame num
	vSERIAL_bout(':');
	vSERIAL_HB8out(ucGLOB_lastAwakeSlot); //slot num

	vSERIAL_sout("  ", 2);

	for (ucCounter = 0; ucCounter < MAXNUM_TASKS_PERSLOT; ucCounter++) {
		if (ucaSlotArray[ucCounter] != GENERIC_NST_NOT_USED_VAL) {
			vTask_showTaskName(ucaSlotArray[ucCounter]);
		}
		else {
			vSERIAL_sout(" --- ", 5);
		}
		vSERIAL_sout("  ", 2);
	}

	vSERIAL_sout("  ", 2); //Sys version num
	vMAIN_showVersionNum();
	vSERIAL_bout(':');
	vMODOPT_showCurRole();
	vSERIAL_bout(':');
	vL2FRAM_showSysID(); //sys ID

	vSERIAL_sout("  Up ", 5); //uptime
	lUpTime = (lTIME_getSysTimeAsLong() - lGLOB_initialStartupTime);
	/* COMPUTE DAYS UP */
	lTmp = lUpTime / 86400L;
	vSERIAL_IV32out(lTmp); //Days
	vSERIAL_bout(':');
	/* SHOW REST OF UP TIME */
	lUpTime %= 86400L;
	vDAYTIME_convertSecToShow(lUpTime);

	if (ucCRLF_termFlg)
		vSERIAL_crlf();

	return;

}/* END: vRTS_showActionHdrLine() */

/////////////////////// vRTS_convertAllRDC4slotsToSleep()  //////////////////
//! \brief converts the current NST/Next NST RDC4's to sleep
//!
//!
//! \param
//! \return
/////////////////////////////////////////////////////////////////////////////
void vRTS_convertAllRTJslotsToSleep(void)
{
	uchar ucNST_slotCnt;
	uchar ucNST_SubslotCnt;
	uchar ucNST_tblNum;
	uchar ucTskIndex, ucRTJTskIndex;

	// Find the index of the request to join task in the task list
	ucRTJTskIndex= ucTask_FetchTaskIndex(TASK_ID_RTJ);

	/* DO BOTH NST TABLES */
	for (ucNST_tblNum = 0; ucNST_tblNum < 2; ucNST_tblNum++) {
		for (ucNST_slotCnt = 0; ucNST_slotCnt < GENERIC_NST_MAX_IDX; ucNST_slotCnt++) {
			for (ucNST_SubslotCnt = 0; ucNST_SubslotCnt < MAXNUM_TASKS_PERSLOT; ucNST_SubslotCnt++) {
				/* GET THE NST ENTRIES */
				ucTskIndex = ucRTS_getNSTSubSlotentry(ucNST_tblNum, ucNST_slotCnt, ucNST_SubslotCnt);

				/* FIND THE RTJ ENTRYS */
				if (ucTskIndex == ucRTJTskIndex) {

					/* FOUND ONE CONVERT IT TO SLEEP */
					vRTS_putNSTSubSlotentry(ucNST_tblNum, ucNST_slotCnt, ucNST_SubslotCnt, GENERIC_NST_NOT_USED_VAL);

					/* CHECK FOR MAX INDEX ERROR */
					if (ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST) {
						vSERIAL_sout("RTS:MxNSTidxInConvertAllRDC4slotsToSlp\r\n", 40);
						ucGLOB_debugBits1.debugBits1_STRUCT.DBG_MaxIdxWriteToNST = 0;
					}

					//Since there is only one RTJ allowed per slot, then we can break out of this loop once we find one
					break;
				}
			} //END: ucNST_SubslotCnt

		}/* END: for(ucNST_slotCnt) */

	}/* END: for(ucNST_tblNum) */

#if 0
	vRTS_showAllNSTentrys(0,YES_SHOW_TCB);
	vRTS_showAllNSTentrys(1,YES_SHOW_TCB);
#endif

	return;

}/* END: vRTS_convertAllRDC4slotsToSleep() */

/////////////////////// ucRTS_nextNSTnum() //////////////////////////////
//! \brief Calculates the next NST number from the last awake frame
//!
//!
//! \param none
//! \return ucNextNSTnum
/////////////////////////////////////////////////////////////////////////////
uchar ucRTS_nextNSTnum(void)
{
	long lNextFrameNum;
	uchar ucNextNSTnum;

	/* GET THE NEXT FRAME NUMBER */
	lNextFrameNum = lGLOB_lastAwakeFrame + 1L;

	/* GET THE NEXT NST NUMBER */
	ucNextNSTnum = (uchar) (lNextFrameNum % 2L); //compute NST tbl from frame

	return (ucNextNSTnum);

} // END: ucRTS_nextNSTnum()

/////////////////////// ucRTS_thisNSTnum()  //////////////////////////////
//! \brief This function computes the NST number from the last awake frame
//!
//!
//! \param none
//! \return ucthisNSTnum
/////////////////////////////////////////////////////////////////////////////
uchar ucRTS_thisNSTnum(void)
{
	uchar ucthisNSTnum;

	/* GET THE NEXT NST NUMBER */
	ucthisNSTnum = (uchar) (lGLOB_lastAwakeFrame % 2L); //compute NST tbl from frame

	return (ucthisNSTnum);

}/* END: ucRTS_thisNSTnum() */

#if 0
/*********************  NOTE  ************************************
 *
 * Under the new scheduler change the scheduler runs at the
 * end of a frame so there is no need to check if it has already
 * run.
 *
 *****************************************************************/

/*********************  ucRTS_hasSchedRun()  *********************************
 *
 * Ret 1 if shed has run
 *	  0 if sched not run
 *
 * if in currently in sched rets 0
 *
 ******************************************************************************/

uchar ucRTS_hasSchedRun(
		void
)
{
	if(lGLOB_lastScheduledFrame > lGLOB_lastAwakeFrame) return(1);
	return(0);

}/* END: ucRTS_hasSchedRun() */
#endif

///////////////////// ucRTS_lastScheduledNSTnum() //////////////////////////////
//! \brief Returns the index of the last scheduled NST tbl
//!
//!
//! \param none
//! \return ucRTS_thisNSTnum()
/////////////////////////////////////////////////////////////////////////////
uchar ucRTS_lastScheduledNSTnum(void)
{

#if 0
	/*********************  NOTE  ************************************
	 *
	 * Under the new scheduler change the scheduler runs at the
	 * end of a frame so there is no need to check if it has already
	 * run.
	 *
	 *****************************************************************/
	if(ucRTS_hasSchedRun()) return(ucRTS_nextNSTnum());
#endif

	return (ucRTS_thisNSTnum());

} // END: ucRTS_lastScheduledNSTnum()

////////////////////////////////////////////////////////////////////////////////
//! \brief Checks to see if a slot contains a task that conflicts with the
//! 			passed task
//!
//! \param ucNSTslotNum: the slot number of interest
//! 			 ucTaskFlags: flags of the task of interest
//!
//! \return 1 if there is a conflict 0 if not
///////////////////////////////////////////////////////////////////////////////
uchar ucRTS_CheckforConflict(uchar ucNST_tblNum, uchar ucNSTslotNum, uint uiTaskFlags)
{
	uchar ucaSlotArray[MAXNUM_TASKS_PERSLOT];
	uchar ucTaskCounter;
	uint uiScheduledFlags = 0;
	ulong ulScheduledFlags;

	//Fetch the actions in the slot and store them in the ucaSlotArray
	vRTS_getNSTentry(ucNST_tblNum, ucNSTslotNum, ucaSlotArray);

	//Read the slot array and fetch flags
	for (ucTaskCounter = 0; ucTaskCounter < MAXNUM_TASKS_PERSLOT; ucTaskCounter++) {
		//Get the flags for the task in the slot
		if (ucTask_GetField(ucaSlotArray[ucTaskCounter], TSK_FLAGS, &ulScheduledFlags) == TASKMNGR_OK) {
			uiScheduledFlags = (uint) ulScheduledFlags;

			// If there is a conflict return 1
			if (uiScheduledFlags & uiTaskFlags)
				return 1;

			// Reset the flag variable
			uiScheduledFlags = 0;
		}
	}

	// No conflict
	return 0;
} // END:  ucRTS_CheckforConflict

////////////////////////////////////////////////////////////////////////////
//!
//! \brief Schedules command tasks. This is used when commands are added to the
//!	task list during opmode comm during scheduling.
//!
//! This is an alternative to scheduling a command as soon as it is received
//!	which fits the scheduling paradigm of the WiSARD OS
//!
////////////////////////////////////////////////////////////////////////////
static void vRTS_schedule_command_slot(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lFrameNumToSched //Frame number to schedule
		)
{
	uchar ucNST_tblNum, ucAttemptCount;
	uint uiFoundSlot;
	uchar ucFoundSlot;
	uchar ucFoundSubSlot;
	ulong ulTskDuration, ulProcID;
	uchar ucProcID = 0;
	long lDispatchTime;
	S_Task_Ctl S_Task;

	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lFrameNumToSched);

	for (ucAttemptCount = 1; ucAttemptCount < 10; ucAttemptCount++) {
		//subslot is in the high byte, slot is in the low byte
		uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, //NST tbl (0 or 1)
				ucAttemptCount, // Close to the start of the next frame
				ucTskIndex);

		//unpack the slot information
		ucFoundSlot = (uchar) uiFoundSlot;
		ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

		if (ucFoundSlot < GENERIC_NST_MAX_IDX)
			break;
	}

	if (ucFoundSlot < GENERIC_NST_MAX_IDX) {
		/* YES WE HAVE A SLOT -- STUFF IT */
		vRTS_putNSTSubSlotentry(ucNST_tblNum, //NST tbl (0 or 1)
				ucFoundSlot, ucFoundSubSlot, //NST slot number
				ucTskIndex //NST value
				);

#if 0
		vSERIAL_sout("CMD Schd\r\n", 10);
		vRTS_showAllNSTentrys(ucNST_tblNum, NO_SHOW_TCB);
#endif

		// Schedule request data task if needed
		if (ucTask_GetField(ucTskIndex, TSK_PROCESSORID, &ulProcID) != TASKMNGR_OK) {
#if 0
			vSERIAL_sout("CMD Sched Failed\r\n", 18);
#endif
		}

		ucProcID = (uchar) ulProcID;

		if (ucProcID != 0) {
			// Fetch the duration required for this task
			ucTask_GetField(ucTskIndex, PARAM_TSKDURATION, &ulTskDuration);

			// Compute the dispatch time (in seconds) from the frame number, slot number, and task duration
			lDispatchTime = ulTskDuration + lFrameNumToSched * SECS_PER_FRAME_L + ucFoundSlot * SECS_PER_SLOT_I;

			// Load the sleep task parameters
			S_Task.m_uiTask_ID = uiTask_GetUniqueID((uint) uslRAND_getFullSysSeed());//TASK_ID_RQSTSPDATA;
			S_Task.m_ucPriority = TASK_PRIORITY_RQSTSPDATA;
			S_Task.m_ucProcessorID = ucProcID;
			S_Task.m_ulTransducerID = (TASK_ID_RQSTSPDATA & TASK_TRANSDUCER_ID_MASK);
			S_Task.m_uiFlags = TASK_FLAGS_RQSTSPDATA;
			S_Task.m_ucState = TASK_STATE_ACTIVE;
			S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_RQSTSPDATA;
			S_Task.m_ulParameters[0x00] = lDispatchTime;
			S_Task.m_ulParameters[0x01] = 0;
			S_Task.m_ulParameters[0x02] = 0;
			S_Task.m_ulParameters[0x03] = 0;
			S_Task.m_cName = TASK_NAME_RQSTSPDATA;
			S_Task.ptrTaskHandler = vTask_SP_EndSlot;
			ucTask_CreateTask(S_Task); // create the task
		} // if(ucProcID)
	} // if(uiFoundSlot)
}

////////////////////////////////////////////////////////////////////////////
//!	\fn vRTS_schedule_ASAP
//! \brief Schedules a task as soon as possible.
//!
//! This is not intended for use by the scheduler function pointer table.
//! This is to be called by the application when a task needs to be scheduled as
//! soon as possible.  If it cannot be scheduled in this frame then the normal
//! scheduler will schedule the task.
//!
//! \param ucTskIndex The index of the task to be scheduled
//!	\param lCurrentTime The current time
//!
////////////////////////////////////////////////////////////////////////////
void vRTS_schedule_ASAP(uchar ucTskIndex, //Tbl Idx of action to schedule
		long lCurrentTime //Current time
		)
{
	uchar ucNST_tblNum;
	uint uiFoundSlot;
	uchar ucFoundSlot;
	uchar ucFoundSubSlot;
	uchar ucSlot;
	uchar ucAttemptCount;


	ucNST_tblNum = ucRTS_computeNSTfromFrameNum(lTIME_getFrameNumFromTime(lCurrentTime));

	// Determine current slot number
	ucSlot = (uchar)lTIME_getSlotNumFromTime(lCurrentTime);

	// Try a several times to schedule the task in this frame
	for (ucAttemptCount = 0; ucAttemptCount < 5; ucAttemptCount++) {

		if(ucSlot >= GENERIC_NST_MAX_IDX)
			break;

		//subslot is in the high byte, slot is in the low byte
		uiFoundSlot = uiRTS_findNearestNSTslot(ucNST_tblNum, //NST tbl (0 or 1)
				(ucSlot), // Start with the next slot
				ucTskIndex);

		//unpack the slot information
		ucFoundSlot = (uchar) uiFoundSlot;
		ucFoundSubSlot = (uchar) (uiFoundSlot >> 8);

		if((ucFoundSlot >= ucSlot) && (uiFoundSlot < GENERIC_NST_MAX_IDX))
			break;

		// Increment the desired slot
		ucSlot++;
	}

	if (uiFoundSlot < GENERIC_NST_MAX_IDX) {
		/* YES WE HAVE A SLOT -- STUFF IT */
		vRTS_putNSTSubSlotentry(ucNST_tblNum, //NST tbl (0 or 1)
				ucFoundSlot, ucFoundSubSlot, //NST slot number
				ucTskIndex //NST value
				);

#if 0
		vSERIAL_sout("CMD Schd\r\n", 10);
		vRTS_showAllNSTentrys(ucNST_tblNum, NO_SHOW_TCB);
#endif
	}
}

////////////////////////////////////////////////////////////////////////////
//! \fn vRTS_CreateRadioDiagTask
//! \brief Creates the task to run radio diagnostics
////////////////////////////////////////////////////////////////////////////
void vRTS_CreateRadioDiagTask(void)
{
	S_Task_Ctl S_Task;
	uchar ucTaskIndex;

	// Get the task index
	ucTaskIndex = ucTask_FetchTaskIndex(TASK_ID_RADIO_DIAG);

	// Only create the task if it does not already exist
	if (ucTaskIndex == INVALID_TASKINDEX) {
		// Create the write FRAM to SD card task
		S_Task.m_uiTask_ID = TASK_ID_RADIO_DIAG;
		S_Task.m_uiFlags = (F_SUICIDE | TASK_FLAGS_RADIO_DIAG);
		S_Task.m_ucPriority = TASK_PRIORITY_RADIO_DIAG;
		S_Task.m_ucProcessorID = (uchar) (TASK_ID_RADIO_DIAG >> 8);
		S_Task.m_ulTransducerID = (ulong) (TASK_ID_RADIO_DIAG & TASK_TRANSDUCER_ID_MASK);
		S_Task.m_ucState = TASK_STATE_ACTIVE;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_RADIO_DIAG;
		S_Task.m_ulParameters[0] = 0;
		S_Task.m_ulParameters[1] = 0;
		S_Task.m_ulParameters[2] = 0;
		S_Task.m_ulParameters[3] = 0;
		S_Task.m_cName = "RDTST";
		S_Task.ptrTaskHandler = vTask_RuntimeRadioDiag;

		ucTask_CreateTask(S_Task);
	}
}

////////////////////////////////////////////////////////////////////////////
//! \fn vRTS_CheckSPDataPending
//! \brief Checks to see if an SP is ready to transmit data
////////////////////////////////////////////////////////////////////////////
void vRTS_CheckSPDataPending(void)
{

	uint16 uiUniqueTaskID;
	uchar ucTaskIndex;
	S_Task_Ctl S_Task;

	if (g_ucSP1Ready == 1 || g_ucSP2Ready == 1 || g_ucSP3Ready == 1 || g_ucSP4Ready == 1) {
		S_Task.m_ucPriority = TASK_PRIORITY_RQSTSPDATA;
		S_Task.m_ulTransducerID = (TASK_ID_RQSTSPDATA & TASK_TRANSDUCER_ID_MASK);
		S_Task.m_uiFlags = TASK_FLAGS_RQSTSPDATA;
		S_Task.m_ucState = TASK_STATE_ACTIVE;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_RQSTSPDATA;
		S_Task.m_ulParameters[0x00] = 0;
		S_Task.m_ulParameters[0x01] = 0;
		S_Task.m_ulParameters[0x02] = 0;
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = TASK_NAME_RQSTSPDATA;
		S_Task.ptrTaskHandler = vTask_SP_EndSlot;
	}

	if (g_ucSP1Ready == 1) {

		// Clear the flag
		g_ucSP1Ready = 0;

		// We need to guarantee a unique task ID here!!!!!!!!!
		uiUniqueTaskID = uiTask_GetUniqueID((uint) uslRAND_getFullSysSeed());

		// Load the request SP data task parameters
		S_Task.m_uiTask_ID = uiUniqueTaskID;
		S_Task.m_ucProcessorID = 1;

		ucTask_CreateTask(S_Task); // create the task

		// Update the task index if the task was created
		ucTaskIndex = ucTask_FetchTaskIndex(uiUniqueTaskID);

		// Try to schedule it as soon as possible (if not normal scheduler will get it done)
		vRTS_schedule_ASAP(ucTaskIndex, lTIME_getSysTimeAsLong());
	}

	if (g_ucSP2Ready == 1) {

		// Clear the flag
		g_ucSP2Ready = 0;

		// We need to guarantee a unique task ID here!!!!!!!!!
		uiUniqueTaskID = uiTask_GetUniqueID((uint) uslRAND_getFullSysSeed());

		// Load the request SP data task parameters
		S_Task.m_uiTask_ID = uiUniqueTaskID;
		S_Task.m_ucProcessorID = 2;
		ucTask_CreateTask(S_Task); // create the task

		// Update the task index if the task was created
		ucTaskIndex = ucTask_FetchTaskIndex(uiUniqueTaskID);

		// Try to schedule it as soon as possible (if not normal scheduler will get it done)
		vRTS_schedule_ASAP(ucTaskIndex, lTIME_getSysTimeAsLong());
	}

	if (g_ucSP3Ready == 1) {

		// Clear the flag
		g_ucSP3Ready = 0;

		// We need to guarantee a unique task ID here!!!!!!!!!
		uiUniqueTaskID = uiTask_GetUniqueID((uint) uslRAND_getFullSysSeed());

		// Load the request SP data task parameters
		S_Task.m_uiTask_ID = uiUniqueTaskID;
		S_Task.m_ucProcessorID = 3;
		ucTask_CreateTask(S_Task); // create the task

		// Update the task index if the task was created
		ucTaskIndex = ucTask_FetchTaskIndex(uiUniqueTaskID);

		// Try to schedule it as soon as possible (if not normal scheduler will get it done)
		vRTS_schedule_ASAP(ucTaskIndex, lTIME_getSysTimeAsLong());
	}

	if (g_ucSP4Ready == 1) {

		// Clear the flag
		g_ucSP4Ready = 0;

		// We need to guarantee a unique task ID here!!!!!!!!!
		uiUniqueTaskID = uiTask_GetUniqueID((uint) uslRAND_getFullSysSeed());

		// Load the request SP data task parameters
		S_Task.m_uiTask_ID = uiUniqueTaskID;
		S_Task.m_ucProcessorID = 4;
		ucTask_CreateTask(S_Task); // create the task

		// Update the task index if the task was created
		ucTaskIndex = ucTask_FetchTaskIndex(uiUniqueTaskID);

		// Try to schedule it as soon as possible (if not normal scheduler will get it done)
		vRTS_schedule_ASAP(ucTaskIndex, lTIME_getSysTimeAsLong());
	}
}

/*-------------------------------  MODULE END  ------------------------------*/
