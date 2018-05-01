#include <msp430x54x.h>
#include "../hal/config.h"
#include "../std.h"
#include "../delay.h"
	

/////////////////////////////////////////////////////////////////////////////
//! \brief Turns off the red LED
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vLED_RedOff(void)
{
	LED_PORT &= ~RED_LED_BIT;
}//end vLED_RedOff

/////////////////////////////////////////////////////////////////////////////
//! \brief Turns on the red LED
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vLED_RedOn(void)
{
	LED_PORT |= RED_LED_BIT;
}//end vLED_RedOn

/////////////////////////////////////////////////////////////////////////////
//! \brief Turns off the green LED
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vLED_GrnOff(void)
{
	LED_PORT &= ~GRN_LED_BIT;
}//end vLED_GrnOff

/////////////////////////////////////////////////////////////////////////////
//! \brief Turns on the green LED
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vLED_GrnOn(void)
{
	LED_PORT |= GRN_LED_BIT;
}//end vLED_GrnOn

void vLED_RedBlink(uint16 ucDuration)
{
	vLED_RedOn();
	vDELAY_wait100usTic(ucDuration);
	vLED_RedOff();
}//end vLED_RedBlink


void vLED_GrnBlink(uint16 ucDuration)
{
	vLED_GrnOn();
	vDELAY_wait100usTic(ucDuration);
	vLED_GrnOff();
}//end vLED_GrnBlink
