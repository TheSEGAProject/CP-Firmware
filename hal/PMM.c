/////////////////////////////////////////////////////////////////////////////
//!
//! \file PMM.c
//!	\brief This file controls the hal interface with power management options
//!					of the MSP430F5438a
//!
//!		This device is capable of changing its core voltage to meet ULP demands
//!		at different clock frequencies.
//!
//! \addtogroup HAL
//! @{
/////////////////////////////////////////////////////////////////////////////

#include <msp430.h>
#include "std.h"

/////////////////////////////////////////////////////////////////////////////
//!
//!
//! RET:	2 = OK Voltage
//!		1 = Bad Detector (Timed out)
//!		0 = Low Voltage
//! This function tests the voltage on the MSP.  The high side detector tests the
//! voltage applied at DVcc and the low side detector(not in use here) checks the core voltage, Vcore.
/////////////////////////////////////////////////////////////////////////////
uint8 ucPMM_chkLowVoltage(//0=Ok Voltage, 1=Bad Detector, 2=Low Voltage
		uint8 ucLowVoltLevel /* LOW_V300, LOW_V194 ... (see defines) */
		)
	{
	uint8 ucii;

	PMMCTL0_H = PMMPW_H;			// Password to allow PMM access
	PMMRIE &= ~(SVMHIE+SVSMLDLYIE);	// make sure Interrupts are disabled
	SVSMHCTL |= ucLowVoltLevel;		// Low Volt Detection Limit
	SVSMHCTL_H |= SVMHE;			// turn on the high side detector

	for(ucii=0;  ucii<200; ucii++)	/* no more than 50us to stabilize - needs change! for timing */
		{
		if(PMMIFG & SVSMHDLYIFG) break; //SVSMHDLYIFG is a delay bit, when 1 we are good to continue
		}
	if(ucii >= 200) return(1);		/* timeout Detector is bad or too slow */

	PMMIFG_L &= ~SVMHIFG_L;				/* clr possible spurious int */

	for(ucii=0;  ucii<200;  ucii++)
		{
		if(PMMIFG & SVMHIFG)
			{
			SVSMHCTL_H |= SVMHE;			/* turn off Detector to save power */
			PMMCTL0_H = 0xA6;				//wrong Password to block PMM access
			return(2);	/* found low voltage */
			}
		}/* END: for() */

	SVSMHCTL_H |= SVMHE;			/* turn off Detector to save power */
	PMMCTL0_H = 0xA6;				//wrong Password to block PMM access
	return(0);						/* voltage OK */

	}/* END: ucMISC_chkLowVoltage() */

//////////////////////////////////////////////////////////////////
//!
//! \brief toggles the software power on reset pin.
//!
//!
//////////////////////////////////////////////////////////////////
void vPMM_Reset(void)
{
	PMMCTL0 |= PMMSWPOR;
}


////////////////////////////////////////////////////////////////
//!
//! \brief Sets the core voltage based on the main clock speed
//! Assumes that Vcc>2.4 V
//!
//! \param ucDCOFreq (MHz)
////////////////////////////////////////////////////////////////
void vPMM_SetCoreVoltage(uint8 ucDCOFreq)
{

	uint8 ucCurrentCoreV, ucNewCoreV;
	uint16 uiSVSHRstLvl, uiSVSHRstRlsLvl, uiSVSLRstLvl, uiSVSLRstRlsLvl;

	// Get the proper core setting based on the MCLK frequency
	if (ucDCOFreq <= 8)
		ucNewCoreV = 0x00;
	else if (ucDCOFreq <= 12)
		ucNewCoreV = 0x01;
	else if (ucDCOFreq <= 20)
		ucNewCoreV = 0x02;
	else
		ucNewCoreV = 0x03;

	// pwd to access PMM registers
	PMMCTL0_H = 0xA5;

	// Adjusting the voltage must be performed one increment at a time, hence the while loop
	// While the core voltage is not equal to the desired voltage
	while((PMMCTL0_L & 0x0003) != ucNewCoreV)
	{
		ucCurrentCoreV = PMMCTL0_L & 0x0003;

		// if we are increasing the core voltage
		if (ucCurrentCoreV < ucNewCoreV)
		{
			// Increment the coreV vaiable
			ucCurrentCoreV += 1;

			// Increment the high-side reset and reset release levels, Bit mask to avoid overflow
			uiSVSHRstLvl = ((SVSMHCTL & SVSHRVL_3) + SVSHRVL_1) & SVSHRVL_3;
			uiSVSHRstRlsLvl = ((SVSMHCTL & SVSMHRRL_7) + SVSMHRRL_1) & SVSMHRRL_7;

			// Program the high-side SVM and SVS to the next level to ensure DVcc is high enough
			SVSMHCTL = (SVSHE | uiSVSHRstLvl | SVMHE | uiSVSHRstRlsLvl);

			// Increment the low-side reset release level, Bit mask to avoid overflow
			uiSVSLRstRlsLvl = ((SVSMLCTL & SVSMLRRL_7) + SVSMLRRL_1) & SVSMLRRL_7;

			// Program low-side SVM to the next level and wait for delay IFG
			SVSMLCTL = (SVSLE | SVMLE | uiSVSLRstRlsLvl);
			while ((PMMIFG & SVSMLDLYIFG) == 0);

			// Clear already set flags
			PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);

			// Set VCore to new level
			PMMCTL0_L = ucCurrentCoreV;

			// Wait till new level reached
			if ((PMMIFG & SVMLIFG))
			while ((PMMIFG & SVMLVLRIFG) == 0);

			// Increment the low-side reset level.  Bit mask to avoid overflow
			uiSVSLRstLvl = ((SVSMLCTL & SVSLRVL_3) + SVSLRVL_1) & SVSLRVL_3;
			// Set SVS/SVM low side to new level
			SVSMLCTL = (SVSLE | uiSVSLRstLvl | SVMLE | uiSVSLRstRlsLvl);


		}
		// We are decreasing the core voltage
		else
		{
			// Decrement the coreV vaiable
			ucCurrentCoreV -= 1;

			// Decrement (if possible) the low-side reset release level, Bit mask to avoid overflow
			if((SVSMLCTL & SVSMLRRL_7) != 0)
			uiSVSLRstRlsLvl = ((SVSMLCTL & SVSMLRRL_7) - SVSMLRRL_1);

			// Decrement (if possible) the low-side reset level.  Bit mask to avoid overflow
			if((SVSMLCTL & SVSLRVL_3) != 0)
			uiSVSLRstLvl = ((SVSMLCTL & SVSLRVL_3) - SVSLRVL_1);

			// Set SVS/SVM low side to new level
			SVSMLCTL = (SVSLE | uiSVSLRstLvl | SVMLE | uiSVSLRstRlsLvl);
			while ((PMMIFG & SVSMLDLYIFG) == 0);

			// Set VCore to new level
			PMMCTL0_L = ucCurrentCoreV;

			// Wait till new level reached
			if ((PMMIFG & SVMLIFG))
			while ((PMMIFG & SVMLVLRIFG) == 0);

		}


	}

	// Lock PMM registers for write access
	PMMCTL0_H = 0x00;

}

//! @}
