////////////////////////////////////////////////////////////////////////
//!	\file flash.c
//! \addtogroup HAL
//! @{
//!	\brief Used to read and write to flash memory
//!
//! The procedure for reading and writing to flash is as follows:
//! 1. Read the segment being written to into a local variable
//! 2. Edit the local copy of the segment
//! 3. Erase the segment in flash
//! 4. Write local copy of the segment back to flash
//!
////////////////////////////////////////////////////////////////////////

#include <msp430.h>
#include "std.h"
#include "flash_mcu.h"


////////////////////////// vFlash_Erase_Seg() ////////////////////////////////////
//! \brief Erases a segment in Flash
//!
//!
//! \param unAddress
//! \return none
//////////////////////////////////////////////////////////////////////////
void vFlash_Erase_Segment(uint32 ulAddress)
{
	uint16 *unFlashPtr;

	//Set the erase bit
	FCTL1 = FWKEY + ERASE;

	// Unlock A
	if(ulAddress == FLASH_INFO_A){
		FCTL3 = FWKEY | LOCKA;
	}

	//initialize the flash pointer to point to the given address
	unFlashPtr = (uint16 *) ulAddress;

	// Erase Flash
	while (BUSY & FCTL3); // Check if Flash being used
	FCTL3 = FWKEY; // Clear Lock bit
	FCTL1 = FWKEY + ERASE; // Set Erase bit
	*unFlashPtr = 0; // Dummy write to erase Flash seg
	while (BUSY & FCTL3); // Check if Erase is done

	// Lock A
	if(ulAddress == FLASH_INFO_A){
		FCTL3 = FWKEY | LOCKA;
	}
}

///////////////////////////////////////////////////////////////////////////
//! \fn ucFlash_Write_B32
//! \brief Writes several 32 bit values starting at the provided address
//! \param ulAddress
//! \param ulData
//! \param ulLength
//! \return 0 success, 1 fail
//////////////////////////////////////////////////////////////////////////
uchar ucFlash_Write_B32(ulong * ulData, ulong ulAddress, ulong ulLength)
{
	ulong *ulFlashPtr;
	uint uiIndex;

	//Restrict the length to 1 sector
	if(ulLength > 512)
		return 1;

	//initialize the flash pointer to point to the given address
	ulFlashPtr = (ulong *)ulAddress;

	//clear the lock bits
	FCTL3 = FWKEY;

	//set the write bit
	FCTL1 = FWKEY + BLKWRT;

	// Loop through the segment and write the data
	for (uiIndex = 0; uiIndex < ulLength; uiIndex++){
		//wait statements prevent writing to flash while module is busy
		while (!(FCTL3 & WAIT));
		*ulFlashPtr++ = *ulData++;
	}

	//clear the write bit
	FCTL1 = FWKEY;
	//set the lock bit
	FCTL3 = FWKEY + LOCK;

	return 0;
} //END: vFlash_Write_B32()

///////////////////////////////////////////////////////////////////////////
//! \fn vFlash_Write_Segment
//! \brief Writes a segment of memory specified by the address.
//! \param ulAddress
//! \param ulData
//! \return success
//////////////////////////////////////////////////////////////////////////
void vFlash_Write_Segment(ulong * ulData, ulong ulAddress)
{
	ulong *ulFlashPtr;
	uint uiIndex;
	uint uiSegmentLength;

	// If the address is in information memory then the segment length is 128 otherwise the segment length is
	if(ulAddress == FLASH_INFO_A || ulAddress == FLASH_INFO_B || ulAddress == FLASH_INFO_C || ulAddress == FLASH_INFO_D)
		uiSegmentLength = 128;
	else
		uiSegmentLength = 512;

	// We are writing 16-bit integers
	uiSegmentLength /= 4;

	//initialize the flash pointer to point to the given address
	ulFlashPtr = (ulong *)ulAddress;

	// Erase the segment
	vFlash_Erase_Segment(ulAddress);

	//set the write bit
	FCTL1 = FWKEY + BLKWRT;

	// Unlock sector A
	if(ulAddress == FLASH_INFO_A){
		FCTL3 = FWKEY | LOCKA;
	}

	// Loop through the segment and write the data
	for (uiIndex = 0; uiIndex < uiSegmentLength; uiIndex++){
		//wait statements prevent writing to flash while module is busy
		while (!(FCTL3 & WAIT));
		*ulFlashPtr++ = *ulData++;
	}

	// Lock sector A
	if(ulAddress == FLASH_INFO_A){
		FCTL3 = FWKEY | LOCKA;
	}

	//clear the write bit
	FCTL1 = FWKEY;
	//set the lock bit
	FCTL3 = FWKEY + LOCK;

} //END: ucFlash_Write_Segment()


///////////////////////////////////////////////////////////////////////////
//! \fn vFlash_Read_B32
//! \brief Reads several 32-bit values from flash at the provided address
//! \param ulAddress
//! \param ulLength
//! \param  ulData
/////////////////////////////////////////////////////////////////////////////
void vFlash_Read_B32(ulong * ulData, ulong ulAddress, ulong ulLength)
{
	ulong *ulFlashPtr;
	uint uiIndex;

	//Restrict the length to 1 sector
	if(ulLength > 512)
		return;

	//initialize the flash pointer to point to the given address
	ulFlashPtr = (ulong *) ulAddress;

	//clear the lock bits
	FCTL3 = FWKEY;

	// Loop through the segment and read out the data
	for (uiIndex = 0; uiIndex < ulLength; uiIndex++) {
		//wait statements prevent writing to flash while module is busy
		while (!(FCTL3 & WAIT));
		*ulData++ = *ulFlashPtr++;
	}

	//clear the write bit
	FCTL1 = FWKEY;
	//set the lock bit
	FCTL3 = FWKEY + LOCK;

} //END: vFlash_Read_B32()

///////////////////////////////////////////////////////////////////////////
//! \fn vFlash_Read_Segment
//! \brief Reads a segment from flash at the provided address
//! \param ulAddress
//! \return ulData
/////////////////////////////////////////////////////////////////////////////
void vFlash_Read_Segment(ulong * ulData, ulong ulAddress)
{
	ulong *ulFlashPtr;
	uint uiIndex;
	uint uiSegmentLength;

	// If the address is in information memory then the segment length is 128 otherwise the segment length is
	if(ulAddress == FLASH_INFO_A || ulAddress == FLASH_INFO_B || ulAddress == FLASH_INFO_C || ulAddress == FLASH_INFO_D)
		uiSegmentLength = 128;
	else
		uiSegmentLength = 512;

	// We are writing 16-bit integers
	uiSegmentLength /= 4;

	//initialize the flash pointer to point to the given address
	ulFlashPtr = (ulong *) ulAddress;

	//clear the lock bits
	FCTL3 = FWKEY;

	// Loop through the segment and read out the data
	for (uiIndex = 0; uiIndex < uiSegmentLength; uiIndex++) {
		//wait statements prevent writing to flash while module is busy
		while (!(FCTL3 & WAIT));
		*ulData++ = *ulFlashPtr++;
	}

	//clear the write bit
	FCTL1 = FWKEY;
	//set the lock bit
	FCTL3 = FWKEY + LOCK;

} //END: vFlash_Read_Segment()

////////////////////////////////////////////////////////////////////////////////
//! \fn vFlash_SetHID
//! \brief Sets the hardware ID in flash.  The hardware ID is unique for every board and
//! is set before deployment.
//!
//! \param ucHID
//! \return node
////////////////////////////////////////////////////////////////////////////////
void vFlash_SetHID(uint *uiHID)
{
	ulong ulSegmentData[32];
	uint uiIndex;
	ulong ulTemp;

	vFlash_Read_Segment(ulSegmentData, HID_SECTOR);

	// Write the HID to the local variable
	for(uiIndex = HID_ADDRESS; uiIndex < (HID_ADDRESS + 2); uiIndex++)
	{
		ulTemp = *uiHID++;
		ulTemp = ulTemp << 16;
		ulTemp |= *uiHID++;
		ulSegmentData[uiIndex] = ulTemp;
	}

	// Write the variable to flash
	vFlash_Write_Segment(ulSegmentData, HID_SECTOR);

}


////////////////////////////////////////////////////////////////////////////////
//! \fn vFlash_GetHID
//! \brief Reads the hardware ID from flash.
//!
//! \param *uiHID
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vFlash_GetHID(uint *uiHID)
{
	ulong ulSegmentData[32];
	uint uiIndex;

	vFlash_Read_Segment(ulSegmentData, HID_SECTOR);

	// Read the HID from the segment
	for(uiIndex = HID_ADDRESS; uiIndex < (HID_ADDRESS + 2); uiIndex++)
	{
		*uiHID++ = (uint)(ulSegmentData[uiIndex] >> 16);
		*uiHID++ = (uint)ulSegmentData[uiIndex];
	}
}

////////////////////////////////////////////////////////////////////////////////
//! \fn vFlash_SetRadioHID
//! \brief Sets the radio board's hardware ID in flash.  The hardware ID is unique for every board and
//! is set before deployment.
//!
//! \param ucHID
//! \return node
////////////////////////////////////////////////////////////////////////////////
void vFlash_SetRadioHID(uint *uiHID)
{
	ulong ulSegmentData[32];
	uint uiIndex;
	ulong ulTemp;

	vFlash_Read_Segment(ulSegmentData, HID_SECTOR);

	// Write the HID to the local variable
	for(uiIndex = RADIO_HID_ADDRESS; uiIndex < (RADIO_HID_ADDRESS + 2); uiIndex++)
	{
		ulTemp = *uiHID++;
		ulTemp = ulTemp << 16;
		ulTemp |= *uiHID++;
		ulSegmentData[uiIndex] = ulTemp;
	}

	// Write the variable to flash
	vFlash_Write_Segment(ulSegmentData, HID_SECTOR);

}


////////////////////////////////////////////////////////////////////////////////
//! \fn vFlash_GetRadioHID
//! \brief Reads the radio board's hardware ID from flash.
//!
//! \param *uiHID
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vFlash_GetRadioHID(uint *uiHID)
{
	ulong ulSegmentData[32];
	uint uiIndex;

	vFlash_Read_Segment(ulSegmentData, HID_SECTOR);

	// Read the HID from the segment
	for(uiIndex = RADIO_HID_ADDRESS; uiIndex < (RADIO_HID_ADDRESS + 2); uiIndex++)
	{
		*uiHID++ = (uint)(ulSegmentData[uiIndex] >> 16);
		*uiHID++ = (uint)ulSegmentData[uiIndex];
	}
}

////////////////////////////////////////////////////////////////////////////////
//! \fn vFlash_SetImageCRC
//! \brief
//!
//! \param ucHID
//! \return node
////////////////////////////////////////////////////////////////////////////////
void vFlash_SetImageCRC(uint uiCRC)
{
	ulong ulSegmentData[32];

	vFlash_Read_Segment(ulSegmentData, FLASH_INFO_A);

	ulSegmentData[CRC_ADDRESS] = uiCRC;

	// Write the variable to flash
	vFlash_Write_Segment(ulSegmentData, FLASH_INFO_A);
}


////////////////////////////////////////////////////////////////////////////////
//! \fn vFlash_GetHID
//! \brief Reads the hardware ID from flash.
//!
//! \param *uiHID
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vFlash_GetImageCRC(uint *uiCRC)
{
	ulong ulSegmentData[32];

	vFlash_Read_Segment(ulSegmentData, FLASH_INFO_A);

	*uiCRC = (uint)ulSegmentData[CRC_ADDRESS];
}


//! @}
