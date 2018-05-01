
/***************************  STD.H  ****************************************
*
* STD port header file
*
*
* V1.00 04/29/2002 wzr
*	started
*
******************************************************************************/

#ifndef STD_H_INCLUDED
	#define STD_H_INCLUDED

#define DEBUG

	#define NULL			0
	
	#define TRUE 			1
	#define FALSE 			0

	#ifndef YES
	  #define YES 			1
	#endif
	#ifndef NO
	  #define NO 			0
	#endif

	#define ON 				1
	#define OFF 			0

	#define NONE 			0

	#define YES_INITQUIT 	1
	#define NO_INITQUIT 	0
	
	typedef unsigned short 	USHORT;
	typedef unsigned short 	ushort;
	
	typedef unsigned char 	uint8;
	typedef signed char 	int8;
	typedef unsigned char 	UCHAR;
	typedef unsigned char 	uchar;
	
	typedef unsigned int 	uint16;
	typedef signed int 		int16;
	typedef unsigned int  	UINT;
	typedef unsigned int  	uint;
	
	typedef unsigned long 	uint32;
	typedef signed long 	int32;
	typedef unsigned long 	ULONG;
	typedef unsigned long 	ulong;
	typedef unsigned long 	USL; // Formerly was unsigned short long, now unsigned long
	typedef unsigned long 	usl; // Formerly was unsigned short long, now unsigned long
	
	
#endif /* STD_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
