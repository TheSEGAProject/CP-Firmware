
/**************************  FLASH.C  *****************************************
*
* Routines to handle low level FLASH control.
*
*
* V1.00 7/13/2004 wzr
*		Started
*
******************************************************************************/

/*lint -e526 */		/* function not defined */
/*lint -e657 */		/* Unusual (nonportable) anonymous struct or union */
/*lint -e714 */		/* symbol not referenced */
/*lint -e750 */		/* local macro not referenced */
/*lint -e754 */		/* local structure member not referenced */
/*lint -e755 */		/* global macro not referenced */
/*lint -e757 */		/* global declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include <msp430x54x.h>		//processor reg description */
#include "../std.h"			//standard definitions
#include "../hal/config.h" 		//system configuration description file
#include "../serial.h" 		//serial IO port stuff
#include "flash.h"			//FLASH control routines
//#include "DELAY.h"		//delay routines
//#include "SDCOM.h"			//SD communication routines

	/* FLASH SERIAL MEMORY COMMANDS */

	#define SPI_MODE_USED 0
//	#define SPI_MODE_USED 3


#if (SPI_MODE_USED == 0)
	#define FLASH_READ_STS_REG			0xD7


	#define FLASH_READ_MEM_TO_BUFF_1	0x53
	#define FLASH_READ_MEM_TO_BUFF_2	0x55
 
	#define FLASH_READ_BUFF_1_TO_USR	0xD4
	#define FLASH_READ_BUFF_2_TO_USR	0xD6



	#define FLASH_WRITE_USR_TO_BUFF_1	 0x84
	#define FLASH_WRITE_USR_TO_BUFF_2	 0x87

	#define FLASH_WRITE_BUFF_1_TO_MEM_WITH_ERASE 0x83
	#define FLASH_WRITE_BUFF_2_TO_MEM_WITH_ERASE 0x86

	#define FLASH_WRITE_USR_TO_BUFF_1_TO_MEM  0x82
	#define FLASH_WRITE_USR_TO_BUFF_2_TO_MEM  0x85

	#define FLASH_AUTO_PAGE_REWRITE_USING_BUFF_1	0x58
	#define FLASH_AUTO_PAGE_REWRITE_USING_BUFF_2	0x59


#endif


#if (SPI_MODE_USED == 3)
	#define FLASH_READ_STS_REG_CMD	0x57

	#define FLASH_READ_BUFF_1_CMD	0x54
	#define FLASH_READ_BUFF_2_CMD	0x56

	#define FLASH_WRITE_BUFF_1_CMD	0x84

	#define FLASH_WRITE_BUFF_2_CMD	0x87
#endif


static void vFLASH_wait_for_not_busy(
		void
		);




/******************  ucFLASH_init()  *******************************************
*
* Routine to initialize the FLASH for use
*
*
* RET:	1 = ok
* 		0 = err
*
******************************************************************************/

//uchar ucFLASH_init( // 1=OK, 0=ERR
//		void
//		)
//	{
//	// SETUP THE SPI REGS
//	vSPI_master_init(SPI_MODE_0, RATE_1);
//
//	return(1);
//
//	}// END: ucFLASH_init()
//



/******************  vFLASH_quit()  *******************************************
*
* Routine to shut off the FLASH
*
*
******************************************************************************/
//
//void vFLASH_quit(
//		void
//		)
//	{
//	/* SHUTDOWN THE SPI REGS */
//	vSPI_master_quit();
//
//	/* SELECT THE FLASH CHIP */
//	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;	//de-select FLASH
//
//	return;
//
//	}/* END: vFLASH_quit() */




/****************** ucFLASH_read_sts_reg()  ***********************************
*
* Return the status byte from the FLASH
*
*
******************************************************************************/

unsigned char ucFLASH_read_sts_reg(
		void
		)
	{
	unsigned char ucData;

	FLASH_SEL_OUT_PORT &= ~FLASH_SEL_BIT;		//select the chip

	vSPI_bout(FLASH_READ_STS_REG);				//send the read sts cmd

	ucData = ucSPI_bin();						//get the data
	
	/* DROP FLASH CHIP SELECT FOR NEXT CMD */
	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;		//deselect chip

	return(ucData);

	}/* END: ucFLASH_read_sts_reg() */








/****************** vFLASH_read_mem_to_buff()  ******************************
*
* Return the data at a specified address
*
******************************************************************************/

void vFLASH_read_mem_to_buff(
		unsigned char ucBuffNum,		//0 or 1
		unsigned int uiPageNum			//0 - 4095
		)
	{
	unsigned char ucPageHI;
	unsigned char ucPageLO;

	uiPageNum <<= 2;					//lint !e701  move page number to align to byte
	ucPageHI = (unsigned char)((uiPageNum & 0x3FFF)>>8);
	ucPageLO = (unsigned char) (uiPageNum & 0xFC);

	FLASH_SEL_OUT_PORT &= ~FLASH_SEL_BIT;			//select the chip

	/* SEND THE COMMAND */
	if(ucBuffNum == 0)
		vSPI_bout(FLASH_READ_MEM_TO_BUFF_1);
	else
		vSPI_bout(FLASH_READ_MEM_TO_BUFF_2);

	vSPI_bout(ucPageHI);				//HI addr
	vSPI_bout(ucPageLO);				//LO addr
	vSPI_bout(0);						//byte addr = 0

	/* DROP FLASH CHIP SELECT FOR NEXT CMD */
	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;;			//deselect chip

	/* WAIT TIL BUSY IS DROPPED */
	vFLASH_wait_for_not_busy();

	return;

	}/* END: vFLASH_read_mem_to_buff() */







/****************** ucFLASH_read_byte_from_buff()  ****************************
*
* Return a byte of data from one of the buffers
*
******************************************************************************/

unsigned char ucFLASH_read_byte_from_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiByteAddr				//0 - 527
		)
	{
	unsigned char ucByteAddrHI;
	unsigned char ucByteAddrLO;
	unsigned char ucData;

	ucByteAddrHI = (unsigned char)((uiByteAddr & 0x3FF)>>8);
	ucByteAddrLO = (unsigned char) (uiByteAddr & 0xFF);

	FLASH_SEL_OUT_PORT &= ~FLASH_SEL_BIT;			//select the chip

	/* SEND THE COMMAND */
	if(ucBuffNum == 0)
		vSPI_bout(FLASH_READ_BUFF_1_TO_USR);
	else
		vSPI_bout(FLASH_READ_BUFF_2_TO_USR);

	vSPI_bout(0);						//HI page = 0
	vSPI_bout(ucByteAddrHI);			//HI byte addr
	vSPI_bout(ucByteAddrLO);			//LO byte addr
	vSPI_bout(0);						//don't care byte

	ucData = ucSPI_bin();				//Read the data
	
	/* DROP FLASH CHIP SELECT FOR NEXT CMD */
	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;			//deselect chip

	return(ucData);

	}/* END: ucFLASH_read_byte_from_buff() */






/****************** uiFLASH_read_word_from_buff()  ***************************
*
* Return a word of data from one of the buffers
*
******************************************************************************/

unsigned int uiFLASH_read_word_from_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiStartByteAddr		//0 - 527
		)
	{
	unsigned char ucByteAddrHI;
	unsigned char ucByteAddrLO;
	unsigned int uiData;

	ucByteAddrHI = (unsigned char)((uiStartByteAddr & 0x3FF)>>8);
	ucByteAddrLO = (unsigned char) (uiStartByteAddr & 0xFF);

	FLASH_SEL_OUT_PORT &= ~FLASH_SEL_BIT;			//select the chip

	/* SEND THE COMMAND */
	if(ucBuffNum == 0)
		vSPI_bout(FLASH_READ_BUFF_1_TO_USR);
	else
		vSPI_bout(FLASH_READ_BUFF_2_TO_USR);

	vSPI_bout(0);						//HI page = 0
	vSPI_bout(ucByteAddrHI);			//HI byte addr
	vSPI_bout(ucByteAddrLO);			//LO byte addr
	vSPI_bout(0);						//don't care byte

	uiData = (unsigned int)ucSPI_bin();			//Read data HI
	uiData <<= 8;
	uiData |= (unsigned int)ucSPI_bin();		//read data LO
	
	/* DROP FLASH CHIP SELECT FOR NEXT CMD */
	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;			//deselect chip

	return(uiData);

	}/* END: ucFLASH_read_word_from_buff() */






/****************** vFLASH_write_buff_to_mem()  ******************************
*
* write the data to a specified page
*
******************************************************************************/

void vFLASH_write_buff_to_mem(
		unsigned char ucBuffNum,		//0 or 1
		unsigned int uiPageNum			//0 - 4095
		)
	{
	uchar ucPageHI;
	uchar ucPageLO;

	uiPageNum <<= 2;					//lint !e701  move page number to align to byte
	ucPageHI = (unsigned char)((uiPageNum & 0x3FFF)>>8);
	ucPageLO = (unsigned char) (uiPageNum & 0xFC);

	FLASH_SEL_OUT_PORT &= ~FLASH_SEL_BIT;			//select the chip

	/* SEND THE COMMAND */
	if(ucBuffNum == 0)
		vSPI_bout(FLASH_WRITE_BUFF_1_TO_MEM_WITH_ERASE);
	else
		vSPI_bout(FLASH_WRITE_BUFF_2_TO_MEM_WITH_ERASE);

	vSPI_bout(ucPageHI);				//HI addr
	vSPI_bout(ucPageLO);				//LO addr
	vSPI_bout(0);						//byte addr = 0

	/* DROP FLASH CHIP SELECT FOR NEXT CMD */
	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;			//deselect chip

	/* WAIT TIL BUSY IS DROPPED */
	vFLASH_wait_for_not_busy();

	return;

	}/* END: vFLASH_write_buff_to_mem() */








/****************** vFLASH_write_byte_to_buff()  *****************************
*
* Return a byte of data from one of the buffers
*
******************************************************************************/

void vFLASH_write_byte_to_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiByteAddr,			//0 - 527
		unsigned char ucDataByte
		)
	{
	unsigned char ucByteAddrHI;
	unsigned char ucByteAddrLO;

	ucByteAddrHI = (unsigned char)((uiByteAddr & 0x3FF)>>8);
	ucByteAddrLO = (unsigned char) (uiByteAddr & 0xFF);

	FLASH_SEL_OUT_PORT &= ~FLASH_SEL_BIT;			//select the chip

	/* SEND THE COMMAND */
	if(ucBuffNum == 0)
		vSPI_bout(FLASH_WRITE_USR_TO_BUFF_1);
	else
		vSPI_bout(FLASH_WRITE_USR_TO_BUFF_2);

	vSPI_bout(0);						//HI page = 0
	vSPI_bout(ucByteAddrHI);			//HI byte addr
	vSPI_bout(ucByteAddrLO);			//LO byte addr
	vSPI_bout(ucDataByte);				//Write the data
	
	/* DROP FLASH CHIP SELECT FOR NEXT CMD */
	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;			//deselect chip

	return;

	}/* END: vFLASH_write_byte_to_buff() */








/****************** vFLASH_write_word_to_buff()  *****************************
*
* Return a byte of data from one of the buffers
*
******************************************************************************/

void vFLASH_write_word_to_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiStartByteAddr,		//0 - 527
		unsigned int uiDataByte
		)
	{
	unsigned char ucByteAddrHI;
	unsigned char ucByteAddrLO;
	unsigned char ucDataByteHI;
	unsigned char ucDataByteLO;

	ucByteAddrHI = (unsigned char)((uiStartByteAddr & 0x3FF)>>8);
	ucByteAddrLO = (unsigned char) (uiStartByteAddr & 0xFF);

	ucDataByteHI = (unsigned char)  ((uiDataByte >> 8) & 0xFF);
	ucDataByteLO = (unsigned char)   (uiDataByte & 0xFF);



	FLASH_SEL_OUT_PORT &= ~FLASH_SEL_BIT;			//select the chip

	/* SEND THE COMMAND */
	if(ucBuffNum == 0)
		vSPI_bout(FLASH_WRITE_USR_TO_BUFF_1);
	else
		vSPI_bout(FLASH_WRITE_USR_TO_BUFF_2);

	vSPI_bout(0);						//HI page = 0
	vSPI_bout(ucByteAddrHI);			//HI byte addr
	vSPI_bout(ucByteAddrLO);			//LO byte addr
	vSPI_bout(ucDataByteHI);			//Write the data
	vSPI_bout(ucDataByteLO);			//Write the data
	
	/* DROP FLASH CHIP SELECT FOR NEXT CMD */
	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;			//deselect chip

	return;

	}/* END: vFLASH_write_word_to_buff() */







/****************** vFLASH_auto_rewrite_single_page()  ******************************
*
* auto rewrite the data to a specified page
*
******************************************************************************/

void vFLASH_auto_rewrite_single_page(
		unsigned char ucBuffNum,		//0 or 1
		unsigned int uiPageNum			//0 - 4095
		)
	{
	unsigned char ucPageHI;
	unsigned char ucPageLO;

	uiPageNum <<= 2;					//lint !e701  move page number to align to byte
	ucPageHI = (unsigned char)((uiPageNum & 0x3FFF)>>8);
	ucPageLO = (unsigned char) (uiPageNum & 0xFC);

	FLASH_SEL_OUT_PORT &= ~FLASH_SEL_BIT;			//select the chip

	/* SEND THE COMMAND */
	if(ucBuffNum == 0)
		vSPI_bout(FLASH_AUTO_PAGE_REWRITE_USING_BUFF_1);
	else
		vSPI_bout(FLASH_AUTO_PAGE_REWRITE_USING_BUFF_2);
	vSPI_bout(ucPageHI);				//HI addr
	vSPI_bout(ucPageLO);				//LO addr
	vSPI_bout(0);						//byte addr = 0

	/* DROP FLASH CHIP SELECT FOR NEXT CMD */
	FLASH_SEL_OUT_PORT |= FLASH_SEL_BIT;			//deselect chip

	/* WAIT TIL BUSY IS DROPPED */
	vFLASH_wait_for_not_busy();

	return;

	}/* END: vFLASH_auto_rewrite_single_page() */







/****************** vFLASH_auto_rewrite_entire_disk()  ******************************
*
* auto rewrite the data to a specified page
*
******************************************************************************/

void vFLASH_auto_rewrite_entire_disk(
		void
		)
	{
	unsigned int uii;

	for(uii=0;  uii<4096;  uii++)
		{
		vFLASH_auto_rewrite_single_page(FLASH_BUFFER_2,uii);
		}/* END: for() */

	return;

	}/* END: vFLASH_auto_rewrite_entire_disk() */





/*******************  vFLASH_wait_for_not_busy()  *****************************
*
* Wait til the busy bit is dropped.
*
******************************************************************************/

static void vFLASH_wait_for_not_busy(
		void
		)
	{
	usl uslCounter;
	#define BUSY_LOOP_COUNT ((usl)50000)


	/* WAIT TIL BUSY IS DROPPED */
	for(uslCounter = 0;  uslCounter <BUSY_LOOP_COUNT;  uslCounter++)
		{
		if(ucFLASH_read_sts_reg() & 0x80) break;
		}
	if(uslCounter >= BUSY_LOOP_COUNT)
		{
		vSERIAL_sout("FlshBsyTout\r\n", 13);
		}

	return;
	}



/*-------------------------------  MODULE END  ------------------------------*/
