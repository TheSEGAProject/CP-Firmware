
/***************************  SPI.H  ****************************************
*
* Header for SPI routine pkg
*
*
*
* V1.00 5/11/2003 wzr
*	started
*
******************************************************************************/

#ifndef SPI_H_INCLUDED
	#define SPI_H_INCLUDED


	#define SPI_MODE_0		0
	#define SPI_MODE_1		1
	#define SPI_MODE_2		2
	#define SPI_MODE_3		3

	#define RATE_0				0
	#define RATE_1				1


	/* ROUTINE DEFINITIONS */

	void vSPI_Init(
		unsigned char ucMode, unsigned char ucRate
		);

	void vSPI_Quit(
		void
		);

	unsigned char ucSPI_bin(
		void
		);

	void vSPI_bout(
		unsigned char ucOutByte
		);

void vSPI_CleanBuff(unsigned char * p_ucBuff, unsigned char ucLength);
void vSPI_TXBytes(unsigned char * p_ucTXData, unsigned int ucByteCount);
void vSPI_RX_Bytes(unsigned char * p_ucRXData, unsigned int ucByteCount);

void vSPI_TXBytes_InterruptDriven(unsigned char * p_ucTXData, unsigned int ucByteCount);
#endif /* SPI_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
