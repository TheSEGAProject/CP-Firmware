/**************************  L2SRAM.C  *****************************************
 *
 * Routines to handle the static ram store and forward data area
 *
 *
 ******************************************************************************/

/*lint -e526 *//* function not defined */
/*lint -e657 *//* Unusual (nonportable) anonymous struct or union */
/*lint -e714 *//* symbol not referenced */
/*lint -e716 *//* while(1) */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
/*lint -e757 *//* global declarator not referenced */
/*lint -e758 *//* global union not referenced */
/*lint -e768 *//* global struct member not referenced */

#include "std.h"			//standard defines
#include "l2sram.h"			//level 2 sram memory routines
#include "config.h"		//configuration parameters
#include "sram.h"	//level 1 sram memory routines
#include "serial.h"			//serial port routines
#include "comm.h"			//message routines
#include "rts.h"			//real time sched
#include "modopt.h"			//Modify Options routines
#include "task.h"			// Definitions regarding tasks
/**********************  EXTERNS  ********************************************/

extern volatile uchar ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];

extern usl uslGLOB_sramQon_NFL; //max. number of sched. entries
extern usl uslGLOB_sramQoff;
extern uint uiGLOB_sramQcnt;

extern int iGLOB_completeSysLFactor;

/*************************** Vars *******************************************/
ulong ulCurrentAddr;
uint uiCurrentNodeID;
/**********************  TABLES  ********************************************/

//! \addtogroup CmdQueue
//! @{
#define NUMCMDS			0
#define NODEID 			1
#define CMDADDRSTRT	2
#define CMDADDR_0		2
#define CMDADDR_1		3
#define CMDADDR_2		4
#define CMDADDR_3		5
#define CMDADDR_4		6
//! @}

/************************  DECLARATIONS  *************************************/

static void vL2SRAM_incQptr(uchar ucQ_ID //Q indicator L2SRAM_Q_ON_ID or L2SRAM_Q_OFF_ID
    );

/************************  CODE  *********************************************/

/**********************  vL2SRAM_init() ***************************************
 *
 * Initialize the L2SRAM data structure in SRAM
 *
 *
 ******************************************************************************/

void vL2SRAM_init(void)
{
#if 1 //was 0
	vSERIAL_sout("E:L2SRAM_init\r\n", 15);
#endif

	uslGLOB_sramQon_NFL = L2SRAM_MSG_Q_BEG_UL;
	uslGLOB_sramQoff = L2SRAM_MSG_Q_BEG_UL;
	uiGLOB_sramQcnt = 0;

#if 1
	vSERIAL_sout("L2SRM:Qon=", 10);
	vSERIAL_HB24out(uslGLOB_sramQon_NFL);
	vSERIAL_sout("  Qoff=", 7);
	vSERIAL_HB24out(uslGLOB_sramQoff);
	vSERIAL_sout(" Qcnt=", 6);
	vSERIAL_UI16out(uiGLOB_sramQcnt);
	vSERIAL_crlf();
#endif

	return;

}/* END: vL2SRAM_init() */



/****************  ulL2SRAM_getGenericTblEntry()  ***************************
 *
 *
 *
 ******************************************************************************/
ulong ulL2SRAM_getPickTblEntry(usl uslTblBaseAddr, //Tbl base addr
    usl uslTblBaseMax, //Tbl base max (LUL)
    uchar ucTblIdx, //Tbl index
    uchar ucTblIdxMax, //Tbl index Max (LUL)
    uchar ucTblByteWidth //Tbl byte width
    )
{
	USL uslAbsTblAddr;
	USL uslTblIdx;
	USL uslTblByteWidth;
	ulong ulRetVal;

	ulRetVal = 0;

	/* REPORT OUT OF RANGE SCHED TBL BASE */
	if (uslTblBaseAddr > uslTblBaseMax)
	{
#if 1
		vSERIAL_sout("L2SRM:TblBaseMaxErr", 19);
#endif
		goto getGenericTblEntry_exit;
	}

	/* REPORT OUT OF RANGE SCHED TBL IDX */
	if (ucTblIdx > ucTblIdxMax)
	{
#if 1
		vSERIAL_sout("L2SRM:TblIdxMaxErr ", 19);
#endif
		goto getGenericTblEntry_exit;
	}

	uslTblIdx = ucTblIdx;
	uslTblByteWidth = ucTblByteWidth;
	uslAbsTblAddr = (uslTblBaseAddr + (uslTblIdx * uslTblByteWidth));

	switch (ucTblByteWidth)
	{
		case 1:
			ulRetVal = (ulong) ucSRAM_read_B8(uslAbsTblAddr);
		break;
		case 2:
			ulRetVal = (ulong) uiSRAM_read_B16(uslAbsTblAddr);
		break;

		case 4:
			ulRetVal = ulSRAM_read_B32(uslAbsTblAddr);
		break;
		default:
#if 1
			vSERIAL_sout("\r\nL2SRAM:OutOfRangeRdByteWidth= ", 32);
			vSERIAL_UIV8out(ucTblByteWidth);
			vSERIAL_crlf();
#endif
			ulRetVal = 0L;
		break;

	}/* END: switch() */

	getGenericTblEntry_exit:

	return (ulRetVal);

}/* END: ulL2SRAM_getGenericTblEntry() */

/***********************  vL2SRAM_putGenericTblEntry()  ************************
 *
 *
 *
 ******************************************************************************/
void vL2SRAM_putPickTblEntry(USL uslTblBaseAddr, //Tbl base addr
    usl uslTblBaseMax, //Tbl base max (LUL)
    uchar ucTblIdx, //Tbl index
    usl ucTblIdxMax, //Tbl index Max (LUL)
    uchar ucTblByteWidth, //Tbl byte width
    ulong ulEntryVal //Tbl entry value
    )
{
	USL uslAbsTblAddr;
	USL uslTblIdx;
	USL uslTblByteWidth;

	/* REPORT OUT OF RANGE SCHED TBL BASE */
	if (uslTblBaseAddr > uslTblBaseMax)
	{
#if 1
		vSERIAL_sout("L2SRM:TblBaseMaxErr", 19);
#endif
		goto putGenericTblEntry_exit;
	}

	/* REPORT OUT OF RANGE SCHED TBL IDX */
	if (ucTblIdx > ucTblIdxMax)
	{
#if 1
		vSERIAL_sout("L2SRM:TblIdxMaxErr ", 19);
#endif
		goto putGenericTblEntry_exit;
	}

	uslTblIdx = ucTblIdx;
	uslTblByteWidth = ucTblByteWidth;
	uslAbsTblAddr = (uslTblBaseAddr + (uslTblIdx * uslTblByteWidth));

	switch (ucTblByteWidth)
	{
		case 1:
			vSRAM_write_B8(uslAbsTblAddr, (uchar) ulEntryVal);
		break;
		case 2:
			vSRAM_write_B16(uslAbsTblAddr, (uint) ulEntryVal);
		break;
		case 4:
			vSRAM_write_B32(uslAbsTblAddr, ulEntryVal);
		break;
		default:
#if 1
			vSERIAL_sout("\r\nL2SRM:BdRdByteWdth= ", 22);
			vSERIAL_UIV8out(ucTblByteWidth);
			vSERIAL_crlf();
#endif
		break;

	}/* END: switch() */


	putGenericTblEntry_exit:

	return;

}/* END: ulL2SRAM_putGenericTblEntry() */



/************************  vL2SRAM_storeMsgToSram() *****************************
 *
 * add a new message to the SRAM storage
 *
 * NOTE: This routine does the actual function of stuffing the msg buffer into
 *		the SRAM, It does not check to see if its the right thing to do.
 *		do not put guards in this code.
 *
 ******************************************************************************/

void vL2SRAM_storeMsgToSram(void)
{
	uchar ucii;

	/* COPY MSG TO SRAM */
	for (ucii = 0; ucii < MAX_MSG_SIZE; ucii++)
	{
		vSRAM_write_B8(uslGLOB_sramQon_NFL + ucii, ucaMSG_BUFF[ucii]);
	}/* END: for(ui) */

	/* CHECK TO SEE IF WE ARE PASSING THE OFF Q PTR -- IF SO INC IT ALSO */
	if ((uiGLOB_sramQcnt != 0) && (uslGLOB_sramQon_NFL == uslGLOB_sramQoff))
	{
		vL2SRAM_incQptr(L2SRAM_Q_OFF_ID);
		vL2SRAM_incQptr(L2SRAM_Q_ON_ID);
		/* DONT BUMP THE COUNT ITS A WASH */
		return;
	}

	/* OTHERWISE ADD A DATA ITEM TO THE COUNT */
	vL2SRAM_incQptr(L2SRAM_Q_ON_ID);
	uiGLOB_sramQcnt++;

	iGLOB_completeSysLFactor++;
	return;

}/* END: vL2SRAM_storeMsgToSram() */

/*******************  vL2SRAM_storeMsgToSramIfAllowed() *************************
 *
 * store a msg to SRAM if we are storing to SRAM.
 *
 ******************************************************************************/

void vL2SRAM_storeMsgToSramIfAllowed(void)
{

	/* WE ARE NOT STORING TO SRAM IF WE ARE YES-SAMPLING AND NOT-SENDING */
	if ((ucMODOPT_readSingleRamOptionBit(OPTPAIR_WIZ_SAMPLES)) && (!ucMODOPT_readSingleRamOptionBit(OPTPAIR_WIZ_SENDS)))
		return;

	/* OTHERWISE SEND TO SRAM */
	vL2SRAM_storeMsgToSram();

	return;

}/* END: vL2SRAM_storeMsgToSramIfAllowed() */

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Fetches a copy of the current message from SRAM.
//!
//!
//! 
//! \param none		  
//! \return 0, nothing to send (we are empty) 1, success
////////////////////////////////////////////////////////////////////////////////
uchar ucL2SRAM_getCopyOfCurMsg(void)
{
	uchar ucii;

	/* CHECK IF WE HAVE ANYTHING TO COPY */
	if (uiGLOB_sramQcnt == 0)
	{
#if 0
		vSERIAL_sout("NoMsgToCopy\r\n", 13);
#endif
		return (0);
	}

	/* COPY SRAM TO MSG BUFFER */
	for (ucii = 0; ucii < MAX_MSG_SIZE; ucii++)
	{
		ucaMSG_BUFF[ucii] = ucSRAM_read_B8(uslGLOB_sramQoff + ucii);

	}/* END: for(ui) */

	return 1;

}/* END: ucL2SRAM_getCopyOfCurMsg() */

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Gets the number of report messages that are stored on the chip
//!
//! \param none
//! \return Number of reports
////////////////////////////////////////////////////////////////////////////////
uint uiL2SRAM_getMsgCount(void)
{
	return (uiGLOB_sramQcnt);

}/* END: uiL2SRAM_getMsgCount() */

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Estimated number of vacant messages on chip.
//!
//! \param none
//! \return ulMsgVacancy, Vacancy count
////////////////////////////////////////////////////////////////////////////////

uint uiL2SRAM_getVacantMsgCount(void)
{
	ulong ulMsgVacancy;

	ulMsgVacancy = L2SRAM_MSG_BUFF_COUNT_UL - uiGLOB_sramQcnt;

	return ((uint) ulMsgVacancy);

}/* END: uiL2SRAM_getVacantMsgCount() */

/**********************  vL2SRAM_incQptr()  ***********************************
 *
 * Inc the Q pointer indicated
 *
 ******************************************************************************/

static void vL2SRAM_incQptr(uchar ucQ_ID //Q indicator L2SRAM_Q_ON_ID or L2SRAM_Q_OFF_ID
    )
{
	usl uslGenericQptr;

	/* ASSUME WHAT THE Q PTR VALUE IS */
	uslGenericQptr = uslGLOB_sramQon_NFL;
	/* CHANGE IT IF IT WAS A BAD ASSUMPTION */
	if (ucQ_ID == L2SRAM_Q_OFF_ID)
		uslGenericQptr = uslGLOB_sramQoff;

	// Increment the queue pointer the length of a data element
	uslGenericQptr += MAX_MSG_SIZE;

	// If message storage in SRAM has been exceeded then point the queue to the beginning address of msg space
	if (uslGenericQptr >= L2SRAM_MSG_Q_END_UL)
		uslGenericQptr = L2SRAM_MSG_Q_BEG_UL;

	/* UPDATE THE VALUE OF THE POINTER */
	if (ucQ_ID == L2SRAM_Q_ON_ID)
	{
		uslGLOB_sramQon_NFL = uslGenericQptr;
		return;
	}

	uslGLOB_sramQoff = uslGenericQptr;
	return;

}/* END: vL2SRAM_incQptr() */


////////////////////////////////////////////////////////////////////////////////
//!
//! \brief removes current message from the SRAM storage
//!
//! \param none
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vL2SRAM_delCurMsg(void)

{
	/* CHECK IF WE HAVE ANYTHING TO DELETE */
	if (uiGLOB_sramQcnt == 0)
	{
#if 0
		vSERIAL_sout("L2SRM:NoMsgToDel\r\n", 18);
#endif
		return;
	}

	vL2SRAM_incQptr(L2SRAM_Q_OFF_ID);

	uiGLOB_sramQcnt--;

	return;

}/* END: vL2SRAM_delCurMsg() */



///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Formats the command queue format section by writing "SRAM" to a reserved
//! memory location.  This way,in the event of a reset,  the device can check
//! if this area is formatted before overwriting a queue that is holding commands.
//!
//!	\param none
//! \return none
///////////////////////////////////////////////////////////////////////////////
void vL2SRAM_FormatCmd_Q(void)
{
	ulong ulAddr;

	// Write the data to SRAM
	vSRAM_write_B8(CMD_Q_FORMAT_ADDR, CMD_Q_FORMAT_XI);
	vSRAM_write_B8(CMD_Q_FORMAT_ADDR + 1, CMD_Q_FORMAT_HI);
	vSRAM_write_B8(CMD_Q_FORMAT_ADDR + 2, CMD_Q_FORMAT_MD);
	vSRAM_write_B8(CMD_Q_FORMAT_ADDR + 3, CMD_Q_FORMAT_LO);

	// Set the total number of commands to 0
	vSRAM_write_B8(NUM_CMDS_VAR_ADDR, 0);

	// Set the next free location pointer to the start of the command queue
	vSRAM_write_B32(CMD_Q_NFL_ADDR, CMD_QUEUE_START_ADDR);

	// Clears the table from the node on
	for (ulAddr = CMD_Q_FIRST_ROW_ADDR; ulAddr <= CMD_METADATA_END_ADDR; ulAddr += 4)
	{
		vSRAM_write_B32(ulAddr, 0x00);
	}

#if 0
	vL2SRAM_Display_CmdQueueMetadata();
#endif

}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief checks to see if the command queue has been previously formatted
//!
//! \param none
//! \return 1 for formatted 0 for not formatted
///////////////////////////////////////////////////////////////////////////////
uchar ucL2SRAM_IsCmdQueueFormatted(void)
{
	if (ucSRAM_read_B8(CMD_Q_FORMAT_ADDR) != CMD_Q_FORMAT_XI)
		return 0;
	if (ucSRAM_read_B8(CMD_Q_FORMAT_ADDR + 1) != CMD_Q_FORMAT_HI)
		return 0;
	if (ucSRAM_read_B8(CMD_Q_FORMAT_ADDR + 2) != CMD_Q_FORMAT_MD)
		return 0;
	if (ucSRAM_read_B8(CMD_Q_FORMAT_ADDR + 3) != CMD_Q_FORMAT_LO)
		return 0;

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Gets the total number of commands stored in the command queue
//!
//!	\param none
//! \return none
///////////////////////////////////////////////////////////////////////////////
uchar ucL2SRAM_GetTotalCmds(void)
{
	return ucSRAM_read_B8(NUM_CMDS_VAR_ADDR);
}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Updates the total number of commands stored in the command queue
//!
//!	\param ucNumCmds; Number of additional commands being stored
//! \return none
///////////////////////////////////////////////////////////////////////////////
void vL2SRAM_UpdateTotalCmds(signed char cNumCmds)
{
	uchar ucOldCmdCount;
	uchar ucNewCmdCount;

	// Get the number of commands currently stored in the queue
	ucOldCmdCount = ucSRAM_read_B8(NUM_CMDS_VAR_ADDR);

	ucNewCmdCount = ucOldCmdCount + cNumCmds;

	// Write the updated value back to the queue
	vSRAM_write_B8(NUM_CMDS_VAR_ADDR, ucNewCmdCount);
}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Checks if there is a command pending for a node
//!
//!	\param uiNodeID; ID of the node in question
//! \return ulMetaAddr; Address of the node in the meta data
///////////////////////////////////////////////////////////////////////////////
ulong ulL2SRAM_CheckForNode(uint uiNodeID)
{
	ulong ulMetaAddr;

	// start at the start increment address by the length of a row and read until max number of cmds
	for (ulMetaAddr = CMD_Q_FIRST_ROW_ADDR; ulMetaAddr < CMD_METADATA_END_ADDR; ulMetaAddr += CMD_METADATA_ROW_LEN)
	{
		if (uiSRAM_read_B16(ulMetaAddr) == uiNodeID)
			return ulMetaAddr;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Gets the addresses of all commands for a particular node
//!
//!
//!	\param uiNodeID, The node identification; p_ulAddr, addresses of all commands
//!	\return none
///////////////////////////////////////////////////////////////////////////////
void vL2SRAM_GetCmdAddrs(uint uiNodeID, ulong *p_ulAddr)
{
	ulong ulNodeIDAddr;
	ulong ulCmdIndex;
	ulong ulCmdAddr;
	uchar ucCount;

	ulNodeIDAddr = ulL2SRAM_CheckForNode(uiNodeID);

	if (ulNodeIDAddr == 0)
		return;

	// Set the command index to the first location for the node of interest
	ulCmdIndex = ulNodeIDAddr + NODE_ID_LEN + NUM_CMDS_PER_NODE_LEN;

	// Look for an empty location for this node in the metadata table
	for (ucCount = 0; ucCount < NUM_CMDS_PER_NODE; ucCount++)
	{
		// Read out the address
		ulCmdAddr = ulSRAM_read_B32(ulCmdIndex);

		*p_ulAddr = ulCmdAddr; //ulSRAM_read_B32(ulCmdIndex);
		p_ulAddr += 1;

		// Update the command index
		ulCmdIndex += CMD_ADDR_LEN;
	}

}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Writes a command into the command queue from the message buffer
//!
//!	\return ulong ulNodeIDAddr Address of the node ID in the metadata
//!	\param uiNodeID
//////////////////////////////////////////////////////////////////////////////
ulong ulL2SRAM_AddNode_toQueue(uint uiNodeID)
{
	ulong ulNodeIDAddr;

	// start at the start increment address by the length of a row and read until max number of cmds
	for (ulNodeIDAddr = CMD_Q_FIRST_ROW_ADDR; ulNodeIDAddr < CMD_METADATA_END_ADDR; ulNodeIDAddr += CMD_METADATA_ROW_LEN)
	{
		if (uiSRAM_read_B16(ulNodeIDAddr) == 0x00)
		{
			vSRAM_write_B16(ulNodeIDAddr, uiNodeID);
			return ulNodeIDAddr;
		}
	}

	// Meta is full so overwrite the data in the beginning of the table
	vSRAM_write_B16(CMD_Q_FIRST_ROW_ADDR, uiNodeID);
	return CMD_Q_FIRST_ROW_ADDR;
}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Writes a command into the command queue from the message buffer
//!
//!	\return 0 success, 1 fail
//!	\param uiNodeID
//////////////////////////////////////////////////////////////////////////////
uchar ucL2SRAM_PutCMD_inQueue(uint uiNodeID)
{
	uchar ucCount;
	ulong ulNodeCmdCntAddr;
	ulong ulCmdIndex;
	ulong ulCmdAddr;
	ulong ulNFL; //Next free location (NFL)
	ulong ulNodeIndex;

	// Get the next free memory location to write the command to
	ulNFL = ulSRAM_read_B32(CMD_Q_NFL_ADDR);

	// Find the Node in the metadata
	ulNodeIndex = ulL2SRAM_CheckForNode(uiNodeID);

	// If the node is not in the metadata table then add it
	if (ulNodeIndex == 0)
	{
		ulNodeIndex = ulL2SRAM_AddNode_toQueue(uiNodeID);
	}

	// Set the address for the number of commands per node byte
	ulNodeCmdCntAddr = ulNodeIndex + NODE_ID_LEN;

	// Set the command index to the first location for the node of interest
	ulCmdIndex = ulNodeCmdCntAddr + NUM_CMDS_PER_NODE_LEN;

	// Look for an empty location for this node in the metadata table
	for (ucCount = 0; ucCount < NUM_CMDS_PER_NODE; ucCount++)
	{
		// Read out the address
		ulCmdAddr = ulSRAM_read_B32(ulCmdIndex);

		// The command index is empty so break out
		if (ulCmdAddr == 0)
			break;

		// Update the command index
		ulCmdIndex += CMD_ADDR_LEN;
	}

	// If there is no open location overwrite the oldest command
	if (ulCmdAddr != 0)
	{
		ulCmdIndex = (ulNodeCmdCntAddr + NUM_CMDS_PER_NODE_LEN) + (ucSRAM_read_B8(ulNodeCmdCntAddr) % NUM_CMDS_PER_NODE) * CMD_ADDR_LEN;
	}
	else // only increment the total number of commands if it is a new addition to the queue, not if it is an overwrite
	{
		// Increment the command count for the node
		vSRAM_write_B8(ulNodeCmdCntAddr, ucSRAM_read_B8(ulNodeCmdCntAddr) + 1);

		// Increment the total number of commands
		vL2SRAM_UpdateTotalCmds(1);
	}

	// Update the metadata table with the address of the command
	vSRAM_write_B32(ulCmdIndex, ulNFL);

	// Write the command to the queue
	for (ucCount = 0; ucCount < MAX_MSG_SIZE; ucCount++)
	{
		vSRAM_write_B8(ulNFL + ucCount, ucaMSG_BUFF[ucCount]);
	}/* END: for(ucCount) */

	// Update the pointer to the NFL
	ulNFL += MAX_MSG_SIZE;
	// If the NFL is at the end of the queue start over at the beginning (simple ring buffer)
	if (ulNFL >= CMD_QUEUE_END_ADDR)
		ulNFL = CMD_QUEUE_START_ADDR;
	// Update the NFL in SRAM
	vSRAM_write_B32(CMD_Q_NFL_ADDR, ulNFL);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Removes a command from the command queue
//!
//!	\return ucStatus
//!	\param
//////////////////////////////////////////////////////////////////////////////
void ucL2SRAM_Delete_CMD(uint uiNodeID, ulong ulAddress)
{
	uchar ucCount;
	ulong ulAddrTemp;
	ulong ulNodeIndex;
	uchar ucNodeCmdCnt;
	ulong ulCmdIndex;
	ulong ulNodeCmdCntAddr;

	// Find the Node in the metadata
	ulNodeIndex = ulL2SRAM_CheckForNode(uiNodeID);
	if (ulNodeIndex == 0)
		return;

	// Set the command index to the first location for the node of interest
	ulCmdIndex = ulNodeIndex + NODE_ID_LEN + NUM_CMDS_PER_NODE_LEN;

	// Look for an matching address for this node in the metadata table
	for (ucCount = 0; ucCount < NUM_CMDS_PER_NODE; ucCount++)
	{

		// Read out the address
		ulAddrTemp = ulSRAM_read_B32(ulCmdIndex);

		if (ulAddrTemp == ulAddress)
			break;

		// Update the command index
		ulCmdIndex = ulCmdIndex + CMD_ADDR_LEN;
	}

	// We have found a matching command
	if (ulAddrTemp == ulAddress)
	{
		// Set the address for the number of commands per node byte
		ulNodeCmdCntAddr = ulNodeIndex + NODE_ID_LEN;

		// Get the command count for this node
		ucNodeCmdCnt = ucSRAM_read_B8(ulNodeCmdCntAddr);

		// Decrement the command count for the node
		vSRAM_write_B8(ulNodeCmdCntAddr, ucNodeCmdCnt - 1);

		// If we have just deleted the last command from the table then delete the NodeID
		if(ucNodeCmdCnt == 1)
			vSRAM_write_B16(ulNodeIndex, 0x00);

		// Clear the address in the metadata
		vSRAM_write_B32(ulCmdIndex, 0x00);

		// Clear the command from the queue
		for (ucCount = 0; ucCount < MAX_MSG_SIZE; ucCount++)
		{
			vSRAM_write_B32(ulAddress + ucCount, 0x00);
		}/* END: for(ucCount) */

		// Decrement the total command count
		vL2SRAM_UpdateTotalCmds(-1);
	}

}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Removes the command from the command queue that was last loaded
//!  			 into the message buffer
//!
//!	\return ucStatus
//!	\param
//////////////////////////////////////////////////////////////////////////////
void ucL2SRAM_Del_current_CMD(void)
{
	ucL2SRAM_Delete_CMD(uiCurrentNodeID, ulCurrentAddr);

}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Fetches a command from memory
//!
//!	\return none
//!	\param ulAddr; the address of the command
//////////////////////////////////////////////////////////////////////////////
void vL2SRAM_FetchCommand(ulong ulAddr)
{
	uchar ucCount;
	for (ucCount = 0; ucCount < MAX_MSG_SIZE; ucCount++)
	{
		// Fetch the entire command, but note that the network layer must be appended afterwards to ensure
		// proper routing.
		ucaMSG_BUFF[ucCount] = ucSRAM_read_B8(ulAddr);
		ulAddr++;
	}

}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Displays the metadata table
//!
//!	\return none
//!	\param none
//////////////////////////////////////////////////////////////////////////////
void vL2SRAM_Display_CmdQueueMetadata(void)
{
	ulong ulRowCnt;
	ulong ulColCnt;
	ulong ulCmdAddr;
	uchar ucCmdCnt;
	ulong ulNodeIDAddr;
	uint uiNodeID;

	// Display the number of commands
	vSERIAL_sout("Total number of commands = ", 27);
	vSERIAL_UI8out(ucL2SRAM_GetTotalCmds());
	vSERIAL_crlf();

	// Display next free location
	vSERIAL_sout("Next free location in memory = ", 31);
	vSERIAL_HB32out(ulSRAM_read_B32(CMD_Q_NFL_ADDR));
	vSERIAL_crlf();

	// Display the format value
	vSERIAL_sout("Formatting indicator = ", 23);
	vSERIAL_bout(ucSRAM_read_B8(CMD_Q_FORMAT_ADDR));
	vSERIAL_bout(ucSRAM_read_B8(CMD_Q_FORMAT_ADDR + 1));
	vSERIAL_bout(ucSRAM_read_B8(CMD_Q_FORMAT_ADDR + 2));
	vSERIAL_bout(ucSRAM_read_B8(CMD_Q_FORMAT_ADDR + 3));
	vSERIAL_crlf();

	vSERIAL_sout("NodeID  NUM   Address 1   Address 2   Address 3   Address 4   Address 5", 71);
	vSERIAL_crlf();

	// Search through the table row by row
	for (ulRowCnt = CMD_Q_FIRST_ROW_ADDR; ulRowCnt < CMD_METADATA_END_ADDR; ulRowCnt += CMD_METADATA_ROW_LEN)
	{

		ulNodeIDAddr = ulRowCnt;

		// Get the node ID and display it
		uiNodeID = uiSRAM_read_B16(ulNodeIDAddr);

		// Don't display empty rows
		if (uiNodeID == 0)
			continue;

		vSERIAL_sout("  ", 2);
		vSERIAL_UI16out(uiNodeID);
		vSERIAL_sout("   ", 3);

		// Get the number of commands pending for a particular node and display it
		ucCmdCnt = ucSRAM_read_B8(ulRowCnt + NODE_ID_LEN);
		vSERIAL_UI8out(ucCmdCnt);
		vSERIAL_sout("   ", 3);
		for (ulColCnt = 0; ulColCnt < NUM_CMDS_PER_NODE; ulColCnt++)
		{
			ulCmdAddr = ulSRAM_read_B32(ulNodeIDAddr + NODE_ID_LEN + NUM_CMDS_PER_NODE_LEN + ulColCnt * CMD_ADDR_LEN);

			vSERIAL_HB32out(ulCmdAddr);
			vSERIAL_sout("    ", 4);
		}
		vSERIAL_crlf();
	}
}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Get a command if it exists
//!
//! \param ucNodeID
//! \return none
//////////////////////////////////////////////////////////////////////////////
uchar ucL2SRAM_LoadCmdIfExists(uint uiNodeID)
{
	ulong ulaCmdArrd[5];
	ulong ulNodeIndex;
	ulong ulNodeCmdCntAddr;
	uchar ucPendingCmds;
	uchar ucOldstCmdIndex;
	uchar ucIndex;

	//Init the command array
	for(ucIndex = 0; ucIndex < 5; ucIndex++){
		ulaCmdArrd[ucIndex] = 0L;
	}

	// Make sure the queue has been formatted before continuing
	if (!ucL2SRAM_IsCmdQueueFormatted())
		return 0;

	// See if there are any commands pending; if not return
	if (!ucL2SRAM_GetTotalCmds())
		return 0;

	// Find the Node in the metadata
	ulNodeIndex = ulL2SRAM_CheckForNode(uiNodeID);

	// If the node isn't in the table then exit
	if (ulNodeIndex == 0)
		return 0;

	// Set the address for the number of commands per node byte
	ulNodeCmdCntAddr = ulNodeIndex + NODE_ID_LEN;

	// Check how many commands are pending for this node; if none then exit
	ucPendingCmds = ucSRAM_read_B8(ulNodeCmdCntAddr);
	if (ucPendingCmds == 0)
		return 0;

	// Get the addresses of the command from the queue
	vL2SRAM_GetCmdAddrs(uiNodeID, ulaCmdArrd);

	// Get the index of the oldest command in the array.
	ucOldstCmdIndex = ucPendingCmds % NUM_CMDS_PER_NODE - 1;

	if(ucOldstCmdIndex == 0xFF)
		ucOldstCmdIndex = 4;

	// Set the last loaded command address and node ID variables.  This is done so that the
	// proper command can be deleted if needed
	ulCurrentAddr = ulaCmdArrd[ucOldstCmdIndex];
	uiCurrentNodeID = uiNodeID;

	// Load the command in the message buffer
	vL2SRAM_FetchCommand(ulaCmdArrd[ucOldstCmdIndex]);

#if 1
	vSERIAL_sout("Cmd ld\r\n", 8);
#endif

	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Tests the command queue to make sure command are properly stored
//! and fetched
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vL2SRAM_TestCmdQueue(void)
{
	uchar ucCount;
	const ulong ulaCmdArrd[5] =
	{ 0 };

	vL2SRAM_FormatCmd_Q();
	vL2SRAM_Display_CmdQueueMetadata();

	for (ucCount = 0; ucCount < 0x20; ucCount++)
	{
		ucaMSG_BUFF[ucCount] = ucCount;
	}

	ucL2SRAM_PutCMD_inQueue(0x0669);
	vL2SRAM_Display_CmdQueueMetadata();
	ucL2SRAM_PutCMD_inQueue(0x0669);
	vL2SRAM_Display_CmdQueueMetadata();
	ucL2SRAM_PutCMD_inQueue(0x0669);
	vL2SRAM_Display_CmdQueueMetadata();
	ucL2SRAM_PutCMD_inQueue(0x0669);
	vL2SRAM_Display_CmdQueueMetadata();
	ucL2SRAM_PutCMD_inQueue(0x0669);
	vL2SRAM_Display_CmdQueueMetadata();
	ucL2SRAM_PutCMD_inQueue(0x0669);
	vL2SRAM_Display_CmdQueueMetadata();
	ucL2SRAM_PutCMD_inQueue(0x0669);
	vL2SRAM_Display_CmdQueueMetadata();

	ucL2SRAM_LoadCmdIfExists(0x0669);
	ucL2SRAM_Del_current_CMD();
	vL2SRAM_Display_CmdQueueMetadata();

	ucL2SRAM_LoadCmdIfExists(0x0669);
	ucL2SRAM_Del_current_CMD();
	vL2SRAM_Display_CmdQueueMetadata();

	ucL2SRAM_LoadCmdIfExists(0x0669);
	ucL2SRAM_Del_current_CMD();
	vL2SRAM_Display_CmdQueueMetadata();

	ucL2SRAM_LoadCmdIfExists(0x0669);
	ucL2SRAM_Del_current_CMD();
	vL2SRAM_Display_CmdQueueMetadata();

	ucL2SRAM_LoadCmdIfExists(0x0669);
	ucL2SRAM_Del_current_CMD();
	vL2SRAM_Display_CmdQueueMetadata();

	ucL2SRAM_LoadCmdIfExists(0x0669);
	ucL2SRAM_Del_current_CMD();
	vL2SRAM_Display_CmdQueueMetadata();

	ucL2SRAM_LoadCmdIfExists(0x0669);
	ucL2SRAM_Del_current_CMD();
	vL2SRAM_Display_CmdQueueMetadata();


	vL2SRAM_GetCmdAddrs(1, (ulong *)ulaCmdArrd);

	for (ucCount = 0; ucCount < 0x20; ucCount++)
	{
		ucaMSG_BUFF[ucCount] = 0;
	}

	vL2SRAM_FetchCommand(ulaCmdArrd[0]);

	ucL2SRAM_Delete_CMD(1, ulaCmdArrd[0]);
	vL2SRAM_Display_CmdQueueMetadata();
	ucL2SRAM_PutCMD_inQueue(2);
	vL2SRAM_Display_CmdQueueMetadata();

	for (ucCount = 0; ucCount < 0x20; ucCount++)
	{
		ucaMSG_BUFF[ucCount] = 0;
	}

}
/*-------------------------------  MODULE END  ------------------------------*/
