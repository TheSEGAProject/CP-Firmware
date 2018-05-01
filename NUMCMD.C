

/**************************  NUMCMD.C  ******************************************
*
* Numbered command matcher
*
* V1.00 12/11/2003 wzr
*		Started
*
******************************************************************************/



/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e716 */		/* while(1) */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "std.h"		//standard defines
#include "key.h"
#include "hal/config.h"		//configuration values
#include "serial.h"		//serial port handler
#include "numcmd.h"		//command matcher







/***********************  DECLARATIONS  **************************************/

void vNUMCMD_showCmdName(	//Show the command name
		char cCmdNum,					 //number of the cmd chosen
		T_Text *S_CmdText, 					//ptr to cmd str array
		uchar ucArraySize				//size of the cmd str array
		);



/***********************  CODE STARTS HERE  **********************************/




/***************  vNUMCMD_doCmdInputAndDispatch()  ***************************************
*
*
******************************************************************************/

void vNUMCMD_doCmdInputAndDispatch( //RET: cmd num, (-1 if Escape)
		uchar ucPromptChar,				  //Prompt to display for user
		T_Text *S_CmdText, 					//ptr to cmd str array
		const GENERIC_CMD_FUNC *vCmdFunction, //ptrs to cmd functions
		uchar ucArraySize,				  //size of the cmd str array
		uchar ucInitialPromptFlag		  //NO_SHOW_INITIAL_PROMPT, YES_SHOW_INITIAL_PROMPT
		)
	{
	signed char cNumRetVal;
	uchar ucii;

	for(ucii=0;  ;  ucii++)				//lint !e716 !e774
		{
		/* OUTPUT COMMAND PROMPT */
		if((ucii != 0) || (ucInitialPromptFlag == YES_SHOW_INITIAL_PROMPT))
			{
			vSERIAL_bout(ucPromptChar);
			}


		/* GET A COMMAND */
		cNumRetVal = cNUMCMD_getCmdNum(
						ucArraySize
						);

		if(cNumRetVal >= 0)
			{
			vNUMCMD_showCmdName(
						cNumRetVal,			//Cmd Num
						S_CmdText,		//Array of Cmd names
						ucArraySize			//Array Size
						);
			}

		/* CHECK FOR A QUIT */
		if(cNumRetVal == 0)	goto DCIAD_xit;

		/* IF NOT ESCAPE THE DO THE COMMAND */
		if(cNumRetVal >= 0)
			{
			/* DISPATCH TO THE COMMAND */
			vCmdFunction[(uchar)cNumRetVal]();
			}

		vSERIAL_crlf();

		}/* END: while(TRUE) */

DCIAD_xit:

	return;

	}/* END: vNUMCMD_doCmdInputAndDispatch() */








/***************  vNUMCMD_showCmdName()  ***************************************
*
* RET:	-1 = ESCAPE
*		 0 = EXIT
*		>0 = CMD
*
******************************************************************************/

void vNUMCMD_showCmdName(	//Show the command name
		char cCmdNum,					 //number of the cmd chosen
		T_Text *S_CmdText, 					//ptr to cmd str array
		uchar ucArraySize				//size of the cmd str array
		)
	{

	/* CHK FOR A BAD NUMBER */
	if(cCmdNum >= ucArraySize)
		{
		vSERIAL_sout("???", 3);
		return;
		}/* END: if() */


	/* ECHO THE COMMAND */
	vSERIAL_sout(S_CmdText[cCmdNum].m_cText, S_CmdText[cCmdNum].m_uiLength);

	vSERIAL_crlf();

	return;

	}/* END: vNUMCMD_showCmdName() */






/***************  cNUMCMD_getCmdNum()  ***************************************
*
* This routine returns a (char) command number from 0 to 127
* or else will return a negative value if escape is hit.
*
* RET:	-1 = ESCAPE
*		 0 = EXIT
*		>0 = CMD
*
******************************************************************************/

signed char cNUMCMD_getCmdNum(		//RET: cmd num, (-1 if Escape)
		uchar ucUpperRangeLimit	//Upper limit for range checking
		)
	{
	ulong ulVal;
	uchar ucStr[4];

	while(TRUE)										//lint !e774
		{
		/* CHECK FOR ESCAPE */
		if(ucSERIAL_getEditedInput(ucStr, 3))
			{
			vSERIAL_showXXXmsg();					//escape was hit
			return(-1);
			}

		/* CHECK FOR AT LEAST 1 NUMERIC */
		if((ucStr[0] < '0') || (ucStr[0] > '9'))
			{
			vSERIAL_sout("MustBeNumeric,TryAgain..", 23);
			continue;
			}

		/* CONVERT THE TEXT INTO A NUMBER */
		ulVal = (ulong)lSERIAL_AsciiToNum(ucStr, UNSIGNED, DECIMAL);

		// Temporary fix
		ucUpperRangeLimit = 0x12;

		/* CHK FOR A BAD NUMBER */
		if(ulVal >= ucUpperRangeLimit)
			{
			vSERIAL_sout("OutOfRange,TryAgain..", 21);
			continue;

			}/* END: if() */

		break;

		}/* END: while() */

	return((char)ulVal);

	}/* END: cNUMCMD_getCmdNum() */






/***********************  vNUMCMD_showCmdList()  *********************************
*
* This is a help message displayer
*
* It shows the	NUMBER,
*				COMMAND (first cmd must be Exit cmd)
*				EXPLANATION
* commands in columns.
*
* NOTE: In the wisard we have an artificial limit of 0 - 15 commands
*		with 0 = EXIT so that it will fit 2 commands nicely into a
*		byte for outbound messaging.
*
*******************************************************************************/

void vNUMCMD_showCmdList(	//show the command list to user
		T_Text *S_CmdText, 					//ptr to cmd str structure
		uchar ucCmdArraySize				//size of cmd str array (above)
		)
	{

	uchar ucii;

	vSERIAL_sout("NUM CMD\r\n", 9);

	for(ucii=0;  ucii<ucCmdArraySize;  ucii++)
		{
		vSERIAL_UI8_2char_out(ucii, ' ');
		vSERIAL_sout(": ", 2);
		vSERIAL_sout(S_CmdText[ucii].m_cText,S_CmdText[ucii].m_uiLength);

		vSERIAL_crlf();
		}

	return;

	}/* END: vNUMCMD_showCmdList() */




/* --------------------------  END of MODULE  ------------------------------- */
