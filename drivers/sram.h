
/***************************  SRAM.H  ****************************************
*
* Header for SRAM routine pkg
*
* NOTE: See Main.h for the disk layout map. 
*
*
*
* V1.00 10/18/2002 wzr
*	started
*
******************************************************************************/

#ifndef SRAM_H_INCLUDED
	#define SRAM_H_INCLUDED


	#define HI_DISK_BANK 1
	#define LO_DISK_BANK 0


	/* ROUTINE DEFINITIONS */

	uchar ucSRAM_read_B8(
		USL uslAddr			
		);

	void vSRAM_write_B8(
		USL uslAddr,		
		uchar ucDataByte
		);

	uint uiSRAM_read_B16(
		USL uslAddr			
		);

	void vSRAM_write_B16(
		USL uslAddr,		
		uint uiData
		);

	ulong ulSRAM_read_B32(
		USL uslAddr			
		);

	void vSRAM_write_B32(
		USL uslAddr,		
		ulong ulData
		);


#endif /* SRAM_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

