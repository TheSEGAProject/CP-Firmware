
/***************************  BUTTON.H  ****************************************
*
* Header for BUTTON routine pkg
*
*
* V1.00 01/28/2005 wzr
*	started
*
******************************************************************************/

#ifndef BUTTON_H_INCLUDED
 #define BUTTON_H_INCLUDED


 /* ROUTINE DEFINITIONS */

 void vBUTTON_init( //Turn on button int & clr count & clr ram flag
		void
		);

 void vBUTTON_quit(
		void
		);

void vBUTTON_ClrOnlyButtonCount(//Clr button count only (not ram flag)
		void
		);

void vBUTTON_reset( //RET: Clr Cnt & Clr ram Flg
		void
		);

uchar ucBUTTON_isButtonFlgSet( //RET: 1=button flg set, 0=not set
		void
		);

ulong ulBUTTON_readButtonCount( //RET: number of times button routine was entered
		void
		);

ulong ulBUTTON_readAndClrButtonCount( //RET: Butt Cnt & Clr Cnt & Clr Flg
		void
		);

uchar ucBUTTON_buttonHit( //RET: 1=Butt cnt more than thshld, 0=not more
		void
		);

uchar ucBUTTON_checkForLongButtonPush( //RET: 1=long butt, 0=not long
		void
		);



#endif /* BUTTON_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
