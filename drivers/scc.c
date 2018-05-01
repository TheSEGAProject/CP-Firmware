/////////////////////////////////////////////////////////////////////////////////
//! \file scc.c
//!	\brief Driver for the solar charge controller
//!
//!
/////////////////////////////////////////////////////////////////////////////////

#include <msp430.h>
#include "std.h"
#include "usci_A0_uart.h"
#include "SP.h"
#include "serial.h"
#include "time.h"
#include "task.h"
#include "crc.h"
#include "scc.h"
#include "delay.h"

//! \var g_ucaSCC_TXBuff
//! \brief Transmit buffer for the SCC
uchar g_ucaSCC_TXBuff[TX_BUFFER_SIZE];

//! \var g_ucaSCC_RXBuff
//! \brief Receive buffer for the SCC
uchar g_ucaSCC_RXBuff[TX_BUFFER_SIZE];

//! \struct
//! \brief The structure holds information about the SCC board
struct
{
	volatile uint8 m_ucAttached;														//!< For determining if SCC is present
	volatile uint8 m_ucMSG_Version;													//!< Message versions of the SCC
	volatile uint8 m_ucState;																//!< Status of the SCC board
	volatile uint8 m_ucSerialNumber[SP_HID_LENGTH];					//!< Unique serial number of the SCC
	volatile uint8 m_ucNumTransducers;											//!< Number of transducers on the SCC
	volatile uint8 m_ucTypeTransducers[MAX_TRANSDUCERS];		//!< Type of transducers on the SCC
	volatile uint8 m_ucTransSmplDur[MAX_TRANSDUCERS];				//!< Sample duration of a transducer
}S_SCC;

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
		unsigned FLG3_GSV_COM_BIT :1; // 1=GSV com is active 0= GSV com is inactive
	} FLAG3_STRUCT;
} ucFLAG3_BYTE;

uint8 ucSCC_SendInterrogate(void);
void vSCC_SpoofInterrogate(void);

///////////////////////////////////////////////////////////////////////////////
//! \fn vSCC_Init
//! \brief Initializes the solar charge controller driver
///////////////////////////////////////////////////////////////////////////////
void vSCC_Init(void){

	uchar ucStructIndex;
	uchar ucMsgIndex;

	if(ucSCC_SendInterrogate() == COMM_ERROR){
		S_SCC.m_ucAttached = FALSE;

		for (ucStructIndex = 0; ucStructIndex < 8; ucStructIndex++) {
			S_SCC.m_ucSerialNumber[ucStructIndex] = 0xFF;
		}

		return;
	}

	ucMsgIndex = SP_MSG_PAYLD_IDX;
	S_SCC.m_ucAttached = TRUE;
	S_SCC.m_ucMSG_Version = g_ucaSCC_RXBuff[SP_MSG_VER_IDX];
	S_SCC.m_ucNumTransducers = g_ucaSCC_RXBuff[ucMsgIndex++];

	for (ucStructIndex = 0; ucStructIndex < S_SCC.m_ucNumTransducers; ucStructIndex++) {
		S_SCC.m_ucTypeTransducers[ucStructIndex] = g_ucaSCC_RXBuff[ucMsgIndex++];
		S_SCC.m_ucTransSmplDur[ucStructIndex] = g_ucaSCC_RXBuff[ucMsgIndex++];
	}

	S_SCC.m_ucState = SP_STATE_INACTIVE;

#if 0
	vSERIAL_sout("SCC Present:\r\n", 14);
	vSERIAL_sout("SCC Msg Version: ", 17);
	vSERIAL_HB8out(S_SCC.m_ucMSG_Version);

	vSERIAL_sout("SCC SerialNumber:", 17);
	for (ucStructIndex = 0; ucStructIndex < 8; ucStructIndex++){
		vSERIAL_HB8out(S_SCC.m_ucSerialNumber[ucStructIndex]);
	}

	vSERIAL_sout("SCC Number of transucers:", 25);
	vSERIAL_HB8out(S_SCC.m_ucNumTransducers);
#endif

}

/////////////////////////////////////////////////////////////////////////////////
//! \fn ucSCC_GetHID
//! \brief Gets the hardware ID of the SCC board
//! \param *ucSCC_HID, pointer to the location where the HID will be copied
//! \return 0 for success, else failure
/////////////////////////////////////////////////////////////////////////////////
uint8 ucSCC_GetHID(uchar * ucSCC_HID)
{
	uint8 ucCounter;

	if(ucSCC_IsAttached() == FALSE)
		return 1;

	for (ucCounter = 0; ucCounter < SP_HID_LENGTH; ucCounter++) {
		*ucSCC_HID++ = S_SCC.m_ucSerialNumber[ucCounter];
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
//! \brief Set HID for the SCC
//!
//!	Sets the hardware ID of the SCC using the diagnostic menu
//!
//! \param ucSerialNum, pointer to the new HID to use
//! \return 0 for success, else failure
///////////////////////////////////////////////////////////////////////////////
uint8 ucSCC_SetHID(uint8 * ucSerialNum)
{
	uchar ucMsgIndex;
	uchar ucRetVal;
	uchar ucAttemptCount, ucByteCount;

	// Assume failure
	ucRetVal = COMM_ERROR;

	if (!ucSCC_IsAttached()) //Do not proceed if there is not a functioning board present
		return SP_NOT_ATTACHED;

	g_ucaSCC_TXBuff[SP_MSG_TYP_IDX] = SET_SERIALNUM;
	g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] = SP_HEADERSIZE + SP_HID_LENGTH;
	g_ucaSCC_TXBuff[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaSCC_TXBuff[SP_MSG_FLAGS_IDX] = 0;

	for (ucMsgIndex = SP_MSG_PAYLD_IDX; ucMsgIndex < (SP_MSG_PAYLD_IDX + SP_HID_LENGTH); ucMsgIndex++) {
		g_ucaSCC_TXBuff[ucMsgIndex] = *ucSerialNum++;
	}

	// Init the uart peripheral
	vUSCI_A0_UART_init();

	// Send the message, repeat if required
	for (ucAttemptCount = 5; ucAttemptCount > 0; ucAttemptCount--) {
		// Clean the RX buffer
		vUSCI_A0_UART_CleanBuff();

		// Compute the CRC for the message
		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, g_ucaSCC_TXBuff, g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

		// Send the message
		vUSCI_A0_UART_SendBytes(g_ucaSCC_TXBuff, g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

		// An SCC requires approximately 10 mS to write the new HID to flash - true for the SCC?
		vDELAY_LPMWait1us(11000, 2);

		// If there were no communication errors and the message is good
		if (ucSCC_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(SET_SERIALNUM, g_ucaSCC_RXBuff) == COMM_OK) {

			ucMsgIndex = SP_MSG_PAYLD_IDX;
			for (ucByteCount = 0; ucByteCount < SP_HID_LENGTH; ucByteCount++)
				S_SCC.m_ucSerialNumber[ucByteCount] = g_ucaSCC_RXBuff[ucMsgIndex++];

			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}


	// Halt communications
 	vUSCI_A0_UART_quit();

	return ucRetVal;
}
///////////////////////////////////////////////////////////////////////////////
//! \brief Waits for reception of a data message
//!
//! This function waits for a data message to be received on the serial port.
//!   \param None.
//!   \return The error code indicating the status after call
///////////////////////////////////////////////////////////////////////////////
uint8 ucSCC_WaitForMessage(void)
{

	uchar ucMessageLength;
	const uchar ucInvalidMsgLength = 0xF0;

	ucMessageLength = ucInvalidMsgLength;

	// Wait for the SCC board to prepare and transmit its message.  Assuming 115.2 kbaud, 10ms is plenty of time
	vTime_SetLPM_DelayAlarm(ON, 20000);

	// Wait for either a timeout or the first byte
	while ((ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_LPM_DELAY_ALARM == 0) && (ucUSCI_A0_UART_GetIndex() < ucMessageLength + 2)){

		// If we have received the message length byte then update the local copy of the message length
		if (ucUSCI_A0_UART_GetIndex() > DE_IDX_LENGTH && ucMessageLength == ucInvalidMsgLength) {
			ucMessageLength = ucUSCI_A0_UART_ReadByte(DE_IDX_LENGTH);

			// If the message length is out of range then return
			if (ucMessageLength > MAX_SP_MSG_LENGTH)
				return COMM_ERROR;
		}

		// Sleep until timeout or next byte
		LPM1;
	}

	// If there was a timeout then return with error message
	if (ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_LPM_DELAY_ALARM == 1) {
		vTime_SetLPM_DelayAlarm(OFF, 0);
		return COMM_ERROR;
	}

	//Turn of the LPM delay
	vTime_SetLPM_DelayAlarm(OFF, 0);

	// No message received, or message is too large
	if (ucUSCI_A0_UART_ReadBuffer(g_ucaSCC_RXBuff, TX_BUFFER_SIZE) == 1)
		return COMM_ERROR;

	//success
	return COMM_OK;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Grabs the raw chars from buffer and puts them into the desired buffer
//!
//! This function takes the characters from \e g_ucaSCC_RXBuff and stores them
//! in a buffer.  This is used by the application to fetch data elements from
//! SP boards and store them within messages in memory
//!
//!   \param p_ucaBuffer Pointer to the message
//!   \return The error code indicating the status after call
///////////////////////////////////////////////////////////////////////////////
uint8 ucSCC_GrabMessageFromBuffer(union SP_DataMessage * message)
{
	uint8 ucLoopCount;

	for (ucLoopCount = 0x00; ucLoopCount < g_ucaSCC_RXBuff[SP_MSG_LEN_IDX]; ucLoopCount++)
		message->ucByteStream[ucLoopCount] = g_ucaSCC_RXBuff[ucLoopCount];

	return COMM_OK;
}


/////////////////////////////////////////////////////////////////////////////////
//! \brief Returns the message version
/////////////////////////////////////////////////////////////////////////////////
uint8 ucSCC_FetchMsgVersion(void)
{
	return S_SCC.m_ucMSG_Version;
}

///////////////////////////////////////////////////////////////////////////////
//! \fn ucSCC_SetState
//! \brief Sets the state of the SCC board in the structure
//!	\param ucState, the desired state
//! \return Error code, 0 = success
///////////////////////////////////////////////////////////////////////////////
uchar ucSCC_SetState(uchar ucState){

	// Return error if there is no SCC attached
	if (S_SCC.m_ucAttached == FALSE)
		return 1;

	S_SCC.m_ucState = ucState;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! \fn ucSCC_GetState
//! \brief Gets the state of the SCC board in the structure
//!	\param *ucState, pointer to the state variable
//! \return Error code, 0 = success
///////////////////////////////////////////////////////////////////////////////
uchar ucSCC_GetState(uchar *ucState){

	// Return error if there is no SCC attached
	if (S_SCC.m_ucAttached == FALSE)
		return 1;

	*ucState = S_SCC.m_ucState;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//! \fn ulSCC_GetSampleDuration
//! \brief Gets the sample duration
//! \return sample duration
///////////////////////////////////////////////////////////////////////////////
uchar ucSCC_GetSampleDuration(void){

	// Return error if there is no SCC attached
	if (S_SCC.m_ucAttached == FALSE)
		return 0;

	return S_SCC.m_ucTransSmplDur[0];
}

///////////////////////////////////////////////////////////////////////////////
//! \fn ucSCC_IsAttached
//! \brief Returns true if the SCC is present
//! \return 1 if attached else 0
///////////////////////////////////////////////////////////////////////////////
uchar ucSCC_IsAttached(void){

	// Return error if there is no SCC attached
	if (S_SCC.m_ucAttached == FALSE)
		return FALSE;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Input the correct parameters into the packet to send a command, send.
//!
//! \param p_ucCmdPayload, pointer to the structure containing the command pay load.
//! \param ucPayloadLen, Length of the payload
//! \return ucRetVal, 0=success else failure
///////////////////////////////////////////////////////////////////////////////
uint8 ucSCC_SendCommand(uint8 ucPayloadLen, uint8 * p_ucCmdPayload)
{
	uint8 ucAttemptCount;
	uint8 ucRetVal;
	uint8 ucLoopCnt;

	// Assume failure
	ucRetVal = COMM_ERROR;

	// Load the command header
	g_ucaSCC_TXBuff[SP_MSG_TYP_IDX] = COMMAND_PKT;
	g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] = (SP_HEADERSIZE + ucPayloadLen);
	g_ucaSCC_TXBuff[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaSCC_TXBuff[SP_MSG_FLAGS_IDX] = 0;

	// Load the command payload
	for (ucLoopCnt = SP_MSG_PAYLD_IDX; ucLoopCnt < g_ucaSCC_TXBuff[SP_MSG_LEN_IDX]; ucLoopCnt++)
		g_ucaSCC_TXBuff[ucLoopCnt] = *p_ucCmdPayload++;

#if 0
	vSERIAL_sout("SCC CMD\r\n", 9);
	for (ucLoopCnt = 0; ucLoopCnt < g_ucaSCC_TXBuff[SP_MSG_LEN_IDX]; ucLoopCnt++)
	{
		vSERIAL_HB8out(g_ucaSCC_TXBuff[ucLoopCnt]);
	}
	vSERIAL_crlf();
#endif

	// Init the uart peripheral
	vUSCI_A0_UART_init();

	// Send the message, repeat if required
 	for (ucAttemptCount = 5; ucAttemptCount > 0; ucAttemptCount--) {

 		// Clean the RX buffer
 		vUSCI_A0_UART_CleanBuff();

 		// Compute the CRC for the message
 		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, g_ucaSCC_TXBuff, g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

 		// Send the message
 		vUSCI_A0_UART_SendBytes(g_ucaSCC_TXBuff, g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

		// If there were no communication errors and the message is good
		if (ucSCC_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(CONFIRM_COMMAND, g_ucaSCC_RXBuff) == COMM_OK) {

			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

#if 0
			vSERIAL_sout("CMD CNFRM[", 10);
			vSERIAL_HB8out (5-ucAttemptCount);
			vSERIAL_sout("]\r\n", 3);
#endif

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications
 	vUSCI_A0_UART_quit();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Input the correct parameters into the packet to request data, send.
//!
//!   \param ucSPNumber
//!   \return length of the received message
///////////////////////////////////////////////////////////////////////////////
uint8 ucSCC_RequestData(void)
{

	uint8 ucAttemptCount;
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	// Build the message and send it
	g_ucaSCC_TXBuff[SP_MSG_TYP_IDX] = REQUEST_DATA;
	g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] = SP_HEADERSIZE;
	g_ucaSCC_TXBuff[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaSCC_TXBuff[SP_MSG_FLAGS_IDX] = 0;

#if 0
	uint8 ucLoopCnt;
	vSERIAL_sout("SP RQST\r\n", 9);
	for (ucLoopCnt = 0; ucLoopCnt < g_ucaTXBuffer[SP_MSG_LEN_IDX]; ucLoopCnt++)
	{
		vSERIAL_HB8out(g_ucaTXBuffer[ucLoopCnt]);
		vSERIAL_crlf();
	}
#endif

	// Init the uart peripheral
	vUSCI_A0_UART_init();

	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {

 		// Clean the RX buffer
 		vUSCI_A0_UART_CleanBuff();

 		// Compute the CRC for the message
 		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, g_ucaSCC_TXBuff, g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

 		// Send the message
 		vUSCI_A0_UART_SendBytes(g_ucaSCC_TXBuff, g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

		// If there were no communication errors and the message is good
		if (ucSCC_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(REPORT_DATA, g_ucaSCC_RXBuff) == COMM_OK) {

			// Indicate the communication succeeded
			ucRetVal = COMM_OK;
#if 0
			vSERIAL_sout("RQST PASS [", 11);
			vSERIAL_HB8out(3-ucAttemptCount);
			vSERIAL_sout("]\r\n", 3);
#endif

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications
 	vUSCI_A0_UART_quit();

	return ucRetVal;
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Request transducer information from the SP and sets an ID in flash
//!				 memory of the SP board.
//!
//!	This function assists the task manager in determining what tasks the SP can perform.
//! The SP ID helps the CP make sure that the attached SPs have not been changed.
//! In the event that they have been changed this function is called to reconfigure
//! the SP driver to match the parameters of the new SP board
//!
//! \param ucSPnumber
//! \return Error code
///////////////////////////////////////////////////////////////////////////////
uint8 ucSCC_SendInterrogate(void)
{
	uint8 ucAttemptCount;
	uint8 ucRetVal;

	// Assume failure
	ucRetVal = COMM_ERROR;

	// Load the header of the message and send
	g_ucaSCC_TXBuff[SP_MSG_TYP_IDX] = INTERROGATE;
	g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] = SP_HEADERSIZE;
	g_ucaSCC_TXBuff[SP_MSG_VER_IDX] = SP_DATAMESSAGE_VERSION;
	g_ucaSCC_TXBuff[SP_MSG_FLAGS_IDX] = 0;

	// Init the uart peripheral
	vUSCI_A0_UART_init();

	// Send the message, repeat if required
	for (ucAttemptCount = 3; ucAttemptCount > 0; ucAttemptCount--) {

		// Clean the RX buffer
 		vUSCI_A0_UART_CleanBuff();

 		// Compute the CRC for the message
 		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, g_ucaSCC_TXBuff, g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

 		// Send the message
 		vUSCI_A0_UART_SendBytes(g_ucaSCC_TXBuff, g_ucaSCC_TXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

		// If there were no communication errors and the message is good
		if (ucSCC_WaitForMessage() == COMM_OK && ucSP_ChkMsgIntegrity(INTERROGATE, g_ucaSCC_RXBuff) == COMM_OK) {
			// Indicate the communication succeeded
			ucRetVal = COMM_OK;

			// Exit the loop
			break;
		}

		// Wait between commands
		vDELAY_LPMWait1us(10000, 2);
	}

	// Halt communications
 	vUSCI_A0_UART_quit();

	return ucRetVal;
}


// Fakes the reception of an interoggate message
void vSCC_SpoofInterrogate(void){


	g_ucaSCC_RXBuff[SP_MSG_TYP_IDX] = INTERROGATE; 		// Message ID
	g_ucaSCC_RXBuff[SP_MSG_LEN_IDX] = 16;							// Length
	g_ucaSCC_RXBuff[SP_MSG_VER_IDX] = 0x10;							// Message version
	g_ucaSCC_RXBuff[3] = 2;														// Number of transducer
	g_ucaSCC_RXBuff[4] = TYPE_IS_SENSOR;							// Type for sensor 1 (read ADC)
	g_ucaSCC_RXBuff[5] = 0;														// Sample duration (0 seconds means the task can be completed within a second)
	g_ucaSCC_RXBuff[6] = TYPE_IS_ACTUATUATOR;					// Type for sensor 2 (Update forecast data)
	g_ucaSCC_RXBuff[7] = 0;														// Sample duration (0 seconds means the task can be completed within a second)
	g_ucaSCC_RXBuff[8] = 0x01;												// Remaining bytes are the HID
	g_ucaSCC_RXBuff[9] = 0x02;
	g_ucaSCC_RXBuff[10] = 0x03;
	g_ucaSCC_RXBuff[11] = 0x04;
	g_ucaSCC_RXBuff[12] = 0x05;
	g_ucaSCC_RXBuff[13] = 0x06;
	g_ucaSCC_RXBuff[14] = 0x07;
	g_ucaSCC_RXBuff[15] = 0x08;

	// Compute the CRC for the message
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, g_ucaSCC_RXBuff, g_ucaSCC_RXBuff[SP_MSG_LEN_IDX] + CRC_SZ);

}
