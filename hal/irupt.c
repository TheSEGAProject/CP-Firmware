/////////////////////////////////////////////////////////////////////////////
//! \file irupt.c
//! \brief Interrupt service routines
//! \addtogroup HAL
//! @{
//! V1.00 01/07/2003 wzr
//!		Started
/////////////////////////////////////////////////////////////////////////////
#include <msp430x54x.h>			//processor register description
#include "config.h"			//system configuration definitions
#include "irupt.h"
#include "std.h"
#include "comm.h"
#include "adf7020.h"	//adf7020 transceiver definitions
#include "SP.h"			//Satellite Processor definitions
#include "serial.h"
#include "usci_A0_uart.h"
/*********************************************************
 * 					Flags
 ********************************************************/
extern volatile union //ucFLAG1_BYTE
{
	uchar byte;

	struct
	{
		unsigned FLG1_X_DONE_BIT :1; //bit 0
		unsigned FLG1_X_LAST_BIT_BIT :1; //bit 1
		unsigned FLG1_X_FLAG_BIT :1; //bit 2 ;1=XMIT, 0=RECEIVE
		unsigned FLG1_R_HAVE_MSG_BIT :1; //bit 3	;1=REC has a msg, 0=no msg
		unsigned FLG1_R_CODE_PHASE_BIT :1; //bit 4 ;1=MSG PHASE, 0=BARKER PHASE
		unsigned FLG1_R_ABORT_BIT :1; //bit 5
		unsigned FLG1_X_NXT_LEVEL_BIT :1; //bit 6
		unsigned FLG1_R_SAMPLE_BIT :1; //bit 7
	} FLAG1_STRUCT;

} ucFLAG1_BYTE;

extern volatile union //ucFLAG2_BYTE
{
	uchar byte;

	struct
	{
		unsigned FLG2_T3_ALARM_MCH_BIT :1; //bit 0 ;1=T3 Alarm, 0=no alarm
		unsigned FLG2_T1_ALARM_MCH_BIT :1; //bit 1 ;1=T1 Alarm, 0=no alarm
		unsigned FLG2_T2_ALARM_MCH_BIT :1; //bit 2 ;1=T2 Alarm, 0=no alarm
		unsigned FLG2_CLK_INT_BIT :1; //bit 3	;1=clk ticked, 0=not
		unsigned FLG2_X_FROM_MSG_BUFF_BIT :1; //bit 4
		unsigned FLG2_R_BUSY_BIT :1; //bit 5 ;int: 1=REC BUSY, 0=IDLE
		unsigned FLG2_R_BARKER_ODD_EVEN_BIT :1; //bit 6 ;int: 1=odd, 0=even
		unsigned FLG2_R_BITVAL_BIT :1; //bit 7 ;int:
	} FLAG2_STRUCT;

} ucFLAG2_BYTE;

extern volatile union
{
	uint8 byte;
	struct
	{
		unsigned FLG3_RADIO_ON_BIT :1;
		unsigned FLG3_RADIO_MODE_BIT :1;
		unsigned FLG3_RADIO_PROGRAMMED :1;
		unsigned FLG2_BUTTON_INT_BIT :1; //bit 2 ;1=XMIT, 0=RECEIVE
		unsigned FLG3_LINKSLOT_ALARM :1;
		unsigned FLG3_LPM_DELAY_ALARM :1;
		unsigned FLG3_KEY_PRESSED :1;
		unsigned FLG3_GSV_COM_BIT :1; // 1=GSV com is active 0= GSV com is inactive
	} FLAG3_STRUCT;
} ucFLAG3_BYTE;
//END: Flags


/*********************************************************
 * 						Radio externs and preamble
 *********************************************************/
extern volatile ADF7020_Driver_t ADF7020_Driver;

extern const uint8 g_ucaADF7020_Preamble[ADF7020_PREAMBLE_BYTE_COUNT];
//End Radio

/*******************************************************
 * Serial Comm. externs
 *******************************************************/
extern uchar g_ucUSCI_A0_UART_RXBuffer[USCI_A0_BUFFER_LEN];
extern uchar g_ucUSCI_A0_UART_RXBufferIndex;
extern unsigned char g_ucaUSCI_A1_RXBuffer[0x100];
extern unsigned char g_ucaUSCI_A1_RXBufferIndex;

/******************************************************
 * SPI extern
 ******************************************************/
extern uchar ucByteSent;

/******************************************************
 *  Time keeping externs
 ******************************************************/
extern volatile ulong uslALARM_TIME;
extern volatile ulong uslCLK_TIME;
extern volatile ulong uslCLK2_TIME;
extern volatile uchar g_ucLatencyTimerState;

/*********************************************************
 * SP externs
 ********************************************************/
extern unsigned char g_ucSP1Ready;
extern unsigned char g_ucSP2Ready;
extern unsigned char g_ucSP3Ready;
extern unsigned char g_ucSP4Ready;
extern volatile uint8 g_ucUART_Flags;
///////////////////////////////////////////////////////////////////////////////
//! \brief TimerA0 ISR, sends and receives on the software UART lines
//!
//! TimerA has been configured by to generate an interrupt
//! for the bit timing on a specific baud rate.
//!
//! This ISR handles the timing for both TX and RX. Therefore, it is best to
//! use this module in half-duplex mode only. Other wise you risk the
//! possibility of having a very long ISR, which will kill the delicate UART
//! timing.
//!
//!
//! NOTE: This interrupt is ONLY for handling CCR0, allowing the user to use
//! the TIMERA1 interrupt for other uses of TA CCR1, CCR2 and TAIFG
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	__bic_SR_register_on_exit(LPM4_bits);
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Timer0_A1_ISR, handles radio communication
//!
//!
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1_ISR(void)
{
	switch (TA0IV)
	{
		case TA0IV_NONE:
		case TA0IV_TA0CCR1:
		case TA0IV_TA0CCR2:

			// Buzzer PWM using OUTMOD not ISR
		case TA0IV_TA0CCR3:
		break;

			//Radio Comm
		case TA0IV_TA0CCR4:
			if (ADF7020_Driver.ucTxState & DATACLK_EDGE_BIT) {
				/* Check for end of byte */
				if (ADF7020_Driver.ucTxBitCount > 0x07) {
					/* Reset bit count */
					ADF7020_Driver.ucTxBitCount = 0x00;

					/* Next byte */
					ADF7020_Driver.ucTxBufferPosition++;

					/* Check for end of message */
					if (ADF7020_Driver.ucTxBufferPosition >= (ADF7020_PREAMBLE_BYTE_COUNT + ADF7020_Driver.uConfig.Registers.ulPacketSize)) {
						/* Set the shutdown bit */
						ADF7020_Driver.ucTxState |= TX_SHUTDOWN_BIT;
					}
					else /* We are at the end of byte but not end of message */
					{
						/* Set the active byte based on position */
						if (ADF7020_Driver.ucTxBufferPosition < ADF7020_PREAMBLE_BYTE_COUNT) {
							ADF7020_Driver.ucTxActiveByte = g_ucaADF7020_Preamble[ADF7020_Driver.ucTxBufferPosition];
						}
						else {
							ADF7020_Driver.ucTxActiveByte = ADF7020_Driver.ucaTxBuffer[(unsigned int)(ADF7020_Driver.ucTxBufferPosition - ADF7020_PREAMBLE_BYTE_COUNT)];
						}
					}
				}
				/* Clear the edge bit for next interrupt */
				ADF7020_Driver.ucTxState &= ~DATACLK_EDGE_BIT;
			}
			else /* We are a falling edge, not a rising */
			{
				/* Check for end of transmission */
				if (ADF7020_Driver.ucTxState & TX_SHUTDOWN_BIT) {
					/* Shut off the timer */
					TA0CTL &= ~(MC1 | MC0);

					/* Return to TX idle state */
					ADF7020_Driver.eRadioState = TX_IDLE;
					//Wake up the MCU if it was asleep
					__bic_SR_register_on_exit(LPM4_bits);
				}

				/* Set the DATAIO line depending on the MSB in the data buffer */
				if (ADF7020_Driver.ucTxActiveByte & 0x80) {
					DATAIO_PIN_REG |= DATAIO_PIN_NUMBER;
				}
				else {
					DATAIO_PIN_REG &= ~DATAIO_PIN_NUMBER;
				}

				/* Shift the byte for next time */
				ADF7020_Driver.ucTxActiveByte <<= 1;

				/* Increment the bit count */
				ADF7020_Driver.ucTxBitCount++;

				/* The edge bit for the next interrupt */
				ADF7020_Driver.ucTxState |= DATACLK_EDGE_BIT;
			}
		break;
	} // END: switch(TA0IV)
}

/////////////////////////////////////////////////////////////////////////
//! \brief This ISR handles the TACCR0 generated interrupts used for
//!  the systems real time clock (not the RTC peripheral).
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
#pragma vector=TIMER1_A0_VECTOR
__interrupt void vTIMERA1_A0_ISR(void)
{
	//Increment the primary clock time
	uslCLK_TIME++;

	//Increment the secondary clock time
	uslCLK2_TIME++;

	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T1_ALARM_MCH_BIT = 1; //sets the alarm bit

	TA1CTL &= ~TAIFG;

	__bic_SR_register_on_exit(LPM4_bits);
}

/////////////////////////////////////////////////////////////////////////
//! \brief ISR to handle the structure of a slot.  This allows the
//!  WiSARD to accomplish multiple sensing actions in one slot while maintaining
//!  the timing required for radio communication.
//!
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
#pragma vector=TIMER1_A1_VECTOR
__interrupt void vTIMER1_A1_ISR(void)
{

	switch (__even_in_range(TA1IV, 4))
	{
		case TA1IV_NONE: //no interrupt
			__no_operation();
		break;

			//Subslot warning
		case TA1IV_TA1CCR1:
			//Disable the interrupt and clear the interrupt flag
			TA1CCTL1 &= ~(CCIE | CCIFG);
			ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T2_ALARM_MCH_BIT = 1;
			__bic_SR_register_on_exit(LPM4_bits);
		break;

			//Subslot ended
		case TA1IV_TA1CCR2:
			TA1CCTL2 &= ~(CCIE | CCIFG);
			ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 1;
			__bic_SR_register_on_exit(LPM4_bits);
		break;

		case TA1IV_TA1IFG:
		break;

		default:
		break;
	}

} //END: TIMER1_A1_ISR()


#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)
{
	ENDSLOT_TIM_CTL &= ~ENDSLOT_INTFLG_BIT; //clear the interrupt
}


/////////////////////////////////////////////////////////////////////////
//! \brief ISR
//!
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void)
{
	switch (__even_in_range(TBIV, 14))
	{
		case TB0IV_NONE:
		break; // No interrupt

			// Used for radio link slot divider so we can send multiple beacons within a slot
		case TB0IV_TBCCR1: // CCR1
			ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_LINKSLOT_ALARM = 1;
			__bic_SR_register_on_exit(LPM4_bits);
		break;

		case TB0IV_TBCCR2: //CCR2
			ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_LPM_DELAY_ALARM = 1;
			__bic_SR_register_on_exit(LPM4_bits);
		break;

		case TB0IV_TBCCR3: //Reserved
		break;

		case TB0IV_TBCCR4: //Reserved
		break;

		case TB0IV_TBCCR5: //Reserved
		break;

		case TB0IV_TBCCR6: //Reserved
		break;

		case TB0IV_TB0IFG:
		break; // overflow

		default:
		break;
	}
//	ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 1;
	ENDSLOT_TIM_CTL &= ~ENDSLOT_INTFLG_BIT; //clear the interrupt
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Port1 ISR, handles SP board interrupts as well as user and radio
//!	interrupts
//!
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
#pragma vector=PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
	/* Find the source of the interrupt */
	switch (P1IV)
	{
		case P1IV_P1IFG0:
			g_ucSP1Ready = 1;
			__bic_SR_register_on_exit(LPM4_bits);
		break;
		case P1IV_P1IFG1:
			g_ucSP2Ready = 1;
			__bic_SR_register_on_exit(LPM4_bits);
		break;

		case P1IV_P1IFG2:
			g_ucSP3Ready = 1;
			__bic_SR_register_on_exit(LPM4_bits);
		break;
		case P1IV_P1IFG3:
			g_ucSP4Ready = 1;
			__bic_SR_register_on_exit(LPM4_bits);
		break;

		case P1IV_P1IFG4:
			__bic_SR_register_on_exit(LPM4_bits);
		break;

		case DATACLK_IV_NUMBER:
			/* If we get a DATACLK interrupt, it means a new bit has been output by
			 * the radio and we need to save it. This driver is configured as an MSB
			 * first driver. Thus we will always left shift the current byte FIRST
			 * and THEN write the bit to the LSB. */

			/* Increment the bit count and shit the bit */
			ADF7020_Driver.ucRxBitCount++;
			if (ADF7020_Driver.ucFirstBit) {
				ADF7020_Driver.ucRxBitCount--;
				ADF7020_Driver.ucFirstBit = 0;
			}

			ADF7020_Driver.ucaRxBuffer[ADF7020_Driver.ucRxBufferPosition] <<= 1;

			/* Read the incoming bit and write it to the buffer*/
			if ((DATAIO_IN_REG & DATAIO_PIN_NUMBER) == DATAIO_PIN_NUMBER) {
				ADF7020_Driver.ucaRxBuffer[ADF7020_Driver.ucRxBufferPosition] |= 0x01;
			}
			else {
				ADF7020_Driver.ucaRxBuffer[ADF7020_Driver.ucRxBufferPosition] &= ~0x01;
			}

			/* Check to see if we have received a whole byte. If we have, then
			 * increment the byte position and reset the bit count */
			if (ADF7020_Driver.ucRxBitCount >= 0x08) {
				ADF7020_Driver.ucRxBitCount = 0x00;

				if (ADF7020_Driver.ucRxBufferPosition > 10)
					ADF7020_Driver.uConfig.Registers.ulPacketSize = ((ADF7020_Driver.ucaRxBuffer[10] ^ 0x0A) + 6);

				ADF7020_Driver.ucRxBufferPosition++;
			}

			/* Check for the end of packet. If it is, then shut off the interrupt and
			 * return to RX_IDLE */
			if (ADF7020_Driver.ucRxBufferPosition >= ADF7020_Driver.uConfig.Registers.ulPacketSize) {
				//Disable interrupts until ready for another message
				DATACLK_IE_REG &= ~DATACLK_PIN_NUMBER;
				INTLOCK_IE_REG &= ~INTLOCK_PIN_NUMBER;

				ADF7020_Driver.eRadioState = RX_IDLE;
				//Let the Application layer know that we have a message
				ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 1;

				//Wake up the MCU if it was asleep
				__bic_SR_register_on_exit(LPM4_bits);
			}

		break;

		case INTLOCK_IV_NUMBER:
			/* This means that the ADF7020 has received an incoming sync word. We
			 * need to turn on the interrupt for DATACLK to start gathering incoming
			 * data bits and turn off the INTLOCK interrupt */
			DATACLK_IE_REG |= DATACLK_PIN_NUMBER;
			INTLOCK_IE_REG &= ~INTLOCK_PIN_NUMBER;

			//To measure RX and decoding time start the timer
			LATENCY_TIMER_CTL |= g_ucLatencyTimerState;

			/* New packet, reset all write pointers and counters */
			ADF7020_Driver.ucRxBufferPosition = 0;
			ADF7020_Driver.ucRxBitCount = 0;
			ADF7020_Driver.ucFirstBit = 1;

			/* Transition the state machine */
			ADF7020_Driver.eRadioState = RX_ACTIVE;
		break;

		case P1IV_P1IFG7:
		break;

		case P1IV_NONE:
		default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Port2 ISR, handles the start of the UART RX
//!
//! The idle state for UART is line high, thus when we get the falling edge
//! indicating the start bit, we can start the timer to handle the UART 
//! sampling.
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
	uchar ucii, ucHigh, ucLow; //Variables for debouncing the PWREN line

	// If we get an interrupt from the RX pin, then we have RXed a start bit
	/* Find the source of the interrupt */
	switch (P2IV)
	{

		// IFG 0-3 are used for SP bit banging interface
		case P2IV_P2IFG0:
		case P2IV_P2IFG1:
		case P2IV_P2IFG2:
		case P2IV_P2IFG3:
			g_ucUART_Flags |= F_COMM_RX_BUSY;
			__bic_SR_register_on_exit(LPM4_bits);
		break;

		case P2IV_P2IFG4:
		break;

			// PWREN# pin from the USB chip
		case P2IV_P2IFG5:

			// Do some debouncing logic here.
			ucHigh = 0; // initialize the logic state counters
			ucLow = 0;
			for (ucii = 0; ucii < 10; ucii++) // poll the pin 10 times to avoid spurious interrupts from setting the UART peripheral state
					{
				if (P2IN & BIT5)
					ucHigh++;
				else
					ucLow++;

				__delay_cycles(32); // delay 2 us between each polling
			}

			// Take a majority vote and if the line is high then turn off the USCI and set the IES bit to falling
			if (ucHigh > ucLow) {
				UCA1CTL1 |= UCSWRST;
				P5SEL &= ~(BIT6 | BIT7);
				P2IES |= BIT5;
				UART1_REG_IE &= ~UART1_RX_IE; // disable serial rx interrupts
			}
			else // Turn on the USCI and set the IES bit to rising
			{
				UCA1CTL1 &= ~UCSWRST;
				P5SEL |= (BIT6 | BIT7); // P5.6,7 = USCI_A1 TXD/RXD
				P2IES &= ~BIT5;
				UART1_REG_IE |= UART1_RX_IE; // enable serial rx interrupts
			}
		break;

		case P2IV_P2IFG6:
		break;

			// User interrupt
		case P2IV_P2IFG7:
			// Do some debouncing logic here.
			ucHigh = 0; // initialize the logic state counters
			ucLow = 0;
			for (ucii = 0; ucii < 10; ucii++) // poll the pin 10 times to avoid spurious interrupts from setting the UART peripheral state
					{
				if (P2IN & BIT7)
					ucHigh++;
				else
					ucLow++;

				__delay_cycles(32); // delay 2 us between each polling
			}

		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// vUSCI_A1_ISR()
//
// Interrupt handler for RX on USCI A1. If a character is received on
// USCI_A1, it is stored in a buffer.
//
// The buffer is only capable of holding 256 characters, after that it will
// wrap around and start overwriting at the beginning.
//
// NOTE: IT IS UP TO THE USER TO CHECK THE BUFFER FOR INPUTS, EITHER HERE OR
//       IN THE CODE ELSE WHERE
///////////////////////////////////////////////////////////////////////////////
#pragma vector=USCI_A1_VECTOR
__interrupt void vUSCI_A1_ISR(void)
{

	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_KEY_PRESSED = 1;
	g_ucaUSCI_A1_RXBuffer[g_ucaUSCI_A1_RXBufferIndex] = UCA1RXBUF;

	// if garden server isn't communicating over usci and button press flag bit is low...
	if (ucFLAG3_BYTE.FLAG3_STRUCT.FLG2_BUTTON_INT_BIT == 0 && ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT == 0) {
		if (g_ucaUSCI_A1_RXBuffer[g_ucaUSCI_A1_RXBufferIndex] == 27) {
			// set flag bit
			ucFLAG3_BYTE.FLAG3_STRUCT.FLG2_BUTTON_INT_BIT = 1;
		}
	}

//	// Store received bytes in the USCI RX buffer unless its a carriage return or line feed
//	switch(UCA1RXBUF)
//	{
//		// if keypress is 'esc' set diagnostic flag
//		case (char)27:
//				// if garden server isn't communicating over usci and button press flag bit is low...
//				if(ucFLAG3_BYTE.FLAG3_STRUCT.FLG2_BUTTON_INT_BIT == 0 && ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT == 0)
//				{
//					// set flag bit
//					ucFLAG3_BYTE.FLAG3_STRUCT.FLG2_BUTTON_INT_BIT = 1;
//				}
//		default:
//			g_ucaUSCI_A1_RXBuffer[g_ucaUSCI_A1_RXBufferIndex++] = UCA1RXBUF;
//	}

	g_ucaUSCI_A1_RXBufferIndex++;
	// Prevent buffer overflow
	if (g_ucaUSCI_A1_RXBufferIndex > 0xFF)
		g_ucaUSCI_A1_RXBufferIndex = 0x00;

	__bic_SR_register_on_exit(LPM4_bits);

	// NOTE: The USCA0RXIFG flag is automatically cleared when UCA0RXBUF is read,
	//       no need to clear in manually.
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{

	// Write incoming byte into buffer
	g_ucUSCI_A0_UART_RXBuffer[g_ucUSCI_A0_UART_RXBufferIndex++] = UCA0RXBUF;

	// Reset index
	if (g_ucUSCI_A0_UART_RXBufferIndex >= USCI_A0_BUFFER_LEN)
		g_ucUSCI_A0_UART_RXBufferIndex = 0;

	__bic_SR_register_on_exit(LPM4_bits);
}

#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
{
	// Pet the dog
	WDTCTL = WDTPW + WDTSSEL_1 + WDTCNTCL + WDTIS_3;
}




//////////////////  Unused ISRs ////////////////////////
#pragma vector=DMA_VECTOR
__interrupt void DMA_ISR(void)
{
	__no_operation();
}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
	__no_operation();
}

#pragma vector=USCI_B3_VECTOR
__interrupt void USCI_B3_ISR(void)
{
	__no_operation();
}

#pragma vector=USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
{
	__no_operation();
}
#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
__no_operation();
}
#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
{
	__no_operation();
}

#pragma vector=USCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
{
	__no_operation();
}

#pragma vector=USCI_A2_VECTOR
__interrupt void USCI_A2_ISR(void)
{
	__no_operation();
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
	__no_operation();
}

#pragma vector=UNMI_VECTOR
__interrupt void UNMI_ISR(void)
{
	__no_operation();
}

#pragma vector=SYSNMI_VECTOR
__interrupt void SYSNMI_ISR(void)
{
	__no_operation();
}

//! @} 
