
/***************************  GID.H  ****************************************
*
* Group ID handlers
*
*
* V1.00 03/18/2005 wzr
*	started
*
******************************************************************************/

#ifndef GID_H_INCLUDED
 #define GID_H_INCLUDED

 #define GID_SELECTOR_MASK 0xE000
 #define GID_SELECTOR_NOT_MASK 0x1FFF


 void vGID_init(
		void
		);

 uint uiGID_getWholeSysGidAsUint(
		void
		);

 uchar ucGID_getWholeSysGidHiByte(
 		void
		);

 uchar ucGID_getWholeSysGidLoByte(
 		void
		);

 uchar ucGID_getSysGrpSelectAsByte(
 		void
		);



 void vGID_copyWholeSysGidToBytes(
		uchar *ucpToPtr
		);

 void vGID_setWholeSysGidFromBytes(
		uchar *ucpFromPtr
		);


 uchar ucGID_compareWholeSysGidToBytes( //RET: 1=match, 0=no match
		uchar *ucpComparePtr,			//ptr to Foreign GID
		uchar ucShowErrorFlag,			//YES_SHOW_ERR, NO_SHOW_ERR
		uchar ucCRLF_flag				//YES_CRLF, NO_CRLF
		);

 uchar ucGID_compareOnlySysGrpSelectToBytes( //RET: 1=match, 0=no match
		uchar *ucpComparePtr,			//ptr for foreign GID
		uchar ucShowErrorFlag,			//YES_SHOW_ERR, NO_SHOW_ERR
		uchar ucCRLF_flag				//YES_CRLF, NO_CRLF
		);

uchar ucGID_compareOnlySysGidToBytes( //RET: 1=match, 0=no match
		uchar *ucpComparePtr,			//ptr for foreign GID
		uchar ucShowErrorFlag,			//YES_SHOW_ERR, NO_SHOW_ERR
		uchar ucCRLF_flag				//YES_CRLF, NO_CRLF
		);


#endif /* GID_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
