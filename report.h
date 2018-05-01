
/***************************  REPORT.H  ****************************************
*
* Header for REPORT routine pkg
*
*
* V1.00 01/28/2005 wzr
*	started
*
******************************************************************************/

#ifndef REPORT_H_INCLUDED
 #define REPORT_H_INCLUDED

/* COPY TO SRAM FLAGS */
#define YES_COPY_TO_SRAM 1
#define  NO_COPY_TO_SRAM 0

//! \def QUEUE_SIZE
//! \brief The size in bytes of the message queue
#define	QUEUE_SIZE		0x200

//! \def MAX_NUM_MSGS
//! \brief The maximum number of messages in the queue
#define	MAX_NUM_MSGS 	QUEUE_SIZE/MAX_MSG_SIZE

//! \struct S_Queue
//! \brief Structure holds all information about a queue
struct S_Queue
{
		uchar m_ucaQueue[QUEUE_SIZE];		//!< The queue
		uint	m_uiQueueHead;						//!< The starting location in the queue
		uint 	m_uiQueueTail;						//!< The ending location in the queue
		uint	m_uiQueueCount;						//!< The number of messages in the queue
};

//! \defgroup Report source identifications
//! @{
//! \def SRC_ID_RPT_HID
//! \brief The hardware IDs of all boards in the WiSARD
#define SRC_ID_RPT_HID					0
//! \def SRC_ID_INPUT_VOLTAGE_LOW
//! \brief Report ID indicates that the source voltage is too low
#define SRC_ID_INPUT_VOLTAGE_LOW		1
//! \def SRC_ID_INPUT_VOLTAGE
//! \brief Reading from the 5V input voltage line
#define SRC_ID_INPUT_VOLTAGE			2
//! \def SRC_ID_INTERNAL_TEMP
//! \brief Reading from the ADC of the MCU's temperature
#define SRC_ID_INTERNAL_TEMP			3
////! \def SRC_ID_COUNT
////! \brief delete this
//#define SRC_ID_COUNT							4
//! \def SRC_ID_MSG_COUNT
//! \brief Number of messages in the queue
#define SRC_ID_MSG_COUNT					5
//! \def SRC_ID_RDIO_DIAG
//! \brief Radio diagnostic results
#define SRC_ID_RDIO_DIAG					6

//! \def SRC_ID_SDCARD_INIT_FAIL
//! \brief Initializing the SD Card was unsuccessful
#define SRC_ID_SDCARD_INIT_FAIL				20

//! \def SRC_ID_SDCARD_BLOCKLEN_FAIL
//! \brief Setting the SD Card block length was unsuccessful
#define SRC_ID_SDCARD_BLOCKLEN_FAIL		21

//! \def SRC_ID_SDCARD_WRITE_FAIL
//! \brief A write to the SD Card was unsuccessful
#define SRC_ID_SDCARD_WRITE_FAIL			22

//! \def SRC_ID_SDCARD_READ_FAIL
//! \brief A read from the SD Card was unsuccessful
#define SRC_ID_SDCARD_READ_FAIL				23

//! \def SRC_ID_SYSTEM_RESTART
//! \brief Indicates that the system has restarted
#define SRC_ID_SYSTEM_RESTART					30

//! \def SRC_ID_SET_TIME
//! \brief Indicates that the system clock was set
#define SRC_ID_SET_TIME								31

//! \def SRC_ID_SUBSECOND_ADJUST
//! \brief Indicates the adjustments to the sub second compare register
#define SRC_ID_SUBSECOND_ADJUST				32

//! \def SRC_ID_RST_VECTOR
//! \brief Indicates reason for the restart
#define SRC_ID_RST_VECTOR							33

//! \def SRC_ID_MISSED_RTR
//! \brief The child missed the ready to receive message
#define	SRC_ID_MISSED_RTR				40

//! \def SRC_ID_MISSED_LRQ
//! \brief The child missed the link request message
#define	SRC_ID_MISSED_LRQ				41

//! \def SRC_ID_LINK_BROKEN
//! \brief The RF link between two nodes has been dropped
#define	SRC_ID_LINK_BROKEN				42

//! \def SRC_ID_ROUTING_FAIL
//! \brief The node that the command was destined for is not in the network
#define SRC_ID_ROUTING_FAIL				43

//! \def SRC_ID_JOINED_NET
//! \brief The node has joined the network
#define SRC_ID_JOINED_NET					44

//! \def SRC_ID_CHILD_JOINED
//! \brief A child node has joined this node
#define SRC_ID_CHILD_JOINED				45

//! \def SRC_ID_RSSI
//! \brief An RSSI reading
#define SRC_ID_RSSI								46

//! \def SRC_ID_SP_COMM_ERR
//! \brief An error communicating with an SP
#define SRC_ID_SP_COMM_ERR				47

//! \def SRC_ID_MSG_DELIVERY_FAIL
//! \brief A message failed to be delivered to a parent node or the garden server
#define SRC_ID_MSG_DELIVERY_FAIL				48

//! \def SRC_ID_CMD_EXPIRED
//! \brief A command has been received but it is too late to execute it
#define SRC_ID_CMD_EXPIRED				49

//debug - remove
#define SRC_ID_SEND_RTR						50
#define SRC_ID_WAIT_RTR						51
#define SRC_ID_LINK_ZERO					52
#define SRC_ID_LINK_LATE					53
#define SRC_ID_FORCE_STATIC				54
#define SRC_ID_FORCE_DYNAMIC 			55
#define SRC_ID_OVERWRITE 					56
#define SRC_ID_NO_OVERWRITE 			57

// Miscellaneous errors
//! \def SRC_ID_CMD_DNE
//! \brief The command received from the datacenter does not exists in this node
#define SRC_ID_CMD_DNE					60

//! \def SRC_ID_MODIFYTCB_FAIL
//! \brief The task manager was unable to modify a task control block parameter over the network
#define SRC_ID_MODIFYTCB_FAIL		61

//! \def SRC_ID_COLLISION
//! \brief There was  a slot collision when scheduling communication between nodes
#define SRC_ID_COLLISION		63

//! \def SRC_ID_FRAM_ACCESS_VIOLATION
//! \brief There was an attempt to access a locked section of FRAM
#define SRC_ID_FRAM_ACCESS_VIOLATION		100

//! \def SRC_ID_FRAM_WRITE_FAIL
//! \brief There was a failure to write to FRAM
#define SRC_ID_FRAM_WRITE_FAIL					101
//! @}


// Report priorities range from 0-5 with zero being the lowest priority and 5 being the highest.
// All reports with priority of 5 are considered crisis level reports and are stored in flash memory
// of the MCU.  When a data element is generated the WiSARD refers to its reporting priority to determine
// if the report should be saved or disregarded.
//! \defgroup Reporting priorities
//! @{
//! \def MAXREPORTPRIORITY
//! \brief The maximum value for reporting priority
#define MAXREPORTPRIORITY 			5
//! \def DEFAULTREPORTINGPRIORITY
//! \brief The default setting for reporting priority
#define DEFAULTREPORTINGPRIORITY	0
//! \def RPT_PRTY_INPUT_VOLTAGE_LOW
//! \brief Priorirty of data element that indicates the source voltage is too low
#define RPT_PRTY_INPUT_VOLTAGE_LOW		5
//! \def RPT_PRTY_INPUT_VOLTAGE
//! \brief Priorirty of data element that reads from the 5V input voltage line
#define RPT_PRTY_INPUT_VOLTAGE			4
//! \def RPT_PRTY_INTERNAL_TEMP
//! \brief Priorirty of data element that reads from the ADC of the MCU's temperature
#define RPT_PRTY_INTERNAL_TEMP			4
//! \def RPT_PRTY_ID_MSG_COUNT
//! \brief Priority of the data element that reports the number messages in the queue
#define RPT_PRTY_MSG_COUNT					1
//! \def RPT_PRTY_RDIO_DIAG
//! \brief Priority of the data element that reports radio diagnostic data
#define RPT_PRTY_RDIO_DIAG					4
//! \def RPT_PRTY_RPT_HID
//! \brief Priority of the data element that reports the hardware IDs of all boards in the WiSARD
#define RPT_PRTY_RPT_HID					4

//! \def RPT_PRTY_COUNT
//! \brief delete this
#define RPT_PRTY_COUNT					1

//Memory stuff
//! \def RPT_PRTY_SDCARD_INIT_FAIL
//! \brief Priorirty of data element that indicates the SD Card initialization was unsuccessful
#define RPT_PRTY_SDCARD_INIT_FAIL		5

//! \def RPT_PRTY_SDCARD_BLOCKLEN_FAIL
//! \brief Priorirty of data element that indicates that setting the SD Card block length was unsuccessful
#define RPT_PRTY_SDCARD_BLOCKLEN_FAIL	5

//! \def RPT_PRTY_SDCARD_WRITE_FAIL
//! \brief Priorirty of data element that indicates a write to the SD Card was unsuccessful
#define RPT_PRTY_SDCARD_WRITE_FAIL		5

//! \def RPT_PRTY_SDCARD_READ_FAIL
//! \brief Priorirty of data element that indicates read from the SD Card was unsuccessful
#define RPT_PRTY_SDCARD_READ_FAIL		5

//! \def RPT_PRTY_SYSTEM_RESTART
//! \brief Priorirty of data element that indicates that the system has restarted
#define RPT_PRTY_SYSTEM_RESTART			5

//! \def RPT_PRTY_RST_VECTOR
//! \brief Priorirty of data element that indicates the reason for a restart
#define RPT_PRTY_RST_VECTOR					5

//! \def RPT_PRTY_SET_TIME
//! \brief Priorirty of data element that indicates that the system clock was set
#define RPT_PRTY_SET_TIME				3

//! \def RPT_PRTY_SUBSECOND_ADJUST
//! \brief Priorirty of data element that indicates the adjustments to the sub second compare register
#define RPT_PRTY_SUBSECOND_ADJUST		3

//RF Stuff
//! \def RPT_PRTY_MISSED_RTR
//! \brief Priorirty of data element that indicates the child missed the ready to receive message
#define	RPT_PRTY_MISSED_RTR				3

//! \def RPT_PRTY_MISSED_LRQ
//! \brief Priorirty of data element that indicates the child missed the link request message
#define	RPT_PRTY_MISSED_LRQ				3

//! \def RPT_PRTY_LINK_BROKEN
//! \brief Priorirty of data element that indicates the RF link between two nodes has been dropped
#define	RPT_PRTY_LINK_BROKEN			4

//! \def RPT_PRTY_ROUTING_FAIL
//! \brief Priorirty of data element that indicates the node that the command was destined for is not in the network
#define RPT_PRTY_ROUTING_FAIL			4

//! \def RPT_PRTY_JOINED_NET
//! \brief Priorirty of data element that indicates the node has joined the network
#define RPT_PRTY_JOINED_NET				4

//! \def RPT_PRTY_CHILD_JOINED
//! \brief Priorirty of data element that indicates a child node has joined this node
#define RPT_PRTY_CHILD_JOINED			4

//! \def RPT_PRTY_RSSI
//! \brief Priorirty of the RSSI reading data element
#define RPT_PRTY_RSSI					4

//! \def RPT_PRTY_SP_COMM_ERR
//! \brief Priorirty of data element that indicates an error communicating with an SP
#define RPT_PRTY_SP_COMM_ERR			5

//! \def RPT_PRTY_MSG_DELIVERY_FAIL
//! \brief Priorirty of data element that indicates a message failed to be delivered to a parent node or the garden server
#define RPT_PRTY_MSG_DELIVERY_FAIL		4

//! \def RPT_PRTY_CMD_EXPIRED
//! \brief Priorirty of data element that indicates a command has expired
#define RPT_PRTY_CMD_EXPIRED		4

//debug - remove
#define RPT_PRTY_SEND_RTR				1
#define RPT_PRTY_WAIT_RTR				1
#define RPT_PRTY_LINK_ZERO				1
#define RPT_PRTY_LINK_LATE				1
#define RPT_PRTY_FORCE_STATIC			1
#define RPT_PRTY_FORCE_DYNAMIC 			1
#define RPT_PRTY_OVERWRITE 				1
#define RPT_PRTY_NO_OVERWRITE 			1

// Miscellaneous errors
//! \def RPT_PRTY_CMD_DNE
//! \brief Priorirty of data element that indicates the command received from the datacenter does not exists in this node
#define RPT_PRTY_CMD_DNE				4

//! \def RPT_PRTY_MODIFYTCB_FAIL
//! \brief Priorirty of data element that indicates the task manager was unable to modify a task control block parameter over the network
#define RPT_PRTY_MODIFYTCB_FAIL			4

//! \def RPT_PRTY_COLLISION
//! \brief Priorirty of data element that indicates there was  a slot collision when scheduling communication between nodes
#define RPT_PRTY_COLLISION				2

//! \def RPT_PRTY_FRAM_ACCESS_VIOLATION
//! \brief Priorirty of data element that indicates there was an attempt to access a locked section of FRAM
#define RPT_PRTY_FRAM_ACCESS_VIOLATION	5

//! \def RPT_PRTY_FRAM_WRITE_FAIL
//! \brief Priorirty of data element that indicates there was a failure to write to FRAM
#define RPT_PRTY_FRAM_WRITE_FAIL		5
//! @}

void vReport_RAM_QueueInit(void);
uchar ucReport_ReadDEFromRAM(uchar *p_ucaBuff);
void vReport_RemoveDEFromRAM(void);
uint uiReport_RAM_QueueCount(void);
void vREPORT_LogReport(void);
void vReport_LogDataElement(unsigned char ucPriority);
void vReport_BuildMsgsFromDEs(void);

void vReport_LoadSRAMFromSDCard(void);
uchar ucReport_CrisisLog(void);
void vReport_PrintCrisisLog(void);
void vReport_EraseCrisisLog(void);
uchar ucReport_Test(void);

#endif /* REPORT_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
