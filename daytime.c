

/**************************  DAYTIME.C  ******************************************
*
* DAYTIME routines here
*
* V1.00 02/02/2006 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* function not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "diag.h"			//diagnostic defines
#include "std.h"			//common definitions
#include "hal/config.h" 	//system configuration definitions
#include "serial.h" 		//serial port IO pkg
#include "time.h"			//TIME routines
#include "daytime.h"		//DAYTIME routines
#include "report.h" 		//msg reporting routines
#include "sensor.h"			//Sensor definitions
#include "modopt.h"			//Modify Options routines





									/*   J  F  M  A  M  J  J  A  S  O  N  D */
const uchar ucaMonthDayCount[12] = {31,29,31,30,31,30,31,31,30,31,30,31};


T_Text S_MonthName[12] = {
		{"JAN", 3},
		{"FEB", 3},
		{"MAR", 3},
		{"APR", 3},
		{"MAY", 3},
		{"JUN", 3},
		{"JUL", 3},
		{"AUG", 3},
		{"SEP", 3},
		{"OCT", 3},
		{"NOV", 3},
		{"DEC", 3},
};

/*****************************  CODE  ***************************************/



/******************  cDAYTIME_convertDstrToDstruct()  ************************
*
* Return a filled Date struct that has been checked.
*
*
* RET:	-1 - Month too bit
*		-2 - Month == 0
*		-3 - Slash 1 is missing
*		-4 - Day too big
*		-5 - Day == 0
*		-6 - Slash 2 is missing
*		-7 - Year > 15
*		-8 - Feb 29 in non-leapYear
*
*	    +1 - Good
*
*****************************************************************************/

signed char cDAYTIME_convertDstrToDstruct(//RET: 1=>DateOk  >0=>DateBad
		uchar ucStr[],				//ASCIZ date str mm/dd/yy after 2000
		DATE_STRUCT *spDate			//ptr to Date struct
		)
	{
	uchar ucMonth;
	uchar ucDay;
	uchar ucYear;
	uchar ucStrIdx;
	long lVal;

	/* GET THE MONTH AND CHECK IT */
	lVal = lSERIAL_AsciiToNum(ucStr, UNSIGNED, 10);		//get month
	if(lVal > 12) return(1);							//month was bad
	ucMonth = (uchar)lVal;								//stuff month
	if(ucMonth == 0) return(-2);						//month was bad

	/* CHECK FOR THE SLASH */
	ucStrIdx = 1;										//pt to possible slash
	if(ucSERIAL_isnum(ucStr[ucStrIdx])) ucStrIdx++;		//pt to slash
	if((ucStr[ucStrIdx] != '/') && (ucStr[ucStrIdx] != ':')) return(-3);	//slash not there
	ucStrIdx++;											//slash OK--pt to day

	/* GET THE DAY AND CHECK IT */
	lVal = lSERIAL_AsciiToNum(&ucStr[ucStrIdx], UNSIGNED, 10);	//get day
	if(lVal > ucaMonthDayCount[ucMonth-1]) return(-4);	//day was bad
	ucDay = (uchar)lVal;								//stuff day
	if(ucDay == 0) return(-5);							//day was bad

	/* CHECK FOR THE SLASH */
	ucStrIdx++;											//pt to possible slash
	if(ucSERIAL_isnum(ucStr[ucStrIdx])) ucStrIdx++;		//pt to slash
	if((ucStr[ucStrIdx] != '/') && (ucStr[ucStrIdx] != ':')) return(-6);	//slash not there
	ucStrIdx++;											//slash OK--pt to yr

	/* GET THE YEAR AND CHECK IT */
	lVal = lSERIAL_AsciiToNum(&ucStr[ucStrIdx], UNSIGNED, 10);	//get year
	if(lVal > MAX_YEAR_DATE_ALLOWED) return(-7);		//year was too high
	if(lVal >= 2000) lVal -= 2000;						//convert 2000 to offset
	if((lVal> MAX_YEAR_DATE_ALLOWED) && (lVal < 2000)) return(-8);	//yr bad
	ucYear = (uchar)lVal;								//stuff year

	/* NOW CHECK FOR LEAP YEAR FOR FEBRUARY DAY */
	if(ucMonth == 2)
		{
		if((ucYear % 4) && (ucDay >= 29)) return(-8);
		}

	/* NOW FILL THE STRUCT */
	spDate->ucMonth = ucMonth;
	spDate->ucDay = ucDay;
	spDate->ucYear = ucYear;

	return(1);

	}/* END: cDAYTIME_convertDstrToDstruct() */






/******************  lDAYTIME_convertDstructToSec()  ****************************
*
* Return number of seconds since Jan-1-2000
*
*
*****************************************************************************/

long lDAYTIME_convertDstructToSec(	//RET: Seconds since Jan-1-2000
		DATE_STRUCT *spDate			//ptr to Date struct
		)
	{
	uchar ucc;
	long lVal;

	/* COMPUTE THE TIME IN WHOLE DAYS (smallest unit here is days)*/
	lVal = (long)spDate->ucDay-1;		//number of full days in this month

	#if 0
	vSERIAL_sout("FullDaysToDate= ", 16);
	vSERIAL_IV32out(lVal);
	vSERIAL_crlf();
	#endif

	/* NOW COMPUTE DAYS TO START OF THIS MONTH */
	for(ucc=0; ucc<spDate->ucMonth-1;  ucc++)
		{
		lVal += (long)ucaMonthDayCount[ucc];
		if((ucc == 1) && (spDate->ucYear % 4)) lVal--;	//Feb = only 28 days
		}

	#if 0
	vSERIAL_sout("FullDaysToDate= ", 16);
	vSERIAL_IV32out(lVal);
	vSERIAL_crlf();
	#endif
	
	/* NOW ADD UP ALL THE DAYS IN THE YEARS TIL THIS YEAR */
	if(spDate->ucYear > 0)
		{
		for(ucc=0; ucc<spDate->ucYear; ucc++)
			{
			lVal += 365;
			if((ucc % 4) == 0) lVal++;			//Leap Year

			}/* END: for() */

		}/* END: if() */

	#if 0
	vSERIAL_sout("FullYears+DaysToDate= ", 22);
	vSERIAL_IV32out(lVal);
	vSERIAL_crlf();
	#endif

	/* NOW COMPUTE DAYS INTO SECONDS */
	lVal *= 86400L;

	#if 0
	vSERIAL_sout("TotSecsToDate= ", 15);
	vSERIAL_IV32out(lVal);
	vSERIAL_crlf();
	#endif

	return(lVal);

	}/* END: lDAYTIME_convertDstructToSec() */







/*********************  vDAYTIME_convertDstructToShow()  *********************
*
*
*
******************************************************************************/

void vDAYTIME_convertDstructToShow( /* show date as 07/24/2004 */
		DATE_STRUCT *spDate,
		uchar ucTextOrNumericFlag	//TEXT_FORM= JUL/24/2004, NUMERIC_FORM= 07/24/2004
		)
	{

	if(ucTextOrNumericFlag == TEXT_FORM)
		vSERIAL_sout(S_MonthName[(spDate->ucMonth)-1].m_cText, S_MonthName[(spDate->ucMonth)-1].m_uiLength);
	else
		vSERIAL_UI8_2char_out(spDate->ucMonth, '0');

	vSERIAL_bout('/');
	vSERIAL_UI8_2char_out(spDate->ucDay, '0');

	vSERIAL_bout('/');
	if(ucTextOrNumericFlag == TEXT_FORM)  vSERIAL_sout("20", 2);
	
	vSERIAL_UI8_2char_out(spDate->ucYear, '0');

	return;

	}/* END: vDAYTIME_convertDstructToShow() */





/*************  cDAYTIME_convertTstrToTstruct()  **************************
*
* Return a filled Time struct that has been checked.
*
* RET:	-1 - Hour is too big
*		-2 - Colon 1 is missing
*		-3 - Min is too big
*		-4 - Colon 2 is missing
*		-5 - Sec is too big
*
*	    +1 - Good
*
*****************************************************************************/

signed char cDAYTIME_convertTstrToTstruct(	//RET: 1->TimeOk  >0->TimeBad
		uchar ucStr[],				//ASCIZ Time str HH:MM:SS
		TIME_STRUCT *spTime			//ptr to Time struct
		)
	{
	uchar ucHour;
	uchar ucMin;
	uchar ucSec;
	uchar ucStrIdx;
	long lVal;

	/* GET THE HOUR AND CHECK IT */
	lVal = lSERIAL_AsciiToNum(ucStr, UNSIGNED, 10);		//get hour
	if(lVal > 23) return(-1);							//hour was bad
	ucHour = (uchar)lVal;								//stuff month

	/* CHECK FOR THE COLON */
	ucStrIdx = 1;										//pt to possible colon
	if(ucSERIAL_isnum(ucStr[ucStrIdx])) ucStrIdx++;		//pt to colon
	if(ucStr[ucStrIdx] != ':') return(-2);				//colon not there
	ucStrIdx++;											//colon OK--pt to min

	/* GET THE MIN AND CHECK IT */
	lVal = lSERIAL_AsciiToNum(&ucStr[ucStrIdx], UNSIGNED, 10);	//get min
	if(lVal > 59) return(-3);							//min was bad
	ucMin = (uchar)lVal;								//stuff min

	/* CHECK FOR THE COLON */
	ucStrIdx++;											//pt to possible colon
	if(ucSERIAL_isnum(ucStr[ucStrIdx])) ucStrIdx++;		//pt to colon
	if(ucStr[ucStrIdx] != ':') return(-4);				//colon not there
	ucStrIdx++;											//colon OK--pt to sec

	/* GET THE SEC AND CHECK IT */
	lVal = lSERIAL_AsciiToNum(&ucStr[ucStrIdx], UNSIGNED, 10);	//get sec
	if(lVal > 59) return(-5);							//sec was bad
	ucSec = (uchar)lVal;								//stuff sec

	/* NOW FILL THE STRUCT */
	spTime->ucHour = ucHour;
	spTime->ucMin = ucMin;
	spTime->ucSec = ucSec;

	return(1);


	}/* END: cDAYTIME_convertTstrToTstruct() */







/*************  lDAYTIME_convertTstructToSec()  *****************************
*
*
*
*****************************************************************************/

long lDAYTIME_convertTstructToSec(	//RET: converted HH:MM:SS to sec
		TIME_STRUCT *spTime			//ptr to Time struct
		)
	{
	long lVal;

	/* COMPUTE THE HOURS IN Minutes */
	lVal = (long)spTime->ucHour;
	lVal *= 60L;

	#if 0
	vSERIAL_sout("MinsInHr= ", 10);
	vSERIAL_IV32out(lVal);
	vSERIAL_crlf();
	#endif

	/* NOW ADD THE NUMBER OF MINUTES */
	lVal += (long)spTime->ucMin;

	#if 0
	vSERIAL_sout("MinsInHr+Min= ", 14);
	vSERIAL_IV32out(lVal);
	vSERIAL_crlf();
	#endif

	/* NOW CONVERT MINUTES INTO SECONDS */
	lVal *= 60L;

	#if 0
	vSERIAL_sout("SecsInHr+Min= ", 14);
	vSERIAL_IV32out(lVal);
	vSERIAL_crlf();
	#endif

	/* NOW ADD IN THE REMAINING SECONDS */
	lVal += (long)spTime->ucSec;

	#if 0
	vSERIAL_sout("TotSecInHr+Min+Sec= ", 21);
	vSERIAL_IV32out(lVal);
	vSERIAL_crlf();
	#endif

	return(lVal);

	}/* END: lDAYTIME_convertTstructToSec() */









/*********************  vDAYTIME_convertTstructToShow()  ********************************
*
*
*
******************************************************************************/

void vDAYTIME_convertTstructToShow( /* show time as HH:MM:SS */
		TIME_STRUCT *spTime
		)
	{

	vSERIAL_UI8_2char_out(spTime->ucHour, '0');
	vSERIAL_bout(':');
	vSERIAL_UI8_2char_out(spTime->ucMin, '0');
	vSERIAL_bout(':');
	vSERIAL_UI8_2char_out(spTime->ucSec, '0');

	return;

	}/* END: vDAYTIME_convertTstructToShow() */








/*********************  vDAYTIME_convertSecToShow()  **************************
*
*
*
******************************************************************************/

void vDAYTIME_convertSecToShow( /* show time as HH:MM:SS */
		long lSec
		)
	{
	DATE_STRUCT sDate;
	TIME_STRUCT sTime;

	vDAYTIME_convertSecsToDstructAndTstruct(lSec, &sDate, &sTime);
	vDAYTIME_convertTstructToShow(&sTime);

	return;

	}/* END: vDAYTIME_convertSecToShow() */





/******************  vDAYTIME_convertSecsToDstructAndTstruct()  *************************
*
*
*
******************************************************************************/

void vDAYTIME_convertSecsToDstructAndTstruct(	//Convert Secs to Dstruct+Tstruct
		long lSecs,					//time in seconds
		DATE_STRUCT *spDate,		//ptr to date struct
		TIME_STRUCT *spTime			//ptr to time struct
		)
	{
	uchar ucc;
	long lVal;
	long lUnitSize;
	long lNewSecs;
	#define MAX_YEARS_POSSIBLE 69


	/* DO NOT ALLOW NEGATIVE SEC COUNTS */
	if(lSecs < 0L) lSecs = -lSecs;

	/* CULL OUT THE YEAR NUMBER */
	for(ucc=0;  ucc<=MAX_YEARS_POSSIBLE;  ucc++)
		{
		lUnitSize = 31536000L;						//365 days in sec
		if((ucc % 4) == 0) lUnitSize = 31622400L;	//366 days in sec

		lNewSecs = lSecs - lUnitSize;
		if(lNewSecs < 0) break;				//leave if over

		lSecs = lNewSecs;					//update the value

		}/* END: for(ucc) */

	spDate->ucYear = ucc;					//save the year number



	/* CULL OUT THE MONTH NUMBER */
	lVal = 0;
	lUnitSize = 0;
	for(ucc=0; ucc<12;  ucc++)
		{
		lVal = lUnitSize;								//save prev value
		lUnitSize += (long)ucaMonthDayCount[ucc];		//size in days
		if((spDate->ucYear % 4) && (ucc == 1))			//not leap year & Feb
			{
			lUnitSize--;								//back up 1 day
			}
		lNewSecs = lSecs - (lUnitSize * 86400L);
		if(lNewSecs < 0) break;							//leave if over

		}/* END: for(ucc) */

	spDate->ucMonth = ucc+1;							//save the month
	lSecs -= (lVal * 86400L);							//update the secs


	/* CULL OUT THE DAY NUMBER */
	lVal = lSecs / 86400L;
	lSecs %= 86400L;
	spDate->ucDay = (uchar)lVal+1;


	/* CULL OUT THE HOUR NUMBER */
	lVal = lSecs / 3600L;
	lSecs %= 3600L;
	spTime->ucHour = (uchar)lVal;

	/* CULL OUT THE MIN NUMBER */
	lVal = lSecs / 60L;
	lSecs %= 60L;
	spTime->ucMin = (uchar)lVal;

	/* CULL OUT THE SEC NUMBER */
	spTime->ucSec = (uchar)lSecs;

	return;

	}/* END: vDAYTIME_convertSecsToDstructAndTstruct() */





/**************  vDAYTIME_convertSysTimeToShowDateAndTime()  ******************************
*
*
*
*****************************************************************************/

void vDAYTIME_convertSysTimeToShowDateAndTime(
		uchar ucTextOrNumericFlag	//TEXT_FORM, or NUMERIC_FORM
		)
	{
	DATE_STRUCT sDate;
	TIME_STRUCT sTime;

	vDAYTIME_convertSecsToDstructAndTstruct(
			lTIME_getSysTimeAsLong(),
			&sDate,
			&sTime
			);


	vDAYTIME_convertDstructToShow(&sDate, ucTextOrNumericFlag);
	vSERIAL_bout(' ');
	vDAYTIME_convertTstructToShow(&sTime);

	return;

	}/* END: vDAYTIME_convertSysTimeToShowDateAndTime() */





/***************  vDAYTIME_convertDstructAndTstructToSetSysTime()  ***********
*
*
*
*
******************************************************************************/

void vDAYTIME_convertDstructAndTstructToSetSysTime(
		DATE_STRUCT *spDate,		//ptr to date struct
		TIME_STRUCT *spTime			//ptr to time struct
		)
	{
	long lSec;

	lSec = lDAYTIME_convertTstructToSec(spTime);
	lSec += lDAYTIME_convertDstructToSec(spDate);

	vTIME_setSysTimeFromLong(lSec);

	return;

	}/* END: vDAYTIME_convertDstructAndTstructToSetSysTime() */


/* --------------------------  END of MODULE  ------------------------------- */
