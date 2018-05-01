

/**************************  MODACT.C  ******************************************
*
* Keyboard Routines to change the action table.
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


#include "task.h"			// Task manager
#include "diag.h"			//diagnostic defines
#include "std.h"			//standard definitions
#include "serial.h"			//serial IO port stuff
#include "mem_mod.h"		// memory module
#include "MODACT.h"			//key action change routines
#include "sensor.h"			//Sensor name list
#include "pick.h"			//pick the point to send routines
#include "key.h"			//top level keyboard routines
#include "numcmd.h"			//numbered command handler module
#include "hal/config.h"			//system configuration description file





/*********************  EXTERNS  *********************************************/


extern uchar ucGLOB_StblIdx_NFL;




/*********************  DECLARATIONS  ****************************************/


void vMODACT_showCurTasks(
		void
		);

void vMODACT_showAllPossibleActions(
		void
		);

void vMODACT_silentDeleteAction(	//Del Actn from SRAM and FRAM tables
		uchar ucSchedTblEntryNum	//SRAM table entry num
		);

void vMODACT_silentAddAction(	//Add Actn to SRAM and FRAM tables
		uchar ucNewActionNum	//Action num to add
		);

void vMODACT_silentChangeInterval(
		//uchar ucSchedEntryNum,
		//uint uiInterval
		void
		);

void vMODACT_silentChangeSenseAct(
		uchar ucSchedEntryNum,
		ulong ulSenseActWord
		);

void vMODACT_silentChangeYtrigger(
		uchar ucSensorNum,
		uint uiTrigVal
		);

void vMODACT_showChangeableSenseActTrigs(
		void
		);


/**********************  TABLES  *********************************************/



/* FUNCTION DELCARATIONS */
//     void vKEY_exitFuncPlaceHolder(void);		// 0 Quit
static void vMODACT_showHelp(void);				// 1 Help
static void vMODACT_showActionTbls(void);		// 2 Show Action Tables
static void vMODACT_addAction(void);			// 3 Add Action
static void vMODACT_deleteAction(void);			// 4 Delete Action
static void vMODACT_modifyInterval(void);		// 5 Modify Sample Interval
static void vMODACT_modifySenseAct(void);		// 6 Modify SenseAct
static void vMODACT_modifyYtrigger(void);		// 7 Modify Y Trigger


#define CMD_ARRAY_SIZE 8

/* NOTE: This array is parallel to the Cmd array */
T_Text S_ModActCmdText[CMD_ARRAY_SIZE] =
	{
			{"Quit", 4},							// 0 Quit
			{"Help", 4},							// 1 Help
			{"ShowTbls", 8},					// 2 Show Action Tables
			{"AddActnToLst", 12},			// 3 Add Action
			{"DeleteActnFrmLst", 16},	// 4 Delete Action
			{"ChgSampIntvl", 12},			// 5 Modify Sample Interval
			{"ChgSenseActLst", 14},		// 6 Modify SenseAct
			{"ChgYTrig", 8},						// 7 Modify YTrigger
	}; /* END: cpaModActCmdText[] */


const GENERIC_CMD_FUNC vMODACTfunctionTbl[CMD_ARRAY_SIZE] =
 {
 vKEY_exitFuncPlaceHolder,			// 0 Quit
 vMODACT_showHelp,							// 1 Help
 vMODACT_showActionTbls,				// 2 Show Action Tables
 vMODACT_addAction,							// 3 Add Action
 vMODACT_deleteAction,					// 4 Delete Action
 vMODACT_silentChangeInterval,	// 5 Modify Sample Interval
 vMODACT_modifySenseAct,				// 6 Modify SenseAct
 vMODACT_modifyYtrigger					// 7 Modify YTrigger
 };


/*----------------------------------------------------------------*/


#define ACTION_LIST_SIZE		14

T_Text S_ActionDesc[ACTION_LIST_SIZE] =
	{
			{"Custom", 6},								// 3 Roll your own
			{"Batt", 4},									// 4
			{"TC 1,2", 6},								// 5
			{"TC 3,4", 6},								// 6
			{"Light 1,2", 9},							// 7
			{"SoilMoisture 1,2", 16},						// 8
			{"Light 3,4", 9},							//16
			{"Light 1,2,3,4", 13},						//17
			{"Light 1,1,1,1",	13},				//18
			{"VaisalaAveWindSpeed&Dir", 23},				//19
			{"VaisalaAveAirPress&Temp", 23},	  			//20
			{"VaisalaRelHumid&RainAccum", 25},			//21
			{"SapFlow 1,2", 11},							//22
			{"SapFlow 3,4",  11},							//23

	}; /* END: S_ActionDesc[] */



/* parallel array with cpaActionDescArray[] */
const unsigned char ucaChangeableActions[ACTION_LIST_SIZE] = 
	{
	 3,
	 4,
	 5,
	 6,
	 7,
	 8,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23
	}; /* END: ucaChangeableActions[] */






/********************  DECLARATIONS  *****************************************/

uchar ucMODACT_isChangeableAction(
		uchar ucActionToCheck
		);





/***************************  CODE  ******************************************/


/************************  vMODACT_modifyActionTbl() *************************
*
*  Add,Del,Modify or Show an Action entry
*
******************************************************************************/

void vMODACT_modifyActionTbl(
		void
		)

	{

	vMODACT_showCurTasks();
	vSERIAL_dash(10);
	vSERIAL_crlf();

//	vMODACT_showAllPossibleActions();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_sout("Cmds:\r\n", 7);
	vMODACT_showHelp();


	vNUMCMD_doCmdInputAndDispatch(
				'*',					//prompt
				S_ModActCmdText,			//command array
				vMODACTfunctionTbl,		//dispatch function ptrs
				CMD_ARRAY_SIZE,			//array size
				YES_SHOW_INITIAL_PROMPT
				);

	vL2FRAM_showTSBTbl();
	return;

	}/* END: uchar ucMODACT_modifyActionTbl() */





/*******************  vMODACT_showHelp()  ****************************************
*
*
*
******************************************************************************/
void vMODACT_showHelp(
		void
		)
	{
	vNUMCMD_showCmdList(S_ModActCmdText, CMD_ARRAY_SIZE);
	vSERIAL_crlf();

	return;

	}/* END: vMODACT_showHelp() */




/*******************  vMODACT_addAction()  ****************************************
*
*
*
******************************************************************************/
void vMODACT_addAction(
		void
		)
	{
	char cNewActionNum;

	/* SHOW WHATS OUT THERE */
	vMODACT_showCurTasks();
	vSERIAL_dash(10);
	vSERIAL_crlf();
	vMODACT_showAllPossibleActions();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_sout("EntrActnNumToAdd: ", 18);
//	cNewActionNum = cNUMCMD_getCmdNum(E_ACTN_MAX_NUM_IDX);
	if(cNewActionNum <= 0)
		{
		goto Add_Action_exit;
		}


	/* CHECK IF IT IS A CHANGEABLE ACTION */
	if(!ucMODACT_isChangeableAction((uchar)cNewActionNum))
		{
		vSERIAL_sout("NotLegalAdd\r\n\n", 14);
		goto Add_Action_exit;
		}

	#if 1
	vSERIAL_sout("ActnAddingIs: ", 14);
	vSERIAL_IV8out(cNewActionNum);
	vSERIAL_crlf();
	#endif

	/* ITS VALID -- ADD IT TO THE SRAM TABLE */
	vMODACT_silentAddAction((uchar)cNewActionNum);

	#if 1
	vL2FRAM_showTSBTbl();
	vSERIAL_crlf();
	#endif

	vMODACT_showCurTasks();
	vSERIAL_crlf();

Add_Action_exit:
	return;

	}/* END: vMODACT_addAction() */





/****************  vMODACT_modifyInterval()  *********************************
*
*
*
******************************************************************************/

static void vMODACT_modifyInterval(
		void
		)
	{
//	char cSchedEntryNum;
//	uchar ucActionNum;
//	uint uiInterval;
//
//	/* SHOW WHATS OUT THERE */
//	vMODACT_showCurTasks();
//	vSERIAL_dash(10);
//	vSERIAL_crlf();
//	vSERIAL_sout("EntryID: ", 9);
//	cSchedEntryNum = cNUMCMD_getCmdNum(ucGLOB_StblIdx_NFL);
//	if(cSchedEntryNum <= 0)
//		{
//		goto Interval_change_exit;
//		}
//
//	/* CHECK IF IT IS A CHANGEABLE ACTION */
//	ucActionNum = (uchar)ulL2SRAM_getStblEntry(
//											SCHED_ACTN_TBL_NUM,
//											(uchar)cSchedEntryNum);
//	if(!ucMODACT_isChangeableAction(ucActionNum))
//		{
//		vSERIAL_sout("NotChgableIntrvl\r\n\n", 19);
//		goto Interval_change_exit;
//		}
//
//	vSERIAL_sout("EnterNewIntrvl: ", 16);
//	uiInterval = (uint)ulSERIAL_UI32in();
//
//	if(uiInterval < 120)
//		{
//		vSERIAL_sout("IntrvlTooSmall\r\n\n", 17);
//		goto Interval_change_exit;
//		}
//	if(uiInterval > 14400)
//		{
//		vSERIAL_sout("IntrvlTooBig\r\n\n", 15);
//		goto Interval_change_exit;
//		}
//
//	/* ITS VALID -- REMOVE IT FROM THE SRAM AND FRAM TABLES */
//	vMODACT_silentChangeInterval((uchar)cSchedEntryNum, uiInterval);
//
//	vMODACT_showCurTasks();
//	vSERIAL_crlf();
//
//Interval_change_exit:
//	return;

	}/* END: vMODACT_modifyInterval() */





/****************  vMODACT_modifySenseActl()  *********************************
*
*
*
******************************************************************************/
static void vMODACT_modifySenseAct(
		void
		)
	{
//	char cSchedEntryNum;
//	uchar ucActionNum;
//	ulong ulSenseActWord;
//	uchar ucii;
//	uchar ucjj;
//
//	/* SHOW WHATS OUT THERE */
//	vMODACT_showCurTasks();
//	vSERIAL_dash(10);
//	vSERIAL_crlf();
//
//	vSERIAL_sout("EntryID: ", 9);
//	cSchedEntryNum = cNUMCMD_getCmdNum(ucGLOB_StblIdx_NFL);
//	if(cSchedEntryNum <= 0)
//		{
//		goto ChgSenseAct_exit;
//		}
//
//	/* CHECK IF IT IS A CHANGEABLE ACTION */
//	ucActionNum = (uchar)ulL2SRAM_getStblEntry(
//											SCHED_ACTN_TBL_NUM,
//											(uchar)cSchedEntryNum
//											);
//	if(!ucMODACT_isChangeableAction(ucActionNum))
//		{
//		vSERIAL_sout("Can'tChangeThatSenseAct\r\n\n", 26);
//		goto ChgSenseAct_exit;
//		}
//
//	/* SHOW THE SENSE ACTS POSSIBLE */
//	vSERIAL_sout("SenseActsPossible:\r\n", 20);
//	vSERIAL_sout("ID SNSACT SZ   ID SNSACT SZ   ID SNSACT SZ   ID SNSACT SZ\r\n", 59);
//	for(ucii=0,ucjj=0;  ucii<SENSOR_MAX_VALUE; ucii++) //ucii=sensor#  ucjj=format count
//		{
//		if(ucSENSEACT_getSensorUserFlag(ucii))
//			{
//			if((ucjj % 4) == 0) vSERIAL_crlf();
//			vSERIAL_HB8out(ucii);
//			vSERIAL_bout(' ');
//			vSENSOR_showSensorName(ucii, L_JUSTIFY);
//			vSERIAL_bout(' ');
//			vSERIAL_UIV8out(ucSENSEACT_getSensorDataSize(ucii));
//			if(((ucjj+1) % 4) != 0)  vSERIAL_sout("    ", 4);
//			ucjj++;
//
//			}/* END: if() */
//
//		}/* END: for(ucii) */
//
//	vSERIAL_sout("\r\n\n", 3);
//
//
//	vSERIAL_sout("CombineUpTo4ActnIDs(include 0\'s)asSingleNum,\r\n", 46);
//	vSERIAL_sout("EnterNow: ", 10);
//	ulSenseActWord = ulSERIAL_H32in();
//
//	/* LEFT JUSTIFY THE SENSE ACT WORD */
//	if(ulSenseActWord != 0)
//		{
//		for(ucii=0;  ucii<3;  ucii++)
//			{
//			if(ulSenseActWord & 0xFF000000) break;
//			ulSenseActWord <<= 8;
//			}
//		}
//
//	#if 0
//	vSERIAL_sout("Thats ", 6);
//	vSERIAL_HB32out(ulSenseActWord);
//	vSERIAL_sout(" = ", 3);
//	vSENSEACT_showSenseActWordInText(ulSenseActWord);
//	vSERIAL_crlf();
//	#endif
//
//	/* CORRECT THIS NUMBER */
//	ulSenseActWord = ulSENSEACT_correctSenseActWord(ulSenseActWord);
//
//	#if 0
//	vSERIAL_sout("Correctd=", 9);
//	vSERIAL_HB32out(ulSenseActWord);
//	vSERIAL_sout(" = ", 3);
//	vSENSEACT_showSenseActWordInText(ulSenseActWord);
//	vSERIAL_crlf();
//	#endif
//
//	/* ITS VALID, STUFF THE STBL */
//	vMODACT_silentChangeSenseAct((uchar)cSchedEntryNum, ulSenseActWord);
//
//	vMODACT_showCurTasks();
//	vSERIAL_crlf();
//
//ChgSenseAct_exit:
//	return;

	}/* END: vMODACT_modifySenseAct() */








/****************  vMODACT_modifyYtriggerl()  *********************************
*
*
*
******************************************************************************/
static void vMODACT_modifyYtrigger(
		void
		)
	{
	char cSensorNum;
	uint uiTrigVal;

	vMODACT_showChangeableSenseActTrigs();
	vSERIAL_dash(10);
	vSERIAL_crlf();

	vSERIAL_sout("EntryID: ", 9);
	cSensorNum = cNUMCMD_getCmdNum(SENSOR_MAX_VALUE);
	if(cSensorNum <= 0)
		{
		goto ChgYtrig_exit;
		}

	vSERIAL_sout("EntrTrigVal: ", 13);
	uiTrigVal = (uint)ulSERIAL_UI32in();
	if(uiTrigVal > 32000)
		{
		vSERIAL_sout("TrigTooBig\r\n\n", 13);
		goto ChgYtrig_exit;
		}

	vMODACT_silentChangeYtrigger((uchar)cSensorNum, uiTrigVal);

	vSERIAL_sout("TrigChged\r\n", 11);
	vMODACT_showChangeableSenseActTrigs();
	vSERIAL_crlf();

ChgYtrig_exit:
	return;

	}/* END: vMODACT_modifyYtrigger() */







/*******************  vMODACT_deleteAction()  ****************************************
*
*
*
******************************************************************************/
void vMODACT_deleteAction(
		void
		)
	{
//	char cSchedEntryNum;
//	uchar ucActionNum;
//
//	/* SHOW WHATS OUT THERE */
//	vMODACT_showCurTasks();
//
//	vSERIAL_sout("EnterActnNumToDel: ", 19);
//	cSchedEntryNum = cNUMCMD_getCmdNum(ucGLOB_StblIdx_NFL);
//	if(cSchedEntryNum <= 0)
//		{
//		goto Remove_Action_exit;
//		}
//
//	/* CHECK IF IT IS A CHANGEABLE ACTION */
//	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, (uchar)cSchedEntryNum);
//	if(!ucMODACT_isChangeableAction(ucActionNum))
//		{
//		vSERIAL_sout("NotLegalDelete\r\n\n", 17);
//		goto Remove_Action_exit;
//		}
//
//	/* ITS VALID -- REMOVE IT FROM THE SRAM AND FRAM TABLES */
//	vMODACT_silentDeleteAction((uchar)cSchedEntryNum);
//
//	#if 1
//	vL2FRAM_showTSBTbl();
//	vSERIAL_crlf();
//	#endif
//
//	vMODACT_showCurTasks();
//	vSERIAL_crlf();
//
//
//Remove_Action_exit:
//	return;

	}/* END: vMODACT_deleteAction() */





/*******************  vMODACT_showAllPossibleActions()  ****************************************
*
*
*
******************************************************************************/

void vMODACT_showAllPossibleActions(
		void
		)
	{
	uchar ucii;
	uchar ucTskIndex;

	vSERIAL_sout("ModifiableActns:\r\n", 18);

	for(ucii=0; ucii<ACTION_LIST_SIZE;  ucii++)
		{
		ucTskIndex = ucaChangeableActions[ucii];
		vSERIAL_UI8out(ucTskIndex);
		vSERIAL_sout(" - ", 3);
		vTask_showTaskName(ucTskIndex);
		vSERIAL_sout(" - ", 3);
		vSERIAL_sout(S_ActionDesc[ucii].m_cText, S_ActionDesc[ucii].m_uiLength);
		vSERIAL_crlf();
		}

	return;

	}/* END: vMODACT_showAllPossibleActions() */





/*******************  vMODACT_showCurTasks()  ****************************************
*
*
*
******************************************************************************/

void vMODACT_showCurTasks(
		void
		)
	{
	vSERIAL_sout("CurTasks:\r\n", 11);
	vTask_ShowAllTasks();

	return;

	}/* END: vMODACT_showCurTasks() */






/*******************  ucMODACT_isChangeableAction()  **************************
*
* Check if an action is on the changeable list
*
* RET 1 - yes
*	  0 - no
*
******************************************************************************/

uchar ucMODACT_isChangeableAction(
		uchar ucActionToCheck
		)
	{
	uchar ucii;

	for(ucii=0;  ucii<ACTION_LIST_SIZE;  ucii++)
		{
		if(ucActionToCheck == ucaChangeableActions[ucii]) return(1);
		}
	return(0);

	}/* END: ucMODACT_isChangeableAction() */





/*******************  vMODACT_silentDeleteAction()  ********************************
*
* Delete an action from the SRAM tables and the FRAM tables
*
******************************************************************************/
void vMODACT_silentDeleteAction(			//Del Actn from SRAM and FRAM tables
		uchar ucStblEntryNum	//SRAM entry num
		)
	{
//	uchar ucStBlkNum;
//	uchar ucActionNum;
//
//
//	/* CHECK THE NUMBER FOR A CHANGEABLE ACTION */
//	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucStblEntryNum);
//	if(!ucMODACT_isChangeableAction(ucActionNum)) return;
//
//	/* NOW FIND THE THE FRAM ST TBL ENTRY */
//	ucStBlkNum = ucL2FRAM_findStBlkTask(ucActionNum);	//find actn in FRAM
//
//	/* REMOVE THE ENTRY FROM THE SCHED TBL */
//	vSTBL_deleteStblEntry(ucStblEntryNum);
//
//	#if 1
//	vSERIAL_sout("DelingSramEntry ", 16);
//	vSERIAL_UIV8out(ucStblEntryNum);
//	vSERIAL_crlf();
//	vSERIAL_sout("AlsoDelingFramStBlk ", 20);
//	vSERIAL_UIV8out(ucStBlkNum);
//	vSERIAL_crlf();
//	#endif
//
//	/* REMOVE THE ENTRY FROM THE FRAM TBL */
//	if(ucStBlkNum != 255) vL2FRAM_deleteStBlk(ucStBlkNum);	//remove from FRAM
//
//	return;

	}/* END: vMODACT_silentDeleteAction() */






/*******************  vMODACT_silentAddAction()  *****************************
*
* Add an action from the SRAM tables and the FRAM tables
*
* NOTE: If the system is out of StBlks then no action will be added.
*
******************************************************************************/

void vMODACT_silentAddAction(	//Add Actn to SRAM and FRAM tables
		uchar ucNewActionNum	//Action num to add
		)
	{
//	uchar ucStBlkNum;
//
//	/* MAKE SURE THIS IS A CHANGEABLE ACTION */
//	if(!ucMODACT_isChangeableAction(ucNewActionNum)) return;
//
//	/* NOW ADD THE FRAM STARTUP ENTRY */
//	ucStBlkNum = ucL2FRAM_addRuntimeStBlk(ucNewActionNum);
//	if(ucStBlkNum == 0) return; //wups no StBlks are available.
//
//
//	/* NOW ADD THE SRAM ENTRY TO THE SCHED TBL */
//	vSTBL_addNewActionToStbl(ucNewActionNum, ucStBlkNum, 12);
//
//	return;

	}/* END: vMODACT_silentAddAction() */




/*********************  vMODACT_silentChangeInterval()  ***********************
//! \brief Allows the sampling interval of changeable tasks to be altered via
//! the text interface. This change will be reflected in the task list in
//! SRAM as well as in the FRAM start block so that the changes will persist
//! through a system restart.
//!
//! \param ucTaskIndex: Index into task list, uiInterval: new interval
//! \return none
******************************************************************************/

void vMODACT_silentChangeInterval(
		//uchar ucTaskIndex,
		//uint uiInterval
		void
		)
	{
	//uchar ucActionNum;
	uchar ucTaskIndex;
	uint uiInterval;
	ulong ulNewLoadFactor;

	// Get task id from user
	vSERIAL_sout("GetTaskIdx: ", 12);
	ucTaskIndex = (uint)ulSERIAL_UI32in();

	// Show task name
	vSERIAL_sout("\r\nTask Name: ", 13);
	vTask_showTaskName(ucTaskIndex);
	vSERIAL_sout("\r\n", 2);

	// Get new interval from user
	vSERIAL_sout("EnterNewIntrvl: ", 16);
	uiInterval = (uint)ulSERIAL_UI32in();

	/* CHECK THE NUMBER FOR A VALID ENTRY */
	if(ucTaskIndex >= TASKPARTITION) return;

	/* CHECK IF IT IS A CHANGEABLE ACTION */
	//ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucTaskIndex);
	//if(!ucMODACT_isChangeableAction(ucActionNum)) return;
	if(ucTask_VerifyChangeableCommand(ucTaskIndex) != CHANGE_PERMISSION_YES)
		return;

	/* RANGE CHECK INTERVAL */
	if(uiInterval < 5)
	{
		vSERIAL_sout("Intrvl out of bounds", 20);
		return;
	}
	if(uiInterval > 14400)
	{
		vSERIAL_sout("Intrvl out of bounds", 20);
		return;
	}

	// Update the interval if allowed
	if (ucTask_SetField(ucTaskIndex, PARAM_INTERVAL, (ulong)uiInterval) == TASKMNGR_OK)
	{
		// Calculate new task load factor
		ulNewLoadFactor = 3600 / uiInterval;
		ucTask_SetField(ucTaskIndex, PARAM_LFACT, ulNewLoadFactor);
	}

	return;
//	uchar ucActionNum;
//	uchar ucStBlkNum;
//
//	/* CHECK THE NUMBER FOR A VALID ENTRY */
//	if(ucTaskIndex >= ucGLOB_StblIdx_NFL) return;
//
//	/* CHECK IF IT IS A CHANGEABLE ACTION */
//	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucTaskIndex);
//	if(!ucMODACT_isChangeableAction(ucActionNum)) return;
//
//	/* RANGE CHECK INTERVAL */
//	if(uiInterval < 128) return;
//	if(uiInterval > 14400) return;
//
//	/* ITS GOOD -- STUFF A NEW SRAM VALUE */
//	vTask_SetInterval(ucTaskIndex, uiInterval);
//
//	/* STUFF THE NEW VALUE INTO THE START TABLE */
//	ucStBlkNum = ucL2FRAM_findStBlkTask(ucActionNum);
//	vL2FRAM_putStBlkEntryVal(ucStBlkNum, FRAM_ST_BLK_INTERVAL_IDX, (ulong)uiInterval);
//
//	return;

	}/* END: vMODACT_silentChangeInterval() */






/*********************  vMODACT_silentChangeSenseAct()  ***********************
*
*
*
******************************************************************************/

void vMODACT_silentChangeSenseAct(
		uchar ucSchedEntryNum,
		ulong ulSenseActWord
		)
	{
//	uchar ucActionNum;
//	uchar ucStBlkNum;
//
//	/* CHECK THE SCHED IDX NUMBER FOR A VALID ENTRY */
//	if(ucSchedEntryNum >= ucGLOB_StblIdx_NFL) return;
//
//	/* CHECK IF IT IS A CHANGEABLE ACTION */
//	ucActionNum = (uchar)ulL2SRAM_getStblEntry(SCHED_ACTN_TBL_NUM, ucSchedEntryNum);
//	if(!ucMODACT_isChangeableAction(ucActionNum)) return;
//
//	/* CORRECT THIS NUMBER */
//	ulSenseActWord = ulSENSEACT_correctSenseActWord(ulSenseActWord);
//
//	#if 0
//	vSERIAL_sout("Corrected_2=", 12);
//	vSERIAL_HB32out(ulSenseActWord);
//	vSERIAL_sout(" = ", 3);
//	vSENSEACT_showSenseActWordInText(ulSenseActWord);
//	vSERIAL_crlf();
//	#endif
//
////	/* ITS GOOD -- STUFF A NEW SRAM VALUE */
////	vL2SRAM_putStblEntry(SCHED_SENSE_ACT_TBL_NUM,	//tbl num
////						ucSchedEntryNum,			//tbl idx
////						ulSenseActWord				//value to stuff
////						);
//
//	/* STUFF THE NEW VALUE INTO THE START TABLE */
//	ucStBlkNum = ucL2FRAM_findStBlkTask(ucActionNum);
//	vL2FRAM_putStBlkEntryVal(ucStBlkNum,
//							FRAM_ST_BLK_SENSOR_ID_IDX,
//							ulSenseActWord
//							);
//
//	return;

	}/* END: vMODACT_silentChangeSenseAct() */






/*********************  vMODACT_silentChangeYtrigger()  ***********************
*
*
*
******************************************************************************/

void vMODACT_silentChangeYtrigger(
		uchar ucSensorNum,
		uint uiTrigVal
		)
	{

	/* CHECK FOR VALID ID NUMBER */
	if(ucSensorNum >= SENSOR_MAX_VALUE) return;
	//if(ucSENSEACT_getSensorUserFlag(ucSensorNum) == 0) return;

	/* CHECK FOR VALID TRIGGER VAL */
	if(uiTrigVal > 32000) return;

	/* STUFF THE SSP TRIG TBL ENTRY */
	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, ucSensorNum, (ulong)uiTrigVal);

	/* STUFF THE FRAM TBL ENTRY */
	vL2FRAM_putYtriggerVal(ucSensorNum, uiTrigVal);

	return;

	}/* END: vMODACT_silentChangeYtrigger() */





/*********************  vMODACT_showChangeableSenseActTrigs()  ***************
*
*
*
******************************************************************************/

void vMODACT_showChangeableSenseActTrigs(
		void
		)
	{
	uchar ucii;
	uchar ucjj;

	/* SHOW THE CHANGEABLE SENSEACT TRIGGERS */
	vSERIAL_sout("SenseActTrigs:\r\n", 16);
	vSERIAL_sout("ID SNSACT  TRIG    ID SNSACT  TRIG    ID SNSACT  TRIG    ID SNSACT  TRIG\r\n", 74);
	for(ucii=0,ucjj=0;  ucii<SENSOR_MAX_VALUE; ucii++) //ucii=sensor#  ucjj=format count
		{
		//if(ucSENSEACT_getSensorUserFlag(ucii))
			//{
			if((ucjj % 4) == 0) vSERIAL_crlf();
			vSERIAL_HB8out(ucii);
			vSERIAL_bout(' ');
			vSENSOR_showSensorName(ucii, L_JUSTIFY);
			vSERIAL_bout(' ');
			vSERIAL_UI16out((uint)ulPICK_getSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, ucii));
			if(((ucjj+1) % 4) != 0)  vSERIAL_sout("    ", 4);
			ucjj++;

			//}/* END: if() */

		}/* END: for(ucii) */

	vSERIAL_crlf();

	return;

	}/* END: vMODACT_showChangeableSenseActTrigs() */






/*******************  vMODACT_showActionTbls()  ****************************
*
*
*
******************************************************************************/

void vMODACT_showActionTbls(
		void
		)
	{
	vSERIAL_sout("CurActns:\r\n", 11);
	//vSTBL_showBriefStblEntrys();

	return;

	}/* END: vMODACT_showActionTbls() */





/*-------------------------------  MODULE END  ------------------------------*/
