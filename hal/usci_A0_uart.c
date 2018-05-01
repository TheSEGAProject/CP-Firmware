/////////////////////////////////////////////////////////////////////////////////
//!	\file usci_A0_uart.c
//!	\brief File for controlling usci A0 uart
//! \addtogroup HAL
//! @{
/////////////////////////////////////////////////////////////////////////////////

#include <msp430.h>
#include "std.h"
#include "usci_A0_uart.h"
#include "time.h"
#include "delay.h"

//! \var g_ucUSCI_A0_UART_RXBuffer
//! \brief Array used to store the messages recieved from the A0 usci
uchar g_ucUSCI_A0_UART_RXBuffer[USCI_A0_BUFFER_LEN];

//! \var g_ucUSCI_A0_UART_RXBufferIndex
//! \brief Index for the A0 UART RX buffer
uchar g_ucUSCI_A0_UART_RXBufferIndex;

/////////////////////////////////////////////////////////////////////////////////
//! \fn vUSCI_A0_UART_init
//! \brief Initializes the periphal for 115.2 kbaud
/////////////////////////////////////////////////////////////////////////////////
void vUSCI_A0_UART_init(void){

	//Config UART for UCA0
	P3SEL |= 0x30;                      // P3.4,5 = USCI_A0 TXD/RXD
	P3DIR &= ~BIT5;
//	P3DIR |= BIT4;

	UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 0x22;                           //4MHz 115200
	UCA0BR1 = 0;                              //4MHz 115200
	UCA0MCTL |= UCBRS_6 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
	UCA0MCTL &= ~UCOS16;  //CLR UCOS16
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

	// Init the RX buffer
	vUSCI_A0_UART_CleanBuff();

}

/////////////////////////////////////////////////////////////////////////////////
//! \fn vUSCI_A0_UART_quit
//! \brief Shutsdown the peripheral
/////////////////////////////////////////////////////////////////////////////////
void vUSCI_A0_UART_quit(void)
{

	// Hold the usci in reset
	UCA0CTL1 |= UCSWRST;

	/* WAIT FOR TRANSMIT TO FINISH */
	while (!(UCA0IFG & UCTXIFG));

	/* DISABLE THE SERIAL PORT */
	UCA0IE &= ~UCRXIE; // Disable USCI_A0 RX interrupt
	UCA0IE &= ~UCTXIE; // Disable USCI_A0 TX interrupt

	// Set the pins to GPIO instead of special function
	P3SEL &= ~0x30;
//	P3DIR &= ~0x30;
	P3OUT &= ~0x30;

	return;
}// END: vUSCI_A0_UART_quit()


/////////////////////////////////////////////////////////////////////////////////
//! \fn vUSCI_A0_UART_CleanBuff
//! \brief Cleans the RX buffer and resets the global index
/////////////////////////////////////////////////////////////////////////////////
void vUSCI_A0_UART_CleanBuff(void){
	uchar ucIndex;

	// Clear all contents in the RX Buffer
	for(ucIndex = 0; ucIndex < USCI_A0_BUFFER_LEN; ucIndex++){
		g_ucUSCI_A0_UART_RXBuffer[ucIndex] = 0;
	}

	// Reset the buffer index
	g_ucUSCI_A0_UART_RXBufferIndex = 0;
}

/////////////////////////////////////////////////////////////////////////////////
//! \fn vUSCI_A0_UART_SendByte
//! \brief Transmits a byte using the usci peripheral
//! \param ucChar, the byte to be sent
/////////////////////////////////////////////////////////////////////////////////
void vUSCI_A0_UART_SendByte(uchar ucChar){

	// Make sure previous transmission is complete
	while (!(UCA0IFG & UCTXIFG));

	//stuff the xmit reg
	UCA0TXBUF = ucChar;

	// Don't exit until finished
	while ((UCA0STAT & UCBUSY));
}

/////////////////////////////////////////////////////////////////////////////////
//! \fn vUSCI_A0_UART_SendByte
//! \brief Transmits several bytes using the usci peripheral
//! \param *ucChar, the pointer to the bytes to be sent
//! \param ucByteCount, the number of bytes to send
/////////////////////////////////////////////////////////////////////////////////
void vUSCI_A0_UART_SendBytes(uchar *ucChar, uchar ucByteCount){

	while(ucByteCount--){
		vUSCI_A0_UART_SendByte(*ucChar++);
	}
}

///////////////////////////////////////////////////////////////////////////////////
////! \fn vUSCI_A0_UART_ReceiveByte
////! \brief Receives a byte using the usci peripheral
////! \param ucChar, the variable to be assigned the incoming byte
////! \return Error code, 0 = success
///////////////////////////////////////////////////////////////////////////////////
//uchar ucUSCI_A0_UART_ReceiveByte(uchar *ucChar){
//
//
//	// Disable USCI_A0 RX interrupt
//	UCA0IE &= ~UCRXIE;
//
//	vTime_SetLPM_DelayAlarm(ON, 100); //100 uS
//	LPM0;
//	vTime_SetLPM_DelayAlarm(OFF, 0);
//
//	if (!(UCA1IFG & UCRXIFG))
//		return 1;
//
//	/* READ THE CHAR */
//	*ucChar = UCA1RXBUF;
//
//	// Enable USCI_A0 RX interrupt
//	UCA0IE |= UCRXIE;
//
//	return 0;
//}


/////////////////////////////////////////////////////////////////////////////////
//! \fn ucUSCI_A0_UART_ReadBuffer
//!	\brief Reads the contents of the usci RX buffer to the passed buffer
//! \param *ucMsgBuff, pointer to the location where the message will be copied
//!	\return Error code, 0 = success
/////////////////////////////////////////////////////////////////////////////////
uchar ucUSCI_A0_UART_ReadBuffer(uchar *ucMsgBuff, uchar ucMaxLength){

	uchar ucByteCount;

	// If there is no message then return an error
	if (g_ucUSCI_A0_UART_RXBufferIndex == 0)
		return 1;

	// If the message in the buffer is too large then return error
	if (g_ucUSCI_A0_UART_RXBufferIndex > ucMaxLength)
		return 1;

	// Copy the message into the desired buffer
	for(ucByteCount = 0; ucByteCount<g_ucUSCI_A0_UART_RXBufferIndex; ucByteCount++){
		*ucMsgBuff++ = g_ucUSCI_A0_UART_RXBuffer[ucByteCount];
	}

	// Reset the Index
	g_ucUSCI_A0_UART_RXBufferIndex = 0;

	return 0;
}

uchar ucUSCI_A0_UART_ReadByte(uchar ucIndex){
	return g_ucUSCI_A0_UART_RXBuffer[ucIndex];
}

/////////////////////////////////////////////////////////////////////////////////
//! \fn ucUSCI_A0_UART_GetIndex
//! \brief Returns the current buffer index
//! \return g_ucUSCI_A0_UART_RXBufferIndex
/////////////////////////////////////////////////////////////////////////////////
uchar ucUSCI_A0_UART_GetIndex(void){

	return g_ucUSCI_A0_UART_RXBufferIndex;
}

/////////////////////////////////////////////////////////////////////////////////
//! \fn ucUSCI_A0_UART_SetIndex
//! \brief Sets the buffer index to the desired value
//! \param ucNewIndex
/////////////////////////////////////////////////////////////////////////////////
void vUSCI_A0_UART_SetIndex(uchar ucNewIndex){

	g_ucUSCI_A0_UART_RXBufferIndex = ucNewIndex;
}

//! @}
