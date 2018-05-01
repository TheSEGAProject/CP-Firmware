/*
 * SP_UART.h
 *
 *  Created on: Aug 16, 2016
 *      Author: cp397
 */

#ifndef DRIVERS_SP_UART_H_
#define DRIVERS_SP_UART_H_

uint8 ucSP_UARTInit(uchar ucSPNumber);
uint8 ucSP_UART_RXByte(void);
void vSP_UART_TXByte(uint8 ucTXChar);
uint8 ucSP_UARTSetCommState(uint16 ucBaud, uint8 ucSPNumber);
void vSP_UART_ResetRXBufferIndex(void);

#endif /* DRIVERS_SP_UART_H_ */
