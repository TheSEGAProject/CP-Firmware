//
//
///**************************  DRADIO.C  ****************************************
//*
//* Routines to perform Diagnostic RADIO during events
//*
//*
//* V1.00 10/04/2003 wzr
//*		Started
//*
//******************************************************************************/
//
//
///*lint -e526 */		/* function not defined */
///*lint -e657 */		/* unusual (nonportable) anonymous struct or union */
///*lint -e714 */		/* symbol not referenced */
///*lint -e716 */		/* while(1)... */
///*lint -e750 */		/* local macro not referenced */
///*lint -e754 */		/* local structure member not referenced */
///*lint -e755 */		/* global macro not referenced */
///*lint -e757 */		/* global declarator not referenced */
///*lint -e752 */		/* local declarator not referenced */
///*lint -e758 */		/* global union not referenced */
///*lint -e768 */		/* global struct member not referenced */
//
//
//#include "std.h"			//standard defines
//#include "misc.h"			//homeless functions
//#include "crc.h"			//CRC calculation module
//#include "serial.h" 		//serial IO port stuff
//#include "dradio.h"			//DRADIO generator routines
//#include "fulldiag.h"		//full blown diagnostic defines
//#include "delay.h"			//delay routines
//#include "comm.h"			//radio msg helper routines
//#include "RAD40.h"			//Radix 40 conversion routines
//#include "drivers/buz.h"	//buzzer routines
//#include "drivers/adf7020.h"  		//event RADIO module
//#include "hal/config.h" 	//system configuration description file
//
//extern volatile unsigned char ucaMSG_BUFF[ MAX_RESERVED_MSG_SIZE ];
//
////extern unsigned char ucGLOB_CurMsgSeqNum;
//
//extern volatile union							//ucFLAG1_BYTE
//		{
//		unsigned char byte;
//
//		struct
//			{
//			unsigned FLG1_X_DONE_BIT:		1;	//bit 0
//			unsigned FLG1_X_LAST_BIT_BIT:	1;	//bit 1
//			unsigned FLG1_X_FLAG_BIT:		1;	//bit 2 ;1=XMIT, 0=RECEIVE
//			unsigned FLG1_R_HAVE_MSG_BIT:	1;	//bit 3	;1=REC has a msg, 0=no msg
//			unsigned FLG1_R_CODE_PHASE_BIT: 1;	//bit 4 ;1=MSG PHASE, 0=BARKER PHASE
//			unsigned FLG1_R_ABORT_BIT:		1;	//bit 5
//			unsigned FLG1_X_NXT_LEVEL_BIT:	1;	//bit 6
//			unsigned FLG1_R_SAMPLE_BIT: 	1;	//bit 7
//			}FLAG1_STRUCT;
//
//		}ucFLAG1_BYTE;
//
//
//
//
//
//
//
///* this packed has same first bytes as discovery 1 packet */
//static unsigned char ucTestMsg[32] =
//	{
//	31,						// 0 msg SIZE
//	MSG_TYPE_TS1,			// 1 msg TYPE
//	50,						// 2 GROUP ID HI
//	50,						// 3 GROUP ID LO
//	00,						// 4 SERIAL MD
//	32,						// 5 SERIAL LO
//	00,						// 6 DEST ID HI
//	05,						// 7 DEST ID LO
//	0,						// 8 msg seq number to chk for lost msgs
//	0xAA,					// 9	// 1010 1010
//	0xF0,					// 10	// 1111 0000
//	0x0F, 					// 11	// 0000 1111
//	0x00,					// 12	// 0000 0000
//	0xFF,					// 13	// 1111	1111
//	0xCC,					// 14	// 1100 1100
//	0x33,					// 15	// 0011 0011
//	0,						// 16 marker
//	0x1C,					// 17	// 0001 1100
//	0xE3,					// 18	// 1110 0011
//	0xA0,					// 19	// 1010 0000
//	0x5F,					// 20	// 0101 1111
//	0xFA,					// 21	// 1111 1010
//	0x05,					// 22	// 0000 0101
//	0,						// 23 marker
//	0xAA,					// 24	// 1010 1010
//	0xF0,					// 25	// 1111 0000
//	0x0F,					// 26	// 0000 1111
//	0x00,					// 27	// 0000 0000
//	0xFF,					// 28	// 1111 1111
//	0xCC,					// 29	// 1100 1100
//	0,						// 30 CRC HI
//	0						// 31 CRC LO
//	}; /* END: ucTestMsg[] */
//
//
//
//
//
///*****************************  CODE STARTS HERE  ****************************/
//
//
//
//
//
///*****************  ucDRADIO_continuousXmitTestMsg() *************************
//*
//* continuously transmit the test message
//*
//*
//******************************************************************************/
//
//unsigned char ucDRADIO_continuousXmitTestMsg(
//		uchar ucChanIdx
//		)
//	{
//	unsigned char ucc;
//	unsigned char ucChar;
//	unsigned char ucRetVal;
//
//
//	/* SETUP RADIO */
//	vADF7020_WakeUp();
//
//	/* SETUP THE RADIO FREQUENCY */
////	vRADIO_setBothFrequencys(
////				ucRADIO_getChanAssignment(ucChanIdx),
////				NO_FORCE_FREQ
////				);
//	unADF7020_SetChannel(TEST_CHANNEL);
//
//	/* ASSUME NO ERRORS */
//	ucRetVal = DIAG_ACTION_KEY_NONE;
//
//
//	/* SETUP THE INITIAL THE MSG */
//	for(ucc=0;  ucc<32;  ucc++)
//		{
//		ucaMSG_BUFF[ucc] = ucTestMsg[ucc];
//		}
//
//	/* LOOP TRANSMITTING EACH LOOP */
//	while(TRUE)		//lint !e774
//		{
//		/* CHECK FOR THE EXIT COMMAND */
//		if(ucSERIAL_kbhit())
//			{
//			ucChar = ucSERIAL_bin();
//
//			ucRetVal = DIAG_ACTION_KEY_EXIT;	//assume an exit
//			if(ucChar == 0x1B) break; //escape
//			if(ucChar == 'q') break;
//			if(ucChar == 'Q') break;
//			if(ucChar == 'x') break;
//			if(ucChar == 'X') break;
//
//			ucRetVal = DIAG_ACTION_KEY_NONE;	//assume no exit
//
//			}/* END: if() */
//
//		/* SEND MSG */
//		vADF7020_SendMsg();
//
//		/* SEND AN INDICATOR TO OPERATOR */
//		vSERIAL_bout('.');
//
//
//		/* CHANGE THE MSG FOR THE NEXT TRANSMISSION */
//		/* STUFF NEW SEQ NUMBER */
//		ucaMSG_BUFF[TS1_IDX_MSG_SEQ_NUM] = ucMSG_incMsgSeqNum();
//		/* CALCULATE THE CRC */
//		ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_SEND);	//lint !e534
//
//		vDELAY_wait100usTic(5000);		//wait 1/2 sec
//
//		}/* END: while() */
//
//	/* MAKE SURE RADIO IS SHUT OFF */
//	vADF7020_Quit();
//
//	return(ucRetVal);
//
//	}/* END: ucDRADIO_continuousXmitTestMsg() */
//
//
//
//
//
//
//
//
///*****************  ucDRADIO_continuousReceiveTestMsg() **********************
//*
//* continuously receive the test message
//*
//*
//******************************************************************************/
//
//unsigned char ucDRADIO_continuousReceiveTestMsg(
//		uchar ucChanIdx
//		)
//	{
//	#define XI_SOUND		1600	//OK
//	#define VHI_SOUND		1500
//	#define HI_SOUND		1400
//	#define MD_SOUND		1300
//	#define LO_SOUND		1200
//	#define VLO_SOUND		1000
//	#define BAD_SOUND		 600	//bad
//
//	#define EIGHTH_NOTE   	500
//
//	#define MAX_WAIT_TIME_UL 14000UL
//	unsigned char ucc;
//	unsigned char ucChar;
//	unsigned char ucRetVal;
//	unsigned int uiRSSI;
//	unsigned long ull;
//	uchar ucErrCount;
//	const char *cpStrPtr;
//	uint uiSound;
//
//
//	/* SETUP RADIO */
//	vADF7020_WakeUp();
//
//	/* SETUP THE RADIO FREQUENCY */
////	vRADIO_setBothFrequencys(
////				ucRADIO_getChanAssignment(ucChanIdx),
////				NO_FORCE_FREQ
////				);
//	unADF7020_SetChannel(TEST_CHANNEL);
//
//	/* ASSUME NO ERRORS */
//	ucRetVal = DIAG_ACTION_KEY_NONE;
//
//	/* SETUP TO READ THE RSSI VALUE OF THE RECEIVED MSG */
//	vAD_init();
//	vAD_setup_analog_bits(RSSI_AD_CHAN);
//	vAD_select_chan(RSSI_AD_CHAN, NO_WAIT);
//
//	while(TRUE)		//lint !e774
//		{
//		/* CHECK FOR THE EXIT COMMAND */
//		if(ucSERIAL_kbhit())
//			{
//			ucChar = ucSERIAL_bin();
//
//			ucRetVal = DIAG_ACTION_KEY_EXIT;	//assume an exit
//			if(ucChar == 0x1B) break; //escape
//			if(ucChar == 'q') break;
//			if(ucChar == 'Q') break;
//			if(ucChar == 'x') break;
//			if(ucChar == 'X') break;
//
//			ucRetVal = DIAG_ACTION_KEY_NONE;	//assume no exit
//
//			}/* END: if() */
//
//
//		/* STARTUP THE RECEIVER */
//		unADF7020_SetRadioState(RX_IDLE);
//
//		/* WAIT FOR THE BARKER TO BE DETECTED */
//		for(ull=0;  ull<MAX_WAIT_TIME_UL;  ull++)
//			{
//			if(ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_CODE_PHASE_BIT) break;
//			}/* END: while() */
//
//		if(ull >= MAX_WAIT_TIME_UL)
//			{
//			vADF7020_abort_receiver();
//			vSERIAL_rom_sout("Tout1\r\n");
//			vBUZ_morrie();
//			/* CHECK FOR THE EXIT COMMAND */
//			continue;
//			}
//		vSERIAL_rom_sout("B,");				//tell that we found a barker code
//
//
//		/* MEASURE THE RSSI FOR THIS MESSAGE */
//		uiRSSI = uiAD_read(RSSI_AD_CHAN, NO_SELECT_CHAN);
//
//		/* WAIT FOR MESSAGE COMPLETE */
//		for(ull=0;  ull<MAX_WAIT_TIME_UL;  ull++)
//			{
//			if(ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT) break;
//			}/* END: while() */
//		if(ull >= MAX_WAIT_TIME_UL)
//			{
//			vADF7020_abort_receiver();
//			vSERIAL_rom_sout("Tout2\r\n");
//			vBUZ_morrie();
//			continue;
//			}
//		vSERIAL_rom_sout("M,");			//tell that we have a msg
//
//
//
//		/* TELL ABOUT RSSI */
//		vSERIAL_rom_sout("R=");
//		vSERIAL_UIV16out(uiRSSI);			//tell the RSSI value
//		vSERIAL_bout('(');
//		cpStrPtr = "XI ";
//		uiSound = XI_SOUND;
//		if(uiRSSI < 500)
//			{
//			cpStrPtr = "VHI";
//			uiSound = VHI_SOUND;
//			}
//		if(uiRSSI < 400)
//			{
//			cpStrPtr = "HI ";
//			uiSound = HI_SOUND;
//			}
//		if(uiRSSI < 300)
//			{
//			cpStrPtr = "MD ";
//			uiSound = MD_SOUND;
//			}
//		if(uiRSSI < 200)
//			{
//			cpStrPtr = "LO ";
//			uiSound = LO_SOUND;
//			}
//		if(uiRSSI < 100)
//			{
//			cpStrPtr = "VLO";
//			uiSound = VLO_SOUND;
//			}
//		vSERIAL_rom_sout(cpStrPtr);
//		vSERIAL_rom_sout("),");
//
//
//		/* LOOP BACK IF ITS A GOOD MESSAGE */
//		if(ucCRC16_compute_msg_CRC(CRC_FOR_MSG_TO_REC))
//			{
//			/* MSG IS GOOD */
//			vSERIAL_rom_sout("OK\r\n");
//
//			/* BEEP */
//			vBUZ(uiSound,EIGHTH_NOTE);			//good
//
//			continue;
//			}/* END: if() */
//
//
//
//		/* MSG HAS AN ERROR -- FIND OUT HOW MANY */
//		ucErrCount = 0;
//		/* CHK FIRST PART OF MSG FOR BIT ERRS */
//		for(ucc=0;  ucc<8;  ucc++)
//			{
//			if(ucaMSG_BUFF[ucc] != ucTestMsg[ucc]) ucErrCount++;
//
//			}/* END: for(ucc) */
//		/* CHK SECOND PART OF MSG FOR BIT ERRS */
//		for(ucc=9;  ucc<30;  ucc++)
//			{
//			if(ucaMSG_BUFF[ucc] != ucTestMsg[ucc]) ucErrCount++;
//			}/* END: for(ucc) */
//
//
//		/* MSG IS BAD */
//		vSERIAL_rom_sout("Bd=");
//		vSERIAL_UIV8out(ucErrCount);
//		vSERIAL_crlf();
//
//		/* BEEP */
//		vBUZ(BAD_SOUND,EIGHTH_NOTE);			//bad
//
//		}/* END: while() */
//
//
//	/* MAKE SURE RADIO IS SHUT OFF */
//	vADF7020_Quit();
//
//	return(ucRetVal);
//
//
//	}/* END: ucDRADIO_continuousReceiveTestMsg() */
//
//
//
//
///*******************  ucDRADIO_run_sniffer()  ********************************
//*
//* This diagnostic is a receive only component
//* There is no chksum used in this diagnostic
//*
//******************************************************************************/
//
//uchar ucDRADIO_run_sniffer(
//		uchar ucChanIdx
//		)
//	{
//	uchar ucRetVal;
//	uchar uc;
//	uchar ucLimit;
//	int iMsgCount;
//	uint uiRSSI;
//	uint uiTmp;
//	uint uiSrcSN;
//	uint uiDestSN;
//	uchar ucChar;
//
//
//	/* SETUP RADIO */
//	vADF7020_WakeUp();
//
//	/* SETUP THE RADIO FREQUENCY */
//	unADF7020_SetChannel(TEST_CHANNEL);
//
//	/* ASSUME NO ERRORS */
//	ucRetVal = DIAG_ACTION_KEY_NONE;
//
//
//	/********************  RECEIVE  ************************/
//
//	/* SETUP TO READ THE RSSI VALUE OF THE RECEIVED MSG */
//	vAD_init();
//	vAD_setup_analog_bits(RSSI_AD_CHAN);
//	vAD_select_chan(RSSI_AD_CHAN, NO_WAIT);
//
//
//	iMsgCount = 1;
//	while(TRUE)			//lint !e716 !e774
//		{
//		/* CHECK FOR THE EXIT COMMAND */
//		if(ucSERIAL_kbhit())
//			{
//			ucChar = ucSERIAL_bin();
//
//			ucRetVal = DIAG_ACTION_KEY_EXIT;	//assume an exit
//			if(ucChar == 0x1B) break; //escape
//			if(ucChar == 'q') break;
//			if(ucChar == 'Q') break;
//			if(ucChar == 'x') break;
//			if(ucChar == 'X') break;
//
//			ucRetVal = DIAG_ACTION_KEY_NONE;	//assume no exit
//
//			}/* END: if() */
//
//
//		/* STARTUP THE RADIO RECEIVER */
//		unADF7020_SetRadioState(RX_IDLE);					//startup the receiver
//
//		/* WAIT FOR THE BARKER TO BE DETECTED */
//		while(!ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_CODE_PHASE_BIT);	//lint !e722
//
//		/* MEASURE THE RSSI FOR THIS MESSAGE */
//		uiRSSI = uiAD_read(RSSI_AD_CHAN, NO_SELECT_CHAN);
//
//		/* WAIT FOR MESSAGE COMPLETE */
//		while(!ucFLAG1_BYTE.FLAG1_STRUCT.FLG1_R_HAVE_MSG_BIT);	//lint !e722
//
//		/* LOAD MESSAGE BYTE COUNT */
//		ucLimit = (ucaMSG_BUFF[GMH_IDX_EOM_IDX] & MAX_MSG_SIZE_MASK);
//
//
//		/**********************  SNIFFER LINE 1  ****************************/
//
//		vSERIAL_rom_sout("\r\n@----  ");
//
//		uiSrcSN = uiMISC_buildUintFromBytes(
//							((uchar *)&ucaMSG_BUFF[GMH_IDX_SRC_SN_HI]),
//							YES_NOINT
//							);
//		vRAD40_showRad40(uiSrcSN);
//
//
//		vSERIAL_bout(' ');
//		vSERIAL_bout(' ');
//
//		switch(ucaMSG_BUFF[GMH_IDX_MSG_TYPE])
//			{
//			case MSG_TYPE_STARTUP:
//				vSERIAL_rom_sout("STUP");
//				break;
//
//			case MSG_TYPE_OM1:
//				vSERIAL_rom_sout("OM1 ");
//				break;
//
//			case MSG_TYPE_OM2:
//				vSERIAL_rom_sout("OM2 ");
//				break;
//
//			case MSG_TYPE_OM3:
//				vSERIAL_rom_sout("OM3 ");
//				break;
//
//			case MSG_TYPE_DC4:
//				vSERIAL_rom_sout("DC4 ");
//				break;
//
//			case MSG_TYPE_DC5:
//				vSERIAL_rom_sout("DC5 ");
//				break;
//
//			case MSG_TYPE_DC6:
//				vSERIAL_rom_sout("DC6 ");
//				break;
//
//			default:
//				vSERIAL_rom_sout("UNK ");
//
//			}/* END: switch() */
//
//		vSERIAL_bout(' ');
//
//		/* SHOW RECIPIENT */
//		uiDestSN = uiMISC_buildUintFromBytes(
//							((uchar *)&ucaMSG_BUFF[GMH_IDX_DEST_SN_HI]),
//							YES_NOINT
//							);
//		if(uiDestSN == 0)
//			vSERIAL_rom_sout("All");				
//		else
//			{
//			vRAD40_showRad40(uiDestSN);
//			}
//
//		vSERIAL_rom_sout("  -----\r\n");
//
//
//		/********************  SNIFFER LINE 2  ***************************/
//
////		vSERIAL_rom_sout("FRM:");
////		vSERIAL_HB8out(ucaMSG_BUFF[GMH_IDX_SRC_SN_LO]);
//
////		vSERIAL_rom_sout("TO: ");
//
////		vSERIAL_rom_sout("-->");
//
////		if(ucaMSG_BUFF[GMH_IDX_DEST_SN_LO] == 0)
////			vSERIAL_rom_sout("All");
////		else
////			vSERIAL_HB8out(ucaMSG_BUFF[GMH_IDX_DEST_SN_LO]);
//
//		vSERIAL_bout('|');
//
////		vSERIAL_rom_sout("RSSI:");
//		vSERIAL_UIV16out(uiRSSI);
//
//
//		vSERIAL_crlf();
//
//
//		/**********************  SNIFFER LINE 3 **************************/
//
//		vSERIAL_bout('#');
//		vSERIAL_UIV16out((uint)iMsgCount);
//		vSERIAL_rom_sout(": ");
//
//		/* SHOW THE WHOLE MESSAGE */
//		for(uc=0;  uc<=ucLimit;  uc++)
//			{
//			if((uc == 2) || (uc == 4) || (uc == 6))
//				{
//				uiTmp = ucaMSG_BUFF[uc];
//				uiTmp <<= 8;
//				uc++;
//				uiTmp |= ucaMSG_BUFF[uc];
//				vSERIAL_HB16out(uiTmp);
//				}
//			else
//				{
//				vSERIAL_HB8out(ucaMSG_BUFF[uc]);
//				}
//
//
//			if(uc != ucLimit)
//				{
//				vSERIAL_bout(',');
//				}
//			else
//				{
//				vSERIAL_crlf();
//				}
//			}
//
//		vSERIAL_crlf();
//
//		iMsgCount++;
//
//
//		}/* END: while() */
//
//
//
//	/* MAKE SURE RADIO IS SHUT OFF */
//	vADF7020_Quit();
//
//	return(ucRetVal);
//
//	} /* END: ucDRADIO_run_sniffer() */
//
//
//
//
//
//
//
//
//
///*-------------------------------  MODULE END  ------------------------------*/
