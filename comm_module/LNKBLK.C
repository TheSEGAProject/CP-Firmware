/**************************  LNKBLK.C  *****************************************
 *
 * Routines to handle the LNKBLK radio link values
 *
 * The LnkBlk table below is a table of blocks that are 8 bytes long and each
 * block is a 4 byte long used to hold the time of the next link.  There are
 * 8 links in a single frame possible hence the 8 entries.
 * Values in the link block table are only valid until the comm. task is scheduled.
 * After scheduling the block is cleared.
 *
 *
 * V1.00 06/29/2008 wzr
 *		Started
 *
 ******************************************************************************/

/*lint -e526 *//* function not defined */
/*lint -e657 *//* unusual (nonportable) anonymous struct or union */
/*lint -e714 *//* symbol not referenced */
/*lint -e716 *//* while(1) */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
/*lint -e757 *//* global declarator not referenced */
///*lint -e752 */		/* local declarator not referenced */
/*lint -e758 *//* global union not referenced */
/*lint -e768 *//* global struct member not referenced */

#include "task.h"
#include "std.h"			//standard defines
#include "serial.h" 		//serial IO port stuff
#include "time.h"			//Time routines
#include "LNKBLK.h"			//	Real Time Scheduler routines
#include "comm.h"			//	radio msg helper routines
#include "rand.h"			//	Random number generator
#include "config.h" 	//	system configuration description file
#include "mem_mod.h"	// memory module

extern uint uiGLOB_lostROM2connections; //counts lost ROM2's
extern uint uiGLOB_lostSOM2connections; //counts lost SOM2's

extern uint uiGLOB_ROM2attempts; //count number of ROM2 attempts
extern uint uiGLOB_SOM2attempts; //count number of SOM2 attempts

extern uint uiGLOB_TotalSDC4trys; //counts number of SDC4 attempts
extern uint uiGLOB_TotalRTJ_attempts; //counts number of request to join attempts

//! \struct S_Link
//! \brief The link block table, storing the times of future communications and flags
struct
{
		uchar m_ucFlags;
		uchar m_ucPriority;
		uchar m_ucMsdMsgCount;
		uchar m_ucLinkReq;
		signed int m_iRSSI;
		uint m_uiSerialNumber;
		ulong m_ulRand;
		ulong m_ulBlock[ENTRYS_PER_LNKBLK_BLK];
		unsigned char m_ucLinkState[ENTRYS_PER_LNKBLK_BLK];
} S_Link[MAX_LINKS];

/*************************  DECLARATIONS  *************************************/

/*****************************  CODE STARTS HERE  ****************************/

///////////////////////////////////////////////////////////////////////////////
//! \brief Finds the link block index for the desired node
//!
//! \param uiSerialNumber
//! \return Error code; 0 for success
///////////////////////////////////////////////////////////////////////////////
unsigned char ucLNKBLK_GetLinkIndex(uint uiSerialNumber, uchar *pucLnkBlkIdx)
{
	uchar ucLnkBlkIdx;

	// Loop through to find the matching node
	for (ucLnkBlkIdx = 0; ucLnkBlkIdx < MAX_LINKS; ucLnkBlkIdx++) {

		// If we find the node then assign the index to the pointer
		if (uiSerialNumber == S_Link[ucLnkBlkIdx].m_uiSerialNumber) {
			*pucLnkBlkIdx = ucLnkBlkIdx;
			return LNKMNGR_OK;
		}
	}

	return LNKMNGR_ERR;
} // END: cLNKBLK_GetLinkIndex()

//////////////////////  lLNKBLK_readSingleLnkBlkEntry()  //////////////////////
//! \brief Reads an entry from the link block table
//!
//!
//! \param uiSerialNumber, ucLnkBlkEntryNum, pulLnkTime
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ReadSingleLnkBlkEntry(uint uiSerialNumber, uchar ucLnkBlkEntryNum, ulong *pulLnkTime)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}
	else {
		*pulLnkTime = S_Link[ucLblkIndex].m_ulBlock[ucLnkBlkEntryNum];
	}

	return LNKMNGR_OK;
}/* END: lLNKBLK_readSingleLnkBlkEntry() */

////////////////// vLNKBLK_writeSingleLnkBlkEntry() //////////////////////////
//! \brief Writes an entry to the link block table
//!
//!
//! \param uiSerialNumber, ucLnkBlkEntryNum, lLnkBlkVal
//! \return none
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_writeSingleLnkBlkEntry(uint uiSerialNumber, //Node ID
		uchar ucLnkBlkEntryNum, //entry number in the LnkBlk
		long lLnkBlkVal //value to write into the LnkBlk
		)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	S_Link[ucLblkIndex].m_ulBlock[ucLnkBlkEntryNum] = lLnkBlkVal;

	return LNKMNGR_OK;

}/* END: vLNKBLK_writeSingleLnkBlkEntry() */

//////////////////////  lLNKBLK_readLnkBlkFlags()  //////////////////////
//! \brief Reads flags from the link block table
//!
//! \param uiSerialNumber,  pucFlags
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ReadFlags(uint uiSerialNumber, uchar * pucFlags)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}
	else {
		*pucFlags = S_Link[ucLblkIndex].m_ucFlags;
	}

	return LNKMNGR_OK;
}/* END: uiLNKBLK_readLnkBlkNodeId() */

//////////////////////  lLNKBLK_writeLnkBlkFlags()  //////////////////////
//! \brief Writes a flag to the link block table
//!
//!
//! \param uiSerialNumber, ucFlag
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_SetFlag(uint uiSerialNumber, uchar ucFlag)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	S_Link[ucLblkIndex].m_ucFlags |= ucFlag;

	return LNKMNGR_OK;
}/* END: vLNKBLK_SetFlag() */

//////////////////////////////////////////////////////////////////////////////
//!	\brief Clears the flags from the link block table
//!
//! \param ucTskIndex, ucFlag
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ClearFlag(uint uiSerialNumber, uchar ucFlag)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	// Clear the node ID
	S_Link[ucLblkIndex].m_ucFlags &= ~ucFlag;

	return LNKMNGR_OK;
} // END: vLNKBLK_ClearFlag()

//////////////////////////////////////////////////////////////////////////////
//! \brief Reads link priority from the link block table
//!
//! The link priority is a way of determining which link is in greater need
//! of communication given that there is a scheduling conflict
//!
//! \param uiSerialNumber, pucPriority
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ReadPriority(uint uiSerialNumber, uchar *pucPriority)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	*pucPriority = S_Link[ucLblkIndex].m_ucPriority;

	return LNKMNGR_OK;
}/* END: ucLNKBLK_ReadRSSI() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Writes the link priority to the link block table
//!
//! The link priority is a way of determining which link is in greater need
//! of communication given that there is a scheduling conflict
//!
//! \param uiSerialNumber, iRSSI
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_WritePriority(uint uiSerialNumber, uchar ucPriority)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	S_Link[ucLblkIndex].m_ucPriority = ucPriority;

	return LNKMNGR_OK;
}/* END: ucLNKBLK_WriteRSSI() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Reads RSSI from the link block table
//!
//!
//! \param uiSerialNumber, puiRSSI
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ReadRSSI(uint uiSerialNumber, signed int *piRSSI)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	*piRSSI = S_Link[ucLblkIndex].m_iRSSI;

	return LNKMNGR_OK;
}/* END: iLNKBLK_ReadRSSI() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Writes the RSSI to the link block table
//!
//!
//! \param uiSerialNumber, iRSSI
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_WriteRSSI(uint uiSerialNumber, signed int iRSSI)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	S_Link[ucLblkIndex].m_iRSSI = iRSSI;

	return LNKMNGR_OK;
}/* END: vLNKBLK_WriteRSSI() */


//////////////////////////////////////////////////////////////////////////////
//! \brief Reads missed message count from the link block structure
//!
//!
//! \param uiSerialNumber, pucCount
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ReadMsdMsgCount(uint uiSerialNumber, uchar *pucCount)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	*pucCount = S_Link[ucLblkIndex].m_ucMsdMsgCount;

	return LNKMNGR_OK;
}/* END: iLNKBLK_ReadRSSI() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Writes the missed message count to the link block structure
//!
//!
//! \param uiSerialNumber, ucCount
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_WriteMsdMsgCount(uint uiSerialNumber, uchar ucCount)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	S_Link[ucLblkIndex].m_ucMsdMsgCount = ucCount;

	return LNKMNGR_OK;
}/* END: vLNKBLK_WriteRSSI() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Writes the RSSI to the link block table
//!
//!
//! \param ucTskIndex, pulRand
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ReadRand(uint uiSerialNumber, ulong * pulRand)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	*pulRand = S_Link[ucLblkIndex].m_ulRand;

	return LNKMNGR_OK;
}/* END: ulLNKBLK_ReadRand() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Writes the random number to the link block table
//!
//!
//! \param uiSerialNumber, ulRand
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_WriteRand(uint uiSerialNumber, ulong ulRand)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	S_Link[ucLblkIndex].m_ulRand = ulRand;

	return LNKMNGR_OK;
}/* END: vLNKBLK_WriteRand() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Reads the link state
//!
//! Link state is used to indicate if a link was or is expected to be succesful
//! For example if a parent node cannot schedule a communication slot with a
//! child node then the link state will be set to unsuccesful.
//!
//! \param uiSerialNumber, ucLnkBlkEntryNum, pucLinkState
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ReadLnkState(uint uiSerialNumber, uchar ucLnkBlkEntryNum, uchar *pucLinkState)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	*pucLinkState = S_Link[ucLblkIndex].m_ucLinkState[ucLnkBlkEntryNum];

	return LNKMNGR_OK;
}/* END: ucLNKBLK_ReadLnkState() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Writes the link state
//!
//! Link state is used to indicate if a link was or is expected to be succesful
//! For example if a parent node cannot schedule a communication slot with a
//! child node then the link state will be set to unsuccesful.
//!
//! \param ucTskIndex, ucLnkBlkEntryNum, ucLinkState
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_WriteLnkState(uint uiSerialNumber, uchar ucLnkBlkEntryNum, uchar ucLinkState)
{
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	// Write the value to the link block table
	S_Link[ucLblkIndex].m_ucLinkState[ucLnkBlkEntryNum] = ucLinkState;

	return LNKMNGR_OK;
}/* END: ucLNKBLK_WriteLnkState() */

//////////////////////////////////////////////////////////////////////////////
//! \brief Reads the link request byte from a child
//!
//! \param uiSerialNumber
//! \param ucMultipleLnkReq
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_ReadLnkReq(uint uiSerialNumber, uchar *pucLinkReq){

	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	*pucLinkReq = S_Link[ucLblkIndex].m_ucLinkReq;

	return LNKMNGR_OK;
}

//////////////////////////////////////////////////////////////////////////////
//! \brief Stores the link request byte from a child
//!
//! \param uiSerialNumber
//! \param ucMultipleLnkReq
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_WriteLnkReq(uiSerialNumber, ucMultipleLnkReq){

	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	// Write the value to the link block table
	S_Link[ucLblkIndex].m_ucLinkReq = ucMultipleLnkReq;

	return LNKMNGR_OK;
}

////////////////////////////////////////////////////////////////
//!
//! \brief Finds an available link block
//!
//!	\param none
//! \return error code
////////////////////////////////////////////////////////////////
uchar ucLNKBLK_FindEmptyBlk(uchar * ucLnkBlkIdx)
{
	uchar ucIndex;

	// Initialize return to invalid value
	*ucLnkBlkIdx = 0xFF;

	for (ucIndex = 0; ucIndex < MAX_LINKS; ucIndex++) {
		if (S_Link[ucIndex].m_uiSerialNumber == 0xFFFF) {
			*ucLnkBlkIdx = ucIndex;
			return LNKMNGR_OK;
		}
	}

	// No open spots
#if 0
	vSERIAL_sout("No Available Link Blocks\r\n", 26);
#endif
	return LNKMNGR_ERR;
}

//////////////////////////////////////////////////////////////////////////////
//! \brief Searches the link block table for a link time
//!
//!
//! \param uiSerialNumber, ulLinkTime
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
unsigned char ucLNKBLK_FindLinkTime(uint uiSerialNumber,
		ulong ulLinkTime,
		uchar *pucLnkBlkEntryIdx
		)
{
	uchar ucLblkIndex;
	uchar ucLnkBlkEntryIdx;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	// Loop through all the entries for this node to find a match
	for (ucLnkBlkEntryIdx = 0; ucLnkBlkEntryIdx < ENTRYS_PER_LNKBLK_BLK; ucLnkBlkEntryIdx++) {
		if (ulLinkTime == S_Link[ucLblkIndex].m_ulBlock[ucLnkBlkEntryIdx]) {
			*pucLnkBlkEntryIdx = ucLnkBlkEntryIdx;
			return LNKMNGR_OK;
		}
	}


	// If we are here then there was not a matching link time
	return LNKMNGR_ERR;
}/* END: ucLNKBLK_FindLinkTime() */

////////////////////////  vLNKBLK_clrSingleLnkBlk()  /////////////////////////
//! \brief  Zeros a single link Blk of 8 bytes.
//!
//! \param uiSerialNumber: The task number accessing the link block
//! \return Error code; 0LNKMNGR_OKfor success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_clrSingleLnkBlk(uint uiSerialNumber)
{
	uchar ucjj;
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	// initialize counter
	ucjj = ENTRYS_PER_LNKBLK_BLK;

	/* LOOP HORIZONTAL ACROSS ALL ENTRYS IN A BLK */
	while (ucjj) {
		ucjj--;
		ucLNKBLK_writeSingleLnkBlkEntry(uiSerialNumber, ucjj, 0L);
	}/* END: for(ucjj) */

	// Clear the link block flags
	ucLNKBLK_ClearFlag(uiSerialNumber, F_ALL);

	return LNKMNGR_OK;

}/* END: vLNKBLK_clrSingleLnkBlk() */

//////////////////////  vLNKBLK_zeroEntireLnkBlkTbl(()  //////////////////////
//!	\brief Zeros all entries in the link block table
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vLNKBLK_zeroEntireLnkBlkTbl(void)
{
	uchar ucii;
	uchar ucjj;

	/* LOOP VERTICAL FOR ALL THE ENTRYS IN THE LNKBLK TBL */

	for (ucjj = 0; ucjj < MAX_LINKS; ucjj++) {
		S_Link[ucjj].m_iRSSI = 0;
		S_Link[ucjj].m_ucFlags = 0;
		S_Link[ucjj].m_uiSerialNumber = 0xFFFF;
		S_Link[ucjj].m_ulRand = 0L;
		for (ucii = 0; ucii < ENTRYS_PER_LNKBLK_BLK; ucii++) {
			S_Link[ucjj].m_ulBlock[ucii] = 0;
			S_Link[ucjj].m_ucLinkState[ucii] = LINK_GOOD;
		}
	}/* END: for(ucii) */

}/* END: vLNKBLK_zeroEntireLnkBlkTbl() */

//////////////////////////////////////////////////////////////////////////////
//! \brief  Adds a node to the link block
//!
//! \param uiSerialNumber
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_AddLink(uint uiSerialNum)
{
	uchar ucLnkBlkIdx;
	uchar ucii;

	// Search for an open link block, return error if there isn't one
	if(ucLNKBLK_FindEmptyBlk(&ucLnkBlkIdx) != LNKMNGR_OK)
		return LNKMNGR_ERR;

	// Add the node serial number to a link block
	S_Link[ucLnkBlkIdx].m_uiSerialNumber = uiSerialNum;
	S_Link[ucLnkBlkIdx].m_ucMsdMsgCount = 0;
	S_Link[ucLnkBlkIdx].m_ucPriority = 0;
	S_Link[ucLnkBlkIdx].m_iRSSI = 0;

	// Clear the link times and states
	for (ucii = 0; ucii < ENTRYS_PER_LNKBLK_BLK; ucii++) {
		S_Link[ucLnkBlkIdx].m_ulBlock[ucii] = 0;
		S_Link[ucLnkBlkIdx].m_ucLinkState[ucii] = LINK_GOOD;
	}

	return LNKMNGR_OK;

}

////////////////////////  ucLNKBLK_RemoveNode()  /////////////////////////
//! \brief  Remove a node from the link block
//!
//! \param uiSerialNumber
//! \return Error code; 0 for success
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_RemoveNode(uint uiSerialNumber)
{
	uchar ucjj;
	uchar ucLblkIndex;

	// Search for this node in the link block table, if it fails then return
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0) {
		return LNKMNGR_ERR;
	}

	// initialize counter
	ucjj = ENTRYS_PER_LNKBLK_BLK;

	/* LOOP HORIZONTAL ACROSS ALL ENTRYS IN A BLK */
	while (ucjj) {
		ucjj--;
		ucLNKBLK_writeSingleLnkBlkEntry(uiSerialNumber, ucjj, 0L);
		ucLNKBLK_WriteLnkState(uiSerialNumber, ucjj, 0);

	}/* END: for(ucjj) */

	// Clear the link block flags, RSSI, and the Rand
	ucLNKBLK_ClearFlag(uiSerialNumber, F_ALL);
	ucLNKBLK_WriteRSSI(uiSerialNumber, 0);
	ucLNKBLK_WriteRand(uiSerialNumber, 0L);

	// Finally clear the serial number
	S_Link[ucLblkIndex].m_uiSerialNumber = 0xFFFF;

	return LNKMNGR_OK;

}/* END: vLNKBLK_RemoveNode() */

///////////////////////  vLNKBLK_showSingleLnkBlk()  //////////////////////////
//!	\brief Show all entries in a single LnkBlk
//!
//! \param uiSerialNumber, ucTimeFormFlag, ucShowZroEntryFlag, ucCRLF_flag
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vLNKBLK_showSingleLnkBlk(uint uiSerialNumber, //ID of other node
		uchar ucTimeFormFlag, //FRAME_SLOT_TIME,HEX_SEC_TIME,INT_SEC_TIME
		uchar ucShowZroEntryFlag, //YES_SHOW_ZRO_ENTRYS, NO_SHOW_ZRO_ENTRYS
		uchar ucCRLF_flag //YES_CRLF, NO_CRLF
		)
{
	uchar ucjj;
	uchar ucLblkIndex;
	ulong ulTime;

	// If the desired node is not in the link table then exit
	if (ucLNKBLK_GetLinkIndex(uiSerialNumber, &ucLblkIndex) != 0)
		return;

	/* CHECK IF WE ARE DUMPING ZERO ENTRYS */
	ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, 0, &ulTime);
	if ((!ucShowZroEntryFlag) && (ulTime == 0L))
		return;

	// Show the serial number
	vSERIAL_sout("Serial Num   Flags   Priority   MsdMsgs   RSSI   Rand\r\n", 55);
	vSERIAL_UI8out(uiSerialNumber);
	vSERIAL_sout("        ", 8);
	vSERIAL_UI8out(S_Link[ucLblkIndex].m_ucFlags);
	vSERIAL_sout("     ", 5);
	vSERIAL_UI8out(S_Link[ucLblkIndex].m_ucPriority);
	vSERIAL_sout("        ", 8);
	vSERIAL_UI8out(S_Link[ucLblkIndex].m_ucMsdMsgCount);
	vSERIAL_sout("    ", 4);
	vSERIAL_I16out(S_Link[ucLblkIndex].m_iRSSI);
	vSERIAL_sout("     ", 5);
	vSERIAL_UI32out(S_Link[ucLblkIndex].m_ulRand);
	vSERIAL_crlf();

	/* LOOP HORIZONTAL FOR ALL TABLE ENTRYS */
	for (ucjj = 0; ucjj < ENTRYS_PER_LNKBLK_BLK; ucjj++) {
		ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, ucjj, &ulTime);
		if ((!ucShowZroEntryFlag) && (ulTime == 0L))
			break;

		vTIME_showTime((long)ulTime, ucTimeFormFlag, NO_CRLF);
		vSERIAL_sout("[", 1);
		vSERIAL_HB8out(S_Link[ucLblkIndex].m_ucLinkState[ucjj]);
		vSERIAL_sout("]", 1);
		if (ucjj != ENTRYS_PER_LNKBLK_BLK - 1) {
			vSERIAL_sout(", ", 2);
		}

	}/* END: for(ucjj) */

	if (ucCRLF_flag)
		vSERIAL_crlf();

	return;

}/* END: vLNKBLK_showSingleLnkBlk() */

/**********************  vLNKBLK_showAllLnkBlkTblEntrys()  *******************
 *
 * Dumps all LnkBlk entries
 *
 ******************************************************************************/

void vLNKBLK_showAllLnkBlkTblEntrys(void)
{
	uchar ucii;

	/* LOOP VERTICAL FOR ALL LINK BLOCKS */
	for (ucii = 0; ucii < MAX_LINKS; ucii++) {

		// If there is a valid link in the table then display it
		if (S_Link[ucii].m_uiSerialNumber != 0xFFFF) {


			vLNKBLK_showSingleLnkBlk(S_Link[ucii].m_uiSerialNumber, //Node ID
					FRAME_SLOT_TIME, //Time Form
					YES_SHOW_ZRO_ENTRYS, //Show Zero Entrys Flag
					YES_CRLF //end with CR-LF
					);
		}
	}/* END: for(ucii) */

	return;

}/* END: vLNKBLK_showAllLnkBlkTblEntrys() */

///////////////////////////////////////////////////////////////////////////////
//! \fn ucLNKBLK_CheckLinkConflict
//!
//! \brief Check for a specific link time
//!
//! \param uiSerialNumber, lLnkTime
//! \return 1 if the link time already exists
////////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_CheckLinkConflict(uint uiSerialNumber, ulong ulLnkTime)
{
	uchar ucLnkBlkEntryNum;
	ulong ulTempLinkTime;

	for (ucLnkBlkEntryNum = 0; ucLnkBlkEntryNum < ENTRYS_PER_LNKBLK_BLK; ucLnkBlkEntryNum++) {
		ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, ucLnkBlkEntryNum, &ulTempLinkTime);
		if (ulTempLinkTime == ulLnkTime) {
			return 1;
		}
	}

	// No match
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! \fn ucLNKBLK_AnyGoodLinks
//!
//! \brief Checks to see if there are any good links remaining for a node
//!
//! \param uiSerialNumber
//! \return 1 if a good link exists else 0
////////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_AnyGoodLinks(uint uiSerialNumber)
{
	uchar ucLinkState, ucLnkBlkEntryNum;
	ulong ulLinkTime;

	for (ucLnkBlkEntryNum = 0; ucLnkBlkEntryNum < ENTRYS_PER_LNKBLK_BLK; ucLnkBlkEntryNum++) {
		ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, ucLnkBlkEntryNum, &ulLinkTime);
		ucLNKBLK_ReadLnkState(uiSerialNumber, ucLnkBlkEntryNum, &ucLinkState);

		// If there is an existing link and it is good then return true
		if (ucLinkState == LINK_GOOD && ulLinkTime != 0) {
			return 1;
		}
	}

	// No match
	return 0;
}

/**********************  lLNKBLK_computeTimeForSingleLnk()  *************************
 *
 * Ret: Time For the next link = (BaseTime + DeltaFrame + RandSlot)
 *
 ******************************************************************************/
uchar ucLNKBLK_computeTimeForSingleLnk(uint uiSerialNumber, //Task list index of the lnk
		uchar ucSpecificLnkReq, //Link Req = 5bits,,3bits => deltaFrame,,delta Link
		long lBaseTime, //Base time to compute the next lnk from
		ulong * pulLinkTime // Pointer to be assigned the result
	)
{
	uchar ucc;
	uchar ucEndLnkNum;
	usl uslSavedSeed;
	long lFrameCount;
	long lReqSlot;
	ulong ulLnkStartTime;

	/* UNPACK THE LINK REQUEST */
	lFrameCount = (long) (ucSpecificLnkReq >> 3);
	ucEndLnkNum = (ucSpecificLnkReq & 0x07); //0000 0111

#if 0
	vSERIAL_sout("lFrameCount= ", 13);
	vSERIAL_IV32out(lFrameCount);
	vSERIAL_crlf();
	vSERIAL_sout("ucEndLnkNum= ", 13);
	vSERIAL_UIV8out(ucEndLnkNum);
	vSERIAL_crlf();
#endif

	// Get the random number from the task manager
	if(ucLNKBLK_ReadRand(uiSerialNumber, &uslSavedSeed) != 0)
		return LNKMNGR_ERR;

#if 0
	vSERIAL_sout("First Seed val= ", 26);
	vSERIAL_HB24out(uslSavedSeed);
	vSERIAL_crlf();
#endif

	/* ROLL THE SEED FOR THE NUMBER OF FRAMES INTO THE FUTURE */
	for (ucc = 0; ucc < (uchar) lFrameCount; ucc++) {
		/* ROLL THE SEED FOR THE NUMBER OF FRAMES IN THE FUTURE */
		uslSavedSeed = uslRAND_getRolledFullForeignSeed(uslSavedSeed);

#if 0
		vSERIAL_sout("Frame loop Seed[", 16);
		vSERIAL_UIV8out(ucc);
		vSERIAL_sout("] val= ", 7);
		vSERIAL_HB24out(uslSavedSeed);
		vSERIAL_crlf();
#endif

	}/* END: for() */

	/* ROLL THE SEED FOR THE LINK THAT IS DESIRED  */
	for (ucc = 0; ucc < ucEndLnkNum; ucc++) {

		/* ROLL THE SEED FOR THE NUMBER OF FRAMES IN THE FUTURE */
		uslSavedSeed = uslRAND_getRolledFullForeignSeed(uslSavedSeed);

#if 0
		vSERIAL_sout("LnkLoopSeed[", 12);
		vSERIAL_UIV8out(ucc);
		vSERIAL_sout("] val= ", 7);
		vSERIAL_HB24out(uslSavedSeed);
		vSERIAL_crlf();
#endif

	}/* END: for() */

#if 0
	vSERIAL_sout("EndSeedVal= ", 12);
	vSERIAL_HB24out(uslSavedSeed);
	vSERIAL_crlf();
#endif

	/* COMPUTE SLOT NUMBER FROM THE SEED */
	lReqSlot = (long) ((uslSavedSeed >> 8) & GENERIC_NST_MAX_IDX_MASK);

	// Subtract one from the last GENERIC_NST_LAST_IDX since the last slot is reserved for scheduling
	if (lReqSlot > (GENERIC_NST_LAST_IDX - 1)) {
		// Make sure that the requested slot falls within the allowable range of NST indices
		do {
			/* ROLL THE SEED FOR THE NUMBER OF FRAMES IN THE FUTURE */
			uslSavedSeed = uslRAND_getRolledFullForeignSeed(uslSavedSeed);

			/* COMPUTE SLOT NUMBER FROM THE SEED */
			lReqSlot = (long) ((uslSavedSeed >> 8) & GENERIC_NST_MAX_IDX_MASK);
		}
		while (lReqSlot > (GENERIC_NST_LAST_IDX - 1));
	}

#if 0
	vSERIAL_sout("ReqSlt= ", 8);
	vSERIAL_HB32out((ulong) lReqSlot);
	vSERIAL_sout("  * 4= ", 7);
	vSERIAL_HB32out((ulong) (lReqSlot * 4));
	vSERIAL_crlf();
#endif

	/* COMPUTE THE FRAME START TIME (IN SLOTS) */

	/*----------------------  NOTE:  -----------------------------------------
	 *
	 * The following computation puts the intermediate products into the
	 * variable ulLnkStartTime to save stack space.  So the ulLnkStartTime is
	 * a multiple use variable.
	 *
	 *------------------------------------------------------------------------*/

	/* COMPUTE THE BASETIME-FRAME-0-NUM */
	ulLnkStartTime = lTIME_getFrameNumFromTime(lBaseTime);

	/* CONVERT THE BASETIME-FRAME-0-NUM TO A LINEAR SLOT NUMBER */
	ulLnkStartTime *= SLOTS_PER_FRAME_L;

#if 0
	vSERIAL_sout("LnkBegTim(part1)= ", 18);
	vSERIAL_HB32out((ulong)(ulLnkStartTime * 4L));
	vSERIAL_sout("  =  ", 5);
	vTIME_ShowTime((ulLnkStartTime * 4L),HEX_SEC_TIME,NO_CRLF);
	vSERIAL_sout("   (Frame base time)", 20);
	vSERIAL_crlf();
#endif

	/* ADD LNKREQ DELTA FRAME COUNT TO BASETIME TO GET LNKUP-FRAME-0-NUM */
	ulLnkStartTime += (lFrameCount * SLOTS_PER_FRAME_L); //+delta frames (in slots)

#if 0
	vSERIAL_sout("LnkBegTim(part2)= ", 18);
	vSERIAL_HB32out((ulong)(ulLnkStartTime * 4L));
	vSERIAL_sout("  =  ", 5);
	vTIME_ShowTime((ulLnkStartTime * 4L),HEX_SEC_TIME,NO_CRLF);
	vSERIAL_sout("   (Added frame count)", 22);
	vSERIAL_crlf();
#endif

	/* ADD RANDOM DELTA SLOT CNT TO LNKUP-FRAME-0-NUM TO GET LNKUP-SLOT-NUM */
	ulLnkStartTime += lReqSlot; //+delta slots (in slots)

#if 0
	vSERIAL_sout("LnkBegTim(part3)= ", 18);
	vSERIAL_HB32out((ulong)(ulLnkStartTime * 4L));
	vSERIAL_sout("  =  ", 5);
	vTIME_ShowTime((ulLnkStartTime * 4L),HEX_SEC_TIME,NO_CRLF);
	vSERIAL_sout("   (Added Slot count)", 21);
	vSERIAL_crlf();
#endif

	/* MULTIPLY LNKUP-SLOT-NUM BY SECS_PER_SLOT TO GET LNKUP-TIME (IN SECS) */
	ulLnkStartTime *= SECS_PER_SLOT_L; //convert to seconds

#if 0
	vSERIAL_sout("NxtLnk= ", 8);
	vTIME_showTime(ulLnkStartTime,FRAME_SLOT_TIME,YES_CRLF);
#endif

	// Assign the result to the pointer
	*pulLinkTime = ulLnkStartTime;

	return LNKMNGR_OK;

}/* END: lLNKBLK_computeTimeForSingleLnk() */

/*****************  vLNKBLK_fillLnkBlkFromMultipleLnkReq()  ******************
 *
 * Fills the Link Blk with Link Times determined from the Link Req
 *
 ******************************************************************************/
uchar ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uint uiSerialNumber, //Task list index
		uchar ucMultipleLnkReq, //Lnk Req = 5bits,,3bits => deltaFrame,,delta Link
		long lBaseTime //Base Time to compute the lnks from
		)
{
	uchar ucc;
	uchar ucUnshiftedFrameCount;
	uchar ucEndLnkNum;
	uchar ucLnkSpecificReq;
	ulong ulLnkTime;

	/* UNPACK THE LINK REQUEST */
	ucUnshiftedFrameCount = ucMultipleLnkReq & 0xF8; // 1111 1000
	ucEndLnkNum = ucMultipleLnkReq & 0x07; // 0000 0111

#if 0
	vSERIAL_sout("lUnshiftedFrameCount= ", 22);
	vSERIAL_UI8out(ucUnshiftedFrameCount);
	vSERIAL_crlf();
	vSERIAL_sout("ucLnkCount= ", 12);
	vSERIAL_UIV8out(ucEndLnkNum);
	vSERIAL_crlf();
#endif

	// Clears link times and flags
	ucLNKBLK_clrSingleLnkBlk(uiSerialNumber);

	// Store the link request
	ucLNKBLK_WriteLnkReq(uiSerialNumber, ucMultipleLnkReq);

	/* LOOP FOR ALL REQUESTS */
	for (ucc = 0; ucc < ucEndLnkNum; ucc++) {

		/* BUILD THE LINK-SPECIFIC-REQ */
		ucLnkSpecificReq = ucUnshiftedFrameCount | (ucc + 1);

		/* GET THE SPECIFIC LINK REQ TURNED INTO TIME */
		if (ucLNKBLK_computeTimeForSingleLnk(uiSerialNumber, ucLnkSpecificReq, lBaseTime, &ulLnkTime) != 0)
			return LNKMNGR_ERR;

		while (ucLNKBLK_CheckLinkConflict(uiSerialNumber, ulLnkTime)) {
			ulLnkTime+=10;
		}

		/* STASH THE LINK TIME IN THE LNKBLK TBL */
		ucLNKBLK_writeSingleLnkBlkEntry(uiSerialNumber, ucc, ulLnkTime);
		ucLNKBLK_WriteLnkState(uiSerialNumber, ucc, LINK_GOOD);
	}/* END: for(ucc) */

#if 0
		vLNKBLK_showSingleLnkBlk(uiSerialNumber, //System Tbl Num
				FRAME_SLOT_TIME,//Time Form
				YES_SHOW_ZRO_ENTRYS,//Show Zero Entrys Flag
				YES_CRLF//end with CR-LF
			);
#endif

	// Clear the overwrite flag, indicating that this block should not be written to until scheduled
	ucLNKBLK_ClearFlag(uiSerialNumber, F_OVERWRITE);

	return LNKMNGR_OK;
}/* END: ucLNKBLK_fillLnkBlkFromMultipleLnkReq() */

/****************  vLNKBLK_showLnkReq()  *************************************
 *
 *
 *
 *****************************************************************************/
void vLNKBLK_showLnkReq(uchar ucLnkReq)
{

	vSERIAL_UIV8out(ucLnkReq >> 3);
	vSERIAL_bout(':');
	vSERIAL_UIV8out(ucLnkReq & 0x7);

	return;

}/* END: vLNKBLK_showLnkReq() */

//////////////////////////////////////////////////////////////////////////////
//!
//! \brief Compute a Multiple link Request from the system load factor
//!
//!
//!
//! \param uiSysLoad, the load on the system
//!	\return ucLnkReq, the link byte
//////////////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_computeMultipleLnkReqFromSysLoad(uint uiSysLoad //system load in msgs/hr
		)
{
	uchar ucLnkReq;
	ulong ulSysLoad;
	ulong ulMsgDistance_in_ms;
	ulong ulLnkDistance_in_ms;

	/* CONVERT THE SYSTEM LOAD TO A LONG */
	ulSysLoad = ((long) uiSysLoad);

	// Determine a period of message generation (3600 seconds x 1000 ms/s) .  Actual messages aren't generated this
	// regularly, but it can serve as an estimation of activity during a frame.
	ulMsgDistance_in_ms = ((3600000L) / ulSysLoad);

	// Determine the time between links given the message period
	ulLnkDistance_in_ms = ulMsgDistance_in_ms * MAX_OM_MSGS;

#if 0

	vSERIAL_sout("System Load: ", 13);
	vSERIAL_HB32out(ulSysLoad);
	vSERIAL_crlf();

	vSERIAL_sout("Req Link Distance: ", 19);
	vSERIAL_HB32out(ulLnkDistance_in_ms);
	vSERIAL_crlf();

	vSERIAL_sout("Min Link Distance: ", 19);
	vSERIAL_HB32out(MIN_LNK_DIST_IN_MS);
	vSERIAL_crlf();

	vSERIAL_sout("Max Link Distance: ", 19);
	vSERIAL_HB32out(MAX_LNK_DIST_IN_MSEC_L);
	vSERIAL_crlf();
#endif

	// If the link distance is less than the minimum allowable distance between
	// then assign the maximum link request
	if (ulLnkDistance_in_ms <= MIN_LNK_DIST_IN_MS) {
		ucLnkReq = MIN_LNKREQ;
	}
	// If the link distance is greater than the maximum allowable distance between links
	// then assign the minimum link request
	else if (ulLnkDistance_in_ms > MAX_LNK_DIST_IN_MSEC_L) {
		ucLnkReq = MAX_LNKREQ;
	}
	// Otherwise compute how many links should occur in the next slot
	else {
		// In the likely event that the link request time is a whole number + a fraction
		// we must either rely on overestimates of the system load or apply some padding here.
		// For now I am doing both by adding an additional slot for comm. to the link request
		ucLnkReq = MSECS_PER_FRAME_L / ulLnkDistance_in_ms;

		// Ensure the link byte is within range
		if (ucLnkReq > 7)
			ucLnkReq = 7;
		if (ucLnkReq == 0)
			ucLnkReq = 1;

		ucLnkReq |= (1 << 3);
	}
	
	// If we have a large backlog of messages then force the maximum link req
	if(uiL2SRAM_getMsgCount() > (uint)(MAX_OM_MSGS*7)){
		ucLnkReq = MIN_LNKREQ;
	}\

#if 0
	vSERIAL_sout("LkRq=", 5);
	vLNKBLK_showLnkReq(ucLnkReq);
	vSERIAL_crlf();
#endif

	return (ucLnkReq);

}/* END: ucLNKBLK_computeMultipleLnkReqFromSysLoad() */

/////////////////////////////////////////////////////////////////////
//!
//! \brief Determines the number of communication slots there are in a
//!  frame for a particular node.
//!
//! Note: the following code does not verify that the task is a operational
//! mode communication task therefore that verification must be done before
//! hand.
//!
//! \param ucTskIndex
//! \return ucNumLinks
/////////////////////////////////////////////////////////////////////
uchar ucLNKBLK_FetchNumofLinkTimes(uint uiSerialNumber, uchar * pucNumLinks)
{
	uchar ucIndex;
	uchar ucNumLinks;
	ulong ulLinkTime;

	// Assume that there are no links in the frame
	ucNumLinks = 0;
	for (ucIndex = 0; ucIndex < ENTRYS_PER_LNKBLK_BLK; ucIndex++) {
		if (ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, ucIndex, &ulLinkTime) != 0)
			return LNKMNGR_ERR;

			// increment the link count
			if (ulLinkTime != 0)
				ucNumLinks++;
	}

	// Assign link count to pointer
	*pucNumLinks = ucNumLinks;

	return LNKMNGR_OK;

}

////////////////////////////////////////////////////////////////
//!
//! \brief Checks to see if the link block table is empty
//!
//!	\param
//!	\return 1 if empty
////////////////////////////////////////////////////////////////
uchar ucLNKBLK_CheckForEmptyBlk(uint uiSerialNumber)
{
	ulong ulLinkTime;

	// Fetch the time of the first link in the block
	ucLNKBLK_ReadSingleLnkBlkEntry(uiSerialNumber, 0, &ulLinkTime);

	// If the first link is zero then the block is empty
	if (ulLinkTime == 0)
		return 1;
	else
		return 0;
}


/////////////////////////////////////////////////////////////////
//!\fn vLNKBLK_showLnkStats()
//!\brief Prints the global link information to the terminal
//!
//!\param none
//!\return none
////////////////////////////////////////////////////////////////
void vLNKBLK_showLnkStats(void)
{

	/* SHOW LOST ROM2'S CONNECTIONS */
	vSERIAL_sout("ROM2Lost=", 9);
	vSERIAL_UI16out(uiGLOB_lostROM2connections);
	/* SHOW TOTAL ROM2'S ATTEMPTED LINKUPS */
	vSERIAL_sout(", ROM2Trys=", 11);
	vSERIAL_UI16out(uiGLOB_ROM2attempts);
	/* SHOW SDC4 DISCOVERY ATTEMPTS */
	vSERIAL_sout(", SDC4Trys=", 11);
	vSERIAL_UI16out(uiGLOB_TotalSDC4trys);
	vSERIAL_crlf();

	/* SHOW LOST SOM2'S CONNECTIONS */
	vSERIAL_sout("SOM2Lost=", 9);
	vSERIAL_UI16out(uiGLOB_lostSOM2connections);
	/* SHOW TOTAL SOM2'S ATTEMPTED LINKUPS */
	vSERIAL_sout(", SOM2Trys=", 11);
	vSERIAL_UI16out(uiGLOB_SOM2attempts);
	/* SHOW RDC4 DISCOVERY ATTEMPTS */
	vSERIAL_sout(", RDC4Trys=", 11);
	vSERIAL_UI16out(uiGLOB_TotalRTJ_attempts);
	vSERIAL_crlf();

	vSERIAL_crlf();

	return;

}/* END: vLNKBLK_showLnkStats() */

/*
 /////////////////////////////////////////////////////////////////
 //!\fn vLNKBLK_TestSlotRandomocity()
 //!\brief Used to test the distribution of slot numbers selected by
 //! the LFSR and scheduling algorithm.
 //!
 //!\param none
 //!\return none
 ////////////////////////////////////////////////////////////////
 void vLNKBLK_TestSlotRandomocity(void) //yeah its a word
 {
 uchar ucMsgIndex;
 static usl uslSavedSeed;
 ulong lReqSlot;

 if (ucFirstRun ==1){
 uslSavedSeed = uslRAND_getFullSysSeed();
 vSERIAL_HB32out(uslSavedSeed);
 vSERIAL_crlf();
 }
 ucFirstRun = 0;


 // ROLL THE SEED FOR THE NUMBER OF FRAMES IN THE FUTURE
 uslSavedSeed = uslRAND_getRolledFullForeignSeed(uslSavedSeed);

 // COMPUTE SLOT NUMBER FROM THE SEED
 lReqSlot = (long) ((uslSavedSeed >> 8) & GENERIC_NST_MAX_IDX_MASK);

 if (lReqSlot > GENERIC_NST_LAST_IDX) {
 // Make sure that the requested slot falls within the allowable range of NST indices
 do {
 // ROLL THE SEED FOR THE NUMBER OF FRAMES IN THE FUTURE
 uslSavedSeed = uslRAND_getRolledFullForeignSeed(uslSavedSeed);

 // COMPUTE SLOT NUMBER FROM THE SEED
 lReqSlot = (long) ((uslSavedSeed >> 8) & GENERIC_NST_MAX_IDX_MASK);
 }
 while (lReqSlot > GENERIC_NST_LAST_IDX);
 }


 // Build a DE
 vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
 ucMsgIndex = DE_IDX_RPT_PAYLOAD;

 ucaMSG_BUFF[ucMsgIndex++] = 6; //Report ID
 ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
 ucaMSG_BUFF[ucMsgIndex++] = 0; // leading 0
 ucaMSG_BUFF[ucMsgIndex++] = (uchar) lReqSlot;

 // Store DE
 vReport_WriteDEToRAM();

 // Build messages from the DEs generated during the slot
 vReport_BuildMsgsFromDEs();

 // Send report to GS
 vGS_ReportToGardenServer();

 }
 */

/*-------------------------------  MODULE END  ------------------------------*/
