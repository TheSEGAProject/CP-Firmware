
/***************************  DELAY.H  ****************************************
*
* DELAY header file
*
*
*
*
* V1.00 10/01/2002 wzr
*	started
*
******************************************************************************/

#ifndef DELAY_H_INCLUDED
	#define DELAY_H_INCLUDED

//! \def SLEEPMODE_0
//! \def SLEEPMODE_1
//! \def SLEEPMODE_2
#define SLEEPMODE_0 	0
#define SLEEPMODE_1 	1
#define SLEEPMODE_2 	2

	void vDELAY_wait4us(
		void
		);

	void vDELAY_wait100usTic(
		unsigned int uiCount
		);

	void vDELAY_LPMWait1us(unsigned long ulDelay, unsigned char ucSleepMode);

#endif /* DELAY_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
