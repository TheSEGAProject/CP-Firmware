///////////////////////////////////////////////////////////////////////////
//! \file
//!
//!
//! addtogroup HAL
//!
///////////////////////////////////////////////////////////////////////////

#include <msp430x54x.h>		//processor reg description */
#include "config.h"			//system configuration description file
#include "PMM.h"


unsigned char g_ucFrequency = 0;

void vUCS_InitializeClock(void)
{
	//disable the FLL
	__bis_SR_register(SCG0);

	// Setup XT1, MCLK, SMCLK, ACLK
	UCSCTL6 = (XT2OFF | XT1DRIVE_3 | XCAP_1); // XT2 Off

	// Loop until XT1,XT2 & DCO stabilizes
	do
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG); // Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG; // Clear fault flags
	}
	while (SFRIFG1 & OFIFG); // Test oscillator fault flag
	UCSCTL6 &= ~(XT1DRIVE_3);				// XT1 is now stable, reduce drive strength

	UCSCTL1 = DCORSEL_16MHZ; // Select DCO range
	UCSCTL2 = DCO_MULT_16MHZ; // Set DCO Multiplier
	UCSCTL4 = SELM__DCOCLK + SELS__DCOCLK + SELA__XT1CLK; // Select sources MCLK, SMCLK, ACLK
	UCSCTL5 = DIVS__4; // Select source divide MCLK = 0, SMCLK = 4, ACLK = 0

	//enable the FLL
	__bic_SR_register(SCG0);

	// Set the global
	g_ucFrequency = DCO_16MHZ;
}


///////////////////////////////////////////////////////////////////////////////////
//!
//! \Sets the frequency of the DCO, MCLK, SMCLK. ACLK is always sourced from XTL1
//!
//!
///////////////////////////////////////////////////////////////////////////////////
void vClock_SetFrequency(unsigned char ucFrequency)
{
	// Monitor the clock frequency on external pins
	#ifdef CLOCK_OUT
		P11SEL = 0x07;
		P11DIR = 0x07;
	#endif

		// Set core voltage first if we are increasing the frequency
		if(ucFrequency > g_ucFrequency)
			vPMM_SetCoreVoltage(ucFrequency);

		///////////////////////Setting the UCS registers////////////////////////////////////

		//selects the XT1 clock as a reference and a divider for the FLL reference
		UCSCTL3 = (SELREF_0 | FLLREFDIV_0);
		UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | XT1HFOFFG | DCOFFG);

		//Disable the FLL
		__bis_SR_register(SCG0);

		//clear any DCO and MOD bits
		UCSCTL0 &= 0x00;

		//cap. for external clock
		UCSCTL6 = (XT2DRIVE_0 | XT2OFF | XCAP_1 | XT1DRIVE_3);

		switch (ucFrequency)
		{
			case DCO_1MHZ:
				//selects a frequency range 1MHz
				UCSCTL1 &= 0x00;
				UCSCTL1 |= (DCORSEL_2);

				//loop divider and and multiplier bits
				UCSCTL2 = (FLLD_0 + 30); //without the appropriate value here the DCOFFG can be set

				//SMCLK source divided by 1  (1 MHz)
				UCSCTL5 |= DIVS_0;
			break;

			case DCO_16MHZ:
				//selects a frequency range 16MHz
				UCSCTL1 &= 0x00;
				UCSCTL1 |= (DCORSEL_5);

				//loop divider and and multiplier bits
				UCSCTL2 = (FLLD_0 + 487); //without the appropriate value here the DCOFFG can be set

				//SMCLK source divided by 4  (4 MHz)
				UCSCTL5 |= DIVS_2;
			break;

		}

		__bic_SR_register(SCG0);
		//Enable FLL

		//time to settle the DCO n*32*32*(F_MCLK/32768)
		if (ucFrequency == DCO_1MHZ)
		{
			__delay_cycles(31250);
		}
		else // 16MHZ settle time
		{
		__delay_cycles(500000);
		}

		// Loop until XT1,XT2 & DCO stabilizes
		do
		{
			UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | XT1HFOFFG | DCOFFG);
			SFRIFG1 &= ~OFIFG;
		}
		while (SFRIFG1 & OFIFG);

		//AClk = XT1, SMClk and MClk = DCO
		UCSCTL4 = (SELA_0 | SELS_3 | SELM_3);

		// Enable requests for the clock sources by peripherals
		UCSCTL8 |= (MODOSCREQEN | SMCLKREQEN | MCLKREQEN | ACLKREQEN);

		// Set core voltage last if we are decreasing the frequency
		if(ucFrequency > g_ucFrequency)
			vPMM_SetCoreVoltage(ucFrequency);

		// Set the global
		g_ucFrequency = ucFrequency;

}

//! @}
