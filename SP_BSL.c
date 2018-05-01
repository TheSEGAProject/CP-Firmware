/**
 * \file SP_BSL.c
 * \brief Driver for the SP bootstrap loader
 * This file enables reporgramming of the SP boards via the CP
 */

//////////////////////////////////////////////////////////////
//! \file SP_BSL.c
//! \brief This file handles Over the Air programming of SP boards
//!
//! @author Christopher Porter, Nicholas Rowe
//! @date 9/14/2012
//!
//////////////////////////////////////////////////////////////

#include <msp430.h>
#include "std.h"
#include "SP_UART.h"
#include "SP_BSL.h"
#include "SP.h"
#include "delay.h"
#include <stdio.h>

#define DEMO

union BSL_Header gSP_BSL_PWHeader;
union BSL_Header gSP_BSL_VersionHeader;
union BSL_Header gSP_BSL_LoadPCHeader;
union BSL_Header gSP_BSL_SendDataHeader;
union BSL_Header gSP_BSL_ChangeBaudHeader;
union BSL_Header gSP_BSL_EraseSegHeader;
union BSL_Header gSP_BSL_EraseMainHeader;
union BSL_Header gSP_BSL_ReceiveDataHeader;
union BSL_Header gSP_BSL_MassEraseHeader;

uint8 g_ucaPWDBuffer[PWRDSIZE];
uint8 g_ucaTempSPCode[72];
extern volatile uint8 g_ucaSP_UART_RXBuffer[RX_BUFFER_SIZE];
extern uint8 g_ucaSP_RXBuffer[RX_BUFFER_SIZE];

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls Reset pin on SP1 Low
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP1_ResetLow(void)
{
	P_SP1_OUT &= ~SP1_RST_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls Reset pin on SP2 Low
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP2_ResetLow(void)
{
	P_SP2_OUT &= ~SP2_RST_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls Reset pin on SP3 Low
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP3_ResetLow(void)
{
	P_SP3_OUT &= ~SP3_RST_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls Reset pin on SP4 Low
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP4_ResetLow(void)
{
	P_SP4_OUT &= ~SP4_RST_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls Reset pin on SP1 High
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP1_ResetHigh(void)
{
	P_SP1_OUT |= SP1_RST_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls Reset pin on SP2 High
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP2_ResetHigh(void)
{
	P_SP2_OUT |= SP2_RST_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls Reset pin on SP3 High
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP3_ResetHigh(void)
{
	P_SP3_OUT |= SP3_RST_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls Reset pin on SP4 High
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP4_ResetHigh(void)
{
	P_SP4_OUT |= SP4_RST_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls TCK pin on SP1 Low
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP1_TCKLow(void)
{
	P_SP1_OUT &= ~SP1_TCK_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls TCK pin on SP2 Low
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP2_TCKLow(void)
{
	P_SP2_OUT &= ~SP2_TCK_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls TCK pin on SP3 Low
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP3_TCKLow(void)
{
	P_SP3_OUT &= ~SP3_TCK_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls TCK pin on SP4 Low
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP4_TCKLow(void)
{
	P_SP4_OUT &= ~SP4_TCK_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls TCK pin on SP1 High
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP1_TCKHigh(void)
{
	P_SP1_OUT |= SP1_TCK_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls TCK pin on SP2 High
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP2_TCKHigh(void)
{
	P_SP2_OUT |= SP2_TCK_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls TCK pin on SP3 High
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP3_TCKHigh(void)
{
	P_SP3_OUT |= SP3_TCK_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Pulls TCK pin on SP4 High
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
static inline void vSP4_TCKHigh(void)
{
	P_SP4_OUT |= SP4_TCK_BIT;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Toggles the Reset and TCK pins on the SP boards to initialize the BSL
//!
//!
//! @param SPNumber
//! @return 1
////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_Init(uint8 ucSPNumber)
{
	__bic_SR_register(GIE);

	switch (ucSPNumber)
	{
		case SP1:
			vSP1_TCKHigh();
			__delay_cycles(50);
			vSP1_ResetLow();
			__delay_cycles(50);
			vSP1_TCKLow();
			__delay_cycles(50);
			vSP1_TCKHigh();
			__delay_cycles(50);
			vSP1_TCKLow();
			__delay_cycles(50);
			vSP1_ResetHigh();
			__delay_cycles(50);

		break;

		case SP2:
			vSP2_ResetLow();
			vSP2_TCKHigh();
			vSP2_TCKLow();
			vSP2_TCKHigh();
			vSP2_TCKLow();
			vSP2_ResetHigh();
		break;

		case SP3:
			vSP3_ResetLow();
			vSP3_TCKHigh();
			vSP3_TCKLow();
			vSP3_TCKHigh();
			vSP3_TCKLow();
			vSP3_ResetHigh();
		break;

		case SP4:
			vSP4_ResetLow();
			vSP4_TCKHigh();
			vSP4_TCKLow();
			vSP4_TCKHigh();
			vSP4_TCKLow();
			vSP4_ResetHigh();
		break;

		default:
		break;
	}
	__bis_SR_register(GIE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Sends the Synch(0x80) to the SP board
//!
//! The BSL uses this to synchronize UART communications
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
//send the password to the BSL
uint8 ucSP_BSL_Send_Sync(void)
{
	uint8 ucReturnVal;

	// Reset the receive buffer index
	vSP_UART_ResetRXBufferIndex();

	//send sync and wait for response
	vSP_UART_TXByte(0x80);
	ucSP_UART_RXByte();

	ucReturnVal = g_ucaSP_UART_RXBuffer[0x00]; //Read error code

	g_ucaSP_UART_RXBuffer[0x00] = 0x00; //Clear the error code for the next use

	return ucReturnVal;
} //END: ucSP_BSL_Send_Sync(void)

/////////////////////////////////////////////////////////////////////////////
//! \brief Sends the Password to the SP board
//!
//!
//! @param none
//! @return none.
////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_Send_PW(void)
{
	uint16 counter = 0;
	uint16 uiHeaderChkSum = 0;
	uint16 uiDataChkSum = 0;
	uint16 uiCheckSum;
	uint8 ucReturnVal;

	//Compute the checksum for the header and the data blocks then
	//put the two together in a final checksum
	uiHeaderChkSum = uiSP_BSL_ComputeHeaderChecksum(&gSP_BSL_PWHeader, 8);
	uiDataChkSum = uiSP_BSL_ComputeChecksum(g_ucaPWDBuffer, PWRDSIZE);
	uiCheckSum = ~(uiHeaderChkSum ^ uiDataChkSum);

	//Send the synchronization byte required before every command
	ucReturnVal = ucSP_BSL_Send_Sync();
	if (ucReturnVal != SUCCESSCODE) {
		//If synch fails then exit and return error
		return ucReturnVal;
	}

	//Send PW_Header
	for (counter = 0; counter < 8; counter++) {
		vSP_UART_TXByte(gSP_BSL_PWHeader.ucByteArray[counter]);
		vDELAY_LPMWait1us(20, 0);
	}
	//Send PW
	for (counter = 0; counter < PWRDSIZE; counter++) {
		vSP_UART_TXByte(g_ucaPWDBuffer[counter]);
		vDELAY_LPMWait1us(20, 0);
	}

	//Send the checksum
	vSP_UART_TXByte((uint8) (uiCheckSum));
	vDELAY_LPMWait1us(20, 0);

	vSP_UART_TXByte((uint8) (uiCheckSum >> 8));

	// Reset buffer index and receive byte
	vSP_UART_ResetRXBufferIndex();
	ucSP_UART_RXByte();

	ucReturnVal = g_ucaSP_UART_RXBuffer[0x00]; //Read error code

	g_ucaSP_UART_RXBuffer[0x00] = 0x00; //Clear the error code for the next use

	return ucReturnVal;
} //END: ucSP_BSL_Send_PW()

/////////////////////////////////////////////////////////////////////////////
//! \brief Erases the main memory
//!
//!
//! @param none
//! @return error code
////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_Erase_Main(void)
{
	uint16 counter;
	uint16 counter2;
	uint16 uiHeaderChkSum = 0;
	uint8 ucReturnVal;

	uiHeaderChkSum = uiSP_BSL_ComputeHeaderChecksum(&gSP_BSL_EraseMainHeader, 8);

	for (counter2 = 0; counter2 < 15; counter2++) {
		//Send the synchronization byte required before every command
		ucReturnVal = ucSP_BSL_Send_Sync();
		if (ucReturnVal != SUCCESSCODE) {
			//If synch fails then exit and return error
			return ucReturnVal;
		}

		//Send PW_Header
		for (counter = 0; counter < 8; counter++) {
			vSP_UART_TXByte(gSP_BSL_EraseMainHeader.ucByteArray[counter]);
			vDELAY_LPMWait1us(2000, 0);
		}

		vSP_UART_TXByte((uint8) (uiHeaderChkSum));
		vDELAY_LPMWait1us(2000, 0);

		vSP_UART_TXByte((uint8) (uiHeaderChkSum >> 8));
		vDELAY_LPMWait1us(15000, 0);
	}

	//The timing for this has not yet been critically evaluated
	vDELAY_LPMWait1us(15000, 0);
	vDELAY_LPMWait1us(15000, 0);
	vDELAY_LPMWait1us(15000, 0);
	vDELAY_LPMWait1us(15000, 0);
	vDELAY_LPMWait1us(15000, 0);

	// Reset buffer index and receive byte
	vSP_UART_ResetRXBufferIndex();
	ucSP_UART_RXByte();

	ucReturnVal = g_ucaSP_UART_RXBuffer[0x00]; //Read error code

	g_ucaSP_UART_RXBuffer[0x00] = 0x00; //Clear the error code for the next use

	return ucReturnVal;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Erases a segment in memory starting at uiAddress and of length 0x100
//!
//!
//! @param none
//! @return error code
////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_Erase_Seg(uint16 uiAddress)
{
	uint8 ucCounter;
	uint8 ucReturnVal;
	uint16 uiHeaderChkSum = 0;

	//Move the starting address into the EraseSeg header
	gSP_BSL_EraseSegHeader.fields.ucAL = (uint8) uiAddress;
	gSP_BSL_EraseSegHeader.fields.ucAH = (uint8) (uiAddress >> 8);

	//Compute the checksum for the header
	uiHeaderChkSum = uiSP_BSL_ComputeHeaderChecksum(&gSP_BSL_EraseSegHeader, 8);

	//Send the synchronization byte required before every command
	ucReturnVal = ucSP_BSL_Send_Sync();

	//If synch fails then exit and return error
	if (ucReturnVal != SUCCESSCODE)
		return ucReturnVal;

	//Send EraseSeg Header
	for (ucCounter = 0; ucCounter < 8; ucCounter++) {
		vSP_UART_TXByte(gSP_BSL_EraseSegHeader.ucByteArray[ucCounter]);
	}

	//Send the checksum
	vSP_UART_TXByte((uint8) (uiHeaderChkSum));

	vSP_UART_TXByte((uint8) (uiHeaderChkSum >> 8));

	// Reset buffer index and receive byte
	vSP_UART_ResetRXBufferIndex();
	ucSP_UART_RXByte();

	ucReturnVal = g_ucaSP_UART_RXBuffer[0x00];

	g_ucaSP_UART_RXBuffer[0x00] = 0x00;

	return ucReturnVal;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Erases a segment in memory starting at uiAddress and of length 0x100
//!
//!
//! @param none
//! @return error code
////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_MassErase(void)
{
	uint8 ucCounter;
	uint8 ucReturnVal;
	uint16 uiHeaderChkSum = 0;
	uint16 uiAttemptCount;

	//Compute the checksum for the header
	uiHeaderChkSum = uiSP_BSL_ComputeHeaderChecksum(&gSP_BSL_MassEraseHeader, 8);

	uiAttemptCount = 50;
	while(uiAttemptCount != 0){
	//Send the synchronization byte required before every command
	ucReturnVal = ucSP_BSL_Send_Sync();

	//If synch fails then exit and return error
	if (ucReturnVal != SUCCESSCODE)
		return ucReturnVal;

	//Send EraseSeg Header
	for (ucCounter = 0; ucCounter < 8; ucCounter++)
		vSP_UART_TXByte(gSP_BSL_MassEraseHeader.ucByteArray[ucCounter]);

	//Send the checksum
	vSP_UART_TXByte((uint8) (uiHeaderChkSum));
	vSP_UART_TXByte((uint8) (uiHeaderChkSum >> 8));

	// Reset buffer index and receive byte
	vSP_UART_ResetRXBufferIndex();
	ucSP_UART_RXByte();

	ucReturnVal = g_ucaSP_UART_RXBuffer[0x00];

	g_ucaSP_UART_RXBuffer[0x00] = 0x00;

	if(ucReturnVal == SUCCESSCODE)
		break;

	}
	return ucReturnVal;
}

/////////////////////////////////////////////////////////////////////////////
//! \brief Erases several segments in memory
//!
//! Segmentation in the MSP430F235 is as follows
//! SegmentA to Segment D is information memory and should be protected
//! Segment 0 to Segment n is from 0xC000 to 0xFEFF each segment is 0x100 bytes long
//!
//! @param uiStartAddress, uiEndAddress
//! @return ucReturnVal
////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_Erase_Multiple_Seg(uint16 uiStartAddress, uint32 ulEndAddress, uint uiDelay)
{
	uint32 ulAddressCounter;
	uint8 ucReturnVal;
	uint uiAttemptCount;

	//Loop calls the function to erase a single segment multiple times
	for (ulAddressCounter = uiStartAddress; ulAddressCounter <= ulEndAddress; ulAddressCounter += 0x100) {

		uiAttemptCount = 200;
		while (uiAttemptCount-- != 0) {
			vDELAY_LPMWait1us(uiDelay, 0);
			ucReturnVal = ucSP_BSL_Erase_Seg((uint16)ulAddressCounter);
			if(ucReturnVal == SUCCESSCODE)
				break;
		}
		//If the erasure of a segment fails then exit the function and return the error code
		if (ucReturnVal != SUCCESSCODE){
			printf("Erase failed at address %08x\r\n", ulAddressCounter);
			break;
		}
	}

	return ucReturnVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief This function loads the BSL password into a buffer for future use
//!
//! This function sends a request to the target for the BSL password and stores it
//! If the target does not respond assume the password is 0xFF
//!   \param ucSPnumber
//!   \sa SP_BSL.h
///////////////////////////////////////////////////////////////////////////////
void vSP_BSL_LoadPwrd(uint8 ucSPnumber)
{

	uint8 ucPwrdLoopCntr;

	if (!(ucSP_RequestBSLPassword(ucSPnumber))) {
		for (ucPwrdLoopCntr = 0x04; ucPwrdLoopCntr < PWRDSIZE + 4; ucPwrdLoopCntr++)
			g_ucaPWDBuffer[(uint8) (ucPwrdLoopCntr - 4)] = g_ucaSP_RXBuffer[ucPwrdLoopCntr];
	}
	else {
		for (ucPwrdLoopCntr = 0; ucPwrdLoopCntr < PWRDSIZE; ucPwrdLoopCntr++)
			g_ucaPWDBuffer[ucPwrdLoopCntr] = 0xFF;
	}

} //END: ucSP_BSLLoadPwrd()

///////////////////////////////////////////////////////////////////////////////
//! \brief This function loads the BSL password into a buffer for future use
//!
//! This function sends a request to the target for the BSL password and stores it
//! If the target does not respond assume the password is 0xFF
//!   \param ucSPnumber
//!   \sa SP_BSL.h
///////////////////////////////////////////////////////////////////////////////
void vSP_BSL_LoadPwrd_Old(uint8 ucSPnumber)
{

	uint8 ucPwrdLoopCntr;

	if (!(ucSP_RequestBSLPassword_Old(ucSPnumber))) {
		for (ucPwrdLoopCntr = 0x03; ucPwrdLoopCntr < PWRDSIZE + 3; ucPwrdLoopCntr++)
			g_ucaPWDBuffer[(uint8) (ucPwrdLoopCntr - 3)] = g_ucaSP_RXBuffer[ucPwrdLoopCntr];
	}
	else {
		for (ucPwrdLoopCntr = 0; ucPwrdLoopCntr < PWRDSIZE; ucPwrdLoopCntr++)
			g_ucaPWDBuffer[ucPwrdLoopCntr] = 0xFF;
	}

} //END: ucSP_BSLLoadPwrd()

///////////////////////////////////////////////////////////////////////////////
//! \brief This function requests the BSL version
//!
//! This function reads the BSL version from the location 0xFFA in the F235's
//! flash memory.
//!   \param Address of where the main BSL code wants to store the version at
//!   \return an error code generated by the BSL
//!   \sa SP_BSL.h
///////////////////////////////////////////////////////////////////////////////
uint16 ucSP_BSL_Get_Version(void) //Include a pass by reference variable to store version in
{
	uint8 ucCounter;
	uint16 uiReturnVal;

	//Send the synchronization byte required before every command
	uiReturnVal = ucSP_BSL_Send_Sync();
	if (uiReturnVal != SUCCESSCODE) {
		//If synch fails then exit and return error
		return uiReturnVal;
	}

	//Send Version_Header
	for (ucCounter = 0; ucCounter < 8; ucCounter++) {
		vSP_UART_TXByte(gSP_BSL_VersionHeader.ucByteArray[ucCounter]);
		vDELAY_LPMWait1us(2000, 0);
	}

	vSP_UART_TXByte(0x83);
	vDELAY_LPMWait1us(2000, 0);
	vSP_UART_TXByte(0xE0);
	vDELAY_LPMWait1us(15000, 0);
	vDELAY_LPMWait1us(15000, 0);
	vDELAY_LPMWait1us(15000, 0);

	uiReturnVal = g_ucaSP_UART_RXBuffer[4];
	uiReturnVal = uiReturnVal << 8;
	uiReturnVal = g_ucaSP_UART_RXBuffer[5];

	return uiReturnVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief This function changes the baud rate of BSL communication
//!
//! This function requests a change in the BSL communication speed
//! Initial communication must be at 9600, until changed by this function
//!   \param the baud rate you want to change to
//!   \return an error code generated by the BSL
//!   \sa SP_BSL.h
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_Change_Baud(uint8 ucBSLBaud)
{
	uint8 counter;
	uint16 uiHeaderChkSum;
	uint8 ucReturnVal;
	switch (ucBSLBaud)
	{
		case BSLBAUD9600:

			gSP_BSL_ChangeBaudHeader.fields.ucAL = 0x80;
			gSP_BSL_ChangeBaudHeader.fields.ucAH = 0x85;
			gSP_BSL_ChangeBaudHeader.fields.ucLL = 0x00;
		break;

		case BSLBAUD19200:

			gSP_BSL_ChangeBaudHeader.fields.ucAL = 0x00;
			gSP_BSL_ChangeBaudHeader.fields.ucAH = 0x8B;
			gSP_BSL_ChangeBaudHeader.fields.ucLL = 0x01;
		break;

		case BSLBAUD38400:
			gSP_BSL_ChangeBaudHeader.fields.ucAL = 0x80;
			gSP_BSL_ChangeBaudHeader.fields.ucAH = 0x8C;
			gSP_BSL_ChangeBaudHeader.fields.ucLL = 0x02;
		break;

		default:
		break;
	}

	//Compute the checksum from the Change Baud header
	uiHeaderChkSum = uiSP_BSL_ComputeHeaderChecksum(&gSP_BSL_ChangeBaudHeader, 8);

	//Send the synchronization byte required before every command
	ucReturnVal = ucSP_BSL_Send_Sync();
	if (ucReturnVal != SUCCESSCODE) {
		//If synch fails then exit and return error
		return ucReturnVal;
	}

	//Send ChangeBaudHeader
	for (counter = 0; counter < 8; counter++) {
		vSP_UART_TXByte(gSP_BSL_ChangeBaudHeader.ucByteArray[counter]);
		vDELAY_LPMWait1us(2000, 0);
	}

	//send Checksum
	vSP_UART_TXByte((uint8) (uiHeaderChkSum));
	vDELAY_LPMWait1us(2000, 0);
	vSP_UART_TXByte((uint8) (uiHeaderChkSum >> 8));
	vDELAY_LPMWait1us(4000, 0);

	//must wait about 10ms before transmitting data after the change in baud
	vDELAY_LPMWait1us(10000, 0);

	// Reset buffer index and receive byte
	vSP_UART_ResetRXBufferIndex();
	ucSP_UART_RXByte();

	ucReturnVal = g_ucaSP_UART_RXBuffer[0x00];

	g_ucaSP_UART_RXBuffer[0x00] = 0x00;

	return ucReturnVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief This function sends the reprogramming data to the BSL
//!
//! This function takes your code block in an array and sends it over
//! the BSL to use for reprogramming
//!   \param an unsigned character array which contains your new data
//!   \return an error code generated by the BSL
//!   \sa SP_BSL.h
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_Send_Data(uint8 *pucBSL_Data, uint16 uiAddress, uint8 ucLength)
{
	uint8 ucReturnVal;
	int16 counter = 0;
	uint16 uiHeaderChkSum = 0;
	uint16 uiDataChkSum = 0;
	uint16 uiCheckSum = 0;

	gSP_BSL_SendDataHeader.fields.ucHDR = HEADER;
	gSP_BSL_SendDataHeader.fields.ucCMD = CMD_SND_DATA_BLOCK;
	gSP_BSL_SendDataHeader.fields.ucL1 = ucLength + 4;
	gSP_BSL_SendDataHeader.fields.ucL2 = ucLength + 4;
	gSP_BSL_SendDataHeader.fields.ucAL = (char) uiAddress; //Low part of the start address
	gSP_BSL_SendDataHeader.fields.ucAH = (char) (uiAddress >> 8); //High part of the start address
	gSP_BSL_SendDataHeader.fields.ucLL = ucLength; //depends on data length
	gSP_BSL_SendDataHeader.fields.ucLH = 0x00; //always 0

	//Compute the checksum for the header and the data blocks then
	//put the two together in a final checksum
	uiHeaderChkSum = uiSP_BSL_ComputeHeaderChecksum(&gSP_BSL_SendDataHeader, 8);
	uiDataChkSum = uiSP_BSL_ComputeChecksum(pucBSL_Data, ucLength);
	uiCheckSum = ~(uiHeaderChkSum ^ uiDataChkSum);

	//Send the synchronization byte required before every command
	ucReturnVal = ucSP_BSL_Send_Sync();
	if (ucReturnVal != SUCCESSCODE) {
		//If synch fails then exit and return error
		return ucReturnVal;
	}
	//send header packet
	for (counter = 0; counter < 8; counter++) {
		vSP_UART_TXByte(gSP_BSL_SendDataHeader.ucByteArray[counter]);
	}

	// Send the code
	counter = 0;
	while (counter < ucLength) {
		vSP_UART_TXByte(pucBSL_Data[counter]);
		counter++;
	}

	//Send the checksum
	vSP_UART_TXByte((uint8) (uiCheckSum));
	vSP_UART_TXByte((uint8) (uiCheckSum >> 8));

	// Reset buffer index and receive byte
	vSP_UART_ResetRXBufferIndex();
	ucSP_UART_RXByte();

	ucReturnVal = g_ucaSP_UART_RXBuffer[0x00];

	g_ucaSP_UART_RXBuffer[0x00] = 0x00;

	return ucReturnVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief This function sends the reprogramming data to the BSL
//!
//! This function takes your code block in an array and sends it over
//! the BSL to use for reprogramming
//!   \param an unsigned character array which contains your new data
//!   \return an error code generated by the BSL
//!   \sa SP_BSL.h
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_Receive_Data(uint8 *pucBSL_Data, uint16 uiAddress, uint8 ucLength)
{

	int16 counter;
	uint16 uiHeaderChkSum = 0;
	uint8 ucReturnVal;
	counter = 0;
	gSP_BSL_ReceiveDataHeader.fields.ucAL = (char) uiAddress; //Low part of the start address
	gSP_BSL_ReceiveDataHeader.fields.ucAH = (char) (uiAddress >> 8); //High part of the start address
	gSP_BSL_ReceiveDataHeader.fields.ucLL = ucLength; //depends on data length

	uiHeaderChkSum = uiSP_BSL_ComputeHeaderChecksum(&gSP_BSL_ReceiveDataHeader, 8);

	//Send the synchronization byte required before every command
	ucReturnVal = ucSP_BSL_Send_Sync();
	if (ucReturnVal != SUCCESSCODE) {
		//If synch fails then exit and return error
		return ucReturnVal;
	}

	//send header packet
	for (counter = 0; counter < 8; counter++) {
		vSP_UART_TXByte(gSP_BSL_ReceiveDataHeader.ucByteArray[counter]);
		vDELAY_LPMWait1us(2000, 0);
	}

	vSP_UART_TXByte((uint8) (uiHeaderChkSum));
	vDELAY_LPMWait1us(2000, 0);

	vSP_UART_TXByte((uint8) (uiHeaderChkSum >> 8));
	vDELAY_LPMWait1us(15000, 0);
	for (counter = 0; counter < ucLength + 6; counter++) {
		vDELAY_LPMWait1us(15000, 0);
	}

	//Transfer the received data to the chosen array
	for (counter = 0x00; counter < ucLength + 6; counter++) {
		*pucBSL_Data = g_ucaSP_UART_RXBuffer[counter];
		pucBSL_Data++;
	}

	if (g_ucaSP_UART_RXBuffer[0x00] == ERRORCODE) {
		ucReturnVal = g_ucaSP_UART_RXBuffer[0x00];
	}

	g_ucaSP_UART_RXBuffer[0x00] = 0x00;

	return ucReturnVal;
} //END:ucSP_BSL_Receive_Data()

///////////////////////////////////////////////////////////////////////////////
//! \brief This function sets the program counter of the reprogrammed SP
//!
//! This function send the request to the BSL to load the program counter
//! to the starting location, determined by where the first piece of
//! was written to
//!   \param void
//!   \return void
//!   \sa SP_BSL.h
///////////////////////////////////////////////////////////////////////////////
uint8 ucSP_BSL_LoadPC()
{
	//send the load program counter to starting address command
	int16 counter = 0;
	uint16 uiCheckSum = 0;
	uint8 ucReturnVal;

	//Send the synchronization byte required before every command
	ucReturnVal = ucSP_BSL_Send_Sync();
	if (ucReturnVal != SUCCESSCODE) {
		//If synch fails then exit and return error
		return ucReturnVal;
	}
	//Send Load PC Header
	for (counter = 0; counter < 8; counter++) {
		vSP_UART_TXByte(gSP_BSL_LoadPCHeader.ucByteArray[counter]);
	}

	//Compute the header checksum
	uiCheckSum = uiSP_BSL_ComputeHeaderChecksum(&gSP_BSL_LoadPCHeader, 8);

	//Send the checksum
	vSP_UART_TXByte((uint8) (uiCheckSum));
	vDELAY_LPMWait1us(2000, 0);

	vSP_UART_TXByte((uint8) (uiCheckSum >> 8));
	vDELAY_LPMWait1us(4000, 0);

	ucReturnVal = g_ucaSP_UART_RXBuffer[0x00];

	g_ucaSP_UART_RXBuffer[0x00] = 0x00;

	return ucReturnVal;
}

int16 uiSP_BSL_ComputeChecksum(uint8 *ucFlashData, uint8 ucLength)
{
	uint8 ucCounter;
	uint8 uiCkl;
	uint8 uiCkh;
	uint16 uiChecksumValue = 0;

	uiCkl = 0x00;
	uiCkh = 0x00;

	for (ucCounter = 0; ucCounter < ucLength; ucCounter++) {
		if (ucCounter % 2 == 0) {
			uiCkl ^= ucFlashData[ucCounter];
		}
		else {
			uiCkh ^= ucFlashData[ucCounter];
		}
	}

	uiCkl = ~uiCkl;
	uiCkh = ~uiCkh;
	uiChecksumValue |= uiCkh;
	uiChecksumValue <<= 8;
	uiChecksumValue |= uiCkl;
	return uiChecksumValue; //maybe we can just return ckh and ckl since they are what we need to send anyway, and we have a send bytes function.

}

int16 uiSP_BSL_ComputeHeaderChecksum(union BSL_Header * ucHeader, uint8 ucLength)
{
	uint8 ucCounter;
	uint8 uiCkl;
	uint8 uiCkh;
	uint16 uiChecksumValue = 0;

	uiCkl = 0x00;
	uiCkh = 0x00;

	for (ucCounter = 0; ucCounter < ucLength; ucCounter++) {
		if (ucCounter % 2 == 0) {
			uiCkl ^= ucHeader->ucByteArray[ucCounter];
		}
		else {
			uiCkh ^= ucHeader->ucByteArray[ucCounter];
		}
	}
	uiCkl = ~uiCkl;
	uiCkh = ~uiCkh;
	uiChecksumValue |= uiCkh;
	uiChecksumValue <<= 8;
	uiChecksumValue |= uiCkl;
	return uiChecksumValue; //maybe we can just return ckh and ckl since they are what we need to send anyway, and we have a send bytes function.
}

void vSP_BSL_CreateHeaders(void)
{
	gSP_BSL_VersionHeader.fields.ucHDR = HEADER;
	gSP_BSL_VersionHeader.fields.ucCMD = CMD_GET_DATA; //The TX_BSL_VERSION command does not work on the 2xx version due to some size restriction
	gSP_BSL_VersionHeader.fields.ucL1 = 0x04;
	gSP_BSL_VersionHeader.fields.ucL2 = 0x04; //fixed data size = 4 bytes
	gSP_BSL_VersionHeader.fields.ucAL = 0xFA; //following two bytes are the address of the BSL version memory
	gSP_BSL_VersionHeader.fields.ucAH = 0x0F;
	gSP_BSL_VersionHeader.fields.ucLL = 0x02; //Length of read
	gSP_BSL_VersionHeader.fields.ucLH = 0x00; //Always zero

	gSP_BSL_PWHeader.fields.ucHDR = HEADER;
	gSP_BSL_PWHeader.fields.ucCMD = CMD_SND_PASSWRD;
	gSP_BSL_PWHeader.fields.ucL1 = 0x24;
	gSP_BSL_PWHeader.fields.ucL2 = 0x24; //fixed data size = 24 bytes
	gSP_BSL_PWHeader.fields.ucAL = DONTCARE; //Don't Care
	gSP_BSL_PWHeader.fields.ucAH = DONTCARE; //Don't Care
	gSP_BSL_PWHeader.fields.ucLL = DONTCARE; //Don't Care
	gSP_BSL_PWHeader.fields.ucLH = DONTCARE; //Don't Care

	gSP_BSL_LoadPCHeader.fields.ucHDR = HEADER;
	gSP_BSL_LoadPCHeader.fields.ucCMD = CMD_LOAD_PC;
	gSP_BSL_LoadPCHeader.fields.ucL1 = 0x04;
	gSP_BSL_LoadPCHeader.fields.ucL2 = 0x04;
	gSP_BSL_LoadPCHeader.fields.ucAL = LOADPCADDRLO; //change to start address where we started writing
	gSP_BSL_LoadPCHeader.fields.ucAH = LOADPCADDRHI; //change to start address where we started writing
	gSP_BSL_LoadPCHeader.fields.ucLL = DONTCARE; //Don't Care
	gSP_BSL_LoadPCHeader.fields.ucLH = DONTCARE; //Don't Care

	gSP_BSL_SendDataHeader.fields.ucHDR = HEADER;
	gSP_BSL_SendDataHeader.fields.ucCMD = CMD_SND_DATA_BLOCK;
	gSP_BSL_SendDataHeader.fields.ucL1 = HEADER;
	gSP_BSL_SendDataHeader.fields.ucL2 = HEADER;
	gSP_BSL_SendDataHeader.fields.ucAL = 0xFF; //part of the start address
	gSP_BSL_SendDataHeader.fields.ucAH = 0xFF; //part of the start address
	gSP_BSL_SendDataHeader.fields.ucLL = 0xFF; //depends on data length
	gSP_BSL_SendDataHeader.fields.ucLH = 0x00; //always 0

	gSP_BSL_ReceiveDataHeader.fields.ucHDR = HEADER;
	gSP_BSL_ReceiveDataHeader.fields.ucCMD = CMD_GET_DATA;
	gSP_BSL_ReceiveDataHeader.fields.ucL1 = 0x04;
	gSP_BSL_ReceiveDataHeader.fields.ucL2 = 0x04;
	gSP_BSL_ReceiveDataHeader.fields.ucAL = DONTCARE;
	gSP_BSL_ReceiveDataHeader.fields.ucAH = DONTCARE;
	gSP_BSL_ReceiveDataHeader.fields.ucLL = DONTCARE;
	gSP_BSL_ReceiveDataHeader.fields.ucLH = 0x00;

	gSP_BSL_ChangeBaudHeader.fields.ucHDR = HEADER;
	gSP_BSL_ChangeBaudHeader.fields.ucCMD = CMD_CHG_BAUD;
	gSP_BSL_ChangeBaudHeader.fields.ucL1 = 0x04;
	gSP_BSL_ChangeBaudHeader.fields.ucL2 = 0x04;
	gSP_BSL_ChangeBaudHeader.fields.ucAL = 0xFF;
	gSP_BSL_ChangeBaudHeader.fields.ucAH = 0xFF;
	gSP_BSL_ChangeBaudHeader.fields.ucLL = 0xFF;
	gSP_BSL_ChangeBaudHeader.fields.ucLH = DONTCARE; //Don't Care

	gSP_BSL_EraseSegHeader.fields.ucHDR = HEADER;
	gSP_BSL_EraseSegHeader.fields.ucCMD = CMD_ERASE_SEG;
	gSP_BSL_EraseSegHeader.fields.ucL1 = 0x04;
	gSP_BSL_EraseSegHeader.fields.ucL2 = 0x04;
	gSP_BSL_EraseSegHeader.fields.ucAL = 0xFF;
	gSP_BSL_EraseSegHeader.fields.ucAH = 0xFF;
	gSP_BSL_EraseSegHeader.fields.ucLL = 0x02;
	gSP_BSL_EraseSegHeader.fields.ucLH = 0xA5; //Don't Care

	gSP_BSL_EraseMainHeader.fields.ucHDR = HEADER;
	gSP_BSL_EraseMainHeader.fields.ucCMD = CMD_ERASE_SEG;
	gSP_BSL_EraseMainHeader.fields.ucL1 = 0x04;
	gSP_BSL_EraseMainHeader.fields.ucL2 = 0x04;
	gSP_BSL_EraseMainHeader.fields.ucAL = 0x00;
	gSP_BSL_EraseMainHeader.fields.ucAH = 0xC0;
	gSP_BSL_EraseMainHeader.fields.ucLL = 0x04;
	gSP_BSL_EraseMainHeader.fields.ucLH = 0xA5; //Don't Care

	gSP_BSL_MassEraseHeader.fields.ucHDR = HEADER;
	gSP_BSL_MassEraseHeader.fields.ucCMD = CMD_MASS_ERASE;
	gSP_BSL_MassEraseHeader.fields.ucL1 = 0x04;
	gSP_BSL_MassEraseHeader.fields.ucL2 = 0x04;
	gSP_BSL_MassEraseHeader.fields.ucAL = 0x00;
	gSP_BSL_MassEraseHeader.fields.ucAH = 0x00;
	gSP_BSL_MassEraseHeader.fields.ucLL = 0x06;
	gSP_BSL_MassEraseHeader.fields.ucLH = 0xA5; //Don't Care


}

