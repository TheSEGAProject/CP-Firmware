/*
 * OTA.h
 *
 *  Created on: Jul 22, 2016
 *      Author: cp397
 */

#ifndef OTA_H_
#define OTA_H_

//! \struct S_Metadata_fields
//! \brief Structure used to store meta data regarding a firmware update
//! Some information is included in both the metadata packet and each program code packet
//! the redundancy is provided to help reduce errors.
struct S_Metadata_fields{
		uint8 m_ucSemaphore;			//!< Used to restrain access to this structure
		uint8 m_ucBoardNum; 			//!< The processor to be reprogrammed
		uint8 m_ucBoardType; 			//!< The board type being reprogrammed
		uint8 m_ucProgID; 				//!< The unique ID of the programming update
		uint16 m_uiCRC; 					//!< The CRC for the incoming block of code
		uint16 m_uiLength; 				//!< The number of bytes in this block
		uint32 m_ulStartAddr; 		//!< The start address of the incoming block of code
};

//! \def  METADATA_STRUCT_SIZE
//! \brief The size of a data message (in bytes).
#define METADATA_STRUCT_SIZE sizeof(struct S_Metadata_fields)

//! \brief This is the union used to work with data messages
//!
//! This union allows us to work with the various fields in the data message
//! easily while also making it easy send and receive using byte oriented
//! communication methods
union U_Metadata
{
		uint8 ucByteStream[METADATA_STRUCT_SIZE];
		struct S_Metadata_fields fields;
};


void vOTA_ReceiveCodePacket(union DE_Code * ProgramCode);
void vOTA_UpdateOverSerial(void);
void vOTA_init(void);
void vOTA_ReprogramSP(void);
void vOTA(void);
#endif /* OTA_H_ */
