/*
 * usci_A0_uart.h
 *
 *  Created on: Mar 8, 2016
 *      Author: cp397
 */

#ifndef HAL_USCI_A0_UART_H_
#define HAL_USCI_A0_UART_H_

//! \def USCI_A0_BUFFER_LEN
//! \brief Buffer size for the A0 UART peripheral
#define USCI_A0_BUFFER_LEN	0x40

void vUSCI_A0_UART_init(void);
void vUSCI_A0_UART_quit(void);
void vUSCI_A0_UART_CleanBuff(void);
void vUSCI_A0_UART_SendByte(uchar ucChar);
void vUSCI_A0_UART_SendBytes(uchar *ucChar, uchar ucByteCount);
uchar ucUSCI_A0_UART_ReceiveByte(uchar *ucChar);
uchar ucUSCI_A0_UART_ReadBuffer(uchar *ucMsgBuff, uchar ucMaxLength);
uchar ucUSCI_A0_UART_ReadByte(uchar ucIndex);
uchar ucUSCI_A0_UART_GetIndex(void);
void vUSCI_A0_UART_SetIndex(uchar);
#endif /* HAL_USCI_A0_UART_H_ */
