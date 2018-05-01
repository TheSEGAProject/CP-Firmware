/**************************  SERIAL.C  *******************************************
 *
 * Serial port handler
 *
 * V1.02 04/27/2004 wzr
 *		Moved this code to work on the PIC-8720
 *
 * V1.01 09/11/2002 wzr
 *		Repaired the receiver (bout) routine to correct for overrrun errors.
 *
 * V1.00 04/29/2002 wzr
 *		Started
 *
 ******************************************************************************/

/*lint -e526 *//* function not defined */
/*lint -e657 *//* Unusual (nonportable) anonymous struct or union */
/*lint -e714 *//* function not referenced */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
/*lint -e757 *//* global declarator not referenced */
/*lint -e758 *//* global union not referenced */
/*lint -e768 *//* global struct member not referenced */
/*lint -e768 *//* global struct member not referenced */

/********************  OPTIMIZATION DEFINITIONS  ******************************/

/****************************  INCLUDES  *************************************/

#include <msp430.h>		//
#include "std.h"
#include "diag.h"			//diagnostic defines
#include "config.h"			//system configuration definitions
#include "serial.h"			//serial port defines
#include "delay.h"			//delay routines
#include "SP.h"
#include "led.h"
#include "mem_mod.h"
#include "time.h"

#ifdef ESPORT_ENABLED		//defined in diag.h
#include "ESPORT.h"		//external serial port
#endif	/* END: INC_ESPORT */

/****************************  DEFINES  **************************************/

#define NUMSTR_ARRAY_SIZE 11

#define DEC_3_UNSIGNED_MASK			0UL
#define DEC_3_SIGN_MASK				0x00000004UL
#define DEC_3_NUMERIC_MASK			0x00000007UL

#define DEC_3_CHAR_COUNT			1
#define UNSIGNED_DEC_3_FLD_SIZE		DEC_3_CHAR_COUNT
#define SIGNED_DEC_3_FLD_SIZE		(DEC_3_CHAR_COUNT+1)

#define DEC_8_UNSIGNED_MASK			0UL
#define DEC_8_SIGN_MASK				0x00000080UL
#define DEC_8_NUMERIC_MASK			0x000000FFUL

#define DEC_8_CHAR_COUNT			3
#define UNSIGNED_DEC_8_FLD_SIZE		DEC_8_CHAR_COUNT
#define SIGNED_DEC_8_FLD_SIZE		(DEC_8_CHAR_COUNT+1)

#define DEC_16_UNSIGNED_MASK		0UL
#define DEC_16_SIGN_MASK			0x00008000UL
#define DEC_16_NUMERIC_MASK			0x0000FFFFUL

#define DEC_16_CHAR_COUNT 			5
#define UNSIGNED_DEC_16_FLD_SIZE	DEC_16_CHAR_COUNT
#define SIGNED_DEC_16_FLD_SIZE		(DEC_16_CHAR_COUNT+1)

#define DEC_24_UNSIGNED_MASK		0UL
#define DEC_24_SIGN_MASK			0x00800000UL
#define DEC_24_NUMERIC_MASK			0x00FFFFFFUL

#define DEC_24_CHAR_COUNT 			8
#define UNSIGNED_DEC_24_FLD_SIZE	DEC_24_CHAR_COUNT
#define SIGNED_DEC_24_FLD_SIZE		(DEC_24_CHAR_COUNT+1)

#define DEC_32_UNSIGNED_MASK		0UL
#define DEC_32_SIGN_MASK			0x80000000UL
#define DEC_32_NUMERIC_MASK			0xFFFFFFFFUL

#define DEC_32_CHAR_COUNT 			10
#define UNSIGNED_DEC_32_FLD_SIZE	DEC_32_CHAR_COUNT
#define SIGNED_DEC_32_FLD_SIZE		(DEC_32_CHAR_COUNT+1)

#define VARIABLE_FLD_SIZE			0

//! \def SERIAL_BUFF_SIZE
//! \brief The length of the serial comm buffer
#define SERIAL_BUFF_SIZE		MAX_MSG_SIZE

/********************************  GLOBALS  **********************************/

extern volatile union //ucFLAG0_BYTE
{
	uchar byte;
	struct
	{
		unsigned FLG0_BIGSUB_CARRY_BIT :1; //bit 0 ;1=CARRY, 0=NO-CARRY
		unsigned FLG0_BIGSUB_6_BYTE_Z_BIT :1; //bit 1 ;1=all diff 0, 0=otherwise
		unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT :1; //bit 2 ;1=top 4 bytes 0, 0=otherwise
		unsigned FLG0_REDIRECT_COMM_TO_ESPORT_BIT :1; //bit 3 ;1=REDIRECT, 0=COMM1
		unsigned FLG0_RESET_ALL_TIME_BIT :1; //bit 4 ;1=do time  reset, 0=dont
		//SET:	when RDC4 gets finds first
		//		SOM2.
		//		or
		//		In a Hub when it is reset.
		//
		//CLR: 	when vMAIN_computeDispatchTiming()
		//		runs next.
		unsigned FLG0_SERIAL_BINARY_MODE_BIT :1; //bit 5 1=binary mode, 0=text mode
		unsigned FLG0_HAVE_WIZ_GROUP_TIME_BIT :1; //bit 6 1=Wizard group time has
		//        been aquired from a DC4
		//      0=We are using startup time
		unsigned FLG0_NOTUSED7_BIT :1; //bit 7
	} FLAG0_STRUCT;
} ucFLAG0_BYTE;

extern volatile union
{
	uint8 byte;
	struct
	{
		unsigned FLG3_RADIO_ON_BIT :1;
		unsigned FLG3_RADIO_MODE_BIT :1;
		unsigned FLG3_RADIO_PROGRAMMED :1;
		unsigned FLG2_BUTTON_INT_BIT :1; //bit 2 ;1=XMIT, 0=RECEIVE
		unsigned FLG3_LINKSLOT_ALARM :1;
		unsigned FLG3_LPM_DELAY_ALARM :1;
		unsigned FLG3_KEY_PRESSED :1;
		unsigned FLG3_UNUSED_BIT7 :1;
	} FLAG3_STRUCT;
} ucFLAG3_BYTE;

#ifdef SERIAL_Q_OUTPUT_ENABLED
extern volatile uchar ucaCommQ[ COMM_Q_SIZE ];
#endif

extern volatile uchar ucQonIdx_LUL;
extern volatile uchar ucQoffIdx_LUL;
extern volatile uchar ucQcount;

extern uchar ucGLOB_lineCharPosition; //line position for computing tabs

uchar g_ucaUSCI_A1_RXBuffer[SERIAL_BUFF_SIZE];
uchar g_ucaUSCI_A1_RXBufferIndex;

/*************************  LOCAL DECLARATIONS  ******************************/

static void vSERIAL_generic_IVout(ulong ulSignMaskBit, //sign mask
    ulong ulNumericMask, //numeric mask
    char cDesiredFldSize, //desired output field size
    uchar ucLeadFillChar, //leading zero fill char
    long lVal //numeric value to show
    );

static void vSERIAL_B2D_Str(long lVal, //IN: value to convert
    ulong ulSignBitMask, //IN: bit on in Sign position (0 if none)
    ulong ulNumericMask, //IN: Mask of number including sign
    uchar ucaValArray[NUMSTR_ARRAY_SIZE] //OUT: ten digits and a leading sign
    );

static void vSERIAL_FormatStrout(char cDesiredFldSize, //fld size (digits+sign), 0 = don't care
    uchar ucLeaderChar, // ' ' or '0'
    uchar ucaValArray[NUMSTR_ARRAY_SIZE] //ten digits and a leading sign
    );

#ifdef SERIAL_Q_OUTPUT_ENABLED
static void vSERIAL_pushQ(
		uchar ucChar
);
#endif

/********************************  CODE  *************************************/

/***************************  vSERIAL_init()  *********************************
 *
 *  ported to hardware rev 1.1 using the UCA1 UART module
 ******************************************************************************/

void vSERIAL_init(void)
{

	/* SETUP Q EVEN IF WE DON'T USE IT */
	ucQcount = 0;
	ucQonIdx_LUL = 0;
	ucQoffIdx_LUL = 0;

	// Put state machine in reset
	UCA1CTL1 |= UCSWRST;

	//Source clock is SMCLK
	UCA1CTL1 |= UCSSEL_2;

	// SETUP THE BAUD RATE
	UCA1BR0 = UCA1BR0_VALUE; // Setup UCBR0
	UCA1BR1 = UCA1BR1_VALUE;
	UCA1MCTL |= (UCBRS0_VALUE * UCBRS0) + (UCBRF0_VALUE * UCBRF0); // Setup UCBRS0 and UCBRF0

	/* ENABLE THE LOW-SPEED/OVERSAMPLING BIT */
#if (UCOS16_BIT_VALUE == 1)
	UCA1MCTL |= UCOS16; //SET UCOS16
#else
	UCA1MCTL &= ~UCOS16; //CLR UCOS16
#endif

	// Only initialize the USCI if the USB cable is plugged in
	if (!(P2IN & BIT5))
	{
		// Initialize USCI state machine
		UCA1CTL1 &= ~UCSWRST;
		// TURN ON THE RECEIVER AND TRANSMITTER FOR THE COM PORT
		P5SEL |= (BIT6 | BIT7); // P5.6,7 = USCI_A1 TXD/RXD
		P2IES &= ~BIT5;
	}
	else
		P2IES |= BIT5;

	vDELAY_wait100usTic(5);

	// Enable serial rx interrupts
	UART1_REG_IE |= UART1_RX_IE;

	return;

}/* END: vSERIAL_init() */

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Cleans the serial comm. buffer
//!
//! \param none
//! \return none
//////////////////////////////////////////////////////////////////////////////
void vSERIAL_CleanBuffer(uchar * p_ucaBuff)
{
	uchar ucCounter;

	for (ucCounter = SERIAL_BUFF_SIZE; ucCounter > 0; --ucCounter)
	{
		*p_ucaBuff++ = 0;
	}

} // END: vSERIAL_CleanBuffer

////////////////////////////////////////////////////////////////////////////////
//!
//!	\brief Reads the contents of the serial comm. buffer into the desired buffer
//!
//!
//! \param none
//!	\return none
///////////////////////////////////////////////////////////////////////////////
void vSERIAL_ReadBuffer(volatile uchar * ucaBuff)
{
	uchar ucCounter;

	for (ucCounter = 0; ucCounter < MAX_MSG_SIZE; ucCounter++)
	{
		*ucaBuff++ = g_ucaUSCI_A1_RXBuffer[ucCounter];
	}

}

/***************************  vSERIAL_quit()  *********************************
 *
 *ported to hardware rev 1.1 using the UCA1 UART module
 ******************************************************************************/

void vSERIAL_quit(void)
{
	/* WAIT FOR COUNT TO ZERO */
	while (ucQcount);

	/* WAIT FOR TRANSMIT TO FINISH */
	while (!(UCA1IFG & UCTXIFG));

	vDELAY_wait100usTic(5);

	/* DISABLE THE SERIAL PORT */
	UCA1IE &= ~UCRXIE; // Disable USCI_A0 RX interrupt
	UCA1IE &= ~UCTXIE; // Disable USCI_A0 TX interrupt

	return;

}/* END: vSERIAL_quit() */

/************************  vSERIAL_setBinaryMode()  **************************
 *
 *
 *
 ******************************************************************************/

void vSERIAL_setBinaryMode(uchar ucBinModeFlg //BINARY_MODE_ON  or  BINARY_MODE_OFF
    )
{

	if (ucBinModeFlg)
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_SERIAL_BINARY_MODE_BIT = 1;
	else
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_SERIAL_BINARY_MODE_BIT = 0;

	return;

}/* END: vSERIAL_setBinaryMode() */

/************************  vSERIAL_bout()  *********************************
 *
 *
 *
 ******************************************************************************/

void vSERIAL_bout(uchar ucChar)
{
	uchar ucc;
	uchar uccMax;

#ifdef DEBUG
	if (!ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_SERIAL_BINARY_MODE_BIT)
	{

		if (ucChar == '\r')
			ucGLOB_lineCharPosition = 0;

		if (ucChar == '\t')
		{
			uccMax = 4 - (ucGLOB_lineCharPosition % 4);
			for (ucc = 0; ucc < uccMax; ucc++)
			{
				vSERIAL_bout('x');
			}
			return;
		}
	}

	if (0)
	{
#ifdef ESPORT_ENABLED				//defined in diag.h
//		vESPORT_bout(ucChar);
#endif
	}
	else
	{
#ifdef SERIAL_Q_OUTPUT_ENABLED
		vSERIAL_pushQ(ucChar); //go stuff the char
#else
		while (!(UCA1IFG & UCTXIFG));// Make sure previous transmission is complete
		UCA1TXBUF = ucChar; //stuff the xmit reg
		while ((UCA1STAT & UCBUSY));// Don't exit until finished

#endif
	}

	if (!ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_SERIAL_BINARY_MODE_BIT)
	{
		switch (ucChar)
		{
			case '\r':
			case '\n':
			case '\v':
			break;

			default:
				ucGLOB_lineCharPosition++;
			break;

		}/* END: switch() */

	}/* END: if() */
#endif

	return;

}/* END: vSERIAL_bout() */

/*************************  ucSERIAL_bin()  *********************************
 *
 *
 *
 ******************************************************************************/

uchar ucSERIAL_bin(void)
{
	uchar ucTmp;

//	UCA1IE &= ~UCRXIE;
	/* WAIT FOR A COMPLETED TRANSMISSION */
	while (!(UCA1IFG & UCRXIFG));
//	UCA1IE |= UCRXIE;

	/* READ THE CHAR */
	ucTmp = UCA1RXBUF;
	return (ucTmp);

}/* END: ucSERIAL_bin() */

/*****************  uiSERIAL_timedBin()  **************************************
 *
 * RET: >  255 = timeout
 *	   <= 255 = char
 *
 ******************************************************************************/

uint uiSERIAL_timedBin( // uiRET <= 255 is char, uiRET > 255 is Timeout
    uint uiReqTime_in_ms //Time limit in mS
    )
{
	uint uiChar;
	ulong ulCntLimit;
	ulong uli;

	/* CONVERT MSEC TO LOOP COUNT */
	ulCntLimit = 40UL * uiReqTime_in_ms;

	/* WAIT FOR A CHAR */
	for (uli = 0; uli < ulCntLimit; uli++)
	{
		if (ucSERIAL_kbhit())
		{
			uiChar = (uint) ucSERIAL_bin();
			return (uiChar);
		}

	}/* END: for() */

	return (0xFFFF);

}/* END: uiSERIAL_timedBin() */


/********************  vSERIAL_repeatChar()  *********************************
 *
 * Does a multiple repeat of any specified char
 *
 * NOTE: If count is < or == 0 then nothing is printed
 *
 ******************************************************************************/

void vSERIAL_repeatChar( //repeated output of a single char
    uchar ucChar, //char to output
    char cCount //number of times to output the char
    )
{
	char cii;
	
	cii = 0;
	while (cii < cCount)
	{
		vSERIAL_bout(ucChar);
		cii++;
	}/* END: while() */

	return;

}/* END: vSERIAL_repeatChar() */

/************************  vSERIAL_dash()  *********************************
 *
 * Print a row of dashes
 *
 * NOTE: If count is < or == 0 then nothing is printed
 *
 ******************************************************************************/

void vSERIAL_dash( //repeated output of dashes
    char cCount //count if <= 0 then nothing output
    )
{
	vSERIAL_repeatChar('-', cCount);
	return;

}/* END: vSERIAL_dash() */

/************************  vSERIAL_spaces()  *********************************
 *
 * Print a row of spacees
 *
 * NOTE: If count is < or == 0 then nothing is printed
 *
 ******************************************************************************/

void vSERIAL_spaces( //repeated output of spaces
    char cCount //count: if <= 0 then nothing output
    )
{

	vSERIAL_repeatChar(' ', cCount);
	return;

}/* END: vSERIAL_spaces() */

/*************************  ucSERIAL_kbhit()  ********************************
 *
 * check for a key hit on the serial port
 *
 *
 * ported to hardware rev 1.1 using the UCA1 UART module
 ******************************************************************************/

uchar ucSERIAL_kbhit(void)
{
	if (ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_KEY_PRESSED == 1)
	{
		ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_KEY_PRESSED = 0;
		return 1;
	}
	else
		return 0;

}/* END: ucSERIAL_kbhit() */

/************************  vSERIAL_crlf()  *********************************
 *
 * Carriage return line feed
 *
 ******************************************************************************/

void vSERIAL_crlf(void)
{
	vSERIAL_bout('\r');
	vSERIAL_bout('\n');
	return;

}/* END: vSERIAL_crlf() */

/************************  vSERIAL_Hformat()  *******************************
 *
 * Put out the Hex format leader "0x"
 *
 ******************************************************************************/

void vSERIAL_Hformat(void)
{
	vSERIAL_bout('0');
	vSERIAL_bout('x');
	return;

}/* END: vSERIAL_Hformat() */

/********************  ucSERIAL_toupper()  ***************************************
 *
 * convert the char passed to an uppercase char
 *
 ******************************************************************************/

uchar ucSERIAL_toupper(uchar ucChar)
{
	if ((ucChar >= 0x61) && (ucChar <= 0x7A))
		ucChar &= ~0x20;
	return (ucChar);

}/* END: ucSERIAL_toupper() */

/********************  ucSERIAL_isnum()  ***************************************
 *
 * Ret:	1= num
 *		0= not num
 *
 ******************************************************************************/

uchar ucSERIAL_isnum( //RET: 1=is a number, 0=not a number
    uchar ucChar //char to check
    )
{
	if ((ucChar >= '0') && (ucChar <= '9'))
		return (1);
	return (0);

}/* END: ucSERIAL_isnum() */

/************************  vSERIAL_rom_sout()  *******************************
 *
 * String Output from ROM string
 *
 ******************************************************************************/

//void vSERIAL_rom_sout(const char *cStrPtr)
//{
//	while (*cStrPtr)
//	{
//		vSERIAL_bout((uchar) *cStrPtr++);
//	}
//	return;
//
//}/* END: vSERIAL_rom_sout() */

////////////////////////////////////////////////////////////////////////////////
//! \fn vSERIAL_sout
//! \brief Prints a string to the terminal
//!	\param *cStrPtr, pointer to the string to be printed
//!	\param uiLength, the length of the string
////////////////////////////////////////////////////////////////////////////////
void vSERIAL_sout(char *cStrPtr, uint uiLength)
{
	uint uiByteCount;

	for(uiByteCount= 0; uiByteCount < uiLength; uiByteCount++)
	{
		vSERIAL_bout((uchar) *cStrPtr++);
	}
	return;

}/* END: vSERIAL_sout() */

/************************  vSERIAL_padded_rom_sout()  *******************************
 *
 * Padded String Output from ROM string, Space padding is either on left if
 * pad count is (-) or is on the right if pad count is (+).
 *
 * NOTE: If pad count is less than string length then no padding is used.
 *
 * NOTE: If pad count is 0 then no padding is used.
 *
 ******************************************************************************/

void vSERIAL_padded_sout(T_Text *S_StrPtr, signed char cPadCount //number of pad spaces (-=left, +=Right)
    )
{
	signed char cPosPadCount;
	char cLenDiff;

	cPosPadCount = cPadCount;
	if (cPosPadCount < 0)
	{
		cPosPadCount = -1 * cPosPadCount;
	}
	
	cLenDiff = cPosPadCount - S_StrPtr->m_uiLength;

	if (cPadCount < 0)
	{
		vSERIAL_spaces(cLenDiff);
	}

	vSERIAL_sout(S_StrPtr->m_cText, S_StrPtr->m_uiLength);

	if (cPadCount > 0)
	{
		vSERIAL_spaces(cLenDiff);

	}/* END: if() */

	return;

}/* END: vSERIAL_padded_rom_sout() */

/************************  cSERIAL_rom_strlen()  ****************************
 *
 * Find string length of a rom string.
 *
 * RET: the length of the string (not including the 0 terminator).
 *
 ******************************************************************************/

char cSERIAL_rom_strlen( //return the length of a rom string
    const char *cStrPtr)
{
	char cStrLen;

	cStrLen = 0;
	while (*cStrPtr++)
	{
		cStrLen++;
	}
	return (cStrLen);

}/* END: cSERIAL_rom_strlen() */

/*******************  vSERIAL_any_char_to_cont()  ****************************
 *
 *
 *
 ******************************************************************************/

void vSERIAL_any_char_to_cont(void)
{

	vSERIAL_sout("AnyChrToCont..", 14);
	ucSERIAL_bin();
	vSERIAL_crlf();
	return;

}/* END: vSERIAL_any_char_to_cont() */

/*******************  vSERIAL_showXXXmsg()  ********************************
 *
 *
 *
 ******************************************************************************/

void vSERIAL_showXXXmsg( //SHOWS: " XXX\r\n" 
    void)
{

	vSERIAL_sout(" XXX\r\n", 6);
	return;

}/* END: vSERIAL_showXXXmsg() */

/*******************  ucSERIAL_confirm()  ************************************
 *
 * RET:	1 = confirmed
 *		0 = not confirmed
 *
 ******************************************************************************/

uchar ucSERIAL_confirm( //RET: 1=YES CONFIRMED, 0=NO NOT CONFIRMED
    uchar ucAbortTextingFlag //YES_SHOW_ABORT_MSG or NO_SHOW_ABORT_MSG
    )
{
	uchar ucChar;

	vSERIAL_sout("[HitEntrToCnfrm] ", 17);
	ucChar = ucSERIAL_bin();
	if ((ucChar != '\r') && (ucChar != '\n'))
	{
		if (ucAbortTextingFlag)
		{
			vSERIAL_showXXXmsg();
		}
		return (0);
	}
	
	vSERIAL_crlf();
	return (1);

}/* END: ucSERIAL_confirm() */

#if 0

/* If'd out because it is not used */

/*******************  ucSERIAL_YorN()  ****************************************
 *
 * RET:	1 = YES
 *		0 = not YES
 *
 ******************************************************************************/

uchar ucSERIAL_YorN( //RET: 1=YES, 0=NOT YES
		void
)
{
	uchar ucChar;

	vSERIAL_sout("[YorN] ", 7);
	ucChar = ucSERIAL_bin();
	vSERIAL_crlf();
	if((ucChar != 'Y') && (ucChar != 'y')) return(1);
	return(0);

}/* END: ucSERIAL_YorN() */

#endif

/****************** ucSERIAL_readColTab() *********************************
 *
 * RET: cur col number (first col = 0)
 *
 ******************************************************************************/

uchar ucSERIAL_readColTab( //RET: cur col number (first col = 0)
    void)
{
	return (ucGLOB_lineCharPosition);

}/* END: ucSERIAL_readColTab() */

/*******************  vSERIAL_colTab()  *************************************
 *
 *
 *
 ******************************************************************************/

void vSERIAL_colTab( //Tab to a particular column (first col = 0)
    uchar ucColNum)
{
	uchar ucc;
	uchar uccMax;

	if (ucGLOB_lineCharPosition >= ucColNum)
		return;
	uccMax = ucColNum - ucGLOB_lineCharPosition;

	for (ucc = 0; ucc < uccMax; ucc++)
	{
		vSERIAL_bout(' ');
	}

	return;

}/* END: vSERIAL_colTab() */

/*************************  vSERIAL_HB4out()  **********************************
 *
 * SERIAL HEX NIBBLE OUT
 *
 *
 *
 ******************************************************************************/

void vSERIAL_HB4out(uchar ucByte)
{
	uchar uc;

	uc = (ucByte & 0x0F) + 0x30;
	if (uc >= 0x3A)
		uc += 7;

	vSERIAL_bout(uc);

	return;

}/* END: vSERIAL_HB4out() */

/*************************  vSERIAL_HB8out()  ********************************
 *
 *
 ******************************************************************************/

void vSERIAL_HB8out(uchar ucByte)
{
	vSERIAL_HB4out(ucByte >> 4);
	vSERIAL_HB4out(ucByte);

	return;

}/* END: vSERIAL_HB8out() */

/************************* vSERIAL_HB8Fout()  *******************************
 *
 * HEX formatted output
 *
 ****************************************************************************/

void vSERIAL_HB8Fout( //HEX formatted output (ie 0x55)
    uchar ucByte)
{

	vSERIAL_Hformat();
	vSERIAL_HB8out(ucByte);

	return;

}/* END: vSERIAL_HB8Fout() */

/*************************  vSERIAL_I8out()  ********************************
 *
 * Signed Integer 8-Bit output
 *
 ******************************************************************************/

void vSERIAL_I8out(char cInt)
{
	long lVal;

	lVal = (long) cInt;
	vSERIAL_generic_IVout(DEC_8_SIGN_MASK, //sign mask
	    DEC_8_NUMERIC_MASK, //numeric mask
	    SIGNED_DEC_8_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );

#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)cInt,DEC_8_SIGN_MASK,DEC_8_NUMERIC_MASK,ucaValArray);

	vSERIAL_FormatStrout(SIGNED_DEC_8_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_I8out() */

/*************************  vSERIAL_IV8out()  ********************************
 *
 * Signed Integer output
 *
 ******************************************************************************/

void vSERIAL_IV8out(char cVal)
{
	long lVal;

	lVal = (long) cVal;
	vSERIAL_generic_IVout(DEC_8_SIGN_MASK, //sign mask
	    DEC_8_NUMERIC_MASK, //numeric mask
	    VARIABLE_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );

#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)ucVal, DEC_8_SIGN_MASK, DEC_8_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif

	return;

}/* END: vSERIAL_IV8out() */

/*************************  vSERIAL_UI8out()  ********************************
 *
 * Unsigned Integer 8-Bit output
 *
 ******************************************************************************/

void vSERIAL_UI8out(uchar ucVal)
{
	long lVal;

	lVal = (long) ucVal;
	vSERIAL_generic_IVout(DEC_8_UNSIGNED_MASK, //sign mask
	    DEC_8_NUMERIC_MASK, //numeric mask
	    UNSIGNED_DEC_8_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );

	return;

}/* END: vSERIAL_UI8out() */

/*************************  vSERIAL_UI8_2char_out()  ********************************
 *
 * Unsigned Integer 8-Bit output
 *
 ******************************************************************************/

void vSERIAL_UI8_2char_out( //2 char fixed width
    uchar ucVal, //value
    uchar ucLeadFillChar //leading fill char
    )
{
	long lVal;

	lVal = (long) ucVal;
	vSERIAL_generic_IVout(DEC_8_UNSIGNED_MASK, //sign mask
	    DEC_8_NUMERIC_MASK, //numeric mask
	    2, //desired output fld size
	    ucLeadFillChar, //Lead fill char
	    lVal //numeric value
	    );

	return;

}/* END: vSERIAL_UI8_2char_out() */

/*************************  vSERIAL_UIV8out()  ********************************
 *
 * Unsigned Integer variable width 8-Bit output
 *
 *
 *
 ******************************************************************************/

void vSERIAL_UIV8out(uchar ucVal)
{
	long lVal;

	lVal = (long) ucVal;
	vSERIAL_generic_IVout(DEC_8_UNSIGNED_MASK, //sign mask
	    DEC_8_NUMERIC_MASK, //numeric mask
	    VARIABLE_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)ucVal, DEC_8_UNSIGNED_MASK, DEC_8_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_UIV8out() */

/*************************  vSERIAL_HB16out()  **********************************
 *
 * Hex Integer output
 *
 ******************************************************************************/

void vSERIAL_HB16out(unsigned int uiVal)
{
	vSERIAL_HB8out((uchar) (uiVal >> 8));
	vSERIAL_HB8out((uchar) uiVal);

	return;

}/* END: vSERIAL_HB16out() */

/*************************  vSERIAL_HBV16out()  ******************************
 *
 * Hex Variable output
 *
 ******************************************************************************/

void vSERIAL_HBV16out(uint uiVal)
{
	ulong ulVal;

	ulVal = uiVal;

	vSERIAL_HBV32out(ulVal);

	return;

}/* END: vSERIAL_HBV16out() */

/************************* vSERIAL_HB16Fout()  *******************************
 *
 * HEX formatted output
 *
 ****************************************************************************/

void vSERIAL_HB16Fout(unsigned int uiInt)
{

	vSERIAL_Hformat();
	vSERIAL_HB16out(uiInt);

	return;

}/* END: vSERIAL_HB16Fout() */

/*************************  vSERIAL_UI16out()  **********************************
 *
 * Unsigned Integer output
 *
 ******************************************************************************/

void vSERIAL_UI16out(unsigned int uiVal)
{
	long lVal;

	lVal = (long) uiVal;
	vSERIAL_generic_IVout(DEC_16_UNSIGNED_MASK, //sign mask
	    DEC_16_NUMERIC_MASK, //numeric mask
	    UNSIGNED_DEC_16_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );

#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)uiVal, DEC_16_UNSIGNED_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(UNSIGNED_DEC_16_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_UI16out() */

/*************************  vSERIAL_UIV16out()  **********************************
 *
 * Unsigned Integer variable field 16 bit output
 *
 *
 *
 ******************************************************************************/

void vSERIAL_UIV16out(unsigned int uiVal)
{
	long lVal;

	lVal = (long) uiVal;
	vSERIAL_generic_IVout(DEC_16_UNSIGNED_MASK, //sign mask
	    DEC_16_NUMERIC_MASK, //numeric mask
	    VARIABLE_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)uiVal, DEC_16_UNSIGNED_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_UIV16out() */

/*************************  vSERIAL_I16outt()  *******************************
 *
 * Signed Integer output
 *
 *
 *
 ******************************************************************************/

void vSERIAL_I16out(int iVal)
{
	long lVal;

	lVal = (long) iVal;
	vSERIAL_generic_IVout(DEC_16_SIGN_MASK, //sign mask
	    DEC_16_NUMERIC_MASK, //numeric mask
	    SIGNED_DEC_16_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)iVal, DEC_16_SIGN_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(SIGNED_DEC_16_FLD_SIZE, ' ', ucaValArray);
#endif

	return;

}/* END: vSERIAL_I16outFormat() */

/*************************  vSERIAL_IV16out()  ********************************
 *
 * Signed Integer output
 *
 *
 *
 ******************************************************************************/

void vSERIAL_IV16out(int iVal)
{
	long lVal;

	lVal = (long) iVal;
	vSERIAL_generic_IVout(DEC_16_SIGN_MASK, //sign mask
	    DEC_16_NUMERIC_MASK, //numeric mask
	    VARIABLE_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)iVal, DEC_16_SIGN_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_IV16out() */

/*************************  vSERIAL_I16outFormat()  ***************************
 *
 * Signed Integer output
 *
 ******************************************************************************/

void vSERIAL_I16outFormat(int iVal, char cFormatWidth)
{
	long lVal;

	lVal = (long) iVal;
	vSERIAL_generic_IVout(DEC_16_SIGN_MASK, //sign mask
	    DEC_16_NUMERIC_MASK, //numeric mask
	    cFormatWidth, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)iVal, DEC_16_SIGN_MASK, DEC_16_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(cFormatWidth, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_I16outFormat() */

/*************************  vSERIAL_HB24out()  ******************************
 *
 * Hex 24bit word output
 *
 ******************************************************************************/

void vSERIAL_HB24out(USL uslB24)
{

	vSERIAL_HB8out((uchar) (uslB24 >> 16));
	vSERIAL_HB8out((uchar) (uslB24 >> 8));
	vSERIAL_HB8out((uchar) uslB24);

	return;

}/* END: vSERIAL_HB24out() */

/*************************  vSERIAL_HBV24out()  ******************************
 *
 * Hex Variable output
 *
 ******************************************************************************/

void vSERIAL_HBV24out(usl uslVal)
{
	ulong ulVal;

	ulVal = uslVal;

	vSERIAL_HBV32out(ulVal);

	return;

}/* END: vSERIAL_HBV24out() */

/************************* vSERIAL_HB24Fout()  *******************************
 *
 * HEX formatted output
 *
 ****************************************************************************/

void vSERIAL_HB24Fout(USL uslB24)
{

	vSERIAL_Hformat();
	vSERIAL_HB24out(uslB24);

	return;

}/* END: vSERIAL_HB24Fout() */

/*************************  vSERIAL_UI24out()  ********************************
 *
 * UNSigned Integer 24-Bit output
 *
 ******************************************************************************/

void vSERIAL_UI24out(USL uslVal)
{
	long lVal;

	lVal = (long) uslVal;
	vSERIAL_generic_IVout(DEC_24_UNSIGNED_MASK, //sign mask
	    DEC_24_NUMERIC_MASK, //numeric mask
	    UNSIGNED_DEC_24_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
	return;

}/* END: vSERIAL_UI24out() */

/*************************  vSERIAL_UIV24out()  ********************************
 *
 * UNSigned Integer 24-Bit output
 *
 ******************************************************************************/

void vSERIAL_UIV24out(USL uslVal)
{
	long lVal;

	lVal = (long) uslVal;
	vSERIAL_generic_IVout(DEC_24_UNSIGNED_MASK, //sign mask
	    DEC_24_NUMERIC_MASK, //numeric mask
	    VARIABLE_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
	return;

}/* END: vSERIAL_UIV24out() */

/*************************  vSERIAL_HB32out()  **********************************
 *
 * Hex Long Integer output
 *
 ******************************************************************************/

void vSERIAL_HB32out(ulong ulLong)
{

	vSERIAL_HB16out((unsigned int) (ulLong >> 16));
	vSERIAL_HB16out((unsigned int) ulLong);

	return;

}/* END: vSERIAL_HB32out() */

/*************************  vSERIAL_HBV32out()  ******************************
 *
 * Hex Long Integer output
 *
 ******************************************************************************/

void vSERIAL_HBV32out(ulong ulLong)
{
	signed char cc;
	uchar ucFoundNonZro;
	uchar ucNumVal;

	ucFoundNonZro = 0;
	for (cc = 7; cc >= 0; cc--)
	{
		ucNumVal = (uchar) ((ulLong >> (4 * cc)) & 0xFL);
		if ((ucNumVal == 0) && (ucFoundNonZro == 0) && (cc != 0))
			continue;
		ucFoundNonZro = 1;
		vSERIAL_HB4out(ucNumVal);
	}
	return;

}/* END: vSERIAL_HBV32out() */

/************************* vSERIAL_HB32Fout()  *******************************
 *
 * HEX formatted output
 *
 ****************************************************************************/

void vSERIAL_HB32Fout(ulong ulLong)
{

	vSERIAL_Hformat();
	vSERIAL_HB32out(ulLong);

	return;

}/* END: vSERIAL_HB32Fout() */

/*************************  vSERIAL_UI32out()  ********************************
 *
 * UNSigned Integer 32-Bit output
 *
 ******************************************************************************/

void vSERIAL_UI32out(ulong ulVal)
{
	long lVal;

	lVal = (long) ulVal;
	vSERIAL_generic_IVout(DEC_32_UNSIGNED_MASK, //sign mask
	    DEC_32_NUMERIC_MASK, //numeric mask
	    UNSIGNED_DEC_32_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str((long)ulVal, DEC_32_UNSIGNED_MASK, DEC_32_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(UNSIGNED_DEC_32_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_UI32out() */

/*************************  vSERIAL_UIV32out()  ********************************
 *
 * Signed Integer 32-Bit output
 *
 ******************************************************************************/

void vSERIAL_UIV32out(ulong ulVal)
{
	long lVal;

	lVal = (long) ulVal;
	vSERIAL_generic_IVout(DEC_32_UNSIGNED_MASK, //sign mask
	    DEC_32_NUMERIC_MASK, //numeric mask
	    VARIABLE_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
	return;

}/* END: vSERIAL_UIV32out() */

/*************************  vSERIAL_I32out()  ********************************
 *
 * Signed Integer 32-Bit output
 *
 ******************************************************************************/

void vSERIAL_I32out(long lVal)
{
	vSERIAL_generic_IVout(DEC_32_SIGN_MASK, //sign mask
	    DEC_32_NUMERIC_MASK, //numeric mask
	    SIGNED_DEC_32_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str(lVal, DEC_32_SIGN_MASK, DEC_32_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(SIGNED_DEC_32_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_I32out() */

/*************************  vSERIAL_IV32out()  ********************************
 *
 * Signed Integer 32-Bit output variable width
 *
 ******************************************************************************/

void vSERIAL_IV32out(long lVal)
{
	vSERIAL_generic_IVout(DEC_32_SIGN_MASK, //sign mask
	    DEC_32_NUMERIC_MASK, //numeric mask
	    VARIABLE_FLD_SIZE, //desired output fld size
	    ' ', //Lead fill char
	    lVal //numeric value
	    );
#if 0
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str(lVal, DEC_32_SIGN_MASK, DEC_32_NUMERIC_MASK, ucaValArray);

	vSERIAL_FormatStrout(VARIABLE_FLD_SIZE, ' ', ucaValArray);
#endif
	return;

}/* END: vSERIAL_IV32out() */

/*************************  vSERIAL_UI32MicroDecOut()  ***********************
 *
 * Unsigned Integer 32-Bit Micro Decimal output
 *
 ******************************************************************************/

void vSERIAL_UI32MicroDecOut(long lVal)
{
//	#define TEST_MICRO_DEC_OUT

#ifdef TEST_MICRO_DEC_OUT
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];
	uchar uci;
#endif

	vSERIAL_generic_IVout(DEC_32_UNSIGNED_MASK, //sign mask
	    0xFFFFFL, //numeric mask
	    6, //desired output fld size
	    '0', //Lead fill char
	    lVal //numeric value
	    );

#ifdef TEST_MICRO_DEC_OUT

#if 1
	vSERIAL_crlf();
	vSERIAL_sout("ValPassedIn= ", 13);
	vSERIAL_UIV32out((ulong)lVal);
	vSERIAL_crlf();
#endif

#if 1
	/* ZRO THE VALUE */
	for(uci=0; uci<NUMSTR_ARRAY_SIZE; uci++)
	{
		ucaValArray[uci] = 0;
	}

	vSERIAL_sout("ucaValArrayBeforeFill= ", 23);
	for(uci=0; uci<NUMSTR_ARRAY_SIZE; uci++)
	{
		vSERIAL_HB8out(ucaValArray[uci]);
		vSERIAL_bout(',');
	}
	vSERIAL_crlf();
#endif

	vSERIAL_B2D_Str(lVal, DEC_32_UNSIGNED_MASK, 0xFFFFFL, ucaValArray);

#if 1
	vSERIAL_crlf();
	vSERIAL_sout("ucaValArrayAfterFill= ", 22);
	for(uci=0; uci<NUMSTR_ARRAY_SIZE; uci++)
	{
		vSERIAL_HB8out(ucaValArray[uci]);
		vSERIAL_bout(' ');
	}
	vSERIAL_crlf();
#endif

	vSERIAL_FormatStrout(6,'0', ucaValArray);
#endif

	return;

}/* END: vSERIAL_UI32MicroDecOut() */

#if 0

/************************** uiSERIAL_UI16in()  *******************************
 *
 * Unsigned integer input
 *
 ******************************************************************************/

unsigned int uiSERIAL_UI16in(
		void
)
{
	unsigned int uiRetVal;

	uiRetVal = 0;

	return(uiRetVal);

}/* END: uiSERIAL_UI16in() */

#endif

/*************************  vSERIAL_B2D_Str()  ***************************
 *
 * BINARY TO DECIMAL CONVERSION
 *
 * This routine calculates a signed or unsigned decimal number to a string
 *
 * Returns array str:
 *
 *   0    1    2    3    4    5    6    7    8    9   10
 * ÚÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄ¿
 * ³ S ³ D9 ³ D8 ³ D7 ³ D6 ³ D5 ³ D4 ³ D3 ³ D2 ³ D1 ³ D0 ³
 * ÀÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÙ
 *  ' '
 *  or
 *  '-'
 *
 ******************************************************************************/

static void vSERIAL_B2D_Str(long lVal, //IN: value to convert
    ulong ulSignBitMask, //IN: bit on in Sign position (0 if none)
    ulong ulNumericMask, //IN: Mask of number including sign
    uchar ucaValArray[NUMSTR_ARRAY_SIZE] //OUT: ten digits and a leading sign
    )
{
	char cc;
	ulong ulUnsignedVal;

	/* ASSUME THAT WE HAVE A POSITIVE NUMBER */
	ucaValArray[0] = ' ';
	ulUnsignedVal = (ulong) lVal;

	/* HANDLE NEGATIVE NUMBER IF REQUIRED */
	if (ulSignBitMask != 0)
	{
		/* CHECK IF NUMBER WAS REALLY NEGATIVE */
		if (ulUnsignedVal & ulSignBitMask)
		{
			ucaValArray[0] = '-'; //its a negative -- save sign and convert

			/* NEGATE THE VALUE */
			ulUnsignedVal = ~ulUnsignedVal;
			ulUnsignedVal++;
			ulUnsignedVal &= ulNumericMask;

		}/* END: if() */

	}/* END: if() */

	/* BREAK THE NUMBER INTO ITS DIGITS */
	for (cc = 10; cc >= 1; cc--)
	{
		ucaValArray[cc] = (uchar) (ulUnsignedVal % 10);
		ulUnsignedVal /= 10;
	}

#if 0
	/* SHOW THE PASSED BACK ARRAY */
	vSERIAL_sout("ARRAY= ", 7);
	for(cc=0; cc<11; cc++)
	{
		vSERIAL_HB8out(ucaValArray[cc]);
		vSERIAL_bout(',');
	}
	vSERIAL_crlf();
#endif

	return;

}/* END: vSERIAL_B2D_Str() */

/*************************  vSERIAL_FormatStrout()  ********************************
 *
 * formatted output
 *
 * Accepts:
 *   0    1    2    3    4    5    6    7    8    9   10
 * ÚÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄ¿
 * ³ S ³ D9 ³ D8 ³ D7 ³ D6 ³ D5 ³ D4 ³ D3 ³ D2 ³ D1 ³ D0 ³
 * ÀÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÙ
 *
 *
 * EXAMPLE:
 *
 * If input was -2135
 *
 * Accepts:
 *   0    1    2    3    4    5    6    7    8    9   10
 * ÚÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄÂÄÄÄÄ¿
 * ³ - ³  0 ³  0 ³  0 ³  0 ³  0 ³  0 ³  2 ³  1 ³  3 ³  5 ³
 * ÀÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÁÄÄÄÄÙ
 *
 ******************************************************************************/

static void vSERIAL_FormatStrout(char cDesiredFldSize, //fld size (digits+sign), 0 = don't care
    uchar ucLeaderChar, // ' ' or '0'
    uchar ucaValArray[NUMSTR_ARRAY_SIZE] //ten digits and a leading sign
    )
{
	char cc;
	char cFirstNonZro;
	char cActualFldSize;
	char cPrintStartIdx;

	/* WALK THE STRING CONVERTING IT TO ASCII */
	for (cc = 1; cc < 11; cc++)
	{
		ucaValArray[cc] |= 0x30; //convert to ascii
	}

	/* WALK THE STRING AND CONVERT ALL LEADING ZEROS TO LEADER CHAR */
	cFirstNonZro = 10; //start with a good value in position 10
	for (cc = 1; cc <= 9; cc++)
	{
		if (ucaValArray[cc] == '0')
		{
			ucaValArray[cc] = ucLeaderChar;
		}
		else
		{
			cFirstNonZro = cc; //shorten up the good value position
			break;
		}
	}/* END: for() */
	
	/* IF THE STRING IS NEGATIVE AND FIRSTNONZRO DIGIT IS NOT POSITION 1 */
	/* MOVE MINUS SIGN TO FIRSTNONZRO DIGIT -1 */
	if ((cFirstNonZro != 1) && (ucaValArray[0] == '-'))
	{
		cFirstNonZro--;
		ucaValArray[cFirstNonZro] = '-';
		ucaValArray[0] = ' ';
	}

	/* COMPUTE THE FIELD SIZE */
	cActualFldSize = 11 - cFirstNonZro;
	if (cDesiredFldSize != 0)
	{
		if (cDesiredFldSize > cActualFldSize)
			cActualFldSize = cDesiredFldSize;
	}

	cPrintStartIdx = 11 - cActualFldSize;
	if ((cDesiredFldSize == 0) && (cActualFldSize == 10) && (ucaValArray[0] == '-'))
		cPrintStartIdx--;

	/* NOW PRINT OUT THE STRING */
	for (cc = cPrintStartIdx; cc < 11; cc++)
	{
		vSERIAL_bout(ucaValArray[cc]);
	}/* END: for(cc) */

	return;

}/* END: vSERIAL_FormatStrout() */

/***********************  ucSERIAL_getEditedInput()  ************************
 *
 * Get a short edited input str from console
 *
 * RET 0 if msg is good
 *	  1 if msg has been aborted (ESC was hit)
 *
 *****************************************************************************/

uchar ucSERIAL_getEditedInput( //RET: 0 if msg is good, 1 if msg aborted
    uchar ucStr[], //ASCIZ str passed back-- CRLF = 0;
    uchar ucMaxCount //max chars = max msg size -1
    )
{
	uchar ucii;
	uchar ucRetVal;
	uchar ucBreakoutFlg;

	ucRetVal = 0; //assume msg is good
	ucBreakoutFlg = FALSE;
	for (ucii = 0; ucii < ucMaxCount;)
	{
		/* GET A CHAR */
		ucStr[ucii] = ucSERIAL_bin();
		
		switch (ucStr[ucii])
		{
			/* HANDLE BACKSPACE */
			case 0x08:
				if (ucii != 0)
				{
					vSERIAL_bout(0x08); //echo backspace
					ucii--; //backup the ptr
				}
			break;

			case 0x0D:
			case 0x0A:
				if (ucSERIAL_readColTab() != 0)
					vSERIAL_crlf(); //echo CR-LF
				ucStr[ucii] = 0; //terminate the line
				ucBreakoutFlg = TRUE;
			break;

			case 0x1B: //escape char
				ucRetVal = 1;
				ucBreakoutFlg = TRUE;
			break;

			default:
				vSERIAL_bout(ucStr[ucii]); //echo backspace
				ucii++; //bump the ptr
			break;
		}/* END: switch() */

		if (ucBreakoutFlg)
			break;

	}/* END: for(ucii) */

	if (ucii >= ucMaxCount)
	{
		if (ucSERIAL_readColTab() != 0)
			vSERIAL_crlf(); //echo CR-LF
	}

	return (ucRetVal);

}/* END: vSERIAL_getEditedInput() */



//////////////////////////////////////////////////////////////////////////////
//!
//! \fn ucSerial_AsciiToByte
//! \brief Converts a byte from ascii to numeric
//!
//! \param ucInput, ucRadix
//! \return ucDigitVal
/////////////////////////////////////////////////////////////////////////////
uchar ucSerial_AsciiToByte(uchar ucInput, uchar ucRadix)
{
	uchar ucDigitVal;

	// Init ucDigitVal to an invalid value
	ucDigitVal = ucRadix;

	// Convert the value
	if ((ucInput >= '0') && (ucInput <= '9'))
		ucDigitVal = ucInput - '0';

	if ((ucInput >= 'A') && (ucInput <= 'Z'))
		ucDigitVal = ucInput - 'A' + 10;

	if ((ucInput >= 'a') && (ucInput <= 'z'))
		ucDigitVal = ucInput - 'a' + 10;

#if 0
	vSERIAL_HB8out(ucDigitVal);
	vSERIAL_bout(',');
#endif

return ucDigitVal;
}

/************************  lSERIAL_AsciiToNum()  *****************************
 *
 * convert an ascii string to a longnumeric value
 *
 *
 ******************************************************************************/

long lSERIAL_AsciiToNum(uchar ucStr[], uchar ucSignFlag, //UNSIGNED = 0, SIGNED = 1
    uchar ucRadix)
{
	long lRetVal;
	uchar ucDigitVal;
	uchar uc;
	uchar ucNegateValue;
	uchar ucIsPreNumeric;

	lRetVal = 0;
	ucNegateValue = FALSE;
	ucIsPreNumeric = TRUE;

	for (uc = 0; uc < 12; uc++)
	{

		/* EAT ANY LEADING SPACES */
		if ((ucStr[uc] == ' ') && (ucIsPreNumeric == TRUE))
		{
			continue;
		}

		/* CHECK FOR A LEADING SIGN */
		if ((ucStr[uc] == '-') && (ucSignFlag == SIGNED) && (ucIsPreNumeric == TRUE))
		{
			ucNegateValue = TRUE;
			continue;
		}

		ucIsPreNumeric = FALSE;

		/* CONVERT THE CHAR TO A DIGIT VALUE */
		ucDigitVal = ucSerial_AsciiToByte(ucStr[uc], ucRadix);

//		ucDigitVal = ucRadix;
//		if ((ucStr[uc] >= '0') && (ucStr[uc] <= '9'))
//			ucDigitVal = ucStr[uc] - '0';
//		if ((ucStr[uc] >= 'A') && (ucStr[uc] <= 'Z'))
//			ucDigitVal = ucStr[uc] - 'A' + 10;
//		if ((ucStr[uc] >= 'a') && (ucStr[uc] <= 'z'))
//			ucDigitVal = ucStr[uc] - 'a' + 10;
//#if 0
//		vSERIAL_HB8out(ucDigitVal);
//		vSERIAL_bout(',');
//#endif
		if (ucDigitVal >= ucRadix)
			break; //throw out illegal chars

		lRetVal *= ucRadix; //shift up for next value
		lRetVal += ucDigitVal; //add in next value

	}/* END: for() */

#if 0
	vSERIAL_crlf();
#endif

	if (ucNegateValue)
		lRetVal = -lRetVal;

	return (lRetVal);

}/* END: lSERIAL_AsciiToNum() */


/************************  lSERIAL_AsciiToNum()  *****************************
 *
 * convert an ascii string to a longnumeric value
 *
 *
 ******************************************************************************/

long lSERIAL_AsciiToNumber(uchar ucStr[], uchar ucSignFlag, //UNSIGNED = 0, SIGNED = 1
    uchar ucRadix, uchar ucLength)
{
	long lRetVal;
	uchar ucDigitVal;
	uchar uc;
	uchar ucNegateValue;
	uchar ucIsPreNumeric;

	lRetVal = 0;
	ucNegateValue = FALSE;
	ucIsPreNumeric = TRUE;

	for (uc = 0; uc < ucLength; uc++)
	{

		/* EAT ANY LEADING SPACES */
		if ((ucStr[uc] == ' ') && (ucIsPreNumeric == TRUE))
		{
			continue;
		}

		/* CHECK FOR A LEADING SIGN */
		if ((ucStr[uc] == '-') && (ucSignFlag == SIGNED) && (ucIsPreNumeric == TRUE))
		{
			ucNegateValue = TRUE;
			continue;
		}

		ucIsPreNumeric = FALSE;


		ucDigitVal = ucSerial_AsciiToByte(ucStr[uc], ucRadix);

//		/* CONVERT THE CHAR TO A DIGIT VALUE */
//		ucDigitVal = ucRadix;
//		if ((ucStr[uc] >= '0') && (ucStr[uc] <= '9'))
//			ucDigitVal = ucStr[uc] - '0';
//		if ((ucStr[uc] >= 'A') && (ucStr[uc] <= 'Z'))
//			ucDigitVal = ucStr[uc] - 'A' + 10;
//		if ((ucStr[uc] >= 'a') && (ucStr[uc] <= 'z'))
//			ucDigitVal = ucStr[uc] - 'a' + 10;
//#if 0
//		vSERIAL_HB8out(ucDigitVal);
//		vSERIAL_bout(',');
//#endif


		if (ucDigitVal >= ucRadix)
			break; //throw out illegal chars

		lRetVal *= ucRadix; //shift up for next value
		lRetVal += ucDigitVal; //add in next value

	}/* END: for() */

#if 0
	vSERIAL_crlf();
#endif

	if (ucNegateValue)
		lRetVal = -lRetVal;

	return (lRetVal);

}/* END: lSERIAL_AsciiToNum() */

/********************  vSERIAL_printDashIntro()  ***************************
 *
 *
 *
 ******************************************************************************/
void vSERIAL_printDashIntro(T_Text S_StrPtr)
{
	vSERIAL_sout("\r\n\r\n---  ", 9);
	vSERIAL_sout(S_StrPtr.m_cText, S_StrPtr.m_uiLength);
	vSERIAL_sout("  ---\r\n\r\n", 9);

}/* END: vSERIAL_printDashIntro() */

/*************************  ulSERIAL_H32in()  *******************************
 *
 * do HEX unsigned long number input.
 *
 *
 *****************************************************************************/

ulong ulSERIAL_H32in(void)
{
	ulong ulVal;
	uchar ucStr[13];

	/* GET THE NUMERIC STRING */
	while (ucSERIAL_getEditedInput(ucStr, 13))
		;

#if 0
	{
		uchar ucc;
		vSERIAL_sout("ucStr=", 6);
		for(ucc=0; ucc<13; ucc++)
		{
			if(ucStr[ucc] == 0) break;
			vSERIAL_bout(ucStr[ucc]);
			vSERIAL_bout(',');
		}
		vSERIAL_crlf();
	}
#endif

	/* CONVERT IT TO A LONG */
	ulVal = (ulong) lSERIAL_AsciiToNum(ucStr, UNSIGNED, HEX);

#if 0
	vSERIAL_sout("Numeric= ", 9);
	vSERIAL_HB32out(ulVal);
	vSERIAL_crlf();
#endif

	return (ulVal);

}/* END: ulSERIAL_H32in() */

/*********************  lSERIAL_I32in()  ***********************************
 *
 * do DECIMAL long number input.
 *
 *
 *****************************************************************************/

long lSERIAL_I32in(void)
{
	long lValue;
	uchar ucStr[13];

	/* GET THE NUMERIC STRING */
	while (ucSERIAL_getEditedInput(ucStr, 13))
		;

	/* CONVERT IT TO A LONG */
	lValue = lSERIAL_AsciiToNum(ucStr, SIGNED, DECIMAL);

	return (lValue);

}/* END: lSERIAL_I32in() */

/*********************  ulSERIAL_UI32in()  ***********************************
 *
 * do DECIMAL ulong number edited input.
 *
 *
 *****************************************************************************/

ulong ulSERIAL_UI32in(void)
{
	ulong ulValue;
	uchar ucStr[13];

	/* GET THE NUMERIC STRING */
	while (ucSERIAL_getEditedInput(ucStr, 13))
		;

	/* CONVERT IT TO A LONG */
	ulValue = (ulong) lSERIAL_AsciiToNum(ucStr, UNSIGNED, DECIMAL);

	return (ulValue);

}/* END: ulSERIAL_UI32in() */

/********************  vSERIAL_generic_IVout()  ******************************
 *
 *
 *
 *****************************************************************************/

static void vSERIAL_generic_IVout(ulong ulSignMaskBit, //sign mask
    ulong ulNumericMask, //numeric mask
    char cDesiredFldSize, //desired output field size
    uchar ucLeadFillChar, //leading zero fill char
    long lVal //numeric value to show
    )
{
	uchar ucaValArray[NUMSTR_ARRAY_SIZE];

	vSERIAL_B2D_Str(lVal, ulSignMaskBit, ulNumericMask, ucaValArray);

	vSERIAL_FormatStrout(cDesiredFldSize, ucLeadFillChar, ucaValArray);

	return;

}/* END: vSERIAL_IV32out() */

/***********************  vSERIAL_HB6ByteOut()  ******************************
 *
 *
 *
 *****************************************************************************/

void vSERIAL_HB6ByteOut(uchar *ucpArrayPtr)
{
	uchar ucc;

	for (ucc = 0; ucc < 6; ucc++)
	{
		vSERIAL_HB8out(*ucpArrayPtr);
		ucpArrayPtr++;
	}/* END: for() */

	return;
	
}/* END: vSERIAL_HB6ByteOut() */

#ifdef SERIAL_Q_OUTPUT_ENABLED

/*********************  vSERIAL_pushQ()  **************************************
 *
 *
 *
 ******************************************************************************/

static void vSERIAL_pushQ(
		uchar ucChar
)
{
	ulong ulWaitCount;

	/* CHK IF THE Q IS FULL */
	ulWaitCount = 10000000;
	while(ucQcount >= COMM_Q_SIZE) //loop until there is space
	{
		ulWaitCount--;
		if(!ulWaitCount) return; //timeout
	}

	/* Q IS NOT FULL -- STUFF THE CHAR ONTO THE Q */
	ucQonIdx_LUL++; //inc idx
	if(ucQonIdx_LUL >= COMM_Q_SIZE) ucQonIdx_LUL = 0;
	ucaCommQ[ucQonIdx_LUL] = ucChar;//stuff the char

	/* TURN INTS OFF TO WRITE THE COUNT */
	UCA1IE &= ~UCTXIE; //go no-int
	ucQcount++;//boost the count
	UCA1IE |= UCTXIE;//go yes-int
	UCA1CTL1 &= ~UCSWRST;//transmit enabled

	return;

}/* END: vSERIAL_pushQ() */
#endif

////////////////////////////////////////////////////////////////////////////////////
//!
//!	\brief Waits for a message over the serial line
//!
//! \param none
//! \return 1 for fail
////////////////////////////////////////////////////////////////////////////////////
uchar ucSERIAL_Wait_for_Message(void)
{
	uchar ucRXMessageLen;

	// Set the received message length to an invalid size
	ucRXMessageLen = MAX_MSG_SIZE + 1;

	// Initialize the serial comm index and clean the buffer
	g_ucaUSCI_A1_RXBufferIndex = 0x00;
	vSERIAL_CleanBuffer(g_ucaUSCI_A1_RXBuffer);

	//Enable interrupts and clear interrupt flags in the UART peripheral
	UART1_REG_IFG &= ~ UART1_RX_IFG;
	UART1_REG_IE |= UART1_RX_IE;

	// while there is time in the slot/subslot
	while(ucTimeCheckForAlarms(ALLSLOT_ALARMS) == 0)
	{
		// If we have received the header from the garden server then set the RX message counter
		// to the length of the incoming message
		if (g_ucaUSCI_A1_RXBufferIndex == MSG_IDX_LEN + NET_HDR_SZ + 1)
		{
			ucRXMessageLen = g_ucaUSCI_A1_RXBuffer[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ;
			if (ucRXMessageLen > MAX_MSG_SIZE)
				ucRXMessageLen = MAX_MSG_SIZE;
		}

		// Once the message has been received break out of the loop.
		if(g_ucaUSCI_A1_RXBufferIndex == ucRXMessageLen)
			break;

		// Wait in low power mode to receive a byte
		LPM0;

	}

	// If we have a comm underflow return an error
	if(ucRXMessageLen < (g_ucaUSCI_A1_RXBufferIndex-1))
		return 1;

	// If we haven't received a message return error
	if(ucRXMessageLen == MAX_MSG_SIZE + 1)
		return 1;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
//!
//!	\brief Waits for a message over the serial line
//!
//! \param none
//! \return 1 for fail
////////////////////////////////////////////////////////////////////////////////////
uchar ucSERIAL_Wait_for_MessageNoTimeout(void)
{
	uchar ucRXMessageLen;

	// Set the received message length to an invalid size
	ucRXMessageLen = MAX_MSG_SIZE + 1;

	// Initialize the serial comm index and clean the buffer
	g_ucaUSCI_A1_RXBufferIndex = 0x00;
	vSERIAL_CleanBuffer(g_ucaUSCI_A1_RXBuffer);

	//Enable interrupts and clear interrupt flags in the UART peripheral
	UART1_REG_IFG &= ~ UART1_RX_IFG;
	UART1_REG_IE |= UART1_RX_IE;

	// while there is time in the slot/subslot
	while(1)
	{
		// If we have received the header from the garden server then set the RX message counter
		// to the length of the incoming message
		if (g_ucaUSCI_A1_RXBufferIndex == MSG_IDX_LEN + NET_HDR_SZ + 1)
		{
			ucRXMessageLen = g_ucaUSCI_A1_RXBuffer[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ;
			if (ucRXMessageLen > MAX_MSG_SIZE)
				ucRXMessageLen = MAX_MSG_SIZE;
		}

		// Once the message has been received break out of the loop.
		if(g_ucaUSCI_A1_RXBufferIndex == ucRXMessageLen)
			break;

		// Wait in low power mode to receive a byte
		LPM0;

	}

	// If we have a comm underflow return an error
	if(ucRXMessageLen < (g_ucaUSCI_A1_RXBufferIndex-1))
		return 1;

	// If we haven't received a message return error
	if(ucRXMessageLen == MAX_MSG_SIZE + 1)
		return 1;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Waits for an ACK, NACK, or a command.
//!
//! The ACK/NACK does not follow the usual communication protocol.  It is kept short
//! intentially to increase throughput.
//!
//!	\param none
//!	\return ucRetVal, the message type or error code
///////////////////////////////////////////////////////////////////////////////////
uchar ucSerial_Wait_for_Ack(void)
{
	uchar ucRetVal;
	uchar ucRXMessageLen;

	// Initialize the serial comm index and clean the buffer
	g_ucaUSCI_A1_RXBufferIndex = 0x00;
	vSERIAL_CleanBuffer(g_ucaUSCI_A1_RXBuffer);

	// Set the received message length to an invalid size
	ucRXMessageLen = MAX_MSG_SIZE + 1;

	//Enable interrupts and clear interrupt flags in the UART peripheral
	UART1_REG_IFG &= ~ UART1_RX_IFG;
	UART1_REG_IE |= UART1_RX_IE;

	// Assume a timeout
	ucRetVal = TIMEOUT;

	// Start the LPM delay timer
	vTime_SetLPM_DelayAlarm(T_START, 50000);

	// while there is time in the slot/subslot
	while(ucTimeCheckForAlarms(LPM_DELAY_ALARM_BIT) == 0)
	{

		// If we have received this much data then check to see if it is an ACK
		if (g_ucaUSCI_A1_RXBufferIndex == 3)
		{
			if (g_ucaUSCI_A1_RXBuffer[0] == 0x41 && g_ucaUSCI_A1_RXBuffer[1] == 0x43 && g_ucaUSCI_A1_RXBuffer[2] == 0x4B)
			{
				ucRetVal = ACK;
				break;
			}
		}

		// If we have received this much data then check to see if it is an NACK
		if (g_ucaUSCI_A1_RXBufferIndex == 4)
		{
			if (g_ucaUSCI_A1_RXBuffer[0] == 0x4E && g_ucaUSCI_A1_RXBuffer[1] == 0x41 && g_ucaUSCI_A1_RXBuffer[2] == 0x43
			    && g_ucaUSCI_A1_RXBuffer[3] == 0x4B)
			{
				ucRetVal = NACK;
				break;
			}
		}

		// If we have received the header from the garden server then set the RX message counter
		// to the length of the incoming message
		if(g_ucaUSCI_A1_RXBufferIndex == MSG_IDX_LEN + NET_HDR_SZ + 1)
			{
			ucRXMessageLen = g_ucaUSCI_A1_RXBuffer[MSG_IDX_LEN] + NET_HDR_SZ + CRC_SZ;
			if (ucRXMessageLen > MAX_MSG_SIZE)
				ucRXMessageLen = MAX_MSG_SIZE;

			ucRetVal = CMD;
			}

		// Once the message has been received break out of the loop.
		if (g_ucaUSCI_A1_RXBufferIndex == ucRXMessageLen + 1) {
			break;
		}

		// Wait in low power mode to receive a byte
		LPM0;
	}

	// Turn off the LPM delay timer
	vTime_SetLPM_DelayAlarm(T_STOP, 0);

#if 0
	uchar ucCount;
	for (ucCount = 0; ucCount < g_ucaUSCI_A1_RXBufferIndex; ucCount++)
	{
		vSERIAL_HB8out(g_ucaUSCI_A1_RXBuffer[ucCount]);
		vSERIAL_crlf();
	}
#endif

	return ucRetVal;
}

/* -----------------------  END OF MODULE  --------------------------------- */
