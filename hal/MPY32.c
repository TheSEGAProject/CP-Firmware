/////////////////////////////////////////////////////////////////////////////
//!
//! \file MPY32.c
//!	\brief This file controls the hal interface with the hardware multiplier
//!					of the MSP430F5438a
//! \addtogroup HAL
//! @{
/////////////////////////////////////////////////////////////////////////////

#include "MSP430x54x.h"
#include	"std.h"

/////////////////////////////////////////////////////////////////////////////
//!
//!	\brief This function performs an unsigned multiply
//!
//! \param ulOperand, ulMultiplier
//! \return ulResult
/////////////////////////////////////////////////////////////////////////////
ulong ulMPY32_UnsignedMultiply(ulong ulOperand, ulong ulMultiplier)
{
	ulong ulResult;

	// Clear the result registers
	RESHI = 0x0000;
	RESLO = 0x0000;

	// Make sure the multiplier is not in fractional mode
  MPY32CTL0 &= ~MPYFRAC;

  MPY = ulMultiplier;

  // Writing to this register initiates the multiplication
  OP2 = ulOperand;

  ulResult = RESHI;
  ulResult = ulResult<<16;
  ulResult |= RESLO;

  return ulResult;
}

/////////////////////////////////////////////////////////////////////////////
//!
//!	\brief This function performs an signed multiply
//!
//! \param ulOperand, lMultiplier
//! \return ulResult
/////////////////////////////////////////////////////////////////////////////
ulong ulMPY32_SignedMultiply(ulong ulOperand, long lMultiplier)
{
	long lResult;

	// Make sure the multiplier is not in fractional mode
  MPY32CTL0 &= ~MPYFRAC;

  MPYS = lMultiplier;

  // Writing to this register initiates the multiplication
  OP2 = ulOperand;

  lResult = RESHI;
  lResult = lResult<<16;
  lResult |= RESLO;

  return lResult;
}


// This should be available to the user not the ones above
long lMPY32_32BitMultiply(ulong ulOperand, long lMultiplier, uchar ucMode)
{
__no_operation();

return 1;

}

//! @}

