//////////////////////////////////////////////////////////////////////////////////
//! \file rom.c
//! \brief Rom Config Byte and Rom Serial Number retrieval.
//!
//!
//!
//////////////////////////////////////////////////////////////////////////////////


/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */


#include "std.h"			//standard defines
#include "ROM.h"				//serial number handler


static const unsigned char ucSN_HI = 0x06;
static const unsigned char ucSN_LO = 0x84;


/****************************  CODE  *****************************************/


//////////////////////////////////////////////////////////////////////////////////
//! \fn uiROM_getRomConfigSnumAsUint
//! \brief Returns the locally defined serial number of the device
//!
//!	\param none
//! \return none
//////////////////////////////////////////////////////////////////////////////////
uint uiROM_getRomConfigSnumAsUint(
		void
		)
	{
	uint uiSN;

	uiSN = ucSN_HI;
	uiSN <<= 8;
	uiSN |= (uint) ucSN_LO; 

	return(uiSN);

	}/* END: uiROM_getRomConfigSnumAsUint() */




/* --------------------------  END of MODULE  ------------------------------- */
