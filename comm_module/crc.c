

/**************************  CRC.C  ******************************************
*
* Example Table Driven CRC16 Routine using 4-bit message chunks
*
* V1.01 10/07/2002 wzr
*		Modified from the original form into a package for the wizard project.
*
* V1.00  By Ashley Roll.  Digital Nemesis Pty Ltd
* www.digitalnemesis.com, ash@digitalnemesis.com
*
* The following is an example of implementing a restricted size CRC16 tbl
* lookup. No optimisation has been done so the code is clear & easy to
* understand.
*
* Test Vector: "123456789" (char str, no quotes) = CRC: 0x29B1
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


#include "diag.h"
#include "std.h"				//standard defines
#include "hal/config.h"		//system configuration definitions
#include "crc.h"				//crc calculator
#include "serial.h"				//comm port handlers
#include "comm.h"				//msg definitions

/* CRC16 "REGISTER" (IMPLEMENTED AS TWO 8BIT VALUES) */
#define CRC16_HI 0					// index into ucaX1FLD[]
#define CRC16_LO 1					// same



/* CRC16 LOOKUP TABLES (HI & LO BYTES) FOR 4 BITS PER ITERATION. */
const unsigned char ucCRC16_lookupHI[16] =
		{
        0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
        0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1
		};

const unsigned char ucCRC16_lookupLO[16] =
		{
        0x00, 0x21, 0x42, 0x63, 0x84, 0xA5, 0xC6, 0xE7,
        0x08, 0x29, 0x4A, 0x6B, 0x8C, 0xAD, 0xCE, 0xEF
		};




/*********************  vCRC16_updateNibble()  *********************************
*
* Compute the CRC from a nibble of the msg. (Nibble is low order nibble).
*
*******************************************************************************/

void vCRC16_updateNibble(
		unsigned char ucNibble,			//nibble to add to CRC
		unsigned char ucCRCarray[2]		//CRC current value
		)
	{
	unsigned char ucTmp;

	/* EXTRACT THE MOST SIGNIFICANT 4 BITS OF THE CRC REG */
	ucTmp = ucCRCarray[CRC16_HI] >> 4;

	/* XOR IN THE MESSAGE DATA INTO THE EXTRACTED BITS */
	ucTmp = ucTmp ^ ucNibble;

	/* SHIFT THE CRC REG LEFT 4 BITS */
	ucCRCarray[CRC16_HI] = (ucCRCarray[CRC16_HI] << 4) | (ucCRCarray[CRC16_LO] >> 4);	//lint !e564 !e734
	ucCRCarray[CRC16_LO] =  ucCRCarray[CRC16_LO] << 4;	//lint !e734

	/* DO THE TABLE LOOKUPS AND XOR THE RESULT INTO THE CRC TABLES */
	ucCRCarray[CRC16_HI] = ucCRCarray[CRC16_HI] ^ ucCRC16_lookupHI[ucTmp];
	ucCRCarray[CRC16_LO] = ucCRCarray[CRC16_LO] ^ ucCRC16_lookupLO[ucTmp];

	return;

	}/* END: vCRC16_updateNibble() */






/***********************  vCRC16_updateByte()  *************************************
*
* compute the crc for a full msg byte.
*
*******************************************************************************/

void vCRC16_updateByte(
		unsigned char ucByteVal,		//byte to add to CRC
		unsigned char ucCRCarray[2]		//CRC current value
		)
	{

	vCRC16_updateNibble(ucByteVal >> 4, ucCRCarray );	// Hi nibble first
	vCRC16_updateNibble(ucByteVal & 0x0F, ucCRCarray );	// LO nibble

	return;

	}/* END: vCRC16_updateByte() */









/***********************  ucCRC16_compute_msg_CRC()  ****************************
*
* This routine calculates the CRC for both the send msg and the receive msg.
*
* NOTE: The msg EOM_index is a pointer to the last used location in the msg.
*
* NOTE:	For a msg that you are sending you must leave an extra 2 bytes for
*		the CRC, and it MUST BE (THAT'S MUST BE) included in the size,
*		that is passed into this routine.
*
* NOTE: This set of CRC routines is hardwired (for speed) to only calculate
*		the CRC in the msg buffer.  We have not implemented a generalized
*		CRC routine here.
*
* ucMsgFlag = CRC_FOR_MSG_TO_SEND for a message to send.
* ucMsgFlag = CRC_FOR_MSG_TO_REC  for a message just received.
*
*
* IF FLAG = SEND-MSG THEN:
*		0.	msg_end_byte is expected to be stuffed with size-1 (including CRC)
*		1a. This routine returns TRUE (1) as the default return.
*		1b. This routine returns FALSE(0) if there is a msg buffer overrun.
*		1c. This routine returns FALSE(0) if there is a msg buffer underrun.
*		2.  The CRC is calculated with two extra bytes of zeros anticipating
*			the CRC check calculation.
*		3.  The calculated CRC is stuffed onto the end of the msg
*		    at locations MsgEnd-1 and MsgEnd-2.
*
* IF FLAG = RECEIVE-MSG THEN:
*		0.	The msg end byte is expected to be stuffed with size-1 (including CRC)
*		1a. This routine returns FALSE(0) if there is a msg buffer underrun.
*		1b. This routine returns TRUE (1) if the new CRC == received CRC.
*		1c. This routine returns FALSE (0) if the new CRC does not match.
*		2.  The CRC is calcuated including the old CRC so a compare to
*			zro is done.
*
*
* RET:	1 = CRC is OK
*		0 = CRC mismatch
*
******************************************************************************/

unsigned char ucCRC16_compute_msg_CRC(		/* RET:	1=CRC is OK, 0=CRC mismatch */
		unsigned char ucMsgFlag,	//send msg or receive msg flag
		volatile uchar *ucMSGBuff, 				//pointer to the message
		uchar ucLength						//length of the message
		)
	{
	unsigned char uc;
	unsigned char ucLimit;			//max counter limit
	unsigned char ucCRCarray[2];	//CRC current value

	ucLimit = ucLength - 1;

	/* RANGE CHECK THE LIMIT */
	if(ucLimit < 2)
		return(0);	//bad return
	if(ucLimit > MAX_OM_MSG_LENGTH)
		return(0);	//bad return)

	/* INIT THE CRC TO 0XFFFF AS PER CCITT SPEC */
	ucCRCarray[CRC16_HI] = 0xFF;
	ucCRCarray[CRC16_LO] = 0xFF;

	/* BACKUP THE MSG SIZE IDX IF ITS A SEND MSG */
	if(ucMsgFlag == CRC_FOR_MSG_TO_SEND) ucLimit -=2;

	/* CALCULATE THE CRC */
	for(uc=0; uc<=ucLimit;  uc++)
		{
		vCRC16_updateByte(*ucMSGBuff++, ucCRCarray);
		}

	/* IF THIS IS A SEND MSG THEN CALCULATE FOR 2 EXTRA BYTES & STUFF THE MSG */
	if(ucMsgFlag == CRC_FOR_MSG_TO_SEND)
		{
		*ucMSGBuff++ = ucCRCarray[CRC16_HI];
		*ucMSGBuff++ = ucCRCarray[CRC16_LO];
		return(1);	//good return
		}

	/* IF THIS IS A RECEIVE MSG DO THE COMPARE AND RET THE ERROR FLAG */
	if((!ucCRCarray[CRC16_HI]) && (!ucCRCarray[CRC16_LO]))
		{
		return(1);	//good return
		}

	#if 0
	/* IT WAS A BAD CRC COMPARE -- RETURN AN ERROR */
	vSERIAL_sout("(BdCrc=", 7);
	vSERIAL_HB8out(ucCRCarray[CRC16_HI]);
	vSERIAL_HB8out(ucCRCarray[CRC16_LO]);
	vSERIAL_sout(")\r\n", 3);
	#endif

	return(0);	//bad return

	}/* END: ucCRC16_compute_msg_CRC() */


////////////////////////////////////////////////////////////////////////
//! \fn uiCRC16_ComputeBlockCRC
//! \brief Computes CRC of a block of bytes
//! \return uiCRC
////////////////////////////////////////////////////////////////////////
unsigned int uiCRC16_ComputeBlockCRC(uchar *ucPointer, ulong ulLength)
	{
	unsigned long ulByteCount;
	unsigned char ucCRCarray[2];	//CRC current value
	uint uiCRC;

	/* INIT THE CRC TO 0XFFFF AS PER CCITT SPEC */
	ucCRCarray[CRC16_HI] = 0xFF;
	ucCRCarray[CRC16_LO] = 0xFF;

	/* CALCULATE THE CRC */
	for(ulByteCount=0; ulByteCount < ulLength;  ulByteCount++)
		{
		vCRC16_updateByte(*ucPointer++, ucCRCarray);
		}

	uiCRC = (uint)(ucCRCarray[CRC16_HI] << 8) | (ucCRCarray[CRC16_LO]);

	return(uiCRC);

	}/* END:uiCRC16_CRC_on_memory() */

////////////////////////////////////////////////////////////////////////
//! \fn uiCRC16_ComputeBlockCRC
//! \brief Computes CRC of a block of bytes with a starting CRC
//! \return uiCRC
////////////////////////////////////////////////////////////////////////
unsigned int uiCRC16_ComputeCRCwithInit(uchar *ucPointer, ulong ulLength, uint uiInitialCRC)
	{
	unsigned long ulByteCount;
	unsigned char ucCRCarray[2];	//CRC current value
	uint uiCRC;

	// INIT THE CRC
	ucCRCarray[CRC16_LO] = (uchar) uiInitialCRC;
	ucCRCarray[CRC16_HI] = (uchar) (uiInitialCRC >> 8);

	/* CALCULATE THE CRC */
	for(ulByteCount=0; ulByteCount < ulLength;  ulByteCount++)
		{
		vCRC16_updateByte(*ucPointer++, ucCRCarray);
		}

	uiCRC = (uint)(ucCRCarray[CRC16_HI] << 8) | (ucCRCarray[CRC16_LO]);

	return(uiCRC);

	}/* END:uiCRC16_CRC_on_memory() */




unsigned int uiCRC16_CRC_on_memory(ulong *ulPointer, ulong ulLength)
	{
	unsigned long ulByteCount;
	unsigned char ucCRCarray[2];	//CRC current value
	uint uiReturn;

	/* INIT THE CRC TO 0XFFFF AS PER CCITT SPEC */
	ucCRCarray[CRC16_HI] = 0xFF;
	ucCRCarray[CRC16_LO] = 0xFF;

	/* CALCULATE THE CRC */
	for(ulByteCount=0; ulByteCount<=ulLength;  ulByteCount++)
		{
		vCRC16_updateByte(*ulPointer++, ucCRCarray);
		}

	uiReturn = (uint)(ucCRCarray[CRC16_HI] << 8) | (ucCRCarray[CRC16_LO]);

	return(uiReturn);

	}/* END:uiCRC16_CRC_on_memory() */


/* --------------------------  END of MODULE  ------------------------------- */
