/*
 * SP_BSL.h
 *
 *  Created on: Aug 15, 2016
 *      Author: cp397
 */

#ifndef SP_BSL_H_
#define SP_BSL_H_


#define ERRORCODE		0xA0
#define SUCCESSCODE		0X90
#define HEADER			0x80
#define DONTCARE		0xFF
#define	PWRDSIZE		0x20
#define BSLBAUD9600		0x00
#define BSLBAUD19200	0x01
#define BSLBAUD38400	0x02
#define LOADPCADDRHI	0xFF
#define LOADPCADDRLO	0xFE

//******************  BSL Command Set  *************************************//
//! @name BSL Command Set
//! These values are commands for the SP's BSL
//! @{
//! \def CMD_SND_DATA_BLOCK
//! \brief Informs the BSL that this message contains code to be written to flash
#define CMD_SND_DATA_BLOCK	0x12
//! \def CMD_SND_PASSWRD
//! \brief Informs the BSL that this message contains the password to unlock protected commands
#define CMD_SND_PASSWRD		0x10
//! \def CMD_ERASE_SEG
//! \brief Informs the BSL that this message contains parameters for erasing a segment of flash
#define CMD_ERASE_SEG		0x16
//! \def CMD_MASS_ERASE
//! \brief BSL command that erases all of flash on the target
#define CMD_MASS_ERASE		0x18
//! \def CMD_ERASE_CHK
//! \brief Command to verify the erasure of flash
#define CMD_ERASE_CHK		0x1C
//! \def CMD_CHG_BAUD
//! \brief Command to change the baud rate between programmer and target
#define CMD_CHG_BAUD		0x20
//! \def CMD_SET_OFFSET
//! \brief Command to set an offset for the memory pointer
#define CMD_SET_OFFSET		0x21
//! \def CMD_LOAD_PC
//! \brief Command to load the targets program counter
#define CMD_LOAD_PC			0x1A
//! \def CMD_GET_DATA
//! \brief Command to read data from the targets flash
#define CMD_GET_DATA		0x14
//! \def CMD_GET_VERSION
//! \brief command to get the BSL version from the target
#define CMD_GET_VERSION		0x1E
//! @}

struct BSL_Command_Packet
{
	uint8 ucHDR;
	uint8 ucCMD;
	uint8 ucL1;
	uint8 ucL2;
	uint8 ucAL;
	uint8 ucAH;
	uint8 ucLL;
	uint8 ucLH;
};


//! \brief Structure of a Command Header Packet union
//!
//! This union of a structure and an array allows the byte fields of the structure
//! to be addressed like an array

union BSL_Header
{
   uint8 ucByteArray[8];
   struct BSL_Command_Packet fields;
};

uint8 ucSP_BSL_Init(uint8 ucSPNumber);
uint8 ucSP_BSL_Send_Sync(void);
uint8 ucSP_BSL_Send_PW();
void vSP_BSL_LoadPwrd(uint8 ucSPnumber);
uint16 ucSP_BSL_Get_Version();
uint8 ucSP_BSL_Change_Baud(uint8);
uint8 ucSP_BSL_Send_Data(uint8 *, uint16 uiAddress, uint8 ucLength);
uint8 ucSP_BSL_Receive_Data(uint8 *pucBSL_Data, uint16 uiAddress , uint8 ucLength);
uint8 ucSP_BSL_Erase_Seg(uint16 uiAddress);
uint8 ucSP_BSL_Erase_Multiple_Seg(uint16 uiStartAddress, uint32 ulEndAddress, uint);
int16 uiSP_BSL_ComputeChecksum(uint8 * ucFlashData, uint8 ucLength);
int16 uiSP_BSL_ComputeHeaderChecksum(union BSL_Header *ucHeader, uint8 ucLength);
uint8 ucSP_BSL_LoadPC();
void vSP_BSL_CreateHeaders(void);
uint8 ucSP_BSL_MassErase(void);

// Todo remove when no longer needed
void vSP_BSL_LoadPwrd_Old(uint8 ucSPnumber);

#endif /* SP_BSL_H_ */
