
/***************************  FRAM.H  ****************************************
*
* Header for FRAM routine pkg
*
*
*
* V1.00 5/11/2003 wzr
*	started
*
******************************************************************************/

#ifndef FRAM_H_INCLUDED
	#define FRAM_H_INCLUDED

	/* FRAM SERIAL MEMORY COMMANDS */

	#define FRAM_WRITE_STS_REG_CMD  0x01
	#define FRAM_WRITE_DATA_CMD		0x02
	#define FRAM_READ_DATA_CMD		0x03
	#define FRAM_WRITE_DIS_CMD		0x04
	#define FRAM_READ_STS_REG_CMD	0x05
	#define FRAM_WRITE_ENA_CMD		0x06

	#define FRAM_MAX_ADDRESS			0x1F40 //8000

	/* ROUTINE DEFINITIONS */

	unsigned char ucFRAM_read_B8(
		unsigned int uiAddr,
		unsigned char *ucData
		);

	unsigned char ucFRAM_write_B8(
		unsigned int uiAddr,
		unsigned char ucData
		);

	unsigned char ucFRAM_read_B16(
		unsigned int uiAddr,
		unsigned int *uiData
		);

	unsigned char ucFRAM_write_B16(
		unsigned int uiAddr,
		unsigned int uiData
		);

	unsigned char ucFRAM_read_B32(
		unsigned int uiAddr,
		unsigned long *ulData
		);

	unsigned char ucFRAM_write_B32(
		unsigned int uiAddr,
		unsigned long ulData
		);

	void vFRAM_fillFramBlk(
		unsigned int uiStartAddr,
		unsigned int uiCount,
		unsigned char ucSetVal
		);

	unsigned char ucFRAM_chk_fram_blk(
		unsigned int uiStartAddr,
		unsigned int uiCount,
		unsigned char ucSetVal
		);

	void vFRAM_show_fram(
		unsigned int uiStartAddr,
		unsigned int uiCount
		);

	void vFRAM_Security(unsigned int uiStartAddr, unsigned int uiEndAddress);

#endif /* FRAM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
