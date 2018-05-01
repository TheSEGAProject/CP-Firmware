
/***************************  MISC.H  ****************************************
*
* Header for Misc routine pkg
*
*
*
*
* V1.00 10/15/2002 wzr
*	started
*
******************************************************************************/

#ifndef MISC_H_INCLUDED
	#define MISC_H_INCLUDED


	#define YES_NOINT 1
	#define NO_NOINT 0

	#define LED_ON 1
	#define LED_OFF 0

	#define SLEEP_MODE 1		//shut down with clk on
	#define HIBERNATE_MODE 0	//shut down with clk off



/* ROUTINE DEFINITIONS */

void vMISC_setLED(
		uchar ucLEDstate		//LED_ON or LED_OFF
		);

void vMISC_blinkLED(
		uchar ucBlinkCount
		);

/*---------------------------------*/

ulong ulMISC_buildUlongFromBytes(
		uchar *ucpBytes,
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		);
usl uslMISC_buildUslFromBytes(
		uchar *ucpBytes,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		);
uint uiMISC_buildUintFromBytes(
		uchar *ucpBytes,
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		);

/*---------------------------------*/


void vMISC_copyUlongIntoBytes(
		ulong ulLongVal,
		uchar *ucpToPtr,
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		);

void vMISC_copyUintIntoBytes(
		uint uiIntVal,
		uchar *ucpToPtr,
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		);

void vMISC_copyUslIntoBytes(
		usl uslVal,					//src
		uchar *ucpToPtr,			//dest ptr
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		);

void vMISC_copy6ByteTo6Byte(
		uchar *ucpFromPtr,
		uchar *ucpToPtr,
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		);

void vMISC_copyBytesToBytes(
		uchar *ucpFromPtr,
		uchar *ucpToPtr,
		uchar ucByteCnt,
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		);

/*---------------------------------*/

ulong uiMISC_doCompensatedBattRead( //RET: Batt Voltage in mV
		void
		);

/*---------------------------------*/

uchar ucMISC_sleep_until_button_or_clk( //RET 0=Timer, 1=button, 2=past alarm
		uchar ucHibernateFlag	//SLEEP_MODE or HIBERNATE_MODE(no clk)
		);

/*---------------------------------*/


#endif /* MISC_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
