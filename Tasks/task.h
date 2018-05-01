/*
 * task.h
 *
 *  Created on: Aug 12, 2013
 *      Author: cp397
 */


#ifndef TASK_H_
#define TASK_H_

#include "rts.h"

//! \def CP_BOARD
//! \brief Indicates that the command/report/code is going to or coming from the CP board
#define CP_ID	0x00

//! \def SP1_BOARD
//! \brief Indicates that the command/report/code is going to or coming from the SP board in location 1
#define SP1_BOARD	0x01

//! \def SP2_BOARD
//! \brief Indicates that the command/report/code is going to or coming from the SP board in location 2
#define SP2_BOARD	0x02

//! \def SP3_BOARD
//! \brief Indicates that the command/report/code is going to or coming from the SP board in location 3
#define SP3_BOARD	0x03

//! \def SP4_BOARD
//! \brief Indicates that the command/report/code is going to or coming from the SP board in location 4
#define SP4_BOARD	0x04

//! \def RADIO_BOARD
//! \brief Indicates that the command/report/code is going to or coming from the radio
#define RADIO_BOARD	0x05

//! \def MAXNUMTASKS
//! \brief The maximum number of tasks allowed at one time
#define MAXNUMTASKS		70

//! \def MAXNUMTASKS
//! \brief The point in the task list where the dynamically created tasks start
#define TASKPARTITION		40

//! \def MAXTSKMGRPARAM
//! \brief The maximum number of task manager parameters allowed for a tasks
#define MAXTSKMGRPARAM			0x04

//! \def MAXCMDPARAM
//! \brief The maximum number of parameters stored for a command
#define MAXCMDPARAM	0x08

//! \def MAXNUM_TASKS_PERSLOT
//! \brief Maximum number of tasks per slot
#define MAXNUM_TASKS_PERSLOT		5

//! \def TASK_TRANSDUCER_ID_MASK
//! \brief used to pick off the bits that correspond to the transducer ID
//! 		   within the Sensor ID
#define TASK_TRANSDUCER_ID_MASK	0x00FF

//! \def INVALID_TASKID
//! \brief The invalid task ID indicating that the task DNE
#define INVALID_TASKID		0xFFFF

//! \def INVALID_TASKINDEX
//! \brief The invalid task index indicating that the task DNE
#define INVALID_TASKINDEX		0xFF

//! \def LABEL_LEN
//! \def Length of the CP task label
#define LABEL_LEN			0x10

//! \defgroup Task states
//! @{
//! \def TASK_STATE_IDLE
#define TASK_STATE_IDLE	0x00
//! \def TASK_STATE_ACTIVE
#define TASK_STATE_ACTIVE	0x01
//! @}

//! \defgroup Task Manager Error Codes
//! @{
//! \typedef T_TaskStatus
//! \brief The status of the task manager
typedef unsigned char T_TaskStatus;
//! \def TASKMNGR_OK
//! \brief Return code indicating no errors
#define TASKMNGR_OK				0x00
//! \def TASKMNGR_OVRFLOW
//! \brief Return code indicating tasks exceed the maximum number of tasks
#define TASKMNGR_OVRFLOW	0x01
//! \def TASKMNGR_INVLD
//! \brief Return code indicating attempted access to non-existent field
#define TASKMNGR_INVLD		0x02
//! @}

//! \defgroup Task Intervals
//! @{
//! def SECONDS_5
#define	SECONDS_5		5
//! def SECONDS_10
#define	SECONDS_10	10
//! \def SECONDS_30
#define	SECONDS_30	30
//! \def MINUTES_1
#define MINUTES_1		60
//! \def MINUTES_5
#define MINUTES_3		180

//! \def MINUTES_5
#define MINUTES_5		300
//! \def MINUTES_10
#define MINUTES_10	600
//! \def MINUTES_15
#define MINUTES_15	900
//! \def MINUTES_30
#define MINUTES_30	1800
//! \def MINUTES_90
#define MINUTES_90	5400
//! \def FRAME_1
#define FRAMES_1			256
//! @}

//! MAX_DE_LEN
//! \brief The maximum length of a data element.
//! This value is used to control storage lengths in SRAM
#define MAX_DE_LEN		0x30

//! \defgroup Task Flags
//! @{
#define F_USE_EXACT_SLOT		0x01	//0000 0000 0000 0001

#define F_USE_NONEXACT_SLOT	0x00	//0000 0000 0000 0000

#define F_USE_THIS_RAND		0x02	//0000 0000 0000 0010

#define F_USE_ANY_RAND		0x00	//0000 0000 0000 0000

//! \def F_USE_MIDDLE_OF_SLOT
//! \brief used to determine where in the slot a function should be executed
#define F_USE_MIDDLE_OF_SLOT  0X0004	//0000 0000 0000 0100
//! \def F_USE_START_OF_SLOT
//! \brief used to determine where in the slot a function should be executed
#define F_USE_START_OF_SLOT   0X0008    //0000 0000 0000 1000
//! \def F_REQUIRES_FOLLOW_UP
//! \brief used to determine if a task requires clean up at the end of the slot
#define F_REQUIRES_FOLLOW_UP	0X0010	//0000 0000 0001 0000

#define F_USE_MAKE_OM2				0x0020	//0000 0000 0010 0000
#define F_USE_NO_MAKE_OM2			0x0000	//0000 0000 0000 0000

//! \def F_USE_END_OF_SLOT
//! \brief used to determine where in the slot a function should be executed
#define F_USE_END_OF_SLOT   	0X0040    //0000 0000 0100 0000
//! \def F_USE_FULL_SLOT
//! \brief used to determine where in the slot a function should be executed
#define F_USE_FULL_SLOT				0X0080	//0000 0000 1000 0000

//! \def F_SUICIDE
//! \brief Tells the task manager to destroy the task after execution
#define F_SUICIDE							0x0100	//0000 0001 0000 0000

//! \def F_SUSPEND
//! \brief Tells the task manager to suspend the task after execution
//! So the task still exists in the task list but will not be scheduled
#define F_SUSPEND							0x0200	//0000 0010 0000 0000
//! @}

//! \def CHANGE_PERMISSION_YES
#define CHANGE_PERMISSION_YES	0x01    //0000 0001
//! \def CHANGE_PERMISSION_NO
#define CHANGE_PERMISSION_NO	0x00	//0000 0000

//! \defgroup Who can run bits
//! @{
#define RBIT_OFF		0x01	//0000 0001		//0
#define RBIT_SENDONLY	0x02	//0000 0010		//1
#define RBIT_HUB		0x04	//0000 0100		//2
#define RBIT_RELAY		0x08	//0000 1000		//3
#define RBIT_STANDALONE 0x10	//0001 0000		//4
#define RBIT_TERMINUS	0x20	//0010 0000		//5
#define RBIT_SAMPLEHUB  0x40	//0100 0000		//6
#define RBIT_SPOKE		0x80	//1000 0000		//7

#define RBIT_ALL		0xFF	//1111 1111
#define RBIT_SAMPLERS	(RBIT_STANDALONE| RBIT_TERMINUS| RBIT_SAMPLEHUB| RBIT_SPOKE)
#define RBIT_SENDERS	(RBIT_SENDONLY| RBIT_RELAY| RBIT_TERMINUS| RBIT_SPOKE)
#define RBIT_RECEIVERS	(RBIT_HUB| RBIT_RELAY| RBIT_SAMPLEHUB| RBIT_SPOKE)
//! @}

//! \struct S_Task_Ctl
//! \brief The structure of a task control
//! Each task can require several different values so the task control structure
//! is generalized for all tasks by using the parameters field
typedef struct
{
		unsigned int 	m_uiTask_ID; //!< The ID of the task
		unsigned char m_ucPriority; //!< Priority of the task
		unsigned char m_ucProcessorID; //!< Processor executing the task
		unsigned long m_ulTransducerID; //!< Transducer number of the task
		unsigned int 	m_uiFlags; //!< Flags
		unsigned char m_ucState; //!< State of the task
		unsigned char m_ucWhoCanRun; //!< Determines if the task can run given the role
		unsigned long m_ulParameters[MAXTSKMGRPARAM]; 	//! Parameters required by the task
		unsigned char m_ucCmdLength;
		unsigned char m_ucCmdParam[MAXCMDPARAM]; 	//! Parameters required by the task
		char *m_cName;
		void (*ptrTaskHandler)(); //!< The function that executes the task
}S_Task_Ctl;

//! \defgroup TaskFields
//! \brief These definitions are used to access parameters of a task control block
//! @{
//! \def TSK_ID
//! \brief Task ID
#define TSK_ID							1
//! \def TSK_PRIORITY
//! \brief Priority
#define TSK_PRIORITY				2
//! \def TSK_PROCESSORID
//! \brief Processor number performing the task
#define TSK_PROCESSORID			3
//! \def TSK_TRANSDUCERID
//! \brief Sensor ID
#define TSK_TRANSDUCERID		4
//! \def TSK_FLAGS
//! \brief Task flags
#define TSK_FLAGS						5
//! \def TSK_STATE
//! \brief Task state
#define TSK_STATE						6
//! \def TSK_WHOCANRUN
//! \brief Defines what role can execute the task
#define TSK_WHOCANRUN				7
//! \def PARAM_SN
//! \brief Serial number of other node
#define PARAM_SN						8
//! \def PARAM_DISPATCHTIME
//! \brief Time that a task should be executed
#define PARAM_DISPATCHTIME	9
//! \def PARAM_INTERVAL
//! \brief Sampling interval
#define PARAM_INTERVAL			10
//! \def PARAM_PHASE
//! \brief Sampling phase
#define PARAM_PHASE					11
//! \def PARAM_LFACT
//! \brief Load factor
#define PARAM_LFACT					12
//! \def PARAM_TSKDURATION
//! \brief Sample duration required by a task
#define PARAM_TSKDURATION		13
//! \def TSK_CMDLENGTH
//! \brief Length of the command parameters
#define TSK_CMDLENGTH				14
//! @}


//! \defgroup CP board task definitions
//! @{
//! The priority byte is used for both the priority and the scheduler function index

/**************************   Sleep Task  ****************************/
//! \def TASK_ID_SLEEP
#define TASK_ID_SLEEP			0x0001
//! \def TASK_PRIORITY_SLEEP
#define TASK_PRIORITY_SLEEP				(PRIORITY_7 | SCHED_FUNC_DORMANT_SLOT)
//! \def TASK_STS_LFACT_SLEEP
#define TASK_STS_LFACT_SLEEP			0
//! \def TASK_FLAGS_SLEEP
#define TASK_FLAGS_SLEEP  				(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 | F_USE_FULL_SLOT)
//! \def TASK_WHO_CAN_RUN_SLEEP
#define TASK_WHO_CAN_RUN_SLEEP		RBIT_ALL
//! \def TASK_INIT_STATE_SLEEP
#define TASK_INIT_STATE_SLEEP		TASK_STATE_ACTIVE
//! \def TASK_NAME_SLEEP
#define TASK_NAME_SLEEP							"SLEEP"


/**************************   Input voltage   ****************************/
//! \def TASK_ID_V_IN
#define TASK_ID_V_IN				0x0002
//! \def TASK_PRIORITY_V_IN
#define TASK_PRIORITY_V_IN				(PRIORITY_6 | SCHED_FUNC_INTERVAL_SLOT)
//! \def TASK_INTERVAL_V_IN
#define TASK_INTERVAL_V_IN			 	MINUTES_10
//! \def TASK_STS_LFACT_V_IN
#define TASK_STS_LFACT_V_IN				(3600/TASK_INTERVAL_V_IN)
//! \def TASK_FLAGS_V_IN
#define TASK_FLAGS_V_IN  					(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT )
//! \def TASK_WHO_CAN_RUN_V_IN
#define TASK_WHO_CAN_RUN_V_IN			RBIT_ALL
//! \def TASK_INIT_STATE_V_IN
#define TASK_INIT_STATE_V_IN			TASK_STATE_ACTIVE

/************************   Run Scheduler   ****************************/
//! \def TASK_ID_SCHED
#define TASK_ID_SCHED			0x0003
//! \def TASK_PRIORITY_SCHED
#define TASK_PRIORITY_SCHED				(PRIORITY_0 | SCHED_FUNC_SCHEDULER_SLOT)
//! \def TASK_STS_LFACT_SCHED
#define TASK_STS_LFACT_SCHED			0
//! \def TASK_FLAGS_SCHED
#define TASK_FLAGS_SCHED 					(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2| F_USE_FULL_SLOT )
//! \def TASK_WHO_CAN_RUN_SCHED
#define TASK_WHO_CAN_RUN_SCHED 		RBIT_ALL
//! \def TASK_INIT_STATE_SCHED
#define TASK_INIT_STATE_SCHED		TASK_STATE_ACTIVE

/*******************   Send Operational Messages   *********************/
//! \def TASK_ID_SOM
#define TASK_ID_SOM				0x0004
//! \def TASK_PRIORITY_SOM
#define TASK_PRIORITY_SOM					(PRIORITY_2 | SCHED_FUNC_OM_SLOT)
//! \def TASK_STS_LFACT_SOM
#define TASK_STS_LFACT_SOM				0
//! \def TASK_FLAGS_SOM
#define TASK_FLAGS_SOM 						(F_USE_EXACT_SLOT | F_USE_THIS_RAND | F_USE_NO_MAKE_OM2 | F_USE_MIDDLE_OF_SLOT)
//! \def TASK_WHO_CAN_RUN_SOM
#define TASK_WHO_CAN_RUN_SOM 			RBIT_SENDERS
//! \def TASK_INIT_STATE_SOM
#define TASK_INIT_STATE_SOM				TASK_STATE_IDLE

/*******************  Receive Operational Messages   *********************/
//! \def TASK_ID_ROM
#define TASK_ID_ROM								0x0005
//! \def TASK_PRIORITY_ROM
#define TASK_PRIORITY_ROM				(PRIORITY_2 | SCHED_FUNC_OM_SLOT)
//! \def TASK_STS_LFACT_ROM
#define TASK_STS_LFACT_ROM			0
//! \def TASK_FLAGS_ROM
#define TASK_FLAGS_ROM 					(F_USE_EXACT_SLOT | F_USE_THIS_RAND | F_USE_NO_MAKE_OM2 | F_USE_MIDDLE_OF_SLOT)
//! \def TASK_WHO_CAN_RUN_ROM
#define TASK_WHO_CAN_RUN_ROM 		RBIT_RECEIVERS
//! \def TASK_INIT_STATE_ROM
#define TASK_INIT_STATE_ROM			TASK_STATE_IDLE

/************************  Move SRAM  to Flash  ************************/
//! \def TASK_ID_FRAM_TO_SDCARD
#define TASK_ID_FRAM_TO_SDCARD						0x0006
//! \def TASK_PRIORITY_FRAM_TO_SDCARD
#define TASK_PRIORITY_FRAM_TO_SDCARD			(PRIORITY_6 | SCHED_CMD_SLOT)
//! \def TASK_FLAGS_FRAM_TO_SDCARD
#define TASK_FLAGS_FRAM_TO_SDCARD 				(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 | F_USE_FULL_SLOT)
//! \def TASK_WHO_CAN_RUN_FRAM_TO_SDCARD
#define TASK_WHO_CAN_RUN_FRAM_TO_SDCARD		RBIT_ALL
//! \def TASK_INIT_STATE_FRAM_TO_SDCARD
#define TASK_INIT_STATE_FRAM_TO_SDCARD		TASK_STATE_IDLE

/******************   Send beacon messages   *******************/
//! \def TASK_ID_BCN
#define TASK_ID_BCN									0x0007
//! \def TASK_PRIORITY_BCN
#define TASK_PRIORITY_BCN						(PRIORITY_3 | SCHED_FUNC_SDC4_SLOT)
//! \def TASK_STS_LFACT_BCN
#define TASK_STS_LFACT_BCN					0
//! \def TASK_FLAGS_BCN
#define TASK_FLAGS_BCN 							(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 |  F_USE_FULL_SLOT)
//! \def TASK_WHO_CAN_RUN_BCN
#define TASK_WHO_CAN_RUN_BCN 				RBIT_RECEIVERS
//! \def TASK_INIT_STATE_BCN
#define TASK_INIT_STATE_BCN					TASK_STATE_ACTIVE

/******************   Receive request to join messages  *******************/
//! \def TASK_ID_RTJ
#define TASK_ID_RTJ									0x0008
//! \def TASK_PRIORITY_RTJ
#define TASK_PRIORITY_RTJ						(PRIORITY_1 | SCHED_FUNC_RDC4_SLOT)
//! \def TASK_STS_LFACT_RTJ
#define TASK_STS_LFACT_RTJ					0
//! \def TASK_FLAGS_RTJ
#define TASK_FLAGS_RTJ 							(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 |  F_USE_FULL_SLOT)
//! \def TASK_WHO_CAN_RUN_RTJ
#define TASK_WHO_CAN_RUN_RTJ 				RBIT_RECEIVERS
//! \def TASK_INIT_STATE_RTJ
#define TASK_INIT_STATE_RTJ					TASK_STATE_ACTIVE

/******************** Report to garden server ***************************/
//! \def TASK_ID_GS
#define TASK_ID_GS									0x0009
//! \def TASK_INTVL_GS
#define TASK_INTVL_GS 		 					SECONDS_5
//! \def TASK_PRIORITY_GS
#define TASK_PRIORITY_GS						(PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT)
//! \def TASK_STS_LFACT_GS
#define TASK_STS_LFACT_GS						0
//! \def TASK_FLAGS_GS
#define TASK_FLAGS_GS 							(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2| F_USE_START_OF_SLOT )
//! \def TASK_WHO_CAN_RUN_GS
#define TASK_WHO_CAN_RUN_GS 	  		(RBIT_HUB | RBIT_SAMPLEHUB)
//! \def TASK_INIT_STATE_GS
#define TASK_INIT_STATE_GS					TASK_STATE_ACTIVE

/******************** Start RSSI ***************************/
//! \def TASK_ID_RSSI
#define TASK_ID_RSSI								0x000A
//! \def TASK_PRIORITY_RSSI
#define TASK_PRIORITY_RSSI					(PRIORITY_5 | SCHED_FUNC_DORMANT_SLOT)
//! \def TASK_FLAGS_RSSI
#define TASK_FLAGS_RSSI 						(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2| F_USE_START_OF_SLOT )
//! \def TASK_WHO_CAN_RUN_RSSI
#define TASK_WHO_CAN_RUN_RSSI 	  	(RBIT_SENDERS | RBIT_RECEIVERS)
//! \def TASK_INIT_STATE_RSSI
#define TASK_INIT_STATE_RSSI				TASK_STATE_IDLE
//! \def TASK_NAME_RSSI
#define TASK_NAME_RSSI							"RSSI "

/**************************   Reset Task  ****************************/
//! \def TASK_ID_RESET
#define TASK_ID_RESET								0x000B
//! \def TASK_PRIORITY_RESET
#define TASK_PRIORITY_RESET					(PRIORITY_7 | SCHED_CMD_SLOT)
//! \def TASK_STS_LFACT_RESET
#define TASK_STS_LFACT_RESET				0
//! \def TASK_FLAGS_RESET
#define TASK_FLAGS_RESET  					(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 | F_USE_START_OF_SLOT)
//! \def TASK_WHO_CAN_RUN_RESET
#define TASK_WHO_CAN_RUN_RESET			RBIT_ALL
//! \def TASK_INIT_STATE_RESET
#define TASK_INIT_STATE_RESET				TASK_STATE_IDLE
//! \def TASK_NAME_RESET
#define TASK_NAME_RESET							"RESET"

/******************   Internal Temperature Sensing Task  *******************/
//! \def TASK_ID_MCUTEMP
#define TASK_ID_MCUTEMP							0x000D
//! \def TASK_PRIORITY_MCUTEMP
#define TASK_PRIORITY_MCUTEMP				(PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT)
//! \def TASK_FLAGS_MCUTEMP
#define TASK_FLAGS_MCUTEMP  				(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT)
//! \def TASK_WHO_CAN_RUN_MCUTEMP
#define TASK_WHO_CAN_RUN_MCUTEMP		RBIT_ALL
//! \def TASK_INIT_STATE_MCUTEMP
#define TASK_INIT_STATE_MCUTEMP			TASK_STATE_ACTIVE
//! \def TASK_INTVL_MCUTEMP
#define TASK_INTVL_MCUTEMP 				  MINUTES_10
//! \def TASK_STS_LFACT_MCUTEMP
#define TASK_STS_LFACT_MCUTEMP			(3600/TASK_INTVL_MCUTEMP)
//! \def TASK_NAME_MCUTEMP
#define TASK_NAME_MCUTEMP						"TMP  "

/******************   Check attached SP boards Task  *******************/
//! \def TASK_ID_CHKSPBOARDS
#define TASK_ID_CHKSPBOARDS							0x000E
//! \def TASK_PRIORITY_CHKSPBOARDS
#define TASK_PRIORITY_CHKSPBOARDS				(PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT)
//! \def TASK_FLAGS_CHKSPBOARDS
#define TASK_FLAGS_CHKSPBOARDS  				(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT)
//! \def TASK_WHO_CAN_RUN_CHKSPBOARDS
#define TASK_WHO_CAN_RUN_CHKSPBOARDS		RBIT_ALL
//! \def TASK_INIT_STATE_CHKSPBOARDS
#define TASK_INIT_STATE_CHKSPBOARDS			TASK_STATE_ACTIVE
//! \def TASK_INTVL_CHKSPBOARDS
#define TASK_INTVL_CHKSPBOARDS 				  MINUTES_30
//! \def TASK_STS_LFACT_CHKSPBOARDS
#define TASK_STS_LFACT_CHKSPBOARDS			0 // doesn't generate messages
//! \def TASK_NAME_CHKSPBOARDS
#define TASK_NAME_CHKSPBOARDS						"CSP  "

/******************   Modify TCB Fields Task  *******************/
//! \def TASK_ID_MODIFYTCB
#define TASK_ID_MODIFYTCB								0x000F //Do not change!!
//! \def TASK_PRIORITY_MODIFYTCB
#define TASK_PRIORITY_MODIFYTCB					(PRIORITY_5 | SCHED_CMD_SLOT)
//! \def TASK_FLAGS_MODIFYTCB
#define TASK_FLAGS_MODIFYTCB 						(F_SUSPEND | F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 | F_USE_START_OF_SLOT)
//! \def TASK_WHO_CAN_RUN_MODIFYTCB
#define TASK_WHO_CAN_RUN_MODIFYTCB 			RBIT_ALL
//! \def TASK_INIT_STATE_MODIFYTCB
#define TASK_INIT_STATE_MODIFYTCB 			TASK_STATE_IDLE
//! \def TASK_NAME_MODIFYTCB
#define TASK_NAME_MODIFYTCB 						"MdTCB"

/******************   Report HID Task  *******************/
//! \def TASK_ID_RPT_HID
#define TASK_ID_RPT_HID								0x0010
//! \def TASK_PRIORITY_RPT_HID
#define TASK_PRIORITY_RPT_HID					(PRIORITY_5 | SCHED_CMD_SLOT)
//! \def TASK_FLAGS_RPT_HID
#define TASK_FLAGS_RPT_HID 						(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT| F_SUICIDE)
//! \def TASK_WHO_CAN_RUN_RPT_HID
#define TASK_WHO_CAN_RUN_RPT_HID 			RBIT_ALL
//! \def TASK_NAME_RPT_HID
#define TASK_NAME_RPT_HID 						"RptID"

/************************  Run radio diagnostic  ************************/
//! \def TASK_ID_RADIO_DIAG
#define TASK_ID_RADIO_DIAG						0x0011
//! \def TASK_PRIORITY_RADIO_DIAG
#define TASK_PRIORITY_RADIO_DIAG			(PRIORITY_6 | SCHED_CMD_SLOT)
//! \def TASK_FLAGS_RADIO_DIAG
#define TASK_FLAGS_RADIO_DIAG 				(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT)
//! \def TASK_WHO_CAN_RUN_RADIO_DIAG
#define TASK_WHO_CAN_RUN_RADIO_DIAG		(RBIT_RECEIVERS | RBIT_SENDERS)
//! \def TASK_INIT_STATE_RADIO_DIAG
#define TASK_INIT_STATE_RADIO_DIAG		TASK_STATE_ACTIVE
//! @} End: CP board task definitions

/*************************** Generic SP task definitions ******************************/
//! \def TASK_FLAGS_SP
#define TASK_FLAGS_SP						(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_MAKE_OM2 | F_USE_START_OF_SLOT | F_REQUIRES_FOLLOW_UP)
//! \def TASK_SP_INTVL
#define TASK_SP_INTVL 				  MINUTES_5
//! \def TASK_PRIORITY_SP
#define TASK_PRIORITY_SP 				(PRIORITY_5 | SCHED_FUNC_INTERVAL_SLOT)
//! \def TASK_STS_LFACT_SP
#define TASK_STS_LFACT_SP				(3600/TASK_SP_INTVL)
//! \def TASK_SP_WHO_CAN_RUN
#define TASK_SP_WHO_CAN_RUN			RBIT_SAMPLERS

//! \def TASK_ID_RQSTSPDATA
#define TASK_ID_RQSTSPDATA								0x0010
//! \def TASK_PRIORITY_RQSTSPDATA
#define TASK_PRIORITY_RQSTSPDATA					(PRIORITY_7 | SCHED_DNCNT_SLOT)
//! \def TASK_FLAGS_RQSTSPDATA
#define TASK_FLAGS_RQSTSPDATA  						(F_USE_NONEXACT_SLOT | F_USE_ANY_RAND | F_USE_NO_MAKE_OM2 | F_USE_END_OF_SLOT | F_SUICIDE)
//! \def TASK_WHO_CAN_RUN_RQSTSPDATA
#define TASK_WHO_CAN_RUN_RQSTSPDATA				RBIT_SAMPLERS
//! \def TASK_INIT_STATE_RQSTSPDATA
#define TASK_INIT_STATE_RQSTSPDATA				TASK_STATE_IDLE
//! \def TASK_NAME_RQSTSPDATA
#define TASK_NAME_RQSTSPDATA							"SP RQ"

//! \defgroup Task manager function definitions
//! @{
T_TaskStatus ucTask_Init(void);
void vTask_UpdateFromFRAM(void);
T_TaskStatus ucTask_CreateTask(S_Task_Ctl S_Task);
uchar ucTask_CreateCombinedTask(uchar TskIndex_1, uchar TskIndex_2);
void vTask_ClearDynSector(void);
T_TaskStatus ucTask_DestroyTask(unsigned char ucTaskIndex);
T_TaskStatus ucTask_RemoveProcessor(uchar ucProcID);
T_TaskStatus ucTask_Dispatcher(unsigned char ucProcessorID);
uchar ucTask_VerifyChangeableCommand(uchar ucTaskIndex);


// This block of functions are used to quickly get and set fields in the task module
// While having a large amount of function seems chaotic it takes less time than one
// or two functions with switch statements
T_TaskStatus ucTask_FetchSingleSPTasks(uchar ucSPNumber);
T_TaskStatus ucTask_FetchAllSPTasks(void);
uchar ucTask_FetchNxtTasksID(void);

ulong ulTask_FetchLnkIdx(uchar ucTskIndex);
uint8 ucTask_SearchforLink(uint uiSerialNum);
uchar ucTask_FetchTaskIndex(uint uiTaskID);
uchar ucTask_GetField(uchar ucTskIndex, uchar ucField, ulong * ulRetPtr);
uchar ucTask_SetField(uchar ucTskIndex, uint8 ucField, ulong ulValue);
uchar ucTask_GetCmdParam(uchar ucTskIndex, uchar * p_ucPtr);
uchar ucTask_SetCmdParam(uchar ucTskIndex, uchar * p_ucPtr);
uchar ucTask_GetParam(uchar ucTskIndex, uchar ucIndex, ulong *ulRetPtr);
uint uiTask_GetUniqueID(uint uiTempID);
uchar ucTask_CheckComp(uchar ucTaskID_1,uchar ucTaskID_2);
void vTask_CleanTaskList(void);
T_TaskStatus ucTask_CreateOMTask(uint uiSerialNum, ulong ulRand, uchar ucNodeRank);
uint uiTask_countSOM2andROM2entrys(void);
void vTask_ComputeSysLFactor(void);
void vTask_showStatsOnLFactor(void);
void vTaskDisplayTaskList(void);
//void vTaskDisplayTask(uchar ucIndex);
void vTask_showTaskName(uchar ucTaskIdx);
void vTask_ShowAllTasks(void);
void vTask_Dispatch(uchar ucNSTtblNum, uchar ucNSTslotNum);
//! @}

//! \defgroup Task function definitions
//! @{
void vTask_Dummy(void);
void vTask_FRAM_to_SDCard(void);
void vTask_SP_CheckBoards(void);
void vTask_SP_StartSlot(void);
void vTask_SP_EndSlot(void);
void vTask_Batt_Sense(void);
void vTask_showBattReading(void);
void vTask_Reset(void);
void vTask_RSSI(void);
void vTask_MCUTemp(void);
void vTask_Sleep(void);
void vTask_ModifyTCB(void);
void vTask_OrphanChildren(void);
void vTask_SCC_StartSlot(void);
void vTask_SCC_EndSlot(void);
void vTask_ReportHID(void);
void vTask_RuntimeRadioDiag(void);
//! @}
#endif /* TASK_H_ */
