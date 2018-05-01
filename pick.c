

/**************************  PICK.C  *****************************************
*
* Routines to perform point PICKs during events
*
*
* V1.00 12/24/2006 wzr
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
/*lint -e752 */		/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "std.h"			//standard defines
#include "config.h" 	//system configuration description file
#include "misc.h"			//homeless functions
#include "serial.h"  	//serial IO port stuff
#include "mem_mod.h" 	//memory module routines
#include "comm.h"			//msg handling routines
#include "sensor.h"		//Sensor name routines
#include "pick.h"			//point pick reporting handler
#include "task.h"			// Task manager module

/********************  DEFINES  *********************************************/




/********************  EXTERNS  *********************************************/

extern volatile uchar ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];


/********************  TABLES  **********************************************/

const usl uslaSSP_BaseAddr[LAST_SSP_TBL_NUM_PLUS_1] =
 {
 SSP_Y_TBL_BASE_ADDR,				//Tbl 0
 SSP_T_TBL_BASE_ADDR,				//Tbl 1
 SSP_DELTA_Y_TRIG_TBL_BASE_ADDR,	//Tbl 2
 SSP_DELTA_T_TRIG_TBL_BASE_ADDR		//Tbl 3
 };

const usl uslaSSP_MaxAddr[LAST_SSP_TBL_NUM_PLUS_1] =
 {
 SSP_Y_TBL_END_ADDR_PLUS_1 - 1,					//Tbl 0
 SSP_T_TBL_END_ADDR_PLUS_1 - 1,					//Tbl 1
 SSP_DELTA_Y_TRIG_TBL_END_ADDR_PLUS_1 - 1,		//Tbl 2
 SSP_DELTA_T_TRIG_TBL_END_ADDR_PLUS_1 - 1		//Tbl 3
 };

const uchar ucaSSP_ByteWidth[LAST_SSP_TBL_NUM_PLUS_1] =
 {
 SSP_Y_TBL_BYTE_WIDTH,					//Tbl 0
 SSP_T_TBL_BYTE_WIDTH,					//Tbl 1
 SSP_DELTA_Y_TRIG_TBL_BYTE_WIDTH,		//Tbl 2
 SSP_DELTA_T_TRIG_TBL_BYTE_WIDTH		//Tbl 3
 };





/*--------------------------  CODE START  ------------------------------------*/




/********************  vPICK_putSSP_tblEntry()  ******************************
*
*
*
******************************************************************************/
void vPICK_putSSP_tblEntry(
		uchar ucSSP_TblNum,		//table number
		uchar ucSensorNum,		//Sensor Number
		ulong ulEntryVal		//table entry value
		)
	{
	usl uslSSP_BaseAddr;
	usl uslSSP_MaxAddr;
	uchar ucSSP_ByteWidth;


	/* GET THE TABLE PRAMETERS FIRST */
	uslSSP_BaseAddr = uslaSSP_BaseAddr[ ucSSP_TblNum];
	uslSSP_MaxAddr = uslaSSP_MaxAddr[ ucSSP_TblNum];
	ucSSP_ByteWidth = ucaSSP_ByteWidth[ ucSSP_TblNum];

	#if 0
	vSERIAL_sout("E:pSSPte\r\n", 10);
	vSERIAL_sout("Tbl[", 4);
	vSERIAL_UIV8out(ucSSP_TblNum);
	vSERIAL_sout("]: Pick", 7);
	vSERIAL_sout("  SensorNum=", 12);
	vSERIAL_HB8out(ucSensorNum);
	vSERIAL_sout("  Val=", 6);
	vSERIAL_UIV32out(ulEntryVal);
	vSERIAL_crlf();
	#endif

	vL2SRAM_putPickTblEntry(
					uslSSP_BaseAddr,			//tbl base addr
					uslSSP_MaxAddr,				//tbl base max addr (LUL)
					ucSensorNum,				//tbl Idx
					SSP_GENERIC_TBL_SIZE - 1,	//tbl Idx Max (LUL)
					ucSSP_ByteWidth,			//tbl Entry Width
					ulEntryVal					//tbl Entry Value
					);

	#if 0
	vSERIAL_sout("X:gSSPte\r\n", 10);
	#endif

	return;

	}/* END: vPICK_putSSP_tblEntry() */

/********************  ulPICK_getSSP_tblEntry()  *****************************
*
*
*
******************************************************************************/
ulong ulPICK_getSSP_tblEntry(
		uchar ucSSP_TblNum,		//table number (0 - 3)
		uchar ucSensorNum		//Sensor Number (0 - SENSOR_MAX_VALUE)
		)
	{
	ulong ulVal;
	usl uslSSP_Base;
	usl uslSSP_MaxAddr;
	uchar ucSSP_ByteWidth;


	/* GET THE TABLE PRAMETERS FIRST */
	uslSSP_Base = uslaSSP_BaseAddr[ ucSSP_TblNum];
	uslSSP_MaxAddr = uslaSSP_MaxAddr[ ucSSP_TblNum];
	ucSSP_ByteWidth = ucaSSP_ByteWidth[ ucSSP_TblNum];


	/* RETRIEVE THE DATA FROM THE SRAM */
	ulVal = ulL2SRAM_getPickTblEntry(
					uslSSP_Base,			//tbl base addr
					uslSSP_MaxAddr,			//tbl base max addr (LUL)
					ucSensorNum,			//tbl Idx
					SENSOR_MAX_VALUE - 1,	//tbl Idx Max (LUL)
					ucSSP_ByteWidth			//tbl Entry Width
					);

	return(ulVal);

	}/* END: ulPICK_getSSP_tblEntry() */



/********************  vPICK_initSSPtbls()  **********************************
*
* Initialize the SSP tables in SRAM
*
******************************************************************************/
void vPICK_initSSPtbls(
		void
		)
	{
	uchar ucSensorNum;
	uint uiTrigVal;

	#if 0
	vSERIAL_sout("E:SITT\r\n", 8);
	#endif

	#if 0	//Start Debug
	{
	uchar ucii;

	for(ucii=0; ucii<LAST_SSP_TBL_NUM_PLUS_1;  ucii++)
		{
		vSERIAL_sout("SSP_tbl: Pick", 13);
		vSERIAL_sout(" Base= ", 7);
		vSERIAL_UIV24out(uslaSSP_BaseAddr[ucii]);
		vSERIAL_sout(" Mx= ", 5);
		vSERIAL_UIV24out(uslaSSP_MaxAddr[ucii]);
		vSERIAL_sout(" Width= ", 8);
		vSERIAL_UIV8out(ucaSSP_ByteWidth[ucii]);
		vSERIAL_crlf();
		}
	}
	#endif	//End: Debug

	for(ucSensorNum=0; ucSensorNum<SENSOR_MAX_VALUE;  ucSensorNum++)
		{
		/* GRAB THE Y-TRIGGER DATA FROM FRAM */
		uiTrigVal = uiL2FRAM_getYtriggerVal(ucSensorNum);
		/* PUT THE Y-TRIGGER DATA INTO SRAM */
		vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, ucSensorNum, (ulong)uiTrigVal);

		/* STUFF THE T TRIGGER VALUE */
		uiTrigVal = 7200;				//1800 * 4 = every 2 hours
		vPICK_putSSP_tblEntry(SSP_DELTA_T_TRIG_TBL_NUM, ucSensorNum, (ulong)uiTrigVal);


		/* ZRO THE Y-BASE & T-BASE VALUES */
		vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, ucSensorNum, 0UL);
		vPICK_putSSP_tblEntry(SSP_T_TBL_NUM, ucSensorNum, 0UL);

		}/* END: for(ucSensorNum) */

	#if 0
	vSERIAL_sout("X:SITT\r\n", 8);
	#endif

	return;

	}/* END: vPICK_initSSPtbls() */







///***************  lPICK_buildUniformCompareEntryFromOM2()  *******************
//*
//* NOTE: Currently we do not use the fact that this passes back a long
//*		This is here for future possibilities with the data.
//*
//* NOTE: This routine needs to be generalized to handle all sensor data types
//*		in an OM2, but at this time it simply handles dual entry TC readings.
//*		SYSACT.C has a routine to identify dual entry sensor ID's.  It can
//*		augmented to return the data form index which could be used to drive
//*		an array of functions to convert to a uniform comparison form.
//*
//* NOTE: When this routine is called for a dual entry data value it picks up
//*		the entry following the one specified to create a single value.
//*		When it has computed the Uniform Compare value it then writes the
//*		2nd part of the dual data into the SSP tables (Y & T) so that the
//*		2nd part of the dual entry will not trigger a new data point.  This
//*		routine is written this way because the calling routine is not supposed
//*		to know about dual (or other grouping) data entrys.
//*
//* RET: long uniform-compare-entry
//*
//******************************************************************************/
//long lPICK_buildUniformCompareEntryFromOM2(
//		uchar ucMsgIdx
//		)
//	{
//	uchar ucSensorNum;
//	int iSensorDataVal1;
//	int iSensorDataVal2;
//	long lUniformDataVal;
//	long lTimeUpdateVal;
//
//
//	ucSensorNum = ucaMSG_BUFF[ucMsgIdx];
//
//	/* CHECK IF THIS IS ONE OF THE DUAL ENTRY DATA POINTS */
//	if((ucSensorNum >= SENSOR_TC_1) && (ucSensorNum <= SENSOR_TC_4))
//		{
//		/* GET THE FIRST PART OF THE DUAL ENTRY */
//		iSensorDataVal1 = (int)uiMISC_buildUintFromBytes(
//										(uchar *)&ucaMSG_BUFF[ucMsgIdx+1],
//										NO_NOINT
//										);
//
//		/* GET THE SECOND PART OF THE DUAL ENTRY */
//		iSensorDataVal2 = (int)uiMISC_buildUintFromBytes(
//										(uchar *)&ucaMSG_BUFF[ucMsgIdx+4],
//										NO_NOINT
//										);
//
//		/* MUNCH THE DATA INTO A SINGLE VALUE */
//		lUniformDataVal = (long)iTHERMO_computeTfromTC(
//										iSensorDataVal1,	//V_actual
//										iSensorDataVal2		//ColdJct
//										);
//
//		#if 0
//		vSERIAL_sout("PK:", 3);
//		vSENSOR_showSensorName(ucSensorNum, R_JUSTIFY);
//		vSERIAL_bout('=');
//		vSERIAL_IV16out(iSensorDataVal1);
//		vSERIAL_bout('(');
//		vSERIAL_HB16out((uint)iSensorDataVal1);
//
//		vSERIAL_sout("),Owir=", 7);
//		vSERIAL_IV16out(iSensorDataVal2);
//		vSERIAL_bout('(');
//		vSERIAL_HB16out((uint)iSensorDataVal2);
//
//		vSERIAL_sout("),Uniform=", 10);
//		vSERIAL_IV32out(lUniformDataVal);
//		vSERIAL_bout('(');
//		vSERIAL_HB32out((ulong)lUniformDataVal);
//		vSERIAL_sout(")(", 2);
//		vTHERMO_showOwireValue((int)lUniformDataVal);
//		vSERIAL_sout("DegC)(", 6);
//		vTHERMO_showOwireValue(iTHERMO_convertOwire_C_to_F((int)lUniformDataVal));
//		vSERIAL_rom_sout("DegF)", 5);
//		vSERIAL_crlf();
//		#endif
//
//
//		/* WRITE THE 2ND PART OF THE DUAL ENTRY (Y) INTO THE SSP TABLE */
//		vPICK_putSSP_tblEntry(	SSP_Y_TBL_NUM,
//								ucaMSG_BUFF[ucMsgIdx+3],	//Owire Sensor ID
//								(ulong)iSensorDataVal2		//Owire Data
//								); //lint !e571
//
//		/* WRITE THE 2ND PART OF THE DUAL ENTRY (T) INTO THE SSP TABLE */
//		lTimeUpdateVal = (long)ulMISC_buildUlongFromBytes(
//							(uchar *)&ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_XI],
//							NO_NOINT
//							);
//
//		vPICK_putSSP_tblEntry(	SSP_T_TBL_NUM,
//								ucaMSG_BUFF[ucMsgIdx+3],	//Owire Sensor ID
//								(ulong)lTimeUpdateVal		//Sample Time
//								);
//
//
//		return(lUniformDataVal);
//
//		}/* END: if() */
//
//
//	/* IF IT WAS A UNARY ENTRY JUST LOAD IT DIRECTLY */
//	lUniformDataVal = (long)uiMISC_buildUintFromBytes(
//										(uchar *)&ucaMSG_BUFF[ucMsgIdx+1],
//										NO_NOINT
//										);
//
//	return(lUniformDataVal);
//
//	}/* END: lPICK_buildUniformCompareEntryFromOM2() */








/********************  ucPICK_isThisAtriggerEvent()  ***********************
*
* RET:	1 = yes a trigger event has     occurred
*		0 =  no a trigger event has not occurred
*
******************************************************************************/
static uchar ucPICK_isThisAtriggerEvent(//1=new pt found, 0=no new pt found
		uchar ucSensorNum,		//sensor number to check
		long lNewSensorPt,		//New sensor data pt value
		uchar ucPointTblNum,	//Pt Tbl Data (last recorded data Pt)
		uchar ucDeltaTblNum		//Delta Tbl Data (trigger threshold for a new Pt)
		)
	{
	uchar ucDecisionFlag;
	long lOldSensorPt;
	long lPtDiff;
	long lTriggerDiff;

	/* INIT DECISION TO == NO TRIGGER EVENT */
	ucDecisionFlag = 0;

	/* GET THE OLD SENSOR POINT */
	lOldSensorPt = (long)ulPICK_getSSP_tblEntry(ucPointTblNum, ucSensorNum);

	/* GET ABSOLUTE DIFF BETWEEN SENSOR PTS */
	lPtDiff = lNewSensorPt - lOldSensorPt;
	if(lPtDiff < 0) lPtDiff = -lPtDiff;

	/* GET THE USER'S TRIGGER DELTA */
	lTriggerDiff = (long)ulPICK_getSSP_tblEntry(ucDeltaTblNum, ucSensorNum);

	/* COMPARE THE PT DIFFERENCE TO THE TRIGGER DIFFERENCE */
	if(lPtDiff >= lTriggerDiff) ucDecisionFlag = 1;

	#if 0
	vSERIAL_sout("TRIGCHK: ", 9);
	vSENSOR_showSensorName(ucSensorNum, R_JUSTIFY);

	vSERIAL_sout(",NewPt=", 7);
	vSERIAL_IV32out(lNewSensorPt);
	vSERIAL_bout('(', 1);
	vSERIAL_HB32out((ulong)lNewSensorPt);

	vSERIAL_sout("),OldPt=", 8);
	vSERIAL_IV32out(lOldSensorPt);
	vSERIAL_bout('(', 1);
	vSERIAL_HB32out((ulong)lOldSensorPt);

	vSERIAL_sout("),PtDiff=", 9);
	vSERIAL_IV32out(lPtDiff);
	vSERIAL_bout('(', 1);
	vSERIAL_HB32out((ulong)lPtDiff);

	vSERIAL_sout("),TrigDelta=", 12);
	vSERIAL_IV32out(lTriggerDiff);
	vSERIAL_bout('(', 1);
	vSERIAL_HB32out((ulong)lTriggerDiff);

	vSERIAL_sout(") Trig=", 7);
	if(ucDecisionFlag)	vSERIAL_sout("NO", 2);
	else				vSERIAL_sout("YS", 2);

	vSERIAL_crlf();
	#endif

	return(ucDecisionFlag);

	}/* END: ucPICK_isThisAtriggerEvent() */



/********************  ucPICK_putTriggeredDataIntoSSP()  **********************
*
* RET: 	1 = yes triggering data and stuffed it into the SSP tables
*		0 =  no triggering data found.
*
******************************************************************************/
uchar ucPICK_putTriggeredDataIntoSSP( //RET: 1=TRIGGERED, 0=NOT TRIGGERED
		void
		)
	{
	uchar ucii;
	uchar ucLastEntryIdx;
	uchar ucSensorID;
	long lSensorData;
	long lOM2_timeStamp;
	uchar ucTriggerFlag;

	/* FIND THE LAST DATA ENTRY IN THE MSG */
	ucLastEntryIdx = ucMSG_getLastFilledEntryInOM2();

	/* LEAVE NOW IF NO DATA ENTRYS EXIST */
	if(ucLastEntryIdx == 0) return(0);

	/* GET THE DATA SAMPLE TIME */
	lOM2_timeStamp = (long)ulMISC_buildUlongFromBytes(
							(uchar *)&ucaMSG_BUFF[OM2_IDX_COLLECTION_TIME_XI],
							NO_NOINT
							);

	/* LOOP FOR ALL DATA CHECKING IF WE HAVE HAD A TRIGGERING EVENT */
	ucTriggerFlag = 0;
	for(ucii=16;  ucii<=ucLastEntryIdx;  ucii+=3)
		{
		/* GET THE DATA ENTRY SENSOR NUMBER */
		ucSensorID = ucaMSG_BUFF[ucii];

		/* GET THE ENTRY DATA VALUE */
		lSensorData = lPICK_buildUniformCompareEntryFromOM2(ucii);

		/* CHECK IF THERE IS A TRIGGER EVENT DUE TO THE Y VALUE */
		ucTriggerFlag |= ucPICK_isThisAtriggerEvent(
									ucSensorID,
									lSensorData,
									SSP_Y_TBL_NUM,
									SSP_DELTA_Y_TRIG_TBL_NUM
									);

		/* CHECK IF THERE IS A TRIGGER EVENT DUE TO THE T VALUE */
		ucTriggerFlag |= ucPICK_isThisAtriggerEvent(
									ucSensorID,
									lOM2_timeStamp,
									SSP_T_TBL_NUM,
									SSP_DELTA_T_TRIG_TBL_NUM
									);


		if(ucTriggerFlag) break;

		}/* END: for(ucii) */


	/* IF NO TRIGGERING THIS TIME JUST LEAVE NOW */
	if(!ucTriggerFlag) return(0);


	/* LOOP FOR ALL DATA CHECKING IF WE HAVE HAD A TRIGGERING EVENT */
	for(ucii=16;  ucii<=ucLastEntryIdx;  ucii+=3)
		{
		/* GET THE DATA ENTRY SENSOR NUMBER */
		ucSensorID = ucaMSG_BUFF[ucii];

		/* GET THE ENTRY DATA VALUE */
		lSensorData = lPICK_buildUniformCompareEntryFromOM2(ucii);

		/* STUFF THE Y VALUE DATA INTO THE SSP TABLE */
		vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, ucSensorID, (ulong)lSensorData);

		/* STUFF THE T VALUE DATA INTO THE SSP TABLE */
		vPICK_putSSP_tblEntry(SSP_T_TBL_NUM, ucSensorID, (ulong)lOM2_timeStamp);

		}/* END: for(ucii) */

	return(1);

	}/* END: ucPICK_putTriggeredDataIntoSSP() */






/***************  vPICK_showSSPtblHeader()  **********************************
*
* Show a single SSP table entry
*
******************************************************************************/
void vPICK_showSSPtblHeader(
		void
		)
	{
	vSERIAL_sout("#### SENSOR  Y-PT TIME-POINT Y-DEL T-DEL\r\n", 42);
	return;

	}/* END: vPICK_showSSPtblHeader() */






/***************  vPICK_showSingleSSPentryFromSRAM()  ************************
*
* Show a single SSP table entry
*
******************************************************************************/
void vPICK_showSingleSSPentryFromSRAM(
		uchar ucSensorID,		//Sensor Number
		uchar ucHDRflag			//YES_HDR, NO_HDR
		)
	{
	uchar ucc;
	ulong ulVal;

	/* SHOW HEADER IF REQUESTED */
	if(ucHDRflag) vPICK_showSSPtblHeader();

	/* SHOW SENSOR NUMBER */
	vSERIAL_UI8out(ucSensorID);
	vSERIAL_sout(": ", 2);

	/* SHOW SENSOR NAME */
//	vSENSOR_showSensorName(ucSensorID, R_JUSTIFY);
	vSERIAL_sout(" ", 1);

	/* SHOW THE ENTRY */
	for(ucc=0;  ucc<LAST_SSP_TBL_NUM_PLUS_1;  ucc++)
		{
		ulVal = ulPICK_getSSP_tblEntry(
								ucc,			//Tbl Num
								ucSensorID		//sensor num
								);

		if(ucc == SSP_T_TBL_NUM)
			vSERIAL_UI32out(ulVal);
		else
			vSERIAL_UI16out((uint)ulVal);

		vSERIAL_sout(" ", 1);

		}/* END: for(ucc) */

	vSERIAL_crlf();

	return;

	}/* END: vPICK_showSingleSSPentryFromSRAM() */



/*****************  vPICK_showEntireSSPtblFromSRAM()  ************************
*
* Show a single SSP table entry
*
******************************************************************************/
void vPICK_showEntireSSPtblFromSRAM(
		uchar ucShowAllFlag		//YES_SHOW_ALL_ENTRYS, NO_SHOW_ALL_ENTRYS
		)
	{
	uchar ucc;

	#if 0
	/* DEBUG */
	vPICK_putSSP_tblEntry(SSP_Y_TBL_NUM, 3, 23UL);
	vPICK_putSSP_tblEntry(SSP_T_TBL_NUM, 3, 24UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM, 3, 25UL);
	vPICK_putSSP_tblEntry(SSP_DELTA_T_TRIG_TBL_NUM, 3, 26UL);
	#endif

	/* SHOW THE TABLE HEADER */
	vPICK_showSSPtblHeader();

	/* SHOW THE REST WITHOUT A HDR */
	for(ucc=0;  ucc<=SENSOR_MAX_VALUE-1;  ucc++)
		{
		if(!ucShowAllFlag)
			{
			if(ulPICK_getSSP_tblEntry(SSP_DELTA_Y_TRIG_TBL_NUM,ucc) == 0) continue;
			}

		vPICK_showSingleSSPentryFromSRAM(
									ucc,		//sensor num
									NO_HDR		//HDR flag
									);

		}/* END: for(ucc) */

	vSERIAL_crlf();

	return;

	}/* END: vPICK_showEntireSSPtblFromSRAM() */







/*--------------------------  MODULE END  ------------------------------------*/
