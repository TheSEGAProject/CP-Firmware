//! \file config.h
//! \brief The general port configurations

/***************************  CONFIG.H  ****************************************
 *
 * CONFIG port header file
 *
 *
 * V1.00 04/29/2002 wzr
 *	started
 *
 ******************************************************************************/

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

//include all the hal layer functions here
#include "ad.h"
#include "spi.h"
#include "ucs.h"
#include <MSP430.h>

/********************  Hardware Descriptors  *********************************/

/********************  SRAM DELCARATIONS  ************************************/

#define MAX_SRAM_ADDR_UL 0x3FFFF		//256K

/********************  FLASH DECLARATIONS  ***********************************/

#define FLASH_MAX_SIZE_IN_PAGES_I	4096
#define FLASH_MAX_SIZE_IN_PAGES_L	((long)FLASH_MAX_SIZE_IN_PAGES_I)

#define FLASH_BYTES_PER_PAGE_L		512

/********************  SYS CLK DECLARATIONS  *********************************/
//! \def MCLK_RATE
//! \brief Rate of main clock
#define MCLK_RATE 16000000L						// MCLK - Master Clock
//! \def SMCLK_RATE
//! \brief Rate of sub main clock
#define SMCLK_RATE  4000000L         	// SMCLK - 4MHz
//#define SMCLK_RATE 8388608L					// SMCLK - 8MHZ

#ifndef SMCLK_RATE
#error "SMCLK_RATE not defined"
#endif

//! \def DCORSEL_16MHZ
//! \Range selection of the DCO
#define DCORSEL_16MHZ           DCORSEL_5
//! \def DCO_MULT_16MHZ
//! \brief Multiplier bits for the FLL
#define DCO_MULT_16MHZ          488
//! \def VCORE_16MHZ
//! \brief Core voltage for a 16 MHz DCO
#define VCORE_16MHZ             PMMCOREV_2

/********************* T1 CLK TIMER DECLARATIONS  *****************************/
/* NOTE: uses LTICS (for clarity) */

#define CLK_nS_PER_LTIC_L        30518L
#define CLK_nS_PER_LTIC_UL       30518UL
#define CLK_uS_PER_LTIC_UL          31UL
#define CLK_uS_x_100_PER_LTIC_UL  3152UL

/*************************  PORT VALUE DECLARATIONS  *************************/
//Hold the reset pins on the SP boards low until they are needed
//this prevents any pull up voltage on the board which prevents SP shutdown and therefore comm.
// Current with Port Map Rev 1.4
//! \name Port Value Declarations
//! @{
//! \def PAOUT_INIT_VAL
//! \brief Port A out initial state
#define PAOUT_INIT_VAL	0x2010 // PWREN line high for pull up resistor, buzzer line held high
//! \def PBOUT_INIT_VAL
//! \brief Port B out initial state
#define PBOUT_INIT_VAL	0x0000//0x0001 // SD CS high
//! \def PCOUT_INIT_VAL
//! \brief Port C out initial state
#define PCOUT_INIT_VAL	0x0010 //FRAM CS high
//! \def PDOUT_INIT_VAL
//! \brief Port D out initial state
#define PDOUT_INIT_VAL	0x0080
//! \def PEOUT_INIT_VAL
//! \brief Port E out initial state
#define PEOUT_INIT_VAL	0x0000
//! \def PFOUT_INIT_VAL
//! \brief Port F out initial state
#define PFOUT_INIT_VAL	0x0000

//! \def PADIR_INIT_VAL
//! \brief Port A direction initial state
#define PADIR_INIT_VAL	0x0010
//! \def PBDIR_INIT_VAL
//! \brief Port B direction initial state
#define PBDIR_INIT_VAL	0xFFFF
//! \def PCDIR_INIT_VAL
//! \brief Port C  direction initial state
#define PCDIR_INIT_VAL	0xFFFF//0x177F//111
//! \def PDDIR_INIT_VAL
//! \brief Port D direction initial state
#define PDDIR_INIT_VAL	0xFFFC
//! \def PEDIR_INIT_VAL
//! \brief Port E direction initial state
#define PEDIR_INIT_VAL	0xFFFF
//! \def PFDIR_INIT_VAL
//! \brief Port F direction initial state
#define PFDIR_INIT_VAL	0x0007

//! \def PAREN_INIT_VAL
//! \brief Port A pullup resistor enable initial state
#define PAREN_INIT_VAL	0x2000
//! \def PBREN_INIT_VAL
//! \brief Port B pullup resistor enable initial state
#define PBREN_INIT_VAL	0x0000
//! \def PCREN_INIT_VAL
//! \brief Port C pullup resistor enable initial state
#define PCREN_INIT_VAL	0x0000
//! \def PDREN_INIT_VAL
//! \brief Port D pullup resistor enable initial state
#define PDREN_INIT_VAL	0x0000
//! \def PEREN_INIT_VAL
//! \brief Port E pullup resistor enable initial state
#define PEREN_INIT_VAL	0x0000
//! \def PFREN_INIT_VAL
//! \brief Port F pullup resistor enable initial state
#define PFREN_INIT_VAL	0x0000

//! \def PASEL_INIT_VAL
//! \brief Port A I/O select initial state
#define PASEL_INIT_VAL	0x0000
//! \def PBSEL_INIT_VAL
//! \brief Port B I/O select initial state
#define PBSEL_INIT_VAL	0x0000
//! \def PCSEL_INIT_VAL
//! \brief Port C I/O select initial state
#define PCSEL_INIT_VAL	0x0000
//! \def PDSEL_INIT_VAL
//! \brief Port D I/O select initial state
#define PDSEL_INIT_VAL	0x0001
//! \def PESEL_INIT_VAL
//! \brief Port E I/O select initial state
#define PESEL_INIT_VAL	0x0000
//! \def PFSEL_INIT_VAL
//! \brief Port F I/O select initial state
#define PFSEL_INIT_VAL	0x0000

//! \def PAIE_INIT_VAL
//! \brief Port A interrupt enable initial state
#define PAIE_INIT_VAL	0x2000

//! \def PAIES_INIT_VAL
//! \brief Port A interrupt edge select initial state
#define PAIES_INIT_VAL	0x2000
//! @}

// LED defines
#define DEBUG_LED_BIT			(BIT6)				// bit 7
#define DEBUG_LED_PORT			(P7OUT)				// port 7
#define RED_LED_BIT				BIT2
#define GRN_LED_BIT				BIT1
#define LED_PORT				P6OUT

// Define for the external power bit
//! \def POWER_SRC_BIT
//! \brief Define for the external power bit
#define POWER_SRC_BIT	(P2IN & BIT6)		// bit 6 port 2
// Button defines
#define BUTTON_BIT			(BIT7)			// BIT 7
#define BUTTON_DIR_PORT		(P2DIR)			// PORT 2 DIR
#define BUTTON_OUT_PORT		(P2OUT)			// PORT 2 OUT
#define BUTTON_IFG_PORT		(P2IFG)			// PORT 2 INT FLAG
#define BUTTON_IE_PORT		(P2IE)			// PORT 2 INT EN
#define BUTTON_IN_PORT		(P2IN)			// PORT 2 IN (read only)
// Button defines
#define USER_INT_BIT			(BIT7)			// BIT 7
#define USER_INT_DIR_PORT		(P2DIR)			// PORT 2 DIR
#define USER_INT_OUT_PORT		(P2OUT)			// PORT 2 OUT
#define USER_INT_IFG_PORT		(P2IFG)			// PORT 2 INT FLAG
#define USER_INT_IE_PORT		(P2IE)			// PORT 2 INT EN
#define USER_INT_IES_PORT		(P2IES)
#define USER_INT_IN_PORT		(P2IN)			// PORT 2 IN (read only)
#define USER_INT_REN_PORT		(P2REN)

// Subsecond timer defines
#define SUB_SEC_TIM_CTL		(TA1CTL)
#define SUB_SEC_TIM			(TA1R)
#define SUB_SEC_TIM_H		(TA1R>)
#define SUB_SEC_TIM_L		(TA1R_L)
#define SUB_SEC_CTL_INIT 	(0x0112)

//! \def LATENCY_TIMER_CTL
//! \brief Timer used to determine packet reception and decoding time
#define LATENCY_TIMER_CTL		(TB0CTL)

//! \def LATENCY_TIMER
//! \brief Timer register
#define LATENCY_TIMER				(TB0R)

// TODO Are these still used?????
#define ENDSLOT_TIM_CTL		(TB0CTL)
#define ENDSLOT_TIM			(TB0R)
#define ENDSLOT_INTFLG_BIT	(TBIFG)
#define ENDSLOT_INTEN_BIT	(TBIE)

// SRAM defines
#define SRAM_ADDR_LO_PORT	(PEOUT)			// PORT E (PORTS 9 & 10)
#define	SRAM_ADDR_HI_PORT	(P11OUT)		// PORT 11
#define	SRAM_ADDR_HI_2BIT	(0x03)			// BITS 0 & 1
#define SRAM_SEL_PORT		(P11OUT)			// PORT 11
#define SRAM_RW_PORT		(P7OUT)				// PORT 7
#define SRAM_CHIP_SELECT	(0x04)			// BIT 2
#define SRAM_READWRITE		(BIT7)			// BIT 3
#define SRAM_DATA_DIR_PORT	(P8DIR)		// PORT 8 DIR
#define SRAM_DATA_OUT_PORT	(P8OUT)		// PORT 8 OUT
#define SRAM_DATA_IN_PORT	(P8IN)			// PORT 8 IN (read only)

// Buzzer defines
#define BUZZER_OUT_PORT		(P1OUT)			// PORT 1 OUT
#define BUZZER_DIR_PORT		(P1DIR)			// PORT 1 DIR
#define BUZZER_SEL_PORT	(P1SEL)				//Port 1 SEL
#define BUZZER_BIT			(BIT4)				// BIT 4


// Flash and FRAM defines
#define FRAM_SEL_BIT		(BIT4)			// BIT 4
#define FRAM_SEL_OUT_PORT	(P5OUT)		    // PORT 5 OUT
#define FLASH_SEL_BIT			(BIT0)			// BIT 0
#define FLASH_SEL_OUT_PORT		(P3OUT)		    // PORT 3 OUT
#define P_SD_PWR_OUT	(P2OUT)
#define P_SD_PWR_DIR	(P2DIR)
#define SD_PWR_PIN		(BIT6)

// SPI uses USCI B1 - these should not be changed unless pin-out/USCI changes
#define	SPI_OUT_BIT			(BIT1)			// BIT 1
#define SPI_IN_BIT			(BIT2)			// BIT 2
#define SPI_CLK_BIT			(BIT3)			// BIT 3
#define SPI_DIR_PORT	(P3DIR)			// PORT 3 DIR
#define SPI_SEL_PORT	(P3SEL)			// PORT 3 SEL
#define SPI_CTL0			(UCB0CTL0)
#define SPI_CTL1			(UCB0CTL1)
#define SPI_CLK_DIVIDER		(UCB0BRW)
#define SPI_INTFLAG_REG		(UCB0IFG)
#define SPI_TX_BUF			(UCB0TXBUF)
#define SPI_RX_BUF			(UCB0RXBUF)

//Serial Comm. definitions
#define UART1_REG_IE				(UCA1IE)
#define UART1_RX_IE					(UCRXIE)
#define UART1_REG_IFG				(UCA1IFG)
#define UART1_RX_IFG				(UCRXIFG)

// T0 debug timer defines
#define T0_CTL				(TB0CTL)		// Timer B0
#define T0_COUNTER			(TB0R)
#define T0_DIV_EX_CTL		(TB0EX0)
#define T0_INT_FLG_BIT		(BIT0)

// ADC channel defines
//! \def ADC_INPUTS_PORT6
//! \brief Battery voltage input
#define ADC_INPUTS_PORT6	(BIT0)			// using only BIT 0
//! \def ADC_INPUTS_PORT5
//! \brief ADC inputs port 5
#define ADC_INPUTS_PORT5	(0x00)			// None used
//! \def BAT_SEN_EN_BIT
//! \brief Battery enable bit
#define BAT_SEN_EN_BIT		(BIT5)
//! \def BAT_VIN_BIT
//! \brief Battery input bit
#define BAT_VIN_BIT			(BIT0)
//! \def BAT_SEN_EN_PORT_DIR
//! \brief Battery enable direction
#define BAT_SEN_EN_PORT_DIR	(P5DIR) //???Why was a variable ending in DIR associated with POUT??//
//! \def BAT_SEN_EN_PORT_OUT
//! \brief Battery enable output port
#define BAT_SEN_EN_PORT_OUT	(P5OUT)
//! \def BAT_VIN_PORT_DIR
//! \brief Battery input port direction
#define BAT_VIN_PORT_DIR	(P6DIR)

//!	\brief Structure of the role and identity of the WiSARD
//!
//! This structure is used to determine the functions the WiSARD will perform
//! as well as the type of boards that are attached
struct Role
{
		unsigned char ucRole;
		unsigned char ucaSP[4][4]; //4 SP boards with 4 characters to describe them
};

#endif /* CONFIG_H_INCLUDED */

//Config function declarations
void vConfig_InitializePorts(void);

/* --------------------------  END of MODULE  ------------------------------- */
