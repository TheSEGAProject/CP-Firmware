

/**************************  BUTTON.C  *****************************************
*
* Routines to perform BUTTONs during events
*
*
* V1.00 10/04/2003 wzr
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
/*lint -e752 */		/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include <msp430x54x.h>		//processor reg description */
#include "std.h"			//standard defines
#include "hal/config.h" 		//system configuration description file
#include "misc.h"			//homeless functions
#include "button.h"			//BUTTON generator routines



extern volatile uchar ucBUTTON_COUNT[4];


extern volatile union						//ucFLAG2_BYTE
  	{
  	uchar byte;

  	struct
  	 {
  	 unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=T3 Alarm, 0=no alarm
  	 unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1 ;1=T1 Alarm, 0=no alarm
  	 unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
  	 unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=clk ticked, 0=not
  	 unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
  	 unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
  	 unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
  	 unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
  	 }FLAG2_STRUCT;

  	}ucFLAG2_BYTE;


/*------------------------  CODE STARTS HERE  -------------------------------*/


/*****************************  NOTE  ****************************************
*
* The button is on P2.4.  When the button is pushed the interrupt
* is self re-triggering so that the interrupt counts the number of interrupt
* entrys that are made during the push.  The count determines whether the
* button push was spurious or real.  In addition to the button re-entry 
* count there is a ram bit in ucFLAG2 that is set on any entry into the
* button interrupt routine.  The ram flag can be used to detect if a button
* push (of any kind) has happened and then the count can be checked for the
* legitimacy of the push.
*
*****************************************************************************/



/*********************  vBUTTON_init()  **************************************
*
* Turns ON the button interrupts and clears the count and ram flag
*
******************************************************************************/

void vBUTTON_init( //Turn ON button int & clr count & clr ram flag
		void
		)
	{
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_BUTTON_INT_BIT = 0;	//clr software flag
	vBUTTON_ClrOnlyButtonCount();			//clr button count

	BUTTON_OUT_PORT &= ~BUTTON_BIT;			//force outgoing latch to 0
	BUTTON_DIR_PORT &= ~BUTTON_BIT;			//button bit to input
	BUTTON_IFG_PORT &= ~BUTTON_BIT;			//clr button int flag
	BUTTON_IE_PORT |= BUTTON_BIT;			//turn on button int

	return;

	}/* END: vBUTTON_init() */





/*********************  vBUTTON_quit()  **************************************
*
* Shuts off the button interrupt and clears the count and ram flag
*
******************************************************************************/

void vBUTTON_quit( //Turn OFF button int & clr count & clr ram flag
		void
		)
	{

	BUTTON_IE_PORT &= ~BUTTON_BIT;							//turn off button int
	BUTTON_IFG_PORT &= ~BUTTON_BIT;							//clr button int flag
	BUTTON_OUT_PORT &= ~BUTTON_BIT;							//force outgoing latch to 0

	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_BUTTON_INT_BIT = 0;	//clr software flag

	vBUTTON_ClrOnlyButtonCount();			//clr button count

	return;

	}/* END: vBUTTON_quit() */






/*********************  vBUTTON_ClrOnlyButtonCount()  ****************************
*
* Clr button count only (not ram flag)
*
******************************************************************************/

void vBUTTON_ClrOnlyButtonCount(//Clr button count only (not ram flag)
		void
		)
	{

	__bic_SR_register(GIE);					// No interrupts

	ucBUTTON_COUNT[0] = 0;
	ucBUTTON_COUNT[1] = 0;
	ucBUTTON_COUNT[2] = 0;
	ucBUTTON_COUNT[3] = 0;

	__bis_SR_register(GIE);					// Yes interrupts

	return;

	}/* END: vBUTTON_ClrOnlyButtonCount() */






/*********************  vBUTTON_reset()  *************************************
*
* 1. Clr Cnt
* 2. Clr ram Flg
*
******************************************************************************/

void vBUTTON_reset( //RET: Clr Cnt & Clr ram Flg
		void
		)
	{
	vBUTTON_ClrOnlyButtonCount();
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_BUTTON_INT_BIT = 0;	//clr software flag

	return;

	}/* END: vBUTTON_reset() */






/*********************  ucBUTTON_isButtonFlgSet()  ************************
*
*
*
******************************************************************************/

uchar ucBUTTON_isButtonFlgSet( //RET: 1=button was pushed, 0=not pushed
		void
		)
	{
	return(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_BUTTON_INT_BIT);

	}/* END: ucBUTTON_isButtonFlgSet() */




/*********************  ulBUTTON_readButtonCount()  ****************************
*
* RET: number of times button interrupt was entered
*
******************************************************************************/

ulong ulBUTTON_readButtonCount( //RET: number of times button routine was entered
		void
		)
	{

	return(ulMISC_buildUlongFromBytes((uchar *)&ucBUTTON_COUNT[0], YES_NOINT));

	}/* END: ulBUTTON_readButtonCount() */




/*********************  ulBUTTON_readAndClrButtonCount()  ********************
*
* 1. read button Cnt
* 2. Clr Cnt
* 3. Clr ram Flg
*
* RET: button count
*
******************************************************************************/

ulong ulBUTTON_readAndClrButtonCount( //RET: read butt Cnt & Clr Cnt & Clr ram Flg
		void
		)
	{
	ulong ulButtonCount;

	ulButtonCount = ulBUTTON_readButtonCount();
	vBUTTON_reset();

	return(ulButtonCount);

	}/* END: ulBUTTON_readAndClrButtonCount() */







/*****************  ucBUTTON_isCountAboveThreshold()  ***********************
*
* 1. Reads button count
* 2. compares it to threhsold
* 3. clears count
* 4. clears ram flag
*
* RET:	1 = button count above threshold
*		0 = button count below threshold
*
*
*****************************************************************************/

uchar ucBUTTON_isCountAboveThreshold( //RET: 1=butt pushed more than cnt, 0=not more
		ulong ulThresholdCount
		)
	{
	ulong ulButtonCount;

	ulButtonCount = ulBUTTON_readAndClrButtonCount();

	#if 0
	vSERIAL_sout("Bttn= ", 6);
	vSERIAL_HB32out(ulButtonCount);
	vSERIAL_crlf();
	#endif


	if(ulButtonCount > ulThresholdCount) return(1);
	return(0);

	}/* END: ucBUTTON_isCountAboveThreshold() */







/*************************  ucBUTTON_buttonHit()  *****************************
*
* 1. Reads button count
* 2. compares it to threshold of 60,000
* 3. clears count
* 4. clears ram flag
*
* RET:	1 = button above 60,000
*		0 = button not above 60,000
*
*****************************************************************************/

uchar ucBUTTON_buttonHit( //RET: 1=Butt cnt more than thshld, 0=not more
		void
		)
	{
	if(ucBUTTON_isCountAboveThreshold(60000UL)) return(1);
	return(0);

	}/* END: ucBUTTON_buttonHit() */







/****************  ucBUTTON_checkForLongButtonPush()  ***************************
*
* 1. Reads button count
* 2. compares it to threshold of 100,000
* 3. clears count
* 4. clears ram flag
*
* RET:	1 = long button push (above 100,000)
*		0 = not long button push (not above 100,000)
*
*****************************************************************************/

uchar ucBUTTON_checkForLongButtonPush(
		void
		)
	{
	if(ucBUTTON_isCountAboveThreshold(100000UL)) return(1);
	return(0);

	}/* END: ucBUTTON_checkForLongButtonPush() */


/*---------------------------  MODULE END  ----------------------------------*/
