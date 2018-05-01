

/**************************  MODSTORE.C  ******************************************
*
* Keyboard Routines to change the Flash Storage tables.
*
*
* V1.00 01/09/2006 wzr
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


#include "diag.h"			//diagnostic defines
#include "std.h"			//standard definitions
#include "hal/config.h"			//system configuration description file
#include "serial.h"			//serial IO port stuff
#include "mem_mod/mem_mod.h"			//Memory module routines
#include "MODSTORE.h"			//key action change routines
#include "key.h"			//Top Level Key commands
#include "numcmd.h"			//numbered command handler module

/*********************  EXTERNS  *********************************************/



/*********************  DECLARATIONS  ****************************************/



/**********************  TABLES  *********************************************/



/* FUNCTION DELCARATIONS */
//     void vKEY_exitFuncPlaceHolder(void);
static void vMODSTORE_showHelp(void);
static void vMODSTORE_showTables(void);
static void vMODSTORE_deleteOldFlashMsgs(void);
static void vMODSTORE_windbackFlashPtrs(void);
static void vMODSTORE_zeroTheFlash(void);


#define CMD_ARRAY_SIZE 6

/* NOTE: This array is parallel to the Cmd array */
T_Text S_ModStorCmdText[CMD_ARRAY_SIZE] =
	{
			{"Quit", 4},										// 0 Quit
			{"Help Msg", 8},								// 1 Help
			{"ShowTbls", 8},								// 2 Show Store Tables
			{"DelOldFlash Msgs", 16},				// 3 Delete old flash
			{"WindbackFlash Ptrs", 18},			// 4 Windback Flash ptrs
			{"ZeroFlash", 9},								// 5 Zero the Flash
	}; /* END: cpaModStorCmdText[] */


const GENERIC_CMD_FUNC vMODSTOREfunctionTbl[CMD_ARRAY_SIZE] =
 {
 vKEY_exitFuncPlaceHolder,			// 0 Quit
 vMODSTORE_showHelp,				// 1 Help
 vMODSTORE_showTables,				// 2 Show Store Tables
 vMODSTORE_deleteOldFlashMsgs,		// 3 Delete Old Msgs
 vMODSTORE_windbackFlashPtrs,		// 4 Windback Flash Ptrs
 vMODSTORE_zeroTheFlash				// 5 Zero the Flash
 };




/********************  DECLARATIONS  *****************************************/





/***************************  CODE  ******************************************/


/************************  vMODSTORE_modifyStorageTbls() *************************
*
*  Modify the Flash Storage Pointers
*
******************************************************************************/

void vMODSTORE_modifyStorageTbls(
		void
		)

	{

	vKEY_showFlashMsgCounts();
	vSERIAL_crlf();


	vMODSTORE_showHelp();


	vNUMCMD_doCmdInputAndDispatch(
				'*',					//prompt
				S_ModStorCmdText,			//command array
				vMODSTOREfunctionTbl,		//dispatch function ptrs
				CMD_ARRAY_SIZE,			//array size
				YES_SHOW_INITIAL_PROMPT
				);

	return;

	}/* END: uchar ucMODSTORE_modifyStorageTbls() */






/*******************  vMODSTORE_showHelp()  ****************************************
*
*
*
******************************************************************************/
void vMODSTORE_showHelp(
		void
		)
	{
	vNUMCMD_showCmdList(S_ModStorCmdText, CMD_ARRAY_SIZE);
	vSERIAL_crlf();

	return;

	}/* END: vMODSTORE_showHelp() */






/*******************  vMODSTORE_showTables()  ****************************************
*
*
*
******************************************************************************/
void vMODSTORE_showTables(
		void
		)
	{
	vKEY_showFlashMsgCounts();
	return;

	}/* END: vMODSTORE_showTables() */






/*******************  vMODSTORE_deleteOldFlashMsgs()  ************************
*
*
*
******************************************************************************/
void vMODSTORE_deleteOldFlashMsgs(
		void
		)
	{
	vKEY_showFlashMsgCounts();

	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG)) return;

	//vL2FRAM_expungeAllUploadedFlashMsgs();
	vKEY_showFlashMsgCounts();

	return;

	}/* END: vMODSTORE_deleteOldFlashMsgs() */





/*****************  vMODSTORE_windbackFlashPtrs()  ********************************
*
*
*
******************************************************************************/
void vMODSTORE_windbackFlashPtrs(
		void
		)
	{
	vKEY_showFlashMsgCounts();

	if(ucSERIAL_confirm(YES_SHOW_ABORT_MSG))
		{
		//vL2FRAM_undeleteAllUploadedFlashMsgs();
		vSERIAL_sout("WindbkDone\r\n", 12);
		vKEY_showFlashMsgCounts();
		}

	return;

	}/* END: vMODSTORE_windbackFlashPtrs() */






/*******************  vMODSTORE_zeroTheFlash()  ******************************
*
*
*
******************************************************************************/
void vMODSTORE_zeroTheFlash(
		void
		)
	{
	vKEY_showFlashMsgCounts();

	if(!ucSERIAL_confirm(YES_SHOW_ABORT_MSG)) return;

	//vL2FRAM_initFramFlashPtrs();

	vKEY_showFlashMsgCounts();

	return;

	}/* END: vMODSTORE_zeroTheFlash() */


/*-------------------------------  MODULE END  ------------------------------*/
