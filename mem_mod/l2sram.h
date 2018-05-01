/***************************  L2SRAM.H  ****************************************
 *
 * Header for L2SRAM routine pkg
 *
 * NOTE: See Main.h for the disk layout map.
 *
 *
 *
 * V1.00 06/18/2003 wzr
 *	started
 *
 ******************************************************************************/

#ifndef L2SRAM_H_INCLUDED
#define L2SRAM_H_INCLUDED

/*---------------  SAVED_SENSE_POINT (SSP) TABLES  --------------------------*/

#define SSP_GENERIC_TBL_SIZE	256

/*--------------------- TABLE 0 (SSP Y READING TBL) -------------------------*/
#define SSP_Y_TBL_NUM					0
#define SSP_Y_TBL_BASE_ADDR	 	0		// start at the first location in SRAM
#define SSP_Y_TBL_BYTE_WIDTH			2
#define SSP_Y_TBL_END_ADDR_PLUS_1 		(SSP_Y_TBL_BASE_ADDR + \
 					(SSP_GENERIC_TBL_SIZE * SSP_Y_TBL_BYTE_WIDTH))
#define SSP_Y_TBL_NOT_USED_VAL	0

/*--------------------- TABLE 1 (SSP T (TIME) TBL) --------------------------*/
#define SSP_T_TBL_NUM					1
#define SSP_T_TBL_BASE_ADDR			SSP_Y_TBL_END_ADDR_PLUS_1
#define SSP_T_TBL_BYTE_WIDTH			4
#define SSP_T_TBL_END_ADDR_PLUS_1		(SSP_T_TBL_BASE_ADDR + \
 					(SSP_GENERIC_TBL_SIZE * SSP_T_TBL_BYTE_WIDTH))
#define SSP_T_TBL_NOT_USED_VAL	0

/*--------------------- TABLE 2 (SSP DELTA_Y_TRIG TBL) -----------------------*/
#define SSP_DELTA_Y_TRIG_TBL_NUM			2
#define SSP_DELTA_Y_TRIG_TBL_BASE_ADDR		SSP_T_TBL_END_ADDR_PLUS_1
#define SSP_DELTA_Y_TRIG_TBL_BYTE_WIDTH	2
#define SSP_DELTA_Y_TRIG_TBL_END_ADDR_PLUS_1 (SSP_DELTA_Y_TRIG_TBL_BASE_ADDR + \
 					(SSP_GENERIC_TBL_SIZE * SSP_DELTA_Y_TRIG_TBL_BYTE_WIDTH))
#define SSP_DELTA_Y_TRIG_TBL_NOT_USED_VAL	0

/*--------------------- TABLE 3 (SSP DELTA_T_TRIG TBL) -----------------------*/
#define SSP_DELTA_T_TRIG_TBL_NUM			3
#define SSP_DELTA_T_TRIG_TBL_BASE_ADDR		SSP_DELTA_Y_TRIG_TBL_END_ADDR_PLUS_1
#define SSP_DELTA_T_TRIG_TBL_BYTE_WIDTH	2
#define SSP_DELTA_T_TRIG_TBL_END_ADDR_PLUS_1 (SSP_DELTA_T_TRIG_TBL_BASE_ADDR + \
 					(SSP_GENERIC_TBL_SIZE * SSP_DELTA_T_TRIG_TBL_BYTE_WIDTH))
#define SSP_DELTA_T_TRIG_TBL_NOT_USED_VAL	0

#define LAST_SSP_TBL_NUM_PLUS_1	4

/*------------------------  END SSP TBLS  -----------------------------------*/


/*------------------  MSG STORE AND FORWARD MEMORY  -------------------------*/
// Note there are two separate queues a command queue and a report queue

#define L2SRAM_BASE_ADDR             SSP_DELTA_T_TRIG_TBL_END_ADDR_PLUS_1
#define L2SRAM_BASE_ADDR_UL ((ulong) SSP_DELTA_T_TRIG_TBL_END_ADDR_PLUS_1)

// This is what the metadata table looks like.
// It holds the total number of commands
//-------------------------------------------------------------------
// Num_cmds  |   NFL   |    S    |   R    |   A    |    M   |       |
//-------------------------------------------------------------------
// Node ID   |Cmds/Node| ADDR_1 | ADDR_2 | ADDR_3 | ADDR_4 | ADDR_5 |
// 2-bytes	  1-byte	   4-bytes  4-bytes  4-bytes  4-bytes  4-bytes
//-------------------------------------------------------------------
//   :			 |		:		 |	 :		|		: 	 |		:	 	|		:	 	 |		:		|
//   :			 |		:		 |	 :		|		: 	 |		:	 	|		:	 	 |		:		|
//   :			 |		:		 |	 :		|		: 	 |		:	 	|		:	 	 |		:		|
//   :			 |		:		 |	 :		|		: 	 |		:	 	|		:	 	 |		:		|
//-------------------------------------------------------------------

// Address of metadata regarding command queue
#define CMD_METADATA_START_ADDR			 	L2SRAM_BASE_ADDR	// Starting address of the command queue metadata
#define NUM_CMDS_VAR_ADDR							CMD_METADATA_START_ADDR
#define NUM_CMDS_VAR_LEN						 	1  	// Length of the variable that holds the number of commands

#define CMD_Q_NFL_ADDR							  NUM_CMDS_VAR_ADDR	+ NUM_CMDS_VAR_LEN //starting address of the next free location pointer
#define CMD_Q_NFL_LEN									4		// Length of the pointer

#define CMD_Q_FORMAT_ADDR							CMD_Q_NFL_ADDR	+ CMD_Q_NFL_LEN	// starting address of the format check bytes
#define CMD_Q_FORMAT_LEN							4		// Length of the format check bytes
#define CMD_Q_FORMAT_XI								'S'
#define CMD_Q_FORMAT_HI								'R'
#define CMD_Q_FORMAT_MD								'A'
#define CMD_Q_FORMAT_LO								'M'

#define CMD_Q_FIRST_ROW_ADDR					(CMD_Q_FORMAT_ADDR + CMD_Q_FORMAT_LEN) 	// Starting address of the node IDs
#define NODE_ID_LEN									 	2		// Length of the ID of a node
#define NUM_CMDS_PER_NODE_LEN				 	1		// Length of the area holding the number of commands pending for a single node
#define CMD_ADDR_LEN								 	4		// Then length of the address where the command is stored
#define NUM_CMDS_PER_NODE						 	5		// Maximum number of commands pending for a single node
#define MAX_NUM_CMDS								 	25	// Maximum number of commands
#define CMD_METADATA_ROW_LEN			    (NUM_CMDS_PER_NODE_LEN + NODE_ID_LEN + (CMD_ADDR_LEN * NUM_CMDS_PER_NODE)) // Length of a row in the table
#define SIZE_OF_METADATA						 	(NUM_CMDS_VAR_LEN	+ CMD_Q_FORMAT_LEN + (MAX_NUM_CMDS * CMD_METADATA_ROW_LEN) + 4)	// Size of all of the metadata section
#define CMD_METADATA_END_ADDR				 	(CMD_METADATA_START_ADDR + SIZE_OF_METADATA) // End address of the metadata
#define CMD_METADATA_END_ADDR_PLUS_1 	(CMD_METADATA_END_ADDR+1)

/*-------------------------- Start of the command queue -------------------------------------*/
#define CMD_QUEUE_START_ADDR					CMD_METADATA_END_ADDR_PLUS_1
#define CMD_QUEUE_LEN									(MAX_NUM_CMDS * MAX_MSG_SIZE_UL)
#define CMD_QUEUE_END_ADDR						(CMD_QUEUE_START_ADDR + CMD_QUEUE_LEN)
#define CMD_QUEUE_END_ADDR_PLUS_1			(CMD_QUEUE_END_ADDR+1)

#define L2SRAM_MSG_PAGE_MASK_UL	((ulong)0x0000001F)
#define L2SRAM_MSG_Q_BEG_UL	((CMD_QUEUE_END_ADDR_PLUS_1+31) & ~L2SRAM_MSG_PAGE_MASK_UL)

/* test length */
//#define SRAM_TEST_MSG_BUFF_SIZE (5UL * MAX_MSG_SIZE_UL)
//#define L2SRAM_MSG_Q_END_UL	(L2SRAM_MSG_Q_BEG_UL + SRAM_TEST_MSG_BUFF_SIZE)
/* run length */
#define L2SRAM_MSG_Q_END_UL	MAX_SRAM_ADDR_UL    //0x3FFFF  256k
#define L2SRAM_MSG_BUFF_COUNT_UL ((L2SRAM_MSG_Q_END_UL - L2SRAM_MSG_Q_BEG_UL)/MAX_MSG_SIZE_UL)

#define L2SRAM_Q_ON_ID			1
#define L2SRAM_Q_OFF_ID			2





/*--------------------  ROUTINE DEFINITIONS  --------------------------------*/

void vL2SRAM_init(void);

ulong ulL2SRAM_getPickTblEntry(usl uslTblBaseAddr, //Tbl base addr
    usl uslTblBaseMax, //Tbl base max (LUL)
    uchar ucTblIdx, //Tbl index
    uchar ucTblIdxMax, //Tbl index Max (LUL)
    uchar ucTblByteWidth //Tbl byte width
    );

void vL2SRAM_putPickTblEntry(USL uslTblBaseAddr, //Tbl base addr
    usl uslTblBaseMax, //Tbl base max (LUL)
    uchar ucTblIdx, //Tbl index
    usl ucTblIdxMax, //Tbl index Max (LUL)
    uchar ucTblByteWidth, //Tbl byte width
    ulong ulEntryVal //Tbl entry value
    );

ulong ulL2SRAM_getStblEntry(uchar ucTblNum, //Tbl Number
    uchar ucTblIdx //Tbl index
    );

void vL2SRAM_putStblEntry(uchar ucTblNum, //Tbl Number
    uchar ucTblIdx, //Tbl index
    ulong ulEntryVal //Tbl entry value
    );

uint uiL2SRAM_getMsgCount(void);

uint uiL2SRAM_getVacantMsgCount(void);

void vL2SRAM_storeMsgToSram(void);

void vL2SRAM_storeMsgToSramIfAllowed(void);

uchar ucL2SRAM_getCopyOfCurMsg(void);


void vL2SRAM_delCurMsg(void);

void vL2SRAM_showTblName( //locate an SRAM addr in an SRAM table & show it
    ulong ulAddr);


//*************************  Report Queue Functions ***********************//
void vL2SRAM_storeDEToSramIfAllowed(void);
uchar ucL2SRAM_getCopyOfCurDE(uchar ucMsgPtr);
void vL2SRAM_delCurDE(void);


//************************** CMD Queue Functions ***************************//
void vL2SRAM_Display_CmdQueueMetadata(void);
void vL2SRAM_FormatCmd_Q(void);
void vL2SRAM_TestCmdQueue(void);
uchar ucL2SRAM_LoadCmdIfExists(uint uiNodeID);
uchar ucL2SRAM_IsCmdQueueFormatted(void);
uchar ucL2SRAM_PutCMD_inQueue(uint uiNodeID);
void ucL2SRAM_Del_current_CMD(void);

#if 0
void vL2SRAM_dumpTblBases(
		void
);
#endif

#endif /* L2SRAM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
