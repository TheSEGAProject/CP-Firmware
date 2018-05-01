
/***************************  CRC.H  ****************************************
*
* CRC calculator header file
*
* V1.00 10/07/2002 wzr
*	started
*
******************************************************************************/

#ifndef CRC_H_INCLUDED
  #define CRC_H_INCLUDED


#define CRC_FOR_MSG_TO_SEND 1
#define CRC_FOR_MSG_TO_REC  0



/* ROUTINE DEFINITIONS */

unsigned char ucCRC16_compute_msg_CRC(		/* RET:	1=CRC is OK, 0=CRC mismatch */
		unsigned char ucMsgFlag,	//send msg or receive msg flag
		volatile uchar *ucMSGBuff, 				//pointer to the message
		uchar ucLength						//length of the message
		);

unsigned int uiCRC16_ComputeBlockCRC(uchar *ucPointer, ulong ulLength);
unsigned int uiCRC16_CRC_on_memory(ulong *ulPointer, ulong ulLength);
unsigned int uiCRC16_ComputeCRCwithInit(uchar *ucPointer, ulong ulLength, uint uiInitialCRC);
#endif /* CRC_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
