#include <msp430.h>
#include "SP.h"
#include "comm.h"
#include "config.h"
#include "std.h"
#include "time.h"

//! \struct S_SP_BSL
//! \brief The structure holds the bit values and registers
//! associated with the SP currently being reprogrammed
struct S_Active_SP_Fields_UART S_SP_BSL;

//******************  Control and Indication Variables  *********************//
//! @name Control and Indication Variables
//! These variables are used to indicate to the system the current status
//! of the \ref SP Module and to store the baud rate timer information.
//! @{
//! \var volatile uint8 g_ucUART_Flags
//! \brief This 8-bit field indicates the status of the COMM module.
volatile uint8 g_ucUART_Flags;

//! \var ucaActiveSPFlag[]
//! \brief This array stores the Active SP board
static const uint8 ucaSPBit[NUMBER_SPBOARDS] =
{ SP1_BIT, SP2_BIT, SP3_BIT, SP4_BIT };

//! \var static const uint8 g_ucaSPRstPins[]
//! \brief This array stores the SP boards Reset pins
static const uint8 g_ucaSPRstPins[NUMBER_SPBOARDS] =
{ SP1_RST_BIT, SP2_RST_BIT, SP3_RST_BIT, SP4_RST_BIT };

//! \var static const uint8 g_ucaSPTckPins[]
//! \brief This array stores the SP boards test clock pins
static const uint8 g_ucaSPTckPins[NUMBER_SPBOARDS] =
{ SP1_TCK_BIT, SP2_TCK_BIT, SP3_TCK_BIT, SP4_TCK_BIT };

//! \var static const uint8 g_ucaSPIntPins[]
//! \brief This array stores the SP boards Interrupt pins
static const uint8 g_ucaSPIntPins[NUMBER_SPBOARDS] =
{ SP1_INT_BIT, SP2_INT_BIT, SP3_INT_BIT, SP4_INT_BIT };

//! \var static const uint8 g_ucaSPEnPins[]
//! \brief This array stores the SP boards enable pins
static const uint8 g_ucaSPEnPins[NUMBER_SPBOARDS] =
{ SP1_EN_BIT, SP2_EN_BIT, SP3_EN_BIT, SP4_EN_BIT };

//! \var uint16 g_unUART_BaudRateControl
//! \brief This is the value used to control the baud rate.
//!
//! This value is the number of timer ticks corresponding to one bit period
//! for the baud rate. It should be set from the \ref comm_baud
//! "Baud Rate Defines".
uint16 g_unUART_BaudRateControl;

//! \var uint16 g_unUART_BaudRateDelayControl
//! \brief This is the value used to delay from the start bit
//!
//! This value is the number of timer ticks to wait from the beginning of the
//! start bit to the middle of the first data bit. It should be set from the
//! \ref SP_baud_delay "Baud Rate Start Delays".
uint16 g_unUART_BaudRateDelayControl;
//! @}

//******************  RX Variables  *****************************************//
//! @name Receive Variables
//! These variables are used in the receiving of data on the \ref SP Module.
//! @{
//! \var volatile uint8 g_ucaSP_UART_RXBuffer[RX_BUFFER_SIZE]
//! \brief The software UART RX Buffer
volatile uint8 g_ucaSP_UART_RXBuffer[RX_BUFFER_SIZE];

//! \var volatile uint8 g_ucRXBufferIndex
//! \brief This index into g_ucaRXBuffer showing the current write position.
volatile uint8 g_ucaSP_UART_RXBufferIndex;

//! \var static const uint8 g_ucaSPRxPins[]
//! \brief This array stores the SP boards Rx pins
static const uint8 g_ucaSPRxPins[NUMBER_SPBOARDS] =
{ SP1_RX_BIT, SP2_RX_BIT, SP3_RX_BIT, SP4_RX_BIT };
//! @}

//! @name Transmit Variables
//! These variables are used for transmitting to the \ref SP Module.
//! @{

//! \var g_ucaSP_UART_RXBuffer[TX_BUFFER_SIZE]
//! \brief The transmit message buffer
volatile uint8 g_ucaSP_UART_TXBuffer[TX_BUFFER_SIZE];

//! @}

//! \var static const uint8 g_ucaSPTxPins[]
//! \brief This array stores the SP boards Tx pins
static const uint8 g_ucaSPTxPins[NUMBER_SPBOARDS] =
{ SP1_TX_BIT, SP2_TX_BIT, SP3_TX_BIT, SP4_TX_BIT };

//******************  Register Variables  *****************************************//
//! @name Register Variables
//! There variables are used for register configuration in the \ref SP
//! Module.
//! @{

//! \var static const uint8 g_ucaSPCtlReg[][]
//! \brief This array stores the control registers of the SP board
//!
//! The elements in this array are the addresses of the SP ports
//!
static volatile uint8 * g_pucSPCtlReg[NUMBER_SPBOARDS][2] =
{
{ &P_SP1_DIR, &P_SP1_OUT },
{ &P_SP2_DIR, &P_SP2_OUT },
{ &P_SP3_DIR, &P_SP3_OUT },
{ &P_SP4_DIR, &P_SP4_OUT } };


///////////////////////////////////////////////////////////////////////////////
//! \brief Sets pin configurations that change during runtime
//!
//!
//!   \param ucSPNumber
//!   \return 0,1 success or failure
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_SetActiveBoard(uint8 ucSPNumber)
{

	if (ucSP_IsAttached(ucSPNumber))
	{
		S_SP_BSL.m_ucActiveSP = ucaSPBit[ucSPNumber];
		S_SP_BSL.m_ucActiveSP_BitTx = g_ucaSPTxPins[ucSPNumber];
		S_SP_BSL.m_ucActiveSP_BitRx = g_ucaSPRxPins[ucSPNumber];
		S_SP_BSL.m_ucActiveSP_BitRst = g_ucaSPRstPins[ucSPNumber];
		S_SP_BSL.m_ucActiveSP_BitTck = g_ucaSPTckPins[ucSPNumber];
		S_SP_BSL.m_ucActiveSP_BitEn = g_ucaSPEnPins[ucSPNumber];
		S_SP_BSL.m_ucActiveSP_BitInt = g_ucaSPIntPins[ucSPNumber];
		S_SP_BSL.m_ucActiveSP_RegDir = g_pucSPCtlReg[ucSPNumber][SP_DIRReg];
		S_SP_BSL.m_ucActiveSP_RegOut = g_pucSPCtlReg[ucSPNumber][SP_OUTReg];
		return 0;
	}

	//the requested board is not attached
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Sets pin configurations and initializes the data structures used by
//! the SP driver in UART mode
//!
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_UARTInit(uchar ucSPNumber)
{
	// Set the S_SP_BSL structure for the target SP
 	ucSP_SetActiveBoard(ucSPNumber);

 	//initialize the communication flag
	g_ucUART_Flags = 0x00;

	// Set the initial pin configurations
	switch (ucSPNumber)
	{

		case SP1:
			P_SP1_DIR |= (SP1_TX_BIT | SP1_EN_BIT | SP1_RST_BIT | SP1_TCK_BIT); //set the SCL, enable, reset, and tck pins as outputs
			P_SP1_SEL &= ~(SP1_TX_BIT | SP1_EN_BIT | SP1_RST_BIT | SP1_TCK_BIT); //set the SCL, enable, reset, and tck pins as GPIO
			P_SP1_OUT &= ~(SP1_TX_BIT | SP1_EN_BIT | SP1_RST_BIT | SP1_TCK_BIT); //set the SCL, enable, reset, and tck pins low

			// Set the direction of the serial data line to output, low, no interrupts
				P_SDA_DIR &= ~SP1_RX_BIT;
				P_SDA_OUT &= ~SP1_RX_BIT;
				P_SDA_IE &= ~SP1_RX_BIT;

			break;

		case SP2:
			P_SP2_DIR |= (SP2_TX_BIT | SP2_EN_BIT | SP2_RST_BIT | SP2_TCK_BIT); //set the SCL, enable, reset, and tck pins as outputs
			P_SP2_SEL &= ~(SP2_TX_BIT | SP2_EN_BIT | SP2_RST_BIT | SP2_TCK_BIT); //set the SCL, enable, reset, and tck pins as GPIO
			P_SP2_OUT &= ~(SP2_TX_BIT | SP2_EN_BIT | SP2_RST_BIT | SP2_TCK_BIT); //set the SCL, enable, reset, and tck pins low

			P_SDA_DIR &= ~SP2_RX_BIT;
			P_SDA_OUT &= ~SP2_RX_BIT;
			P_SDA_IE &= ~SP2_RX_BIT;
			break;

		case SP3:
			P_SP3_DIR |= (SP3_TX_BIT | SP3_EN_BIT | SP3_RST_BIT | SP3_TCK_BIT); //set the SCL, enable, reset, and tck pins as outputs
			P_SP3_SEL &= ~(SP3_TX_BIT | SP3_EN_BIT | SP3_RST_BIT | SP3_TCK_BIT); //set the SCL, enable, reset, and tck pins as GPIO
			P_SP3_OUT &= ~(SP3_TX_BIT | SP3_EN_BIT | SP3_RST_BIT | SP3_TCK_BIT); //set the SCL, enable, reset, and tck pins low

			// Set the direction of the serial data line to output, low, no interrupts
				P_SDA_DIR &= ~SP3_RX_BIT;
				P_SDA_OUT &= ~SP3_RX_BIT;
				P_SDA_IE &= ~SP3_RX_BIT;
			break;

		case SP4:
			P_SP4_DIR |= (SP4_TX_BIT | SP4_EN_BIT | SP4_RST_BIT | SP4_TCK_BIT); //set the SCL, enable, reset, and tck pins as outputs
			P_SP4_SEL &= ~(SP4_TX_BIT | SP4_EN_BIT | SP4_RST_BIT | SP4_TCK_BIT); //set the SCL, enable, reset, and tck pins as GPIO
			P_SP4_OUT &= ~(SP4_TX_BIT | SP4_EN_BIT | SP4_RST_BIT | SP4_TCK_BIT); //set the SCL, enable, reset, and tck pins low

			// Set the direction of the serial data line to output, low, no interrupts
			P_SDA_DIR &= ~SP4_RX_BIT;
			P_SDA_OUT &= ~SP4_RX_BIT;
			P_SDA_IE &= ~SP4_RX_BIT;
			break;
	}

	// Setup the interrupt lines
	P_INT_DIR &= ~(SP1_INT_BIT | SP2_INT_BIT | SP3_INT_BIT | SP4_INT_BIT);

	return COMM_OK;
}


///////////////////////////////////////////////////////////////////////////////
//! \brief This prepares the driver for communication with a selected SP board
//!
//! Since we are doing UART without the USCI, we use TimerA and it's interrupt
//! to control the baud rate. The TX and RX pins are completely controllable
//! during run time. The software UART expects 1 start bit, 8 data bits, 1
//! parity bit and 1 stop bit.
//!
//!
//!   \param ucBaud The baud rate define to use, ucSPnumber The SP board in use
//!   \return None
//!   \sa vCOMM_SendByte(), TIMERA0_ISR()
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_UARTSetCommState(uint16 ucBaud, uint8 ucSPNumber)
{
	//initialize the communication flag
	g_ucUART_Flags = 0x00;

	//Set the transmit pin
	*S_SP_BSL.m_ucActiveSP_RegDir |= g_ucaSPTxPins[ucSPNumber];

	//Set the Receive pins
	P_RX_DIR &= ~S_SP_BSL.m_ucActiveSP_BitRx;
	// Enable the falling edge interrupt on RX to see start bits
	P_RX_IES |= S_SP_BSL.m_ucActiveSP_BitRx;
	P_RX_IFG &= ~S_SP_BSL.m_ucActiveSP_BitRx;
	P_RX_IE |= S_SP_BSL.m_ucActiveSP_BitRx;

	// Set the interrupt pin low
	P_INT_DIR |= S_SP_BSL.m_ucActiveSP_BitInt;
	P_INT_OUT &= ~S_SP_BSL.m_ucActiveSP_BitInt;

	// Clear the RX buffer and reset index
	for (g_ucaSP_UART_RXBufferIndex = 0x00; g_ucaSP_UART_RXBufferIndex < RX_BUFFER_SIZE; g_ucaSP_UART_RXBufferIndex++)
	{
		g_ucaSP_UART_RXBuffer[g_ucaSP_UART_RXBufferIndex] = 0x00;
	}
	g_ucaSP_UART_RXBufferIndex = 0x00;

	// BUG FIX: Clear TACTL in case someone was using it before us
	TA0CTL = 0x0000;
	TA0EX0 = 0x00;

	// Hold TimerA in reset
	TA0CTL &= ~(MC0 | MC1);

	// Use the SMCLK, enable CCR0 interrupt
	TA0CTL |= TASSEL_2;
	TA0CCTL0 |= CCIE;

	// The timer interrupt controls the baud rate, currently configured for a
	// 4 MHz SMCLK
	g_unUART_BaudRateControl = ucBaud;
	switch (g_unUART_BaudRateControl)
	{
		case BAUD_9600:
			g_unUART_BaudRateDelayControl = BAUD_9600_DELAY;
		break;

		case BAUD_19200:
			g_unUART_BaudRateDelayControl = BAUD_19200_DELAY;
		break;

		case BAUD_57600:
			g_unUART_BaudRateDelayControl = BAUD_57600_DELAY;
		break;

		case BAUD_115200:
			g_unUART_BaudRateDelayControl = BAUD_115200_DELAY;
		break;

		case BAUD_230400:
			g_unUART_BaudRateDelayControl = BAUD_230400_DELAY;
		break;

		case BAUD_345600:
			g_unUART_BaudRateDelayControl = BAUD_345600_DELAY;
		break;

		case BAUD_460800:
			g_unUART_BaudRateDelayControl = BAUD_460800_DELAY;
		break;

		default:
			g_unUART_BaudRateControl = BAUD_9600;
			g_unUART_BaudRateDelayControl = BAUD_9600_DELAY;
		break;
	}

	TA0CCR0 = g_unUART_BaudRateControl;

	g_ucUART_Flags |= F_COMM_RUNNING;

	return COMM_OK;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Resets the index of the receive buffer
///////////////////////////////////////////////////////////////////////////////
void vSP_UART_ResetRXBufferIndex(void){
	g_ucaSP_UART_RXBufferIndex = 0;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Sends a byte via the software UART
//!
//! This function pushes \e ucChar into the global TX buffer, where the
//! TimerA ISR can access it. The system drops into LPM0, keeping the SMCLK
//! alive for TimerA. The ISR handles the start and stop bits as well as the
//! baud rate. This is a blocking call and will not return until the software
//! has sent the entire message.
//!   \param ucChar The 8-bit value to send
//!   \return None
///////////////////////////////////////////////////////////////////////////////
void vSP_UART_TXByte(uint8 ucTXChar)
{

	uint8 ucParityBit;
	uint8 ucBitIdx;
	uint8 ucTXBitsLeft;

	// If we are already busy, return
	if (g_ucUART_Flags & F_COMM_TX_BUSY)
		return;

	P_RX_IE &= ~S_SP_BSL.m_ucActiveSP_BitRx;

	// Indicate in the status register that we are now busy
	g_ucUART_Flags |= F_COMM_TX_BUSY;

	// Calculate the parity bit prior to transmission
	ucParityBit = 0;
	for (ucBitIdx = 0; ucBitIdx < 8; ucBitIdx++)
		ucParityBit ^= ((ucTXChar >> ucBitIdx) & 0x01);

	// Reset the bit count so the ISR knows how many bits left to send
	ucTXBitsLeft = 0x0A;

	TA0CCR0 = g_unUART_BaudRateControl;
	TA0CCTL0 &= ~CCIFG;
	TA0CCTL0 |= CCIE;

	// Starts the counter in 'Up-Mode'
	TA0CTL |= (TACLR | MC_1);

	while (g_ucUART_Flags & F_COMM_TX_BUSY)
	{

		// wait in LPM until the end of the bit
		LPM0;

		switch (ucTXBitsLeft)
		{
			case 0x00:
				// Last bit is stop bit, return to idle state
				*S_SP_BSL.m_ucActiveSP_RegOut |= S_SP_BSL.m_ucActiveSP_BitTx;
				g_ucUART_Flags &= ~F_COMM_TX_BUSY;
			break;

			case 0x01:
				if (ucParityBit)
					*S_SP_BSL.m_ucActiveSP_RegOut |= S_SP_BSL.m_ucActiveSP_BitTx;
				else
					*S_SP_BSL.m_ucActiveSP_RegOut &= ~S_SP_BSL.m_ucActiveSP_BitTx;
			break;

			case 0x0A:
				// if else statement used to keep bit timing uniform with the rest of the message
				if (ucTXChar & 0x01)
					*S_SP_BSL.m_ucActiveSP_RegOut &= ~S_SP_BSL.m_ucActiveSP_BitTx;
				else
					*S_SP_BSL.m_ucActiveSP_RegOut &= ~S_SP_BSL.m_ucActiveSP_BitTx;
			break;

			default:
				// For data bits, mask to get correct value and the shift for next time
				if (ucTXChar & 0x01)
					*S_SP_BSL.m_ucActiveSP_RegOut |= S_SP_BSL.m_ucActiveSP_BitTx;
				else
					*S_SP_BSL.m_ucActiveSP_RegOut &= ~S_SP_BSL.m_ucActiveSP_BitTx;
				ucTXChar >>= 1;
			break;
		}

		// Decrement the total bit count
		ucTXBitsLeft--;

	}

	P_RX_IE |= S_SP_BSL.m_ucActiveSP_BitRx;

	// Stop the timer
	TA0CTL &= ~(MC0 | MC1 | TAIFG);
	TA0CCTL0 &= ~(CCIE | CCIFG);

}

///////////////////////////////////////////////////////////////////////////////
//! \brief Receives a byte via the software UART
//!
//! Returns an error if the board does not respond.  Parity bit errors are
//! handled after transmission.
//!
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_UART_RXByte(void)
{
	uint8 ucRXBitsLeft;
	uint8 ucParityBit; // The calculated parity bit
	uint8 ucRxParityBit; // The received parity bit

	ucParityBit = 0; // start the parity bit at 0

	// Start the LPM timer
	vTime_SetLPM_DelayAlarm(ON, 60000);// most of the time we will receive a start bit and exit LPM

	// Wait in LPM for the start bit
	LPM0;

	// Turn off the timer
	vTime_SetLPM_DelayAlarm(OFF, 0);

	P2IE &= ~0x0F; // Disable interrupts on the RX pin
	TA0CTL = (TASSEL_2 | TACLR);
	TA0CCTL0 &= ~(CCIE | CCIFG);

	//If a port interrupt was received then we are in the RX active state else exit
	if (!(g_ucUART_Flags & F_COMM_RX_BUSY))
	{
		return COMM_ERROR;
	}

	ucRXBitsLeft = 0x09;

	// Start timer and delay for half a bit
	TA0CCR0 = g_unUART_BaudRateDelayControl;
	TA0CTL |= MC_1;
	while (!(TA0CTL & TAIFG));

	TA0CTL &= ~TAIFG;

	// Set up timer for comm. at the baud rate
	TA0CTL = (TASSEL_2 | TACLR);
	TA0CCTL0 = CCIE;
	TA0CCR0 = g_unUART_BaudRateControl;

	// Start the timer
	TA0CTL |= MC_1;

	while (g_ucUART_Flags & F_COMM_RX_BUSY)
	{
		switch (ucRXBitsLeft)
		{
			case 0x00:
				// There are no bits left, so lets reset all the values and stop timer
				TA0CTL = (TASSEL_2 | TACLR);
				P_RX_IFG &= ~S_SP_BSL.m_ucActiveSP_BitRx;
				P_RX_IE |= S_SP_BSL.m_ucActiveSP_BitRx;
				g_ucUART_Flags &= ~F_COMM_RX_BUSY;
			break;

				// Parity Bit
			case 0x01:
				if (P_RX_IN & S_SP_BSL.m_ucActiveSP_BitRx)
					ucRxParityBit = 1;
				else
					ucRxParityBit = 0;

				LPM0;
			break;

				// Last data bit no shift
			case 0x02:
				if (P_RX_IN & S_SP_BSL.m_ucActiveSP_BitRx)
				{
					g_ucaSP_UART_RXBuffer[g_ucaSP_UART_RXBufferIndex] |= 0x80;
					ucParityBit ^= 1;
				}
				else
				{
					g_ucaSP_UART_RXBuffer[g_ucaSP_UART_RXBufferIndex] &= ~0x80;
					ucParityBit ^= 0;
				}
				LPM0;
			break;

			default:
				if (P_RX_IN & S_SP_BSL.m_ucActiveSP_BitRx)
				{
					g_ucaSP_UART_RXBuffer[g_ucaSP_UART_RXBufferIndex] |= 0x80;
					ucParityBit ^= 1;
				}
				else
				{
					g_ucaSP_UART_RXBuffer[g_ucaSP_UART_RXBufferIndex] &= ~0x80;
					ucParityBit ^= 0;
				}

				// Shift over for the next bit then sleep
				g_ucaSP_UART_RXBuffer[g_ucaSP_UART_RXBufferIndex] >>= 1;
				LPM0;
			break;
		}
		ucRXBitsLeft--;
	}

	// Set the parity bit error flag
	if(ucParityBit != ucRxParityBit)
		g_ucUART_Flags |= F_COMM_PARITY_ERR;

	// Increment index for next byte
	g_ucaSP_UART_RXBufferIndex++;
	return COMM_OK;
}


///////////////////////////////////////////////////////////////////////////////
//! \brief Shuts off the software modules
//!
//! This shuts down TimerA and disables all of the interrupts used. The vSP_ShutdownComm()
//! function must be used when switching between SP boards otherwise the communication
//! will fail.
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
void vSP_UART_ShutdownComm(void)
{
	// Halt timer and clear interrupt enables
	TA0CTL &= ~(MC0 | MC1 | TAIE | TAIFG);
	TA0CCTL0 &= ~CCIE;

	//Disable active SP
	S_SP_BSL.m_ucActiveSP = 0xFF;

	//Disable interrupts on the RX line
	P_RX_IE &= ~S_SP_BSL.m_ucActiveSP_BitRx;

	// Pull interrupt line low
	P_INT_OUT &= ~S_SP_BSL.m_ucActiveSP_BitInt;

	//Indicate the comm channel is inactive
	g_ucUART_Flags &= ~F_COMM_RUNNING;
}


///////////////////////////////////////////////////////////////////////////////
//! \brief Waits for reception of a data message
//!
//! This function waits for a data message to be received on the serial port.
//!   \param None.
//!   \return The error code indicating the status after call
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_UART_WaitForMessage(void)
{

	uint8 ucRXMessageSize;

	// Set the size of the received message to the minimum
	ucRXMessageSize = SP_HEADERSIZE;

	// Wait to receive the message
	do // saves 3 cycles as opposed to while
	{
		if (ucSP_UART_RXByte()) {
			return COMM_ERROR;
		}

		// If we have received the header, update the RX message length
		if (g_ucaSP_UART_RXBufferIndex == SP_HEADERSIZE) {
			ucRXMessageSize = g_ucaSP_UART_RXBuffer[SP_MSG_LEN_IDX];

			// Range check the ucRXMessageSize variable
			if (ucRXMessageSize > MAX_SP_MSGSIZE || ucRXMessageSize < SP_HEADERSIZE)
				return COMM_ERROR;
		}
	}
	while (g_ucaSP_UART_RXBufferIndex != (ucRXMessageSize + CRC_SZ));

	// No message received
	if (g_ucaSP_UART_RXBufferIndex == 0) {
		return COMM_ERROR;
	}

  //success
	return COMM_OK;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Sends a data message on the serial port
//!
//! This function sends the message in the TX buffer
//!
//!   \param none
//!   \return None
///////////////////////////////////////////////////////////////////////////////
void vSP_UART_SendMessage(uchar *ucMsg, uint uiLength)
{
	uint8 ucLoopCount;

	for (ucLoopCount = 0x00; ucLoopCount < uiLength; ucLoopCount++)
		vSP_UART_TXByte(*ucMsg++);
}
