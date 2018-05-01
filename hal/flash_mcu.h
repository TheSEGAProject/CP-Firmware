///////////////////////////////////////////////////////////////////////////////
//! \file flash.h
//! \brief Header file for the flash module
//!
//!
#ifndef FLASH_H_
#define FLASH_H_

//! \def FLASH_INFO_A
//! \brief The address of sector A of information memory
#define FLASH_INFO_A	0x00001980

//! \def FLASH_INFO_B
//! \brief The address of sector B of information memory
#define FLASH_INFO_B	0x00001900

//! \def FLASH_INFO_C
//! \brief The address of sector C of information memory
#define FLASH_INFO_C	0x00001880

//! \def FLASH_INFO_D
//! \brief The address of sector D of information memory
#define FLASH_INFO_D	0x00001800

//! \def HID_SECTOR
//! \brief Address of the hardware ID number
#define HID_SECTOR FLASH_INFO_A

//! \def HID_ADDRESS
//! \brief Location within the HID sector that the first byte of the HID is stored
#define HID_ADDRESS		0

//! \def RADIO_HID_ADDRESS
//! \brief Address of the radio HID within
#define RADIO_HID_ADDRESS	2

//! \def CRC_ADDRESS
//! \brief Location within the sector that the first byte of the image CRC is stored
#define CRC_ADDRESS		4

void vFlash_SetRadioHID(uint *uiHID);
void vFlash_GetRadioHID(uint *uiHID);
void vFlash_SetHID(unsigned int *uiHID);
void vFlash_GetHID(unsigned int *uiHID);
void vFlash_SetImageCRC(uint uiCRC);
void vFlash_GetImageCRC(uint *uiCRC);
void vFlash_Read_B32(ulong * ulData, ulong ulAddress, ulong ulLength);
uchar ucFlash_Write_B32(ulong * ulData, ulong ulAddress, ulong ulLength);
void vFlash_Write_Segment(ulong * ulData, ulong ulAddress);
void vFlash_Read_Segment(ulong * ulData, ulong ulAddress);
void vFlash_Erase_Segment(uint32 unAddress);
#endif /* FLASH_H_ */
