
//------------------------  BIGSUB.C  ---------------------------------------
//
// Six byte difference package
//
//
//----------------------------------------------------------------------------

 #include "bigsub.h"
 #include "std.h"
 


/*******************  EXTERNS  ***********************************************/

extern volatile union					//ucFLAG0_BYTE
  {
  uchar byte;
  struct
    {
	unsigned FLG0_BIGSUB_CARRY_BIT:1;		//bit 0 ;1=CARRY, 0=NO-CARRY
	unsigned FLG0_BIGSUB_6_BYTE_Z_BIT:1;	//bit 1 ;1=all diff 0, 0=otherwise
	unsigned FLG0_BIGSUB_TOP_4_BYTE_Z_BIT:1;//bit 2 ;1=top 4 bytes 0, 0=otherwise
	unsigned FLG0_NOTUSED_3_BIT:1;			//bit 3 ;1=SOM2 link exists, 0=none
											//SET:	when any SOM2 links exist
											//CLR: 	when the SOM2 link is lost
	unsigned FLG0_RESET_ALL_TIME_BIT:1;		//bit 4 ;1=do time  reset, 0=dont
											//SET:	when RDC4 gets finds first
											//		SOM2.
											//		or
											//		In a Hub when it is reset.
											//
											//CLR: 	when vMAIN_computeDispatchTiming()
											//		runs next.
	unsigned FLG0_SERIAL_BINARY_MODE_BIT:1;	//bit 5 1=binary mode, 0=text mode
	unsigned FLG0_HAVE_WIZ_GROUP_TIME_BIT:1;//bit 6 1=Wizard group time has
											//        been aquired from a DC4
											//      0=We are using startup time
	unsigned FLG0_NOTUSED7_BIT:1;			//bit 7
	}FLAG0_STRUCT;
  }ucFLAG0_BYTE;

extern volatile uchar ucaBigMinuend[6];
extern volatile uchar ucaBigSubtrahend[6];
extern volatile uchar ucaBigDiff[6];


//  vBIGSUB_doBigDiff CODE

//------------------------  vBIGSUB_doBigDiff()  -----------------------------
//
// Calculate the 6 byte difference
//
// RET: none
//
// ucaBigMinuend - ucaBigSubtrahend = ucaBigDiff
//
// This may need to be looked at closer because it seems that it shouldn't work.
//------------------------------------------------------------------------------

 void vBIGSUB_doBigDiff(
 						void
						)
	{
		
		// Assume zero difference for 6byte and 4byte
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT = 1;
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT = 1;
		// Assume no carry
		ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_CARRY_BIT = 0;
		
		ucaBigDiff[5] = ucaBigMinuend[5] - ucaBigSubtrahend[5];
		if(ucaBigDiff[5])ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT = 0;
		ucaBigDiff[4] = ucaBigMinuend[4] - ucaBigSubtrahend[4];
		if(ucaBigDiff[4])ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT = 0;
		ucaBigDiff[3] = ucaBigMinuend[3] - ucaBigSubtrahend[3];
		if(ucaBigDiff[3])
		{
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT = 0;
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT = 0;
		}
		ucaBigDiff[2] = ucaBigMinuend[2] - ucaBigSubtrahend[2];
		if(ucaBigDiff[2])
		{
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT = 0;
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT = 0;
		}
		ucaBigDiff[1] = ucaBigMinuend[1] - ucaBigSubtrahend[1];
		if(ucaBigDiff[1])
		{
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT = 0;
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT = 0;
		}
		ucaBigDiff[0] = ucaBigMinuend[0] - ucaBigSubtrahend[0];
		if(ucaBigDiff[0])
		{
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_6_BYTE_Z_BIT = 0;
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_TOP_4_BYTE_Z_BIT = 0;
		}
		if(ucaBigSubtrahend[0] > ucaBigMinuend[0])
			ucFLAG0_BYTE.FLAG0_STRUCT.FLG0_BIGSUB_CARRY_BIT = 1;
		
	}
		
// -----------------------  END OF MODULE  ------------------------------- 
