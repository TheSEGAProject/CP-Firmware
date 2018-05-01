
/**************************  CMD.C  ******************************************
*
* General command matcher
*
* V1.00 12/11/2003 wzr
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


#include "std.h"				//standard defines
#include "hal/config.h"		//configuration values
#include "serial.h"			//serial port handler
#include "cmd.h"			//command matcher







/***********************  DECLARATIONS  **************************************/



static uchar ucCMD_chkSingleStr(
		char *cpEnteredStr,				//Entered string to check
		const char *cpSingleCmdStr,	//List string to compare against
		uchar ucCaseFlag				//case sensitivity flag
		);




/***********************  CODE STARTS HERE  **********************************/



/***************  ucCMD_getTerminalInputAndMchCmdStr()  **********************
*
* The command array is a ragged string array that is sorted alphabetically.
*
* Will return a positive if enough chars provide a unique match, in other words
* you do not have to have an exact match to get a match.  This means that
* partially entered commands that are matched only require enough characters
* to reach a unique identification.
*
* RET:	255 - no match
*		254 - ambiguous (more than one match)
*		253 -> 0 - single match either exact or partial
*
******************************************************************************/

uchar ucCMD_getTerminalInputAndMchCmdStr(
		const char *cpaCmdStrArray[], //ptr to cmd str array (11 char max)
		uchar ucArraySize,				//size of the cmd str array
		uchar ucCaseFlag				//case sensitivity flag
		)
	{
	uchar ucStr[12];
	uchar ucMchRet;

	while(ucSERIAL_getEditedInput(ucStr, 10));	//lint !e722

	ucMchRet = ucCMD_matchCmdStr(
								cpaCmdStrArray,
								ucArraySize,
								(char *)ucStr,
								ucCaseFlag
								);

	return(ucMchRet);

	}/* END: ucCMD_getTerminalInputAndMchCmdStr() */







/**********************  ucCMD_matchCmdStr()  ******************************
*
* Return the Md part of the serial number.
*
* The array is a ragged string array that is sorted alphabetically.
*
* Will return a postive if enough chars provide a unique match, in otherwords
* you do not have to have an exact match to get a match.  This means that
* partially entered commands that are matched only require enought characters
* to reach a unique identification.
*
* RET:	255 - no match
*		254 - ambiguous (more than one match)
*		253 -> 0 - single match either exact or partial
*
******************************************************************************/

uchar ucCMD_matchCmdStr(
		const char *cpaCMDstrArray[], //ptr to cmd str array (11 char max)
		uchar ucArraySize,				//size of the command str array
		char *cpEnteredStr,				//the potential command str ptr
		uchar ucCaseFlag				//case sensitivity flag
		)
	{
	uchar ucii;
	uchar ucMchCnt;
	uchar ucMchIdx;
	uchar ucSingleMchVal;

	/* SEARCH THE ARRAY */
	ucMchCnt = 0;
	ucMchIdx = 255;
	for(ucii=0;  ucii<ucArraySize;  ucii++)
		{
		ucSingleMchVal = ucCMD_chkSingleStr(
								cpEnteredStr, 
								cpaCMDstrArray[ucii],
								ucCaseFlag
								);

		/* CHECK IF WE HAVE PASSED ANY CHANCE OF A MATCH */
		if(ucSingleMchVal == 0) break; //EnteredStr is earlier in alphabet

		/* CHECK FOR AN EXACT MATCH AND IF SO STOP THE SEARCH */
		if(ucSingleMchVal == 1)
			{
			ucMchCnt++;				//found a potential match
			ucMchIdx = ucii;		//save the idx
			break;
			}

		/* CHECK FOR AN AMBIGUOUS MATCH -- MARK IT AND KEEP GOING */
		if(ucSingleMchVal == 2)
			{
			ucMchCnt++;				//found a potential match
			ucMchIdx = ucii;		//save the idx
			}

		}/* END: for(ucii) */

	if(ucMchCnt == 0) return(255);
	if(ucMchCnt > 1) return(254);
	return(ucMchIdx);

	}/* END: ucCMD_matchCmdStr() */






/********************  ucCMD_chkSingleStr()  **********************************
*
* RET: 	0 if Entered str value is < List Str value
*		1 if there was an exact char for char match
*		2 if Entered str value matches Cmd str value but is not
*			 an exact char for char match
*		3 if Entered str value is > List Str value
*
******************************************************************************/
static uchar ucCMD_chkSingleStr(
		char *cpEnteredStr,				//Entered string to check
		const char *cpSingleCmdStr,	//List string to compare against
		uchar ucCaseFlag				//case sensitivity flag
		)
	{
	uchar ucChar1;
	uchar ucChar2;
	uchar ucMatchVal;


	while(TRUE)	//lint !e716 !e774
		{
		ucChar1 = (uchar)*cpEnteredStr++;
		ucChar2 = (uchar)*cpSingleCmdStr++;
		if(ucCaseFlag)
			{
			ucChar1 = ucSERIAL_toupper(ucChar1);
			ucChar2 = ucSERIAL_toupper(ucChar2);
			}

		/* CHECK FOR AN EXACT CHAR FOR CHAR MATCH */
		ucMatchVal = 1;
		if((ucChar1 == 0) && (ucChar2 == 0)) break;

		/* CHECK FOR ENTERED STR END */
		ucMatchVal = 2;
		if(ucChar1 == 0) break;

		/* CHECK FOR NO MATCH -- ENTERED STR < LIST STR */
		ucMatchVal = 0;
		if(ucChar1 < ucChar2) break;

		/* CHECK FOR NO MATCH -- ENTERED STR > LIST STR */
		ucMatchVal = 3;
		if(ucChar1 > ucChar2) break;

		}/* END: while() */

	#if 0
	vSERIAL_sout("CSS:MchVal= ", 12);
	vSERIAL_UIV8out(ucMatchVal);
	vSERIAL_sout(" = ", 3);
	switch(ucMatchVal)
		{
		case 0:	vSERIAL_sout("EStr<LStr", 9); break;
		case 1: vSERIAL_sout("EStr==LStr", 10); break;
		case 2: vSERIAL_sout("EStr><LStr:", 11); break;
		case 3: vSERIAL_sout("EStr>LStr:", 10); break;
		default: vSERIAL_sout("EStr??LStr:", 11); break;
		}
	vSERIAL_crlf();
	#endif

	return(ucMatchVal);

	}/* END: ucCMD_chkSingleStr() */







/***********************  vCMD_showCmdList()  *********************************
*
* This is a help message displayer
*
* It shows the command and then show the text explanation to the right of the 
* command.
*
*******************************************************************************/

void vCMD_showCmdList(
		T_Text *S_CmdStrArray,	//ptr to cmd str (11 char max)
		uchar ucCmdArraySize,				//size of cmd str array (above)
		T_Text *S_TextStrArray	//ptr to the cmd text explanation
											//no size is passed here because
											//this is a parallel array to the
											//cmd str array.
		)
	{

	uchar ucii;

	for(ucii=0;  ucii<ucCmdArraySize;  ucii++)
		{
		vSERIAL_sout(S_CmdStrArray[ucii].m_cText, S_CmdStrArray[ucii].m_uiLength);
		vSERIAL_colTab(10);

		vSERIAL_sout("- ", 2);
		vSERIAL_sout(S_TextStrArray[ucii].m_cText, S_TextStrArray[ucii].m_uiLength);

		vSERIAL_crlf();
		}

	return;

	}/* END: vCMD_showCmdList() */




/* --------------------------  END of MODULE  ------------------------------- */
