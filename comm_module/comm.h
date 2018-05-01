//! \file comm.h
//! \brief The file contains the Messages module
//!
//!
//! @addtogroup Communications
//! The Communication Module (comm.h) contains all of the data structures and defines
//! to communicate back and forth between the SP Board boards, the radio, the
//! garden server and the brain board
//! @{
///////////////////////////////////////////////////////////////////////////////
#ifndef COMM_H_INCLUDED
#define COMM_H_INCLUDED

#include "task.h"

//! @name Data Element definitions
//! {
//! \def TRANSDUCER_LABEL_LEN
//! \brief The fixed length of the transducer labels
#define TRANSDUCER_LABEL_LEN 0x10
//! \def VERSION_LABEL_LEN
//! \brief The fixed length of the version labels
#define VERSION_LABEL_LEN    0x10
//! \def SP_CORE_VERSION
//! \brief This sensor number is for requesting the version string
//!
//! This sensor number should only EVER be used with REQUEST_LABEL packet.
//! Otherwise the system will try to access an array index that does
//! not exist, which will fault the hardware and lock up the software.
#define SP_CORE_VERSION 0x10
//! \def WRAPPER_VERSION
//! \brief This sensor number is for requesting the wrapper version string
//!
//! This sensor number should only EVER be used with REQEUST_LABEL packet.
//! Otherwise the system will try to access an array index that does not
//! exist, which will fault the hardware and lock up the software.
#define WRAPPER_VERSION 0x11

//! \def SW_VERSION
//! \brief This is the version of the software running on the device generating the message
//!
//! For the release of the generation 3 WiSARD we are starting the software version at 4.0
//! as the latest release of the generation 2 software was 3.62a
#define SW_VERSION	0x04

//****************  SP Board Data Messages  *********************************//
//! @defgroup msg_data SP Board Data Message
//! The SP Board Data Message is used to sent data back and forth between the
//! SP Board and the CP Board. The request message is ALWAYS a data message, the
//! return message can be a data message or a label message.
//! @{
#define SP_DATAMESSAGE_VERSION 110     //!< Version 1.10
// Message Types
//! @name Data Message Types
//! These are the possible data message types.
//! @{
//! \def COMMAND_PKT
//! \brief This packet contains the command from the CP Board to the SP Board on
//! what functions to execute.
//!
//! The SP Board replies with a CONFIRM_COMMAND if the command is valid
#define COMMAND_PKT   		0x01

//! \def REPORT_DATA
//! \brief This packet reports a transducer measurement to the CP Board
//!
//! This packet is only sent to the CP Board. The
//! transducer measurement is contained in data1 through data 8.
//! The SP sends a
#define REPORT_DATA     	0x02

//! \def PROGRAM_CODE
//! \brief This packet contains program code
//!
//! This packet is only sent from the real-time-data-center to the target. The
//! packet contains a portion of a programming update.
#define PROGRAM_CODE     0x03

//! \def REQUEST_DATA
//! \brief This packet requests data from the SP Board board
//!
//! This packet is only sent from the CP Board to the SP Board.
//! The SP will send a REPORT_DATA packet as a reply, either 32 bit or 128 bit
//! packet depending on the SP function.
#define REQUEST_DATA    	0x04

//! \def REQUEST_LABEL
//! \brief This packet asks the SP Board board to send a label message back
//!
//! The packet is sent from the CP Board to the SP Board and the CP Board will
//! expect a SP_LabelMessage in return. At this time the data1 and
//! data2 fields in the message contain do-not-care values.
#define REQUEST_LABEL   	0x05

//! \def ID_PKT
//! \brief This packet identifies the SP Board board to the CP Board
//!
//! This packet is only ever sent when the SP Board starts up and should
//! never be sent by the CP Board.
#define ID_PKT          	0x06

//! \def CONFIRM_COMMAND
//! \brief This packet is used by the SP Board to confirm that the COMMAND_PKT
//! sent by the CP Board is valid and that the commands will be executed.
//!
#define CONFIRM_COMMAND 	0x07

//! \def SP_ERROR
//! \brief This packet is used by the SP Board to inform the CP board
//! that there was a transducer error
//!
#define REPORT_ERROR 			0x08

//! \def REQUEST_BSL_PW
//! \brief This packet is used by the CP board to request the bootstrap loader password
//!
//! The SP will look into flash address 0xFFE0 to 0xFFFF and send the contents to the
//! CP board.
//!
#define REQUEST_BSL_PW   	0x09

//! \def INTERROGATE
//! \brief This packet is used by the CP board to request the transducer information
//!
//!
#define INTERROGATE   0x0A

//! \def SET_SERIALNUM
//! \brief Used to set the serial number on the SP board from the CP.
//! This is done only during diagnostics mode
#define SET_SERIALNUM		0x0B

//! \def REPORT_LABEL
//! \brief This packet contains a label indicated by the sensor number.
//!
//! This type of label packet is only sent by the SP Board board in response
//! to a LABEL_REQUEST Data Message. It has a different packet length and
//! is longer than a Data Message and therefore can be received as a
//! Data Message in error.
#define REPORT_LABEL             0x0A

//! \def PROGRAM_CODE_META
//! \brief This packet contains meta data about program code
//!
//! This packet is only sent from the real-time-data-center to the target. The
//! packet contains meta data about a portion of a programming update.
#define PROGRAM_CODE_META     0x0D

//! \def PROGRAM_CODE_INIT
//! \brief This packet contains meta data about program code
//!
//! This packet is only sent from the real-time-data-center to the target. The
//! packet informs the WiSARD that progamming code is in route and provides image size and CRC info
#define PROGRAM_CODE_INIT     0x0E

//! @}

// Sensor Numbers
//! @name Data Message Sensor Numbers
//! These are the possible sensor numbers
//! @{
#define TRANSDUCER_0    0x00
#define TRANSDUCER_1    0x01
#define TRANSDUCER_2    0x02
#define TRANSDUCER_3    0x03
#define TRANSDUCER_4    0x04
#define TRANSDUCER_5    0x05
#define TRANSDUCER_6    0x06
#define TRANSDUCER_7    0x07
#define TRANSDUCER_8    0x08
#define TRANSDUCER_9    0x09
#define TRANSDUCER_A    0x0A
#define TRANSDUCER_B    0x0B
#define TRANSDUCER_C    0x0C
#define TRANSDUCER_D    0x0D
#define TRANSDUCER_E    0x0E
#define TRANSDUCER_F    0x0F
//! @}

//! \def MAX_PARAMETERS
//! \brief The limit on the length of the parameter and data fields of application layer messages
#define MAX_SP_PARAMETERS 0x20

//! \def MAX_OP_MSG_LENGTH
//! \brief The limit on the length of the parameter and data fields of transport layer messages
#define MAX_OM_MSG_LENGTH 0x40

//! \def MAX_SP_MSG_LENGTH
//! \brief The limit on the length of an SP message
#define MAX_SP_MSG_LENGTH 0x30

//! \def SP_HEADERSIZE
//! \brief The size of the SP message header Type, Size, Version, Flag fields
#define SP_HEADERSIZE		0x04

//! \def CODEHEADERSIZE
//! \brief The size of the code DE header in addition to the standard
#define CODEHEADERSIZE		12

//! \def SP_MSG_TYP_IDX
#define SP_MSG_TYP_IDX			0
//! \def SP_MSG_LEN_IDX
#define SP_MSG_LEN_IDX			1
//! \def SP_MSG_VER_IDX
#define SP_MSG_VER_IDX			2
//! \def SP_MSG_FLAGS_IDX
#define SP_MSG_FLAGS_IDX		3
//! \def SP_MSG_PAYLD_IDX
#define SP_MSG_PAYLD_IDX		4

//! \def MAX_SP_DATA
//! \brief The maximum length of the SP data payload
#define MAX_SP_DATA		 (MAX_OM_MSG_LENGTH - SP_HEADERSIZE)

//! \def MAX_CODE
//! \brief The maximum length of the code in an SP code message
#define MAX_CODE		 (MAX_OM_MSG_LENGTH - CODEHEADERSIZE)

//! @name Data Element structures
//! @{
//! \brief Structure of a command Data Element(DE)
//!
//! This structure is used to define the command DE.
struct DE_Command_feilds
{
		uint8 ucDEID; //!< Type of the message.
		uint8 ucDE_Length; //!< Size of the message
		uint8 ucVersion; //!< The version number of the sources software
		uint8 ucCommandID_HI; //!< High byte of the command ID, the address of the processor receiving the command
		uint8 ucCommandID_LO; //!< Low byte of the command ID, the command to be executed
		uint8 ucaParameters[MAX_SP_PARAMETERS];
};

//! \def  DE_COMMAND_SIZE
//! \brief The size of a data message (in bytes).
#define DE_COMMAND_SIZE sizeof(struct DE_Command_feilds)

//! \brief This is the union used to work with data messages
//!
//! This union allows us to work with the various fields in the data message
//! easily while also making it easy send and receive using byte oriented
//! communication methods
union DE_Command
{
		uint8 ucByteStream[DE_COMMAND_SIZE];
		struct DE_Command_feilds fields;
};

//! \brief Structure of a report Data Element(DE)
//!
//! This structure is used to define the report DE.
struct DE_Report_fields
{
		uint8 ucDEID; //!< Type of the message.
		uint8 ucDE_Length; //!< Size of the message
		uint8 ucVersion; //!< The version number of the sources software
		uint8 ucReportID_HI; //!< High byte of the report ID, the address of the processor sending the report
		uint8 ucReportID_LO; //!< Low byte of the report ID, the transducer ID of the report
		uint8 ucTimestamp_XI; //!< The fourth byte of the timestamp (highest)
		uint8 ucTimestamp_HI; //!< The third byte of the timestamp
		uint8 ucTimestamp_MD; //!< The second byte of the timestamp
		uint8 ucTimestamp_LO; //!< The first byte of the timestamp (lowest)
		uint8 ucaData[MAX_SP_PARAMETERS]; //!< The sensor data
};

//! \def  DE_COMMAND_SIZE
//! \brief The size of a data message (in bytes).
#define DE_REPORT_SIZE sizeof(struct DE_Report_fields)

//! \def DE_HEADER_LEN
//! \brief Length of the DE header
#define DE_HEADER_LEN		9

//! \name Communication Flags
//! These are flags are used to pass information between CP and SP in the flags byte
//! @{
#define SP_SHUTDOWN_BIT		0x01
//! @}

//! \brief This is the union used to work with data messages
//!
//! This union allows us to work with the various fields in the data message
//! easily while also making it easy send and receive using byte oriented
//! communication methods
union DE_Report
{
		uint8 ucByteStream[DE_REPORT_SIZE];
		struct DE_Report_fields fields;
};

//! \brief Structure of a code Data Element(DE)
//!
//! This structure is used to define the code DE.
struct S_DE_Code
{
		uint8 m_ucDEID; //!< Type of the message.
		uint8 m_ucDE_Length; //!< Size of the message
		uint8 m_ucVersion; //!< The version number of the software
		uint8 m_ucBoardNum; //!< The processor to be reprogrammed
		uint8 m_ucBoardType; //!< The board type being reprogrammed
		uint8 m_ucProgID; //!< The unique ID of the programming update
		uint16 m_uiComponentNum; //!< The component number
		uint32 m_ulStartAddr; //!< The start address of the programming update (lowest)
		uint8 m_ucCode[MAX_CODE]; //!< The array storing the actual program code
};

//! \brief Structure of a metadata code Data Element(DE)
//!
//! This structure is used to define the metadata code DE.
struct DE_MetadataCode_feilds
{
		uint8 m_ucDEID; //!< Type of the message.
		uint8 m_ucDE_Length; //!< Size of the message
		uint8 m_ucVersion; //!< The version number of the software
		uint8 m_ucBoardNum; //!< The processor to be reprogrammed
		uint8 m_ucBoardType; //!< The board type being reprogrammed
		uint8 m_ucProgID; //!< The unique ID of the programming update
		uint16 m_uiComponentNum; //!< The component number
		uint32 m_ulStartAddr; //!< The start address of the programming update (lowest)
		uint8 m_ucCode[MAX_CODE]; //!< The array storing the actual program code
};

//! \brief This is the union used to work with data messages
//!
//! This union allows us to work with the various fields in the data message
//! easily while also making it easy send and receive using byte oriented
//! communication methods
union DE_Code
{
		uint8 ucByteStream[sizeof(struct S_DE_Code)];
		struct S_DE_Code fields;
};
//! @}

//! @name SP data messages
//! \brief Structure of a SP Data Message
//!
//! This structure is only used to define the fields in SP_DataMessage.
struct SP_DataMessage_Fields
{
		uint8 ucMsgType; //!< Type of the message.
		uint8 ucMsgSize; //!< Size of the message
		uint8 ucMsgVersion; //!< The version number of the message protocol
		uint8 ucFlags;			//!< Flags
		uint8 ucaData[MAX_SP_DATA]; //!< Sensor readings, command arguments
};

//! \def SP_DATAMESSAGE_SIZE
//! \brief The size of a data message (in bytes).
#define SP_DATAMESSAGE_SIZE sizeof(struct SP_DataMessage_Fields)

//! \brief This is the union used to work with data messages
//!
//! This union allows us to work with the various fields in the data message
//! easily while also making it easy send and receive using byte oriented
//! communication methods
union SP_DataMessage
{
		uint8 ucByteStream[SP_DATAMESSAGE_SIZE];
		struct SP_DataMessage_Fields fields;
};
//! @}

typedef struct{
		uint m_uiTaskId;
		uchar m_ucCmdLength;
		uchar m_ucCmdParam[MAXCMDPARAM];
}S_Cmd;

//! \struct S_Discovery
//! \brief Holds discovery state information
typedef struct {
		uint8 m_ucMode;
		ulong m_ulStartTime;  		//!< Time spent in a particular mode mode discovery started
		ulong m_ulMaxDuration;  //!< Maximum time allowed in a particular mode
}T_Discovery;


//! @{name SP board label message
//! \brief Structure of a SP Board Label Message
//!
//! This structure is only used inside of the SP_LabelMessage. It
//! should never be allocated on it's own.
struct SP_LabelMessage_Fields
{
		uint8 ucMsgType; //!< Type of the message.
		uint8 ucMsgSize; //!< Size of the message
		uint8 ucMsgVersion; //!< The version number of the message protocol
		uint8 ucSensorNumber; //!< Sensor Number
		uint8 ucaDescription[TRANSDUCER_LABEL_LEN]; //!< 16-byte char array for the label
};

//! \def SP_LABELMESSAGE_SIZE
//! \brief The size of the SP Board Label Message (in bytes).
#define SP_LABELMESSAGE_SIZE sizeof(struct SP_LabelMessage_Fields)

//! \brief The union used to work with label messages
//!
//! This union is what should be allocated to work with label messages. It
//! allows easy access into the various fields of the message and makes it
//! easy to send and receive using byte oriented communication methods.
union SP_LabelMessage
{
		uint8 ucByteStream[SP_LABELMESSAGE_SIZE];
		struct SP_LabelMessage_Fields fields;
};

//! @}

//****************  SP Board Label Message  *********************************//
//! @defgroup msg_label SP Board Label Message
//! The SP Board Label Message is used to send labels (byte strings) to the
//! CP board. The label message is only ever sent in response to a
//! LABEL_REQUEST Data Message. (See \ref msg_data).
//! @{
#define SP_LABELMESSAGE_VERSION 102        //!< v1.02
// Sensor Types
//! \name Label Message Sensor Numbers
//! These are the possible sensor numbers for Label Messages. While these
//! do overlap with the sensor numbers in \ref msg_data it is important
//! to use the right sensor number with the right packet in case one of the
//! message types changes.
//! @{
#define TRANSDUCER_0_LABEL       0x00
#define TRANSDUCER_1_LABEL       0x01
#define TRANSDUCER_2_LABEL       0x02
#define TRANSDUCER_3_LABEL       0x03
#define TRANSDUCER_4_LABEL       0x04
#define TRANSDUCER_5_LABEL       0x05
#define TRANSDUCER_6_LABEL       0x06
#define TRANSDUCER_7_LABEL       0x07
#define TRANSDUCER_8_LABEL       0x08
#define TRANSDUCER_9_LABEL       0x09
#define TRANSDUCER_A_LABEL       0x0A
#define TRANSDUCER_B_LABEL       0x0B
#define TRANSDUCER_C_LABEL       0x0C
#define TRANSDUCER_D_LABEL       0x0D
#define TRANSDUCER_E_LABEL       0x0E
#define TRANSDUCER_F_LABEL       0x0F
#define SP_CORE_VERSION_LABEL    0x10
#define WRAPPER_VERSION_LABEL    0x11
//! @}



//! \def MAX_SP_MSGSIZE
//! \brief The max limit on messages received from SP boards
#define MAX_SP_MSGSIZE		0x30

#define YES_RECEIVER_START 1
#define  NO_RECEIVER_START 0

#define USE_CLK2 1
#define USE_CLK1 0

#define SHOW_MSG_RAW		1
#define SHOW_MSG_COOKED		2

#define MAX_RESERVED_MSG_SIZE 70

#define MAX_MSG_SIZE 64							//radio msg pkt max size
#define MAX_MSG_SIZE_L ((long) MAX_MSG_SIZE)
#define MAX_MSG_SIZE_UL ((ulong) MAX_MSG_SIZE)

#define MAX_MSG_SIZE_MASK  (MAX_MSG_SIZE -1)
#define MAX_MSG_SIZE_MASK_L ((long)MAX_MSG_SIZE_MASK)

//! \def MAX_OM_MSGS
//! \brief Maximum number of operational messages in a slot (actual value is higher, but we leave room for error)
#define MAX_OM_MSGS			10

/* CHECK_BYTE_BIT DEFINITIONS */
#define CHKBIT_CRC			0x80	//10000000
#define CHKBIT_MSG_TYPE		0x40	//01000000
#define CHKBIT_GRP_SEL		0x20	//00100000
#define CHKBIT_GID			0x10	//00010000
#define CHKBIT_DEST_SN		0x08	//00001000
#define CHKBIT_SRC_SN		0x04	//00000100
#define CHKBIT_UNUSED_1		0x02	//00000010
#define CHKBIT_UNUSED_2		0x01	//00000001
/* SLOT AND FRAME DEFINITIONS */
#define SLOTS_PER_FRAME_I  60
#define SLOTS_PER_FRAME_L  ((long) SLOTS_PER_FRAME_I)

#define SECS_PER_SLOT_I 1
#define SECS_PER_SLOT_L ((long) SECS_PER_SLOT_I)

#define SECS_PER_FRAME_L (SLOTS_PER_FRAME_L * SECS_PER_SLOT_L)
#define MSECS_PER_FRAME_L (SECS_PER_FRAME_L*1000)

#define FRAMES_PER_HOUR_L (3600/SECS_PER_FRAME_L)

/* GENERAL RADIO DEFINES */

#define MSG_ST_DELAY_IN_nS_UL ((ulong) (300 * 1000000)) //300ms
#define MSG_ST_DELAY_IN_TICS_UL (MSG_ST_DELAY_IN_nS_UL / CLK_nS_PER_LTIC_UL)
#define MSG_ST_DELAY_IN_TICS_UI ((uint) MSG_ST_DELAY_IN_TICS_UL)

#define MSG_ST_DELAY_IN_CLKS_UI (MSG_ST_DELAY_IN_TICS_UI | 0x8000)

#define RADIO_BIT_IN_nS_UL (800UL * 1000UL)
#define RADIO_BIT_IN_TICS_X_10000_UL ((ulong) 262144)

#define RADIO_WARMUP_IN_nS_UL   (20UL * 1000000UL)			//20ms
#define RADIO_WARMUP_IN_100us_UL (RADIO_WARMUP_IN_nS_UL/100000)
#define RADIO_WARMUP_IN_100us_UI ((uint) RADIO_WARMUP_IN_100us_UL)
#define RADIO_WARMUP_IN_TICS_UL (RADIO_WARMUP_IN_nS_UL / CLK_nS_PER_LTIC_UL)
#define RADIO_WARMUP_IN_TICS_UI ((uint) RADIO_WARMUP_IN_TICS_UL)

/* --------------------  MESSAGE DEFINES ------------------------------- */

#define PARENT	0
#define CHILD		1
/********  MESSAGE TYPES  ***********/


#define MSG_ID_BEACON						1
#define MSG_ID_REQUEST_TO_JOIN	2
#define MSG_ID_OPERATIONAL			3
#define MSG_ID_RTR							4
#define MSG_ID_LRQ							5
#define MSG_ID_RTS							6

#define MSG_TYPE_MAX_COUNT	 7

#define FULLDISCOVERY						0
#define PARTIALDISCOVERY				1
#define BURSTDISCOVERY					2
#define MAXDISCOVERYMODES				3

//! \def CRC_SZ
//! \brief Size of the CRC bytes.
//! I add this to make the code more readable
#define CRC_SZ									2

//! \defgroup Network Layer Message Indices
//! @{
//! \def  NET_HDR_SZ
//! \brief Network layer size
#define NET_HDR_SZ							4

//! \def NET_IDX_DEST_HI
//! \brief Network layer destination address index (high byte)
#define NET_IDX_DEST_HI					0

//! \def NET_IDX_DEST_LO
//! \brief Network layer destination address index (low byte)
#define NET_IDX_DEST_LO					1

//! \def NET_IDX_SRC_HI
//! \brief Network layer source address index (high byte)
#define NET_IDX_SRC_HI		  		2

//! \def NET_IDX_SRC_LO
//! \brief Network layer source address index (low byte)
#define NET_IDX_SRC_LO		  		3
//! @}

//! \defgroup Transport Layer Message Indices
//! @{

//! \def  MSG_HDR_SZ
//! \brief Transport layer message size
#define MSG_HDR_SZ							7

//! \def MSG_IDX_ID
//! \brief Transport layer message identification index (8-bits)
#define MSG_IDX_ID							4

//! \def MSG_IDX_FLG
//! \brief Transport layer message flags index (8-bits)
#define MSG_IDX_FLG							5

//! \def MSG_IDX_NUM_HI
//! \brief Transport layer message number index (high byte)
#define MSG_IDX_NUM_HI					6

//! \def MSG_IDX_NUM_LO
//! \brief Transport layer message number index (low byte)
#define MSG_IDX_NUM_LO					7

//! \def MSG_IDX_ADDR_HI
//! \brief Transport layer message address index (high byte)
#define MSG_IDX_ADDR_HI					8

//! \def MSG_IDX_ADDR_LO
//! \brief Transport layer message address index (low byte)
#define MSG_IDX_ADDR_LO					9

//! \def MSG_IDX_LEN
//! \brief Transport layer message length index (8-bits)
#define MSG_IDX_LEN							10

//! \def MSG_IDX_PAYLD
//! \brief Transport layer message payload start index (8-bits)
#define MSG_IDX_PAYLD						11

//! \def MSG_IDX_LRQ
//! \brief Link request index (8-bits)
#define MSG_IDX_LRQ						11

//! \def MSG_IDX_GID_HI
//! \brief Transport layer beacon message group ID index (high byte)
#define MSG_IDX_GID_HI					11

//! \def MSG_IDX_GID_LO
//! \brief Transport layer beacon message group ID index (low byte)
#define MSG_IDX_GID_LO					12

//! \def MSG_IDX_RANDSEED_XI
//! \brief Transport layer request to join message random seed index (extra high byte)
#define MSG_IDX_RANDSEED_XI			11

//! \def MSG_IDX_RANDSEED_HI
//! \brief Transport layer request to join message random seed index (high byte)
#define MSG_IDX_RANDSEED_HI			12

//! \def MSG_IDX_RANDSEED_MD
//! \brief Transport layer request to join message random seed index (mid byte)
#define MSG_IDX_RANDSEED_MD			13

//! \def MSG_IDX_RANDSEED_LO
//! \brief Transport layer request to join message random seed index (low high byte)
#define MSG_IDX_RANDSEED_LO			14

//! \def MSG_IDX_NUM_EDGES
//! \brief Index for the number of edges in the edge list.  Used in discovery mode
#define MSG_IDX_NUM_EDGES				15

//! \def MSG_IDX_EDGE_DISC
//! \brief Starting index for the edge list.  Used in discovery mode
#define MSG_IDX_EDGE_DISC				16

//! \def BCNMSG_IDX_TIME_SEC_XI
//! \brief Transport layer beacon message time in seconds index (extra high byte)
#define BCNMSG_IDX_TIME_SEC_XI			13

//! \def BCNMSG_IDX_TIME_SEC_HI
//! \brief Transport layer beacon message time in seconds index (high byte)
#define BCNMSG_IDX_TIME_SEC_HI			14

//! \def BCNMSG_IDX_TIME_SEC_MD
//! \brief Transport layer beacon message time in seconds index (mid byte)
#define BCNMSG_IDX_TIME_SEC_MD			15

//! \def BCNMSG_IDX_TIME_SEC_LO
//! \brief Transport layer beacon message time in seconds index (low byte)
#define BCNMSG_IDX_TIME_SEC_LO			16

//! \def BCNMSG_IDX_TIME_SUBSEC_HI
//! \brief Transport layer beacon message time in sub-seconds index (high byte)
#define BCNMSG_IDX_TIME_SUBSEC_HI	17

//! \def BCNMSG_IDX_TIME_SUBSEC
//! \brief Transport layer beacon message time in sub-seconds index (low byte)
#define BCNMSG_IDX_TIME_SUBSEC_LO 	18

//! \def MSG_IDX_SRC_LEVEL
//! \brief Transport layer beacon message source level index (8-bits)
#define MSG_IDX_SRC_LEVEL		19

//! \def OPMSG_IDX_TIME_SEC_XI
//! \brief Transport layer operational message time in seconds index (extra high byte)
#define OPMSG_IDX_TIME_SEC_XI			11

//! \def OPMSG_IDX_TIME_SEC_HI
//! \brief Transport layer operational message time in seconds index (high byte)
#define OPMSG_IDX_TIME_SEC_HI			12

//! \def OPMSG_IDX_TIME_SEC_MD
//! \brief Transport layer operational message time in seconds index (mid byte)
#define OPMSG_IDX_TIME_SEC_MD			13

//! \def OPMSG_IDX_TIME_SEC_LO
//! \brief Transport layer operational message time in seconds index (low byte)
#define OPMSG_IDX_TIME_SEC_LO			14

//! \def OPMSG_IDX_TIME_SUBSEC_HI
//! \brief Transport layer operational message time in sub-seconds index (high byte)
#define OPMSG_IDX_TIME_SUBSEC_HI	15

//! \def OPMSG_IDX_TIME_SUBSEC
//! \brief Transport layer operational message time in sub-seconds index (low byte)
#define OPMSG_IDX_TIME_SUBSEC_LO 	16
//! @}


//! \defgroup Application Layer Message Indices
//! @{
//! \def DEID_IDX
//! \brief Index of the data element ID
#define DE_IDX_ID							0x00
//! \def DE_IDX_LENGTH
//! \brief
#define DE_IDX_LENGTH					0x01
//! \def DE_IDX_VERSION
//! \brief
#define DE_IDX_VERSION				0x02
//! \def
//! \brief
#define	DE_IDX_RPT_PROCID	 		0x03
//! \def DE_IDX_TIME_SEC_XI
//! \brief
#define	DE_IDX_TIME_SEC_XI 		0x04
//! \def DE_IDX_TIME_SEC_HI
//! \brief
#define DE_IDX_TIME_SEC_HI		0x05
//! \def DE_IDX_TIME_SEC_MD
//! \brief
#define DE_IDX_TIME_SEC_MD		0x06
//! \def DE_IDX_TIME_SEC_LO
//! \brief
#define DE_IDX_TIME_SEC_LO		0x07
//! \def
//! \brief
#define	DE_IDX_RPT_PAYLOAD	 	0x08

//! @}
//! @defgroup Messages Flags
//! @{

//! \def MSG_FLG_ACKRQST
//! \brief Message is requesting an acknowledgment
#define MSG_FLG_ACKRQST			0x08

//! \def MSG_FLG_ACK
//! \brief Message is an acknowledgment
#define MSG_FLG_ACK					0x10

//! \def MSG_FLG_SINGLE
//! \brief Message is not part of a sequence of messages
#define MSG_FLG_SINGLE			0x20

//! \def MSG_FLG_END
//! \brief Message is the end of a sequence of messages
#define MSG_FLG_END					0x40

//! \def MSG_FLG_BEG
//! \brief Message is the beginning of a sequence of messages
#define MSG_FLG_BEG					0x80
//! @}

//! @defgroup Error Messages
//! @{
//! \def SRCLVLERR
//! \brief Source level error flag
#define SRCLVLERR						0x01

//! \def PREXISTLNKERR
//! \brief Pre-existing link error flag
#define PREXISTLNKERR				0x02

//! \def MSGINTGRTYERR
//! \brief Message integrity error flag
#define MSGINTGRTYERR				0x04

//! \def TIMEOUTERR
//! \brief Time out error
#define TIMEOUTERR					0x08

//! \def RSSIERR
//! \brief RSSI too low
#define RSSIERR							0x10

//! @}

//! \def YES_RSSI
//! \brief Used to tell radio to sample RSSI
#define YES_RSSI		1
//! \def NO_RSSI
//! \brief Used to tell radio not to sample RSSI
#define NO_RSSI			0

/* OPERATIONAL MODE DEFINES */

///* OM2 INDEXES */
#define OM2_IDX_COLLECTION_TIME_XI	12
#define OM2_IDX_DATA_0_SENSOR_NUM	16
#define OM2_IDX_DATA_0_HI			17

#define BROKEN_LINK_MAX_COUNT		3


void vComm_DE_BuildReportHdr(uchar ucProcID,  uchar ucPayloadLen, uchar ucVersion);
void vComm_NetPkg_buildHdr(uint uiDest);

uchar ucComm_chkMsgIntegrity(uchar ucCheckByteBits, uchar ucReportByteBits, uchar ucMsgType, uint uiExpectedSrcSN, uint uiExpectedDestSN);

void vComm_showSNmismatch(uint uiExpectedVal, uint uiGotVal, uchar ucCRLF_flag //YES_CRLF, NO_CRLF
    );

void vComm_SendBeacon(void);
void vComm_Request_to_Join(void);

void vComm_Child(void);
void vComm_Parent(void);
void vComm_Msg_buildOperational(uchar ucFlags, uint uiMsgNum, uint uiDest, uchar ucMsgID);

uint uiComm_incMsgSeqNum( //RET: Incremented Msg Seq Num (not 0 or 255)
    void);

uchar ucComm_Handle_Command(S_Cmd S_CommandData);

void vComm_SynchFreq(signed char cClockAdjust);
void vComm_showSOMandROMcounts(uchar ucCRLF_termFlag);
void vCommTest(void);

void vCommSetDiscMode(uint8 ucMode);
void vCommGetDiscMode(T_Discovery *S_Disc);

/* ROUTINE DEFINITIONS */
uchar ucMSG_chkMsgIntegrity(uchar ucCheckByteBits, uchar ucReportByteBits, uchar ucMsgType, uint uiExpectedSrcSN, uint uiExpectedDestSN);

void vMSG_showSNmismatch(uint uiExpectedVal, uint uiGotVal, uchar ucCRLF_flag //YES_CRLF, NO_CRLF
    );

void vMSG_showStorageErr(char *cpLeadinMsg, uint uiStrLength, unsigned long ulFailAddr, unsigned long ulWroteVal, unsigned long ulReadVal);
void vMSG_buildMsgHdr_GENERIC( //HDR=Len,Type,Group,Src,Dest
    uchar ucMsgLen, uchar ucMsgType, uint uiDestSN, uchar ucFlags);

uchar ucComm_waitForMsgOrTimeout(unsigned char ucReadRSSI);
void vMSG_rec_obnd_msgs(void);
void vMSG_showMsgBuffer(uchar ucCRLF_termFlag, //YES_CRLF, NO_CRLF
    uchar ucShowTypeFlag //SHOW_MSG_RAW, SHOW_MSG_COOKED
    );

uchar ucComm_doSubSecXmit( //RET: 1-sent, 0-too late
    ulong ulSeconds,
		uint uiSubSecond,
    uchar ucClkChoiceFlag, //USE_CLK2, USE_CLK1
    uchar ucStartRecFlag //YES_RECEIVER_START, NO_RECEIVER_START
    );

void vMSG_stuffFakeMsgToSRAM(void);
void vMSG_checkFakeMsg(void);

uchar ucMSG_getLastFilledEntryInOM2( //RET: 0=none, ##=idx of last entry in OM2
    void);

#define MAX_NODES 128
#define MAX_EDGES (MAX_NODES-1)
#define MAX_UPDATES	20
#define ROUTE_ERROR_TABLE_EMPTY    0x10
#define ROUTE_ERROR_TABLE_FULL     0x20
#define ROUTE_ERROR_DOES_NOT_EXIST 0x30
#define ROUTE_ERROR_INVALID_EDGE   0x40


//! \def F_DELETE
//! \def F_JOIN
//! \def F_DROP
//! \brief Flags for updates to the routing module
#define F_DELETE			0x01
#define F_JOIN				0x02
#define F_DROP				0x04

typedef struct
{
	uint m_uiSrc;
	uint m_uiDest;
} S_Edge;

uchar ucRoute_Init(uint address);
uint uiRoute_GetNextHop(uint dest);
uchar ucRoute_NodeUnjoin(uint child);
uchar ucRoute_NodeJoin(uint parent, uint child, S_Edge* edges, int iNumEdges);
uchar ucRoute_GetUpdateCountBytes(uchar ucAddOrDrop);
void vRoute_GetUpdates(volatile uchar *ucaBuff, uchar ucSpaceAvail);
void vRoute_SetUpdates(volatile uchar *ucaBuff);
void vRouteClrFlaggedUpdates(void);
void vRoute_DisplayEdges(void);

// remove
void vComm_zroMissedMsgCnt(void);
//! @}
#endif /* COMM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
