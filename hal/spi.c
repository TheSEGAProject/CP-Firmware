/////////////////////////////////////////////////////////////////////////////
//! \file spi.c
//! \brief provides functionality for the spi peripheral
//! V1.00 10/14/2003 wzr
//! 	Started
//! \addtogroup HAL
//! @{
/////////////////////////////////////////////////////////////////////////////

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include <msp430.h>		//processor reg description */
#include "std.h"			//standard definitions
#include "config.h"			//system configuration description file
#include "serial.h"			//serial IO port stuff


//////////////////////////////////////////////////////////////////////////////*
//! \brief Setup the SPI port as a master,
//!
//!			1. CLK DATA ON RISING_EDGE
//!			2. CLK REST STATE IS LO
//!			3. Read data at middle of sample
//!			4. CLK RATE = FOSC/4
//!
//!
//////////////////////////////////////////////////////////////////////////////
void vSPI_Init(
		unsigned char ucMode, uint8 ucRate
		)
{

	SPI_CTL1 |= UCSWRST; // **Put state machine in reset**

	switch (ucMode)
	{
		case 0:
			SPI_CTL0 = 0xA9; // MSB first, 8-bit, 3-pin SPI, Master,
			// clk polarity low, synchronous, FALLINGEdge
		break;

		case 1:
			SPI_CTL0 = 0x29; // MSB first, 8-bit, 3-pin SPI, Master,
			// clk polarity low, synchronous, RISINGEdge
		break;

		case 2:
			SPI_CTL0 = 0x69; // MSB first, 8-bit, 3-pin SPI, Master,
			// clk polarity high, synchronous, FALLINGEdge
		break;

		case 3:
			SPI_CTL0 = 0xE9; // MSB first, 8-bit, 3-pin SPI, Master,
			// clk polarity high, synchronous, RISINGEdge
		break;

		default:

			vSERIAL_sout("SPI_initCallBad\r\n", 17);
		break;

	}/* END: switch() */
	SPI_CTL1 |= UCSSEL_2; // SMCLK used for BRCLK

	// SPI CLK runs at freq(BRCLK)/UCBR1
	if (ucRate == RATE_0) //added by cp 3/30/12 for SD card initialization
	{
		SPI_CLK_DIVIDER = 0x000A; // UCBR1 - 16-bit
	}
	else
	{
		SPI_CLK_DIVIDER = 0x0001; // UCBR1 - 16-bit
	}

	SPI_CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

	/* SET THE SPI PIN DIRECTIONS */
	SPI_DIR_PORT |= ( SPI_OUT_BIT | SPI_CLK_BIT);
	SPI_DIR_PORT &= ~SPI_IN_BIT;
	SPI_SEL_PORT |= (SPI_OUT_BIT | SPI_IN_BIT | SPI_CLK_BIT);

	return;

	}/* END: vSPI_Init() */


//////////////////////////////////////////////////////////////////////////////
//!
//! \brief Quit the SPI port as a master
//!
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vSPI_Quit(
		void
		)
	{
	
	SPI_CTL0 = UCSYNC;
	SPI_CTL1 |= UCSWRST;	// **Put state machine in reset**

	/* SET THE SPI PIN DIRECTIONS */
	SPI_DIR_PORT |= (SPI_OUT_BIT | SPI_CLK_BIT | SPI_IN_BIT );
	SPI_SEL_PORT &= ~(SPI_OUT_BIT | SPI_IN_BIT | SPI_CLK_BIT);

	return;

	}/* END: vSPI_Quit() */


//////////////////////////////////////////////////////////////////////////////
//!
//! \brief Sets the desired message buffer to a known state
//!
//!
//! \param p_ucBuff, ucLength
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vSPI_CleanBuff( uchar * p_ucBuff, uchar ucLength)
{
	uint uiCount;

	//loop sets values in buffer to 0
	for(uiCount = 0x00 ;uiCount < ucLength ;uiCount++)
		{
			*p_ucBuff = 0x00;
			p_ucBuff++;
		}
}


////////////////////////////////////////////////////////////////////////
//!
//! \brief Outputs a single byte over the SPI tx buffer and returns the
//!		received byte
//!
//! \param p_ucTXData, p_ucRXData, ucByteCount
//! \return none
///////////////////////////////////////////////////////////////////////
static unsigned char ucSPI_IO_Transaction(
		unsigned char ucOutByte
		)
	{
	#define WAITCOUNT_UL  500000UL
	unsigned long ulCounter;

	// Initialize timeout counter
	ulCounter = WAITCOUNT_UL;

	// Make sure the previous transmission is complete
  while(!(UCB0IFG & UCTXIFG));

  //stuff the output byte
	SPI_TX_BUF = ucOutByte;

	while(--ulCounter){
		if(SPI_INTFLAG_REG & UCTXIFG)
			break;	//wait for cmd to complete
		}

	// Make sure all TX RX transactions are complete
	while(UCB0STAT & UCBUSY);

	if(ulCounter == 0)
		vSERIAL_sout("SPItout\r\n", 9);

	return(SPI_RX_BUF);

	}/* END: ucSPI_IO_Transaction() */

////////////////////////////////////////////////////////////////////////
//!
//! \brief Transmits the desired number of bytes
//!
//!
//! \param p_ucTXData, ucByteCount
//! \return none
///////////////////////////////////////////////////////////////////////
void vSPI_TXBytes(unsigned char * p_ucTXData, unsigned int ucByteCount)
{

  uint16 gie = __get_SR_register() & GIE;               //Store current GIE state

  __disable_interrupt();                                  //Make this operation atomic

  // Clock out the data
	for (; ucByteCount > 0x00; --ucByteCount)
		ucSPI_IO_Transaction(*p_ucTXData++);

  UCB0RXBUF;                                              //Dummy read to empty RX buffer
                                                          //and clear any overrun conditions

  __bis_SR_register(gie);                                 //Restore original GIE state
}

////////////////////////////////////////////////////////////////////////
//!
//! \brief Receives the desired number of bytes
//!
//!
//! \param p_ucRXData
//! \param uiByteCount The number of bytes to be transmitted
//! \return none
///////////////////////////////////////////////////////////////////////
void vSPI_RX_Bytes(unsigned char * p_ucRXData, unsigned int uiByteCount)
{
  uint16 gie = __get_SR_register() & GIE;               //Store current GIE state

  __disable_interrupt();                                  //Make this operation atomic

	for (; uiByteCount > 0x00; --uiByteCount)
		*p_ucRXData++ = ucSPI_IO_Transaction(0xFF);

  __bis_SR_register(gie);                                 //Restore original GIE state

}

//////////////////////////////////////////////////////////////////////////////
//!
//! \brief MASTER Inputs a single byte from the SPI port using a dummy write.
//!
//! \param none
//! \return the byte
//////////////////////////////////////////////////////////////////////////////
unsigned char ucSPI_bin(void)
{
	return (ucSPI_IO_Transaction(0xFF));

}/* END: ucSPI_bin() */

/*********************  vSPI_bout()  **********************************
 *
 * MASTER Outputs a single byte to the SPI pprt
 * and ignores the return input.
 *
 ******************************************************************************/

void vSPI_bout(unsigned char ucOutByte)
{

	ucSPI_IO_Transaction(ucOutByte);

	return;

}/* END: vSPI_bout() */


/*-------------------------------  MODULE END  ------------------------------*/
//! @}
