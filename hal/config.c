/////////////////////////////////////////////////////////////////////////////
//! \file config.c
//! \addtogroup HAL

/**************************  config.C  *****************************************
*
* Routines to handle basic hardware configuration.
*
*
* V3.00 3/20/2013 cp
*		Another config.c file was written to read from the pic18 registers and has not been
*		ported but can be referenced in earlier revisions of the operational code
*
******************************************************************************/


#include <msp430x54x.h>		//processor reg description */
#include "../std.h"			//standard definitions
#include "config.h"			//system configuration description file



void vConfig_InitializePorts(void)
	{

	// SET PORT DIRECTIONS
	PADIR = PADIR_INIT_VAL;
	PBDIR = PBDIR_INIT_VAL;
	PCDIR = PCDIR_INIT_VAL;
	PDDIR = PDDIR_INIT_VAL;
	PEDIR = PEDIR_INIT_VAL;
	PFDIR = PFDIR_INIT_VAL;

	//SET OUTPUT VALUES
	PAOUT = PAOUT_INIT_VAL;
	PBOUT = PBOUT_INIT_VAL;
	PCOUT = PCOUT_INIT_VAL;
	PDOUT = PDOUT_INIT_VAL;
	PEOUT = PEOUT_INIT_VAL;
	PFOUT = PFOUT_INIT_VAL;

	// SET PORT PULL UP/DOWN RESISTOR
	PAREN = PAREN_INIT_VAL;
	PBREN = PBREN_INIT_VAL;
	PCREN = PCREN_INIT_VAL;
	PDREN = PDREN_INIT_VAL;
	PEREN = PEREN_INIT_VAL;
	PFREN = PFREN_INIT_VAL;

	// SET PORT FUNCTION I/O OR PERIPHERAL
	PASEL = PASEL_INIT_VAL;
	PBSEL = PBSEL_INIT_VAL;
	PCSEL = PCSEL_INIT_VAL;
	PDSEL = PDSEL_INIT_VAL;
	PESEL = PESEL_INIT_VAL;
	PFSEL = PFSEL_INIT_VAL;

	// SET PORTA INTERRUPT ENABLES
	PAIE = PAIE_INIT_VAL;

	// SET PORTA INTERRUPT EDGE (0 = RISING EDGE, 1 = FALLING)
	PAIES = PAIES_INIT_VAL;

	// Clear the interrupt flags
	PAIFG = 0x0000;

	// Increase the drive strength for the b0 usci
	P3DS |= 0x06;
	}

//! @}
