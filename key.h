
/***************************  KEY.H  *****************************************
*
* Header for KEY routine pkg
*
*
*
* V1.00 10/14/2003 wzr
*	started
*
******************************************************************************/

#ifndef KEY_H_INCLUDED
 #define KEY_H_INCLUDED

 /* ROUTINE DEFINITIONS */
unsigned char ucKEY_doKeyboardCmdInput(
		void
		);

uint uiKEY_doTimedBin(
		uint uiMilliSec
		);

void vKEY_showFlashMsgCounts(
		void
		);

void vKEY_exitFuncPlaceHolder(
		void
		);

void vKEY_checkAndRepairFlashMsgs(
		void
		);

void vKEY_restoreFRAM(
		void
		);


#endif /* KEY_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
