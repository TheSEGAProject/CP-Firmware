
/***************************  L2FLASH.H  ****************************************
*
* Header for L2FLASH routine pkg
*
*
* V1.00 06/18/2003 wzr
*	started
*
******************************************************************************/

#ifndef L2FLASH_H_INCLUDED

#include "../std.h"			//standard definitions

 #define L2FLASH_H_INCLUDED

 #define FLASH_CHK_REPORT_MODE	1
 #define FLASH_CHK_SILENT_MODE	0
 

 #define FLASH_RESERVED_PAGES_L 8L
 #define FLASH_MSG_SIZE_L		MAX_MSG_SIZE_L
 #define FLASH_MSG_SIZE_MASK_L	MAX_MSG_SIZE_MASK_L

 #define FLASH_MAX_AVAIL_PAGES_L (FLASH_MAX_SIZE_IN_PAGES_L - FLASH_RESERVED_PAGES_L)
 #define FLASH_MAX_MSG_COUNT_L (FLASH_MAX_AVAIL_PAGES_L *(FLASH_BYTES_PER_PAGE_L/FLASH_MSG_SIZE_L))

 #define FLASH_FIRST_LINEAR_ADDR_L (FLASH_RESERVED_PAGES_L << 9)
 #define FLASH_MAX_LINEAR_ADDR_L (FLASH_MAX_SIZE_IN_PAGES_L << 9)

 #define FLASH_LINEAR_ADDR_PAGE_MASK_L ((FLASH_MAX_SIZE_IN_PAGES_L -1L) << 9)
 #define FLASH_LINEAR_ADDR_BYTE_MASK_L (FLASH_BYTES_PER_PAGE_L -1L)
 #define FLASH_LINEAR_ADDR_MSG_MASK_L (FLASH_LINEAR_ADDR_BYTE_MASK_L & (~MAX_MSG_SIZE_MASK_L))
 #define FLASH_LINEAR_ADDR_FULL_MASK_L ((FLASH_MAX_SIZE_IN_PAGES_L << 9) -1)


 #define FLASH_REFRESH_PAGE_MASK_UI  ((unsigned int)(FLASH_MAX_SIZE_IN_PAGES_I -1))


 /* ROUTINE DEFINITIONS */

 uchar ucL2FLASH_chkForFlash(
		uchar ucReportMode,	//FLASH_CHK_SILENT_MODE, FLASH_CHK_REPORT_MODE
		uchar ucInitQuitFlag /* YES_INITQUIT, NO_INITQUIT */
		);

 void vL2FLASH_storeMsgToFlash(
		void
		);

 void vL2FLASH_getNextMsgFromFlash(
		void
		);


#endif /* L2FLASH_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
