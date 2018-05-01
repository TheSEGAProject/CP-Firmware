#ifndef SD_CARD_H
#define SD_CARD_H
///////////////////////////////////////////////////////////////////////////////
//! \file SD_Card.h
//! \brief Header file for the \ref SD Card Test Module.
//!
//! This file provides all of the defines and function prototypes for the 
//! \ref SD Card Test Module.
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
typedef struct tagSD_R1
{
	union
	{
		uint8 uiRaw;
		struct
		{
			uint8 ucIdleState      : 1;
			uint8 ucEraseReset     : 1;
			uint8 ucIllegalCommand : 1;
			uint8 ucCRCError       : 1;
			uint8 ucEraseSeqError  : 1;
			uint8 ucAddressError   : 1;
			uint8 ucParameterError : 1;
			uint8 ucStartBit       : 1; //Always 0
		} flags;
	};
} SD_R1;

typedef struct tagSD_OCR
{
	union
	{
		uint32 ulRaw;
		uint8 ucBytes[4];
		struct
		{
			uint8 ucReserved_0      : 1;
			uint8 ucReserved_1      : 1;
			uint8 ucReserved_2      : 1;
			uint8 ucReserved_3      : 1;
			uint8 ucReserved_4      : 1;
			uint8 ucReserved_5      : 1;
			uint8 ucReserved_6      : 1;
			uint8 ucReserved_7      : 1;
			uint8 ucReserved_8      : 1;
			uint8 ucReserved_9      : 1;
			uint8 ucReserved_10     : 1;
			uint8 ucReserved_11     : 1;
			uint8 ucReserved_12     : 1;
			uint8 ucReserved_13     : 1;
			uint8 ucReserved_14     : 1;
			uint8 ucVDD27_28      	: 1; // Voltage range
			uint8 ucVDD28_29      	: 1; // ""
			uint8 ucVDD29_30      	: 1; // ""
			uint8 ucVDD30_31      	: 1; // ""
			uint8 ucVDD31_32      	: 1; // ""
			uint8 ucVDD32_33      	: 1; // ""
			uint8 ucVDD33_34      	: 1; // ""
			uint8 ucVDD34_35      	: 1; // ""
			uint8 ucVDD35_36      	: 1; // ""
			uint8 ucS18A						: 1; // If 1 then supports switching to 1.8V
			uint8 ucReserved_25     : 1;
			uint8 ucReserved_26     : 1;
			uint8 ucReserved_27     : 1;
			uint8 ucReserved_28     : 1;
			uint8 ucReserved_29     : 1;
			uint8 ucCCS 					 	: 1; // If 0 then SDSC else SDHC or SDXC
			uint8 ucPowerUpStatus   : 1; // If 1 then the power up procedure is complete
		} flags;
	};
} SD_OCR;

#define ucLockUnlockFailed ucWPEraseSkip
#define ucCSDOverwrite ucOutOfRange
typedef struct tagSD_R2
{
	union
	{
		uint8 uiRaw;
		struct
		{
			uint8 ucCardLocked          : 1;
			uint8 ucWPEraseSkip         : 1; //Write protect error OR Lock/Unlock Failed
			uint8 ucUnspecifiedError    : 1;
			uint8 ucCardControllerError : 1;
			uint8 ucCardECCFailed       : 1;
			uint8 ucWriteProtectError   : 1;
			uint8 ucEraseParameter      : 1;
			uint8 ucOutOfRange          : 1; //Out of Range error OR CSD overwrite error
		} flags;
	};
} SD_R2;

//typedef struct tagSD_CSD
//{
//	union
//	{
//		uint8 uiRaw;
//		struct
//		{
//			uint8 ucCardLocked          : 1;
//			uint8 ucWPEraseSkip         : 1; //Write protect error OR Lock/Unlock Failed
//			uint8 ucUnspecifiedError    : 1;
//			uint8 ucCardControllerError : 1;
//			uint8 ucCardECCFailed       : 1;
//			uint8 ucWriteProtectError   : 1;
//			uint8 ucEraseParameter      : 1;
//			uint8 ucOutOfRange          : 1; //Out of Range error OR CSD overwrite error
//		} flags;
//	};
//} SD_CSD;

typedef struct tagSD_Info
{
	unsigned char ucType;
	unsigned char ucVersion;
	unsigned long ulNumBlocks;
} SD_INFO;

typedef struct tagSD_DataBlock
{
	unsigned char magic_num[4];		//Should always be 'SEGA' for formatted SEGA card (0x53 0x45 0x47 0x41)
	unsigned char version;			//Compatible SEGA SD driver version
	unsigned char start_block;		//First block of actual data
} SD_DataBlock;

//SD Information Defines
#define SD_TYPE_SDSC 1
#define SD_TYPE_SDHC 2
#define SD_TYPE_SDXC 3
#define SD_VERSION_1 1
#define SD_VERSION_2 2
#define SD_VERSION_3 3
#define SD_VERSION_4 4

//SD Driver Return Codes
#define SD_SUCCESS 0x00
#define SD_FAILED  0x01

//Misc. Defines
#define SD_DRIVER_VERSION	0x02																													// Version of the driver
#define SD_CARD_CODE_METABLOCK		1																											// Block where metadata about program code is stored
#define SD_CARD_CODE_STARTBLOCK		2																											// Block where program code is stored
#define SD_CARD_CODE_ALLOCATION		40																										// Number of blocks allocated to program code
#define SD_CARD_DATA_BLOCK	(SD_CARD_CODE_STARTBLOCK + SD_CARD_CODE_ALLOCATION + 1)			// Block with meta-data table
#define SD_CARD_START_BLOCK (SD_CARD_DATA_BLOCK + 1)																		// First block of actual data
#define SD_CARD_BLOCKLEN	512
#define SD_CMD_TIMEOUT		300
#define MSK_TOK_DATAERROR 	0xE0
#define SD_IDLE_CRC			0x95
#define SD_IFCOND_CRC		0x87
#define SD_DEFAULT_CRC		0xFF

//Driver API Prototypes
unsigned char ucSD_Init(void);
void vSD_PowerOn(void);
void vSD_PowerOff(void);
unsigned long ulSD_GetCapacity(void);
unsigned char ucSD_GetType(void);
char SD_Write_Block(uint8 *pucData_TXBuffer, unsigned long ulAddress);
unsigned char SD_Read_Block(uint8 *pucData_RXBuffer, unsigned long ulAddress);
char SD_Erase_Block(unsigned long ulBlockNum);
unsigned char SD_Format();
unsigned char SD_CheckFormat();
unsigned char SD_Read_MultipleBlocks(unsigned char *pucData_RXBuffer, unsigned long ulStartBlock, uchar ucBlockCount);
unsigned char ucSD_CheckForCard(void);

#endif
