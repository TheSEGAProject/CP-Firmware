///////////////////////////////////////////////////////////////////////////////
//! \file SD_Card.c
//! \brief File for the \ref SD Card Driver Module.
//!
//! This file provides all of the function implementations for the
//! \ref SD Card Driver Module.  It supports V2 SDSC and SDHC cards
//! only and requires SDSC cards to use a block length of 512 (the
//! same as an SDHC card).
//!
//! @addtogroup Hardware Tests SD_Card
//! The SD Card Test Module handles all of the communication to the SD Card as well
//! @{
///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
// By: Ryan Middleton
//     Wireless Networks Research Lab
//     Dept of Electrical Engineering, CEFNS
//     Northern Arizona University
//
// Edited by CP
//*****************************************************************************
#include <msp430.h>
#include "serial.h"
#include "spi.h"
#include "config.h"
#include "SD_Card.h"
#include "SD_Card_Testing.h"
#include "delay.h"

//Defines and Macros
#define SD_Card_OUT	P3OUT
#define SD_Card_CS	BIT0

#define HIGH	1
#define LOW		0
#define SD_CS(LEVEL) (LEVEL==LOW) ? (SD_Card_OUT &= ~SD_Card_CS) : (SD_Card_OUT |= SD_Card_CS)

#define SD_CLK_INIT		0	//SPI BRCLK at 400Khz
#define SD_CLK_OPERATE	1	//SPI BRCLK at 4Mhz

#define SD_DUMMY_CHAR		0xFF
#define SD_WRITE_TOKEN		0xE5
#define SD_READ_TOKEN		0xFE
#define SD_IF_COND_TOKEN	0x01AA

//SD Card Commands
#define SD_CMD_GO_IDLE_STATE 				0	//Resets SD card and enters idle state
#define SD_CMD_SEND_OP_COND					1	//Send host capacity support info
#define SD_CMD_SEND_IF_COND					8	//Sends SD interface host supply voltage and asks card if it can operate at that voltage
#define SD_CMD_SEND_CSD							9	//Asks card to send its card-specific data (CSD)
#define SD_CMD_SEND_CID							10	//Asks card to send its card ID (CID)
#define SD_CMD_STOP_TRANSMISSION		12	//Forces card to stop transmission in SD_CMD_READ_MULTIPLE_BLOCK
#define SD_CMD_SEND_STATUS					13 //Sends the contents of the SD cards status register
#define SD_CMD_SET_BLOCKLEN					16	//Sets block length for SDSC cards (and LOCK_UNLOCK length) - block length for other capacities is fixed at 512
#define SD_CMD_READ_SINGLE_BLOCK		17	//Reads a block of size set by SD_CMD_SET_BLOCKLEN
#define SD_CMD_READ_MULTIPLE_BLOCK	18	//Continuously transfers data blocks from card until interrupted by SD_CMD_STOP_TRANSMISSION
#define SD_CMD_WRITE_BLOCK					24	//Writes a block of size 512 or that set by SD_CMD_SET_BLOCKLEN for SDSC cards
#define SD_CMD_PROGRAM_CSD					27	//Programming of the programmable bits of the CSD
#define SD_CMD_ERASE_WR_BLK_START		32	//Sets address of first write block to be erased
#define SD_CMD_ERASE_WR_BLK_END			33	//Sets address of the last write block to be erased
#define SD_CMD_ERASE								38	//Erases write blocks in the selected range
#define SD_ACMD_SEND_SCR						51	//Reads the SD configuration register (SCR)
#define SD_CMD_APP_CMD							55	//Escapes the next command which will be an application specific command (ACMD)
#define SD_CMD_READ_OCR							58	//Reads OCR register

#define SD_ACMD_OP_COND							41	//Sends host capacity support info and activates card init process - reserved bits shall be set to 0

//#define SD_CMD_SWITCH_FUNC			6	//Checks switchable fn (mode 0) and switches fn (mode 1)
//#define SD_CMD_WRITE_MULTIPLE_BLOCK	25	//Continuously writes blocks until 'Stop Tran' token is sent (instead of 'Start Block' token)
//#define SD_CMD_SET_WRITE_PROT			28	//If card has write protection features this sets the wp bit of the addressed group (SDHC and SDXC cards not supported)
//#define SD_CMD_CLR_WRITE_PROT			29	//If card has write protection features this clears the wp bit of the addressed group (SDHC and SDXC cards not supported)
//#define SD_CMD_SEND_WRITE_PROT		30	//If card has write protection features this asks card to send status of the wp bits (SDHC and SDXC cards not supported)
//#define SD_CMD_LOCK_UNLOCK			42	//Set/Reset the password or lock/unlock card
//#define SD_CMD_GEN_CMD				56	//Either transfer data block to card or get from card for general purpose/app sepcific commands
//#define SD_CMD_CRC_ON_OFF				59	//Turns CRC option on (1) or off (0) - default for SPI mode is off
//#define SD_ACMD_STATUS				13	//Sends the SD status:
//#define SD_ACMD_SEND_NUM_WR_BLOCKS	22	//Sends # of well written (no errors) blocks - responds with 32-bit CRC data block
//#define SD_ACMD_SET_WR_BLK_ERASE_COUNT 23	//Set # of write blocks to be pre-erased before writing (faster multiple block write) - default is 1
//#define SD_ACMD_SET_CLR_CARD_DETECT	42	//Connect/disconnect 50-KOhm pull-up resistor on CS

//Internal function prototypes
static void inline SD_Send_Dummy(unsigned char uiCount);
static void inline SD_Send_Token(void);
static unsigned char inline SD_Read_Byte(void);
unsigned char SD_Card_WaitBusy(void);
void vSD_Card_GetR1(void);
void vSD_Card_GetR1b(void);
//unsigned char SD_Read_Register(unsigned char ucRegCmd, unsigned char *pucBuffer);

//Globals
#define SD_BUFFERLEN		0x10
unsigned char ucaSDTXBuffer[SD_BUFFERLEN];
unsigned char ucaSDRXBuffer[SD_BUFFERLEN];
unsigned char g_ucaExtendedResponse[16];
SD_R1 g_r1Response;
SD_R2 g_rStatus;
SD_OCR	g_rOCR;
SD_INFO g_sdInfo;

///////////////////////////////////////////////////////////////////////
//!
//! \brief Turns off the power supply to the SD card.
//!
//! It was found that the SD card has a high quiescent current so in
//! order to extend battery life we completely cutoff power to the part.
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////
void vSD_PowerOff(void)
{
	P3REN &= ~BIT2;
	P3OUT &= ~BIT2;
	P_SD_PWR_OUT &= ~SD_PWR_PIN;
	SD_CS(LOW);
}

///////////////////////////////////////////////////////////////////////
//!
//! \brief Turns on the power supply to the SD card.
//!
//! It was found that the SD card has a high quiescent current so in
//! order to extend battery life we completely cutoff power to the part.
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////
void vSD_PowerOn(void)
{
	P3REN |= BIT2;
	P3OUT |= BIT2;
	P_SD_PWR_DIR |= SD_PWR_PIN;
	P_SD_PWR_OUT |= SD_PWR_PIN;
	SD_CS(HIGH);
}

/////////////////////////////////////////////////////////////////////////
//! \fn ucSD_Send_Command(unsigned char ucCommand, unsigned long ulArgument)
//!
//! \brief Sends a command to the SD card
//!
//! Sends a specified command to the SD card and reads the response, the
//! length of which depends upon the command specified.
//!
//! \param ucCommand - The SD command to send to the card
//! \param ulArgument - The argument values for the command
//! \return SD_SUCCESS if the command succeeded, SD_FAILURE otherwise
/////////////////////////////////////////////////////////////////////////
unsigned char ucSD_Send_Command(unsigned char ucCommand, unsigned long ulArgument)
{
	signed char cByteCount;
	int i=0;

	//All SD Card commands have the form [01xxxxxx] where the x's are the command
	ucaSDTXBuffer[0] = ((ucCommand & 0x3F) | 0x40);
	ucaSDTXBuffer[1] = (unsigned char)(ulArgument >> 24);
	ucaSDTXBuffer[2] = (unsigned char)(ulArgument >> 16);
	ucaSDTXBuffer[3] = (unsigned char)(ulArgument >> 8);
	ucaSDTXBuffer[4] = (unsigned char)(ulArgument);

	//CRC - required for GO_IDLE_STATE and SEND_IF_CMD, but ignored otherwise
	ucaSDTXBuffer[0x05] = (ucCommand == SD_CMD_GO_IDLE_STATE) ? SD_IDLE_CRC   :
						  (ucCommand == SD_CMD_SEND_IF_COND)  ? SD_IFCOND_CRC :
						  SD_DEFAULT_CRC;

#if 0
	vSERIAL_sout("SD Card: sending command ", 25);
	vSERIAL_UI8out(ucCommand);
	vSERIAL_sout(" - ", 3);
#endif

	// Make sure the card is ready for the next command
	i=0;
	while (SD_Read_Byte() != 0xFF && i++ < 100);

	//Assert chip select
	SD_CS(LOW);

	// Make sure the card is ready for the next command
	i=0;
	while (SD_Read_Byte() != 0xFF && i++ < 100);

	//Send the array to the SD Card
	vSPI_TXBytes(ucaSDTXBuffer, 0x06);

	// Get the first response byte
	vSD_Card_GetR1();

	// Handle extended responses accordingly
	switch (ucCommand)
	{
		case SD_CMD_STOP_TRANSMISSION:
			vSD_Card_GetR1b();
		break;

		case SD_CMD_SEND_STATUS:
			g_rStatus.uiRaw = SD_Read_Byte();
		break;

		case SD_CMD_SEND_IF_COND:
			cByteCount = 0;
			while (cByteCount < 4)
				g_ucaExtendedResponse[(unsigned char)cByteCount++] = SD_Read_Byte();
		break;

		case SD_CMD_SEND_CSD:
			cByteCount = 15;
			i=0;

			// Wait for the start token
			while(SD_Read_Byte() != 0xFE && i++ < 10);

			// If we received the start token then read the register
			if (i < 10) {
				while (cByteCount >= 0)
					g_ucaExtendedResponse[(unsigned char) cByteCount--] = SD_Read_Byte();
			}

			// Get device size
			if((g_ucaExtendedResponse[15] & 0xC0) == 0){ //Is the CSD structure of version 1.0
					ulong ulBlockNR;
					ulong ulCSize;
					ulong ulMult;
					ulong ulBlockLen;

					// Device Size = BlockNR * BlockLen
					// BlockNR = (CSize + 1) * Mult
					// Mult = 2 ^ (CSize_Multiplier + 2)
					// BlockLen = 2 ^ ReadBlockLen;

					// Parse the response and compute intermediate values
					ulMult = 1 << ((((g_ucaExtendedResponse[6] & 0x03) << 1) + (g_ucaExtendedResponse[5] >> 7)) + 2);
					ulBlockLen = (uint32) 1 << (g_ucaExtendedResponse[10] & 0x0F);
					ulCSize = ((uint32) (g_ucaExtendedResponse[9] & 0x03) << 10) + (g_ucaExtendedResponse[8] << 2) + (g_ucaExtendedResponse[7] >> 6 & 0x03);
					ulBlockNR = (ulCSize + 1) * ulMult;

					// Compute the number of blocks
					g_sdInfo.ulNumBlocks = (ulBlockNR * ulBlockLen)/SD_CARD_BLOCKLEN;
			}
			else {
				ulong ulTemp;
				ulTemp = (uint32) g_ucaExtendedResponse[8] & 0x3F;
				ulTemp = ulTemp << 8;
				ulTemp += (uint32) g_ucaExtendedResponse[7];
				ulTemp = ulTemp << 8;
				ulTemp += (uint32) g_ucaExtendedResponse[6];

				// Get the size in KB
				ulTemp = (ulTemp + 1)*512;
				 g_sdInfo.ulNumBlocks = ((ulTemp/SD_CARD_BLOCKLEN)*1000);
			}
		break;


		case SD_CMD_READ_OCR:
			g_rOCR.ulRaw = 0L;
			cByteCount = 3;
			while (cByteCount >= 0)
				g_rOCR.ucBytes[(unsigned char)cByteCount--] = SD_Read_Byte();
		break;

		case SD_ACMD_SEND_SCR:
			cByteCount = 7;
			i=0;

			// Wait for the start token
			while(SD_Read_Byte() != 0xFE && i++ < 10);

			// If we received the start token then read the register
			if (i < 10) {
				while (cByteCount >= 0)
					g_ucaExtendedResponse[(unsigned char) cByteCount--] = SD_Read_Byte();

			uchar ucSDSpec = g_ucaExtendedResponse[7] & 0x0F;
			uchar ucSDSpec3 = (g_ucaExtendedResponse[5] & 0x80) >> 7;
			uchar ucSDSpec4 = (g_ucaExtendedResponse[5] & 0x04) >> 2;

			if (ucSDSpec == 0)
					g_sdInfo.ucVersion = SD_VERSION_1;
				else if (ucSDSpec == 1)
					g_sdInfo.ucVersion = SD_VERSION_1;
				else if (ucSDSpec == 2 && ucSDSpec3 == 0 && ucSDSpec4 == 0)
					g_sdInfo.ucVersion = SD_VERSION_2;
				else if (ucSDSpec == 2 && ucSDSpec3 == 1 && ucSDSpec4 == 0)
					g_sdInfo.ucVersion = SD_VERSION_3;
				else if (ucSDSpec == 2 && ucSDSpec3 == 1 && ucSDSpec4 == 1)
					g_sdInfo.ucVersion = SD_VERSION_4;

			}

		break;
	}

	// De-assert the chip select
	SD_CS(HIGH);
	SD_Read_Byte();


	#if 0
	SD_Testing_EnumerateResponse();
#else
	vDELAY_LPMWait1us(6250,0);
#endif

	//If the card responds with a 0 or a 1 (in idle state) then success, otherwise an error occurred
	if(g_r1Response.uiRaw > 0x01) return SD_FAILED;
	else return SD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////
//! \fn unsigned char SD_Init(void)
//!	Edited by CP
//! \brief This function initializes the SD card
//!
//! Sends a specified command to the SD card and reads the response, the
//! length of which depends upon the command specified.
//!
//! \param none
//! \return SD_SUCCESS if initialization succeeds, SD_FAILED otherwise
/////////////////////////////////////////////////////////////////////////
unsigned char ucSD_Init(void)
{
	uint16 uiRetries = 0;

	//Set the RX and TX buffers to a known state
	vSPI_CleanBuff(ucaSDTXBuffer, SD_BUFFERLEN);
	vSPI_CleanBuff(ucaSDRXBuffer, SD_BUFFERLEN);

	// Initialize the SPI peripheral, start the clock at 400 kHz as per SD Card specifications
	vSPI_Init(SPI_MODE_2, RATE_0);

	//Clock 80 cycles (74 min. required) over SPI MOSI
	SD_Send_Dummy(10);

	//Start by instructing the card to go into the idle state
	ucSD_Send_Command(SD_CMD_GO_IDLE_STATE, 0x00);
	if(g_r1Response.uiRaw != 1)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	//Set allowable voltages (and ensure that the card is V2+)
	ucSD_Send_Command(SD_CMD_SEND_IF_COND, SD_IF_COND_TOKEN);
	if(g_r1Response.uiRaw != 1)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	//Wait for the card to finish initializing
	for(uiRetries = SD_CMD_TIMEOUT; uiRetries > 0; uiRetries--) {

		//Escape the next command (an app command)
		ucSD_Send_Command(SD_CMD_APP_CMD, 0x00);
		if(g_r1Response.uiRaw > 1)
		{
			vSPI_Quit();
			return SD_FAILED;
		}

		//Poll the status of the card to see if the card has left idle state
		ucSD_Send_Command(SD_ACMD_OP_COND, 0x40000000);
		if(g_r1Response.uiRaw == 0) break;
		else if(g_r1Response.uiRaw > 1)
		{
			uiRetries = 0;
			break;
		}
	}

	if(uiRetries == 0)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	//Determine if the card is SDSC or SDHC
	ucSD_Send_Command(SD_CMD_READ_OCR, 0x00);
	if(g_r1Response.uiRaw > 0)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	// Read the status register
	ucSD_Send_Command(SD_CMD_SEND_STATUS, 0);
	if (g_r1Response.uiRaw > 0) {
		vSPI_Quit();
		return SD_FAILED;
	}

	// Determine the card capacity from the OCR register
	if(g_rOCR.flags.ucCCS == 1)
		g_sdInfo.ucType = SD_TYPE_SDHC;
	else
		g_sdInfo.ucType = SD_TYPE_SDSC;

	//Set block length (for SDSC cards) to 512 bytes
	if(g_sdInfo.ucType == SD_TYPE_SDSC)
	{
		ucSD_Send_Command(SD_CMD_SET_BLOCKLEN, SD_CARD_BLOCKLEN);
		if(g_r1Response.uiRaw > 0)
		{
			vSPI_Quit();
			return SD_FAILED;
		}
	}

	//Card is initialized, increase the SPI clock to 4 MHz
	vSPI_Init(SPI_MODE_2, RATE_1);
	return SD_SUCCESS;
}

////////////////////////////////////////////////////////////////////////
//!
//! \brief Returns the capacity of the SD card
//!
//! \param none
//! \return The capacity of the card in number of blocks
////////////////////////////////////////////////////////////////////////
unsigned long ulSD_GetCapacity(void)
{
	return g_sdInfo.ulNumBlocks;
}

////////////////////////////////////////////////////////////////////////
//!
//! \brief Returns the type of the SD card
//!
//! \param none
//! \return The capacity of the card in number of blocks
////////////////////////////////////////////////////////////////////////
unsigned char ucSD_GetType(void)
{
	return g_sdInfo.ucType;
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function writes a block
//!
//! This function writes a block of data from the given buffer into the specified block
//!
//! \param pucData_TXBuffer - The buffer with the data to write
//! \param unsigned long ulBlockNum - The block to which to write the data
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
char SD_Write_Block(unsigned char *pucData_TXBuffer, unsigned long ulBlockNum)
{
	unsigned char ucReturn;
	unsigned int i;

	vSPI_Init(SPI_MODE_2, RATE_1);

	//Correct address for SDSC cards which use byte-addressing instead of block-addressing
	if(g_sdInfo.ucType == SD_TYPE_SDSC) ulBlockNum *= SD_CARD_BLOCKLEN;

	//Send the WRITE_BLOCK command
	if(ucSD_Send_Command(SD_CMD_WRITE_BLOCK, ulBlockNum) == SD_FAILED)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	// Make sure the card is ready before proceeding
	i=0;
	while (SD_Read_Byte() != 0xFF && i++ < 100);

	SD_CS(LOW);

	// Make sure the card is ready before proceeding
	i=0;
	while (SD_Read_Byte() != 0xFF && i++ < 100);

	//SD card protocol requires start block token before a block write
	SD_Send_Token();

	vSPI_TXBytes(pucData_TXBuffer, SD_CARD_BLOCKLEN);

	//16-bit CRC - disabled by default so values not predictable
	SD_Send_Dummy(2);

	//Wait for the write to finish
	ucReturn = SD_Card_WaitBusy();
	SD_CS(HIGH);

	if(ucReturn == SD_FAILED)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	// The data token following the write includes a CRC error and an unspecified error
	// but is not comprehensive.  Therefore check the status register to verify the write.
	ucSD_Send_Command(SD_CMD_SEND_STATUS, 0);
	if (g_r1Response.uiRaw > 0 || g_rStatus.uiRaw > 0) {
		vSPI_Quit();
		return SD_FAILED;
	}

//Ultra safe write check - verify the written data
#if 0
	//Read back the written block
	ucReturn = SD_Read_Block(ucaSDRXBuffer, ulBlockNum);
	if(ucReturn == SD_FAILED)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	//Verify that the read data matches the written data
	for(i=0; i<SD_CARD_BLOCKLEN; i++)
	{
		if(ucaSDRXBuffer[i] != pucData_TXBuffer[i])
		{
			vSPI_Quit();
			return SD_FAILED;
		}
	}
#endif

	vSPI_Quit();
	return SD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function reads a block
//!
//! This function reads a block of 512 bytes from the specified block number
//!
//! \param pucData_RXBuffer - The buffer into which to read the data
//! \param ulBlockNum - Which block to read from
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
unsigned char SD_Read_Block(unsigned char *pucData_RXBuffer, unsigned long ulBlockNum)
{
	int iCount;
	unsigned char ucByte;

	vSPI_Init(SPI_MODE_2, RATE_0);

	//Correct address for SDSC cards which use byte-addressing instead of block-addressing
	if(g_sdInfo.ucType == SD_TYPE_SDSC) ulBlockNum *= SD_CARD_BLOCKLEN;

	//Send the command to write a block
	if (ucSD_Send_Command(SD_CMD_READ_SINGLE_BLOCK, ulBlockNum) == SD_FAILED) {
		vSPI_Quit();
		return SD_FAILED;
	}

	// Check for errors and idle state
	if (g_r1Response.uiRaw != 0) {
		vSPI_Quit();
		return SD_FAILED;
	}

	//Assert CS
	SD_CS(LOW);

	//Wait for the token preceding the data block
	for (iCount = SD_CMD_TIMEOUT; iCount > 0; iCount--) {
		ucByte = SD_Read_Byte();
		if (ucByte == 0xFE)
			break;
	}
	if (iCount == 0) {
		vSPI_Quit();
		return SD_FAILED;
	}

	//Read the data block and 2-byte CRC
	vSPI_RX_Bytes(pucData_RXBuffer, SD_CARD_BLOCKLEN);
	SD_Read_Byte();
	SD_Read_Byte();

	//De-assert CS
	SD_CS(HIGH);

	// The data token following the write includes a CRC error and an unspecified error
	// but is not comprehensive.  Therefore check the status register to verify the write.
	ucSD_Send_Command(SD_CMD_SEND_STATUS, 0);
	if (g_r1Response.uiRaw > 0 || g_rStatus.uiRaw > 0) {
		vSPI_Quit();
		return SD_FAILED;
	}

	vSPI_Quit();
	return SD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function reads a block
//!
//! This function several 512 byte blocks starting at the specified block number
//!
//! \param pucData_RXBuffer - The buffer into which to read the data
//! \param ulStartBlock - Which block to start reading from
//! \param ucBlockCount - The number of blocks to read
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
unsigned char SD_Read_MultipleBlocks(unsigned char *pucData_RXBuffer, unsigned long ulStartBlock, uchar ucBlockCount)
{
	int iCount;
	unsigned char ucByte;
	unsigned char ucBlockNum;
	vSPI_Init(SPI_MODE_2, RATE_1);

	//Correct address for SDSC cards which use byte-addressing instead of block-addressing
	if(g_sdInfo.ucType == SD_TYPE_SDSC)
		ulStartBlock *= SD_CARD_BLOCKLEN;

	//Send the command to write a block
	if (ucSD_Send_Command(SD_CMD_READ_MULTIPLE_BLOCK, ulStartBlock) == SD_FAILED) {
		vSPI_Quit();
		return SD_FAILED;
	}

	// Check for errors and idle state
	if (g_r1Response.uiRaw != 0) {
		vSPI_Quit();
		return SD_FAILED;
	}

	//Assert CS
	SD_CS(LOW);

	for (ucBlockNum = 0; ucBlockNum < ucBlockCount; ucBlockNum++) {
	//Wait for the token preceding the data block
	for (iCount = SD_CMD_TIMEOUT; iCount > 0; iCount--) {
		ucByte = SD_Read_Byte();
		if (ucByte == 0xFE)
			break;
	}
	if (iCount == 0) {
		vSPI_Quit();
		return SD_FAILED;
	}

	//Read the data block and 2-byte CRC
	vSPI_RX_Bytes(pucData_RXBuffer, SD_CARD_BLOCKLEN);
	SD_Read_Byte();
	SD_Read_Byte();

	}

	//De-assert CS
	SD_CS(HIGH);

	if(ucSD_Send_Command(SD_CMD_STOP_TRANSMISSION, 0) == SD_FAILED) {
		vSPI_Quit();
		return SD_FAILED;
	}
	vSPI_Quit();
	return SD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function erases a block
//!
//! This function erases the specified block of data by filling each byte with 0x00 or
//! 0xFF (determined by the brand of the card)
//!
//! \param ulBlockNum - The block to erase
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
char SD_Erase_Block(unsigned long ulBlockNum)
{
	vSPI_Init(SPI_MODE_2, RATE_1);

	//Correct address for SDSC cards which use byte-addressing instead of block-addressing
	if(g_sdInfo.ucType == SD_TYPE_SDSC) ulBlockNum *= SD_CARD_BLOCKLEN;

	if (ucSD_Send_Command(SD_CMD_ERASE_WR_BLK_START, ulBlockNum) == SD_FAILED)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	if (ucSD_Send_Command(SD_CMD_ERASE_WR_BLK_END, ulBlockNum) == SD_FAILED)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	if (ucSD_Send_Command(SD_CMD_ERASE, 0x0000) == SD_FAILED)
	{
		vSPI_Quit();
		return SD_FAILED;
	}

	vSPI_Quit();

	return SD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function formats the SD card
//!
//! This function clocks out a specified number of dummy bytes over the SPI MOSI line
//!
//! \param uiCount - The number of dummy bytes to write
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
unsigned char SD_Format()
{
	int i;
	unsigned char ucResult;
	unsigned char ucaBuffer[SD_CARD_BLOCKLEN];
	SD_DataBlock *pDataBlock = ucaBuffer;

	//Zero out the data block buffer
	for(i=0; i<SD_CARD_BLOCKLEN; i++) ucaBuffer[i] = 0xAA;

	//Initialize the data block values
	pDataBlock->magic_num[0] = 'S';
	pDataBlock->magic_num[1] = 'E';
	pDataBlock->magic_num[2] = 'G';
	pDataBlock->magic_num[3] = 'A';
	pDataBlock->start_block  = SD_CARD_START_BLOCK;
	pDataBlock->version      = SD_DRIVER_VERSION;

	//Write the data block to the card
	ucResult = SD_Write_Block(ucaBuffer, SD_CARD_DATA_BLOCK);
	if(ucResult == SD_FAILED)
	{
#if 1
		vSERIAL_sout("Unable to format!\r\n", 19);
#endif
		return SD_FAILED;
	}

	return SD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This inline function checks to see if the SD card is formatted
//!
//! This function reads the SD_CARD_DATA_BLOCK and checks the magic value to verify
//! if the card has been formatted.
//!
//! \param uiCount - The number of dummy bytes to write
//! \return SD_SUCCESS if formatted, SD_FAILED otherwise
unsigned char SD_CheckFormat()
{
	unsigned char result;
	SD_DataBlock *pDataBlock = ucaSDRXBuffer;

	//Attempt to read the data block
	result = SD_Read_Block(ucaSDRXBuffer, SD_CARD_DATA_BLOCK);
	if(result == SD_FAILED) return SD_FAILED;

	//Check the magic value
	if(pDataBlock->magic_num[0] != 'S' || pDataBlock->magic_num[1] != 'E' || pDataBlock->magic_num[2] != 'G' || pDataBlock->magic_num[3] != 'A')
		return SD_FAILED;
	else
		return SD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This inline function sends dummy bytes to the SD card
//!
//! This function clocks out a specified number of dummy bytes over the SPI MOSI line
//!
//! \param uiCount - The number of dummy bytes to write
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
static void inline SD_Send_Dummy(unsigned char uiCount)
{
	ucaSDTXBuffer[0] = SD_DUMMY_CHAR;
	while(uiCount--) {
		vSPI_bout(SD_DUMMY_CHAR);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This inline function sends the write block token to the SD card
//!
//! This function sends the 0xFE token to the SD card, indicating that the proceeding
//! bytes are to be written to the card
//!
//! \param none
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
static void inline SD_Send_Token() { vSPI_bout(0xFE); }

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function reads a byte from the SD card
//!
//! This function clocks in a byte from the SPI MISO line
//!
//! \param *pucData_TXBuffer, unsigned long ulAddress
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
static unsigned char inline SD_Read_Byte() { return ucSPI_bin(); }

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function waits for a write operation to finish
//!
//! This function reads in the result byte (0E5 = success) from a write operation and
//! then polls the card until it has finished its operation (no more 0s returned)
//!
//! \param none
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
unsigned char SD_Card_WaitBusy(void)
{
	//Response comes 1-8 bytes after the command
	//  - the first bit will be a 0 followed by an error code
	//	- data will be 0xFF until response
	int i = 0;
	unsigned char response = 0;
	unsigned char rvalue = 0;
	unsigned int uiTimeout = 6500;

	//Get the result
	response = SD_Read_Byte();
	do {
		rvalue = SD_Read_Byte();
	}while (rvalue != 0xFF && i++ < uiTimeout);

	if(response != 0xE5) return SD_FAILED;
	else if (i >= uiTimeout ) return SD_FAILED;
	else return SD_SUCCESS;
}


////////////////////////////////////////////////////////////////////
//! \fn vSD_Card_GetR1()
//!	\brief Gets the R1 type response
//!
//! Bit		Value
//! 7 		0
//! 6 		parameter error
//! 5 		address error
//! 4 		erase sequence error
//! 3 		com crc error
//! 2 		illegal command
//! 1			erase reset
//! 0   	in idle state
//!
///////////////////////////////////////////////////////////////////
void vSD_Card_GetR1(void)
{
	unsigned int i;
	// Loop through and get the R1 response.  The card should transmit 0xFF until
	// the response.
	for (i = 0; i < 100; i++) {
		g_r1Response.uiRaw = SD_Read_Byte(); //This 'should' be the response
		if (g_r1Response.uiRaw & 0x80) continue;
		else break;
	}

}

////////////////////////////////////////////////////////////////////
//! \fn vSD_Card_GetR1b()
//!	\brief Gets the R1b type response
//!
//! The R1b type response is the R1 response followed by a busy
//! signal indicated by zeros.
//!
///////////////////////////////////////////////////////////////////
void vSD_Card_GetR1b(void)
{
	uint i;
	uchar ucResponse;

	// Loop through and get the R1 response.  The card should transmit 0xFF until
	// the response.
	for (i = 0; i < 28; i++) {
		g_r1Response.uiRaw = SD_Read_Byte(); //This 'should' be the response

		if (g_r1Response.uiRaw & 0x80) continue;
		else break;
	}

	// Request bytes until the busy signal (0x00) is gone
	for (i = 0; i < 28; i++) {
		ucResponse = SD_Read_Byte(); //This 'should' be the response

		if (ucResponse == 0x00) continue;
		else break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function verifies that there is an SD card present
//!
//! Checks to see if an SD card exists and stores pertinent information
//!
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
unsigned char ucSD_CheckForCard(void){

	if(ucSD_Init() == SD_FAILED)
		return SD_FAILED;

	if (ucSD_Send_Command(SD_CMD_SEND_CSD, 0) == SD_FAILED || g_r1Response.uiRaw > 0) {
		vSPI_Quit();
		return SD_FAILED;
	}

	if(ucSD_Send_Command(SD_CMD_APP_CMD, 0x00) == SD_FAILED || g_r1Response.uiRaw > 1) {
		vSPI_Quit();
		return SD_FAILED;
	}

	if(ucSD_Send_Command(SD_ACMD_SEND_SCR, 0x00) || g_r1Response.uiRaw > 1) {
		vSPI_Quit();
		return SD_FAILED;
	}

	vSERIAL_sout("\r\n---- SD Card ----\r\n", 21);
	vSERIAL_sout("\r\nType:         ", 16);

	switch (g_sdInfo.ucType){
		case SD_TYPE_SDSC:
			vSERIAL_sout("SDSC", 4);
		break;
		case SD_TYPE_SDHC:
			vSERIAL_sout("SDHC", 4);
		break;
		case SD_TYPE_SDXC:
			vSERIAL_sout("SDXC", 4);
		break;

		default:
			vSERIAL_sout("Unknown", 7);
		break;
	}
	vSERIAL_crlf();

	vSERIAL_sout("Size (KB): ", 11);
	vSERIAL_UI32out(((g_sdInfo.ulNumBlocks/1000)*SD_CARD_BLOCKLEN));
	vSERIAL_crlf();

	vSERIAL_sout("Version:    ", 12);
	vSERIAL_UI8out(g_sdInfo.ucVersion);
	vSERIAL_crlf();

	vSPI_Quit();

	return SD_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////
//! \brief This function writes a block
//!
//! In depth description
//!
//! \param *pucData_TXBuffer, unsigned long ulAddress
//! \return error code
/////////////////////////////////////////////////////////////////////////////////////
//OCR, CID, CSD, RCA, DSR, SCR
//unsigned char ucSD_Read_Register(unsigned char ucRegister, unsigned char *pucBuffer)
//{
//	int i;
//	unsigned char RegisterIndex;
//	unsigned char ucResp;
//	unsigned char ucRegCmd;
//	unsigned int uiRegLength;
//
//	switch(ucRegister)
//	{
//		case SD_REGISTER_OCR:
//			ucRegCmd = SD_CMD_READ_OCR;
//			uiRegLength = 4;
//			break;
//		case SD_REGISTER_CID:
//			ucRegCmd = SD_CMD_SEND_CID;
//			uiRegLength = 16;
//		break;
//		case SD_REGISTER_CSD:
//			ucRegCmd = SD_CMD_SEND_CSD;
//		break;
////		case SD_REGISTER_DSR: ucRegCmd = ; break;
////		case SD_REGISTER_SCR: ucRegCmd = SD_ACMD_SEND_SCR; break;
//		default:
//			return SD_FAILED;
//	}
//
//	// If it is application specific send the application specific indicator
//	if(ucRegCmd == SD_ACMD_SEND_SCR) ucSD_Send_Command(SD_CMD_APP_CMD, 0);
//
//	//Send command to read register
//	if(ucSD_Send_Command(ucRegCmd, 0) == SD_FAILED)
//		return SD_FAILED;
//
//	//Wait for start token 0xFE
//	i = 100;
//	SD_CS(LOW);
//	while(i-- > 0)
//	{
//		ucResp = SD_Read_Byte();
//		if((ucResp==0xFE)) break;
//	}
//	for(RegisterIndex=0; RegisterIndex < SD_REGISTER_LENGTH(ucRegCmd); RegisterIndex++)
//	{
//		pucBuffer[RegisterIndex] = SD_Read_Byte();
//	}
//
//	//Get 16-bit CRC
//	SD_Send_Dummy(0x02);
//	SD_CS(HIGH);
//	SD_Send_Dummy(0x01);
//
//	return SD_SUCCESS;
//}

