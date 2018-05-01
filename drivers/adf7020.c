/*******************************************************************************
 * adf7020.c
 *
 * By: Kenji Yamamoto
 *     BluePlasma Engineering LLC
 *
 * Edited by CP
 *
 * Driver module for utilizing the ADF7020 radio with a packet interface
 *******************************************************************************/

#include "std.h"
#include "delay.h"
#include "adf7020.h"
#include <msp430.h>
#include "SERIAL.h"
#include "comm.h"
#include "time.h"

extern volatile union
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

extern volatile union
{
	uchar byte;
	struct
	{
		unsigned FLG3_RADIO_ON_BIT :1;
		unsigned FLG3_RADIO_MODE_BIT :1;
		unsigned FLG3_RADIO_PROGRAMMED :1;
		unsigned FLG3_UNUSED_BIT3 :1;
		unsigned FLG3_UNUSED_BIT4 :1;
		unsigned FLG3_UNUSED_BIT5 :1;
		unsigned FLG3_UNUSED_BIT6 :1;
		unsigned FLG3_UNUSED_BIT7 :1;
	} FLAG3_STRUCT;
} ucFLAG3_BYTE;

extern uchar ucGLOB_radioChannel;

/** \addtogroup ADF7020Driver
 * @{
 */

//! \var ucRSSI_Idx
static uint8 ucRSSI_Idx = 0;
//! \var ucaRSSI_Arr[]
//! \brief Stores a sequence of RSSI measurements for future analysis
static uint16 ucaRSSI_Arr[0x20];

#ifdef ADF7020_DRIVER_USE_CHANNEL_LUT
extern const uint32 g_ucaADF7020_ChannelFrequencyLUT[128][2];
#endif

/***************************************************************************//**
 * \brief ADF7020 Software Driver
 *
 * This variable is the software driver. All functions of the driver utilize
 * this variable to operate the ADF7020.
 *
 * Note that is needs to be declared volatile to ensure that the ISRs can
 * properly access its member fields.
 ******************************************************************************/
extern volatile ADF7020_Driver_t ADF7020_Driver;

/***************************************************************************//**
 * \brief The preamble transmitted before every packet
 *
 * This is the data pattern transmitted as the preamble for every packet. It
 * consists of a repeating 1/0 pattern to lock the PLL to the bit timing
 * followed by the sync word.
 ******************************************************************************/
const uint8 g_ucaADF7020_Preamble[ADF7020_PREAMBLE_BYTE_COUNT] =
{ 0xAA, 0xAA, 0xAA, 0xAA, SYNC_BYTE_01, SYNC_BYTE_02, SYNC_BYTE_03 };

/***************************************************************************//**
 * \brief Changes the direction of the DATACLK pin between input and output
 *
 * It may seem strange to use a function to do basic pin manipulation, but with
 * a little complier magic we can reap multiple benefits.
 *
 * -# Using a function makes the code significantly easier to read and follow
 * -# If a hardware change is made, the code only need to be changed in one
 *    location.
 * -# A function enjoys the type checking and protections provided by the
 *    compiler that a macro does not.
 *
 * By declaring the function an inline and ONLY using the defined values for
 * the inputs (PIN_INPUT and PIN_OUTPUT), then the compiler (even at the basic
 * level of optimization) will compile the inline down to the single line need
 * to change the input direction. This results in all the benefits of direct
 * bit manipulation (speed, smaller code space) with the benefits mentioned
 * above.
 *
 * Doubters can check the assembly where this function is used. Instead of the
 * overhead of the function call, one will see either an OR or AND
 * instruction.
 *
 * @param ucDir The direction to set the GPIO pin.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_SetDATACLKDirection(uint8 ucDir)
{
	if (ucDir)
		DATACLK_DIR_REG |= DATACLK_PIN_NUMBER;
	else
		DATACLK_DIR_REG &= ~DATACLK_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Changes the direction of the INT/LOCK pin between input and output
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucDir The direction to set the GPIO pin.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_SetINTLOCKDirection(uint8 ucDir)
{
	if (ucDir)
		INTLOCK_DIR_REG |= INTLOCK_PIN_NUMBER;
	else
		INTLOCK_DIR_REG &= ~INTLOCK_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Changes the direction of the DATAIO pin between input and output
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucDir The direction to set the GPIO pin.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_SetDATAIODirection(uint8 ucDir)
{
	if (ucDir)
		DATAIO_DIR_REG |= DATAIO_PIN_NUMBER;
	else
		DATAIO_DIR_REG &= ~DATAIO_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Enables or disables the interrupt on INT/LOCK
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucInt Turns the interrupt on or off
 * @return None.
 ******************************************************************************/
static inline void vADF7020_SetINTLOCKInterrupt(uint8 ucInt)
{
	if (ucInt)
		INTLOCK_IE_REG |= INTLOCK_PIN_NUMBER;
	else
		INTLOCK_IE_REG &= ~INTLOCK_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Enables or disables the interrupt on DATACLK
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucInt Turns the interrupt on or off
 * @return None.
 ******************************************************************************/
static inline void vADF7020_SetDATACLKInterrupt(uint8 ucInt)
{
	if (ucInt)
		DATACLK_IE_REG |= DATACLK_PIN_NUMBER;
	else
		DATACLK_IE_REG &= ~DATACLK_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Takes the CE pin high or low.
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucPin Sets the pin high or low.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_WriteCE(uint8 ucPin)
{
	if (ucPin)
		CE_PIN_REG |= CE_PIN_NUMBER;
	else
		CE_PIN_REG &= ~CE_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Takes the DATACLK pin high or low.
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucPin Sets the pin high or low.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_WriteDATACLK(uint8 ucPin)
{
	if (ucPin)
		DATACLK_PIN_REG |= DATACLK_PIN_NUMBER;
	else
		DATACLK_PIN_REG &= ~DATACLK_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Takes the DATAIO pin high or low.
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucPin Sets the pin high or low.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_WriteDATAIO(uint8 ucPin)
{
	if (ucPin)
		DATAIO_PIN_REG |= DATAIO_PIN_NUMBER;
	else
		DATAIO_PIN_REG &= ~DATAIO_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Takes the SCLK pin high or low.
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucPin Sets the pin high or low.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_WriteSCLK(uint8 ucPin)
{
	if (ucPin)
		SCLK_PIN_REG |= SCLK_PIN_NUMBER;
	else
		SCLK_PIN_REG &= ~SCLK_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Takes the SDATA pin high or low.
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucPin Sets the pin high or low.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_WriteSDATA(uint8 ucPin)
{
	if (ucPin)
		SDATA_PIN_REG |= SDATA_PIN_NUMBER;
	else
		SDATA_PIN_REG &= ~SDATA_PIN_NUMBER;
}

/***************************************************************************//**
 * \brief Takes the SLE pin high or low.
 *
 * @sa vADF7020_SetDATACLKDirection()
 * @param ucPin Sets the pin high or low.
 * @return None.
 ******************************************************************************/
static inline void vADF7020_WriteSLE(uint8 ucPin)
{
	if (ucPin)
		SLE_PIN_REG |= SLE_PIN_NUMBER;
	else
		SLE_PIN_REG &= ~SLE_PIN_NUMBER;
}

/***************************************************************************
 * \brief Writes a 32-bit register into the ADF7020 via the serial interface.
 *
 * Clocks a 32-bit register into the ADF7020 one bit at a time via the
 * SCLK interface. Once the 32-bits are clocked in, the latch line
 * is toggled to save the register data into the radio.
 *
 * @param ulRegValue The value of the register to program
 * @return None.
 ******************************************************************************/
static void vADF7020_WriteRegister(uint32 ulRegValue)
{
	uint8 ucLoopCount;

	/* Start with the latch and clock lines low */
	vADF7020_WriteSLE(PIN_LOW);
	vADF7020_WriteSCLK(PIN_LOW);

	/* Loop through each bit of the 32-bit value, writing each out to the radio*/
	for (ucLoopCount = 0x20; ucLoopCount > 0x00; ucLoopCount--)
	{

		/* Take the serial clock low to change the data line, as the latch is
		 * rising edge sensitive. */
		vADF7020_WriteSCLK(PIN_LOW);

		/* Determine if the MSB is a 1 or 0, write the line appropriately */
		if (ulRegValue & 0x80000000)
			vADF7020_WriteSDATA(PIN_HIGH);
		else
			vADF7020_WriteSDATA(PIN_LOW);

		/* Take the clock line high to clock the data in */
		vADF7020_WriteSCLK(PIN_HIGH);

		/* Shift to prepare the field for the next bit */
		ulRegValue <<= 1;

	}

	/* For the last bit, we need to take the clock low */
	vADF7020_WriteSCLK(PIN_LOW);

	/* To latch the shift register into the radio we need to strobe the latch. We
	 * do it twice to make sure the time high meets the timing spec'd by the
	 * radio */
	vADF7020_WriteSLE(PIN_HIGH);
	vADF7020_WriteSLE(PIN_HIGH);
	vADF7020_WriteSLE(PIN_LOW);
	vADF7020_WriteSDATA(PIN_LOW);




}

/***************************************************************************
 * \brief Writes a 9-bit register into the ADF7020 via the serial interface.
 *
 * Clocks a 9-bit register into the ADF7020 one bit at a time via the
 * SCLK interface. Once the 9-bits are clocked in, the latch line
 * is toggled to save the register data into the radio.
 *
 * @param ulRegValue The value of the register to program
 * @return None.
 ******************************************************************************/
static void vADF7020_Write_Readback_Register(uint16 unRegValue)
{
	uint8 ucLoopCount;

	/* Start with the latch and clock lines low */
	vADF7020_WriteSLE(PIN_LOW);
	vADF7020_WriteSCLK(PIN_LOW);

	/* Loop through each bit of the 9-bit value, writing each out to the radio*/
	for (ucLoopCount = 0x09; ucLoopCount > 0x00; ucLoopCount--)
	{
		/* Take the serial clock low to change the data line, as the latch is
		 * rising edge sensitive. */
		vADF7020_WriteSCLK(PIN_LOW);

		/* Determine if the MSB is a 1 or 0, write the line appropriately */
		if (unRegValue & 0x100)
			vADF7020_WriteSDATA(PIN_HIGH);
		else
			vADF7020_WriteSDATA(PIN_LOW);

		/* Take the clock line high to clock the data in */
		vADF7020_WriteSCLK(PIN_HIGH);

		/* Shift to prep the field for the next bit */
		unRegValue <<= 1;
	}

	/* For the last bit, we need to take the clock low */
	vADF7020_WriteSCLK(PIN_LOW);

	/* To latch the shift register into the radio we need to strobe the latch. We
	 * do it twice to make sure the time high meets the timing spec'd by the
	 * radio */
	vADF7020_WriteSLE(PIN_HIGH);
	vADF7020_WriteSLE(PIN_HIGH);
	vADF7020_WriteSLE(PIN_LOW);
	vADF7020_WriteSDATA(PIN_LOW);
}

/***************************************************************************
 * \brief Reads a 16-bit register from the ADF7020 via the serial interface.
 *
 * Clocks a 16-bit register out of the ADF7020 Readback register one bit at
 * a time via the SCLK interface.
 *
 * @param none
 * @return unReadbackByte The value in the ADF7020 Readback register.
 ******************************************************************************/
static uint16 unADF7020_Read_Readback_Register(void)
{

	uint8 ucLoopCount;
	uint16 unReadbackByte;

	vADF7020_WriteSCLK(PIN_LOW);
	vADF7020_WriteSDATA(PIN_LOW);
	vADF7020_WriteSLE(PIN_HIGH);

	//Clock in the first bit then discard as per ADF7020 specifications
	vADF7020_WriteSCLK(PIN_HIGH);
	__delay_cycles(100); //Extend the pulse time a little
	vADF7020_WriteSCLK(PIN_LOW);

	// Start in a known state
	unReadbackByte = 0;

	// *Note depending on the read back data requested the unReadbackByte length will be different
	for (ucLoopCount = 0x10; ucLoopCount > 0x00; ucLoopCount--)
	{
		vADF7020_WriteSCLK(PIN_HIGH);
		unReadbackByte <<= 1;
		vADF7020_WriteSCLK(PIN_LOW);

		if (SREAD_IN_REG & SREAD_PIN_NUMBER)
			unReadbackByte |= 1;
	}

	//Set the load enable pin back to state for writing to the ADF7020
	vADF7020_WriteSLE(PIN_LOW);

	return (unReadbackByte);

}

/***************************************************************************
 * \brief Requests information from the ADF7020 Readback register
 *
 * Writes the passed register value to the ADF7020 and requests the read back
 * register value.  This can be used to determine AFC shift, RSSI values, ADC values
 * silicon revision, and filter calibration.
 *
 * @param unRegValue The values to be written to the Readback register.
 * @return unReadbackByte The value in the ADF7020 Readback register.
 ******************************************************************************/
uint16 unADF7020_Read_Readback(uint16 unRegValue)
{
	uint16 unReadbackByte;
// I may need to add some catch here that checks the state of the radio before executing the code below
// The complicated problem here is that different values of unRegValue require different radio states

	vADF7020_Write_Readback_Register(unRegValue);
	unReadbackByte = unADF7020_Read_Readback_Register();

	return (unReadbackByte);
}

////////////////////////////////////////////////////////////////////////////////
//! \brief Uses the filter gain and low-noise amplifier gain settings to
//! return the gain mode correction value.  Used in calculating the RSSI in dBm.
//!
//! \param ucFG, ucLG the gain values
//! \return ucGMC, the gain mode correction value
////////////////////////////////////////////////////////////////////////////////
static uint8 ucADF7020_GetGMC(uint8 ucFG, uint8 ucLG)
{
	uint8 ucGMC;
	uint8 ucLGFG;

	//Set ucGMC to an illegal value
	ucGMC = 0xFF;

	ucLGFG = (ucLG << 4) | ucFG;

	switch (ucLGFG)
	{
		case 0x32:
			ucGMC = 0;
		break;

		case 0x22:
			ucGMC = 24;
		break;

		case 0x21:
			ucGMC = 45;
		break;

		case 0x20:
			ucGMC = 63;
		break;

		case 0x10:
			ucGMC = 90;
		break;

		case 0x00:
			ucGMC = 105;
		break;

		default:
			ucGMC = 0xFF;
		break;
	}
	return ucGMC;
}

/***************************************************************************//**
 * \brief Returns the radio state to the user.
 *
 * @return The state of the radio
 ******************************************************************************/
RadioState_t unADF7020_GetRadioState(void)
{
	return ADF7020_Driver.eRadioState;
}

/***************************************************************************//**
 * \brief Clears the RX buffer.
 *
 * Clears the RX buffer where incoming radio packets are stored. The default is
 * to zero the buffer, however the user can also specify an 8 bit field to fill
 * the buffer with.
 *
 * If the radio is in any receiving state, this call will return an error.
 *
 * @param ucByteField The value to clear the RX buffer to.
 * @return Status code indicating the results.
 */
RadioRetCode_t unADF7020_ClearRXBuffer(uint8 ucByteField)
{
	uint16 unLoopCount;

	/* If we are in a receiving state, don't let the user clear the buffer */
	if ((ADF7020_Driver.eRadioState == RX_ACTIVE) || (ADF7020_Driver.eRadioState == RX_IDLE))
	{
		return ADF7020_BAD_STATE_FOR_ACTION;
	}

	/* Write the field to the buffer */
	/* TODO: Consider using a DMA transfer and LPM to save energy */
	for (unLoopCount = 0; unLoopCount < ADF7020_RX_BUFFER_SIZE; unLoopCount++)
	{
		ADF7020_Driver.ucaRxBuffer[unLoopCount] = ucByteField;
	}

	/* Reset the position */
	ADF7020_Driver.ucRxBufferPosition = 0;

	return ADF7020_OK;
}

/***************************************************************************//**
 * \brief Clears the TX buffer.
 *
 * Clears the TX buffer where the user stores outgoing data. The default is to
 * zero the buffer, however the user can also specify an 8-bit field to fill
 * the buffer with.
 *
 * If the radio is in any TX state, this call will return an error.
 *
 * @param ucByteField The value to clear the TX buffer to.
 * @return Status code indicating the results.
 ******************************************************************************/
RadioRetCode_t unADF7020_ClearTXBuffer(uint8 ucByteField)
{
	uint16 unLoopCount;

	/* If we are in the TX state, don't let the user clear the buffer */
	if ((ADF7020_Driver.eRadioState == TX_ACTIVE) || (ADF7020_Driver.eRadioState == TX_IDLE))
	{
		return ADF7020_BAD_STATE_FOR_ACTION;
	}

	/* Write the field to the buffer */
	/* TODO: Consider using a DMA transfer and LPM to save energy */
	for (unLoopCount = 0; unLoopCount < ADF7020_TX_BUFFER_SIZE; unLoopCount++)
	{
		ADF7020_Driver.ucaTxBuffer[unLoopCount] = ucByteField;
	}

	/* Reset the position */
	ADF7020_Driver.ucTxBufferPosition = 0;

	return ADF7020_OK;
}

/***************************************************************************//**
 * \brief Forces the driver to receive a packet.
 *
 * The driver simulates an incoming packet by forcing an interrupt on
 * INT/LOCK. This results in the radio driver saving whatever interference/noise
 * it is receiving on its currently programmed channel.
 *
 * This function is used to generate "random" data used by other modules.
 *
 * @return Status code that indicated the results.
 ******************************************************************************/
RadioRetCode_t unADF7020_ForceRXPacket(void)
{
	RadioRetCode_t RetCode;

	switch (ADF7020_Driver.eRadioState)
	{
		case RX_IDLE:
			/* Force an INTLOCK interrupt which will cause the state machine to move
			 * into RX_ACTIVE and start receiving whatever garbage the radio is
			 * demodulating. */
			INTLOCK_IFG_REG |= INTLOCK_PIN_NUMBER;

			RetCode = ADF7020_OK;
		break;

		case TX_IDLE:
		case TX_ACTIVE:
		case RX_ACTIVE:
		case RADIO_OFF:
		default:
			RetCode = ADF7020_BAD_STATE_FOR_ACTION;
		break;
	}

	return RetCode;
}

/***************************************************************************//**
 * \brief Initialize the radio driver.
 *
 * Initialize the driver for operating the ADF7020. This includes configuring
 * all of the required peripherals, GPIO lines, state machines and buffers.
 *
 * At the end of the initialization, the ADF7020 will be in the OFF state and
 * all buffers will be zero'd.
 *
 * @param pConfig Pointer to a set of ADF7020 config data
 * @return Status code indicating the result
 ******************************************************************************/
RadioRetCode_t unADF7020_Initialize(ADF7020_Configuration_t * pConfig)
{
	uint8 ucLoopCount;

	/* Don't allow this function to run if the driver is already running. */
	if (ADF7020_Driver.eDriverState != SHUTDOWN)
	{
		return ADF7020_BAD_STATE_FOR_ACTION;
	}

	/* Set the status of the radio */
	ADF7020_Driver.eRadioState = RADIO_OFF;

	/* If the incoming config is null, then use the default values */
	if (pConfig)
	{
		/* Copy the incoming data to our local driver */
		for (ucLoopCount = 0; ucLoopCount < sizeof(ADF7020_Configuration_t); ucLoopCount++)
		{
			ADF7020_Driver.uConfig.ulaRegisterArray[ucLoopCount] = pConfig->ulaRegisterArray[ucLoopCount];
		}
	}
	else
	{
		/* If the user didn't provide a config, use the defaults. */
		ADF7020_Driver.uConfig.Registers.ulPacketSize = ADF7020_PACKETSIZE_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister0_RX = ADF7020_REG0RX_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister0_TX = ADF7020_REG0TX_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister1_RX = ADF7020_REG1RX_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister1_TX = ADF7020_REG1TX_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister2_RX = ADF7020_REG2RX_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister2_TX = ADF7020_REG2TX_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister3 = ADF7020_REG3_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister4 = ADF7020_REG4_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister5 = ADF7020_REG5_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister6 = ADF7020_REG6_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister7 = ADF7020_REG7_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister8 = ADF7020_REG8_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegister9 = ADF7020_REG9_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegisterA = ADF7020_REGA_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegisterB = ADF7020_REGB_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegisterC = ADF7020_REGC_DEFAULT;
		ADF7020_Driver.uConfig.Registers.ulRegisterD = ADF7020_REGD_DEFAULT;
	}

	/* Sanity check on the packet size */
	if (ADF7020_Driver.uConfig.Registers.ulPacketSize > ADF7020_MAX_BYTES_IN_PACKET)
	{
		return ADF7020_PACKET_TOO_LARGE;
	}

	/* Zero the index */
	ADF7020_Driver.ucTxBufferPosition = 0x00;
	ADF7020_Driver.ucRxBufferPosition = 0x00;

	/* Clear the buffer */
	unADF7020_ClearRXBuffer(0x00);
	unADF7020_ClearTXBuffer(0x00);

	// Configure GPIO pins that do not change
	CE_DIR_REG |= CE_PIN_NUMBER;
	SCLK_DIR_REG |= SCLK_PIN_NUMBER;
	SDATA_DIR_REG |= SDATA_PIN_NUMBER;
	SLE_DIR_REG |= SLE_PIN_NUMBER;
	SREAD_DIR_REG &= ~SREAD_PIN_NUMBER;

	// INTLOCK and DATACLK interrupts are always rising edge sensitive
	INTLOCK_IES_REG &= ~INTLOCK_PIN_NUMBER;
	DATACLK_IES_REG &= ~DATACLK_PIN_NUMBER;

	/* Set direction defaults for other pins */
	vADF7020_SetDATACLKDirection(PIN_INPUT);
	vADF7020_SetINTLOCKDirection(PIN_INPUT);
	vADF7020_SetDATAIODirection(PIN_INPUT);

	/* Disable the interrupts */
	vADF7020_SetINTLOCKInterrupt(INTERRUPT_OFF);
	vADF7020_SetDATACLKInterrupt(INTERRUPT_OFF);

	/* Make sure that the radio is off */
	vADF7020_WriteCE(PIN_LOW);

	/* Set the driver state */
	ADF7020_Driver.eDriverState = ACTIVE;
	return ADF7020_OK;
}

/***************************************************************************//**
 * \brief Sets the TX and RX frequencies for a given radio channel
 *
 * This function loads the N and Fractional N values for the PLL on the ADF7020
 * depending on what channel is desired.
 *
 * If the code is using a lookup table then the channel number is used as the
 * lookup index and the values for N and Fractional N are saved into the ADF7020
 * configuration structure.
 *
 * If no look up table is being used then we compute the N and Fractional N
 * values. First, we start with the lowest N number for our channel range. Then
 * we take the starting Fractional N number and add the required number of
 * channel steps. If the Fractional N overflows, we increment the N count and
 * set the Frac N less the overflow amount. Then we shift the value to the
 * proper bit position for the ADF7020 register and save the value into the
 * ADF7020 configuration structure. The process is the same for both the TX and
 * RX frequencies.
 *
 * @param ucChannel The channel number to use
 * @return Status code indicating the results
 ******************************************************************************/
RadioRetCode_t unADF7020_SetChannel(uint8 ucChannel)
{
#ifdef ADF7020_DRIVER_USE_CHANNEL_LUT

	/* Check to make sure that the channel is valid */
	if (ucChannel >= ADF7020_MAX_CHANNEL)
	{
		return ADF7020_CHANNEL_OUT_OF_RANGE;
	}

	// Set the global radio channel variable
	ucGLOB_radioChannel = ucChannel;

	/* Clear out the N and Frac N bits in the register, then set new ones */
	ADF7020_Driver.uConfig.Registers.ulRegister0_TX &= ~(ADF7020_N_BIT_MASK | ADF7020_FRAC_N_BIT_MASK);
	ADF7020_Driver.uConfig.Registers.ulRegister0_TX |= g_ucaADF7020_ChannelFrequencyLUT[ucChannel][ADF7020_TX_INDEX];

	/* Clear out the N and Frac N bits in the register, then set the new ones */
	ADF7020_Driver.uConfig.Registers.ulRegister0_RX &= ~(ADF7020_N_BIT_MASK | ADF7020_FRAC_N_BIT_MASK);
	ADF7020_Driver.uConfig.Registers.ulRegister0_RX |= g_ucaADF7020_ChannelFrequencyLUT[ucChannel][ADF7020_RX_INDEX];

#else /* In this case, we are not using the Channel Lookup Table and must *
       * compute the frequency on our own.                               */
	uint32 ulTempN = 0;
	uint32 ulTempFracN = 0;

	/* Don't allow this function to run if the driver isn't running. */
	if (ADF7020_Driver.eDriverState != ACTIVE)
	{
		return ADF7020_BAD_STATE_FOR_ACTION;
	}

	/* Check to make sure that the channel is valid */
	if (ucChannel >= ADF7020_MAX_CHANNEL)
	{
		return ADF7020_CHANNEL_OUT_OF_RANGE;
	}

	/* Setting the TX frequency. */
	{
		/* Start with the lowest N value */
		ulTempN = ADF7020_N_START;

		/* Add the proper number of increments per channel to the Frac N */
		ulTempFracN = ADF7020_FRAC_N_START_TX;
		ulTempFracN += ulHDWMULT_u32xu32( (uint32)ucChannel,
				ADF7020_FRAC_N_CHANNEL_STEP,
				NULL);

		/* If we overflow the Frac N... */
		if (ulTempFracN > ADF7020_FRAC_N_MAX)
		{
			/* Increment the N count, then set the Frac N less the overflow */
			ulTempN++;
			ulTempFracN = ulTempFracN - ADF7020_FRAC_N_OVERFLOW_AMOUNT;
		}

		/* Shift both the N and Frac N over to their proper positions in Register 0 */
		ulTempN <<= ADF7020_N_SHIFT;
		ulTempFracN <<= ADF7020_FRAC_N_SHIFT;

		/* Clear out the N and Frac N bits in the register, then set the new ones */
		ADF7020_Driver.uConfig.Registers.ulRegister0_TX &= ~(ADF7020_N_BIT_MASK | ADF7020_FRAC_N_BIT_MASK);

		/* Set the new N and Frac N values */
		ADF7020_Driver.uConfig.Registers.ulRegister0_TX |= (ulTempN | ulTempFracN);
	}

	/* Repeat the same process for the RX frequency */
	{
		/* Start with the lowest N value */
		ulTempN = ADF7020_N_START;

		/* Add the proper number of increments per channel to the Frac N */
		ulTempFracN =
		ADF7020_FRAC_N_START_RX + (ucChannel * ADF7020_FRAC_N_CHANNEL_STEP);

		/* If we overflow the Frac N... */
		if (ulTempFracN > ADF7020_FRAC_N_MAX)
		{
			/* Increment the N count, then set the Frac N less the overflow */
			ulTempN++;
			ulTempFracN = ulTempFracN - ADF7020_FRAC_N_OVERFLOW_AMOUNT;
		}

		/* Shift both the N and Frac N over to their proper positions in Register 0 */
		ulTempN <<= ADF7020_N_SHIFT;
		ulTempFracN <<= ADF7020_FRAC_N_SHIFT;

		/* Clear out the N and Frac N bits in the register, then set the new ones */
		ADF7020_Driver.uConfig.Registers.ulRegister0_RX &= ~(ADF7020_N_BIT_MASK | ADF7020_FRAC_N_BIT_MASK);

		/* Set the new N and Frac N values */
		ADF7020_Driver.uConfig.Registers.ulRegister0_RX |= (ulTempN | ulTempFracN);
	}
#endif

	return ADF7020_OK;
}

/***************************************************************************//**
 * \brief Loads the user's message into the Tx buffer.
 *
 * Loads the TX buffer with the user specified buffer. The amount of data
 * copied is the packet size stored in the global configuration structure
 * set by the Initialize function.
 *
 * @param pBuffer Pointer to a buffer holding the data to tx.
 * @return Status code indicating the results
 ******************************************************************************/
RadioRetCode_t unADF7020_LoadTXBuffer(uint8 * pBuffer)
{
	uint16 unLoopCount;

	/* Do not allow this function to run if the state machine is in the
	 * transmitting state. */
	if ((ADF7020_Driver.eRadioState == TX_IDLE) || (ADF7020_Driver.eRadioState == TX_ACTIVE))
	{
		return ADF7020_BAD_STATE_FOR_ACTION;
	}

	/* Copy the data into the TX buffer */
	for (unLoopCount = 0; unLoopCount < ADF7020_Driver.uConfig.Registers.ulPacketSize; unLoopCount++)
	{
		ADF7020_Driver.ucaTxBuffer[unLoopCount] = *pBuffer++;

		/* To satisfy run-length requirements, we need to whiten the data to prevent
		 * long strings of 1's or 0's. This is done by XORing each byte with its
		 * position in the packet.
		 */
		ADF7020_Driver.ucaTxBuffer[unLoopCount] ^= unLoopCount;
	}

	return ADF7020_OK;
}

/***************************************************************************//**
 * \brief Reads the RX buffer to a user defined buffer.
 *
 * Reads the RX buffer into the user specified buffer. The amount of data copied
 * is the packet size stored in the global configuration structure
 *
 * The routine doesn't check for a proper buffer passed in by the pointer, it
 * blindly copies data.
 *
 * @param pRXData Pointer to the user buffer to copy data to.
 * @return Status code indicating the results.
 ******************************************************************************/
RadioRetCode_t unADF7020_ReadRXBuffer(volatile uint8 * pRXData)
{
	uint16 unLoopCount;


	/* If the radio is currently receiving, don't allow this function to run */
	if ((ADF7020_Driver.eRadioState == RX_ACTIVE))//|| (ADF7020_Driver.eRadioState == RX_IDLE))
	{
		return ADF7020_BAD_STATE_FOR_ACTION;
	}

	// Copy the buffer to the user location.  The + 6 is for the network layer and CRC.
	for (unLoopCount = 0; unLoopCount < ADF7020_Driver.uConfig.Registers.ulPacketSize + NET_HDR_SZ + CRC_SZ; unLoopCount++)
	{
		//The data has been whitened and it must be undone here
		ADF7020_Driver.ucaRxBuffer[unLoopCount] ^= unLoopCount;

		// Read from the buffer
		*pRXData++ = ADF7020_Driver.ucaRxBuffer[unLoopCount];
	}

	/* Reset the position */
	ADF7020_Driver.ucRxBufferPosition = 0;

	return ADF7020_OK;
}

/***************************************************************************//**
 * \brief Changes the state of the radio and driver
 *
 * The function is the heart of the radio state machine and controls
 * what registers get programmed for each state.
 *
 * The user can only initiate the change into three states via this function:
 * -# OFF
 * -# RX_IDLE
 * -# TX_IDLE
 *
 * Moving the driver into other states must be done with other function calls
 * or with automatic (i.e. interrupt) transitions.
 *
 * WARNING: This function will delay for a significant amount of time as
 *          requires for the programming of ADF7020 hardware registers! See the
 *           ADF7020 datasheet for more details.
 *
 * @param eState The state to change to.
 * @return Status code indicating the results.
 ******************************************************************************/
RadioRetCode_t unADF7020_SetRadioState(RadioState_t eState)
{
	RadioRetCode_t unReturnCode;

	if (ADF7020_Driver.eDriverState == SHUTDOWN)
	{
		return ADF7020_BAD_STATE_FOR_ACTION;
	}

	switch (eState)
	{
		case RADIO_OFF:
			/* Turn off the radio by de-asserting CE */
			vADF7020_WriteCE(PIN_LOW);

			/* Wipe out the timers */
			TA0CTL = 0x0000;
			TA0CCTL0 = 0x0000;
			TA0CCTL4 = 0x0000;

			/* Update the state variable */
			ADF7020_Driver.eRadioState = RADIO_OFF;

			DATACLK_SEL_REG &= ~DATACLK_PIN_NUMBER; //if the radio was previously transmitting then the dataclk line is controlled by the timer
			vADF7020_SetDATACLKDirection(PIN_OUTPUT);
			vADF7020_SetDATAIODirection(PIN_OUTPUT);
			vADF7020_SetINTLOCKDirection(PIN_OUTPUT);

			/* Disable INT/LOCK interrupt and DATACLK interrupt */
			vADF7020_SetINTLOCKInterrupt(INTERRUPT_OFF);
			vADF7020_SetDATACLKInterrupt(INTERRUPT_OFF);
		break;

		case RX_IDLE:
		case RX_ACTIVE:
			/* Power up the chip */
			vADF7020_WriteCE(PIN_HIGH);

			/* Wait 10 us for the regulator to power up */
			__delay_cycles(160);

			/* Write registers 0 and 1 (sets the PLL freq) */
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister0_RX);
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister1_RX);

			// Wait 2 ms for the oscillator stabilize
			vTime_SetLPM_DelayAlarm(ON, 2000);
			LPM1;
			vTime_SetLPM_DelayAlarm(OFF, 0);

			/* Write the remaining registers. ORDER IS IMPORTANT */
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegisterB);
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister9);
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister2_RX);
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister3);
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister5);
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister4);
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister6);

			/* After writing register 6 the IF filter is calibrated, wait for
			 * 200 us for it to finish */
			vTime_SetLPM_DelayAlarm(ON, 200);
			LPM0;
			vTime_SetLPM_DelayAlarm(OFF, 0);

			/* Set the GPIO pins */
			DATACLK_SEL_REG &= ~DATACLK_PIN_NUMBER; //if the radio was previously transmitting then the dataclk line is controlled by the timer
			vADF7020_SetDATACLKDirection(PIN_INPUT);
			vADF7020_SetDATAIODirection(PIN_INPUT);
			vADF7020_SetINTLOCKDirection(PIN_INPUT);

			// Make sure the IFGs aren't set
			DATACLK_IFG_REG	&= ~DATACLK_PIN_NUMBER;
			INTLOCK_IFG_REG &= ~INTLOCK_PIN_NUMBER;
			/* Enable the interrupt on INT/LOCK but not DATACLK */
			vADF7020_SetINTLOCKInterrupt(INTERRUPT_ON);
			vADF7020_SetDATACLKInterrupt(INTERRUPT_OFF);

			// Start the RSSI index at zero to prepare to measure the forthcoming reception
			ucRSSI_Idx = 0x00;

			ADF7020_Driver.uConfig.Registers.ulPacketSize = ADF7020_PACKETSIZE_DEFAULT;

			/* Set the return code and state */
			ADF7020_Driver.eRadioState = RX_IDLE;
			unReturnCode = ADF7020_OK;
		break;

		case TX_IDLE:
		case TX_ACTIVE:
			/* Power up the radio */
			vADF7020_WriteCE(PIN_HIGH);

			/* Wait 10 us for the regulator to power up */
			__delay_cycles(160);

			/* Write Reg 0 and 1 */
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister0_TX);
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister1_TX);

			/* Wait 2 ms for the oscillator to stabilize after the freq change */
			vTime_SetLPM_DelayAlarm(ON, 2000);
			LPM1;
			vTime_SetLPM_DelayAlarm(OFF, 0);

			/* Write register 2 */
			vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister2_TX);

			/* Set the required output pins */
			vADF7020_SetDATACLKDirection(PIN_OUTPUT);
			vADF7020_SetDATAIODirection(PIN_OUTPUT);
			vADF7020_SetINTLOCKDirection(PIN_OUTPUT);

			/* Disable the GPIO interrupts */
			vADF7020_SetINTLOCKInterrupt(INTERRUPT_OFF);
			vADF7020_SetDATACLKInterrupt(INTERRUPT_OFF);

			/* Set the new state and the return code */
			ADF7020_Driver.eRadioState = TX_IDLE;
			unReturnCode = ADF7020_OK;
		break;

		default:
			unReturnCode = ADF7020_UNKNOWN_STATE;
		break;
	}

	return unReturnCode;
}


/********************************************************************************
 *
 * \brief quick switch from tx to rx or vice versa
 *
 * This function requires that the radio be properly configured upon power up
 * before using the 1 register write to change modes.  Therefore the adf7020_wakeup
 * function initializes the radio in RX mode.
 *
 * \param ucMode
 * ****************************************************************************/
void vADF7020_TXRXSwitch(uchar ucMode)
{
	/* Write Reg 1 */

	if (ucMode == RADIO_TX_MODE)
	{
		/* Write Reg 0 and 1 */
		vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister0_TX);

		/* Disable the GPIO interrupts */
		vADF7020_SetINTLOCKInterrupt(INTERRUPT_OFF);
		vADF7020_SetDATACLKInterrupt(INTERRUPT_OFF);

		/* Set the required output pins */
		vADF7020_SetDATACLKDirection(PIN_OUTPUT);
		vADF7020_SetDATAIODirection(PIN_OUTPUT);
		vADF7020_SetINTLOCKDirection(PIN_OUTPUT);

		/* Set the new state and the return code */
		ADF7020_Driver.eRadioState = TX_IDLE;
	}
	else if(ucMode == RADIO_RX_MODE)
	{
		/* Write registers 0 and 1 (sets the PLL freq) */
		vADF7020_WriteRegister(ADF7020_Driver.uConfig.Registers.ulRegister0_RX);
		ADF7020_Driver.uConfig.Registers.ulPacketSize = ADF7020_PACKETSIZE_DEFAULT;

		/* Set the GPIO pins */
		DATACLK_SEL_REG &= ~DATACLK_PIN_NUMBER; //if the radio was previously transmitting then the dataclk line is controlled by the timer
		vADF7020_SetDATACLKDirection(PIN_INPUT);
		vADF7020_SetDATAIODirection(PIN_INPUT);
		vADF7020_SetINTLOCKDirection(PIN_INPUT);

		// Make sure the IFGs aren't set
		DATACLK_IFG_REG	&= ~DATACLK_PIN_NUMBER;
		INTLOCK_IFG_REG &= ~INTLOCK_PIN_NUMBER;
		/* Enable the interrupt on INT/LOCK but not DATACLK */
		vADF7020_SetINTLOCKInterrupt(INTERRUPT_ON);
		vADF7020_SetDATACLKInterrupt(INTERRUPT_OFF);

		// Start the RSSI index at zero to prepare to measure the forthcoming reception
		ucRSSI_Idx = 0x00;

		/* Reset the position */
		ADF7020_Driver.ucRxBufferPosition = 0;

		// Set the state
		ADF7020_Driver.eRadioState = RX_IDLE;

	}

}

/***************************************************************************//**
 * \fn vADF7020_SetPacketLength
 *
 * \brief Sets the packet length
 *
 * @param ulLength
 *******************************************************************************/
void vADF7020_SetPacketLength(ulong ulLength)
{
	ADF7020_Driver.uConfig.Registers.ulPacketSize = ulLength;
}

/***************************************************************************//**
 * \brief Starts the transmission of a packet
 *
 * Configures the timer to begin sending out bits on the radio. The interrupt
 * changes the bits depending on the rising or falling edge of the auto
 * generated data clk.
 *
 * @return Status code indicating the results.
 ******************************************************************************/
RadioRetCode_t unADF7020_StartTransmission(void)
{
	if (ADF7020_Driver.eRadioState != TX_IDLE)
	{
		return ADF7020_BAD_STATE_FOR_ACTION;
	}

	/* Start with DATACLK and DATAIO lines low */
	vADF7020_WriteDATACLK(PIN_LOW);
	vADF7020_WriteDATAIO(PIN_LOW);

	/* Configure the timer to produce interrupts at our data rate. We are going
	 * to use the timer in up/down mode and let the CCR module output our data clk
	 * for us. This free's up out interrupt time for outputting data and making
	 * control decisions.*/
	{
		/* Zero out the control register, holding the timer in reset */
		TA0CTL = 0x0000;
		TA0CCTL0 = 0x0000;
		TA0CCTL4 = 0x0000;

		/* Configure timer to use the SMCLK / 1 = 4 MHz tick */
		TA0CTL |= (TASSEL_2 | TACLR);

		/* Timer will count up to 208, then down to 0, then back up. Every time we
		 * hit 104, we will trigger an interrupt to handle data and state machine
		 * transitions. */
//		TA0CCR0 = 208;
//		TA0CCTL4 |= CCIE | OUTMOD_6;
//		TA0CCR4 = 104;

		// 19.2 kbaud
		TA0CCR0 = 104;
		TA0CCTL4 |= CCIE | OUTMOD_6;
		TA0CCR4 = 52;

		/* Switch the DATACLK line over to Timer control */
		DATACLK_SEL_REG |= DATACLK_PIN_NUMBER;
	}

	/* Zero the state variables and counters */
	ADF7020_Driver.ucTxBufferPosition = 0;
	ADF7020_Driver.ucTxActiveByte = 0xAA;
	ADF7020_Driver.ucTxState = DATACLK_EDGE_BIT;
	ADF7020_Driver.ucTxBitCount = 0;

	/* Start the timer */
	TA0CTL |= MC__UPDOWN;

	/* Update the state variable */
	ADF7020_Driver.eRadioState = TX_ACTIVE;

	return ADF7020_OK;
}

/***************************************************************************//**
 * \brief Checks to see of the radio board is connected.
 *
 * The MUXOUT pin is set to the Regulator Ready function by default.
 * This means that when the chip enable(CE) pin is pulled high the
 * MUXOUT line will be pulled high once the ADF7020 voltage regulators
 * stabilize.  According to the datasheet this should take about 50uS
 *
 * @return True/False Status of the radio board connectivity
 ******************************************************************************/
uint8 ucADF7020_ChkforRadio(void)
{
#if RADIO_DEBUG == 1
	vSERIAL_sout("ucRADIO_chk_for_radio_board\r\n", 29);
#endif

	// Setup the necessary pins
	CE_DIR_REG |= CE_PIN_NUMBER; // Output

	// Start with the mux pin low to avoid getting a false positive on radio connectivity
	MUXOUT_DIR_REG |= MUXOUT_PIN_NUMBER;
	MUXOUT_PIN_REG &= ~MUXOUT_PIN_NUMBER;

	// Change the direction to input for the test
	MUXOUT_DIR_REG &= ~MUXOUT_PIN_NUMBER; // Input

	// Raise the CE bit to start radio and wait for MUXOUT to go high
	vADF7020_WriteCE(PIN_HIGH);

	// Clocking at 10MHz, so give the radio some time to power up (100us)
	vDELAY_wait100usTic(1);

	// Check for MUXOUT high, fail if not
	if (!(MUXOUT_IN_REG & MUXOUT_PIN_NUMBER))
	{
		vADF7020_WriteCE(PIN_LOW);
		return (FALSE);
	}

	// If we get here then the radio has been detected, lets shut off and
	// return
	vADF7020_WriteCE(PIN_LOW);
	return (TRUE);

}

/***************************************************************************//**
 * \brief Initializes the radio driver, sets the state to RX_Idle, and sets
 * the radio status flags
 *
 * This handles all the required function calls and flag setting to get the radio
 * ready to receive data.  It also informs the application that the radio is
 * ready by setting the flags.
 *
 * @return none
 ******************************************************************************/
void vADF7020_WakeUp(void)
{

#if RADIO_DEBUG == 1
	vSERIAL_init();
	vSERIAL_sout("vRADIO_init\r\n", 13);
#endif

	// Initializes the radio structure with the default values,
	// sets up the pins and turns the radio off
	// *Note it does not turn on the radio or write any values to the registers
	if (ADF7020_Driver.eDriverState != ACTIVE)
		unADF7020_Initialize(NULL);

	// Set the global radio channel variable
	ucGLOB_radioChannel = 0x00;

	// Set radio state to RX_IDLE to write to all channels
	unADF7020_SetRadioState(RX_IDLE);

	// Set the flags
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_RADIO_ON_BIT = 1;
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_RADIO_PROGRAMMED = 1;

}

/***************************************************************************//**
 * \brief Initializes the radio driver, sets the state to RX_Idle, and sets
 * the radio status flags
 *
 * This handles all the required function calls and flag setting to get the radio
 * ready to receive data.  It also informs the application that the radio is
 * ready by setting the flags.
 *
 * @return none
 ******************************************************************************/
void vADF7020_Quit(void)
{

#if RADIO_DEBUG == 1
	vSERIAL_sout("vRADIO_quit\r\n", 13);
#endif

	// Turn off the radio, clear timers and set the state variable
	unADF7020_SetRadioState(RADIO_OFF);

	//force an abort
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_ABORT_BIT = 1;

	//force a no-msg condition
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 0;
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_RADIO_ON_BIT = 0;

	return;
}

/***************************************************************************//**
 * \brief Initializes the radio driver, sets the state to RX_Idle, and simulates
 * the reception of a packet.
 *
 * This routines handles the function calls required to start up the radio and
 * simulate the reception of a packet, at which point the radio samples the
 * RF noise in the area
 *
 *
 * @return uslRetVal The first 4 bytes in the ADF7020 RX buffer
 ******************************************************************************/
usl uslADF7020_GetRandomNoise(void)
{
	usl uslRetVal;

#if RADIO_DEBUG == 1
	vSERIAL_sout("uslRADIO_getRandomNoise\r\n", 25);
#endif

	uslRetVal = 0;

	// Make sure that the radio is in receive mode and force the reception of a packet
	vADF7020_StartReceiver();
	unADF7020_ForceRXPacket();

	//Wait for the incoming packet
	while (ADF7020_Driver.eRadioState == RX_ACTIVE);

	uslRetVal |= (ulong) ADF7020_Driver.ucaRxBuffer[4];
	uslRetVal <<= 8;
	uslRetVal |= (ulong) ADF7020_Driver.ucaRxBuffer[5];
	uslRetVal <<= 8;
	uslRetVal |= (ulong) ADF7020_Driver.ucaRxBuffer[6];
	uslRetVal <<= 8;
	uslRetVal |= (ulong) ADF7020_Driver.ucaRxBuffer[7];

	vADF7020_Quit();
	return uslRetVal;
}

/***************************************************************************//**
 * \brief Checks the status of the radio to see if it is receiving
 *
 *
 ******************************************************************************/
unsigned char ucADF7020_ChkRecBusy(void)
{
#if RADIO_DEBUG == 1
	vSERIAL_sout("ucRADIO_chk_rec_busy\r\n", 22);
#endif

	//if the radio is in the receiving state then return 1
	if ((unADF7020_GetRadioState() == RX_IDLE) || (unADF7020_GetRadioState() == RX_ACTIVE))
		return 1;

	//If we get here then the radio is in a state other than receiving
	return 0;
}

/***************************************************************************//**
 * \brief Sets the driver state to active, sets the radio state to TX_IDLE, and
 * starts transmission.
 *
 * This routine calls all the required functions to wake up the radio, set the
 * state to TX and start a transmission.  The routine won't return until the
 * message has been sent.
 *
 *
 ******************************************************************************/
void vADF7020_SendMsg(void)
{
#if RADIO_DEBUG == 1
	vSERIAL_sout("vRADIO_xmit_msg\r\n", 17);
#endif

	// Make sure radio driver is active
	if (ADF7020_Driver.eDriverState != ACTIVE)
		vADF7020_WakeUp();

	// Make sure the radio is in the TX_IDLE state
	if (unADF7020_GetRadioState() != TX_IDLE)
		unADF7020_SetRadioState(TX_IDLE);

	//Configures timer and begins clocking out data
	unADF7020_StartTransmission();

	LPM0;
}

/***************************************************************************//**
 * \brief Initializes the radio driver, sets the state to RX_Idle, and sets
 *
 * This handles the required functions to start up the radio and set in RX_IDLE
 * state.  At this point the radio is listening for a sync string and the interrupts
 * are enabled
 *
 * @return none
 ******************************************************************************/
void vADF7020_StartReceiver(void)
{
#if RADIO_DEBUG == 1
	vSERIAL_sout("vRADIO_start_receiver\r\n", 23);
#endif

	// Make sure radio is on, and in RX mode
	if (ADF7020_Driver.eDriverState != ACTIVE)
		vADF7020_WakeUp();

	if (unADF7020_GetRadioState() != RX_IDLE)
		unADF7020_SetRadioState(RX_IDLE);

}

/***************************************************************************//**
 * \brief Sets the flags to exit from the receiving
 *
 *
 *
 * @return none
 ******************************************************************************/
void vADF7020_abort_receiver(void)
{
#if RADIO_DEBUG == 1
	vSERIAL_sout("vRADIO_abort_receiver\r\n", 23);
#endif

	//force an abort
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_ABORT_BIT = 1;

	//force a no-msg condition
	ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT = 0;
}


/***************************************************************************//**
 * \brief Requests data from the readback register and determines the RSSI
 *
 *	The RSSI for a single reception is stored in the ucaRSSI_Arr[] array
 *
 * @return Status of the RSSI array if full return 1
 ******************************************************************************/
uint8 ucADF7020_SampleRSSI(void)
{
	uint16 uiReadBack_RSSI;
	int16 iRSSI_dbm;
	uint8 ucGMC;
	uint8 ucRSSI;
	uint8 ucFG;
	uint8 ucLG;


	// If the RSSI array is full then do not sample
	if(ucRSSI_Idx>=0x20)
		return 1;

	// Set the readback setup register for RSSI
	vADF7020_Write_Readback_Register(ADF7020_REG7_RSSI);
	// Read from the readback register
	uiReadBack_RSSI = unADF7020_Read_Readback_Register();

	// Mask the bits from the readback register
	ucRSSI = (uchar) (uiReadBack_RSSI & 0x7F);
	ucFG = (uchar) ((uiReadBack_RSSI & 0x180) >> 7);
	ucLG = (uchar) ((uiReadBack_RSSI & 0x600) >> 9);

	ucGMC = ucADF7020_GetGMC(ucFG, ucLG);

	iRSSI_dbm = -120 + (ucRSSI + ucGMC)/2;

	ucaRSSI_Arr[ucRSSI_Idx] = iRSSI_dbm;

	ucRSSI_Idx++;

	return 0;
}


/***************************************************************************//**
 * \brief Loop through the values in the RSSI array and compute the average
 *
 *
 * @return iRSSI_Ave, average RSSI
 ******************************************************************************/
int16 iADF7020_RequestRSSI(void)
{
	int32	lRSSI_Temp;
	int16 iRSSI_Ave;
	uint8 ucCounter;

	// Initialize the temporary RSSI
	lRSSI_Temp=0;

	// Sum the values in the RSSI array
 	for(ucCounter=0; ucCounter<ucRSSI_Idx; ucCounter++)
	{
		lRSSI_Temp += ucaRSSI_Arr[ucCounter];
	}

	// Divide the sum by the total number of samples
	iRSSI_Ave = lRSSI_Temp/ucRSSI_Idx;

	// Reset the index for the next sample
	ucRSSI_Idx = 0;

	return iRSSI_Ave;
}


/***************************************************************************//**
 * \brief Fetches the automatic frequency controller (AFC) value from the
 * readback register
 *
 *
 * @return unReadbackByte, the AFC value
 ******************************************************************************/
uint16 uiADF7020_AFC(void)
{
	uint16 unReadbackByte;

	vADF7020_Write_Readback_Register(ADF7020_REG7_AFC);
	unReadbackByte = unADF7020_Read_Readback_Register();


	return unReadbackByte;
}


/***************************************************************************//**
 * \brief Performs runtime diagnostic on the radio.
 *
 *
 ******************************************************************************/
void vADF7020_RunTimeDiagnostic(uint8 *ucReturnCode, uint8 *ucReturnLength, uint8 *ucaRetData){

	uint16 uiReadback, uiProdCode;

	// Assume no errors
	*ucReturnCode = 0;
	*ucReturnLength = 1;

	// Make sure the radio is powered off before we start
	if (unADF7020_SetRadioState(RADIO_OFF) != ADF7020_OK) {
		*ucReturnCode = 1;
	}

	ADF7020_Driver.eDriverState = SHUTDOWN;

	// Set RAM copy of registers to default values and setup the necessary pins
	unADF7020_Initialize(NULL);

	if (*ucReturnCode == 0) {
		if (ucADF7020_ChkforRadio() == FALSE) {
			// Report radio not responding
			*ucReturnCode = 2;
		}
	}

	if (*ucReturnCode == 0) {
		// Turn on the radio
		vADF7020_WakeUp();

		// Test the serial interface by writing and reading to the readback register
		// Test the readback value against the product code for the ADF7020 (0x200).
		vADF7020_Write_Readback_Register(ADF7020_REG7_REV);
		// Read from the readback register
		uiReadback = unADF7020_Read_Readback_Register();

		// Shift over 4 bits to get the product code
		uiProdCode = uiReadback >> 4;
		if (uiProdCode != 0x200) {
			*ucReturnCode = 3;
			*ucaRetData++ = (uchar) (uiProdCode >> 8);
			*ucaRetData = (uchar) uiProdCode;
			*ucReturnLength = 3;
		}
	}

	// Shutdown the radio
	vADF7020_Quit();

}



