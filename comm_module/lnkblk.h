/**************************  LNKBLK.H  ***************************************
 *
 * Header for LNKBLK radio linkup routine pkg
 *
 *
 * V1.00 06/29/2008 wzr
 *	started
 *
 ******************************************************************************/

#ifndef LNKBLK_H_INCLUDED
#define LNKBLK_H_INCLUDED

#define LNKREQ_1FRAME_1LNK  ((1<<3) | (1)) 	//1-Frame out, 1-Link
#define LNKREQ_1FRAME_3LNK  ((1<<3) | (3)) 	//1-Frame out, 3-Link
#define LNKREQ_1FRAME_7LNK  ((1<<3) | (7)) 	//1-Frame out, 7-Link
//Used as LnkReq & LnkConfirm value
#define LNKREQ_2FRAME_1LNK  ((2<<3) | (1)) 	//2-Frame out, 1-Link
//Used as Force value in sched

//! \def ENTRYS_PER_LNKBLK_BLK
//! \brief Constants describing the size of the link block table
#define ENTRYS_PER_LNKBLK_BLK			8
#define	MAX_LINKS	30//0x10
#define MAX_LINKS_PER_FRAME	0x08

#define MAX_LNK_DIST_IN_FRAMES   3				//3-Frames out
#define MAX_LNK_DIST_IN_FRAMES_L ((long)MAX_LNK_DIST_IN_FRAMES)

#define MAX_LNK_DIST_IN_MSEC_L 	(((long)MAX_LNK_DIST_IN_FRAMES_L * 1000) * SECS_PER_FRAME_L)
#define MAX_LNK_DIST_IN_SEC_L 	(MAX_LNK_DIST_IN_MSEC_L / 1000 )
#define MAX_LNKREQ  ((MAX_LNK_DIST_IN_FRAMES<<3) | (1))

#define MIN_LNK_DIST_IN_MS	(((long)SECS_PER_FRAME_L*1000) / ENTRYS_PER_LNKBLK_BLK)
#define MIN_LNK_DIST_IN_SEC_L (MIN_LNK_DIST_IN_MS / 1000)
#define MIN_LNKREQ  ((1<<3) | (ENTRYS_PER_LNKBLK_BLK -1))

#define LNK_MSG_TRANSFER_THRESHOLD    10		//number of msgs 
#define LNK_MSG_TRANSFER_THRESHOLD_L ((long)LNK_MSG_TRANSFER_THRESHOLD)


//! \defgroup Link Block Flags
//! @{
//! \def F_OVERWRITE
//! \brief Indicates the contents of the link block are safe to overwrite
#define F_OVERWRITE		0x01

//! \def F_ALL
//! \brief All the flags in the byte
#define F_ALL	0xFF

#define LNKMNGR_OK	0
#define LNKMNGR_ERR	1
//! @}

#define LINK_GOOD		0
#define LINK_FAIL 	1
/*--------------------------------*/

uchar ucLNKBLK_ReadSingleLnkBlkEntry(uint uiSerialNumber, // Other nodes serial number
		uchar ucLnkBlkEntryNum, //link time index
		ulong * ulLnkTime // pointer to link time
		);

/*--------------------------------*/

uchar ucLNKBLK_AddLink(uint uiSerialNum);
uchar ucLNKBLK_RemoveNode(uint uiSerialNumber);
void vLNKBLK_zeroEntireLnkBlkTbl(void);

/*--------------------------------*/

void vLNKBLK_showSingleLnkBlk(uint uiSerialNumber, //ID of other node
		uchar ucTimeFormFlag, //FRAME_SLOT_TIME,HEX_SEC_TIME,INT_SEC_TIME
		uchar ucShowZroEntryFlag, //YES_SHOW_ZRO_ENTRYS, NO_SHOW_ZRO_ENTRYS
		uchar ucCRLF_flag //YES_CRLF, NO_CRLF
		);
unsigned char ucLNKBLK_FindLinkTime(uint uiSerialNumber,
		ulong ulLinkTime,
		uchar *pucLnkBlkEntryIdx
		);

void vLNKBLK_showAllLnkBlkTblEntrys(void);

/*--------------------------------*/

uchar ucLNKBLK_fillLnkBlkFromMultipleLnkReq(uint uiSerialNumber, //Node ID
		uchar ucMultipleLnkReq, //Lnk Req = 5bits,,3bits => deltaFrame,,delta Link
		long lBaseTime //Base Time to compute the lnks from
		);

/*--------------------------------*/

uchar ucLNKBLK_computeMultipleLnkReqFromSysLoad(uint iSysLoad //system load in msgs/hr
    );

void vLNKBLK_showLnkReq(uchar ucLnkReq);

uchar ucLNKBLK_FetchNumofLinkTimes(uint uiSerialNumber, uchar * pucNumLinks);

uchar ucLNKBLK_CheckForEmptyBlk(uint uiSerialNumber);
//uchar ucLNKBLK_FindEmptyBlk(uchar * ucLnkBlkIdx);

uchar ucLNKBLK_ReadFlags(uint uiSerialNumber, uchar * pucFlags);
uchar ucLNKBLK_SetFlag(uint uiSerialNumber, uchar ucFlag);
uchar ucLNKBLK_ClearFlag(uint uiSerialNumber, uchar ucFlag);
uchar ucLNKBLK_ReadRSSI(uint uiSerialNumber, signed int *piRSSI);
uchar ucLNKBLK_WriteRSSI(uint uiSerialNumber, signed int iRSSI);
uchar ucLNKBLK_ReadPriority(uint uiSerialNumber, uchar *pucPriority);
uchar ucLNKBLK_WritePriority(uint uiSerialNumber, uchar ucPriority);
uchar ucLNKBLK_ReadMsdMsgCount(uint uiSerialNumber, uchar *pucCount);
uchar ucLNKBLK_WriteMsdMsgCount(uint uiSerialNumber, uchar ucCount);
uchar ucLNKBLK_ReadRand(uint uiSerialNumber, ulong * pulRand);
uchar ucLNKBLK_WriteRand(uint uiSerialNumber, ulong ulRand);
uchar ucLNKBLK_WriteLnkState(uint uiSerialNumber, uchar ucLnkBlkEntryNum, uchar ucLinkState);
uchar ucLNKBLK_ReadLnkState(uint uiSerialNumber, uchar ucLnkBlkEntryNum, uchar *pucLinkState);
void vLNKBLK_showLnkStats(void);
uchar ucLNKBLK_AnyGoodLinks(uint uiTempSN);
uchar ucLNKBLK_FindEmptyBlk(uchar * ucLnkBlkIdx);
/*--------------------------------*/

#endif /* LNKBLK_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
