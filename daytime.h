
/***************************  DAYTIME.H  ****************************************
*
* Header for DAYTIME routine pkg
*
*
*
*
* V1.00 2/2/2006 wzr
*	started
*
******************************************************************************/

/* NOTE: Read overview in DAYTIME.c */

#ifndef DAYTIME_H_INCLUDED
 #define DAYTIME_H_INCLUDED

 #define MAX_YEARS_ALLOWED	15
 #define MAX_YEAR_DATE_ALLOWED	(MAX_YEARS_ALLOWED + 2000)

 #define TEXT_FORM  0
 #define NUMERIC_FORM 1



 /* DATE_STRUCT */
 typedef struct {

	uchar ucMonth;	//1 - 12
	uchar ucDay;	//1 - 31
	uchar ucYear;	//since 2000

	} DATE_STRUCT;

 /* END: DATE_STRUCT */


/*-----------------------------------*/

 /* TIME_STRUCT */
 typedef struct {

	uchar ucHour;	//0 - 23
	uchar ucMin;	//0 - 59
	uchar ucSec;	//0 - 59

	} TIME_STRUCT;

 /* END: TIME_STRUCT */


/*-----------------------------------*/


signed char cDAYTIME_convertDstrToDstruct(	//RET: 1=>DateOk  >0=>DateBad
		uchar ucStr[],				//ASCIZ date str mm/dd/yy after 2000
		DATE_STRUCT *spDate			//ptr to Date struct
		);

long lDAYTIME_convertDstructToSec(	//RET: Seconds since Jan-1-2000
		DATE_STRUCT *spDate			//ptr to Date struct
		);

void vDAYTIME_convertDstructToShow(
		DATE_STRUCT *spDate,
		uchar ucTextOrNumbericFlag	//TEXT_FORM= JUL/24/2004, NUMERIC_FORM= 07/24/2004
		);

/*-----------------------------------*/

signed char cDAYTIME_convertTstrToTstruct(	//RET: 1->TimeOk  >0->TimeBad
		uchar ucStr[],				//ASCIZ Time str HH:MM:SS
		TIME_STRUCT *spTime			//ptr to Time struct
		);

long lDAYTIME_convertTstructToSec(	//RET: converted HH:MM:SS to sec
		TIME_STRUCT *spTime			//ptr to Time struct
		);

void vDAYTIME_convertTstructToShow( /* show time as HH:MM:SS */
		TIME_STRUCT *spTime
		);

void vDAYTIME_convertSecToShow( /* show time as HH:MM:SS */
		long lSec
		);

/*-----------------------------------*/

void vDAYTIME_convertSecsToDstructAndTstruct(	//Convert Secs to Dstruct+Tstruct
		long lSecs,					//time in seconds
		DATE_STRUCT *spDate,		//ptr to date struct
		TIME_STRUCT *spTime			//ptr to time struct
		);

void vDAYTIME_convertSysTimeToShowDateAndTime(
		uchar ucTextOrNumericFlag	//TEXT_FORM, or NUMERIC_FORM
		);


/*-----------------------------------*/

void vDAYTIME_convertDstructAndTstructToSetSysTime(
		DATE_STRUCT *spDate,		//ptr to date struct
		TIME_STRUCT *spTime			//ptr to time struct
		);



#endif /* DAYTIME_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
