
/***************************  FLASH.H  ****************************************
*
* Header for FLASH routine pkg
*
*
*
* V1.00 7/13/2004 wzr
*	started
*
******************************************************************************/

#ifndef FLASH_H_INCLUDED
	#define FLASH_H_INCLUDED

	/* DEFINE THE FLASH BUFFERS FOR EASE OF READING */
	#define FLASH_BUFFER_1 0
	#define FLASH_BUFFER_2 1


	/* ROUTINE DEFINITIONS */

	uchar ucFLASH_init( // 1=ok, 0=err
		void
		);

	void vFLASH_quit(
		void
		);

	unsigned char ucFLASH_read_sts_reg(
		void
		);

	void vFLASH_read_mem_to_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiPageNum				//0 - 4095
		);

	unsigned char ucFLASH_read_byte_from_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiByteAddr				//0 - 527
		);

	unsigned int uiFLASH_read_word_from_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiStartByteAddr		//0 - 527
		);

	void vFLASH_write_buff_to_mem(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiPageNum				//0 - 4095
		);

	void vFLASH_write_byte_to_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiByteAddr,			//0 - 527
		unsigned char ucDataByte
		);

	void vFLASH_write_word_to_buff(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiStartByteAddr,		//0 - 527
		unsigned int uiDataByte
		);

	void vFLASH_auto_rewrite_single_page(
		unsigned char ucBuffNum,			//0 or 1
		unsigned int uiPageNum				//0 - 4095
		);

	void vFLASH_auto_rewrite_entire_disk(
		void
		);


#endif /* FLASH_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
