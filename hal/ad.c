/////////////////////////////////////////////////////////////////////////////
//! \file ad.c
//! \addtogroup HAL
//! @{
/////////////////////////////////////////////////////////////////////////////
/*******************************  AD.C  **************************************
*
* This is the module to read the system A/D.
*
* V1.00 09/23/2002 wzr
*		Started.
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



#include <msp430x54x.h>		//processor reg definitions
#include "config.h" 		//system configuration definitions
#include "ad.h"


//! \def SCALEFACTOR
//! \brief The scaling factor for the temperature readings
#define SCALEFACTOR		100

//! \def TEMPERATURE_SCALED_SLOPE_CAL
//! \brief Temperature calibration parameter scaled by 100
#define TEMPERATURE_SCALED_SLOPE_CAL	((85-30)*SCALEFACTOR)

/***************************  vAD_init()  *************************************
*
*
*
*
******************************************************************************/
void vAD_init(
		void
		)
	{
		
	// SET ALL ADC INPUT PINS TO PERIPHERAL FROM I/O
	P6SEL |= ADC_INPUTS_PORT6;
	P5SEL |= ADC_INPUTS_PORT5; //there are no ADC inputs used on Port 5

   	// CLEAR THE INTERRUPT FLAGS
	ADC12IFG = 0;

  ADC12CTL0 = ADC12ON | ADC12SHT03 | ADC12SHT13;
  ADC12CTL1 = ADC12SHP | ADC12CONSEQ_1 | ADC12SSEL_1;    // Pulse Sampling / Sequence / Software Start
  ADC12CTL2 = ADC12RES_2;
  ADC12MCTL0 = ADC12INCH_10 | ADC12SREF_1;

	return;

	}/* END: vAD_init() */





/*********************  vAD_setup_analog_bits()  *****************************
*
*
*
*
******************************************************************************/

void vAD_setup_analog_bits(
		unsigned char ucChanNum
		)
	{
	ucChanNum &= 0x0F; 				// scrub off spurious bits 


	switch(ucChanNum)
		{
		case BATT_AD_CHAN:							// BATTERY

			REFCTL0 = (REFMSTR | REFVSEL_2 | REFON | REFTCOFF);   // Ref 2.5V
			BAT_VIN_PORT_DIR &= ~BAT_VIN_BIT;		// voltage pin input
			BAT_SEN_EN_PORT_DIR |= BAT_SEN_EN_BIT;	// batt sense enable pin output
			BAT_SEN_EN_PORT_OUT |= BAT_SEN_EN_BIT;	// enable batt sense
			break;

		case INTERNAL_TEMP:

		  /* Initialize the shared reference module */
		  REFCTL0 = REFMSTR + REFVSEL_0 + REFON;    // Enable internal 1.5V reference
		break;

		default:
			break;

		}/* END: switch() */

	return;

	}/* END: vAD_setup_analog_bits() */






/********************  vAD_select_chan() *************************************
*
*
*
******************************************************************************/

void vAD_select_chan(
		unsigned char ucChanNum,		// chan num (0-15)
		unsigned char ucWaitType		// YES_WAIT, NO_WAIT
		)
	{
	/* SELECT THE CHANNEL */
	ADC12MCTL0 &= 0xF0;					// scrub off chan bits
	ucChanNum &= 0x0F;
	ADC12MCTL0 |= ucChanNum;			// set the channel

	if(ucWaitType) 	__delay_cycles(1600);

	return;								// leave

	}/* END: vAD_select_chan() */








/*********************  vAD_clearout_analog_bits()  **************************
*
*
*
*
******************************************************************************/

void vAD_clearout_analog_bits(
		unsigned char ucChanNum
		)
	{
	ucChanNum &= 0x0F; 									// scrub off spurious bits 

	switch(ucChanNum)
		{
		case BATT_AD_CHAN:								//BATTERY
			BAT_VIN_PORT_DIR &= ~BAT_VIN_BIT;		// voltage pin input
			BAT_SEN_EN_PORT_OUT &= ~BAT_SEN_EN_BIT;	// enable batt sense

			REFCTL0 &= ~(REFMSTR | REFON);
			break;


		case INTERNAL_TEMP:
		  // Shutdown the shared reference module
		  REFCTL0 &= ~(REFMSTR | REFON);
		break;


		default:
			break;

		}/* END: switch() */

	return;

	}/* END: vAD_clearout_analog_bits() */








/***************************  uiAD_read() ************************************
*
*
*
******************************************************************************/

unsigned int uiAD_read(
		unsigned char ucChanNum,		//(0 - 15)
		unsigned char ucChanSelectFlag	//(YES_SELECT_CHAN, NO_SELECT_CHAN)
		)
	{
	unsigned int uiAD_val;

	/* SELECT THE CHANNEL IF REQUESTED */
	if(ucChanSelectFlag) vAD_select_chan(ucChanNum, YES_WAIT);

	/* STARTUP A CONVERSION */
 ADC12CTL0 |= (ADC12SC | ADC12ENC); 

	/* WAIT FOR CONVERSION */
	while(!(ADC12IFG & ADC12IFG0));

	/* READ THE VALUE */
	uiAD_val = ADC12MEM0;

	// Disable conversion
 ADC12CTL0 &= ~(ADC12SC | ADC12ENC);

	return(uiAD_val);					//leave

	}/* END: uiAD_read() */




/******************  uiAD_full_init_setup_read_and_shutdown() ****************
*
* This is the lazy mans AD read (the way its supposed to be)
*
******************************************************************************/

unsigned int uiAD_full_init_setup_read_and_shutdown(
		unsigned char ucChanNum		//(0 - 15)
		)
	{
	unsigned int uiAD_val;

	/* INIT */
	vAD_init();

	/* SETUP THE ANALOG BITS */
	vAD_setup_analog_bits(ucChanNum);

	/* READ THE VALUE */
	uiAD_val = uiAD_read(ucChanNum, YES_SELECT_CHAN);

	/* DESELECT THE ANALOG BITS */
	vAD_clearout_analog_bits(ucChanNum);

	/* QUIT THE AD */
	vAD_quit();

	return(uiAD_val);					//leave

	}/* END: uiAD_full_init_setup_read_and_shutdown() */





/***************************  vAD_quit()  *************************************
*
*
*
*
******************************************************************************/

void vAD_quit(
		void
		)
	{

	/* TURN OFF A/D */
	ADC12CTL0 &= ~ADC12ON;				//shut off A/D
	P6SEL &= ~ADC_INPUTS_PORT6;			//turn port 6 back to digital
	P5SEL &= ~ADC_INPUTS_PORT5;			//turn port 5 back to digital

	}/* END: vAD_quit() */


////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Calibrates the MCU temperature readings using the two calibration
//! constants stored in the device descriptor table.
//!
//! \param uiTempRaw
//!	\return uiTempCal
////////////////////////////////////////////////////////////////////////////////
unsigned int uiAD_CalibrateTemp(unsigned int uiTempRaw)
{
	unsigned int Cal30;
	unsigned int Cal85;
	unsigned int uislopeScaled;
	unsigned int uiTempCal;

	// Get the calibration constants from memory
	unsigned int *AD_REFV15T30 = (unsigned int*) 0x1A1A;
	unsigned int *AD_REFV15T85 = (unsigned int*) 0x1A1C;

	Cal30 = *AD_REFV15T30;
	Cal85 = *AD_REFV15T85;
	uislopeScaled = (TEMPERATURE_SCALED_SLOPE_CAL / (Cal85 - Cal30));

	uiTempCal = (uiTempRaw - Cal30) * uislopeScaled + (30*SCALEFACTOR);

	return uiTempCal;
}
/**************************  MODULE END  *************************************/
//! @}
