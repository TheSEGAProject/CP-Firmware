/////////////////////////////////////////////////////////////////////////////
//!
//! \brief This file is part of the communications module and handles serial
//! communication with the garden server.
//!
//!
//! Chris Porter 7/14
//!
/////////////////////////////////////////////////////////////////////////////

#include "serial.h"
#include "comm.h"
#include "mem_mod.h"			//memory module
#include "crc.h"					//CRC calculation module
#include "time.h"					//Time routines
#include "led.h"
#include "misc.h"
#include "report.h"
#include "main.h"					// Gets the version of main


//! \var g_ucLastGSCommTime
//! \brief Time inbetween the garden server comm. slots (seconds)
ulong g_ulLastGSCommTime;


// global message buffer
extern volatile uint8 ucaMSG_BUFF[MAX_RESERVED_MSG_SIZE];
extern uchar ucFreqAdjustIndex;
extern uchar g_ucFreqLocked;
extern volatile union //ucFLAG0_BYTE
{
	uint8 byte;
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
		unsigned FLG0_ECLK_OFFLINE_BIT :1; //bit 7 1=ECLK is not being used
	//      0=ECLK is being used
	} FLAG0_STRUCT;
} ucFLAG0_BYTE;

extern volatile union //ucFLAG2_BYTE
{
	uint8 byte;
	struct
	{
		unsigned FLG2_T3_ALARM_MCH_BIT :1; //bit 0 ;1=T3 Alarm, 0=no alarm
		unsigned FLG2_T1_ALARM_MCH_BIT :1; //bit 1 ;1=T1 Alarm, 0=no alarm
		unsigned FLG2_T2_ALARM_MCH_BIT :1; //bit 2 ;1=T2 Alarm, 0=no alarm
		unsigned FLG2_CLK_INT_BIT :1; //bit 3	;1=clk ticked, 0=not
		unsigned FLG2_X_FROM_MSG_BUFF_BIT :1; //bit 4
		unsigned FLG2_R_BUSY_BIT :1; //bit 5 ;int: 1=REC BUSY, 0=IDLE
		unsigned FLG2_R_BARKER_ODD_EVEN_BIT :1; //bit 6 ;int: 1=odd, 0=even
		unsigned FLG2_R_BITVAL_BIT :1; //bit 7 ;int:
	} FLAG2_STRUCT;

} ucFLAG2_BYTE;

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


void vGS_SynchGardenServer(void);


/////////////////////////////////////////////////////////////////////////
//!
//!	\brief Send the request to send message to the garden server
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vGS_SendRTS(void)
{
  uchar ucII;
  const uchar ucPacketSize = NET_HDR_SZ + MSG_HDR_SZ + CRC_SZ;

  // Pack the message fields
  vComm_NetPkg_buildHdr(0xFEFE);
  vComm_Msg_buildOperational(MSG_FLG_SINGLE, 1, 0xFEFE, MSG_ID_RTS);
  ucaMSG_BUFF[MSG_IDX_LEN] = MSG_HDR_SZ;

	// COMPUTE THE CRC
	ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF, ucPacketSize) ; //lint !e534 //compute the CRC

  // Send the message
  for (ucII = 0; ucII < ucPacketSize; ucII++)
  	  {
	  vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
  	  }
  vSERIAL_crlf();

}

/////////////////////////////////////////////////////////////////////////
//!
//!	\brief Waits for the ready to receive message from the garden server
//!				 and updates system time
//!
//! \param none
//! \return 0 if message received else error code
/////////////////////////////////////////////////////////////////////////
uchar ucGS_WaitForRTR(void)
{
	uchar ucRetVal;
	int iError;
	int iProportional;
	uint uiNewSubSec;
	uint uiGSSubSec;
	uint uiPrevSubsec;
	uchar ucKp;
	uchar ucDelta;
	signed char cFreqAdjust;
	uint uiErrMag;


	// Restrict access of the peripheral to the garden server
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 1;

	uint uiRTSSubsec = uiTIME_getSubSecAsUint();

	// Assume success
	ucRetVal = 0;

	// If we receive a message from the GS
	if (!ucSERIAL_Wait_for_Message())
	{
		// If we have the message then read it into the message buffer
		vSERIAL_ReadBuffer(ucaMSG_BUFF);

		/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
		if (!(ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
		CHKBIT_CRC + CHKBIT_MSG_TYPE, //chk flags
		    CHKBIT_CRC + CHKBIT_MSG_TYPE, //report flags
		    MSG_ID_RTR, //msg type
		    0, //src SN
		    0 //Dst SN
		    )))
		{

			uiPrevSubsec = uiTIME_getSubSecAsUint();

			// Save the new time in Clk2, therefore Clk1 and alarms are still good
			vTIME_setClk2FromBytes((uchar *) &ucaMSG_BUFF[OPMSG_IDX_TIME_SEC_XI]);

			// Get the sub-seconds from the packet
			uiGSSubSec = (uint) ((ucaMSG_BUFF[OPMSG_IDX_TIME_SUBSEC_HI] << 8) | ucaMSG_BUFF[OPMSG_IDX_TIME_SUBSEC_LO]);

			// Get the difference between the current time and the garden server time
			iError = uiGSSubSec - uiPrevSubsec;

			// Get the magnitude of the error
			if(iError < 0)
				uiErrMag = iError*-1;
			else
				uiErrMag = iError;

			// If the magnitude of the difference between sub-second clocks is large then set clock to the garden server time
			if (uiErrMag > 200)
			{
				// Set the new time and reset the control system
				vGS_SynchGardenServer();
				// set return value to an error
				ucRetVal = 2;
			}
			else
			{
				// Compute the error
				iError = uiGSSubSec - uiPrevSubsec;
				// Get the magnitude of the error
				if(iError < 0)
					uiErrMag = iError*-1;
				else
					uiErrMag = iError;


				// Determine the value of the coefficients based on the magnitude of the deviation
				if (uiErrMag > 8)
				{
					ucKp = 8;
				}
				else if (uiErrMag > 4)
				{
					ucKp = 4;
				}
				else if (uiErrMag > 2)
				{
					ucKp = 2;
				}
				else
				{
					ucKp = 1;
				}

				// Determine the proportional adjustment
				iProportional = iError/ucKp;

				// Get the delta-t between now and the last communication slot
				ucDelta = lTIME_getClk2AsLong() - g_ulLastGSCommTime;

				g_ulLastGSCommTime = lTIME_getClk2AsLong();

				uiNewSubSec = uiPrevSubsec + iProportional;

				vTIME_setSubSecFromUint(uiNewSubSec);

				cFreqAdjust = (signed char) (uiNewSubSec - uiPrevSubsec) / ucDelta;

				vComm_SynchFreq(cFreqAdjust);

#if 0
				vSERIAL_sout("GS = ", 5);
				vSERIAL_UI16out(uiGSSubSec);
				vSERIAL_crlf();

				vSERIAL_sout("Old = ", 5);
				vSERIAL_UI16out(uiPrevSubsec);
				vSERIAL_crlf();

				vSERIAL_sout("New = ", 5);
				vSERIAL_UI16out(uiNewSubSec);
				vSERIAL_crlf();

				vSERIAL_sout("Error = ", 8);
				vSERIAL_I16out(iError);
				vSERIAL_crlf();

				vSERIAL_sout("Proportional = ", 15);
				vSERIAL_I16out(iProportional);
				vSERIAL_crlf();

				vSERIAL_sout("Kp = ", 5);
				vSERIAL_UI8out(ucKp);
				vSERIAL_crlf();
#endif

				ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT = 1;
				ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT = 1;
			}
		}
		else // CRC or message type error
		{
			// set return value to an error
			ucRetVal = 1;
		}
	}
	else // The RTR was not received
	{
		// set return value to an error
		ucRetVal = 1;
	}

	// Release the serial channel
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 0;

	return ucRetVal;
}

/////////////////////////////////////////////////////////////////////////
//!
//!	\brief Report data to the garden server
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vGS_ReportToGardenServer(void)
{

	uchar ucII;
	uchar ucAckRet;
	uint uiNodeID;
	uint uiNextHopID;
	S_Cmd S_CommandData;
	uchar ucDELength;
	uchar ucDEVersion;
	uchar ucProcID;
	uchar ucTransID;
	uchar ucDEID;
	uchar ucCount, ucCmdParamCount;
	uint uiMySN;
	uchar ucMsgIndex;
	uchar ucPacketSize;
	uchar ucAttemptCount;
	long lExpTime;

	// Restrict access of the peripheral to the garden server
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 1;

	// Send the request to send packet
	vGS_SendRTS();

	// Wait for the ready to receive packet
	// if it never comes then return
	if(ucGS_WaitForRTR())
	{
		ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 0;
		return;
	}

	// Retrieve serial number from FRAM
	uiMySN = uiL2FRAM_getSnumLo16AsUint();

	// Set the attempt count to zero
	ucAttemptCount = 0;

	// Send data and receive commands while the subslot alarms are not set
	while (ucTimeCheckForAlarms(SUBSLOT_ALARMS) == 0)
	{
		// If there is a message
		if (ucL2SRAM_getCopyOfCurMsg())
		{
			// Get the size of the packet
		  ucPacketSize = NET_HDR_SZ + ucaMSG_BUFF[MSG_IDX_LEN] + CRC_SZ;

		  // Check the message before sending
		  if(ucPacketSize > MAX_MSG_SIZE){

		  	// Build the report data element header
				vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
				ucMsgIndex = DE_IDX_RPT_PAYLOAD;

				ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_MSG_DELIVERY_FAIL;
				ucaMSG_BUFF[ucMsgIndex] = 0; // data length

				// Store DE
				vReport_LogDataElement(RPT_PRTY_MSG_DELIVERY_FAIL);

				// delete message
				vL2SRAM_delCurMsg();

		  	continue;
		  }

			// Prepend the net layer and append the crc
			vComm_NetPkg_buildHdr(0xFEFE);
			ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF,ucPacketSize); //lint !e534 //compute the CRC

			// Send bytes
			for (ucII = 0; ucII < ucPacketSize; ucII++) {
				vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
			}
			vSERIAL_crlf();
		}
		else //no messages in SRAM send empty packet
		{
		  // Set the size of the packet
		  ucPacketSize = NET_HDR_SZ + MSG_HDR_SZ + CRC_SZ;

			vComm_Msg_buildOperational((MSG_FLG_SINGLE | MSG_FLG_ACKRQST), 1, uiMySN, MSG_ID_OPERATIONAL);
			vComm_NetPkg_buildHdr(0xFEFE);
			ucaMSG_BUFF[MSG_IDX_LEN] = MSG_HDR_SZ;
			ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND, ucaMSG_BUFF,ucPacketSize);

			// Send bytes
			for (ucII = 0; ucII < ucPacketSize; ucII++){
				vSERIAL_HB8out(ucaMSG_BUFF[ucII]);
			}
			vSERIAL_crlf();
		}

		// Wait for the response
		ucAckRet = ucSerial_Wait_for_Ack();

//		ucAckRet = 3;
//
//		ucaMSG_BUFF[0] = 0x00;
//		ucaMSG_BUFF[1] = 0x04;
//		ucaMSG_BUFF[2] = 0xFE;
//		ucaMSG_BUFF[3] = 0xFE;
//		ucaMSG_BUFF[4] = 0x03;
//		ucaMSG_BUFF[5] = 0x20;
//		ucaMSG_BUFF[6] = 0x00;
//		ucaMSG_BUFF[7] = 0x01;
//		ucaMSG_BUFF[8] = 0x80;
//		ucaMSG_BUFF[9] = 0x08;
//		ucaMSG_BUFF[10] = 0x0C;
//		ucaMSG_BUFF[11] = 0x01;
//		ucaMSG_BUFF[12] = 0x05;
//		ucaMSG_BUFF[13] = 0x6E;
//		ucaMSG_BUFF[14] = 0x00;
//		ucaMSG_BUFF[15] = 0x0B;
//
//		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);

		switch (ucAckRet)
		{
			case TIMEOUT: // No response from GS
			break;

			case ACK:
				ucAttemptCount = 0;
				vL2SRAM_delCurMsg();
			break;

			case NACK: // Nack, try to send the message 5 time, if it fails then delete it.
				if(ucAttemptCount == 5) {
					// Build the report data element header
					vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
					ucMsgIndex = DE_IDX_RPT_PAYLOAD;

					ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_MSG_DELIVERY_FAIL;
					ucaMSG_BUFF[ucMsgIndex] = 0; // data length

					// Store DE
					vReport_LogDataElement(RPT_PRTY_MSG_DELIVERY_FAIL);

					// delete message
					vL2SRAM_delCurMsg();

					// Clear the attempt counter
					ucAttemptCount = 0;
				}
				else {
					ucAttemptCount++;
				}
			break;

			case CMD: // Command
				vSERIAL_ReadBuffer(ucaMSG_BUFF);

				/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
				if ((ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
						CHKBIT_CRC + CHKBIT_MSG_TYPE, //chk flags
						0, //report flags
						MSG_ID_OPERATIONAL, //msg type
						0, //src SN
						0 //Dst SN
						))) {
					// If we are here then the message failed the integrity check
					// Send a nack message
					vSERIAL_sout("NACK\r\n", 6);
				}
				else // The command is good
				{
					// Send ACK
					vSERIAL_sout("ACK\r\n", 5);

					// Get node ID
					uiNodeID = (ucaMSG_BUFF[MSG_IDX_ADDR_HI] << 8) | ucaMSG_BUFF[MSG_IDX_ADDR_LO];

					// If the command is for the hub
					if (uiNodeID == uiMySN) {
						// Set the starting point to the payload
						ucCount = MSG_IDX_PAYLD;

						// Get the data element ID
						ucDEID = ucaMSG_BUFF[ucCount++];
						ucDELength = ucaMSG_BUFF[ucCount++];
						ucDEVersion = ucaMSG_BUFF[ucCount++];

						switch (ucDEID)
						{
							case COMMAND_PKT:

								// Get the processor and transducer ID
								ucProcID = ucaMSG_BUFF[ucCount++];
								lExpTime = (long) ulMISC_buildUlongFromBytes((uchar*) &ucaMSG_BUFF[ucCount], YES_NOINT);
								ucCount += 4;
								ucTransID = ucaMSG_BUFF[ucCount++];
								S_CommandData.m_uiTaskId = (uint) ((ucProcID << 8) | ucTransID);
								S_CommandData.m_ucCmdLength = ucDELength - 9;

								// Ensure the command parameters meet size constraints
								if (S_CommandData.m_ucCmdLength > MAXCMDPARAM) {

									// Build the report data element header
									vComm_DE_BuildReportHdr(CP_ID, 2, ucMAIN_GetVersion());
									ucMsgIndex = DE_IDX_RPT_PAYLOAD;

									ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_MSG_DELIVERY_FAIL;
									ucaMSG_BUFF[ucMsgIndex] = 0; // data length

									// Store DE
									vReport_LogDataElement(RPT_PRTY_MSG_DELIVERY_FAIL);

								}
								else {

									// Get the command parameters from the message
									for (ucCmdParamCount = 0; ucCmdParamCount < S_CommandData.m_ucCmdLength; ucCmdParamCount++) {
										S_CommandData.m_ucCmdParam[ucCmdParamCount] = ucaMSG_BUFF[ucCount++];
									}

									// If the command has expired then report it.  Otherwise, go handle the command.
									if (lExpTime < lTIME_getSysTimeAsLong()) {

										// If the destination node is not in the route list then report the delivery failure
										vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
										ucMsgIndex = DE_IDX_RPT_PAYLOAD;

										ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_CMD_EXPIRED;
										ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
										ucaMSG_BUFF[ucMsgIndex++] = ucProcID;
										ucaMSG_BUFF[ucMsgIndex] = ucTransID;
										// Store DE
										vReport_LogDataElement(RPT_PRTY_CMD_EXPIRED);

									}
									else {
										// Get the command parameters from the message
										for (ucCmdParamCount = 0; ucCmdParamCount < S_CommandData.m_ucCmdLength; ucCmdParamCount++) {
											S_CommandData.m_ucCmdParam[ucCmdParamCount] = ucaMSG_BUFF[ucCount++];
										}

										// Set up the command
										ucComm_Handle_Command(S_CommandData);
									}
								}
							break;

							case PROGRAM_CODE:
								// hmm...
							break;

							case REQUEST_DATA:
							break;

						} // END switch (DEID)
					}
					// Otherwise the command is for a descendant node
					else
					{
					uiNextHopID = uiRoute_GetNextHop(uiNodeID);

					// If the node is not in the edge list then generate a report stating the destination was not reached
					if (uiNextHopID == 0)
					{
						// If the report is destined for a node that is not in the network then generate the report DE and store it

						// Build the DE header
						vComm_DE_BuildReportHdr(CP_ID, 4, ucMAIN_GetVersion());
						ucMsgIndex = DE_IDX_RPT_PAYLOAD;

						// Load the rest of the DE
						ucaMSG_BUFF[ucMsgIndex++] = SRC_ID_ROUTING_FAIL; // Src ID
						ucaMSG_BUFF[ucMsgIndex++] = 2; // data length
						ucaMSG_BUFF[ucMsgIndex++] = (uchar) (uiNodeID>> 8);
						ucaMSG_BUFF[ucMsgIndex++] = (uchar) uiNodeID;

						// Store DE
						vReport_LogDataElement(RPT_PRTY_ROUTING_FAIL);

						}
						else
						{
							// Store the command in the queue
							ucL2SRAM_PutCMD_inQueue(uiNextHopID);
						}
					}
					// Delete the transmitted message
					vL2SRAM_delCurMsg();
				}
			break;

			default:
			break;
		}

	}
	// Release the peripheral
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 0;
}


//////////////////////////////////////////////////////////////////////////
//!
//! \brief Synchronizes clock and frequency to the garden server clock
//!
//! This process is complicated by the fact that the garden server cannot
//! report time to the hub in an accurate way.  This non-determinism of the garden server
//! is caused by operating system and driver overhead.
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////
void vGS_SynchGardenServer(void)
{
	uint uiNewSubSec;
	uint uiGSSubSec;
	int iDifference;

	// Restrict access of the peripheral to the garden server
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 1;

	// Send the request to send packet
	vGS_SendRTS();

#if 0
	uint uiRTSSubsec;
	uiRTSSubsec = uiTIME_getSubSecAsUint();
#endif

	// If we receive a message from the GS
	if (!ucSERIAL_Wait_for_Message())
	{
		// If we have the message then read it into the message buffer
		vSERIAL_ReadBuffer(ucaMSG_BUFF);

		/* GOT A MSG -- CHK FOR: CRC, MSGTYPE, GROUPID, DEST_SN */
		if (!(ucComm_chkMsgIntegrity( //RET: Bit Err Mask, 0 if OK
		    CHKBIT_CRC + CHKBIT_MSG_TYPE, //chk flags
		    CHKBIT_CRC + CHKBIT_MSG_TYPE, //report flags
		    MSG_ID_RTR, //msg type
		    0, //src SN
		    0 //Dst SN
		    )))
		{

#if 0
			uint uiPrevSubsec;
			uiPrevSubsec = uiTIME_getSubSecAsUint();
#endif

			// Get the sub-seconds from the packet
			uiGSSubSec = (uint) ((ucaMSG_BUFF[OPMSG_IDX_TIME_SUBSEC_HI] << 8) | ucaMSG_BUFF[OPMSG_IDX_TIME_SUBSEC_LO]);

			// Set the new sub-second time equal to the GS time
			uiNewSubSec = uiGSSubSec;

			// Force an alarm if the new subsecond is outside of this slot
			if ((uiNewSubSec > SUBSLOT_ONE_END) || (uiNewSubSec > SUBSLOT_TWO_END) || (uiNewSubSec > SUBSLOT_THREE_END))
				ucFLAG2_BYTE.FLAG2_STRUCT.FLG2_T3_ALARM_MCH_BIT = 1;

			vTIME_setSubSecFromUint(uiNewSubSec);
			// Save the new time in Clk2, therefore Clk1 and alarms are still good
			vTIME_setClk2FromBytes((uchar *) &ucaMSG_BUFF[OPMSG_IDX_TIME_SEC_XI]);

			// Compute the difference then get the magnitude of the difference (satisfies break out condition)
			iDifference = uiNewSubSec - uiGSSubSec;
			if (iDifference < 0)
				iDifference *= -1;

			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_RESET_ALL_TIME_BIT = 1;
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_HAVE_WIZ_GROUP_TIME_BIT = 1;

#if 0
			vSERIAL_sout("RTS Snt = ", 10);
			vSERIAL_UI16out(uiRTSSubsec);
			vSERIAL_crlf();

			vSERIAL_sout("GS = ", 5);
			vSERIAL_UI16out(uiGSSubSec);
			vSERIAL_crlf();

			vSERIAL_sout("Old = ", 6);
			vSERIAL_UI16out(uiPrevSubsec);
			vSERIAL_crlf();

			vSERIAL_sout("New = ", 6);
			vSERIAL_UI16out(uiNewSubSec);
			vSERIAL_crlf();

			vSERIAL_sout("Diff = ", 7);
			vSERIAL_I16out(iDifference);
			vSERIAL_crlf();

#endif

		}
	}

	ucFreqAdjustIndex = 0;

	// Release the peripheral
	ucFLAG3_BYTE.FLAG3_STRUCT.FLG3_GSV_COM_BIT = 0;
}


//////////////////////////////////////////////////////////////////////////
//! \fn vGS_Init
//! \brief Initializes garden server communication variables
//!
//!
//!
/////////////////////////////////////////////////////////////////////////
void vGS_Init(void)
{
	// Initialize the last comm time variable
	g_ulLastGSCommTime = 0;

	g_ucFreqLocked = 0;
}
