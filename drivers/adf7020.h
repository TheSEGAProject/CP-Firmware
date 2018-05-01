/*******************************************************************************
 * adf7020.c
 *
 *  By: Kenji Yamamoto
 *      BluePlasma Engineering LLC
 *
 * Driver module for utilizing the ADF7020 radio with a packet interface.
 *******************************************************************************/
#ifndef ADF7020_H_
#define ADF7020_H_

/** \defgroup ADF7020Driver ADF7020 Driver
 *
 * This software driver allows the MSP430 to interface and utilize the ADF7020
 * radio from Analog Devices. The software driver provides a packet based
 * interface.
 *
 * The basic operation is a simple state machine with 5 states:
 * -# OFF
 * -# RX_IDLE
 * -# RX_ACTIVE
 * -# TX_IDLE
 * -# TX_ACTIVE
 *
 * The user puts the radio into IDLE states in preparation for TX or RX. The
 * state machine automatically transitions into the RX_ACTIVE state from
 * RX_IDLE when an incoming packet is detected. The state machine
 * automatically transitions back to RX_IDLE when all of the packet bytes have
 * been received.
 *
 * The state machine transitions from TX_IDLE to TX_ACTIVE when the user
 * has loaded the TX buffer and tells the radio to transmit. The state
 * machine automatically transitions back into TX_IDLE after all of the
 * packet bytes have transmitted.
 *
 * IMPORTANT NOTES:
 *
 * -# When the state machine is in TX_IDLE, the radio is radiating power.
 * -# The software driver is programmed for 100 kHz channel spacing. The IF of
 * the ADF7020 is 200 kHz. This means that it is possible for non-adjacent
 * channels to interfere with each other due to the nature of RF frequencies.
 * For example, transmitting on channel 0 will be heard by a radio listening
 * on channel 0 or channel 4.
 * -# This driver uses the TimerA0 and PORT1 IVs. If these interrupts are
 * needed for multiple uses then the should probably be relocated and the code
 * modified for the changes.
 * @{
 */

#include "adf7020_reg.h"  //ADF7020 register definitions

/*************************************************************************//**
 * \brief Driver state variables.
 *
 * This enumeration is used to indicate the state of the driver code. If the
 * driver is ACTIVE then the functions are ready to be called and the radio
 * is ready for use.
 ****************************************************************************/
typedef enum eDriverState
{
	SHUTDOWN, /**< Driver is in shutdown                                     */
	ACTIVE, /**< Driver is capable of operation                            */
	TEST, /**< Driver is in a test state, should not see in release code */
	NUM_DRIVER_STATES /**< Number of driver states, useful for bounds check          */
} DriverState_t;

/*************************************************************************//**
 * \brief Radio state variables.
 *
 * This enumeration is used to indicate the physical state of the ADF7020
 * radio.
 ****************************************************************************/
typedef enum eRadioState
{
	RADIO_OFF, /**< The radio is off, shutdown via the CE pin                 */
	RX_IDLE, /**< The radio is listening for a message, IE waiting for sync */
	RX_ACTIVE, /**< The radio is in the middle of receiving a message         */
	TX_IDLE, /**< The radio is transmitting a carrier, no modulation        */
	TX_ACTIVE, /**< The radio is active transmitting a message                */
	NUM_RADIO_STATES /**< Number of radio states, useful for bounds check           */
} RadioState_t;

/*************************************************************************//**
 * \brief Driver return codes.
 *
 * Enumeration for the possible function return codes that the driver member
 * functions can produce. Containing them in an enum allows the compiler to
 * manage the numbering and ordering, if a new return codes is created then
 * we just add a new line to the enum.
 ****************************************************************************/
typedef enum eADF7020_ReturnCode
{
	ADF7020_OK, /**< Function completed successfully                  */
	ADF7020_BAD_STATE_FOR_ACTION, /**< Function cannot complete in current driver state */
	ADF7020_UNKNOWN_STATE, /**< Driver is in an unknown state                    */
	ADF7020_PACKET_TOO_LARGE, /**< Trying to write a packet that is too big         */
	ADF7020_CHANNEL_OUT_OF_RANGE, /**< Channel number isn't valid                       */
	NUM_ADF7020_RETURN_CODES /**< Number of return codes, useful for bounds check  */
} RadioRetCode_t;

/*************************************************************************//**
 * @name ADF7020 Default Register Settings
 * These defines are the default settings for the ADF7020 radio. This will
 * configure the radio for 9600 bps operation on Channel 0.
 * @{
 ****************************************************************************/

#ifdef ADF7020_BAUD_9600
#define ADF7020_PACKETSIZE_DEFAULT 0x00000040
#define ADF7020_REG0RX_DEFAULT     0x7A8C5800
#define ADF7020_REG0TX_DEFAULT     0x728C7D10
#define ADF7020_REG1RX_DEFAULT     0x004A9011
#define ADF7020_REG1TX_DEFAULT     0x000A9011
#define ADF7020_REG2RX_DEFAULT     0x8007E012
#define ADF7020_REG2TX_DEFAULT     0x80077E12
#define ADF7020_REG3_DEFAULT       0x006F2453
#define ADF7020_REG4_DEFAULT       0x01000154 // for correlator //0x01000144 // for linear demod  //
#define ADF7020_REG5_DEFAULT       0x12345675
#define ADF7020_REG6_DEFAULT       0x0DD81146
#define ADF7020_REG7_DEFAULT       0x00000000
#define ADF7020_REG8_DEFAULT       0x00000000
#define ADF7020_REG9_DEFAULT       0x00B231E9
#define ADF7020_REGA_DEFAULT       0x00000000
#define ADF7020_REGB_DEFAULT       0x00002F7B//0x00102F7B
#define ADF7020_REGC_DEFAULT       0x00000000
#define ADF7020_REGD_DEFAULT       0x00000000
#endif

//#ifdef ADF7020_BAUD_19200
//#define ADF7020_REG0RX_DEFAULT     0x7A8C5800  //default frequency 902.000138 MHz
//#define ADF7020_REG0TX_DEFAULT     0x728C7D10
//#define ADF7020_REG1RX_DEFAULT     0x007A9011//0x004A9011
//#define ADF7020_REG1TX_DEFAULT     0x007A9011//0x004A9011
//#define ADF7020_REG2RX_DEFAULT     0xC00F7E12
//#define ADF7020_REG2TX_DEFAULT     0xC00F7E12
//#define ADF7020_REG3_DEFAULT       0x006F1253
//#define ADF7020_REG4_DEFAULT       0x01000254
//#define ADF7020_REG5_DEFAULT       0x12345675
//#define ADF7020_REG6_DEFAULT       0x2DD808A6
//#define ADF7020_REG7_DEFAULT       0x00000000
//#define ADF7020_REG8_DEFAULT       0x00000000
//#define ADF7020_REG9_DEFAULT       0x00B231E9
//#define ADF7020_REGA_DEFAULT       0x0000AA2A
//#define ADF7020_REGB_DEFAULT       0x00002F7B
//#define ADF7020_REGC_DEFAULT       0x00000000
//#define ADF7020_REGD_DEFAULT       0x0000032D
//#endif

//! \def ADF7020_PACKETSIZE_DEFAULT
//! \brief Default packet size
#define ADF7020_PACKETSIZE_DEFAULT 0x00000040

//! \def ADF7020_REG0RX_DEFAULT
//! \def ADF7020_REG0TX_DEFAULT
//! \brief N register - mux output set to digital lock detect, phase-locked loop on, and N sets the desired frequency
#define ADF7020_REG0RX_DEFAULT 	(REG0_MUXOUT_DIGLCK | REG0_PLL_ON | REG0_RX | REG0_RX_INT_N | REG0_RX_FRAC_N | REG0_ADDRESS)
#define ADF7020_REG0TX_DEFAULT 	(REG0_MUXOUT_DIGLCK | REG0_PLL_ON | REG0_TX_INT_N | REG0_TX_FRAC_N | REG0_ADDRESS)

//! \def ADF7020_REG1RX_DEFAULT
//! \def ADF7020_REG1TX_DEFAULT
//! \brief Oscillator/Filter register - IF filter bandwidth=150kHz, VCO adjust=3, VCO bias=2625uA, Charge pump current=1500uA, External oscillator enabled, R Counter=1
#define ADF7020_REG1RX_DEFAULT 	(REG1_IFBW_150 | REG1_VCOADJUST_3 | REG1_VCOBIAS_2625uA | REG1_ICP_1500uA | REG1_XOSC_EN | REG1_RCOUNT_1 | REG1_ADDRESS)
#define ADF7020_REG1TX_DEFAULT 	(REG1_IFBW_150 | REG1_VCOADJUST_3 | REG1_VCOBIAS_2625uA | REG1_ICP_1500uA | REG1_XOSC_EN | REG1_RCOUNT_1 | REG1_ADDRESS)

//! \def ADF7020_REG2RX_DEFAULT
//! \def ADF7020_REG2TX_DEFAULT
//! \brief Transmit modulation register - Power amp bias=11uA, Modulation parameter = 30, Power amp=13dBm, Power amp on
#define MODULATION_PARAMETER (REG2_FSK_MP_B5 | REG2_FSK_MP_B4 | REG2_FSK_MP_B3 | REG2_FSK_MP_B2)
#define ADF7020_REG2RX_DEFAULT 	(REG2_PA_11uA | MODULATION_PARAMETER | REG2_PA_13dBm | REG2_PA_ON | REG2_ADDRESS)
#define ADF7020_REG2TX_DEFAULT	(REG2_PA_11uA | MODULATION_PARAMETER | REG2_PA_13dBm | REG2_PA_ON | REG2_ADDRESS)

//! \def ADF7020_REG3_DEFAULT
//! \brief Receiver clock register - Sequencer clock divider=111, CDR divider=18, Demodulator clock divider=1, Baseband offset clock divider=8
#define SEQ_CLKDIV (REG3_SEQ_CLKDIV_B7 | REG3_SEQ_CLKDIV_B6 | REG3_SEQ_CLKDIV_B4 | REG3_SEQ_CLKDIV_B3 | REG3_SEQ_CLKDIV_B2 | REG3_SEQ_CLKDIV_B1)
#define CDR_CLKDIV (REG3_CDR_CLKDIV_B5 | REG3_CDR_CLKDIV_B2)
#define ADF7020_REG3_DEFAULT	(SEQ_CLKDIV | CDR_CLKDIV | REG3_DEMOD_CLKDIV_1 | REG3_BBOS_CLKDIV_8 | REG3_ADDRESS)

//! \def ADF7020_REG4_DEFAULT
//! \brief Demodulator setup register - Synch word detect free running, Post demodulator bandwidwth=9, Demodulator type=correlator
#define ADF7020_REG4_DEFAULT (REG4_SWD_FREE | REG4_PSTDEMODBW_B4 | REG4_PSTDEMODBW_B1 | REG4_DEMODSEL_CORR | REG4_ADDRESS)

//! \def ADF7020_REG5_DEFAULT
//! \brief Sync byte register - Sync word = 0x123456, Errors allowed = 1, Sync length = 24 bits
#define ADF7020_REG5_DEFAULT (REG5_SYNCWORD | REG5_SYNCERR_1 | REG5_SYNCLEN_24 | REG5_ADDRESS)

//! \def ADF7020_REG6_DEFAULT
//! \brief Correlator/demodulator register - RX data inverted, IF filter divider=221, Calibrate IF, Discriminator bandwidth=138
#define IF_FILTER_DIV (REG6_IFDIV_B8 | REG6_IFDIV_B7 | REG6_IFDIV_B5 | REG6_IFDIV_B4 | REG6_IFDIV_B3 | REG6_IFDIV_B1)
#define DISC_BW (REG6_DISCBW_B8 | REG6_DISCBW_B4 | REG6_DISCBW_B2)
#define ADF7020_REG6_DEFAULT	(REG6_RX_INVRT | IF_FILTER_DIV | REG6_IF_CAL | DISC_BW | REG6_ADDRESS)

//! \def ADF7020_REG7_DEFAULT
//! \brief Readback setup register-not used in normal operation
#define ADF7020_REG7_DEFAULT	(REG7_ADDRESS)

//! \def ADF7020_REG8_DEFAULT
//! \brief Power-down test register-not used in normal operation
#define ADF7020_REG8_DEFAULT	(REG8_ADDRESS)

//! \def ADF7020_REG9_DEFAULT
//! \brief AGC register - Filter current = low, filter gain = 72, LNA gain = 30, AGC high threshold = 70, AGC low threshold = 30
#define AGC_LO_THRESH	(REG9_GL_B5 | REG9_GL_B4 | REG9_GL_B3 | REG9_GL_B2)
#define AGC_HI_THRESH (REG9_GH_B7 | REG9_GH_B3 | REG9_GH_B2)
#define ADF7020_REG9_DEFAULT	(REG9_FIL_GAIN_72 | REG9_LNA_GAIN_30 | AGC_HI_THRESH | AGC_LO_THRESH | REG9_ADDRESS)

//! \def ADF7020_REGA_DEFAULT
//! \brief AGC register 2 - AGC delay = 10, Leak factor = 10, Peak response = 2
#define ADF7020_REGA_DEFAULT ( REGA_DH_DEFAULT | REGA_GL_DEFAULT | REGA_PR_DEFAULT | REGA_ADDRESS)

//! \def ADF7020_REGB_DEFAULT
//! \brief AFC register - AFC scaling coefficient=;
#define ADF7020_REGB_DEFAULT (REGB_AFCSCALE_B10 | REGB_AFCSCALE_B8 | REGB_AFCSCALE_B7 | REGB_AFCSCALE_B6 | REGB_AFCSCALE_B5 | REGB_AFCSCALE_B3 | REGB_AFCSCALE_B2 | REGB_AFCSCALE_B1 | REGB_ADDRESS)

//! \def ADF7020_REGC_DEFAULT
//! \brief Test register - not used in normal operation
#define ADF7020_REGC_DEFAULT 	(REGC_ADDRESS)

//! \def ADF7020_REGD_DEFAULT
//! \brief Offset removal and signal gain register - not used in normal operation
#define ADF7020_REGD_DEFAULT	(REGD_KI_3 | REGD_KP_2 | REGD_ADDRESS)

///***********************  Debugging ******************************/
////#ifdef ADF7020_BAUD_19200
//#define ADF7020_PACKETSIZE_DEFAULT 0x00000040
//#define ADF7020_REG0RX_DEFAULT     0x7A8C5800  //default frequency 902.000138 MHz
//#define ADF7020_REG0TX_DEFAULT     0x728C7D10
//#define ADF7020_REG1RX_DEFAULT     0x004A9011
//#define ADF7020_REG1TX_DEFAULT     0x004A9011
//#define ADF7020_REG2RX_DEFAULT     0xC00F7E12
//#define ADF7020_REG2TX_DEFAULT     0xC00F7E12
//#define ADF7020_REG3_DEFAULT       0x006F1253
//#define ADF7020_REG4_DEFAULT       0x01000254
//#define ADF7020_REG5_DEFAULT       0x12345675
//#define ADF7020_REG6_DEFAULT       0x2DD808A6
//#define ADF7020_REG7_DEFAULT       0x00000007
//#define ADF7020_REG8_DEFAULT       0x00000008
//#define ADF7020_REG9_DEFAULT       0x00B231E9
//#define ADF7020_REGA_DEFAULT       0x0000AA2A
//#define ADF7020_REGB_DEFAULT       0x00102F7B
//#define ADF7020_REGC_DEFAULT       0x0000000C
//#define ADF7020_REGD_DEFAULT       0x0000032D
////#endif
///***********************  Debugging ******************************/

#ifdef ADF7020_BAUD_38400
#define ADF7020_PACKETSIZE_DEFAULT 0x00000040
#define ADF7020_REG0RX_DEFAULT     0x7A8D1130
#define ADF7020_REG0TX_DEFAULT     0x728D3640
#define ADF7020_REG1RX_DEFAULT     0x004A9011
#define ADF7020_REG1TX_DEFAULT     0x004A9011
#define ADF7020_REG2RX_DEFAULT     0x800F6012
#define ADF7020_REG2TX_DEFAULT     0x800F6012
#define ADF7020_REG3_DEFAULT       0x006F0953
#define ADF7020_REG4_DEFAULT       0x01000454
#define ADF7020_REG5_DEFAULT       0x12345675
#define ADF7020_REG6_DEFAULT       0x2DD808A6
#define ADF7020_REG7_DEFAULT       0x00000000
#define ADF7020_REG8_DEFAULT       0x00000000
#define ADF7020_REG9_DEFAULT       0x00B231E9
#define ADF7020_REGA_DEFAULT       0x0000AA2A
#define ADF7020_REGB_DEFAULT       0x00002F7B//
#define ADF7020_REGC_DEFAULT       0x00000000
#define ADF7020_REGD_DEFAULT       0x0000032D
#endif

/**@}*/

/*************************************************************************//**
 * @name ADF7020 Readback Register Commands
 * These defines are the commands sent to get the AFC, RSSI, and silicon revision values
 * from the ADF7020
 * @{
 ****************************************************************************/
#define ADF7020_REG7_AFC					0x0107 //get AFC read back register value
#define ADF7020_REG7_RSSI					0x0147 //get RSSI read back register value
#define ADF7020_REG7_REV					0x01C7 //get silicon revision read back register value

//! @}

/*************************************************************************//**
 * @name ADF7020 Channel Settings
 * These defines are used to configure the N and Fractional N counts in the
 * PLL controlling the carrier frequency in the ADF7020. The starting values
 * will configure the the PLL for operation on Channel 0. The channel spacing
 * is 100 kHz.
 * @{
 ****************************************************************************/
#define ADF7020_N_START                0x00000051
#define ADF7020_FRAC_N_START_TX        0x000047D1
#define ADF7020_FRAC_N_START_RX        0x00004580
#define ADF7020_FRAC_N_CHANNEL_STEP    0x00000128
#define ADF7020_FRAC_N_MAX             0x00007FFF
#define ADF7020_FRAC_N_OVERFLOW_AMOUNT 0x00008000
#define ADF7020_N_SHIFT                19
#define ADF7020_FRAC_N_SHIFT           4
#define ADF7020_N_BIT_MASK             0x07F80000
#define ADF7020_FRAC_N_BIT_MASK        0x0007FFF0
/**@}*/

/*************************************************************************//**
 * @name ADF7020 Channel Look Up
 *
 * These defines are used to interact with the channel look up table.
 * @{
 ****************************************************************************/
/** If this is defined, the the channel control will use the lookup table
 * instead of computing the register settings. */
#define ADF7020_DRIVER_USE_CHANNEL_LUT
#define ADF7020_MAX_CHANNEL          128
#define ADF7020_TX_INDEX             0
#define ADF7020_RX_INDEX             1
/**@}*/

/*************************************************************************//**
 * \brief ADF7020 Radio Register Structure
 *
 * This structure is used to hold the current values of the ADF7020 internal
 * registers. This prevents us from having to query the registers for their
 * values. We use this structure to update the internal registers as well.
 * For more information about the individual registers and the bit mapping
 * within consult the ADF7020 datasheet from Analog Devices.
 *
 * The user can use this structure to pass in new radio configuration data,
 * which is why the packet size is also included in the structure.
 *
 * We have two copies of Register 0, 1 and 2 because their values change
 * depending on whether or not the radio is transmitting or receiving.
 ****************************************************************************/
struct sADF7020_Configuration
{
		uint32 ulRegister0_RX; /**< PLL N and Frac N values for Rx          */
		uint32 ulRegister0_TX; /**< PLL N and Frac N values for TX          */
		uint32 ulRegister1_RX; /**< Oscillator and filter settings for RX   */
		uint32 ulRegister1_TX; /**< Oscillator and filter settings for TX   */
		uint32 ulRegister2_RX; /**< Modulation settings for RX              */
		uint32 ulRegister2_TX; /**< Modulation settings for TX              */
		uint32 ulRegister3; /**< Receiver clock settings                 */
		uint32 ulRegister4; /**< Demodulation settings                   */
		uint32 ulRegister5; /**< Sync word settings                      */
		uint32 ulRegister6; /**< Correlator/Demodulator settings         */
		uint32 ulRegister7; /**< Readback settings                       */
		uint32 ulRegister8; /**< Power down test settings                */
		uint32 ulRegister9; /**< AGC1 settings                           */
		uint32 ulRegisterA; /**< AGC2 settings                           */
		uint32 ulRegisterB; /**< AFC settings                            */
		uint32 ulRegisterC; /**< Test settings                           */
		uint32 ulRegisterD; /**< Offset removal and signal gain settings */
		uint32 ulPacketSize; /**< Packet size the driver will use         */
};

/*************************************************************************//**
 * \brief ADF7020 Register Union
 *
 * This union is used to access the ADF7020 Register configuration as a
 * unsigned long array. This allows for serialization of the driver settings
 * for loads and saves.
 *
 * @sa \ref sADF7020_Configuration
 ****************************************************************************/
typedef union uADF7020_Configuration
{
		uint32 ulaRegisterArray[sizeof(struct sADF7020_Configuration) / 4];
		struct sADF7020_Configuration Registers;
} ADF7020_Configuration_t;

/*************************************************************************//**
 * @name ADF7020 Driver Defines
 *
 * These defines are general values used across the ADF7020 software driver.
 * @{
 ****************************************************************************/
#define ADF7020_RX_BUFFER_SIZE  256
#define ADF7020_TX_BUFFER_SIZE  256

/** \brief Function input for taking hardware pins high
 * @sa vADF7020_SetDATACLKDirection()
 */
#define PIN_HIGH 0xFF
/** \brief Function input for taking hardware pins low
 * @sa vADF7020_SetDATACLKDirection()
 */
#define PIN_LOW  0x00

/** \brief Function input for changing hardware pin directions
 * @sa vADF7020_SetDATACLKDirection()
 */
#define PIN_INPUT 0x00
/** \brief Function input for changing hardware pin directions
 * @sa vADF7020_SetDATACLKDirection()
 */
#define PIN_OUTPUT 0xFF

/** \brief Function input for changing hardware pin interrupts
 * @sa vADF7020_SetDATACLKDirection()
 */
#define INTERRUPT_ON  0xFF
/** \brief Function input for changing hardware pin interrupts
 * @sa vADF7020_SetDATACLKDirection()
 */
#define INTERRUPT_OFF 0x00

#define TX_SHUTDOWN_BIT   0x80
#define DATACLK_EDGE_BIT  0x40

#define ADF7020_PREAMBLE_BYTE_COUNT 0x07
#define SYNC_BYTE_01 0x12
#define SYNC_BYTE_02 0x34
#define SYNC_BYTE_03 0x56

#define ADF7020_MAX_BYTES_IN_PACKET 0x40
/**@}*/

/*************************************************************************//**
 * @name CE Pin Hardware Defines
 * These defines are used by the driver to control the CE pin on the radio.
 *
 * This pin is always a MSP output / ADF7020 input. When this pin is low,
 * the ADF7020 is powered-down.
 * @{
 ****************************************************************************/
#define CE_PIN_REG          P3OUT
#define CE_DIR_REG          P3DIR
#define CE_PIN_NUMBER       BIT7
/**@}*/

/*************************************************************************//**
 * @name DATA_CLK Pin Hardware Defines
 * These defines are used by the driver to control the DATA_CLK pin on the
 * radio. The driver also requires an interrupt on DATA_CLK, so all registers
 * for interrupt control must also be included.
 *
 * This pin switched direction depending on the radio mode. It is a
 * MSP output / ADF7020 input when the radio is transmitting and a
 * MSP input / ADF7020 output when the radio is receiving. In receive mode,
 * we enable the interrupt on the rising edge so we know when to sample the
 * DATA I/O pin. In transmit mode, this pin is toggles by TimerA to produce
 * a good clock source.
 * @{
 ****************************************************************************/
#define DATACLK_PIN_REG     P1OUT
#define DATACLK_DIR_REG     P1DIR
#define DATACLK_IE_REG      P1IE
#define DATACLK_IES_REG     P1IES
#define DATACLK_IFG_REG     P1IFG
#define DATACLK_SEL_REG     P1SEL
#define DATACLK_IV_NUMBER   P1IV_P1IFG5
#define DATACLK_PIN_NUMBER  BIT5
/**@}*/

/*************************************************************************//**
 * @name MUXOUT Pin Hardware Defines
 * These defines are used by the driver to control the MUXOUT pin on the
 * radio.
 *
 * This pin is always a MSP input / ADF7020 output. The ADF7020 pulls it high
 *  when the radio is ready for operation.
 * @{
 ****************************************************************************/
#define MUXOUT_PIN_REG	  P6OUT
#define MUXOUT_DIR_REG	  P6DIR
#define MUXOUT_PIN_NUMBER   BIT5
#define MUXOUT_IN_REG	  	  P6IN
/**@}*/

/*************************************************************************//**
 * @name INT/LOCK Pin Hardware Defines
 * These defines are used by the driver to control the INT/LOCK pin on the
 * radio. The driver also requires an interrupt on INT/LOCK, so all registers
 * for interrupt control must also be included.
 *
 * This pin is always a MSP input / ADF7020 output. It goes high when the
 * radio detects the sync word. When the driver is in the RX_IDLE state we
 * are waiting on this interrupt to change us to the RX_ACTIVE state.
 * @{
 ****************************************************************************/
#define INTLOCK_PIN_REG     P1OUT
#define INTLOCK_DIR_REG     P1DIR
#define INTLOCK_IE_REG      P1IE
#define INTLOCK_IES_REG     P1IES
#define INTLOCK_IFG_REG     P1IFG
#define INTLOCK_IV_NUMBER   P1IV_P1IFG6
#define INTLOCK_PIN_NUMBER  BIT6
/**@}*/

/*************************************************************************//**
 * @name DATA_IO Pin Hardware Defines
 * These defines are used by the driver to control the DATA_IO pin on the
 * radio.
 *
 * This pin switches direction depending on the radio mode. When we are
 * receiving, this pin in a MSP430 input / ADF7020 output. When we are
 * transmitting the pin is a MSP430 output / ADF7020 input.
 * @{
 ****************************************************************************/
#define DATAIO_PIN_REG      P2OUT
#define DATAIO_DIR_REG      P2DIR
#define DATAIO_IN_REG       P2IN
#define DATAIO_PIN_NUMBER   BIT4
/**@}*/

/*************************************************************************//**
 * @name SCLK Pin Hardware Defines
 * These defines are used by the driver to control the SCLK pin on the radio.
 *
 * The SCLK pin is always a MSP430 output / ADF7020 input. A rising edge on
 * this pin shifts the value of the SDATA pin into the ADF7020's internal
 * shift register.
 * @{
 ****************************************************************************/
#define SCLK_PIN_REG        P6OUT
#define SCLK_DIR_REG        P6DIR
#define SCLK_PIN_NUMBER     BIT4
/**@}*/

/*************************************************************************//**
 * @name SDATA Pin Hardware Defines
 * These defines are used by the driver to control the SDATA pin on the radio.
 *
 * This pin is always a MSP430 output / ADF7020 input. The value on this pin
 * is stored into the ADF7020's internal shift register when a rising edge is
 * detected on the SCLK pin.
 * @{
 ****************************************************************************/
#define SDATA_PIN_REG       P6OUT
#define SDATA_DIR_REG       P6DIR
#define SDATA_PIN_NUMBER    BIT3
/**@}*/

/*************************************************************************//**
 * @name SREAD Pin Hardware Defines
 * These defines are used by the driver to control the SREAD pin on the radio.
 *
 * This pin is always a MSP430 input / ADF7020 output. This pin is used to
 * feed read back data from the ADF7020 to the MSP430
 * @{
 ****************************************************************************/
#define SREAD_DIR_REG       P1DIR
#define SREAD_IN_REG		  	P1IN
#define SREAD_PIN_NUMBER    BIT7
/**@}*/

/*************************************************************************//**
 * @name SLE Pin Hardware Defines
 * These defines are used by the driver to control the SLE pin on the radio.
 *
 * This pin is always a MSP430 output / ADF7020 input. When this pin goes
 * high, the value of the ADF7020's internal shift register is loaded into
 * ADF7020 register specified by the least significant 4 bits in the shift
 * register.
 * @{
 ****************************************************************************/
#define SLE_PIN_REG         P3OUT
#define SLE_DIR_REG         P3DIR
#define SLE_PIN_NUMBER      BIT6
/**@}*/

//! @name Channel Organization Methods
//! Allows switching between various channel organizations.
//!
//! @{

//! \def FIXED_CHANNEL_0
//! \brief Channel organization #0: data=1, discover=3, test=5
#define FIXED_CHANNEL_0         1

//! \def FIXED_CHANNEL_1
//! \brief Channel organization #1: data=0, discover=2, test=6
#define FIXED_CHANNEL_1         2

//! \def FIXED_CHANNEL_2
//! \brief Channel organization #2: data=7, discover=9, test=11
#define FIXED_CHANNEL_2         3

//! \def RANDOM_CHANNEL
//! \brief Channel organization #2: data=0-31, discover=35, test=39
#define RANDOM_CHANNEL        	4

//! \def CURRENT_CHANNELS
//! \brief Illegal channel assignment
#define CURRENT_CHANNELS			RANDOM_CHANNEL

//! \def DISCOVERY_CHANNEL
//! \brief Channel for network discovery

//! \def TEST_CHANNEL
//! \brief Channel for testing

//! \def ILLEGAL_CHANNEL
//! \brief Illegal channel assignment

#if (CURRENT_CHANNELS == FIXED_CHANNEL_0)
#define DATA_CHANNEL			    1
#define DISCOVERY_CHANNEL			3
#define TEST_CHANNEL				5
#define ILLEGAL_CHANNEL				128
#endif

#if (CURRENT_CHANNELS == FIXED_CHANNEL_1)
#define DATA_CHANNEL			    25
#define DISCOVERY_CHANNEL			35
#define TEST_CHANNEL				40
#define ILLEGAL_CHANNEL				128
#endif

#if (CURRENT_CHANNELS == FIXED_CHANNEL_2)
#define DATA_CHANNEL			    7
#define DISCOVERY_CHANNEL			9
#define TEST_CHANNEL				11
#define ILLEGAL_CHANNEL				128
#endif

#if (CURRENT_CHANNELS == RANDOM_CHANNEL)
#define DATA_CHANNEL			    0
#define DISCOVERY_CHANNEL			15
#define TEST_CHANNEL				20
#define ILLEGAL_CHANNEL				128
#endif

//! @}

//! \def CHANNEL_MASK
//! \brief bit mask for the available channels
#define CHANNEL_MASK		0x7F

//! @name Transmit Powers
//! The powers at which the radio can transmit
//! @{
//! \def XMIT_PWR_OFF
//! \brief Transmitter PA is off
#define XMIT_PWR_OFF     0x00

//! \def XMIT_PWR_LOW
//! \brief Transmitter PA is at low power
#define XMIT_PWR_LOW     0x10

//! \def XMIT_PWR_MEDIUM
//! \brief Transmitter PA is at medium power
#define XMIT_PWR_MEDIUM  0x20

//! \def XMIT_PWR_HI
//! \brief Transmitter PA is at maximum power
#define XMIT_PWR_HI      0x3F
//! @}

//! @name Radio Statuses
//! The states the radio can be in at any time
//! @{
//! \def RADIO_NOT_ON
//! \brief Radio is off
#define RADIO_NOT_ON		1

//! \def RADIO_RX_MODE
//! \brief Radio is in receive mode (but may not be actually recieving)
#define RADIO_RX_MODE		0

//! \def RADIO_TX_MODE
//! \brief Radio is in transmit mode (but may not be actually transmitting)
#define RADIO_TX_MODE		1
//! @}

/***************************************************************************//**
 * \brief ADF7020 Driver Data Structure
 *
 * This structure contains all the state variables and storage required for the
 * ADF7020 software driver.
 ******************************************************************************/
typedef struct sADF7020_Driver
{
  uint8 ucaRxBuffer[ADF7020_RX_BUFFER_SIZE]; /**< Rx data stored here      */
  uint8 ucaTxBuffer[ADF7020_TX_BUFFER_SIZE]; /**< Tx data stored here      */
  uint8 ucTxBufferPosition; /**< Indicates the active byte in ucaTxBuffer  */
  uint8 ucRxBufferPosition; /**< Indicates the active byte in ucaRxBuffer  */
  uint8 ucTxActiveByte; /**< This is the current byte being transmitted    */
  uint8 ucTxState; /**<  Bit flags controlling interrupt behavior          */
  uint8 ucRxBitCount; /**< The current bit we are recieving in the byte    */
  uint8 ucTxBitCount; /**< The current bit we are transmitting in the byte */
  uint8 ucFirstBit;
  ADF7020_Configuration_t uConfig; /**< ADF7020 Register Data              */
  RadioState_t eRadioState; /**< Current state of the ADF7020              */
  DriverState_t eDriverState; /**< Current state of the software driver    */
} ADF7020_Driver_t;



RadioRetCode_t unADF7020_ForceRXPacket(void);
RadioRetCode_t unADF7020_Initialize(ADF7020_Configuration_t * pConfig);
RadioState_t unADF7020_GetRadioState(void);
RadioRetCode_t unADF7020_SetChannel(uint8 ucChannel);
RadioRetCode_t unADF7020_SetRadioState(RadioState_t eState);
RadioRetCode_t unADF7020_StartTransmission(void);
void vADF7020_SetPacketLength(ulong ulLength);

RadioRetCode_t unADF7020_ClearRXBuffer(uint8 ucByteField);
RadioRetCode_t unADF7020_ClearTXBuffer(uint8 ucByteField);
RadioRetCode_t unADF7020_LoadTXBuffer(uint8 * pBuffer);
RadioRetCode_t unADF7020_ReadRXBuffer(volatile uint8 * pRXData);
void vADF7020_TXRXSwitch(uchar ucMode);
void vADF7020_SendMsg(void);
void vADF7020_WakeUp(void);
void vADF7020_Quit(void);
void vADF7020_abort_receiver(void);
uint8 ucADF7020_ChkforRadio(void);
uint32 uslADF7020_GetRandomNoise(void);
void vADF7020_StartReceiver();
uint8 ucADF7020_SampleRSSI(void);
int16 iADF7020_RequestRSSI(void);
uint16 uiADF7020_AFC(void);
void vADF7020_RunTimeDiagnostic(uint8 *ucReturnCode, uint8 *ucReturnLength, uint8 *ucaRetData);
/**@}*/

#endif /* ADF7020_H_ */
