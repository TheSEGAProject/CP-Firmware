
/**************************  MISC.C  ******************************************
*
* Miscelaneous routines here
*
*
* V1.00 10/15/2002 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* function not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include <msp430x54x.h>		//register and ram definition file 
#include "std.h"			//common definitions
#include "misc.H"			//misc functions
#include "delay.h"			//delay routines
#include "serial.H"			//serial port IO pkg
#include "time.h"			//system time routines
#include "button.h"			//button handler
#include "config.h"		//system configuration definitions
#include "PMM.h"			// Power management module

/*----------------------  DEFINES  ------------------------------------------*/

/* LEVELs from PIC
#define BATT_LOW_V464	//00001111
#define BATT_LOW_V433	//00001101
#define BATT_LOW_V413	//00001100
#define BATT_LOW_V392	//00001011
#define BATT_LOW_V372	//00001010
#define BATT_LOW_V361	//00001001
#define BATT_LOW_V341	//00001000
#define BATT_LOW_V310	//00000111
#define BATT_LOW_V289	//00000110
#define BATT_LOW_V278	//00000101
#define BATT_LOW_V258	//00000100
*/


/*----------------------  EXTERNS  ------------------------------------------*/

extern volatile union							//ucFLAG2_BYTE
		{
		uchar byte;

		struct
		 {
	  	 unsigned FLG2_T3_ALARM_MCH_BIT:1;		//bit 0 ;1=Alarm, 0=no alarm
		 unsigned FLG2_T1_ALARM_MCH_BIT:1;		//bit 1
		 unsigned FLG2_BUTTON_INT_BIT:1;		//bit 2 ;1=XMIT, 0=RECEIVE
		 unsigned FLG2_CLK_INT_BIT:1;			//bit 3	;1=REC DONE, 0=NOT DONE
		 unsigned FLG2_X_FROM_MSG_BUFF_BIT:1;	//bit 4
		 unsigned FLG2_R_BUSY_BIT:1;			//bit 5 ;int: 1=REC BUSY, 0=IDLE
		 unsigned FLG2_R_BARKER_ODD_EVEN_BIT:1;	//bit 6 ;int: 1=odd, 0=even
		 unsigned FLG2_R_BITVAL_BIT:1;			//bit 7 ;int: 
		 }FLAG2_STRUCT;

		}ucFLAG2_BYTE;


/**************************  vMISC_setLED()  **************************************
*
*	May be deleted if LED goes
*
******************************************************************************/

void vMISC_setLED(
		uchar ucLEDstate		//LED_ON or LED_OFF
		)
	{

	if(ucLEDstate)
		DEBUG_LED_PORT |= DEBUG_LED_BIT;
	else
		DEBUG_LED_PORT &= ~DEBUG_LED_BIT;

	return;

	}/* END vMISC_setLED() */







/************************  vMISC_blinkLED()  **************************************
*
*
* 
*****************************************************************************/
void vMISC_blinkLED(
		uchar ucBlinkCount
		)
	{
	uchar uc;

	for(uc=0;  uc<ucBlinkCount;  uc++)
		{
		DEBUG_LED_PORT |= DEBUG_LED_BIT;

		vDELAY_wait100usTic(800);

		DEBUG_LED_PORT &= ~DEBUG_LED_BIT;

		vDELAY_wait100usTic(800);

		}

	return;

	}/* END: vMISC_blinkLED() */










/**************** ulMISC_buildGenericFromBytes()  ***********************
*
* Routine to collect consecutive bytes into an ulong. 
*
******************************************************************************/

static ulong ulMISC_buildGenericFromBytes(
		uchar *ucpBytes,					//src bytes
		uchar ucByteCount,					//number of bytes to build
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	ulong ulTmp;

	if(ucIntFlag)							//disables interrupts
		{
		  __bic_SR_register(GIE);
		}

	ulTmp = 0;
	switch(ucByteCount)
		{
		case 4:
			ulTmp |= (ulong) *ucpBytes++;  
			ulTmp <<= 8;                    
			/* fall through */
		case 3:								
			ulTmp |= (ulong) *ucpBytes++;
			ulTmp <<= 8;
			/* fall through */
		case 2:								
			ulTmp |= (ulong) *ucpBytes++;
			ulTmp <<= 8;
			/* fall through */
		case 1:								
			ulTmp |= (ulong) *ucpBytes;
			/* fall through */
		default:							
			break;

		}/* END: switch() */

	if(ucIntFlag)
		{
		  __bis_SR_register(GIE);
		}

	return(ulTmp);

	}/* END: ulMISC_buildGenericFromBytes() */







/**************** ulMISC_buildUlongFromBytes()  *******************************
*
* Routine to collect 4 consecutive bytes into an ulong. 
*
******************************************************************************/

ulong ulMISC_buildUlongFromBytes(
		uchar *ucpBytes,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	return(ulMISC_buildGenericFromBytes((uchar *)ucpBytes, 4, ucIntFlag));

	}/* END: ulMISC_buildUlongFromBytes() */




/**************** uslMISC_buildUslFromBytes()  ********************************
*
* Routine to collect 3 consecutive bytes into an ulong. 
*
******************************************************************************/

usl uslMISC_buildUslFromBytes(
		uchar *ucpBytes,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	return(ulMISC_buildGenericFromBytes((uchar *)ucpBytes, 3, ucIntFlag));

	}/* END: uslMISC_buildUslFromBytes() */



/**************** uiMISC_buildUintFromBytes()  *******************************
*
* Routine to collect 2 consecutive bytes into an ulong. 
*
******************************************************************************/

uint uiMISC_buildUintFromBytes(
		uchar *ucpBytes,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	uint uiVal;

	uiVal = (uint)ulMISC_buildGenericFromBytes(ucpBytes, 2, ucIntFlag);

	#if 0
	vSERIAL_sout("MISC:uiVal=", 11);
	vSERIAL_HBV16out(uiVal);
	vSERIAL_crlf();
	#endif

	return(uiVal);

	}/* END: uiMISC_buildUintFromBytes() */







/**************** vMISC_copyGenericIntoBytes()  *********************************
*
* Routine to copy up to 4 bytes into bytes
*
******************************************************************************/

static void vMISC_copyGenericIntoBytes(
		ulong ulLongVal,					//src value
		uchar *ucpToPtr,					//dest byte ptr
		uchar ucByteCount,					//number of bytes to copy
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{

	if(ucIntFlag)
		{
		  __bic_SR_register(GIE);
		}

	switch(ucByteCount)
		{
		case 4:					//lint !e616 //long
			*ucpToPtr = (uchar)((ulLongVal >> 24) & 0xFF);
			ucpToPtr++;
			/* fall through */
		case 3:					//lint !e616 //usl
			*ucpToPtr = (uchar)((ulLongVal >> 16) & 0xFF);
			ucpToPtr++;
			/* fall through */
		case 2:					//lint !e616 //int
			*ucpToPtr = (uchar)((ulLongVal >>  8) & 0xFF);
			ucpToPtr++;
			/* fall through */
		case 1:					//lint !e616 //char
			*ucpToPtr = (uchar)(ulLongVal & 0xFF);
			/* fall through */
		default:				//lint !e616
			break;

		}/* END: /switch() */

	if(ucIntFlag)
		{
		  __bis_SR_register(GIE);
		}

	return;

	}/* END: vMISC_copyGenericIntoBytes() */







/**************** vMISC_copyUlongIntoBytes()  *********************************
*
* Routine to copy an ulong into 4 ram bytes
*
******************************************************************************/

void vMISC_copyUlongIntoBytes(
		ulong ulVal,
		uchar *ucpToPtr,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	vMISC_copyGenericIntoBytes(ulVal, (uchar *)ucpToPtr, 4, ucIntFlag);

	return;

	}/* END: vMISC_copyUlongIntoBytes() */




/**************** vMISC_copyUslIntoBytes()  *********************************
*
* Routine to copy an ulong into 4 ram bytes
*
******************************************************************************/

void vMISC_copyUslIntoBytes(
		usl uslVal,
		uchar *ucpToPtr,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	vMISC_copyGenericIntoBytes((ulong)uslVal, (uchar *)ucpToPtr, 3, ucIntFlag);

	return;

	}/* END: vMISC_copyUslIntoBytes() */






/**************** vMISC_copyUintIntoBytes()  *********************************
*
* Routine to copy an ulong into 4 ram bytes
*
******************************************************************************/

void vMISC_copyUintIntoBytes(
		uint uiVal,
		uchar *ucpToPtr,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	vMISC_copyGenericIntoBytes((ulong)uiVal, (uchar *)ucpToPtr, 2, ucIntFlag);

	return;

	}/* END: vMISC_copyUintIntoBytes() */






/**************** vMISC_copy6ByteTo6Byte()  *********************************
*
* Routine to copy a 6byte to a 6byte array
*
******************************************************************************/

void vMISC_copy6ByteTo6Byte(
		uchar *ucpFromPtr,			//Src array ptr
		uchar *ucpToPtr,			//Dest array ptr
		uchar ucIntFlag				//YES_NOINT,  NO_NOINT
		)
	{
	vMISC_copyBytesToBytes(ucpFromPtr, ucpToPtr, 6, ucIntFlag);
	return;

	}/* END: vMISC_copy6ByteTo6Byte() */








/**************** ulMISC_copyBytesToBytes() **********************************
*
* Routine to copy n bytes to n bytes in ram
*
******************************************************************************/

void vMISC_copyBytesToBytes(
		uchar *ucpFromPtr,
		uchar *ucpToPtr,
		uchar ucByteCnt,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{
	uchar ucc;

	if(ucIntFlag)
		{
		  __bic_SR_register(GIE);
		}

	for(ucc=0; ucc<ucByteCnt;  ucc++)
		{
		*ucpToPtr = *ucpFromPtr;
		ucpToPtr++;
		ucpFromPtr++;
		}/* END: for() */

	if(ucIntFlag)
		{
		  __bis_SR_register(GIE);
		}

	return;

	}/* END: vMISC_copyBytesToBytes() */



/**************** vTEMP_MISC_copyBytesToBytes() **********************************
*
* Routine to copy n bytes to n bytes in ram
*
******************************************************************************/

void vMISC_copyPtrToLong(
		uchar *ucpFromPtr,
		ulong uslDest,
		uchar ucByteCnt,
		uchar ucIntFlag						//YES_NOINT,  NO_NOINT
		)
	{

	if(ucIntFlag)
		{
		  __bic_SR_register(GIE);
		}



	if(ucIntFlag)
		{
		  __bis_SR_register(GIE);
		}

	return;

	}/* END: vMISC_copyBytesToBytes() */


/*******************  uiMISC_doCompensatedBattRead() *************************
*
* Special routine to compute the battery voltage even after we have
* dropped below the 3.3V level from the regulator.
*
* NOTE: During some routine battery margin tests we discovered that the 
*		battery voltage computation did not correspond to the actual 
*		external battery voltage.  After some more specific investigative
*		testing we found that when the battery voltage drops the A/D stops
*		reading linearly at about 3.7V.  We also discovered was that under
*		the 3.7V thershold the A/D reports a rising reading but to our good
*		luck this rising reading is nearly linear, so the A/D reading graph
*		has two parts that can be approximated by two linear functions.  The
*		left linear function is approximated by the function:
*						X = .0064935Y and the
*		right linear function is approximated by the function:
*						X = -.046154Y + 29.90769
*		where X is in volts and Y is the A/D reading value.
*
*		The problem to solve was to determine from a single battery reading
*		which portion of the A/D curve (LEFT or RIGHT) we were on.  To 
*		solve this question we used the LVD (LOW VOLTAGE DETECT) functions
*		that are built into the PIC processor.  By interrogating the LVD
*		and determining which side of the A/D curve we are on we can 
*		get an approximate reading of the external voltage in milliVolts
*		that in only off about (+/-) 6 millivolts.  This is close enough
*		for us to estimate the battery strength and protect the processor
*		and other chips from malfunctions due to low voltage.
*
*		The following voltages are the death voltages for components on the
*		brain board: (decending order):
*
*						Fram chip	2.7V
*						Flash chip	2.5V
*						SRAM chip	2.5V
*						Processor	2.2V
*						Radio		2.2V
*						Buzzer		2.0V
*
*		Because the FRAM is essential for long term operation we do not
*		allow the processor to continue running when the external battery
*		voltage falls below about 2.8V.
*
*		In addition we will not allow a restart of the system if the 
*		external battery voltage is below 3.8V.  The assumption here is
*		that there is a good chance that if the system is restarting then
*		a human operator is near, and the batterys can be changed before
*		a unit is deployed in the field with a low battery pack.
*
*
******************************************************************************/

ulong uiMISC_doCompensatedBattRead( 			//RET: Batt Voltage in mV
		void
		)
	{
	uchar ucReadingCount;
	long lResult = 0x00;
	long lVal;
	long lBattReading_in_mV;

	// zero the reading
	lVal = 0L;
	// Take 10 measurements and find the average
	for (ucReadingCount = 0; ucReadingCount < 10; ucReadingCount++) {
		lVal += (long) uiAD_full_init_setup_read_and_shutdown(BATT_AD_CHAN);
	}

	lVal = lVal/10;

	//guarantee the multiplier is not in fractional mode
    //then multiply the ADC reading by (2.5/4096)*2 = 0x2FAF (ignoring floating pt.)
    //for accuracy sake the multiplier has been slightly adjusted
    MPY32CTL0 &= ~MPYFRAC;
    MPY = lVal;
    //OP2 = 0x3F61; //for Vr=AVcc
    OP2 = 0x2FAF;  //for Vr=2.5V

    lResult = RESHI;
    lResult = lResult<<16;
    lResult |= RESLO;

    lBattReading_in_mV = lResult/10000;

	return(lBattReading_in_mV);

	}// END: uiMISC_doCompensatedBattRead()





/****************  ucMISC_sleep_until_button_or_clk() ***********************
* 
*  code here likely to change! needs a going over based on sleep mode selections
* 
* NOTE: This routine returns immediately if alarm time is past current time.
*
* RET:	0  if awakened from sleep by timer alarm
*		1  if awakened from sleep by button push
*		2  if by prince (not really) (alarm has already gone off)
*
******************************************************************************/

uchar ucMISC_sleep_until_button_or_clk( //RET 0=Timer, 1=button, 2=past alarm
		uchar ucHibernateFlag	//SLEEP_MODE or HIBERNATE_MODE(no clk)
		)
	{
	long lCurTime;
	long lAlarmTime;
	uchar ucRetVal;


	/* CONVERT THE ALARM TIME TO A LONG */
	lAlarmTime = lTIME_getAlarmAsLong();
	lCurTime   = lTIME_getSysTimeAsLong();

	if(ucHibernateFlag != HIBERNATE_MODE)
		{
		/* IF CURRENT TIME IS PAST ALARM TIME -- THEN RETURN IMMEDIATELY */
		if(lCurTime >= lAlarmTime)
			{
			ucRetVal = 2;
			goto Sleep_immediate_exit;
			}
		}

	/* SHUT OFF THE SERIAL PORT */
	vSERIAL_quit();

	ADC12CTL0_L &= ~ADC12ENC_L;				//Disable A/D Conversion allowing changes
	ADC12CTL0_L &= ~ADC12ON_L;				//Turn Off A/D
	vDELAY_wait100usTic(20);				//2ms settle time

	vBUTTON_init();

	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;	//clr alarm flag
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 0;	//clr alarm flag

	if(ucHibernateFlag == HIBERNATE_MODE)
		{
			// Decide about hibernation and make change! to all that follows.
//		T1CONbits.TMR1ON = 0;				//shut off T1 clk
//		T1CONbits.T1OSCEN = 0;				//shut off T1 osc
		}
/*
DoSleep:
	_asm
		SLEEP
		NOP									//lint !e40 !e522 !e10
	_endasm;								//lint !e40 !e522 !e10
*/

	ucRetVal = 1;
	if(ucBUTTON_isButtonFlgSet()) goto Wakeup;	//check if its a button

	ucRetVal = 0;
	if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT) goto Wakeup;
	if(ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT) goto Wakeup;

//	goto DoSleep;


Wakeup:

	if(ucHibernateFlag == HIBERNATE_MODE)
		{
//		T1CONbits.T1OSCEN = 1;				//Turn T1 osc
//		T1CONbits.TMR1ON = 1;				//Turn ON T1 clk
		}

	vSERIAL_init();		//500us wait

Sleep_immediate_exit:

	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 0;	//clr alarm flag
	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 0;	//clr alarm flag

	return(ucRetVal);

	}/* END: ucMISC_sleep_until_button_or_clk() */


/* --------------------------  END of MODULE  ------------------------------- */
