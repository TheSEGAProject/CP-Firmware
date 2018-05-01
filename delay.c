
/*******************************  DELAY.C  ***********************************
*
* This is the module to perform delays  
* 
* will need to change! to actually reflect the amount of time expected
*
*
* V1.00 10/01/2002 wzr
*		Started.
*		
******************************************************************************/



/*lint -e714 */		/* symbol not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e768 */		/* global struct member not referenced */

#include <msp430.h>
#include "time.h"
#include "delay.h"
#include "std.h"

void vDELAY_wait4us(
		void
		)
	{

	__delay_cycles(56);

	return;

	}/* END: vDELAY_wait4us() */



void vDELAY_wait100us(
		void
		)
	{
	__delay_cycles(1600);

	return;

	}/* END: vDELAY_wait100us() */




void vDELAY_wait100usTic(
		unsigned int uiCount
		)
	{

	for( ;uiCount>0 ;uiCount--)
		{
		vDELAY_wait100us();
		}

	return;

	}/* END: vDELAY_wait100usTic() */


///////////////////////////////////////////////////////////////////
//!
//! \brief Sends the MCU into low power mode for the specified time
//!
//!
//!	\param ulDelay, ucSleepMode
//!	\return none
///////////////////////////////////////////////////////////////////
void vDELAY_LPMWait1us(ulong ulDelay, uchar ucSleepMode)
{
	// Set the alarm
	vTime_SetLPM_DelayAlarm(T_START, ulDelay);

	// Sleep until the alarm is set
	do {
		switch (ucSleepMode)
		{
			case 0:
				LPM0;
			break;

			case 1:
				LPM1;
			break;

			case 2:
				LPM2;
			break;

			default:
				LPM0;
			break;
		}
	}while (ucTimeCheckForAlarms(LPM_DELAY_ALARM_BIT) == 0);

	// Turn off the alarm and clear the timer registers
	vTime_SetLPM_DelayAlarm(T_STOP, 0);

}
