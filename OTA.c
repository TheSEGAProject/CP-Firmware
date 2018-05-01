//! \file OTA.c
//! \brief Handles Over The Air reprogramming of MCUs

#include "comm.h"
#include "delay.h"
#include "misc.h"
#include "serial.h"
#include "std.h"
#include "SD_Card.h"
#include "task.h"
#include "time.h"
#include "report.h"
#include "mem_mod.h"
#include "crc.h"
#include "OTA.h"
#include "SP.h"
#include "SP_BSL.h"
#include "SP_UART.h"
#include "PMM.h"
#include <stdio.h>

#define NEGATIVE_ACK 		0
#define ACKNOWLEDGMENT 	1

//! \def PRGM_BUFFERSZ
//! \brief Size of program code buffer (must be in multiples of SD card block size)
#define PRGM_BUFFERSZ 	(2*SD_CARD_BLOCKLEN)

union U_Metadata U_PrgmMetadata;

//! \struct S_ImageData
//! \brief Structure used to store data about the entire image
struct
{
	uint8 m_ucSemaphore; //!< Used to restrain access to this structure
	uint8 m_ucBoardNum; //!< The processor to be reprogrammed
	uint8 m_ucBoardType; //!< The board type being reprogrammed
	uint8 m_ucProgID; //!< The unique ID of the programming update
	uint16 m_uiCRC; //!< The CRC for the entire
	uint32 m_ulLength; //!< The number of bytes in the image
	uint32 m_BytesReceived; //!< The number of bytes received
} S_ImageData;

struct
{

	uchar m_ucProgramCode[PRGM_BUFFERSZ]; //!< array that stores program code and metadata before writing to SD card
	uint m_uiPrgmCodeIdx; //!< Index into the program code array
	uint m_uiBytesReceived; //!< The number of bytes of code received
} S_CodeBlock;

ulong g_ulSDCardAddress = 0;

extern volatile uchar ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];

void vOTA_Report(uchar ucMsgID);
uchar ucOTA_ReadFromSDCard(ulong ulAddress, uchar *ucReturnBlock);

//! \fn ulOTA_GetSDCardAddress
//! \brief Returns the current address for the SD card
ulong ulOTA_GetSDCardAddress(void)
{
	return g_ulSDCardAddress;
}

//! \fn vOTA_SetSDCardAddress
//! \brief Sets the current address for the SD card
void vOTA_SetSDCardAddress(ulong ulAddress)
{
	g_ulSDCardAddress = ulAddress;
}

//! \fn vOTA_IncrementSDCardAddress
//! \brief Points to the next 1kB block on the SD card
static void vOTA_IncrementSDCardAddress(void)
{
	g_ulSDCardAddress += 2;
}

//! \fn vOTA_init
//! \brief Initializes variable pertaining to OTA
void vOTA_init(void)
{
	uint uiByteCount;

	// Initialize the metadata union
	U_PrgmMetadata.fields.m_ucBoardNum = 0;
	U_PrgmMetadata.fields.m_ucBoardType = 0;
	U_PrgmMetadata.fields.m_ucProgID = 0;
	U_PrgmMetadata.fields.m_ucSemaphore = 0;
	U_PrgmMetadata.fields.m_uiCRC = 0;
	U_PrgmMetadata.fields.m_uiLength = 0;
	U_PrgmMetadata.fields.m_ulStartAddr = 0;

	// Initialize the image structure
	S_ImageData.m_ucBoardNum = 0;
	S_ImageData.m_ucBoardType = 0;
	S_ImageData.m_ucProgID = 0;
	S_ImageData.m_ucSemaphore = 0;
	S_ImageData.m_uiCRC = 0;
	S_ImageData.m_ulLength = 0;
	S_ImageData.m_BytesReceived = 0;

	vOTA_SetSDCardAddress(0);

	for (uiByteCount = 0; uiByteCount < PRGM_BUFFERSZ; uiByteCount++)
		S_CodeBlock.m_ucProgramCode[uiByteCount] = 0xFF;

}

////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_GetMetadataFromBlock
//! \brief Retrieves the metadata from a block
//! \param *MetaData
//! \param *ucBlock
///////////////////////////////////////////////////////////////////////////////////////////
void vOTA_GetMetadataFromBlock(union U_Metadata * MetaData, uchar *ucBlock)
{
	uchar ucByteCount;

	// Loop throught the block and read out the metadata
	for (ucByteCount = 0; ucByteCount < 13; ucByteCount++) {
		MetaData->ucByteStream[ucByteCount] = *ucBlock++;
	}

}

////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_AppendToProgramCode
//!	\brief Writes data to global program code buffer
//!	\param *ucArray
//! \param ucLength
//! \return 0 for success, 1 if buffer is too full to write to
////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_AppendToProgramCodeBuffer(uchar *ucArray, uint uiLength)
{
	uchar ucByteCount;
	uint uiSpaceLeft;

	// Find out how much room is left in the buffer
	uiSpaceLeft = PRGM_BUFFERSZ - S_CodeBlock.m_uiPrgmCodeIdx;

	// If there isn't enough space left then return 1
	if (uiLength > uiSpaceLeft)
		return 1;

	// Write to program code buffer
	for (ucByteCount = 0; ucByteCount < uiLength; ucByteCount++) {
		S_CodeBlock.m_ucProgramCode[S_CodeBlock.m_uiPrgmCodeIdx++] = *ucArray++;
	}

	// If we have the header stored in the code block, then  bytes recieved = index - header otherwise bytes received = 0
	if (S_CodeBlock.m_uiPrgmCodeIdx > 12)
		S_CodeBlock.m_uiBytesReceived = S_CodeBlock.m_uiPrgmCodeIdx - 12;
	else
		S_CodeBlock.m_uiBytesReceived = 0;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_ValidateBlock
//!	\brief Checks the CRC on a block and compares it to what was received in the metadata header
//!	\return 0 for success, else failure
////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_ValidateBlock(void)
{
	uint uiReceivedCRC, uiComputedCRC;

	uiReceivedCRC = U_PrgmMetadata.fields.m_uiCRC;

	uiComputedCRC = uiCRC16_ComputeBlockCRC(&S_CodeBlock.m_ucProgramCode[12], U_PrgmMetadata.fields.m_uiLength);

	// If the CRCs match then success
	if (uiReceivedCRC == uiComputedCRC) {
		puts("Blk CRC Good");
		return 0;
	}

	// Failure
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_ValidateImage
//!	\brief Checks the CRC on the entire image and compares it to what was received in the image data packet
//!	\return 0 for success, else failure
////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_ValidateImage(void)
{
	uint uiComputedCRC;
	uchar ucBlock[2 * SD_CARD_BLOCKLEN];
	ulong ulAddress;
	union U_Metadata TempMeta;

	// Init the CRC as per CCIT specifications
	uiComputedCRC = 0xFFFF;

	// Loop through all
	for (ulAddress = 0; ulAddress < ulOTA_GetSDCardAddress(); (ulAddress+=2)) {

		// Read a block from the SD card and get its metadata
		if(ucOTA_ReadFromSDCard(ulAddress, ucBlock) != 0) return 1;
		vOTA_GetMetadataFromBlock(&TempMeta, ucBlock);

		// Validate the CRC for this block
		if(uiCRC16_ComputeBlockCRC(&ucBlock[12], TempMeta.fields.m_uiLength) != TempMeta.fields.m_uiCRC) return 1;

		// Compute the CRC for this block using the previous blocks CRC as an initial value
		uiComputedCRC = uiCRC16_ComputeCRCwithInit(&ucBlock[12], TempMeta.fields.m_uiLength, uiComputedCRC);
	}

	// If the CRCs match then success
	if (S_ImageData.m_uiCRC == uiComputedCRC) {
		puts("Image CRC Good");
		return 0;
	}

	// Failure
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_WriteToSDCard
//! \brief Writes the program code block to the SD card
//! \param ulAddress
//!	\return 0 for success, else failure
////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_WriteToSDCard(ulong ulAddress)
{
	uchar ucBlock[2][SD_CARD_BLOCKLEN];

	uint uiByteCount, uiPrgmCounter;
	uint uiCount;
	uchar ucBlockCount;
	uchar ucRetVal;
	const uchar ucTimeout = 50;

	// Assume success
	ucRetVal = 0;

	uiPrgmCounter = 0;

	// Write the global program code array to a local block
	for (ucBlockCount = 0; ucBlockCount < 2; ucBlockCount++) {
		for (uiByteCount = 0; uiByteCount < SD_CARD_BLOCKLEN; uiByteCount++) {
			ucBlock[ucBlockCount][uiByteCount] = S_CodeBlock.m_ucProgramCode[uiPrgmCounter++];
		}
	}

	// Power up the SD card and run the initialization sequence
	vSD_PowerOn();

	// Try the initialization function a few times
	for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
		if (ucSD_Init() == SD_SUCCESS)
			break;
	}
	if (uiCount == ucTimeout) {
		ucRetVal = 1;
#if 1
		vSERIAL_sout("OTA SD Init Fail\r\n", 18);
#endif
	}

	// Only proceed if initialization was successful
	if (uiCount != ucTimeout) {

		for (ucBlockCount = 0; ucBlockCount < 2; ucBlockCount++) {
			// Write the block to the SD card
			for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
				if (SD_Write_Block(ucBlock[ucBlockCount], ulAddress) == SD_SUCCESS) {
					ulAddress++;
					break;
				}
			}
		}
		if (uiCount == ucTimeout) {
			ucRetVal = 1;
#if 1
			vSERIAL_sout("SD write fail\r\n", 15);
#endif
		}
	}

	//Power down the SD card
	vSD_PowerOff();

	return ucRetVal;
}

////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_ReadFromSDCard
//! \brief Writes the program code block to the SD card
//! \param ulAddress
//!	\return 0 for success, else failure
////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_ReadFromSDCard(ulong ulAddress, uchar *ucReturnBlock)
{
	uchar ucBlock[2][SD_CARD_BLOCKLEN];

	uint uiByteCount;
	uint uiCount;
	uchar ucBlockCount;
	uchar ucRetVal;
	const uchar ucTimeout = 50;

	// Assume success
	ucRetVal = 0;

	// Clear the local block
	for (ucBlockCount = 0; ucBlockCount < 2; ucBlockCount++) {
		for (uiByteCount = 0; uiByteCount < SD_CARD_BLOCKLEN; uiByteCount++) {
			ucBlock[ucBlockCount][uiByteCount] = 0x00;
		}
	}

	// Power up the SD card and run the initialization sequence
	vSD_PowerOn();

	// Try the initialization function a few times
	for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
		if (ucSD_Init() == SD_SUCCESS)
			break;
	}
	if (uiCount == ucTimeout) {
		ucRetVal = 1;
#if 1
		vSERIAL_sout("OTA SD Init Fail\r\n", 18);
#endif
	}

	// Only proceed if initialization was successful
	if (uiCount != ucTimeout) {

		for (ucBlockCount = 0; ucBlockCount < 2; ucBlockCount++) {
			// Write the block to the SD card
			for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
				if (SD_Read_Block(ucBlock[ucBlockCount], ulAddress) == SD_SUCCESS) {
					ulAddress++;
					break;
				}
			}
		}
		if (uiCount == ucTimeout) {
			ucRetVal = 1;
#if 1
			vSERIAL_sout("SD read fail\r\n", 15);
#endif
		}
	}

	//Power down the SD card
	vSD_PowerOff();

	// Clear the local block
	for (ucBlockCount = 0; ucBlockCount < 2; ucBlockCount++) {
		for (uiByteCount = 0; uiByteCount < SD_CARD_BLOCKLEN; uiByteCount++) {
			*ucReturnBlock++ = ucBlock[ucBlockCount][uiByteCount];
		}
	}

	return ucRetVal;
}

////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_ValidateWrite
//! \brief Reads the desired block from the SD Card and compares it to the current
////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_ValidateWrite(ulong ulAddress, uchar ucRAMBlock[2 * SD_CARD_BLOCKLEN])
{
	uchar ucBlock[2 * SD_CARD_BLOCKLEN];
	uint uiByteCount;

	ucOTA_ReadFromSDCard(ulAddress, ucBlock);

	for (uiByteCount = 0; uiByteCount < (2 * SD_CARD_BLOCKLEN); uiByteCount++) {
		if (ucBlock[uiByteCount] != ucRAMBlock[uiByteCount]) {
			return 1;
		}
	}

	puts("SD Write Good");
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_ParseCodePacket
//!	\brief Reads the message buffer and writes the fields to the program code union
//!	\param *ucaMSGBuffer
//! \param *U_ProgramCode
//!	\return 0 for success, else failure
////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_ParseCodePacket(volatile uchar *ucaMSGBuffer, union DE_Code * U_ProgramCode)
{
	uchar ucMsgIndex, ucByteCount;

	// Reset the program code array
	for (ucByteCount = 0; ucByteCount < MAX_CODE; ucByteCount++) {
		U_ProgramCode->fields.m_ucCode[ucByteCount] = 0xFF;
	}

	// Set the starting point to the payload
	ucMsgIndex = MSG_IDX_PAYLD;

	// Write the data element to the program code union
	U_ProgramCode->fields.m_ucDEID = ucaMSGBuffer[ucMsgIndex++];
	U_ProgramCode->fields.m_ucDE_Length = ucaMSGBuffer[ucMsgIndex++];
	U_ProgramCode->fields.m_ucVersion = ucaMSGBuffer[ucMsgIndex++];
	U_ProgramCode->fields.m_ucBoardNum = ucaMSGBuffer[ucMsgIndex++];
	U_ProgramCode->fields.m_ucBoardType = ucaMSGBuffer[ucMsgIndex++];
	U_ProgramCode->fields.m_ucProgID = ucaMSGBuffer[ucMsgIndex++];
	U_ProgramCode->fields.m_uiComponentNum = uiMISC_buildUintFromBytes((uchar*) &ucaMSGBuffer[ucMsgIndex], YES_NOINT);
	ucMsgIndex += 2;
	U_ProgramCode->fields.m_ulStartAddr = (long) ulMISC_buildUlongFromBytes((uchar*) &ucaMSGBuffer[ucMsgIndex], YES_NOINT);
	ucMsgIndex += 4;

	// Make sure that the data element is program code
	if (U_ProgramCode->fields.m_ucDEID != PROGRAM_CODE)
		return 1;

	// Check the size of the program code before copying to the buffer
	if (U_ProgramCode->fields.m_ucDE_Length <= MAX_CODE) {

		// Get the code
		for (ucByteCount = 0; ucByteCount < (U_ProgramCode->fields.m_ucDE_Length - CODEHEADERSIZE); ucByteCount++) {
			U_ProgramCode->fields.m_ucCode[ucByteCount] = ucaMSGBuffer[ucMsgIndex++];
		}
	}
	else {
		return 1;
	}

	// Success
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_ParseMetaPacket
//! \brief Parses the reprogramming metadata message and assignes fields to S_PrgmMetaData
//!	\param *ucaMSG_BUFF
//! \return 0 = success
//////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_ParseMetaPacket(volatile uchar *ucaMSGBuffer)
{
	uchar ucMsgIndex;

	// The metadata structure is not accessible, it is currently in use.
	if (U_PrgmMetadata.fields.m_ucSemaphore != 0)
		return 1;

	// Make sure that the incoming update is expected
	if (S_ImageData.m_ucSemaphore == 0)
		return 1;

	// Set the starting point to the payload
	ucMsgIndex = MSG_IDX_PAYLD;

	// Make sure it is the correct DE
	if (ucaMSGBuffer[ucMsgIndex++] != PROGRAM_CODE_META)
		return 1;
	// Skip the version and length bytes
	ucMsgIndex += 2;

	// Write the fields to the structure
	U_PrgmMetadata.fields.m_ucBoardNum = ucaMSGBuffer[ucMsgIndex++];
	U_PrgmMetadata.fields.m_ucBoardType = ucaMSGBuffer[ucMsgIndex++];
	U_PrgmMetadata.fields.m_ucProgID = ucaMSGBuffer[ucMsgIndex++];
	U_PrgmMetadata.fields.m_uiCRC = uiMISC_buildUintFromBytes((uchar*) &ucaMSGBuffer[ucMsgIndex], YES_NOINT);
	ucMsgIndex += 2;
	U_PrgmMetadata.fields.m_uiLength = uiMISC_buildUintFromBytes((uchar*) &ucaMSGBuffer[ucMsgIndex], YES_NOINT);
	ucMsgIndex += 2;
	U_PrgmMetadata.fields.m_ulStartAddr = (long) ulMISC_buildUlongFromBytes((uchar*) &ucaMSGBuffer[ucMsgIndex], YES_NOINT);
	ucMsgIndex += 4;

//	vSERIAL_sout("--- Prgm Metadata ---\r\n", 23);
//	vSERIAL_sout("Semaphore: ", 11);vSERIAL_UI8out(S_PrgmMetaData.m_ucSemaphore);vSERIAL_crlf();
//	vSERIAL_sout("Board Num: ", 11);vSERIAL_UI8out(S_PrgmMetaData.m_ucBoardNum);vSERIAL_crlf();
//	vSERIAL_sout("Board Typ: ", 11);vSERIAL_UI8out(S_PrgmMetaData.m_ucBoardType);vSERIAL_crlf();
//	vSERIAL_sout("Prgrm Id:  ", 11);vSERIAL_UI8out(S_PrgmMetaData.m_ucProgID);vSERIAL_crlf();
//	vSERIAL_sout("CRC:       ", 11);vSERIAL_HB16out(S_PrgmMetaData.m_uiCRC);vSERIAL_crlf();
//	vSERIAL_sout("Blk Len:   ", 11);vSERIAL_UI16out(S_PrgmMetaData.m_uiLength);vSERIAL_crlf();
//	vSERIAL_sout("Strt Addr: ", 11);vSERIAL_HB32out(S_PrgmMetaData.m_ulStartAddr);vSERIAL_crlf();

	// Check the block header against the image data
	if (S_ImageData.m_ucBoardNum != U_PrgmMetadata.fields.m_ucBoardNum || S_ImageData.m_ucBoardType != U_PrgmMetadata.fields.m_ucBoardType
			|| S_ImageData.m_ucProgID != U_PrgmMetadata.fields.m_ucProgID)
		return 1;

	// The metadata structure is now locked until this block of code has been received
	U_PrgmMetadata.fields.m_ucSemaphore = 1;

	// Success
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_ParseImageDataPacket
//! \brief Parses the image data from the packet and stores it in the image structure
//!	\param *ucaMSG_BUFF
//! \return 0 = success
//////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_ParseImageDataPacket(volatile uchar *ucaMSGBuffer)
{
	uchar ucMsgIndex;

	// The metadata structure is not accessible, it is currently in use
	if (S_ImageData.m_ucSemaphore != 0)
		return 1;

	// Set the starting point to the payload
	ucMsgIndex = MSG_IDX_PAYLD;

	// Make sure it is the correct DE
	if (ucaMSGBuffer[ucMsgIndex++] != PROGRAM_CODE_INIT)
		return 1;
	// Skip the version and length bytes
	ucMsgIndex += 2;

	// Write the fields to the structure
	S_ImageData.m_ucBoardNum = ucaMSGBuffer[ucMsgIndex++];
	S_ImageData.m_ucBoardType = ucaMSGBuffer[ucMsgIndex++];
	S_ImageData.m_ucProgID = ucaMSGBuffer[ucMsgIndex++];
	S_ImageData.m_uiCRC = uiMISC_buildUintFromBytes((uchar*) &ucaMSGBuffer[ucMsgIndex], YES_NOINT);
	ucMsgIndex += 2;
	S_ImageData.m_ulLength = ulMISC_buildUlongFromBytes((uchar*) &ucaMSGBuffer[ucMsgIndex], YES_NOINT);
	ucMsgIndex += 4;

//	vSERIAL_sout("--- Image data ---\r\n", 23);
//	vSERIAL_sout("Semaphore: ", 11);vSERIAL_UI8out(S_ImageData.m_ucSemaphore);vSERIAL_crlf();
//	vSERIAL_sout("Board Num: ", 11);vSERIAL_UI8out(S_ImageData.m_ucBoardNum);vSERIAL_crlf();
//	vSERIAL_sout("Board Typ: ", 11);vSERIAL_UI8out(S_ImageData.m_ucBoardType);vSERIAL_crlf();
//	vSERIAL_sout("Prgrm Id:  ", 11);vSERIAL_UI8out(S_ImageData.m_ucProgID);vSERIAL_crlf();
//	vSERIAL_sout("CRC:       ", 11);vSERIAL_HB16out(S_ImageData.m_uiCRC);vSERIAL_crlf();
//	vSERIAL_sout("Blk Len:   ", 11);vSERIAL_UI16out(S_ImageData.m_ulLength);vSERIAL_crlf();

//		puts("--- Image data ---\r\n");
//		printf("Semaphore: %d \n", S_ImageData.m_ucSemaphore);
//		printf("Board Num: %d \n", S_ImageData.m_ucBoardNum);
//		printf("Board Typ: %d \n", S_ImageData.m_ucBoardType);
//		printf("Prgrm Id:  %d \n", S_ImageData.m_ucProgID);
//		printf("CRC:       %x \n", S_ImageData.m_uiCRC);
//		printf("Img Len:   %lu \n", S_ImageData.m_ulLength);

	// The metadata structure is now locked until this block of code has been received
	S_ImageData.m_ucSemaphore = 1;

	// Success
	return 0;

}

uchar ucOTA_write_programheader(union DE_Code * ProgramCode)
{
	uchar ucBlock[SD_CARD_BLOCKLEN];
	uchar ucBlkIndex;
	uchar ucAttemptCount;
	ulong ulAddress;
	uint uiCount;
	uchar ucErrorCode;
	uchar ucErrorCodePriority;
	uchar ucMsgIndex;
	const uchar ucTimeout = 50;
	const uchar MetaSize = 9;
	uint uiCRC;
	ulong ulImageSize_Packets;

	// The address where SD card metadata is stored
	ulAddress = SD_CARD_CODE_METABLOCK;

	// Optimism at it's finest
	ucErrorCode = 0;

//	ProgramCode->fields.m_ucProgID;

// If at first you don't succeed.....
	ucAttemptCount = 5;

	// Try 5 times or until the subslot ends, whichever comes first
	while (ucAttemptCount-- > 0 && (ucTimeCheckForAlarms(SUBSLOT_WARNING_ALARM_BIT) == 0)) {

		// Power up the SD card and run the initialization sequence
		vSD_PowerOn();

		// Try the initialization function a few times
		for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
			if (ucSD_Init() == SD_SUCCESS)
				break;
		}

		// Set the return code to indicate a failure
		if (uiCount == ucTimeout) {
			ucErrorCode = SRC_ID_SDCARD_INIT_FAIL;
			ucErrorCodePriority = RPT_PRTY_SDCARD_INIT_FAIL;
//			vSERIAL_sout("SD Init Fail\r\n", 14);
		}

		// Only proceed if initialization was successful
		if (ucErrorCode == 0) {
			// Read the block from the SD card
			for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
				if (SD_Read_Block(ucBlock, ulAddress) == SD_SUCCESS)
					break;
			}
		}

		ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucProgID;
		ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucBoardNum;
		ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucBoardType;

//	ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucCode[];
//	ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucCode[];
//	ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucCode[];
//	ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucCode[];
//
//	ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucCode[];
//	ucBlock[ucBlkIndex++] = ProgramCode->fields.m_ucCode[];

// Fetch the image size and CRC
//		ulImageSize_Packets = ulMISC_buildUlongFromBytes(ProgramCode->fields.m_ucCode[ucMsgIndex], YES_NOINT);
//		ucMsgIndex += 4;
//
//		uiCRC = uiMISC_buildUintFromBytes(ProgramCode->fields.m_ucCode[ucMsgIndex], YES_NOINT);
//		ucMsgIndex += 2;
//
//		uint8 m_ucBoardNum; //!< The processor to be reprogrammed
//		uint8 m_ucBoardType; //!< The board type being reprogrammed
//		uint8 m_ucProgID; //!< The unique ID of the programming update

// Only proceed if initialization was successful
		if (ucErrorCode == 0) {
			// Write the block to the SD card
			for (uiCount = 0; uiCount < ucTimeout; uiCount++) {
				if (SD_Write_Block(ucBlock, ulAddress) == SD_SUCCESS)
					break;
			}
			if (uiCount == ucTimeout) {
				ucErrorCode = SRC_ID_SDCARD_WRITE_FAIL;
				ucErrorCodePriority = RPT_PRTY_SDCARD_WRITE_FAIL;
//				vSERIAL_sout("SD write fail\r\n", 15);
			}
			else {
				//Power down the SD card and exit
				vSD_PowerOff();
				break;
			}
		}

		//Power down the SD card
		vSD_PowerOff();

		// Allow time for the voltage to decay
		vDELAY_LPMWait1us(5000, 1);

	} // END: while(ucAttemptCount)

	// If there was an error...
	if (ucErrorCode != 0) {

		// Build the report data element header
		vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
		ucMsgIndex = DE_IDX_RPT_PAYLOAD;
		ucaMSG_BUFF[ucMsgIndex++] = ucErrorCode;
		ucaMSG_BUFF[ucMsgIndex++] = 0; // data length

		// Store DE
		vReport_LogDataElement(ucErrorCodePriority);

	}
	else {

// Update the SD card address
		vL2FRAM_IncrementSDCardBlockNum();

// Write the messages in the overflow area to the buffer
		vL2FRAM_SDCardOvrflowToBuff();
	}
}

//! \fn vOTA_ReceiveCodePacket
//! \brief Takes in a code packet and stores it
//!
//!

void vOTA_ReceiveCodePacket(union DE_Code * ProgramCode)
{
	uchar ucMsgIndex;
	uint uiCRC;
	ulong ulImageSize_Packets;

	// Make sure that it is a program packet
	if (ProgramCode->fields.m_ucDEID != PROGRAM_CODE) {
		return;
	}

	ucMsgIndex = 0;
	// The first packet in the sequence contains required header information
	if (ProgramCode->fields.m_uiComponentNum == 0) {

		ucOTA_write_programheader(ProgramCode);
	}
}

/////////////////////////////////////////////////////////////////////////
//! \fn vOTA_UpdateOverSerial
//! \brief Takes in code packets and pushes them directly to an SP
//!
/////////////////////////////////////////////////////////////////////////
void vOTA_UpdateOverSerial(void)
{
	uchar ucSPIndex, ucByteCount;
	uchar MsgGoodFlag, DEGoodFlag;
	uint uiNextComponent;
	union DE_Code U_ProgramCode;
	uchar ucDEID;
	uchar ucBlockCounter;

	ucBlockCounter = 0;

	// Start at the beginning (very zenlike)
	U_ProgramCode.fields.m_uiComponentNum = uiNextComponent = 1;

	puts("running...");

	while (1) {

		// Assume message will fail
		MsgGoodFlag = DEGoodFlag = FALSE;

		// Wait for a packet over serial
		if (!ucSERIAL_Wait_for_MessageNoTimeout()) {
			// If we have the message then read it into the message buffer
			vSERIAL_ReadBuffer(ucaMSG_BUFF);

			/* GOT A MSG -- CHK FOR: CRC, MSGTYPE,*/
			if (!(ucComm_chkMsgIntegrity(CHKBIT_CRC + CHKBIT_MSG_TYPE, CHKBIT_CRC + CHKBIT_MSG_TYPE, MSG_ID_OPERATIONAL, 0, 0))) {
				MsgGoodFlag = TRUE;
			}
			else {
				__no_operation();
			}
		}

		// If the message is good check the message number and push to SP
		if (MsgGoodFlag == TRUE) {

			ucDEID = ucaMSG_BUFF[MSG_IDX_PAYLD];

			switch (ucDEID)
			{

				case PROGRAM_CODE_INIT:
					// Pull the data from the packet and stuff it in the image structure
					if (ucOTA_ParseImageDataPacket(ucaMSG_BUFF) == 0) {
						DEGoodFlag = TRUE;
					}
				break;

				case PROGRAM_CODE_META:
					// Pull the metadata from the packet and stuff it in the metadata union
					if (ucOTA_ParseMetaPacket(ucaMSG_BUFF) == 0) {
						DEGoodFlag = TRUE;

						// Make sure that the program index is reset
						S_CodeBlock.m_uiPrgmCodeIdx = 0;

						// Write the metadata to the start of the program code block
						ucOTA_AppendToProgramCodeBuffer(U_PrgmMetadata.ucByteStream, 12);
						printf("---Block %d ---\n", ucBlockCounter);
						ucBlockCounter++;
					}
					else {
						puts("Parse Metadata Failed");
					}

				break;

				case PROGRAM_CODE:
					// Parse the packet and writes to ProgramCode union
					if (ucOTA_ParseCodePacket(ucaMSG_BUFF, &U_ProgramCode) == 0) {
						DEGoodFlag = TRUE;
						// Write the code to the program code block, if returns 1 then the buffer is full
//						if(U_ProgramCode.fields.m_uiComponentNum == uiNextComponent){
						if (ucOTA_AppendToProgramCodeBuffer(U_ProgramCode.fields.m_ucCode, (U_ProgramCode.fields.m_ucDE_Length - CODEHEADERSIZE)) == 1) {
							//vSERIAL_sout("PrgrmCde Buff OvrFlw\r\n", 21);
							__no_operation();
							puts("Append code failure");
						}

						// If we have received all the data we expect then validate and write to the SD Card
						if (S_CodeBlock.m_uiBytesReceived == U_PrgmMetadata.fields.m_uiLength) {

							// Validate the CRC on this block
							if (ucOTA_ValidateBlock() == 0) {

								// If the SD card write is a success then reset the program code buffer index
								if (ucOTA_WriteToSDCard(ulOTA_GetSDCardAddress()) == 0) {

									// Verify that the write to the SD card was successful
									if (ucOTA_ValidateWrite(ulOTA_GetSDCardAddress(), S_CodeBlock.m_ucProgramCode) != 0)
										puts("Write validata fail");

									// Update the SD card address and reset the program code index
									vOTA_IncrementSDCardAddress();
									S_CodeBlock.m_uiPrgmCodeIdx = 0;

									// Add the length of this block to the total bytes received
									S_ImageData.m_BytesReceived += U_PrgmMetadata.fields.m_uiLength;

									// Clear the semaphore to prepare for the next block
									U_PrgmMetadata.fields.m_ucSemaphore = 0;

								}
							}
							// Validation failed
							else {
								__no_operation();
								puts("Block validate failed");
							}
						}
					}
					else {
						puts("Parsing code failure");
						__no_operation(); // Parse failed
					}
				break;

				default:
					// Non program code DE received
					puts("Unknown packet received");
				break;
			}
		}

		// If the message or data element fail then send nack
		if (DEGoodFlag == FALSE || MsgGoodFlag == FALSE) {
			vOTA_Report(NEGATIVE_ACK);
		}
		else {
			vOTA_Report(ACKNOWLEDGMENT);
		}

		// If we have received the entire image and it is good then break out and reprogramm the target
		if (S_ImageData.m_BytesReceived == S_ImageData.m_ulLength) {
			if (ucOTA_ValidateImage() != 0) {
				__no_operation();
				puts("Image validataion failed");
			}
			else
				break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
//! \fn vOTA_Report
//!	\brief Builds and sends messages to the PC
//!	\param ucMsgID, the type of message to send
//!
//////////////////////////////////////////////////////////////////////
void vOTA_Report(uchar ucMsgID)
{
	uchar ucByteCount;
	uchar ucMsgIndex;
	long lTime;
	uint uiMsgNumber;
	uchar ucMsgNumArray[2];
	//get the system time
	lTime = lTIME_getSysTimeAsLong();

//	//stuff the received fields into the data element structure
//	ucaMSG_BUFF[DE_IDX_ID] = REPORT_DATA;
//	ucaMSG_BUFF[DE_IDX_LENGTH] = ucPayloadLen + 8;
//	ucaMSG_BUFF[DE_IDX_VERSION] = ucVersion;
//	ucaMSG_BUFF[DE_IDX_RPT_PROCID] = ucProcID;
//
//	// Pack the time into the buffer
//	ucaMSG_BUFF[DE_IDX_TIME_SEC_LO] = (uchar) lTime;
//	lTime = lTime >> 8;
//	ucaMSG_BUFF[DE_IDX_TIME_SEC_MD] = (uchar) lTime;
//	lTime = lTime >> 8;
//	ucaMSG_BUFF[DE_IDX_TIME_SEC_HI] = (uchar) lTime;
//	lTime = lTime >> 8;
//	ucaMSG_BUFF[DE_IDX_TIME_SEC_XI] = (uchar) lTime;
//
//
//	// Build the report data element header
//	vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
//	ucMsgIndex = DE_IDX_RPT_PAYLOAD;
//	ucaMSG_BUFF[ucMsgIndex++] = ucErrorCode;
//	ucaMSG_BUFF[ucMsgIndex++] = 0; // data length

	// All messages are for the PC so the net layer can stay the same
	vComm_NetPkg_buildHdr(0xFEFE);

	switch (ucMsgID)
	{

		case NEGATIVE_ACK:

			// Store a local copy of the message number
			uiMsgNumber = (uint) ((ucaMSG_BUFF[MSG_IDX_NUM_HI] << 8) | ucaMSG_BUFF[MSG_IDX_NUM_LO]);

			// Single message, since it is a nack, don't set the ack bit
			vComm_Msg_buildOperational((MSG_FLG_SINGLE), uiMsgNumber, 0xFEFE, MSG_ID_OPERATIONAL);

			ucaMSG_BUFF[MSG_IDX_LEN] = MSG_HDR_SZ;
		break;

		case ACKNOWLEDGMENT:

			// Store a local copy of the message number
			uiMsgNumber = (uint) ((ucaMSG_BUFF[MSG_IDX_NUM_HI] << 8) | ucaMSG_BUFF[MSG_IDX_NUM_LO]);

			// Single message
			vComm_Msg_buildOperational((MSG_FLG_SINGLE | MSG_FLG_ACK), uiMsgNumber, 0xFEFE, MSG_ID_OPERATIONAL);

			ucaMSG_BUFF[MSG_IDX_LEN] = MSG_HDR_SZ;
		break;

	}

// COMPUTE THE CRC
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, (ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ)); //lint !e534 //compute the CRC

// Send the message
	for (ucByteCount = 0; ucByteCount < (ucaMSG_BUFF[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ); ucByteCount++) {
		vSERIAL_HB8out(ucaMSG_BUFF[ucByteCount]);
	}
	vSERIAL_crlf();

}


//////////////////////////////////////////////////////////////////////////////////////////////
//! \fn ucOTA_WriteBlockToSP
//! \brief Writes a block of code to the SP board
//////////////////////////////////////////////////////////////////////////////////////////////
uchar ucOTA_WriteBlockToSP(uint uiBlockCounter)
{
	uint uiByteCount;
	uchar ucBlock[2 * SD_CARD_BLOCKLEN];
	union U_Metadata TempMeta;
	const uchar ucDefaultLength = 200;
	unsigned char ucPayloadLength;
	uint uiRemainingBytes;
	uchar ucAttemptCount, ucReturnCode;

	// Read the code from the SD card
	if (ucOTA_ReadFromSDCard(uiBlockCounter, ucBlock) != 0) {
		puts("SD card read failed during reprogramming");
		return 0;
	}

		// Pull the metadata information from each block
		vOTA_GetMetadataFromBlock(&TempMeta, ucBlock);

		// Loop through the block and send packets to the SP's BSL
		for (uiByteCount = 0; uiByteCount < TempMeta.fields.m_uiLength;) {

			// Determine the number of bytes left in this block and determine the payload length
			uiRemainingBytes = (TempMeta.fields.m_uiLength - uiByteCount);
			if (uiRemainingBytes >= ucDefaultLength)
				ucPayloadLength = ucDefaultLength;
			else
				ucPayloadLength = (uchar) uiRemainingBytes;

			ucAttemptCount = 75;

			if(ucPayloadLength % 2 != 0)
				ucPayloadLength++;

			while (ucAttemptCount-- != 0) {
				ucReturnCode = ucSP_BSL_Send_Data(&ucBlock[(uiByteCount + 12)], (TempMeta.fields.m_ulStartAddr + uiByteCount), ucPayloadLength);
				if (ucReturnCode == SUCCESSCODE)
					break;
				else {
					printf("SP BSL write error at address %04x\r\n", (uint) (TempMeta.fields.m_ulStartAddr + uiByteCount));
					vDELAY_LPMWait1us(1000, 0);
				}
				if (ucAttemptCount == 0) {
					puts("She's a brick....House");
					return 0;
				}
			}
			uiByteCount += ucPayloadLength;
		}
		return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! \fn vOTA_ReprogramSP
//! \brief Performs all tasks required to reprogram an SP board
//////////////////////////////////////////////////////////////////////////////////////////////
void vOTA_ReprogramSP(void)
{
	uint uiBlockCounter;
	uchar ucProgrammingStatus, ucProgrammingAttempts;
	uchar ucSPIndex;

	ucSPIndex = S_ImageData.m_ucBoardNum - 1;

	// Assume failure and zero attempt count
	ucProgrammingStatus = 0;
	ucProgrammingAttempts = 0;

	//Build the command specific headers ahead of time
	vSP_BSL_CreateHeaders();

	// Start up the SP and load the BSL password
	if ((ucSP_Start(ucSPIndex) == COMM_OK) && (ucSP_FetchMsgVersion(ucSPIndex) >= 120)) {

		vSP_BSL_LoadPwrd(ucSPIndex);

		vSP_TurnOff(ucSPIndex);
	}
	// If the SP board firmware version predates the addition of the Flag byte then use old protocol
	else if (ucSP_Start_Old(ucSPIndex) == COMM_OK){

		vSP_BSL_LoadPwrd_Old(ucSPIndex);

		vSP_TurnOff(ucSPIndex);
	}
	// No SP board found
	else{
		return;
	}

	ucSP_UARTInit(ucSPIndex);

	ucSP_UARTSetCommState(BAUD_9600, ucSPIndex);

	vSP_TurnonAll();

	while (ucProgrammingStatus == 0 && ucProgrammingAttempts++ < 10) {

		// Maybe it'll work
		ucProgrammingStatus = 1;

		ucSP_BSL_Init(ucSPIndex);

		if (ucSP_BSL_Send_PW() != SUCCESSCODE) {
			puts("BSL Password Failed");
			ucProgrammingStatus = 0;
		}

		puts("Erasing main...");
		if (ucProgrammingStatus == 1) {
			if (ucSP_BSL_Erase_Multiple_Seg(0xC000, 0x0000FF00L, 0) != SUCCESSCODE) {
				puts("Erasure Failed");
				ucProgrammingStatus = 0;
			}
		}

		if (ucProgrammingStatus == 1) {
			puts("Reprogramming...");
			for (uiBlockCounter = 0; uiBlockCounter < ulOTA_GetSDCardAddress(); uiBlockCounter += 2) {

				ucProgrammingStatus = ucOTA_WriteBlockToSP(uiBlockCounter);
			}
			if (ucProgrammingStatus == 1) break;
		}
	}
	printf("Programming complete! Attempts: %d\r\n", ucProgrammingAttempts);
}

void vOTA(void){
	vOTA_init();
	vOTA_UpdateOverSerial();
	vOTA_ReprogramSP();

	vPMM_Reset();
}
