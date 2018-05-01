
/***************************  CMD.H  ****************************************
*
* General command matcher routines
*
*
* V1.00 12/11/2003 wzr
*	started
*
******************************************************************************/

#ifndef CMD_H_INCLUDED
 #define CMD_H_INCLUDED

uchar ucCMD_getTerminalInputAndMchCmdStr(
		const char *cpaCmdStrArray[], //ptr to the command str array
		uchar ucArraySize,				//size of the command str array
		uchar ucCaseFlag				//case sensitivity flag
		);

uchar ucCMD_matchCmdStr(
		const char *cpaCMDstrArray[], //ptr to the command str array
		uchar ucArraySize,				//size of the command str array
		char *cpEnteredStr,				//the potential command str ptr
		uchar ucCaseFlag				//case sensitivity flag
		);

void vCMD_showCmdList(
		T_Text *S_CmdStrArray,	//ptr to cmd str (11 char max)
		uchar ucCmdArraySize,				//size of cmd str array (above)
		T_Text *S_TextStrArray	//ptr to the cmd text explanation
											//no size is passed here because
											//this is a parallel array to the
											//cmd str array.
		);





#endif /* CMD_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
