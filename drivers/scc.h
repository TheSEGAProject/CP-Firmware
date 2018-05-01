/*
 * scc.h
 *
 *  Created on: Mar 8, 2016
 *      Author: cp397
 */

#ifndef CP_FIRMWARE4D_DRIVERS_SCC_H_
#define CP_FIRMWARE4D_DRIVERS_SCC_H_


void vSCC_Init(void);
uint8 ucSCC_SendCommand( uint8 ucPayloadLen, uint8 * p_ucCmdPayload);
uint8 ucSCC_RequestData(void);
uint8 ucSCC_GrabMessageFromBuffer(union SP_DataMessage * message);
uint8 ucSCC_WaitForMessage(void);
uchar ucSCC_SetState(uchar ucState);
uchar ucSCC_GetState(uchar *ucState);
uchar ucSCC_IsAttached(void);

uchar ucSCC_GetSampleDuration(void);
uint8 ucSCC_FetchMsgVersion(void);
uint8 ucSCC_GetHID(uchar * ucSCC_HID);
uint8 ucSCC_SetHID(uchar * ucSCC_HID);
#endif /* CP_FIRMWARE4D_DRIVERS_SCC_H_ */
