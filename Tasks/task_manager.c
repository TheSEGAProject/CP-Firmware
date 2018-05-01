////////////////////////////////////////////////////////////////////////
//! \file task.c
//! \brief Task manager module
//!
//! All task interactions are handled between the this module and the scheduler
//!
//!   Created on: Dec 31, 2013
//!       Author: cp397
//!
/////////////////////////////////////////////////////////////////////////
#include "msp430.h"
#include "std.h"		// standard definitions
#include "task.h" 	// task definitions
#include "rts.h"		// Scheduler
#include "mem_mod.h" 	// memory module definitions
#include "sysact.h" //some tasks are here we need to examine the reasoning behind this and consolidate tasks if possible
#include "SP.h"
#include "modopt.h"
#include "serial.h"
#include "gs.h"				// Garden server comm
#include "time.h"
#include "lnkblk.h"		//Link block definitions
#include "report.h"
#include "misc.h"			// Miscellaneous functions
#include "fulldiag.h"
#include "main.h"
#include "scc.h"

//! \defgroup Radio Task Parameter Indices
//! @{
//! \def PARAM_SN
//! \brief Serial number of other node
#define PARAM_IDX_SN			0x00
//! \def PARAM_STATUS
//! \brief Includes dropped nodes
#define PARAM_IDX_STATUS	0x01
//! @}

#define PARAM_IDX_DISPATCHTIME		0x00
#define PARAM_IDX_INTERVAL				0x00
#define PARAM_IDX_LFACT						0x01
#define PARAM_IDX_TSKDURATION			0x02

extern int iGLOB_completeSysLFactor; //global load factor
extern ulong ulGLOB_msgSysLFactor; // load factor from reporting
extern volatile uint8 ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];


//! \var cSPNames
//! \brief Holds the names of the SP boards
char *cSPNames[NUMBER_SPBOARDS] = { "SP1  ", "SP2  ", "SP3  ", "SP4  " };

//! \var p_saTaskList
//! \brief List of task control blocks

//! \var p_saTaskList
//! \brief This contains all the task resources needed by the scheduler and the dispatcher
//! The table is divided into two sections. The first part contains static tasks and the second part contains
//! tasks that have been created by the scheduler.  This list is initialized with several fixed tasks.
S_Task_Ctl p_saTaskList[MAXNUMTASKS];

//! \var g_ucNxtTskIdx
//! \brief Next free location for the creation of a task within the task list
static unsigned char g_ucNxtTskIdx;

static unsigned char g_ucNxtDynTskIdx;

//////////////////////////////////////////////////////////////////////////
//!
//! \brief Dummy function assigned to empty function pointers in the task
//!        list.  This way the device won't hang if accidently attempts
//! 			 to execute an empty task.
//!
////////////////////////////////////////////////////////////////////////
void vTask_Dummy(void)
{
	return;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Initializes the task list and loads executable tasks.
//!
//! This function loads a set of tasks that all the WiSARDs have and then
//! checks the mode option bits to determine if some additional tasks
//! should be created.  This looks ugly, but the other options weren't
//! much better
//!
//! \param none
//! \return T_TaskStatus
/////////////////////////////////////////////////////////////////////////
T_TaskStatus ucTask_Init(void)
{
	uchar ucIndex;
	uchar ucCmdByteCount;
	uchar ucFoundMatch, ucJJ, ucTSBCount;
	signed char cII;
	uint uiTSB_TaskID;
	ulong ulTCB_TaskID;
	S_Task_Ctl S_Task;
	uchar ucRetVal;

	// Assume success
	ucRetVal = TASKMNGR_OK;

	// Start the next task index pointer at the start of the task list
	g_ucNxtTskIdx = 0;

	// Set the dynamically created task pointer to the area after the partition in the task list
	g_ucNxtDynTskIdx = TASKPARTITION;

	// Set some of the fields of the task list to a known state
	for (ucIndex = g_ucNxtTskIdx; ucIndex < MAXNUMTASKS; ucIndex++) {
		// Set the task IDs to invalid
		p_saTaskList[ucIndex].m_uiTask_ID = 0xFFFF;
		p_saTaskList[ucIndex].m_cName = "  ---";
		p_saTaskList[ucIndex].m_ucState = TASK_STATE_IDLE;
		p_saTaskList[ucIndex].m_ucWhoCanRun = 0x00;
		p_saTaskList[ucIndex].ptrTaskHandler = vTask_Dummy;
		p_saTaskList[ucIndex].m_ucCmdLength = 0;

		// Set all elements in the command parameters array to 0
		for (ucCmdByteCount = 0; ucCmdByteCount < MAXCMDPARAM; ucCmdByteCount++) {
			p_saTaskList[ucIndex].m_ucCmdParam[ucCmdByteCount] = 0;
		}

	}

	// Load some of the basic tasks that all wisards have

	// Load the sleep task parameters
	S_Task.m_uiTask_ID = TASK_ID_SLEEP;
	S_Task.m_ucPriority = TASK_PRIORITY_SLEEP;
	S_Task.m_ucProcessorID = (uchar) (TASK_ID_SLEEP >> 8);
	S_Task.m_ulTransducerID = (TASK_ID_SLEEP & TASK_TRANSDUCER_ID_MASK);
	S_Task.m_uiFlags = TASK_FLAGS_SLEEP;
	S_Task.m_ucState = TASK_INIT_STATE_SLEEP;
	S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_SLEEP;
	S_Task.m_ulParameters[0x00] = 0x00;
	S_Task.m_ulParameters[0x01] = 0;
	S_Task.m_ulParameters[0x02] = 0;
	S_Task.m_ulParameters[0x03] = 0;
	S_Task.m_cName = " --- ";
	S_Task.ptrTaskHandler = vTask_Sleep;
	if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
		ucRetVal = TASKMNGR_OVRFLOW;

	// Load the sample input voltage task parameters
	S_Task.m_uiTask_ID = TASK_ID_V_IN;
	S_Task.m_ucPriority = TASK_PRIORITY_V_IN;
	S_Task.m_ucProcessorID = (uchar) (TASK_ID_V_IN >> 8);
	S_Task.m_ulTransducerID = (TASK_ID_V_IN & TASK_TRANSDUCER_ID_MASK);
	S_Task.m_uiFlags = TASK_FLAGS_V_IN;
	S_Task.m_ucState = TASK_INIT_STATE_V_IN;
	S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_V_IN;
	S_Task.m_ulParameters[0x00] = TASK_INTERVAL_V_IN;
	S_Task.m_ulParameters[0x01] = TASK_STS_LFACT_V_IN;
	S_Task.m_ulParameters[0x02] = 0;
	S_Task.m_ulParameters[0x03] = 0;
	S_Task.m_cName = "BATT ";
	S_Task.ptrTaskHandler = vTask_Batt_Sense;
	if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
		ucRetVal = TASKMNGR_OVRFLOW;

	// Load the scheduler task parameters
	S_Task.m_uiTask_ID = TASK_ID_SCHED;
	S_Task.m_ucPriority = TASK_PRIORITY_SCHED;
	S_Task.m_ucProcessorID = (uchar) (TASK_ID_SCHED >> 8);
	S_Task.m_ulTransducerID = (TASK_ID_SCHED & TASK_TRANSDUCER_ID_MASK);
	S_Task.m_uiFlags = TASK_FLAGS_SCHED;
	S_Task.m_ucState = TASK_INIT_STATE_SCHED;
	S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_SCHED;
	S_Task.m_ulParameters[0x00] = 0;
	S_Task.m_ulParameters[0x01] = 0;
	S_Task.m_ulParameters[0x02] = 0;
	S_Task.m_ulParameters[0x03] = 0;
	S_Task.m_cName = "SCHED";
	S_Task.ptrTaskHandler = vRTS_runScheduler;
	if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
		ucRetVal = TASKMNGR_OVRFLOW;

	// Load the reset task parameters
	S_Task.m_uiTask_ID = TASK_ID_RESET;
	S_Task.m_ucPriority = TASK_PRIORITY_RESET;
	S_Task.m_ucProcessorID = (uchar) (TASK_ID_RESET >> 8);
	S_Task.m_ulTransducerID = (TASK_ID_RESET & TASK_TRANSDUCER_ID_MASK);
	S_Task.m_uiFlags = TASK_FLAGS_RESET;
	S_Task.m_ucState = TASK_INIT_STATE_RESET;
	S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_RESET;
	S_Task.m_ulParameters[0x00] = 0;
	S_Task.m_ulParameters[0x01] = 0;
	S_Task.m_ulParameters[0x02] = 0;
	S_Task.m_ulParameters[0x03] = 0;
	S_Task.m_cName = "RESET";
	S_Task.ptrTaskHandler = vTask_Reset;
	if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
		ucRetVal = TASKMNGR_OVRFLOW;

	// Load the mcu temperature task parameters
	S_Task.m_uiTask_ID = TASK_ID_MCUTEMP;
	S_Task.m_ucPriority = TASK_PRIORITY_MCUTEMP;
	S_Task.m_ucProcessorID = (uchar) (TASK_ID_MCUTEMP >> 8);
	S_Task.m_ulTransducerID = (TASK_ID_MCUTEMP & TASK_TRANSDUCER_ID_MASK);
	S_Task.m_uiFlags = TASK_FLAGS_MCUTEMP;
	S_Task.m_ucState = TASK_INIT_STATE_MCUTEMP;
	S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_MCUTEMP;
	S_Task.m_ulParameters[0x00] = TASK_INTVL_MCUTEMP;
	S_Task.m_ulParameters[0x01] = TASK_STS_LFACT_MCUTEMP;
	S_Task.m_ulParameters[0x02] = 0;
	S_Task.m_ulParameters[0x03] = 0;
	S_Task.m_cName = TASK_NAME_MCUTEMP;
	S_Task.ptrTaskHandler = vTask_MCUTemp;
	if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
		ucRetVal = TASKMNGR_OVRFLOW;

	// Load the modify TCB task
	S_Task.m_uiTask_ID = TASK_ID_MODIFYTCB;
	S_Task.m_ucPriority = TASK_PRIORITY_MODIFYTCB;
	S_Task.m_ucProcessorID = CP_ID;
	S_Task.m_ulTransducerID = (TASK_PRIORITY_MODIFYTCB & TASK_TRANSDUCER_ID_MASK);
	S_Task.m_uiFlags = TASK_FLAGS_MODIFYTCB;
	S_Task.m_ucState = TASK_INIT_STATE_MODIFYTCB;
	S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_MODIFYTCB;
	S_Task.m_ulParameters[0x00] = 0;
	S_Task.m_ulParameters[0x01] = 0;
	S_Task.m_ulParameters[0x02] = 0;
	S_Task.m_ulParameters[0x03] = 0;
	S_Task.m_cName = TASK_NAME_MODIFYTCB;
	S_Task.ptrTaskHandler = vTask_ModifyTCB;
	if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
		ucRetVal = TASKMNGR_OVRFLOW;

	// If the WiSARD is a transmitter then load the request to join task
	if (ucL2FRAM_isSender()) {
		S_Task.m_uiTask_ID = TASK_ID_RTJ;
		S_Task.m_ucPriority = TASK_PRIORITY_RTJ;
		S_Task.m_ucProcessorID = (uchar) (TASK_ID_RTJ >> 8);
		S_Task.m_ulTransducerID = (TASK_ID_RTJ & TASK_TRANSDUCER_ID_MASK);
		S_Task.m_uiFlags = TASK_FLAGS_RTJ;
		S_Task.m_ucState = TASK_INIT_STATE_RTJ;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_RTJ;
		S_Task.m_ulParameters[0x00] = 0;
		S_Task.m_ulParameters[0x01] = 0;
		S_Task.m_ulParameters[0x02] = 0;
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = "RTJ  ";
		S_Task.ptrTaskHandler = vComm_Request_to_Join;

		// Save a copy of the task index
		ucJJ = g_ucNxtTskIdx;
		if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
			ucRetVal = TASKMNGR_OVRFLOW;

		// Need to make sure that this task starts active, regardless of the previous state in FRAM
		ucTask_SetField(ucJJ, TSK_STATE, (ulong) TASK_STATE_ACTIVE);
	}

	// If the WiSARD is a receiver then:
	if (ucL2FRAM_isReceiver()) {
		// Load the beacon task
		S_Task.m_uiTask_ID = TASK_ID_BCN;
		S_Task.m_ucPriority = TASK_PRIORITY_BCN;
		S_Task.m_ucProcessorID = (uchar) (TASK_ID_BCN >> 8);
		S_Task.m_ulTransducerID = (TASK_ID_BCN & TASK_TRANSDUCER_ID_MASK);
		S_Task.m_uiFlags = TASK_FLAGS_BCN;
		S_Task.m_ucState = TASK_INIT_STATE_BCN;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_BCN;
		S_Task.m_ulParameters[0x00] = 0;
		S_Task.m_ulParameters[0x01] = 0;
		S_Task.m_ulParameters[0x02] = 0;
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = "BCN  ";
		S_Task.ptrTaskHandler = vComm_SendBeacon;

		// Save a copy of the task index
		ucJJ = g_ucNxtTskIdx;
		if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
			ucRetVal = TASKMNGR_OVRFLOW;

		// Need to make sure that this task starts active, regardless of the previous state in FRAM
		ucTask_SetField(ucJJ, TSK_STATE, (ulong) TASK_STATE_ACTIVE);

		// Load RSSI task
		S_Task.m_uiTask_ID = TASK_ID_RSSI;
		S_Task.m_ucPriority = TASK_PRIORITY_RSSI;
		S_Task.m_ucProcessorID = (uchar) (TASK_ID_RSSI >> 8);
		S_Task.m_ulTransducerID = (TASK_ID_RSSI & TASK_TRANSDUCER_ID_MASK);
		S_Task.m_uiFlags = TASK_FLAGS_RSSI;
		S_Task.m_ucState = TASK_INIT_STATE_RSSI;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_RSSI;
		S_Task.m_ulParameters[0x00] = 0;
		S_Task.m_ulParameters[0x01] = 0;
		S_Task.m_ulParameters[0x02] = 0;
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = TASK_NAME_RSSI;
		S_Task.ptrTaskHandler = vTask_RSSI;
		if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
			ucRetVal = TASKMNGR_OVRFLOW;
	}

	// If the WiSARD samples then fetch the SP tasks
	if (ucMODOPT_readSingleRamOptionBit(OPTPAIR_WIZ_SAMPLES)) {
		ucTask_FetchAllSPTasks();

		// Load the check for SP boards task
		S_Task.m_uiTask_ID = TASK_ID_CHKSPBOARDS;
		S_Task.m_ucPriority = TASK_PRIORITY_CHKSPBOARDS;
		S_Task.m_ucProcessorID = (uchar) (TASK_ID_CHKSPBOARDS >> 8);
		S_Task.m_ulTransducerID = (TASK_ID_CHKSPBOARDS & TASK_TRANSDUCER_ID_MASK);
		S_Task.m_uiFlags = TASK_FLAGS_CHKSPBOARDS;
		S_Task.m_ucState = TASK_INIT_STATE_CHKSPBOARDS;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_CHKSPBOARDS;
		S_Task.m_ulParameters[0x00] = TASK_INTVL_CHKSPBOARDS;
		S_Task.m_ulParameters[0x01] = TASK_STS_LFACT_CHKSPBOARDS;
		S_Task.m_ulParameters[0x02] = 0;
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = TASK_NAME_CHKSPBOARDS;
		S_Task.ptrTaskHandler = vTask_SP_CheckBoards;
		if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
			ucRetVal = TASKMNGR_OVRFLOW;
	}

	// If the WISARD is a hub
	if (ucL2FRAM_isHub()) // If hub
	{
		// load the garden server comm. task
		S_Task.m_uiTask_ID = TASK_ID_GS;
		S_Task.m_uiFlags = TASK_FLAGS_GS;
		S_Task.m_ucPriority = TASK_PRIORITY_GS;
		S_Task.m_ucProcessorID = (uchar) (TASK_ID_GS >> 8);
		S_Task.m_ucState = TASK_INIT_STATE_GS;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_GS;
		S_Task.m_ulTransducerID = (uchar) TASK_ID_GS;
		S_Task.m_ulParameters[0x00] = TASK_INTVL_GS;
		S_Task.m_ulParameters[0x01] = 0;
		S_Task.m_ulParameters[0x02] = 0;
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = "GSV  ";
		S_Task.ptrTaskHandler = vGS_ReportToGardenServer;
		if(ucTask_CreateTask(S_Task) != TASKMNGR_OK) // create the task
			ucRetVal = TASKMNGR_OVRFLOW;
	}

	// Add the SCC tasks if it is present
	if (ucSCC_IsAttached() == TRUE) {
		S_Task.m_uiTask_ID = 0x0601;
		S_Task.m_uiFlags = TASK_FLAGS_SP;
		S_Task.m_ucPriority = TASK_PRIORITY_SP;
		S_Task.m_ucProcessorID = 0x06;
		S_Task.m_ucState = TASK_STATE_ACTIVE;
		S_Task.m_ucWhoCanRun = RBIT_ALL;
		S_Task.m_ulTransducerID = 1L; //Trans 1
		S_Task.m_ulParameters[0x00] = (((ulong) ((7) * 5) << 24) | SECONDS_10);
		S_Task.m_ulParameters[0x01] = 0;
		S_Task.m_ulParameters[PARAM_IDX_TSKDURATION] = ucSCC_GetSampleDuration();
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = "SCC  ";
		S_Task.ptrTaskHandler = vTask_SCC_StartSlot;
		ucTask_CreateTask(S_Task);

		S_Task.m_uiTask_ID = 0x0602;
		S_Task.m_uiFlags = (TASK_FLAGS_SP | F_SUSPEND);
		S_Task.m_ucPriority = (PRIORITY_5 | SCHED_CMD_SLOT);
		S_Task.m_ucProcessorID = 0x06; // Start with SP 1
		S_Task.m_ucState = TASK_STATE_IDLE;
		S_Task.m_ucWhoCanRun = RBIT_ALL;
		S_Task.m_ulTransducerID = 1L; //Trans 0 is test so add 1
		S_Task.m_ulParameters[0x00] = 0;
		S_Task.m_ulParameters[0x01] = 0;
		S_Task.m_ulParameters[0x02] = 0;
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = "SCC  ";
		S_Task.ptrTaskHandler = vTask_SCC_StartSlot;
		ucTask_CreateTask(S_Task);
	}

	// Compute the initial load factor of the system.  This is updated every time the
	// scheduler runs.
	vTask_ComputeSysLFactor();

	// Remove unneeded tasks from FRAM
	ucTSBCount = ucL2FRAM_getTSBTblCount();

	for (cII = ucTSBCount-1; cII >= 0; cII--) {
		uiTSB_TaskID = (uint) ulL2FRAM_getTSBEntryVal(cII, FRAM_ST_BLK_TASK_ID);

		// Reset the match found indicator
		ucFoundMatch = 0;

		// Loop through all the tasks in the
		for (ucJJ = 0; ucJJ < TASKPARTITION; ucJJ++) {
			if (ucTask_GetField(ucJJ, TSK_ID, &ulTCB_TaskID) == TASKMNGR_OK) {

				// If the task control block ID and the task state block ID match then break
				if (uiTSB_TaskID == (uint) ulTCB_TaskID) {
					ucFoundMatch = 1;
					break;
				}
			} //END: if (ucTask_GetField())
		} // END: for(ucJJ)

		// If no match is found then remove the task from FRAM
		if (ucFoundMatch == 0)
			vL2FRAM_deleteTSB((uchar)cII);
	} // END: for(ucII)


#if(0)
	vTaskDisplayTaskList();
#endif

	return ucRetVal;
}

/////////////////////////////////////////////////////////////////////////
//! \brief Searches the tasks list and finds an open structure
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
static void vUpdateTskIdx(void)
{
	uchar ucIndex;

	// Loop through entire list of tasks
	for (ucIndex = 0; ucIndex < TASKPARTITION; ucIndex++) {
		// Break out when the empty one is found
		if (p_saTaskList[ucIndex].m_uiTask_ID == INVALID_TASKID) {
			g_ucNxtTskIdx = ucIndex;
			break;
		}
	}

	// Loop through entire list of tasks
	for (ucIndex = TASKPARTITION; ucIndex < MAXNUMTASKS; ucIndex++) {
		// Break out when the empty one is found
		if (p_saTaskList[ucIndex].m_uiTask_ID == INVALID_TASKID) {
			g_ucNxtDynTskIdx = ucIndex;
			break;
		}
	}

} // End: vUpdateTskIdx()

//////////////////////////////////////////////////////////////////////////
//! \brief Appends a task to the static task list
//!
//!	This function also checks in FRAM to see if this task already exists
//! and if so, updates the newly generated task with FRAM parameters
//!
//! \param S_Task_Ctl
//! \return T_TaskStatus
/////////////////////////////////////////////////////////////////////////
T_TaskStatus ucTask_CreateTask(S_Task_Ctl S_Task)
{
	uchar ucTSBIndex;

	// Assume this is a new task
	ucTSBIndex = 0xFF;

	// Don't load data from FRAM if it is an OM task
	if ((S_Task.m_uiTask_ID != TASK_ID_SOM) & (S_Task.m_uiTask_ID != TASK_ID_ROM)) {

		// Search FRAM to see if this task is already stored there
		ucTSBIndex = ucL2FRAM_findTSB(S_Task.m_uiTask_ID);
		if (ucTSBIndex != 0xFF) {
			S_Task.m_uiFlags = (uint) ulL2FRAM_getTSBEntryVal(ucTSBIndex, FRAM_ST_BLK_FLAGS);
			S_Task.m_ucState = (uchar) ulL2FRAM_getTSBEntryVal(ucTSBIndex, FRAM_ST_BLK_TASK_STATE);
			S_Task.m_ulParameters[0] = ulL2FRAM_getTSBEntryVal(ucTSBIndex, FRAM_ST_BLK_PARAM1);
			S_Task.m_ulParameters[1] = ulL2FRAM_getTSBEntryVal(ucTSBIndex, FRAM_ST_BLK_PARAM2);
			S_Task.m_ulParameters[2] = ulL2FRAM_getTSBEntryVal(ucTSBIndex, FRAM_ST_BLK_PARAM3);
			S_Task.m_ulParameters[3] = ulL2FRAM_getTSBEntryVal(ucTSBIndex, FRAM_ST_BLK_PARAM4);

			vL2FRAM_putTSBEntryVal(ucTSBIndex, FRAM_ST_BLK_TASK_IDX, g_ucNxtTskIdx);
		}
	}

	// Copy the fields of the passed task into the task list
	p_saTaskList[g_ucNxtTskIdx].m_uiTask_ID = S_Task.m_uiTask_ID;
	p_saTaskList[g_ucNxtTskIdx].m_uiFlags = S_Task.m_uiFlags;
	p_saTaskList[g_ucNxtTskIdx].m_ucPriority = S_Task.m_ucPriority;
	p_saTaskList[g_ucNxtTskIdx].m_ucProcessorID = S_Task.m_ucProcessorID;
	p_saTaskList[g_ucNxtTskIdx].m_ulTransducerID = S_Task.m_ulTransducerID;
	p_saTaskList[g_ucNxtTskIdx].m_ucState = S_Task.m_ucState;
	p_saTaskList[g_ucNxtTskIdx].m_ucWhoCanRun = S_Task.m_ucWhoCanRun;
	p_saTaskList[g_ucNxtTskIdx].m_ulParameters[0] = S_Task.m_ulParameters[0];
	p_saTaskList[g_ucNxtTskIdx].m_ulParameters[1] = S_Task.m_ulParameters[1];
	p_saTaskList[g_ucNxtTskIdx].m_ulParameters[2] = S_Task.m_ulParameters[2];
	p_saTaskList[g_ucNxtTskIdx].m_ulParameters[3] = S_Task.m_ulParameters[3];
	p_saTaskList[g_ucNxtTskIdx].m_cName = S_Task.m_cName;
	p_saTaskList[g_ucNxtTskIdx].ptrTaskHandler = S_Task.ptrTaskHandler;

	if(ucTSBIndex == 0xFF)
	{
		if(cL2FRAM_addTSB(g_ucNxtTskIdx) < 0)
			vSERIAL_sout("Unable to add task to FRAM\r\n", 48);
	}

	// The task list is full
	if (g_ucNxtTskIdx == TASKPARTITION)
		return TASKMNGR_OVRFLOW;

#if 0
	vTaskDisplayTaskList();
#endif

	vUpdateTskIdx();

	return TASKMNGR_OK; // Success
}

//////////////////////////////////////////////////////////////////////////
//! \brief Appends a task to the dynamic task list
//!
//! Many of the fields are the same for both task 1 and task 2
//! Here we focus primarily on the transducers being combined
//!
//! \param S_Task_Ctl
//! \return T_TaskStatus
/////////////////////////////////////////////////////////////////////////
uchar ucTask_CreateCombinedTask(uchar TskIndex_1, uchar TskIndex_2)
{

	uchar ucTskIndex;
	ulong ulTransducerID;
	uchar ucProcID;

	ulTransducerID = ((p_saTaskList[TskIndex_1].m_ulTransducerID << 8) | p_saTaskList[TskIndex_2].m_ulTransducerID);
	ucProcID = p_saTaskList[TskIndex_1].m_ucProcessorID;

	// Check to see if this task already exists in the list
	for (ucTskIndex = TASKPARTITION; ucTskIndex < MAXNUMTASKS; ucTskIndex++) {
		if (p_saTaskList[ucTskIndex].m_ucProcessorID == ucProcID && p_saTaskList[ucTskIndex].m_ulTransducerID == ulTransducerID)
			return ucTskIndex;
	}

	// Copy the fields of the passed task into the task list
	p_saTaskList[g_ucNxtDynTskIdx].m_uiTask_ID = (p_saTaskList[TskIndex_1].m_uiTask_ID | p_saTaskList[TskIndex_2].m_uiTask_ID);
	p_saTaskList[g_ucNxtDynTskIdx].m_uiFlags = p_saTaskList[TskIndex_1].m_uiFlags;
	p_saTaskList[g_ucNxtDynTskIdx].m_ucPriority = p_saTaskList[TskIndex_1].m_ucPriority;
	p_saTaskList[g_ucNxtDynTskIdx].m_ucProcessorID = ucProcID;
	p_saTaskList[g_ucNxtDynTskIdx].m_ulTransducerID = ulTransducerID;
	p_saTaskList[g_ucNxtDynTskIdx].m_ucState = p_saTaskList[TskIndex_1].m_ucState;
	p_saTaskList[g_ucNxtDynTskIdx].m_ucWhoCanRun = p_saTaskList[TskIndex_1].m_ucWhoCanRun;
	p_saTaskList[g_ucNxtDynTskIdx].m_ulParameters[0] = p_saTaskList[TskIndex_1].m_ulParameters[0];
	p_saTaskList[g_ucNxtDynTskIdx].m_ulParameters[1] = p_saTaskList[TskIndex_1].m_ulParameters[1];
	p_saTaskList[g_ucNxtDynTskIdx].m_ulParameters[2] = p_saTaskList[TskIndex_1].m_ulParameters[2];
	p_saTaskList[g_ucNxtDynTskIdx].m_ulParameters[3] = p_saTaskList[TskIndex_1].m_ulParameters[3];
	p_saTaskList[g_ucNxtDynTskIdx].m_cName = cSPNames[(uchar) (p_saTaskList[g_ucNxtDynTskIdx].m_ucProcessorID - 1)];
	p_saTaskList[g_ucNxtDynTskIdx].ptrTaskHandler = p_saTaskList[TskIndex_1].ptrTaskHandler;

	// The task list is full
	if (g_ucNxtDynTskIdx == MAXNUMTASKS)
		return TASKMNGR_OVRFLOW;

#if 0
	vTaskDisplayTask(g_ucNxtDynTskIdx);
#endif

	// Set the local variable equal to the index of the task just added to the list
	ucTskIndex = g_ucNxtDynTskIdx;

	// Update the global variable
	vUpdateTskIdx();

	// Return the index of the task just scheduled
	return ucTskIndex;

}

//////////////////////////////////////////////////////////////////////////
//! \brief Removes a task from the task list
//!
//!
//!
//! \param S_Task_Ctl
//! \return T_TaskStatus
/////////////////////////////////////////////////////////////////////////
T_TaskStatus ucTask_DestroyTask(uchar ucTaskIndex)
{
	uchar ucTSBIndex;

	// IF this is an OM task then search for the TSB by serial number not task ID
	if ((p_saTaskList[ucTaskIndex].m_ucPriority & SCHED_FUNC_MASK) == SCHED_FUNC_OM_SLOT) {
		ucTSBIndex = ucL2FRAM_findTSB_SN(p_saTaskList[ucTaskIndex].m_ulParameters[PARAM_IDX_SN]);
	}
	else {
		ucTSBIndex = ucL2FRAM_findTSB(p_saTaskList[ucTaskIndex].m_uiTask_ID);
	}

	if (ucTSBIndex != 255){
		vL2FRAM_deleteTSB(ucTSBIndex);
	}
	// Delete the fields corresponding to that task
	p_saTaskList[ucTaskIndex].m_uiTask_ID = INVALID_TASKID;
	p_saTaskList[ucTaskIndex].m_uiFlags = 0;
	p_saTaskList[ucTaskIndex].m_ucPriority = 0;
	p_saTaskList[ucTaskIndex].m_ucProcessorID = 0;
	p_saTaskList[ucTaskIndex].m_ucState = 0;
	p_saTaskList[ucTaskIndex].m_ucWhoCanRun = 0;
	p_saTaskList[ucTaskIndex].m_ulTransducerID = 0;
	p_saTaskList[ucTaskIndex].m_ulParameters[0x00] = 0;
	p_saTaskList[ucTaskIndex].m_ulParameters[0x01] = 0;
	p_saTaskList[ucTaskIndex].m_ulParameters[0x02] = 0;
	p_saTaskList[ucTaskIndex].m_ulParameters[0x03] = 0;
	p_saTaskList[ucTaskIndex].m_cName = " --- ";
	p_saTaskList[ucTaskIndex].ptrTaskHandler = vTask_Dummy;

	return TASKMNGR_OK;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Determines the SP tasks available for scheduling based on the
//! 			 the attached boards and their transducer types.
//!
//!
//! \param none
//! \return T_TaskStatus
/////////////////////////////////////////////////////////////////////////
T_TaskStatus ucTask_FetchSingleSPTasks(uchar ucSPNumber)
{
	uchar ucReturnCode;
	uchar ucNumTrans;
	uchar ucTransCounter;
	uchar ucTransGrpCnt;
	S_Task_Ctl S_SPTask; //Local declaration of an SP task structure

	ucReturnCode = TASKMNGR_OK; // Assume all is well

	if (ucSP_IsAttached(ucSPNumber)) {
		// Fetch the number of transducers from the SP driver
		ucNumTrans = ucSP_FetchNumTransducers(ucSPNumber);

		// If there are 16 or more transducers on the SP board then build groups of the sensors
		if (ucNumTrans > 15) {
			ucTransGrpCnt = 0;

			// Loop through the transducers
			for (ucTransCounter = 0; ucTransCounter < ucNumTrans; ucTransCounter++) {
				switch (ucSP_FetchTransType(ucSPNumber, ucTransCounter))
				{
					// Actuator (ASCII A)
					case 0x41:
						S_SPTask.m_uiTask_ID = ((ucSPNumber + 1 << 8) | ucTransCounter + 1);
						S_SPTask.m_uiFlags = TASK_FLAGS_SP | F_SUSPEND;
						S_SPTask.m_ucPriority = (PRIORITY_5 | SCHED_CMD_SLOT);
						S_SPTask.m_ucProcessorID = ucSPNumber + 1; // Start with SP 1
						S_SPTask.m_ucState = TASK_STATE_IDLE;
						S_SPTask.m_ucWhoCanRun = TASK_SP_WHO_CAN_RUN;
						S_SPTask.m_ulTransducerID = (ulong) (ucTransCounter + 1); //Trans 0 is test so add 1
						S_SPTask.m_ulParameters[0x00] = 0;
						S_SPTask.m_ulParameters[0x01] = 0;
						S_SPTask.m_ulParameters[0x02] = 0;
						S_SPTask.m_ulParameters[0x03] = 0;
						S_SPTask.m_cName = cSPNames[ucSPNumber];
						S_SPTask.ptrTaskHandler = vTask_SP_StartSlot;

						ucTask_CreateTask(S_SPTask);

					break;

						// Sensor (ASCII S)
					case 0x53:
						S_SPTask.m_uiTask_ID = (((ucSPNumber + 1) << 8) | ucTransCounter + 1);
						S_SPTask.m_uiFlags = TASK_FLAGS_SP;
						S_SPTask.m_ucPriority = TASK_PRIORITY_SP;
						S_SPTask.m_ucProcessorID = ucSPNumber + 1;
						S_SPTask.m_ucState = TASK_STATE_ACTIVE;
						S_SPTask.m_ucWhoCanRun = TASK_SP_WHO_CAN_RUN;
						S_SPTask.m_ulTransducerID = (S_SPTask.m_ulTransducerID << 8) | (ulong) (ucTransCounter + 1); //Trans 0 is test so add 1
						S_SPTask.m_ulParameters[PARAM_IDX_INTERVAL] = (((ulong)((ucSPNumber + 1)*5)<<24) | TASK_SP_INTVL);
						S_SPTask.m_ulParameters[PARAM_IDX_LFACT] = TASK_STS_LFACT_SP;
						S_SPTask.m_ulParameters[PARAM_IDX_TSKDURATION] = 0;
						S_SPTask.m_ulParameters[0x03] = 0;
						S_SPTask.m_cName = cSPNames[ucSPNumber];
						S_SPTask.ptrTaskHandler = vTask_SP_StartSlot;

						ucTransGrpCnt++;

						if (ucTransGrpCnt == 4) {
							ucTask_CreateTask(S_SPTask);
							ucTransGrpCnt = 0;
						}

					break;

						// Invalid transducer type
					default:
						ucReturnCode = TASKMNGR_INVLD;
					break;
				}

			} //END: for(transducer number)
		}
		else {
			// Loop through the transducers
			for (ucTransCounter = 0; ucTransCounter < ucNumTrans; ucTransCounter++) {
				switch (ucSP_FetchTransType(ucSPNumber, ucTransCounter))
				{
					// Actuator (ASCII A)
					case 0x41:
						S_SPTask.m_uiTask_ID = (((ucSPNumber + 1) << 8) | (ucTransCounter + 1));
						S_SPTask.m_uiFlags = TASK_FLAGS_SP | F_SUSPEND;
						S_SPTask.m_ucPriority = (PRIORITY_7 | SCHED_CMD_SLOT);
						S_SPTask.m_ucProcessorID = ucSPNumber + 1; // Start with SP 1
						S_SPTask.m_ucState = TASK_STATE_IDLE;
						S_SPTask.m_ucWhoCanRun = TASK_SP_WHO_CAN_RUN;
						S_SPTask.m_ulTransducerID = (ulong) (ucTransCounter + 1); //Trans 0 is test so add 1
						S_SPTask.m_ulParameters[0x00] = 0;
						S_SPTask.m_ulParameters[0x01] = 0;
						S_SPTask.m_ulParameters[0x02] = 0;
						S_SPTask.m_ulParameters[0x03] = 0;
						S_SPTask.m_cName = cSPNames[ucSPNumber];
						S_SPTask.ptrTaskHandler = vTask_SP_StartSlot;

					break;

						// Sensor (ASCII S)
					case 0x53:
						S_SPTask.m_uiTask_ID = (((ucSPNumber + 1) << 8) | (ucTransCounter + 1));
						S_SPTask.m_uiFlags = TASK_FLAGS_SP;
						S_SPTask.m_ucPriority = TASK_PRIORITY_SP;
						S_SPTask.m_ucProcessorID = ucSPNumber + 1;
						S_SPTask.m_ucState = TASK_STATE_ACTIVE;
						S_SPTask.m_ucWhoCanRun = TASK_SP_WHO_CAN_RUN;
						S_SPTask.m_ulTransducerID = (ulong) (ucTransCounter + 1); //Trans 0 is test so add 1
						S_SPTask.m_ulParameters[PARAM_IDX_INTERVAL] = (((ulong)((ucSPNumber + 1)*5)<<24) | TASK_SP_INTVL);
						S_SPTask.m_ulParameters[PARAM_IDX_LFACT] = TASK_STS_LFACT_SP;
						S_SPTask.m_ulParameters[PARAM_IDX_TSKDURATION] = ucSP_FetchTransSmplDur(ucSPNumber, ucTransCounter);
						S_SPTask.m_ulParameters[0x03] = 0;
						S_SPTask.m_cName = cSPNames[ucSPNumber];
						S_SPTask.ptrTaskHandler = vTask_SP_StartSlot;

					break;

						// Invalid transducer type
					default:
						ucReturnCode = TASKMNGR_INVLD;
					break;
				}

				if (ucReturnCode == TASKMNGR_OK) {
					// Create the task
					ucTask_CreateTask(S_SPTask);
				}

			} //END: for(transducer number)
		} // End: if(numtrans)
	}
	return ucReturnCode;
} //End:ucTask_FetchSPTasks()

//////////////////////////////////////////////////////////////////////////
//! \brief Determines the SP tasks available for scheduling based on the
//! 			 the attached boards and their transducer types.
//!
//!
//! \param none
//! \return T_TaskStatus
/////////////////////////////////////////////////////////////////////////
T_TaskStatus ucTask_FetchAllSPTasks(void)
{
	uchar ucReturnCode;
	uchar ucSPCounter;

	ucReturnCode = TASKMNGR_OK; // Assume all is well

	// Loop through the SP boards
	for (ucSPCounter = 0; ucSPCounter < NUMBER_SPBOARDS; ucSPCounter++) {
		ucReturnCode = ucTask_FetchSingleSPTasks(ucSPCounter);

	} //End: for(SPNumber)

	return ucReturnCode;
} //End:ucTask_FetchSPTasks()


/////////////////////////////////////
////! \brief Consolidates SP tasks into a single task where possible
////!
////!
////!
//////////////////////////////////////
//void vTask_ConsolidateSPTasks(void){
//	uchar ucTaskIndex, SPTaskIndex;
//	uchar ucActiveSPs[];
//	uchar ucSPNumber, ucSPIndex;
//
//	for(ucSPIndex = SP1; ucSPIndex < NUMBER_SPBOARDS; ucSPIndex++){
//
//		// Initialize the SP task index to an invalid value
//		SPTaskIndex = 0xFF;
//
//		// If this SP is not attached then don't look for it in the task list
//		if(ucSP_IsAttached(ucSPIndex) == FALSE)
//			continue;
//
//		// Processor ID is SPIndex + 1
//		ucSPNumber = ucSPIndex + 1;
//
//		// Search for all tasks belonging to each attached SP
//		for (ucTaskIndex = 0; ucTaskIndex < TASKPARTITION; ucTaskIndex++) {
//			if (p_saTaskList[ucTaskIndex].m_ucProcessorID == ucSPNumber){
//
//				// If we have found the first SP task
//				if(SPTaskIndex == 0xFF){
//					SPTaskIndex = ucTaskIndex;
//					continue;
//				}
//
//				if(ucTask_CheckComp())
//					ucTaskIdxTemp = ucTask_CreateCombinedTask(g_ucNextSlotTaskTable[ucNST_tblNum][ucFoundSlot][ucFoundSubSlot], ucTaskIndex);
//			}
//	}
//}
//// If the parameters are the same then consolidate into a new task and set the old tasks to idle
//
//	// Create the new task
//
//
//}

//////////////////////////////////////////////////////////////////////////
//! \brief Removes all tasks pertaining to a processor
//!
//!
//!
//! \param ucSPNumber
//! \return T_TaskStatus
/////////////////////////////////////////////////////////////////////////
T_TaskStatus ucTask_RemoveProcessor(uchar ucProcID)
{
	uchar ucIndex;

	// Block access to CP tasks
	if (ucProcID != CP_ID) {
		// Search the task list to see if the link exists, if so return error
		for (ucIndex = 0; ucIndex < MAXNUMTASKS; ucIndex++) {
			if (p_saTaskList[ucIndex].m_ucProcessorID == ucProcID)
				ucTask_DestroyTask(ucIndex);
		}
	}

	return TASKMNGR_OK;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Clears the dynamic sector of the task list
//!
//!
//!
//! \param none
//! \return void
/////////////////////////////////////////////////////////////////////////
void vTask_ClearDynSector(void)
{
	uchar ucIndex;

	// Loop through entire list of dynamically created tasks tasks
	for (ucIndex = TASKPARTITION; ucIndex < MAXNUMTASKS; ucIndex++) {
		// Clear the task ID field
		p_saTaskList[ucIndex].m_uiTask_ID = INVALID_TASKID;
		p_saTaskList[ucIndex].m_ulTransducerID = 0;
		p_saTaskList[ucIndex].m_cName = " --- ";

	}

	// Set the dynamically created task pointer to the area after the partition in the task list
	g_ucNxtDynTskIdx = TASKPARTITION;

}


//////////////////////////////////////////////////////////////////////////
//! \brief Finds a unique task ID
//!
//!
//! \param uiTempID
//! \return uiTempID
/////////////////////////////////////////////////////////////////////////
uint uiTask_GetUniqueID(uint uiTempID)
{
	uchar ucIndex;

	// Hopefully we find one before 100 iterations of the loop
	ucIndex = 100;

	// Search through all tasks
	while(ucIndex) {
		ucIndex--;

		// Get the serial number for the task if allowed
		if(ucTask_FetchTaskIndex(uiTempID) == INVALID_TASKINDEX)
			return uiTempID;
		else
				uiTempID++;
			}

	return INVALID_TASKID;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Checks to see if the two tasks can be scheduled at the same
//! time
//!
//!
//! \param ucTaskID_1,ucTaskID_2
//! \return yes, no
/////////////////////////////////////////////////////////////////////////
uchar ucTask_CheckComp(uchar ucTaskID_1, uchar ucTaskID_2)
{
	// Make sure processor is the same
	if (p_saTaskList[ucTaskID_1].m_ucProcessorID != p_saTaskList[ucTaskID_2].m_ucProcessorID)
		return 0;

	// Make sure the Flags are the same
	if (p_saTaskList[ucTaskID_1].m_uiFlags != p_saTaskList[ucTaskID_2].m_uiFlags)
		return 0;

	// Make sure the handler is the same
	if (p_saTaskList[ucTaskID_1].ptrTaskHandler != p_saTaskList[ucTaskID_2].ptrTaskHandler)
		return 0;

	// If we get here then all fields match and they are compatible
	return 1;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Searches for dead tasks in the list.
//!
//! If there are tasks in the task list that are never going to execute
//! then we need to remove them to keep the task manager from taking up
//! more memory than is allocated.
//!
//! \param none
//!	\return none
//////////////////////////////////////////////////////////////////////////
void vTask_CleanTaskList(void)
{
	uchar ucTskIdx;
	ulong ulDispatchTime;

	// Loop through the task list
	for (ucTskIdx = 0; ucTskIdx < MAXNUMTASKS; ucTskIdx++) {

		// Remove the task if the dispatch time has passed.
		if (ucTask_GetField(ucTskIdx, PARAM_DISPATCHTIME, &ulDispatchTime) == TASKMNGR_OK) {

			if (ulDispatchTime < lTIME_getSysTimeAsLong())
				ucTask_DestroyTask(ucTskIdx);
		}

	}
	return;
}


////////////////////////////////////////////////////////////////////////
//! \brief Creates the operational message task for a node
//!
//!
//! \param uiSerialNum, ulRand, ucNodeRank
//! \return T_TaskStatus
/////////////////////////////////////////////////////////////////////////
T_TaskStatus ucTask_CreateOMTask(uint uiSerialNum, ulong ulRand, uchar ucNodeRank)
{
	S_Task_Ctl S_OMTask;

	// Check to see if this link already exists
	if (ucTask_SearchforLink(uiSerialNum) != INVALID_TASKINDEX)
		return TASKMNGR_INVLD;

	// Find an open link block index.  If non are available then exit
	if (ucLNKBLK_AddLink(uiSerialNum))
		return TASKMNGR_INVLD;

	// Add the rand to the link manager
	ucLNKBLK_WriteRand(uiSerialNum, ulRand);

	S_OMTask.m_ulParameters[PARAM_IDX_SN] = uiSerialNum;
	S_OMTask.m_ulParameters[1] = 0;
	S_OMTask.m_ulParameters[2] = 0;
	S_OMTask.m_ulParameters[3] = 0;
	S_OMTask.m_ucState = TASK_STATE_ACTIVE;

	if (ucNodeRank == PARENT) {
		// Load the local task structure with the task fields
		S_OMTask.m_uiTask_ID = TASK_ID_ROM;
		S_OMTask.m_uiFlags = TASK_FLAGS_ROM;
		S_OMTask.m_ucPriority = TASK_PRIORITY_ROM;
		S_OMTask.m_ucProcessorID = (uchar) (TASK_ID_ROM >> 8);
		S_OMTask.m_ucWhoCanRun = TASK_WHO_CAN_RUN_ROM;
		S_OMTask.m_ulTransducerID = (uchar) TASK_ID_ROM;
		S_OMTask.m_cName = "ROM";
		S_OMTask.ptrTaskHandler = vComm_Parent;

		// Add the task to the list
		ucTask_CreateTask(S_OMTask);

		ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiSerialNum, ((1 << 3) | 2), //next frame, 2 link per frame
		    lTIME_getSysTimeAsLong());

	}
	else if (ucNodeRank == CHILD) {

		S_OMTask.m_uiTask_ID = TASK_ID_SOM;
		S_OMTask.m_uiFlags = TASK_FLAGS_SOM;
		S_OMTask.m_ucPriority = TASK_PRIORITY_SOM;
		S_OMTask.m_ucProcessorID = (uchar) (TASK_ID_SOM >> 8);
		S_OMTask.m_ucWhoCanRun = TASK_WHO_CAN_RUN_SOM;
		S_OMTask.m_ulTransducerID = (uchar) TASK_ID_SOM;
		S_OMTask.m_cName = "SOM";
		S_OMTask.ptrTaskHandler = vComm_Child;

		// Add the task to the list
		ucTask_CreateTask(S_OMTask);

		// SETUP THE LNKBLK FOR THE NEXT OM2 COMMUNICATION (USING CLK2 TIME)
		ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uiSerialNum, ((1 << 3) | 2), //next frame, 2 link per frame
		    lTIME_getClk2AsLong());
	}

	return TASKMNGR_OK;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Checks for a link and returns the task index if the link exists
//!				else returns invalid index
//! \param uiSerialNum, *p_uiExistingSN
//!	\return Error code
//////////////////////////////////////////////////////////////////////////
uint8 ucTask_SearchforLink(uint uiSerialNum)
{
	signed char cIndex;
	ulong ulExistingSN;

	// Search through all tasks
	for (cIndex = MAXNUMTASKS; cIndex > 0; --cIndex) {

		// Get the serial number for the task if allowed
		if (ucTask_GetField(cIndex, PARAM_SN, &ulExistingSN) == TASKMNGR_OK) {

			// If there is a match then return the task index
			if (uiSerialNum == (uint) ulExistingSN)
				return (uchar) cIndex;
		}
	}

	// No links found
	return INVALID_TASKINDEX;
}

//////////////////////////////////////////////////////////////////////////
//! \brief returns the task index
//!
//! \param
//!	\return ucTskIdx
//////////////////////////////////////////////////////////////////////////
uchar ucTask_FetchTaskIndex(uint uiTaskID)
{
	uchar ucTskIdx;
	uchar ucCount;

	// Assume failure
	ucTskIdx = INVALID_TASKINDEX;

	// Loop through the task list
	for (ucCount = 0; ucCount < MAXNUMTASKS; ucCount++) {
		// If the task ID matches then break out
		if (p_saTaskList[ucCount].m_uiTask_ID == uiTaskID) {
			ucTskIdx = ucCount;
			break;
		}
	}

	return ucTskIdx;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Assigns fields of the task control block to the pointer ulRetPtr
//!
//! This function uses a pointer to pass values to the calling functions
//! because there is not a return value that is always guaranteed to
//! be an error code.  Parameters must be formatted (uchar, uchar, (ulong *)& )
//!
//! \param ucTskIndex, ucField, ulRetPtr
//!	\return ucErrCode
//////////////////////////////////////////////////////////////////////////
uchar ucTask_GetField(uchar ucTskIndex, uchar ucField, ulong * ulRetPtr)
{
	uchar ucErrCode;

	// Assume success
	ucErrCode = TASKMNGR_OK;

	// Error handling
	// Make sure task index is in range
	if (ucTskIndex >= MAXNUMTASKS)
		ucErrCode = 1;

	// Make sure the TCB contains a valid task
	if (p_saTaskList[ucTskIndex].m_uiTask_ID == INVALID_TASKID)
		ucErrCode = 1;

	// If there are no fundamental errors with the request then continue
	if (ucErrCode == 0) {
		switch (ucField)
	{
		case TSK_ID:
			*ulRetPtr = (ulong) p_saTaskList[ucTskIndex].m_uiTask_ID;
		break;

			case TSK_PRIORITY:
				*ulRetPtr = (ulong) p_saTaskList[ucTskIndex].m_ucPriority;
			break;

			case TSK_PROCESSORID:
				*ulRetPtr = (ulong) p_saTaskList[ucTskIndex].m_ucProcessorID;
			break;

			case TSK_TRANSDUCERID:
				*ulRetPtr = p_saTaskList[ucTskIndex].m_ulTransducerID;
			break;

			case TSK_FLAGS:
				*ulRetPtr = (ulong) p_saTaskList[ucTskIndex].m_uiFlags;
			break;

			case TSK_STATE:
				*ulRetPtr = (ulong) p_saTaskList[ucTskIndex].m_ucState;
			break;

			case TSK_WHOCANRUN:
				*ulRetPtr = (ulong) p_saTaskList[ucTskIndex].m_ucWhoCanRun;
			break;

			case PARAM_SN:
				// If this is not an OM task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_FUNC_OM_SLOT)
					ucErrCode = 1;
				else
					*ulRetPtr = p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_SN];
			break;

			case PARAM_DISPATCHTIME:
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_DNCNT_SLOT)
					ucErrCode = 1;
				else
					*ulRetPtr = p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_DISPATCHTIME];
				break;

			case PARAM_INTERVAL:
				// If this is not an interval task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_FUNC_INTERVAL_SLOT)
					ucErrCode = 1;
				else if (p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL] == 0)
					ucErrCode = 1;
				else
					*ulRetPtr = (0x00FFFFFF & p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL]);
				break;

			case PARAM_PHASE:
				// If this is not an interval task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_FUNC_INTERVAL_SLOT)
					ucErrCode = 1;
				else if (p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL] == 0)
					ucErrCode = 1;
				else
					*ulRetPtr = (p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL] >> 24);
				break;

			case PARAM_LFACT:
				// If this is not an interval task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_FUNC_INTERVAL_SLOT)
					ucErrCode = 1;
				else
					*ulRetPtr = p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_LFACT];
				break;

			case PARAM_TSKDURATION:
				*ulRetPtr = p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_TSKDURATION];
				break;

			case TSK_CMDLENGTH:
				*ulRetPtr = (ulong) p_saTaskList[ucTskIndex].m_ucCmdLength;
				break;
		}
	}

	return ucErrCode;

}

//////////////////////////////////////////////////////////////////////////
//! \brief Sets feilds of a task control block
//!
//! \param ucTskIndex
//!	\return ucErrCode
//////////////////////////////////////////////////////////////////////////
uchar ucTask_SetField(uchar ucTskIndex, uint8 ucField, ulong ulValue)
{

	uchar ucErrCode, ucTSBNum;
	ulong ulTemp;

	// Assume success
	ucErrCode = 0;

	// Error handling
	// Make sure task index is in range
	if (ucTskIndex >= MAXNUMTASKS)
		ucErrCode = 1;

	// Make sure the TCB contains a valid task
	if (p_saTaskList[ucTskIndex].m_uiTask_ID == INVALID_TASKID)
		ucErrCode = 1;

	// If there are no fundamental errors with the request then continue
	if (ucErrCode == 0) {
		switch (ucField)
		{
			case TSK_ID:
				p_saTaskList[ucTskIndex].m_uiTask_ID = (uint) ulValue;
			break;

			case TSK_PRIORITY:
				p_saTaskList[ucTskIndex].m_ucPriority = (uchar) ulValue;
			break;

			case TSK_PROCESSORID:
				p_saTaskList[ucTskIndex].m_ucProcessorID = (uchar) ulValue;
			break;

			case TSK_TRANSDUCERID:
				p_saTaskList[ucTskIndex].m_ulTransducerID = (uchar) ulValue;
			break;

			case TSK_FLAGS:
				p_saTaskList[ucTskIndex].m_uiFlags = (uint) ulValue;

				// Update the FRAM TSB
				ucTSBNum = ucL2FRAM_findTSB(p_saTaskList[ucTskIndex].m_uiTask_ID);
				if (ucTSBNum != 255)
					vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_FLAGS, ulValue);
			break;

			case TSK_STATE:
				p_saTaskList[ucTskIndex].m_ucState = (uchar) ulValue;

				// Update the FRAM TSB
				ucTSBNum = ucL2FRAM_findTSB(p_saTaskList[ucTskIndex].m_uiTask_ID);
				if (ucTSBNum != 255)
					vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_TASK_STATE, ulValue);
			break;

			case TSK_WHOCANRUN:
				p_saTaskList[ucTskIndex].m_ucWhoCanRun = (uchar) ulValue;
			break;

			case PARAM_SN:
				// If this is not an OM task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_FUNC_OM_SLOT)
					ucErrCode = 1;
				else {
					p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_SN] = ulValue;

					// Update the FRAM TSB
					ucTSBNum = ucL2FRAM_findTSB(p_saTaskList[ucTskIndex].m_uiTask_ID);
					if (ucTSBNum != 255)
						vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM1, ulValue);
				}
			break;

			case PARAM_DISPATCHTIME:
				// If this is not a count down task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_DNCNT_SLOT)
					ucErrCode = 1;
				else{
					p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_DISPATCHTIME] = ulValue;

					// Update the FRAM TSB
					ucTSBNum = ucL2FRAM_findTSB(p_saTaskList[ucTskIndex].m_uiTask_ID);
					if (ucTSBNum != 255)
						vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM1, ulValue);
				}
			break;

			case PARAM_INTERVAL:
				// If this is not an interval task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_FUNC_INTERVAL_SLOT)
					ucErrCode = 1;
				else {
					// Update the RAM TCB
					p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL] &= 0xFF000000;
					p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL] |= (0x00FFFFFF & ulValue);

					// Also update the load factor in RAM
					p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_LFACT] = 3600/ulValue;

					// Update the FRAM TSB
					ucTSBNum = ucL2FRAM_findTSB(p_saTaskList[ucTskIndex].m_uiTask_ID);
					if (ucTSBNum != 255){
						vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM1, p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL]);

						// Also update the load factor in FRAM
						vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM2, 3600/ulValue);
					}
				}
			break;

			case PARAM_PHASE:
				// If this is not an interval task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_FUNC_INTERVAL_SLOT)
					ucErrCode = 1;
				else {

					// Temporary compy of phase/interval field for computation
					ulTemp = p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL];
					ulTemp |= (uchar)(ulValue << 24);

					// Update the RAM TCB
					p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_INTERVAL] = ulTemp;

					// Update the FRAM TSB
					ucTSBNum = ucL2FRAM_findTSB(p_saTaskList[ucTskIndex].m_uiTask_ID);
					if (ucTSBNum != 255){
						vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM1, ulTemp);
					}
				}
				break;

			case PARAM_LFACT:
				// If this is not an interval task then the request is invalid
				if ((p_saTaskList[ucTskIndex].m_ucPriority & SCHED_FUNC_MASK) != SCHED_FUNC_INTERVAL_SLOT)
					ucErrCode = 1;
				else {
					p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_LFACT] = ulValue;

					// Update the FRAM TSB
					ucTSBNum = ucL2FRAM_findTSB(p_saTaskList[ucTskIndex].m_uiTask_ID);
					if (ucTSBNum != 255)
						vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM2, ulValue);
				}
			break;

			case PARAM_TSKDURATION:
				p_saTaskList[ucTskIndex].m_ulParameters[PARAM_IDX_TSKDURATION] = ulValue;

				// Update the FRAM TSB
				ucTSBNum = ucL2FRAM_findTSB(p_saTaskList[ucTskIndex].m_uiTask_ID);
				if (ucTSBNum != 255)
					vL2FRAM_putTSBEntryVal(ucTSBNum, FRAM_ST_BLK_PARAM3, ulValue);
			break;

			case TSK_CMDLENGTH:
				p_saTaskList[ucTskIndex].m_ucCmdLength = (uchar) ulValue;
			break;

				// The field doesn't exist
			default:
				ucErrCode = 1;
			break;
		}
	}

	return ucErrCode;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Gets the parameters of a task control block
//!
//! This function does not provide the error handling that ucTask_GetField
//! does.  Therefore, it is recommended that this function is only used
//! by the FRAM task state block routines
//!
//! \param ucTskIndex
//!	\return ucErrCode
//////////////////////////////////////////////////////////////////////////
uchar ucTask_GetParam(uchar ucTskIndex, uchar ucIndex, ulong *ulRetPtr)
{
	uchar ucErrCode;

	// Assume success
	ucErrCode = 0;

	// Error handling
	// Make sure task index is in range
	if (ucTskIndex >= MAXNUMTASKS)
		ucErrCode = 1;

	// Make sure the TCB contains a valid task
	if (p_saTaskList[ucTskIndex].m_uiTask_ID == INVALID_TASKID)
		ucErrCode = 1;

	if (ucErrCode == 0) {
		// Assign the desired parameter to the pointer
		switch (ucIndex)
		{
			case 0:
				*ulRetPtr = p_saTaskList[ucTskIndex].m_ulParameters[0];
			break;

			case 1:
				*ulRetPtr = p_saTaskList[ucTskIndex].m_ulParameters[1];
			break;

			case 2:
				*ulRetPtr = p_saTaskList[ucTskIndex].m_ulParameters[2];
			break;

			case 3:
				*ulRetPtr = p_saTaskList[ucTskIndex].m_ulParameters[3];
			break;

			default:
				ucErrCode = 1;
			break;
		}
	}
	return ucErrCode;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Sets the parameters of a task control block
//!
//! \param ucTskIndex
//!	\return ucErrCode
//////////////////////////////////////////////////////////////////////////
uchar ucTask_SetCmdParam(uchar ucTskIndex, uchar * p_ucPtr)
{
	uchar ucErrCode;
	uchar ucCount;

	// Assume success
	ucErrCode = 0;

	// Error handling
	// Make sure task index is in range
	if (ucTskIndex >= MAXNUMTASKS)
		ucErrCode = 1;

	// Make sure the TCB contains a valid task
	if (p_saTaskList[ucTskIndex].m_uiTask_ID == INVALID_TASKID)
		ucErrCode = 1;

	if (ucErrCode == 0) {
		// loop through the command parameters and load them into the passed pointer
		for (ucCount = 0; ucCount < MAXCMDPARAM; ucCount++) {
			p_saTaskList[ucTskIndex].m_ucCmdParam[ucCount] = *p_ucPtr++;
		}
	}

	return ucErrCode;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Sets the parameters of a task control block
//!
//! \param ucTskIndex
//!	\return ucErrCode
//////////////////////////////////////////////////////////////////////////
uchar ucTask_GetCmdParam(uchar ucTskIndex, uchar * p_ucPtr)
{
	uchar ucErrCode;
	uchar ucCount;

	// Assume success
	ucErrCode = 0;

	// Error handling
	// Make sure task index is in range
	if (ucTskIndex >= MAXNUMTASKS)
		ucErrCode = 1;

	// Make sure the TCB contains a valid task
	if (p_saTaskList[ucTskIndex].m_uiTask_ID == INVALID_TASKID)
		ucErrCode = 1;

	// loop through the command parameters and load them into the passed pointer
	for (ucCount = 0; ucCount < MAXCMDPARAM; ucCount++) {
		*p_ucPtr++ = p_saTaskList[ucTskIndex].m_ucCmdParam[ucCount];
	}

	return ucErrCode;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Fetches the next open location in the task list
//!
//! \param none
//! \return ucNxtTskIdx
/////////////////////////////////////////////////////////////////////////
uchar ucTask_FetchNxtTaskIndex(void)
{
	return g_ucNxtTskIdx;
}

//////////////////////////////////////////////////////////////////////////
//! \brief Find how many radio links we have
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
uint uiTask_countSOM2andROM2entrys(void)
{
	uchar ucIndex;
	uchar ucActiveSOMslots;
	uchar ucActiveROMslots;
	uint uiCountTotals;

	/* Count all active operational message entries we have*/
	ucActiveSOMslots = 0;
	ucActiveROMslots = 0;
	for (ucIndex = (TASKPARTITION - 1); ucIndex > 0; ucIndex--) {
		if (p_saTaskList[ucIndex].m_uiTask_ID == TASK_ID_ROM)
			ucActiveROMslots++;
		if (p_saTaskList[ucIndex].m_uiTask_ID == TASK_ID_SOM)
			ucActiveSOMslots++;
	}/* END: for() */

	uiCountTotals = ucActiveSOMslots;
	uiCountTotals <<= 8;
	uiCountTotals |= ucActiveROMslots;

	return (uiCountTotals);
}

//////////////////////////////////////////////////////////////////////////
//! \brief Delete all child nodes
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_OrphanChildren(void)
{
	uchar ucIndex, ucMsgIndex;
	ulong ulSN;
	uint uiSN;

	for (ucIndex = (TASKPARTITION - 1); ucIndex > 0; ucIndex--) {
		if (p_saTaskList[ucIndex].m_uiTask_ID == TASK_ID_ROM){

			ucTask_GetField(ucIndex, PARAM_SN, &ulSN);
			uiSN=(uint)ulSN;

			// Remove the dropped node from the link block table
			ucLNKBLK_RemoveNode(uiSN);

			// Clean the route list
			ucRoute_Init(uiL2FRAM_getSnumLo16AsUint());

			ucTask_DestroyTask(ucIndex);

			// Build the report data element header
			vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
			ucMsgIndex = DE_IDX_RPT_PAYLOAD;

			ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_LINK_BROKEN;
			ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
			ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiSN >> 8);
			ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiSN;

			// Store DE
			vReport_LogDataElement(RPT_PRTY_LINK_BROKEN);
		}
	}/* END: for() */

}

//////////////////////////////////////////////////////////////////////////
//! \brief Displays all tasks created
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTaskDisplayTaskList(void)
{
	uchar ucIndex;

	vSERIAL_sout("\r\n\r\n----  RAM TCB TBL ----\r\n", 28);
	vSERIAL_sout("## NAME   ID   FLAGS  STATE    PARAM 1     PARAM 2     PARAM 3     PARAM 4\r\n", 76);
	for (ucIndex = 0; ucIndex < MAXNUMTASKS; ucIndex++) {
		if(p_saTaskList[ucIndex].m_uiTask_ID != INVALID_TASKID){

			vSERIAL_UI8_2char_out(ucIndex, ' ');
			vSERIAL_colTab(3);
			vTask_showTaskName(ucIndex);
			vSERIAL_colTab(10);
			vSERIAL_HB16out(p_saTaskList[ucIndex].m_uiTask_ID);
			vSERIAL_sout(" ", 1);
			vSERIAL_HB16out(p_saTaskList[ucIndex].m_uiFlags);
			vSERIAL_sout("    ", 4);
			vSERIAL_HB8out(p_saTaskList[ucIndex].m_ucState);
			vSERIAL_sout("   ", 3);
			vSERIAL_UI32out(p_saTaskList[ucIndex].m_ulParameters[0]);
			vSERIAL_sout("  ", 2);
			vSERIAL_UI32out(p_saTaskList[ucIndex].m_ulParameters[1]);
			vSERIAL_sout("  ", 2);
			vSERIAL_UI32out(p_saTaskList[ucIndex].m_ulParameters[2]);
			vSERIAL_sout("  ", 2);
			vSERIAL_UI32out(p_saTaskList[ucIndex].m_ulParameters[3]);
			vSERIAL_crlf();


		}//END: if(p_saTaskList)
	}//END: for(cIndex)
}

//////////////////////////////////////////////////////////////////////////
//! \brief Displays a single task
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTaskDisplayTask(uchar ucIndex)
{

	vSERIAL_crlf();

	vSERIAL_sout("Task ID:        ", 16);
	vSERIAL_HB16out(p_saTaskList[ucIndex].m_uiTask_ID);
	vSERIAL_crlf();

	vSERIAL_sout("Processor ID:   ", 16);
	vSERIAL_HB8out(p_saTaskList[ucIndex].m_ucProcessorID);
	vSERIAL_crlf();

	vSERIAL_sout("Transducer ID:  ", 16);
	vSERIAL_HB8out(p_saTaskList[ucIndex].m_ulTransducerID);
	vSERIAL_crlf();

}

//////////////////////////////////////////////////////////////////////////
//!
//! \brief Determines the amount of messages generated by the WiSARD depending
//! on the number of tasks that generate messages and how often they are executed.
//!
//! This updates the global load factor variable and is used to determine how
//! many links per slot should occur between the parent and child node so that the
//! child doesn't get backed up with messages. Computed once per frame in runScheduler().
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////
void vTask_ComputeSysLFactor(void)
{
	uchar ucTaskIndex;
	uchar ucNumLinks;
	int iNewSysLFactor;
	uint uiTaskID;
	ulong ulSerialNumber;
	int iMsgsPerHour;
	ulong ulTaskID;
	uint uiMsgsInQueue;

	/* INIT THE NEW VALUE */
	iNewSysLFactor = 0;

	// Loop through all possible static tasks
	for (ucTaskIndex = (MAXNUMTASKS - 1); ucTaskIndex > 0; ucTaskIndex--) {

		// Get the task ID to find operational messages if allowed
		if (ucTask_GetField(ucTaskIndex, TSK_ID, &ulTaskID) != TASKMNGR_OK)
			continue;
		uiTaskID = (uint) ulTaskID;

		/*---------- COMPUTE THE SYSTEM LFACTOR -----------------------------*/

		if (uiTaskID == TASK_ID_ROM) {
			// Calculate the number of messages received from children
			// This is based on the number of links per frame * the number of msgs per link * number of frames per hour

			// Get the serial number
			ucTask_GetField(ucTaskIndex, PARAM_SN, &ulSerialNumber);

			// Fetch the link count, if the node is not in the link block then continue
			if (ucLNKBLK_FetchNumofLinkTimes((uint) ulSerialNumber, &ucNumLinks) != 0)
				continue;

			iMsgsPerHour = ucNumLinks * MAX_OM_MSGS * 60;

			iNewSysLFactor += iMsgsPerHour;
	}

}/* END: for(ucTaskIndex) */

	/* ADD IN THE NUMBER OF MSGS ALREADY AQUIRED AS AN INSTANT RATE */
	uiMsgsInQueue = (int) uiL2SRAM_getMsgCount();
	iNewSysLFactor += ulGLOB_msgSysLFactor + uiMsgsInQueue;

	/* STUFF THE COMPLETE SYS LFACTOR */
	iGLOB_completeSysLFactor = iNewSysLFactor;

#if 1
	vSERIAL_sout("NewLd=", 6);
	vSERIAL_IV16out(iNewSysLFactor);
	vSERIAL_crlf();
#endif

	return;

}/* END: vTask_computeSysLFactor() */

///////////////////////////////////////////////////////////////////////////////
//! \fn vTask_showStatsOnLFactor()
//!	\brief Computes then prints load factor information to the terminal
//!
//!	\param none
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vTask_showStatsOnLFactor(void)
{
	uchar ucTaskIndex;
	uchar ucNumLinks;
	int iNewSysLFactor;
	int iMsgsPerHour;
	uint uiTaskFlgs;
	uint uiTaskID;
	uint uiMsgCount;
	ulong ulSerialNumber;
	ulong ulLFactVal;
	ulong ulTaskID, ulTaskFlgs;

	/* SHOW TABLE HEADER */
	vSERIAL_crlf();
	vSERIAL_dash(13);
	vSERIAL_sout("  LOAD FACTOR STATS  ", 21);
	vSERIAL_dash(11);
	vSERIAL_crlf();
	vSERIAL_sout("Tbl Act Act Flg  LFact Formula     Msg   Tot\r\n", 46);
	vSERIAL_sout("Ent Num Nam Val    Val             Cnt   Val\r\n", 46);

	/* LOOP VERTICAL BY TBL INDEX -- COMPUTING NEW LFACTOR VALUE */
	iNewSysLFactor = 0;

	// Loop through all possible static tasks
	for (ucTaskIndex = (TASKPARTITION - 1); ucTaskIndex > 0; ucTaskIndex--) {

		if (ucTask_GetField(ucTaskIndex, TSK_ID, &ulTaskID) == TASKMNGR_OK) {

			// Get the flags to make sure is generates messages (therfore contributing to the load)
			ucTask_GetField(ucTaskIndex, TSK_FLAGS, &ulTaskFlgs);
			uiTaskFlgs = (uint) ulTaskFlgs;

			/*---------- COMPUTE THE SYSTEM LFACTOR -----------------------------*/
			if (uiTaskID == TASK_ID_ROM) {
				// Calculate the number of messages received from children

				// Get the serial number
				ucTask_GetField(ucTaskIndex, PARAM_SN, &ulSerialNumber);

				// Fetch the link count, if the node is not in the link block then continue
				if(ucLNKBLK_FetchNumofLinkTimes((uint)ulSerialNumber, &ucNumLinks) != 0)
					continue;

				// This is based on the number of links per frame * the number of msgs per link * number of frames per hour
				iMsgsPerHour = ucNumLinks * MAX_OM_MSGS * 60;
			}
			else {
				if ((uiTaskFlgs & F_USE_MAKE_OM2) != 0) {
					// Get the load factor of the task
					if (ucTask_GetField(ucTaskIndex, PARAM_LFACT, &ulLFactVal))
						iMsgsPerHour = (int) ulLFactVal;
				}
				else {
					iMsgsPerHour = 0;
				}
			}/* END: if(ucTaskID) */

			iNewSysLFactor += iMsgsPerHour;

			/*----------  FINISH REST OF TABLE --------------*/

			vSERIAL_colTab(32);
			vSERIAL_I16out(iMsgsPerHour);
			vSERIAL_I16out(iNewSysLFactor);
			vSERIAL_crlf();

		} // End: if(!= InvalidTask)
	}/* END: for(ucTaskIndex) */

	uiMsgCount = uiL2SRAM_getMsgCount();
	iMsgsPerHour = (int) uiMsgCount;
	iNewSysLFactor += iMsgsPerHour;

	vSERIAL_sout("Msgcnt=", 7);
	vSERIAL_colTab(17);
	vSERIAL_UI16out(uiMsgCount);
	vSERIAL_colTab(32);
	vSERIAL_I16out(iMsgsPerHour);
	vSERIAL_colTab(38);
	vSERIAL_I16out(iNewSysLFactor);
	vSERIAL_crlf();

	vSERIAL_colTab(40);
	vSERIAL_sout("ÄÄÄÄ\r\n", 6);

	vSERIAL_sout("Load=", 5);
	vSERIAL_colTab(16);
	vSERIAL_I16out(iGLOB_completeSysLFactor);
	vSERIAL_colTab(38);
	vSERIAL_I16out(iNewSysLFactor);
	vSERIAL_crlf();

	return;

}/* END: vTask_showStatsOnLFactor() */

/////////////////////////////////////////////////////////////////////////
//! \brief Displays the name of the task
//!
//! \param ucTskIndex: The index of the task
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_showTaskName(uchar ucTskIndex)
{
	uint uiTaskID;
	uint uiSerialNumber;
	ulong ulTaskID, ulSerialNumber;

	// If it is a radio task then print out the serial number of the other node
	// otherwise print out the task name
	if (ucTask_GetField(ucTskIndex, TSK_ID, &ulTaskID) == TASKMNGR_OK) {
		uiTaskID = (uint) ulTaskID;
		if (uiTaskID == TASK_ID_SOM || uiTaskID == TASK_ID_ROM) {
			if (ucTask_GetField(ucTskIndex, PARAM_SN, &ulSerialNumber) == TASKMNGR_OK)
				uiSerialNumber = (uint) ulSerialNumber;
			vSERIAL_UI16out(uiSerialNumber);
		}
		else {
			vSERIAL_sout(p_saTaskList[ucTskIndex].m_cName, 5);
		}
	}
	else
		vSERIAL_sout(" --- ", 5);

} // END: vTask_showTaskName()

/////////////////////////////////////////////////////////////////////////
//! \brief Displays all the tasks in the list
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vTask_ShowAllTasks(void)
{
	uchar ucTskIndex;

	for (ucTskIndex = 0; ucTskIndex < TASKPARTITION; ucTskIndex++) {
		vTask_showTaskName(ucTskIndex);
		vSERIAL_crlf();
	}

} // END: vTask_showTaskName()




////////////////////////////
//!
//! unit test area for the task manager
//!
//!
////////////////////////////
void vTaskTest(void){
	uint uiII;

	S_Task_Ctl S_Task;

	//Halt the dog while the system initializes
	WDTCTL = WDTPW + WDTHOLD;

	uiII = 50;
	while(uiII-- != 0){
		// Load the sleep task parameters
		S_Task.m_uiTask_ID = uiII;
		S_Task.m_ucPriority = TASK_PRIORITY_SLEEP;
		S_Task.m_ucProcessorID = (uchar) (TASK_ID_SLEEP >> 8);
		S_Task.m_ulTransducerID = (TASK_ID_SLEEP & TASK_TRANSDUCER_ID_MASK);
		S_Task.m_uiFlags = TASK_FLAGS_SLEEP;
		S_Task.m_ucState = TASK_INIT_STATE_SLEEP;
		S_Task.m_ucWhoCanRun = TASK_WHO_CAN_RUN_SLEEP;
		S_Task.m_ulParameters[0x00] = 0x00;
		S_Task.m_ulParameters[0x01] = 0;
		S_Task.m_ulParameters[0x02] = 0;
		S_Task.m_ulParameters[0x03] = 0;
		S_Task.m_cName = " --- ";
		S_Task.ptrTaskHandler = vTask_Sleep;
		ucTask_CreateTask(S_Task); // create the task
	}
}

