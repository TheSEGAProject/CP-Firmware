

/**************************  GID.C  ******************************************
*
* Group selector and Group ID routines
*
* V1.00 03/19/2005 wzr
*		Started
*
******************************************************************************/



/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "std.h"			//standard defines
#include "hal/config.h" 		//configuration values
#include "serial.H" 		//serial port handler
#include "gid.h"			//serial number handler
#include "misc.h"			//misc utility routines
#include "rand.h"			//random number routines



extern uint uiGLOB_grpID;			//group ID for this group




/*****************************  CODE  ****************************************/

/*************************** vGID_init() *************************************
*
*
*****************************************************************************/
void vGID_init(
		void
		)
	{
	uint uiThisGrpSelector;

	uiThisGrpSelector = (0x00 & GID_SELECTOR_MASK);

	uiGLOB_grpID  = (uint)uslRAND_getRolledFullSysSeed();
	uiGLOB_grpID &= GID_SELECTOR_NOT_MASK;
	uiGLOB_grpID |= uiThisGrpSelector;

	return;

	}/* END: vGID_init() */



/***********************  uiGID_getWholeSysGidAsUint()  *****************************
*
*
*****************************************************************************/

uint uiGID_getWholeSysGidAsUint(
		void
		)
	{

	return(uiGLOB_grpID);

	}/* END: uiGID_getWholeSysGidAsUint() */





/***********************  ucGID_getWholeSysGidLoByte()  *************************
*
*
*****************************************************************************/

uchar ucGID_getWholeSysGidLoByte(
		void
		)
	{

	return((uchar)uiGLOB_grpID);

	}/* END: ucGID_getWholeSysGidLoByte() */






/***********************  ucGID_getWholeSysGidHiByte()  *****************************
*
*
*****************************************************************************/

uchar ucGID_getWholeSysGidHiByte(
		void
		)
	{

	return((uchar)(uiGLOB_grpID>>8));

	}/* END: ucGID_getWholeSysGidHiByte() */






/***********************  ucGID_getSysGrpSelectAsByte()  *****************************
*
*
*****************************************************************************/

uchar ucGID_getSysGrpSelectAsByte(
		void
		)
	{

	return((uchar)((uiGLOB_grpID & GID_SELECTOR_MASK)>>8));

	}/* END: ucGID_getSysGrpSelectAsByte() */






/*****************  vGID_copyWholeSysGidToBytes()  **************************
*
*
*****************************************************************************/

void vGID_copyWholeSysGidToBytes(
		uchar *ucpToPtr
		)
	{

	vMISC_copyUintIntoBytes(uiGLOB_grpID, ucpToPtr, NO_NOINT);

	return;

	}/* END: vGID_copyWholeSysGidToBytes() */






/**************** vGID_setWholeSysGidFromBytes() **************************
*
*
*****************************************************************************/

void vGID_setWholeSysGidFromBytes(
		uchar *ucpFromPtr
		)
	{
	uiGLOB_grpID = uiMISC_buildUintFromBytes((uchar *)ucpFromPtr, NO_NOINT);

	return;

	}/* END: vGID_setWholeSysGidFromBytes() */






/********************** ucGID_compareWholeSysGidToBytes() ************************
*
* RET:	1 = match
*		0 = no match
*
*****************************************************************************/

uchar ucGID_compareWholeSysGidToBytes( //RET: 1=match, 0=no match
		uchar *ucpComparePtr,			//ptr to Foreign GID
		uchar ucShowErrorFlag,			//YES_SHOW_ERR, NO_SHOW_ERR
		uchar ucCRLF_flag				//YES_CRLF, NO_CRLF
		)
	{
	uint uiCompareGid;
	uiCompareGid = uiMISC_buildUintFromBytes((uchar *)ucpComparePtr, NO_NOINT);
	if(uiCompareGid != uiGLOB_grpID)
		{
		if(ucShowErrorFlag)
			{
			vSERIAL_sout("1-GidMchNeeded ", 15);
			vSERIAL_HB16out(uiGLOB_grpID);
			vSERIAL_sout(" got ", 5);
			vSERIAL_HB16out(uiCompareGid);
			if(ucCRLF_flag) vSERIAL_crlf();
			}
		return(0);
		}

	return(1);

	}/* END: ucGID_compareWholeSysGidToBytes() */





/********************** ucGID_compareOnlySysGrpSelectToBytes() ************************
*
* RET:	1 = match
*		0 = no match
*
*****************************************************************************/

uchar ucGID_compareOnlySysGrpSelectToBytes( //RET: 1=match, 0=no match
		uchar *ucpComparePtr,			//ptr for foreign GID
		uchar ucShowErrorFlag,			//YES_SHOW_ERR, NO_SHOW_ERR
		uchar ucCRLF_flag				//YES_CRLF, NO_CRLF
		)
	{
	uint uiCompareSelector;
	uint uiSysSelector;

	uiCompareSelector = 
	  uiMISC_buildUintFromBytes(ucpComparePtr, NO_NOINT) & GID_SELECTOR_MASK;

	uiSysSelector = uiGLOB_grpID & GID_SELECTOR_MASK;

	if(uiCompareSelector != uiSysSelector)
		{
		if(ucShowErrorFlag)
			{
			vSERIAL_sout("GidSelectMchNeeded ", 19);
			vSERIAL_HB16out(uiSysSelector);
			vSERIAL_sout(" got ", 5);
			vSERIAL_HB16out(uiCompareSelector);
			if(ucCRLF_flag) vSERIAL_crlf();
			}
		return(0);
		}

	return(1);

	}/* END: ucGID_compareOnlySysGrpSelectToBytes() */





/******************* ucGID_compareOnlySysGidToBytes() ************************
*
* RET:	1 = match
*		0 = no match
*
*****************************************************************************/

uchar ucGID_compareOnlySysGidToBytes( //RET: 1=match, 0=no match
		uchar *ucpComparePtr,			//ptr for foreign GID
		uchar ucShowErrorFlag,			//YES_SHOW_ERR, NO_SHOW_ERR
		uchar ucCRLF_flag				//YES_CRLF, NO_CRLF
		)
	{
	uint uiCompareGid;
	uint uiSysGid;

	uiCompareGid = 
		uiMISC_buildUintFromBytes((uchar *)ucpComparePtr, NO_NOINT) & GID_SELECTOR_NOT_MASK;

	uiSysGid = uiGLOB_grpID & GID_SELECTOR_NOT_MASK;

	if(uiCompareGid != uiSysGid)
		{
		if(ucShowErrorFlag)
			{
			vSERIAL_sout("2-GidMchNeeded ", 15);
			vSERIAL_HB16out(uiSysGid);
			vSERIAL_sout(" got ", 5);
			vSERIAL_HB16out(uiCompareGid);
			if(ucCRLF_flag) vSERIAL_crlf();
			}
		return(0);
		}

	return(1);

	}/* END: ucGID_compareOnlySysGidToBytes() */





/* --------------------------  END of MODULE  ------------------------------- */
