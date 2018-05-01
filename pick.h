
/***************************  PICK.H  ****************************************
*
* Header for PICK (pick a sample to send) routines
*
*
*
* V1.00 12/24/2006 wzr
*	started
*
******************************************************************************/

#ifndef PICK_H_INCLUDED
  #define PICK_H_INCLUDED



void vPICK_putSSP_tblEntry(
		uchar ucSSP_TblNum,		//table number
		uchar ucSensorNum,		//Sensor Number
		ulong ulEntryVal		//table entry value
		);

ulong ulPICK_getSSP_tblEntry(
		uchar ucSSP_TblNum,		//table number
		uchar ucSensorNum		//Sensor Number
		);

void vPICK_initSSPtbls(
		void
		);

long lPICK_buildUniformCompareEntryFromOM2(
		uchar ucMsgIdx
		);

uchar ucPICK_putTriggeredDataIntoSSP( //RET: 1=TRIGGERED, 0=NOT TRIGGERED
		void
		);

void vPICK_showSingleSSPentryInSRAM(
		uchar ucSensorNum,		//Sensor Number
		uchar ucHDRflag			//YES_HDR, NO_HDR
		);

void vPICK_showEntireSSPtblFromSRAM(
		uchar ucShowAllFlag		//YES_SHOW_ALL_ENTRYS, NO_SHOW_ALL_ENTRYS
		);

#endif /* PICK_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
