
/***************************  NUMCMD.H  ***************************************
*
* General command matcher routines
*
*
* V1.00 12/11/2003 wzr
*	started
*
*******************************************************************************/

#ifndef NUMCMD_H_INCLUDED
 #define NUMCMD_H_INCLUDED


/* DECLARE THE GENERIC FUNCTION TYPE FOR THE TABLE */
typedef void (*GENERIC_CMD_FUNC)(void);

#define  NO_SHOW_INITIAL_PROMPT		0
#define YES_SHOW_INITIAL_PROMPT		1




void vNUMCMD_doCmdInputAndDispatch( //RET: cmd num, (-1 if Escape)
		uchar ucPromptChar,				  //Prompt to display for user
		T_Text *S_CmdText, 				//ptr to cmd str array
		const GENERIC_CMD_FUNC *vCmdFunction, //ptrs to cmd functions
		uchar ucArraySize,				  //size of the cmd str array
		uchar ucInitialPromptFlag		  //NO_SHOW_INITIAL_PROMPT, YES_SHOW_INITIAL_PROMPT
		);

signed char cNUMCMD_getCmdNum(		//RET: cmd num, (-1 if Escape)
		uchar ucUpperRangeLimit	//Upper limit for range checking
		);

void vNUMCMD_showCmdList(	//show the command list to user
		T_Text *S_CmdText, 					//ptr to cmd str array
		uchar ucCmdArraySize				//size of cmd str array (above)
		);


#endif /* NUMCMD_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
