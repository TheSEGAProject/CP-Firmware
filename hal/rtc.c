/////////////////////////////////////////////////////////////////////////////
//!
//! \file rtc.c
//!	\brief This file provides functionality for the real-time clock peripheral
//!
//! \addtogroup HAL
//! @{
/////////////////////////////////////////////////////////////////////////////

#include <msp430F5438.h>
#include "std.h"

/////////////////////////////////////////////////////////////////////////////////
//! \brief Initializes the real time clock
//!
//! Starts the RTC module in counter mode, sourced from ACLK. RTCPS0 and RTCPS1 are 
//! cascaded with the appropriate dividers to get 1 second intervals
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////////
void vRTC_Init(void)
{
	// Setup RTC Timer
	RTCCTL01 = RTCSSEL_2 + RTCTEV_0;			 					// Counter Mode, RTC1PS, 8-bit ovf                                          						
	RTCPS0CTL = RT0PSDIV_7;                   					// ACLK, /256, start timer
	RTCPS1CTL = RT1SSEL_2 + RT1PSDIV_6 + RT1IP_6 + RT1PSIE;       // out from RT0PS, /128, start timer/RTPS1 interrupt enabled
}  
  

/////////////////////////////////////////////////////////////////////////////////
//! \brief Writes the value of the counter, in seconds, into the 4 RTCNT registers 
//!
//! \param ulTimeinSec, the time to set the counter to
//! \return none
/////////////////////////////////////////////////////////////////////////////////
void vRTC_SetCounter(uint32 ulTimeinSec)
{
  uint32 ulTMP;
  //halt the counter to avoid erronious writing to the registers while in operation
	RTCCTL1 |= RTCHOLD;
	
	ulTMP = ulTimeinSec;
	
	//Write the new time in seconds into the 4 counter registers 
	RTCNT1 = (uint8)(ulTMP);
	ulTMP = (ulTMP>>8);
	RTCNT2 = (uint8)(ulTMP);
	ulTMP = (ulTMP>>8);
	RTCNT3 = (uint8)(ulTMP);
	ulTMP = (ulTMP>>8);
	RTCNT4 = (uint8)(ulTMP);
	
	//Restart Clock
	RTCCTL1 &= ~RTCHOLD;

}//END vRTC_SetCounter(uint32);


/////////////////////////////////////////////////////////////////////////////////
//! \brief Reads the value in the RTCNT registers
//!
//! \param none
//! \return ulTimeinSec, the time in the RTCNT registers as a long
/////////////////////////////////////////////////////////////////////////////////
uint32 ulRTC_ReadCounter(void)
{
	uint32 ulTimeinSec; 	
	
	ulTimeinSec = 0x00;
	
	ulTimeinSec |= RTCNT4;
	ulTimeinSec = (ulTimeinSec<<8);
	ulTimeinSec |= RTCNT3;
	ulTimeinSec = (ulTimeinSec<<8);
	ulTimeinSec |= RTCNT2;
	ulTimeinSec = (ulTimeinSec<<8);
	ulTimeinSec |= RTCNT1;
	
  return ulTimeinSec;
}//END ulRTC_ReadCounter() 	

//! @}
