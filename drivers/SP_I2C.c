#include <msp430x54x.h>
#include "SP.h"
#include "comm.h"
#include "config.h"
#include "std.h"
#include "serial.h"
#include "time.h"
#include "crc.h"
#include "delay.h"

//! \struct S_ActiveSP
//! \brief The structure holds the bit values and registers
//! associated with the active satellite processor
struct S_Active_SP_Fields_I2C S_ActiveSP;

//! \struct
//! \brief The structure holds the bit values and registers
//! associated with the active satellite processor
struct S_Attached_SP_Fields S_AttachedSP;

//******************  Control and Indication Variables  *********************//
//! @name Control and Indication Variables
//! These variables are used to indicate to the system the current status
//! of the \ref SP Module and to store the baud rate timer information.
//! @{
//! \var volatile uint8 g_ucCOMM_Flags
//! \brief This 8-bit field indicates the status of the COMM module.
uint8 g_ucCOMM_Flags;

//! \var ucaActiveSPFlag[]
//! \brief This array stores the Active SP board
const uint8 ucaSPBit[NUMBER_SPBOARDS] = { SP1_BIT, SP2_BIT, SP3_BIT, SP4_BIT };

//! \var static const uint8 g_ucaRstPins[]
//! \brief This array stores the SP boards Reset pins
static const uint8 g_ucaRstPins[NUMBER_SPBOARDS] = { SP1_RST_BIT, SP2_RST_BIT, SP3_RST_BIT, SP4_RST_BIT };

//! \var static const uint8 g_ucaTckPins[]
//! \brief This array stores the SP boards test clock pins
static const uint8 g_ucaTckPins[NUMBER_SPBOARDS] = { SP1_TCK_BIT, SP2_TCK_BIT, SP3_TCK_BIT, SP4_TCK_BIT };

//! \var static const uint8 g_ucaIntPins[]
//! \brief This array stores the SP boards Interrupt pins
static const uint8 g_ucaIntPins[NUMBER_SPBOARDS] = { SP1_INT_BIT, SP2_INT_BIT, SP3_INT_BIT, SP4_INT_BIT };

//! \var static const uint8 g_ucaEnPins[]
//! \brief This array stores the SP boards enable pins
static const uint8 g_ucaEnPins[NUMBER_SPBOARDS] = { SP1_EN_BIT, SP2_EN_BIT, SP3_EN_BIT, SP4_EN_BIT };

//! \enum volatile SP_DriverState_t g_eSP_DriverState
//! \brief This enum indicates the state of the SP driver
//!
//! This tells the system if the hardware and other common variables
//! have been set properly before beginning calls to the SPs.
volatile SP_DriverState_t g_eSP_DriverState = SP_DRIVER_SHUTDOWN;

//! \var uint16 g_unCOMM_BaudRateControl
//! \brief This is the value used to control the baud rate.
//!
//! This value is the number of timer ticks corresponding to one bit period
//! for the baud rate. It should be set from the \ref comm_baud
//! "Baud Rate Defines".
uint16 g_unCOMM_BaudRateControl;

//! \var uint16 g_unCOMM_BaudRateDelayControl
//! \brief This is the value used to delay from the start bit
//!
//! This value is the number of timer ticks to wait from the beginning of the
//! start bit to the middle of the first data bit. It should be set from the
//! \ref SP_baud_delay "Baud Rate Start Delays".
uint16 g_unCOMM_BaudRateDelayControl;
//! @}

//******************  RX Variables  *****************************************//
//! @name Receive Variables
//! These variables are used in the receiving of data on the \ref SP Module.
//! @{
//! \var volatile uint8 g_ucaSP_RXBuffer[RX_BUFFER_SIZE]
//! \brief The software UART RX Buffer
uint8 g_ucaSP_RXBuffer[RX_BUFFER_SIZE];

//! \var volatile uint8 g_ucRXBufferIndex
//! \brief This index into g_ucaRXBuffer showing the current write position.
volatile uint8 g_ucaSP_RXBufferIndex;

//! \var static const uint8 g_ucaSDAPins[]
//! \brief This array stores the SP boards Rx pins
static const uint8 g_ucaSDAPins[NUMBER_SPBOARDS] = { SP1_RX_BIT, SP2_RX_BIT, SP3_RX_BIT, SP4_RX_BIT };
//! @}

//! @name Transmit Variables
//! These variables are used for transmitting to the \ref SP Module.
//! @{

//! \var g_ucaSP_RXBuffer[TX_BUFFER_SIZE]
//! \brief The transmit message buffer
volatile uint8 g_ucaTXBuffer[TX_BUFFER_SIZE];

//! @}

//! \var static const uint8 g_ucaSPTxPins[]
//! \brief This array stores the SP boards Tx pins
static const uint8 g_ucaSCLPins[NUMBER_SPBOARDS] = { SP1_TX_BIT, SP2_TX_BIT, SP3_TX_BIT, SP4_TX_BIT };

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
static volatile uint8 * g_pucSPCtlReg[NUMBER_SPBOARDS][2] = { { &P_SP1_DIR, &P_SP1_OUT }, { &P_SP2_DIR, &P_SP2_OUT }, { &P_SP3_DIR, &P_SP3_OUT }, {
		&P_SP4_DIR, &P_SP4_OUT } };

union SP_DataMessage g_SendDataMsg;
union SP_DataMessage g_RecDataMsg;

///////////////////////////////////////////////////////////////////////////////
//! \brief Turns on an SP board
//!
//! This turns on the selected SP board; function only called within the module.
//! 
//!
//!   \param ucSPNumber
//!   \return None
///////////////////////////////////////////////////////////////////////////////
static void vSP_TurnOn(uint8 ucSPNumber)
{
	switch (ucSPNumber)
	{
		case SP1:
			P_SDA_DIR |= SP1_SDA_BIT;
			P_SP1_OUT |= (SP1_EN_BIT | SP1_SCL_BIT | SP1_RST_BIT);
			P_SDA_OUT |= SP1_SDA_BIT;
		break;

		case SP2:
			P_SDA_DIR |= SP2_SDA_BIT;
			P_SP2_OUT |= (SP2_EN_BIT | SP2_SCL_BIT | SP2_RST_BIT);
			P_SDA_OUT |= SP2_SDA_BIT;
		break;

		case SP3:
			P_SDA_DIR |= SP3_SDA_BIT;
			P_SP3_OUT |= (SP3_EN_BIT | SP3_SCL_BIT | SP3_RST_BIT);
			P_SDA_OUT |= SP3_SDA_BIT;
		break;

		case SP4:
			P_SDA_DIR |= SP4_SDA_BIT;
			P_SP4_OUT |= (SP4_EN_BIT | SP4_SCL_BIT | SP4_RST_BIT);
			P_SDA_OUT |= SP4_SDA_BIT;
		break;

		default:
		break;
	}

	// Allow time for the device to power up and initialize
	vDELAY_LPMWait1us(1000,1);

	// Clear interrupt flags and enable interrupts on hardware lines
	P_INT_IFG &= ~g_ucaIntPins[ucSPNumber];
	P_INT_IE |= g_ucaIntPins[ucSPNumber];
}

//
/////////////////////////////////////////////////////////////////////////////
//! \brief Turns off an SP board
//!
//! This turns off the selected SP board; function only called within the module.
//!   \param ucSPNumber
//!   \return None
///////////////////////////////////////////////////////////////////////////////
void vSP_TurnOff(uint8 ucSPNumber)
{
	// Disable interrupts from the SP
	P_INT_IE &= ~g_ucaIntPins[ucSPNumber];

	switch (ucSPNumber)
	{
		case SP1:
			P_SP1_OUT &= ~(SP1_EN_BIT | SP1_SCL_BIT | SP1_RST_BIT);
			P_SDA_OUT &= ~SP1_SDA_BIT;
		break;

		case SP2:
			P_SP2_OUT &= ~(SP2_EN_BIT | SP2_SCL_BIT | SP2_RST_BIT);
			P_SDA_OUT &= ~SP2_SDA_BIT;
		break;

		case SP3:
			P_SP3_OUT &= ~(SP3_EN_BIT | SP3_SCL_BIT | SP3_RST_BIT);
			P_SDA_OUT &= ~SP3_SDA_BIT;
		break;

		case SP4:
			P_SP4_OUT &= ~(SP4_EN_BIT | SP4_SCL_BIT | SP4_RST_BIT);
			P_SDA_OUT &= ~SP4_SDA_BIT;
		break;

		default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Shuts off all active SP boards
//!
//! This shuts off all the SP boards in use.
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
void vSP_TurnoffAll(void)
{
	uchar ucSPNumber;

	for (ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++)
		vSP_TurnOff(ucSPNumber);
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Powers up all active SP boards
//!
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
void vSP_TurnonAll(void)
{
	uchar ucSPNumber;

	for (ucSPNumber = 0; ucSPNumber < 4; ucSPNumber++)
		vSP_TurnOn(ucSPNumber);
}


///////////////////////////////////////////////////////////////////////////////
//! \brief Configures the pins to allow the SPs to be reprogrammed using a FET
//! while sourcing power from the CP
//!
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
void vSP_OnboardProgramming(void)
{
vSP_TurnonAll();

P_SP1_DIR &= ~(SP1_RST_BIT | SP1_TCK_BIT);
P_SP2_DIR &= ~(SP2_RST_BIT | SP2_TCK_BIT);
P_SP3_DIR &= ~(SP3_RST_BIT | SP3_TCK_BIT);
P_SP4_DIR &= ~(SP4_RST_BIT | SP4_TCK_BIT);

while(1);
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Removes an SP board from the attached SP structure
//!
///////////////////////////////////////////////////////////////////////////////
static void vSP_DetachBoard(uchar ucSP_Number)
{
	uchar ucCounter;

	//Inform the driver that the SP is not attached
	S_AttachedSP.m_ucAttachedSPs &= ~ucaSPBit[ucSP_Number];

	// Set the message version to 0
	S_AttachedSP.m_ucaSP_MSG_Version[ucSP_Number] = 0;

	//Set the name of the SP board into the SP name array to "EMTY"
	S_AttachedSP.m_ucaSP_Name[ucSP_Number][0x00] = 0x45;
	S_AttachedSP.m_ucaSP_Name[ucSP_Number][0x01] = 0x4D;
	S_AttachedSP.m_ucaSP_Name[ucSP_Number][0x02] = 0x54;
	S_AttachedSP.m_ucaSP_Name[ucSP_Number][0x03] = 0x59;

	// Clear remaining name field
	for (ucCounter = 4; ucCounter < SP_NAME_LENGTH; ucCounter++) {
		S_AttachedSP.m_ucaSP_Name[ucSP_Number][ucCounter] = 0;
	}

	// Clear the transducer types and sample duration
	for (ucCounter = 0; ucCounter < S_AttachedSP.m_ucaSP_NumTransducers[ucSP_Number]; ucCounter++) {
		S_AttachedSP.m_ucaSP_TypeTransducers[ucSP_Number][ucCounter] = 0x00;
		S_AttachedSP.m_ucaSP_TransSmplDur[ucSP_Number][ucCounter] = 0x00;
	}

	// Clear the hardware ID
	for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
		S_AttachedSP.m_ucaSP_SerialNumber[ucSP_Number][ucCounter] = 0x00;
	}
	S_AttachedSP.m_ucaSP_NumTransducers[ucSP_Number] = 0x00;

	// Set the state of the board to inactive
	vSP_SetSPState(ucSP_Number, SP_STATE_INACTIVE);

} //END vSP_DetachBoard

///////////////////////////////////////////////////////////////////////////////
//! \brief Sets pin configurations that change during runtime
//!
//!
//!   \param ucSPNumber
//!   \return 0,1 success or failure
///////////////////////////////////////////////////////////////////////////////
static uint8 ucSP_SetActiveBoard(uint8 ucSPNumber)
{

	uint8 ucSPBit;

	ucSPBit = ucaSPBit[ucSPNumber];

if (S_AttachedSP.m_ucAttachedSPs & ucSPBit)
	{
	S_ActiveSP.m_ucActiveSP = ucaSPBit[ucSPNumber];
	S_ActiveSP.m_ucActiveSP_BitSCL = g_ucaSCLPins[ucSPNumber];
	S_ActiveSP.m_ucActiveSP_BitSDA = g_ucaSDAPins[ucSPNumber];
	S_ActiveSP.m_ucActiveSP_BitRst = g_ucaRstPins[ucSPNumber];
	S_ActiveSP.m_ucActiveSP_BitTck = g_ucaTckPins[ucSPNumber];
	S_ActiveSP.m_ucActiveSP_BitEn = g_ucaEnPins[ucSPNumber];
	S_ActiveSP.m_ucActiveSP_BitInt = g_ucaIntPins[ucSPNumber];
	S_ActiveSP.m_ucActiveSP_RegDir = g_pucSPCtlReg[ucSPNumber][SP_DIRReg];
	S_ActiveSP.m_ucActiveSP_RegOut = g_pucSPCtlReg[ucSPNumber][SP_OUTReg];

	return 0;
	}

	//the requested board is not attached
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Sets pin configurations and initializes the data structures used by
//! the SP driver in I2C mode
//!
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_Init(void)
{
	uchar ucSP_Number;

	// Don't allow this function to run if the driver is already running.
	if (g_eSP_DriverState != SP_DRIVER_SHUTDOWN) {
		return SP_BAD_STATE;
	}

	S_ActiveSP.m_ucActiveSP = 0xFF;
	S_ActiveSP.m_ucActiveSP_BitSCL = NULL;
	S_ActiveSP.m_ucActiveSP_BitSDA = NULL;
	S_ActiveSP.m_ucActiveSP_BitRst = NULL;
	S_ActiveSP.m_ucActiveSP_BitTck = NULL;
	S_ActiveSP.m_ucActiveSP_BitEn = NULL;
	S_ActiveSP.m_ucActiveSP_BitInt = NULL;
	S_ActiveSP.m_ucActiveSP_RegDir = NULL;
	S_ActiveSP.m_ucActiveSP_RegOut = NULL;

	//Initialize the attached SP structure
	S_AttachedSP.m_ucAttachedSPs = 0x00;

	// Set the Attached_SP data structure to a known state
	for (ucSP_Number = 0; ucSP_Number < NUMBER_SPBOARDS; ucSP_Number++) {
		vSP_DetachBoard(ucSP_Number);
	}

	//initialize the communication flag
	g_ucCOMM_Flags = 0x00;

// Set the initial pin configurations
	//SP1
	P_SP1_DIR |= (SP1_SCL_BIT | SP1_EN_BIT | SP1_RST_BIT | SP1_TCK_BIT); //set the SCL, enable, reset, and tck pins as outputs
	P_SP1_SEL &= ~(SP1_SCL_BIT | SP1_EN_BIT | SP1_RST_BIT | SP1_TCK_BIT); //set the SCL, enable, reset, and tck pins as GPIO
	P_SP1_OUT &= ~(SP1_SCL_BIT | SP1_EN_BIT | SP1_RST_BIT | SP1_TCK_BIT); //set the SCL, enable, reset, and tck pins low

	//SP2
	P_SP2_DIR |= (SP2_SCL_BIT | SP2_EN_BIT | SP2_RST_BIT | SP2_TCK_BIT); //set the SCL, enable, reset, and tck pins as outputs
	P_SP2_SEL &= ~(SP2_SCL_BIT | SP2_EN_BIT | SP2_RST_BIT | SP2_TCK_BIT); //set the SCL, enable, reset, and tck pins as GPIO
	P_SP2_OUT &= ~(SP2_SCL_BIT | SP2_EN_BIT | SP2_RST_BIT | SP2_TCK_BIT); //set the SCL, enable, reset, and tck pins low

	//SP3
	P_SP3_DIR |= (SP3_SCL_BIT | SP3_EN_BIT | SP3_RST_BIT | SP3_TCK_BIT); //set the SCL, enable, reset, and tck pins as outputs
	P_SP3_SEL &= ~(SP3_SCL_BIT | SP3_EN_BIT | SP3_RST_BIT | SP3_TCK_BIT); //set the SCL, enable, reset, and tck pins as GPIO
	P_SP3_OUT &= ~(SP3_SCL_BIT | SP3_EN_BIT | SP3_RST_BIT | SP3_TCK_BIT); //set the SCL, enable, reset, and tck pins low

	//SP4
	P_SP4_DIR |= (SP4_SCL_BIT | SP4_EN_BIT | SP4_RST_BIT | SP4_TCK_BIT); //set the SCL, enable, reset, and tck pins as outputs
	P_SP4_SEL &= ~(SP4_SCL_BIT | SP4_EN_BIT | SP4_RST_BIT | SP4_TCK_BIT); //set the SCL, enable, reset, and tck pins as GPIO
	P_SP4_OUT &= ~(SP4_SCL_BIT | SP4_EN_BIT | SP4_RST_BIT | SP4_TCK_BIT); //set the SCL, enable, reset, and tck pins low

	// Set the direction of the serial data line to output, low, no interrupts
	P_SDA_DIR |= SP1_SDA_BIT | SP2_SDA_BIT | SP3_SDA_BIT | SP4_SDA_BIT;
	P_SDA_OUT &= ~(SP1_SDA_BIT | SP2_SDA_BIT | SP3_SDA_BIT | SP4_SDA_BIT);
	P_SDA_IE &= ~(SP1_SDA_BIT | SP2_SDA_BIT | SP3_SDA_BIT | SP4_SDA_BIT);

	// Setup the interrupt lines
	P_INT_DIR &= ~(SP1_INT_BIT | SP2_INT_BIT | SP3_INT_BIT | SP4_INT_BIT);

	g_eSP_DriverState = SP_DRIVER_ACTIVE;

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
uint8 ucSP_SetCommState(uint16 ucBaud, uint8 ucSPNumber)
{

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	//initialize the communication flag
	g_ucCOMM_Flags = 0x00;

	if (ucSP_SetActiveBoard(ucSPNumber))
		return SP_NOT_ATTACHED;

	// Clear the RX buffer and reset index
	for (g_ucaSP_RXBufferIndex = 0x00; g_ucaSP_RXBufferIndex < RX_BUFFER_SIZE; g_ucaSP_RXBufferIndex++) {
		g_ucaSP_RXBuffer[g_ucaSP_RXBufferIndex] = 0x00;
	}
	g_ucaSP_RXBufferIndex = 0x00;

	// BUG FIX: Clear TACTL in case someone was using it before us
	TA0CTL = 0x0000;
	TA0EX0 = 0x00;

	// Hold TimerA in reset
	TA0CTL &= ~(MC0 | MC1);

	// Use the SMCLK, enable CCR0 interrupt
	TA0CTL |= TASSEL_2;

	// Set the interrupt pin low
	P_INT_DIR &= ~S_ActiveSP.m_ucActiveSP_BitInt;
	P_INT_OUT &= ~S_ActiveSP.m_ucActiveSP_BitInt;

	// The timer interrupt controls the baud rate, currently configured for a
	// 4 MHz SMCLK
	g_unCOMM_BaudRateControl = ucBaud;
	switch (g_unCOMM_BaudRateControl)
	{
		case BAUD_9600:
			g_unCOMM_BaudRateDelayControl = BAUD_9600_DELAY;
			g_unCOMM_BaudRateControl = SCL_BAUD_9600;
		break;

		case BAUD_19200:
			g_unCOMM_BaudRateDelayControl = BAUD_19200_DELAY;
			g_unCOMM_BaudRateControl = SCL_BAUD_19200;
		break;

		case BAUD_57600:
			g_unCOMM_BaudRateDelayControl = BAUD_57600_DELAY;
			g_unCOMM_BaudRateControl = SCL_BAUD_57600;
		break;

		case BAUD_115200:
			g_unCOMM_BaudRateDelayControl = BAUD_115200_DELAY;
			g_unCOMM_BaudRateControl = SCL_BAUD_115200;
		break;

		case BAUD_230400:
			g_unCOMM_BaudRateDelayControl = BAUD_230400_DELAY;
			g_unCOMM_BaudRateControl = SCL_BAUD_230400;
		break;

		case BAUD_345600:
			g_unCOMM_BaudRateDelayControl = BAUD_345600_DELAY;
			g_unCOMM_BaudRateControl = SCL_BAUD_345600;
		break;

		case BAUD_460800:
			g_unCOMM_BaudRateDelayControl = BAUD_460800_DELAY;
			g_unCOMM_BaudRateControl = SCL_BAUD_460800;
		break;

		default:
			g_unCOMM_BaudRateDelayControl = BAUD_9600_DELAY;
			g_unCOMM_BaudRateControl = SCL_BAUD_9600;
		break;
	}

	TA0CCR0 = g_unCOMM_BaudRateControl;

	g_ucCOMM_Flags |= F_COMM_RUNNING;

	return COMM_OK;
}

//////////////////////////////////////////////////////////////////////////////
//! \brief Sends the start condition informing the SP to prepare for a transaction
//!
//! I2C requires that the start of every message includes a start condition packet.
//! This informs the SP to prepare for a transaction
//!
//////////////////////////////////////////////////////////////////////////////
static void ucSP_TXStartCondition(void)
{
	// If the comm module isn't running then exit
	if (!(g_ucCOMM_Flags & F_COMM_RUNNING))
		return;

	P_SDA_DIR |= S_ActiveSP.m_ucActiveSP_BitSDA;

	// Pull the clock and the data line high
	*S_ActiveSP.m_ucActiveSP_RegOut |= S_ActiveSP.m_ucActiveSP_BitSCL;
	P_SDA_OUT |= S_ActiveSP.m_ucActiveSP_BitSDA;

	// Pull the data line low and wait
	P_SDA_OUT &= ~S_ActiveSP.m_ucActiveSP_BitSDA;

	// Wait for 10uS for SP to jump to ISR, exit LPM, and prepare for interrupt on clock line
	vDELAY_LPMWait1us(10, 2);

	// Pull the clock line low
	*S_ActiveSP.m_ucActiveSP_RegOut &= ~S_ActiveSP.m_ucActiveSP_BitSCL;

	// Assume we are receiving a message
	P_SDA_DIR &= ~S_ActiveSP.m_ucActiveSP_BitSDA;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Sends a byte via the software I2C
//!
//! This function pushes \e ucChar into the global TX buffer, where the
//! TimerA ISR can access it. The system drops into LPM0, keeping the SMCLK
//! alive for TimerA. The ISR handles the start and stop bits as well as the
//! baud rate. This is a blocking call and will not return until the software
//! has sent the entire message.
//!   \param ucChar The 8-bit value to send
//!   \return None
///////////////////////////////////////////////////////////////////////////////
static uint8 ucSP_I2C_TXByte(uint8 ucTXChar)
{

	uint8 ucParityBit;
	uint8 ucBitCount;
	uint8 ucAckBit;
	uint16 uiTXChar;
	uint8 ucSDABit;
	uint8 ucSCLBit;

	// If we are already busy, return
	if (g_ucCOMM_Flags & F_COMM_TX_BUSY)
		return COMM_ERROR;

	// Indicate in the status register that we are now busy
	g_ucCOMM_Flags |= F_COMM_TX_BUSY;

	// Local declarations of this bits place the values in registers - accessing them is faster.
	ucSDABit = S_ActiveSP.m_ucActiveSP_BitSDA;
	ucSCLBit = S_ActiveSP.m_ucActiveSP_BitSCL;

	// Calculate the parity bit prior to transmission
	ucParityBit = 0;
	for (ucBitCount = 0; ucBitCount < 8; ucBitCount++)
		ucParityBit ^= ((ucTXChar >> ucBitCount) & 0x01);

	uiTXChar = (uint16) (ucTXChar | (ucParityBit << 8));

	// Set bit count
	ucBitCount = 9;

	// Set the timer count value
	TA0CCR0 = g_unCOMM_BaudRateControl;

	// Starts the counter in 'Up-Mode'
	TA0CTL |= (TACLR | MC_1);
	TA0CCTL0 &= ~CCIFG;

	P_SDA_DIR |= ucSDABit;

	do {
		// For data bits, mask to get correct value and the shift for next time
		if (uiTXChar & 0x01)
			P_SDA_OUT |= ucSDABit;
		else
			P_SDA_OUT &= ~ucSDABit;

		// Wait then clear the flag
		while (!(TA0CCTL0 & CCIFG));
		TA0CCTL0 &= ~CCIFG;

		*S_ActiveSP.m_ucActiveSP_RegOut |= ucSCLBit;

		ucBitCount--;
		uiTXChar >>= 1;

		// Wait then clear the flag
		while (!(TA0CCTL0 & CCIFG));
		TA0CCTL0 &= ~CCIFG;

		*S_ActiveSP.m_ucActiveSP_RegOut &= ~ucSCLBit;

	}
	while (ucBitCount != 0);

	// The next bit is the ack bit from the slave processor,
	// so set up the pin to input
	P_SDA_DIR &= ~ucSDABit;

	// Clock high
	while (!(TA0CCTL0 & CCIFG));
	TA0CCTL0 &= ~CCIFG;
	*S_ActiveSP.m_ucActiveSP_RegOut |= ucSCLBit;

	// Last bit is ack
	ucAckBit = (P_RX_IN & ucSDABit);

	// Wait then clear the flag
	while (!(TA0CCTL0 & CCIFG));
	TA0CCTL0 &= ~CCIFG;

	// Pull the clock low
	*S_ActiveSP.m_ucActiveSP_RegOut &= ~ucSCLBit;

	//Reset the data line direction to output and return to idle state
	P_SDA_DIR |= ucSDABit;
	P_SDA_OUT |= ucSDABit;

	// Disable interrupts
	TA0CCTL0 &= ~CCIE;

	// Stop the timer
	TA0CTL &= ~(MC0 | MC1 | TAIFG);

	g_ucCOMM_Flags &= ~F_COMM_TX_BUSY;

	if (ucAckBit)
		return COMM_ERROR;
	else
		return COMM_OK;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Receives a byte via the software I2C
//!
//! Returns an error if the board does not respond.  Parity bit errors are
//! handled after transmission.
//!
//!   \param None
//!   \return None
///////////////////////////////////////////////////////////////////////////////
static uint8 ucSP_I2C_RXByte(void)
{
	uint8 ucParityBit; // The calculated parity bit
	uint8 ucRxParityBit; // The received parity bit
	uint8 ucBitCount;
	uint8 ucSDABit;
	uint8 ucSCLBit;
	uint8 ucRXByte;

	//If a we are already busy, return
	if ((g_ucCOMM_Flags & F_COMM_RX_BUSY)) {
		return COMM_ERROR;
	}

	// Init the parity bit
	ucParityBit = 0;

	// Set bit count
	ucBitCount = 8;

	// Local declarations of this bits place the values in registers - accessing them is faster.
	ucSDABit = S_ActiveSP.m_ucActiveSP_BitSDA;
	ucSCLBit = S_ActiveSP.m_ucActiveSP_BitSCL;

	// The next bit is the ack bit from the slave processor,
	// so set up the pin to input
	P_SDA_DIR &= ~ucSDABit;

	// Set the timer count value
	TA0CCR0 = g_unCOMM_BaudRateControl;
	TA0CCTL0 &= ~CCIFG;

	// Starts the counter in 'Up-Mode'
	TA0CTL |= (TACLR | MC_1);

	// Clock is low when entering this loop
	do {
		// Shift over for the next bit then sleep
		ucRXByte >>= 1;

		// Wait then clear the flag
		while (!(TA0CCTL0 & CCIFG));
		TA0CCTL0 &= ~CCIFG;

		// Pull the clock high
		*S_ActiveSP.m_ucActiveSP_RegOut |= ucSCLBit;

		// Sample the bit on the SDA line
		if (P_SDA_IN & ucSDABit) {
			ucRXByte |= 0x80;
			ucParityBit++;
		}
		else {
			ucRXByte &= ~0x80;
		}

		// Wait then clear the flag
		while (!(TA0CCTL0 & CCIFG));
		TA0CCTL0 &= ~CCIFG;

		// Pull the clock low
		*S_ActiveSP.m_ucActiveSP_RegOut &= ~ucSCLBit;

	}
	while (--ucBitCount != 0);

	// Wait then clear the flag
	while (!(TA0CCTL0 & CCIFG));
	TA0CCTL0 &= ~CCIFG;

	// Pull the clock high
	*S_ActiveSP.m_ucActiveSP_RegOut |= ucSCLBit;

	// Sample the parity bit
	if (P_RX_IN & ucSDABit)
		ucRxParityBit = 1;
	else
		ucRxParityBit = 0;

	// Wait then clear the flag
	while (!(TA0CCTL0 & CCIFG));
	TA0CCTL0 &= ~CCIFG;

	// Pull the clock low
	*S_ActiveSP.m_ucActiveSP_RegOut &= ~ucSCLBit;

	// Calculate the expected parity bit
	ucParityBit = ucParityBit % 2;

	// Send the ack if the parity is good
	if (ucParityBit == ucRxParityBit)
		P_SDA_OUT &= ~S_ActiveSP.m_ucActiveSP_BitSDA;
	else
		P_SDA_OUT |= S_ActiveSP.m_ucActiveSP_BitSDA;

	// The next bit is the ack bit from the slave processor, so set up the pin accordingly
	P_SDA_DIR |= ucSDABit;

	// Wait then clear the flag
	while (!(TA0CCTL0 & CCIFG));
	TA0CCTL0 &= ~CCIFG;

	// Pull the clock high
	*S_ActiveSP.m_ucActiveSP_RegOut |= ucSCLBit;

	// Wait then clear the flag
	while (!(TA0CCTL0 & CCIFG));
	TA0CCTL0 &= ~CCIFG;

	// Pull the clock low
	*S_ActiveSP.m_ucActiveSP_RegOut &= ~ucSCLBit;

	// Set the SDA line to input for the next byte
	P_SDA_DIR &= ~ucSDABit;

	// Set the parity bit error flag
	if (ucParityBit != ucRxParityBit)
		g_ucCOMM_Flags |= F_COMM_PARITY_ERR;

	g_ucaSP_RXBuffer[g_ucaSP_RXBufferIndex] = ucRXByte;
	g_ucaSP_RXBufferIndex++; // Increment index for next byte

	return COMM_OK;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Verifies the integrity of an SP message
//!
//!   \param ucDEID,
//!   \return ucErrorCode
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_ChkMsgIntegrity(uint8 ucDEID, uint8 *ucMsgBuff)
{
	uint8 ucErrorCode;
	uint8 ucPacketLength;

	// Assume success. Hey, why not be optimistic?
	ucErrorCode = COMM_OK;

	// Check to see if the length of the message exceeds allowable
	if (ucMsgBuff[SP_MSG_LEN_IDX] > MAX_SP_MSG_LENGTH) {
		ucErrorCode = COMM_ERROR;
		vSERIAL_sout("BdMsgLen\r\n", 10);
	}

	// Check to see if the message type is not what we expect
	if (ucMsgBuff[SP_MSG_TYP_IDX] != ucDEID) {
		ucErrorCode = COMM_ERROR;
		vSERIAL_sout("BdMsgID\r\n", 9);
	}

	// Check the CRC of the message
	ucPacketLength = ucMsgBuff[SP_MSG_LEN_IDX] + CRC_SZ;
	if (!ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_REC, ucMsgBuff, ucPacketLength)) {
		ucErrorCode = COMM_ERROR;
		vSERIAL_sout("BdCRC\r\n", 7);
	}

	// Check the comm flags to see if any bytes failed parity
	if (g_ucCOMM_Flags & F_COMM_PARITY_ERR) {
		ucErrorCode = COMM_ERROR;
		vSERIAL_sout("BdPrty\r\n", 8);
	}

#if 0
	uchar ucII;

	if(ucErrorCode != COMM_OK)
	{
		for(ucII = 0; ucII < 48; ucII++)
		{
			vSERIAL_HB8out(ucMsgBuff[ucII]);
		}
		vSERIAL_crlf();
	}
#endif

	return ucErrorCode;
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
void vSP_ShutdownComm(void)
{
	// Halt timer and clear interrupt enables
	TA0CTL &= ~(MC0 | MC1 | TAIE | TAIFG);
	TA0CCTL0 &= ~CCIE;

	//Disable active SP
	S_ActiveSP.m_ucActiveSP = 0xFF;

	//Disable interrupts on the RX line
	P_RX_IE &= ~S_ActiveSP.m_ucActiveSP_BitSDA;

	// Pull interrupt line low
	P_INT_OUT &= ~S_ActiveSP.m_ucActiveSP_BitInt;

	//Indicate the comm channel is inactive
	g_ucCOMM_Flags &= ~F_COMM_RUNNING;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Waits for reception of a data message
//!
//! This function waits for a data message to be received on the serial port.
//!   \param None.
//!   \return The error code indicating the status after call
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_WaitForMessage(void)
{

	uint8 ucRXMessageSize;

	// Set the size of the received message to the minimum
	ucRXMessageSize = SP_HEADERSIZE;

	// An SP requires approximately 1 mS to prepare to transmit a message (CRC etc.)
	vDELAY_LPMWait1us(1000, 2);

	// Wait to receive the message
	do // saves 3 cycles as opposed to while
	{
		if (ucSP_I2C_RXByte()) {
			return COMM_ERROR;
		}

		// If we have received the header, update the RX message length
		if (g_ucaSP_RXBufferIndex == SP_HEADERSIZE) {
			ucRXMessageSize = g_ucaSP_RXBuffer[SP_MSG_LEN_IDX];

			// Range check the ucRXMessageSize variable
			if (ucRXMessageSize > MAX_SP_MSGSIZE || ucRXMessageSize < SP_HEADERSIZE)
				return COMM_ERROR;
		}

		// SP requires about 20 uS to prepare the next byte (parity, I/O etc.)
		// An additional 10 uS wait along with the statements in this function
		// provide the desired delay
		vDELAY_LPMWait1us(10, 0);

	}	while (g_ucaSP_RXBufferIndex != (ucRXMessageSize + CRC_SZ));

	// No message received
	if (g_ucaSP_RXBufferIndex == 0) {
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
void vSP_SendMessage(void)
{
	uint8 ucLoopCount;

	// Initiate communication by sending the start condition
	ucSP_TXStartCondition();

	// Compute the CRC for the message
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, g_ucaTXBuffer, g_ucaTXBuffer[SP_MSG_LEN_IDX] + CRC_SZ);

	for (ucLoopCount = 0x00; ucLoopCount < g_ucaTXBuffer[SP_MSG_LEN_IDX] + CRC_SZ; ucLoopCount++)
		ucSP_I2C_TXByte(g_ucaTXBuffer[ucLoopCount]);
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Grabs the raw chars from buffer and puts them into the desired buffer
//!
//! This function takes the characters from \e g_ucaSP_RXBuffer and stores them
//! in a buffer.  This is used by the application to fetch data elements from
//! SP boards and store them within messages in memory
//!
//!   \param p_ucaBuffer Pointer to the message
//!   \return The error code indicating the status after call
//!   \sa comm.h msg.h
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_GrabMessageFromBuffer(union SP_DataMessage * message)
{
	uint8 ucLoopCount;

	if (g_ucaSP_RXBufferIndex < g_ucaSP_RXBuffer[0x01])
		return COMM_ERROR;

	for (ucLoopCount = 0x00; ucLoopCount < g_ucaSP_RXBuffer[SP_MSG_LEN_IDX]; ucLoopCount++)
		message->ucByteStream[ucLoopCount] = g_ucaSP_RXBuffer[ucLoopCount];

	g_ucaSP_RXBufferIndex = 0x00;

	return COMM_OK;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Turns on and verifies the SP is the correct one.  Leaves the SP board
//!				 on but disables the timers and ISR upon exit.
//!
//!
//!   \param ucSPNumber, ucPrint
//!   \return 1 Error, 0 Success
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_Start(uint8 ucSPNumber)
{
	uint8 ucWakeUpCount;
	uint8 ucCounter; // Generic counter used in several loops
	uint8 ucMsgIdx;
	uint8 ucFoundNewSP;

	// Assume all SP boards are known
	ucFoundNewSP = 0;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	//Assume the SP is attached
	S_AttachedSP.m_ucAttachedSPs |= ucaSPBit[ucSPNumber];

	//Initialize the counter that keeps track of attempts to wake up the SP board
	ucWakeUpCount = 0x05;

	//Attempt to wake up the SP board a few times
	while (ucWakeUpCount != 0) {
		vSP_TurnOn(ucSPNumber);
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);

		// Send the start condition
		ucSP_TXStartCondition();
		if ((ucSP_WaitForMessage() == COMM_OK) && (ucSP_ChkMsgIntegrity(ID_PKT,g_ucaSP_RXBuffer) == COMM_OK)) {
			break;
		}
		vSP_TurnOff(ucSPNumber);
		ucWakeUpCount--;
	}

	//If the counter equals zero then the wake up attempt was a failure
	if (ucWakeUpCount == 0x00) {
		vSP_ShutdownComm(); // Shutdown communication
		vSP_TurnOff(ucSPNumber); // Power off the board

		vSP_DetachBoard(ucSPNumber); // Clear the board from the SP structure

		return 1;
	}

	// Compare SP serial number with what the driver has on record
	ucMsgIdx = SP_MSG_PAYLD_IDX;
	for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
		// If the HID does not match then this is a new board and it must be interrogated
		if (g_ucaSP_RXBuffer[ucMsgIdx++] != S_AttachedSP.m_ucaSP_SerialNumber[ucSPNumber][ucCounter]) {
			ucFoundNewSP = 1;
			break;
		}
	}

	// Update driver with the current serial number
	ucMsgIdx = SP_MSG_PAYLD_IDX;
	for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
		S_AttachedSP.m_ucaSP_SerialNumber[ucSPNumber][ucCounter] = g_ucaSP_RXBuffer[ucMsgIdx++];
	}

	if (ucFoundNewSP) {
		// Send the interrogate message
		if (ucSP_SendInterrogate(ucSPNumber)) {
			// Stop timer and disable interrupts
			vSP_ShutdownComm();
			return 1; // Command failed
		}
		// Update the version number
		S_AttachedSP.m_ucaSP_MSG_Version[ucSPNumber] = g_ucaSP_RXBuffer[SP_MSG_VER_IDX];

		// Start the message index at the top of the payload
		ucMsgIdx = SP_MSG_PAYLD_IDX;

		// Get the number of transducers from the SP
		S_AttachedSP.m_ucaSP_NumTransducers[ucSPNumber] = g_ucaSP_RXBuffer[ucMsgIdx++];

		// Get the transducer types (sensor or actuator)
		for (ucCounter = 0; ucCounter < S_AttachedSP.m_ucaSP_NumTransducers[ucSPNumber]; ucCounter++) {
			S_AttachedSP.m_ucaSP_TypeTransducers[ucSPNumber][ucCounter] = g_ucaSP_RXBuffer[ucMsgIdx++];
			S_AttachedSP.m_ucaSP_TransSmplDur[ucSPNumber][ucCounter] = g_ucaSP_RXBuffer[ucMsgIdx++];
		}

		// Get the SP boards name from the final 8 bytes in the interrogate packet
		for (ucCounter = 0; ucCounter < SP_NAME_LENGTH; ucCounter++) {
			S_AttachedSP.m_ucaSP_Name[ucSPNumber][ucCounter] = g_ucaSP_RXBuffer[ucMsgIdx++];
		}

	}
// Stop timer and disable interrupts
	vSP_ShutdownComm();

	return COMM_OK; //success

}


///////////////////////////////////////////////////////////////////////////////
//!\brief The SP firmware has been updated to include a flag byte, therefore
//! in order to reprogram one of these boards we need to start it and retrieve
//! the BSL password using the older protocol
//!
//!   \param ucSPNumber, ucPrint
//!   \return 1 Error, 0 Success
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_Start_Old(uint8 ucSPNumber)
{
	uint8 ucWakeUpCount;
	uint8 ucCounter; // Generic counter used in several loops
	uint8 ucMsgIdx;
	uint8 ucFoundNewSP;

	// Assume all SP boards are known
	ucFoundNewSP = 0;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	//Assume the SP is attached
	S_AttachedSP.m_ucAttachedSPs |= ucaSPBit[ucSPNumber];

	//Initialize the counter that keeps track of attempts to wake up the SP board
	ucWakeUpCount = 0x05;

	//Attempt to wake up the SP board a few times
	while (ucWakeUpCount != 0) {
		vSP_TurnOn(ucSPNumber);
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);

		// Send the start condition
		ucSP_TXStartCondition();
		if ((ucSP_WaitForMessage() == COMM_OK) && (ucSP_ChkMsgIntegrity(ID_PKT,g_ucaSP_RXBuffer) == COMM_OK)) {
			break;
		}
		vSP_TurnOff(ucSPNumber);
		ucWakeUpCount--;
	}

	//If the counter equals zero then the wake up attempt was a failure
	if (ucWakeUpCount == 0x00) {
		vSP_ShutdownComm(); // Shutdown communication
		vSP_TurnOff(ucSPNumber); // Power off the board

		vSP_DetachBoard(ucSPNumber); // Clear the board from the SP structure

		return 1;
	}

	// Compare SP serial number with what the driver has on record
	ucMsgIdx = 3;
	for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
		// If the HID does not match then this is a new board and it must be interrogated
		if (g_ucaSP_RXBuffer[ucMsgIdx++] != S_AttachedSP.m_ucaSP_SerialNumber[ucSPNumber][ucCounter]) {
			ucFoundNewSP = 1;
			break;
		}
	}

	// Update driver with the current serial number
	ucMsgIdx = 3;
	for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
		S_AttachedSP.m_ucaSP_SerialNumber[ucSPNumber][ucCounter] = g_ucaSP_RXBuffer[ucMsgIdx++];
	}

	if (ucFoundNewSP) {
		// Send the interrogate message
		if (ucSP_SendInterrogate_Old(ucSPNumber)) {
			// Stop timer and disable interrupts
			vSP_ShutdownComm();
			return 1; // Command failed
		}
		// Update the version number
		S_AttachedSP.m_ucaSP_MSG_Version[ucSPNumber] = g_ucaSP_RXBuffer[SP_MSG_VER_IDX];

		// Start the message index at the top of the payload
		ucMsgIdx = 3;

		// Get the number of transducers from the SP
		S_AttachedSP.m_ucaSP_NumTransducers[ucSPNumber] = g_ucaSP_RXBuffer[ucMsgIdx++];

		// Get the transducer types (sensor or actuator)
		for (ucCounter = 0; ucCounter < S_AttachedSP.m_ucaSP_NumTransducers[ucSPNumber]; ucCounter++) {
			S_AttachedSP.m_ucaSP_TypeTransducers[ucSPNumber][ucCounter] = g_ucaSP_RXBuffer[ucMsgIdx++];
			S_AttachedSP.m_ucaSP_TransSmplDur[ucSPNumber][ucCounter] = g_ucaSP_RXBuffer[ucMsgIdx++];
		}

		// Get the SP boards name from the final 8 bytes in the interrogate packet
		for (ucCounter = 0; ucCounter < SP_NAME_LENGTH; ucCounter++) {
			S_AttachedSP.m_ucaSP_Name[ucSPNumber][ucCounter] = g_ucaSP_RXBuffer[ucMsgIdx++];
		}

	}
// Stop timer and disable interrupts
	vSP_ShutdownComm();

	return COMM_OK; //success

}

/////////////////////////////////////////////////////////////////////////////////

//Write a set of functions here that return required information to the task manager
//		without passing some complex data type structs etc..

/////////////////////////////////////////////////////////////////////////////////
//! \brief Checks if the S is attached
/////////////////////////////////////////////////////////////////////////////////
uchar ucSP_IsAttached(uchar ucSPNumber)
{
	if (S_AttachedSP.m_ucAttachedSPs & ucaSPBit[ucSPNumber])
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Returns the message version
/////////////////////////////////////////////////////////////////////////////////
uint8 ucSP_FetchMsgVersion(uchar ucSPNumber)
{
	return S_AttachedSP.m_ucaSP_MSG_Version[ucSPNumber];
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Returns the number of transducers and SP board has
/////////////////////////////////////////////////////////////////////////////////
uchar ucSP_FetchNumTransducers(uchar ucSPNumber)
{
	return S_AttachedSP.m_ucaSP_NumTransducers[ucSPNumber];
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Returns the type of transducer and SP board has
/////////////////////////////////////////////////////////////////////////////////
uchar ucSP_FetchTransType(uchar ucSPNumber, uchar ucTransNumber)
{
	return S_AttachedSP.m_ucaSP_TypeTransducers[ucSPNumber][ucTransNumber];
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Returns the sample duration of a transducer
/////////////////////////////////////////////////////////////////////////////////
uchar ucSP_FetchTransSmplDur(uchar ucSPNumber, uchar ucTransNumber)
{
	return S_AttachedSP.m_ucaSP_TransSmplDur[ucSPNumber][ucTransNumber];
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Returns the state of the SP board
/////////////////////////////////////////////////////////////////////////////////
uint8 ucSP_FetchSPState(uchar ucSPNumber)
{
	return S_AttachedSP.m_ucaSP_Status[ucSPNumber];
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Sets the state of the SP board
/////////////////////////////////////////////////////////////////////////////////
void vSP_SetSPState(uchar ucSPNumber, uchar ucState)
{
	S_AttachedSP.m_ucaSP_Status[ucSPNumber] = ucState;
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Returns the name of the SP board
/////////////////////////////////////////////////////////////////////////////////
void vSP_FetchSPName(uchar ucSPNumber, uchar *p_caName)
{
	uint8 ucIndex;

	//Display the characters in the cpaSPLabels array only if they are ascii characters
	for (ucIndex = 0; ucIndex < SP_NAME_LENGTH; ucIndex++) {
		*p_caName = S_AttachedSP.m_ucaSP_Name[ucSPNumber][ucIndex];
		p_caName++;
	}

}

/////////////////////////////////////////////////////////////////////////////////
//! \fn ucSP_GetHID
//! \brief Gets the hardware ID of an SP board
//! \param ucSPNumber, the desired SP board
//! \param *ucSP_HI, pointer to the location where the HID will be copied
//! \return 0 for success, else failure
/////////////////////////////////////////////////////////////////////////////////
uint8 ucSP_GetHID(uchar ucSPNumber, uchar * ucSP_HID)
{
	uint8 ucCounter;

	if(ucSP_IsAttached(ucSPNumber) == FALSE)
		return 1;

	for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
		*ucSP_HID++ = S_AttachedSP.m_ucaSP_SerialNumber[ucSPNumber][ucCounter];
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Input the correct parameters into the packet to request data, send.
//!
//!   \param uint8 ucSPnumber, uint8LabelIndex
//!   \return none
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_Request_TransducerLabel(uint8 ucSPNumber, uint8 ucChannelNum, uint8 *pucLabelArray)
{
	uint8 ucBuffIndex;
	uint8 ucAttemptCount;
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	// If the channel is greater than the number of transducers on the board then return an error
	if (ucChannelNum > S_AttachedSP.m_ucaSP_NumTransducers[ucSPNumber])
		return ucRetVal;

	// Load the message into the TX buffer
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = REQUEST_LABEL;
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = 5;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaTXBuffer[SP_MSG_FLAGS_IDX] = 0;
	g_ucaTXBuffer[SP_MSG_PAYLD_IDX] = ucChannelNum;

	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {
		// Set up communication with this SP
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);

		// Send the message
		vSP_SendMessage();

		// If there were no communication errors and the message is good
		if (ucSP_WaitForMessage() == 0 && ucSP_ChkMsgIntegrity(REPORT_LABEL, g_ucaSP_RXBuffer) == 0) {
			// Load the label into the passed array
			for (ucBuffIndex = SP_MSG_PAYLD_IDX; ucBuffIndex < g_ucaSP_RXBuffer[SP_MSG_LEN_IDX];) {
				*pucLabelArray++ = g_ucaSP_RXBuffer[ucBuffIndex++];
			}

			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Display a single transducer label
//!
//! The number of transducers on an SP can be large and are regarded as
//! non-essential information.  At the request of the user they can be fetched
//! from the SP and printed.  Storing them in a local data structure is not
//! scalable.
//!
//!   \param ucSPNumber, ucTransducer
//!   \return none
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_DisplaySingleLabel(uint8 ucSPNumber, uint8 ucChannelNum)
{
	uint8 ucIndex;
	uint8 ucaLabelArray[LABEL_LENGTH];

	for (ucIndex = 0; ucIndex < LABEL_LENGTH; ucIndex++) {
		ucaLabelArray[ucIndex] = 0;
	}

	if (ucSP_Request_TransducerLabel(ucSPNumber, ucChannelNum, ucaLabelArray))
		return 1;

	//Display the characters in the cpaSPLabels array only if they are ascii characters
	for (ucIndex = 0; ucIndex < LABEL_LENGTH; ucIndex++) {
		if ((ucaLabelArray[ucIndex] > 0x2F) && (ucaLabelArray[ucIndex] < 0x7B)) {
			vSERIAL_bout(ucaLabelArray[ucIndex]);
		}
		else
			vSERIAL_sout(" ", 1);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Display all transducer labels for an SP
//!
//!   \param ucSPNumber,
//!   \return none
///////////////////////////////////////////////////////////////////////////////
void vSP_DisplayLabels(uint8 ucSPNumber)
{
	uint8 ucTransducer;

	// Loop through the transducers for the desired SP
	for (ucTransducer = 0; ucTransducer < 0x10; ucTransducer++) {
		ucSP_DisplaySingleLabel(ucSPNumber, ucTransducer);
	}
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Input the correct parameters into the packet to request data, send.
//!
//!   \param ucSPNumber
//!   \return length of the received message
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_RequestData(uint8 ucSPNumber)
{

	uint8 ucAttemptCount;
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	if (!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	// Build the message and send it
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = REQUEST_DATA;
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = SP_HEADERSIZE;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaTXBuffer[SP_MSG_FLAGS_IDX] = 0;

#if 0
	uint8 ucLoopCnt;
	vSERIAL_sout("SP RQST\r\n", 9);
	for (ucLoopCnt = 0; ucLoopCnt < g_ucaTXBuffer[SP_MSG_LEN_IDX]; ucLoopCnt++)
	{
		vSERIAL_HB8out(g_ucaTXBuffer[ucLoopCnt]);
		vSERIAL_crlf();
	}
#endif

	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);
		vSP_SendMessage();

		// If there were no communication errors and the message is good
		if (ucSP_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(REPORT_DATA, g_ucaSP_RXBuffer) == COMM_OK) {
			// Indicate the communication succeeded
			ucRetVal = COMM_OK;
#if 0
			vSERIAL_sout("RQST PASS [", 11);
			vSERIAL_HB8out(ucAttemptCount);
			vSERIAL_sout("]\r\n", 3);
#endif

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Input the correct parameters into the packet to send a command, send.
//!
//!   \param ucSPNumber, the desired SP; p_ucCmdPayload, pointer to the structure
//!					containing the command pay load.
//!   \return none
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_SendCommand(uint8 ucSPNumber, uint8 ucPayloadLen, uint8 * p_ucCmdPayload)
{
	uint8 ucAttemptCount;
	uint8 ucRetVal;
	uint8 ucLoopCnt;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	if (!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	// Load the command header
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = COMMAND_PKT;
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = (SP_HEADERSIZE + ucPayloadLen);
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaTXBuffer[SP_MSG_FLAGS_IDX] = 0;

	// Load the command payload
	for (ucLoopCnt = SP_MSG_PAYLD_IDX; ucLoopCnt < g_ucaTXBuffer[SP_MSG_LEN_IDX]; ucLoopCnt++)
		g_ucaTXBuffer[ucLoopCnt] = *p_ucCmdPayload++;

#if 0
	vSERIAL_sout("SP CMD\r\n", 8);
	for (ucLoopCnt = 0; ucLoopCnt < g_ucaTXBuffer[SP_MSG_LEN_IDX]; ucLoopCnt++)
	{
		vSERIAL_HB8out(g_ucaTXBuffer[ucLoopCnt]);
	}
	vSERIAL_crlf();
#endif

	// Send the message, repeat if required
 	for (ucAttemptCount = 5; ucAttemptCount > 0; ucAttemptCount--) {
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);
		vSP_SendMessage();

		// If there were no communication errors and the message is good
		if (ucSP_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(CONFIRM_COMMAND, g_ucaSP_RXBuffer) == COMM_OK) {
			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

#if 0
			vSERIAL_sout("CMD CNFRM[", 10);
			vSERIAL_HB8out (5-ucAttemptCount);
			vSERIAL_sout("]\r\n", 3);
#endif

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Request transducer information from the SP and sets an ID in flash
//!				 memory of the SP board.
//!
//!	This function assists the task manager in determining what tasks the SP can perform.
//! The SP ID helps the CP make sure that the attached SPs have not been changed.
//! In the event that they have been changed this function is called to reconfigure
//! the SP driver to match the parameters of the new SP board
//!
//! \param ucSPnumber
//! \return Error code
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_SendInterrogate(uint8 ucSPNumber)
{
	uint8 ucAttemptCount;
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	if (!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	// Load the header of the message and send
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = INTERROGATE;
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = SP_HEADERSIZE;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaTXBuffer[SP_MSG_FLAGS_IDX] = 0;


	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);
		vSP_SendMessage();

		// If there were no communication errors and the message is good
		if (ucSP_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(INTERROGATE, g_ucaSP_RXBuffer) == COMM_OK) {
			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Request transducer information from the SP and sets an ID in flash
//!				 memory of the SP board.
//!
//!	This function assists the task manager in determining what tasks the SP can perform.
//! The SP ID helps the CP make sure that the attached SPs have not been changed.
//! In the event that they have been changed this function is called to reconfigure
//! the SP driver to match the parameters of the new SP board
//!
//! \param ucSPnumber
//! \return Error code
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_SendInterrogate_Old(uint8 ucSPNumber)
{
	uint8 ucAttemptCount;
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	if (!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	// Load the header of the message and send
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = INTERROGATE;
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = 3;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;

	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);
		vSP_SendMessage();

		// If there were no communication errors and the message is good
		if (ucSP_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(INTERROGATE, g_ucaSP_RXBuffer) == COMM_OK) {
			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Request transducer information from the SP and sets an ID in flash
//!				 memory bootstrap loader password from the SP for reprogramming
//!
//!
//! \param ucSPnumber
//! \return Error code
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_RequestBSLPassword(uint8 ucSPNumber)
{
	uint8 ucAttemptCount;
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	if (!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	// Load the header of the message and send
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = REQUEST_BSL_PW;
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = SP_HEADERSIZE;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaTXBuffer[SP_MSG_FLAGS_IDX] = 0;

	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);
		vSP_SendMessage();

		// If there were no communication errors and the message is good
		if (ucSP_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(REQUEST_BSL_PW, g_ucaSP_RXBuffer) == COMM_OK) {
			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Request transducer information from the SP and sets an ID in flash
//!				 memory bootstrap loader password from the SP for reprogramming
//!
//!  Exists to handle antiquated protocol
//!
//! \param ucSPnumber
//! \return Error code
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_RequestBSLPassword_Old(uint8 ucSPNumber)
{
	uint8 ucAttemptCount;
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	// Load the header of the message and send
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = REQUEST_BSL_PW;
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = 3;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;

	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);
		vSP_SendMessage();

		// If there were no communication errors and the message is good
		if (ucSP_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(REQUEST_BSL_PW, g_ucaSP_RXBuffer) == COMM_OK) {
			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Request transducer information from the SP and sets an ID in flash
//!				 memory of the SP board.
//!
//!	This function assists the task manager in determining what tasks the SP can perform.
//! The SP ID helps the CP make sure that the attached SPs have not been changed.
//! In the event that they have been changed this function is called to reconfigure
//! the SP driver to match the parameters of the new SP board
//!
//! \param ucSPnumber; ulSerialNum, the new SPID
//! \return Error code
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_SetHID(uint8 ucSPNumber, uint8 * ucSerialNum)
{
	uchar ucMsgIndex;
	uchar ucRetVal;
	uchar ucAttemptCount, ucByteCount;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (g_eSP_DriverState == SP_DRIVER_SHUTDOWN)
		return SP_BAD_STATE;

	if (!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	g_ucaTXBuffer[SP_MSG_TYP_IDX] = SET_SERIALNUM;
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = SP_HEADERSIZE + SP_HID_LENGTH;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaTXBuffer[SP_MSG_FLAGS_IDX] = 0;

	for (ucMsgIndex = SP_MSG_PAYLD_IDX; ucMsgIndex < (SP_MSG_PAYLD_IDX + SP_HID_LENGTH); ucMsgIndex++) {
		g_ucaTXBuffer[ucMsgIndex] = *ucSerialNum++;
	}

	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);
		vSP_SendMessage();

		// An SP requires approximately 4 mS to write the new HID to flash
		vDELAY_LPMWait1us(4000, 2);

		// If there were no communication errors and the message is good
		if (ucSP_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(SET_SERIALNUM, g_ucaSP_RXBuffer) == COMM_OK) {
			// Write the new ID to the driver
			ucMsgIndex = SP_MSG_PAYLD_IDX;
			for (ucByteCount = 0; ucByteCount < SP_HID_LENGTH; ucByteCount++)
				S_AttachedSP.m_ucaSP_SerialNumber[ucSPNumber][ucByteCount] = g_ucaSP_RXBuffer[ucMsgIndex++];

			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Sends a command to an SP telling it to acquire the sensor types.
//!  This command has to be done, prior to requesting that the SP send the
//!  types back because the types are not stored locally on the SP, and data
//!  must be sampled for the SP to get the type from the sensor. Sensors
//!  communicate with the SP on the order of 100ms where the CPs and SPs comm
//!  operates at 115200, this would be a timing nightmare for the cp to wait
//!  exactly long enough for the SP, mid-function, so we do it in 2 parts now,
//!  first we send a command telling the SP to take samples and acquire sensor
//!  types. Then we send a request for the types
//!
//! \param uint8 ucSPNumber
//  \return none
///////////////////////////////////////////////////////////////////////////////
uint8 vSP_Command_SensorTypes(uint8 ucSPNumber)
{
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if(!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	// Load the message into the TX buffer
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = 0x0C; // COMMAND_SENSOR_TYPES
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = SP_HEADERSIZE;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaTXBuffer[SP_MSG_FLAGS_IDX] = 0;

	// Set up communication with this SP
	ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);

	// Send the message
	vSP_SendMessage();

	// Halt communications with this SP
	vSP_ShutdownComm();

	ucRetVal = COMM_OK;

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Requests sensor types from an SP. The types are written to a passed
//!  in pointer array. This function should be called after a command to get
//!  the types has been sent. This function is part 2 in the "get types"
//!  process.
//!
//! \param uint8 ucSPNumber, *pucSensorTypes
//  \return none
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_Request_SensorTypes(uint8 ucSPNumber, uint8 *pucSensorTypes)
{
	uint8 ucBuffIndex;
	uint8 ucAttemptCount;
	uint8 result1 = 0;
	uint8 result2 = 0;
	uint8 ucRetVal;
	uint8 uiCount;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if(!ucSP_IsAttached(ucSPNumber)) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	// Load the message into the TX buffer
	g_ucaTXBuffer[SP_MSG_TYP_IDX] = 0x0D; // COMMAND_SENSOR_TYPES
	g_ucaTXBuffer[SP_MSG_LEN_IDX] = SP_HEADERSIZE;
	g_ucaTXBuffer[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaTXBuffer[SP_MSG_FLAGS_IDX] = 0;

	// Send the message, repeat if required
	for(ucAttemptCount = 3; ucAttemptCount >  0; ucAttemptCount--)
	{
		// Set up communication with this SP
		ucSP_SetCommState(SP_OPMODE_BAUD, ucSPNumber);

		// Send the message
		vSP_SendMessage();

		result1 = ucSP_WaitForMessage();
		result2 = ucSP_ChkMsgIntegrity(0x0D, g_ucaSP_RXBuffer);

		// If there were no communication errors and the message is good
		//if (ucSP_WaitForMessage() == 0 && ucSP_ChkMsgIntegrity(0x0C) == 0)
		if(result1 == 0 && result2 == 0)
		{
			// Load the type into the passed array
			for (ucBuffIndex = SP_MSG_PAYLD_IDX, uiCount = 0; ucBuffIndex < g_ucaSP_RXBuffer[SP_MSG_LEN_IDX] + SP_MSG_PAYLD_IDX; uiCount++, ucBuffIndex++)
			{
				pucSensorTypes[uiCount] = g_ucaSP_RXBuffer[ucBuffIndex];
			}

			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}
	}

	// Halt communications with this SP
	vSP_ShutdownComm();

	return ucRetVal;
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Wakes up all attached SP boards and determines what type they are
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////////
void vSP_SetRole(void)
{
	uchar ucSPIndex, ucAttemptCount;

	// Wake up each SP a few times
	for (ucSPIndex = 0; ucSPIndex < NUMBER_SPBOARDS; ucSPIndex++) {
		ucAttemptCount = 5;
		while (ucAttemptCount) {
			if (ucSP_Start(ucSPIndex) == COMM_OK) {
				ucAttemptCount = 0;
			}
			else {
				ucAttemptCount--;
				vSP_TurnOff(ucSPIndex);

				// Wait in LPM for voltage to decay on SP before retrying
				vDELAY_LPMWait1us(5000, 2);
			}
		} // End: while(ucAttemptCount)
	} // End: for(ucSPNumber)

	// Print the SP info to the terminal
	vSP_Display();

//Turn off the SPs
	vSP_TurnoffAll();
}



void vSP_DisplayName(uchar ucSPNumber)
{
	uchar ucCounter;

	for (ucCounter = 0; ucCounter < SP_NAME_LENGTH; ucCounter++) {
		if (S_AttachedSP.m_ucaSP_Name[ucSPNumber][ucCounter] == 0)
			vSERIAL_sout(" ", 1);
		else
			vSERIAL_bout((uchar) S_AttachedSP.m_ucaSP_Name[ucSPNumber][ucCounter]);
	}
}

/////////////////////////////////////////////////////////////////////////////////
//! \brief Displays the SP information such as who is attached their state and
//!  capabilities
//!
//! I know the code looks ugly but it gets the formatting right
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////////
void vSP_Display(void)
{
	uchar ucSPNumber;
	signed char cCounter; // general purpose counters reused several times
	uchar ucMaxNumLabels;

	vSERIAL_sout("------- SP1 ------------ SP2 ------------ SP3 ------------ SP4 ---", 66);
	vSERIAL_crlf();
	vSERIAL_sout("Name    ", 8);

	// Display SP names
	for (ucSPNumber = 0; ucSPNumber < NUMBER_SPBOARDS; ucSPNumber++) {
		vSP_DisplayName(ucSPNumber);
		vSERIAL_sout("         ", 9);
	}
	vSERIAL_crlf();

	// Display SP serial numbers
	vSERIAL_sout("SN      ", 8);
	for (ucSPNumber = 0; ucSPNumber < NUMBER_SPBOARDS; ucSPNumber++) {
		for (cCounter = SP_HID_LENGTH - 1; cCounter >= 0; cCounter--) {
			vSERIAL_HB8out(S_AttachedSP.m_ucaSP_SerialNumber[ucSPNumber][cCounter]);
		}
		vSERIAL_sout(" ", 1);
	}
	vSERIAL_crlf();

	vSERIAL_sout("Labels", 6);
	vSERIAL_crlf();

	// Determine which SP has the most labels and how many there are
	ucMaxNumLabels = 0;
	for (ucSPNumber = 0; ucSPNumber < NUMBER_SPBOARDS; ucSPNumber++) {
		if (ucMaxNumLabels < S_AttachedSP.m_ucaSP_NumTransducers[ucSPNumber])
			ucMaxNumLabels = S_AttachedSP.m_ucaSP_NumTransducers[ucSPNumber];
	}

	ucMaxNumLabels++; //plus one for the test function

	// Display labels
	for (cCounter = 0; cCounter < ucMaxNumLabels; cCounter++) {
		vSERIAL_sout("        ", 8);
		for (ucSPNumber = 0; ucSPNumber < NUMBER_SPBOARDS; ucSPNumber++) {
			if (ucSP_DisplaySingleLabel(ucSPNumber, cCounter))
				vSERIAL_sout("                 ", 17);
			else
				vSERIAL_sout(" ", 1);
		}
		vSERIAL_crlf();
	}
} //END: vSP_Display()

/////////////////////////////////////////////////////////////////////////////////
//! \brief Prints SP version stored in the driver
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////////
void vSP_GetSPMsgVersions(void)
{
	uint8 ucSPNum;

	for (ucSPNum = 0; ucSPNum < NUMBER_SPBOARDS; ucSPNum++) {
		if (S_AttachedSP.m_ucaSP_MSG_Version[ucSPNum] != 0) {
			vSERIAL_sout("\r\nSP", 4);
			vSERIAL_UIV8out(ucSPNum + 1);
			vSERIAL_sout("_Msg-V: ", 8);
			vSERIAL_UIV8out((uchar) (S_AttachedSP.m_ucaSP_MSG_Version[ucSPNum])); //Version Num
			vSERIAL_crlf();
		}
	}

}

void vSP_TestI2C(void)
{
	uint8 ucByte;
	uint8 ucRet;

	ucByte = 0xAA;
	vSP_TurnOn(SP1);
	__delay_cycles(8000000);
	ucSP_SetCommState(BAUD_9600, 0);
//	ucSP_TXStartCondition();
	ucSP_TXStartCondition();
	while (1) {
//		ucSP_SetCommState(BAUD_9600, 0);
		ucRet = ucSP_I2C_TXByte(ucByte);
		__delay_cycles(8000);
		ucRet = ucSP_I2C_RXByte();
		g_ucaSP_RXBufferIndex = 0;
		if (g_ucaSP_RXBuffer[g_ucaSP_RXBufferIndex] != ucByte)
			__no_operation();
//		ucByte++;

//		ucSP_SetCommState(BAUD_19200, 0);
//		ucRet = ucSP_I2C_RXByte();
//
//		ucSP_SetCommState(BAUD_57600, 0);
//		ucRet = ucSP_I2C_RXByte();
//
//		ucSP_SetCommState(BAUD_115200, 0);
//		ucRet = ucSP_I2C_RXByte();
//
//		ucSP_SetCommState(BAUD_230400, 0);
//		ucRet = ucSP_I2C_RXByte();

		vSERIAL_UIV8out(ucRet);

//		vSP_TurnOff(SP1);
//		__delay_cycles(8000000);
	}
}

