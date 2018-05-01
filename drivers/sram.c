
/**************************  SRAM.C  *****************************************
*
* Routines to read and write the static ram
*
*
*
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include <msp430x54x.h>		//register and ram definition file
#include "../std.h"			//std include
#include "../hal/config.h" 		//system configuration definitions
#include "sram.h"			//static ram routines
#include "../serial.h"			//serial port




/************************  ucSRAM_read_B8() *******************************
*
* read a byte from the RAM DISK.
*
*
******************************************************************************/

unsigned char ucSRAM_read_B8(
		USL uslAddr		
		)
	{
	unsigned char ucAddrXI;
//	unsigned char ucAddrHI;
//	unsigned char ucAddrLO;
	unsigned int uiAddrTwoByte;
	unsigned char ucDataByte;

	/* BREAK THE ADDRESS INTO BYTES */
	//ucAddrLO = ((unsigned char) uslAddr);
	//ucAddrHI = ((unsigned char) (uslAddr >>  8));
	uiAddrTwoByte = ((unsigned int) uslAddr);
	ucAddrXI = ((unsigned char) (uslAddr >> 16));

	__bic_SR_register(GIE);						// No interrupts


	/* LATCH IN THE XI ADDR BYTE */
	SRAM_ADDR_HI_PORT &= ~SRAM_ADDR_HI_2BIT;	//clr the bits
	SRAM_ADDR_HI_PORT |= ucAddrXI;				//set the bits

	// LATCH IN THE LOWER TWO ADDR BYTES
	SRAM_ADDR_LO_PORT = uiAddrTwoByte;
	
	/* TURN ON READ ENABLE */
	SRAM_RW_PORT |= SRAM_READWRITE;			//Turn on read

	/* SET BYTE REG TO INPUT */
	SRAM_DATA_DIR_PORT = 0x00;		  			//convert latch to an input

	/* TURN ON CHIP ENABLE */
	SRAM_SEL_PORT |= SRAM_CHIP_SELECT;		//enable the data onto the bus

	/* READ THE DATA */
	ucDataByte = SRAM_DATA_IN_PORT;		  		//read the data

	/* TURN OFF CHIP ENABLE */
	SRAM_SEL_PORT &= ~SRAM_CHIP_SELECT;  		//disable the data from the bus

	__bis_SR_register(GIE);						// Yes interrupts

	#if 0
	vSERIAL_sout("SRM:RD[", 7);
	vSERIAL_HBV32out((unsigned long)uslAddr);
	vSERIAL_sout("]=", 2);
	vSERIAL_HB8out(ucDataByte);
	vSERIAL_crlf();
	#endif

	return(ucDataByte);

	}/* END: ucSRAM_read_B8() */








/************************  vSRAM_write_B8() *******************************
*
* Write a byte to the RAM DISK.
*
*
******************************************************************************/

void vSRAM_write_B8(
		USL uslAddr,		
		unsigned char ucDataByte
		)
	{
	unsigned char ucAddrXI;
//	unsigned char ucAddrHI;
//	unsigned char ucAddrLO;
	unsigned int uiAddrTwoByte;

	/* BREAK THE ADDRESS INTO BYTES */
//	ucAddrLO = ((unsigned char) uslAddr);
//	ucAddrHI = ((unsigned char) (uslAddr >>  8));
	uiAddrTwoByte = ((unsigned int) uslAddr);
	ucAddrXI = ((unsigned char) (uslAddr >> 16));

	__bic_SR_register(GIE);								// No interrupts

	/* LATCH IN THE XI ADDR BYTE */
	SRAM_ADDR_HI_PORT &= ~SRAM_ADDR_HI_2BIT;			//clr the bits
	SRAM_ADDR_HI_PORT |= ucAddrXI;						//set the bits

	// LATCH IN THE LOWER TWO ADDR BYTES
	SRAM_ADDR_LO_PORT = uiAddrTwoByte;
	
	/* TURN ON WRITE ENABLE */
	SRAM_RW_PORT &= ~SRAM_READWRITE;					//Turn on write
	
	/* MAKE SURE DATA IS POINTING IN RIGHT DIRECTION */
	SRAM_DATA_DIR_PORT = 0xFF;					  		//restore the latch to an output

	/* FLOP THE CLK UP */
	SRAM_SEL_PORT |= SRAM_CHIP_SELECT;				//clock in the data

	/* WRITE THE DATA */
	SRAM_DATA_OUT_PORT = ucDataByte;					//stuff data

   	/* FLOP THE CLK DOWN */
	SRAM_SEL_PORT &= ~SRAM_CHIP_SELECT;				//set select back to deselect

	/* TURN OFF WRITE ENABLE */
	SRAM_RW_PORT |= SRAM_READWRITE;					//set Write ena back to read

	/* SET DATA BUS BITS BACK TO INPUT */
	SRAM_DATA_DIR_PORT = 0x00;					  		//restore the latch to an intput

	__bis_SR_register(GIE);								// Yes interrupts


	#if 0
	vSERIAL_sout("SRM:WR[", 7);
	vSERIAL_HBV32out((unsigned long)uslAddr);
	vSERIAL_sout("]=", 2);
	vSERIAL_HB8out(ucDataByte);
	vSERIAL_crlf();
	#endif

	return;

	}/* END: vSRAM_write_B8() */






/***********************  uiSRAM_read_B16  ***********************************
*
* Read a Word from the SRAM
*
*
*****************************************************************************/

unsigned int uiSRAM_read_B16(
		USL uslAddr		
		)
	{
	unsigned char ucHI_val;
	unsigned char ucLO_val;
	unsigned int uiRetVal;

	/* GET THE HI BYTE VALUE */
	ucHI_val = ucSRAM_read_B8(uslAddr);
	
	/* INC THE ADDR PTR */
	uslAddr++;

	/* GET THE LOW BYTE VALUE */
	ucLO_val = ucSRAM_read_B8(uslAddr);

	uiRetVal = (unsigned int)ucHI_val;
	uiRetVal =  uiRetVal << 8;
	uiRetVal |= (unsigned int) ucLO_val;

	return(uiRetVal);

	}/* END: uiSRAM_read_B16() */







/***********************  vSRAM_write_B16  ***********************************
*
* Write a 16 bit value to the SRAM
*
*****************************************************************************/

void vSRAM_write_B16(
		USL uslAddr,	
		unsigned int uiData
		)
	{
	unsigned char ucVal;

	/* WRITE THE HI BYTE VALUE */
	ucVal = ((unsigned char) (uiData >> 8));
	vSRAM_write_B8(uslAddr, ucVal);
	
	/* INC THE ADDR PTR */
	uslAddr++;

	/* WRITE THE LOW BYTE VALUE */
	ucVal = ((unsigned char) uiData);
	vSRAM_write_B8(uslAddr, ucVal);

	return;

	}/* END: vSRAM_write_B16() */




/***********************  uiSRAM_read_B24  **********************************
*
* Read a 24bit word from the DISK
* 
* This needs a closer look and may change! since the short longs are actually longs now
*
*****************************************************************************/

USL uslSRAM_read_B24(		
		USL uslAddr			
		)		
	{
	unsigned char ucHI_val;
	unsigned char ucMD_val;
	unsigned char ucLO_val;
	USL uslRetVal;	

	// GET THE HI BYTE VALUE //
	ucHI_val = ucSRAM_read_B8(uslAddr);
	
	// GET THE MID BYTE VALUE //
	uslAddr++;
	ucMD_val = ucSRAM_read_B8(uslAddr);

	// GET THE LOW BYTE VALUE //
	uslAddr++;
	ucLO_val = ucSRAM_read_B8(uslAddr);

	uslRetVal = ucHI_val;
	uslRetVal <<= 8;
	uslRetVal |= (USL) ucMD_val; 
	uslRetVal <<= 8;
	uslRetVal |= (USL) ucLO_val; 

	return(uslRetVal);

	}// END: uslSRAM_read_B24() //






/***********************  vSRAM_write_B24  ***********************************
*
* Write a 24 bit value to the DISK
*
* This needs a closer look and may change! since the short longs are actually longs now
*
*****************************************************************************/

void vSRAM_write_B24(
		USL uslAddr,	
		USL uslData		
		)
	{
	unsigned char ucVal;

	// WRITE THE HI BYTE VALUE //
	ucVal = ((unsigned char) (uslData >> 16));
	vSRAM_write_B8(uslAddr, ucVal);
	
	// INC THE ADDR PTR //
	uslAddr++;

	// WRITE THE MD BYTE VALUE //
	ucVal = ((unsigned char) (uslData >> 8));
	vSRAM_write_B8(uslAddr, ucVal);

	// INC THE ADDR PTR //
	uslAddr++;

	// WRITE THE LOW BYTE VALUE //
	ucVal = ((unsigned char) uslData);
	vSRAM_write_B8(uslAddr, ucVal);

	return;

	}// END: vSRAM_write_B24() //





/***********************  uiSRAM_read_B32  **********************************
*
* Read a 32bit word from the DISK
*
*****************************************************************************/

unsigned long ulSRAM_read_B32(
		USL uslAddr		
		)		
	{
	unsigned char ucXI_val;
	unsigned char ucHI_val;
	unsigned char ucMD_val;
	unsigned char ucLO_val;
	unsigned long ulRetVal;	

	/* GET THE XI BYTE VALUE */
	ucXI_val = ucSRAM_read_B8(uslAddr);

	/* GET THE HI BYTE VALUE */
	uslAddr++;
	ucHI_val = ucSRAM_read_B8(uslAddr);
	
	/* GET THE MID BYTE VALUE */
	uslAddr++;
	ucMD_val = ucSRAM_read_B8(uslAddr);

	/* GET THE LOW BYTE VALUE */
	uslAddr++;
	ucLO_val = ucSRAM_read_B8(uslAddr);

	ulRetVal = ucXI_val;
	ulRetVal <<= 8;
	ulRetVal |= (unsigned long) ucHI_val; 
	ulRetVal <<= 8;
	ulRetVal |= (unsigned long) ucMD_val; 
	ulRetVal <<= 8;
	ulRetVal |= (unsigned long) ucLO_val; 

	return(ulRetVal);

	}/* END: uslSRAM_read_B32() */








/***********************  vSRAM_write_B32  ***********************************
*
* Write a 32 bit value to the DISK
*
*****************************************************************************/

void vSRAM_write_B32(
		USL uslAddr,		
		unsigned long ulData
		)
	{
	unsigned char ucVal;

	/* WRITE THE XI BYTE VALUE */
	ucVal = ((unsigned char) (ulData >> 24));
	vSRAM_write_B8(uslAddr, ucVal);
	
	/* INC THE ADDR PTR */
	uslAddr++;

	/* WRITE THE HI BYTE VALUE */
	ucVal = ((unsigned char) (ulData >> 16));
	vSRAM_write_B8(uslAddr, ucVal);

	/* INC THE ADDR PTR */
	uslAddr++;

	/* WRITE THE MD BYTE VALUE */
	ucVal = ((unsigned char) (ulData >> 8));
	vSRAM_write_B8(uslAddr, ucVal);

	/* INC THE ADDR PTR */
	uslAddr++;

	/* WRITE THE LOW BYTE VALUE */
	ucVal = ((unsigned char) ulData);
	vSRAM_write_B8(uslAddr, ucVal);

	return;

	}/* END: vSRAM_write_B32() */





/*-------------------------------  MODULE END  ------------------------------*/
