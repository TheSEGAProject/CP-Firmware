
/***************************  MODOPT.H  ****************************************
*
* Header for MODOPT pkg.
*
* MODOPT deals with the option bits that determine the "persona" that
* the wisard can execute.  The user can choose the options that he wishes the
* wisard to have, and those options are then copied to FRAM, where they are
* loaded on wisard startup.
*
*
* V1.00 12/01/2007 wzr
*	started
*
******************************************************************************/

#ifndef MODOPT_H_INCLUDED
 #define MODOPT_H_INCLUDED


#define DEFAULT_ROLE_IDX	  7 //hub =2, spoke = 7, standalone = 4, sampling hub = 6


#define NUMBER_OF_ROLES			8

#define OPTION_BYTE_COUNT		5

#define TOTAL_OPTION_BITS		(OPTION_BYTE_COUNT * 8)

#define MAX_OPTPAIR_VALUE		((uchar)((OPTION_BYTE_COUNT<<3)+ 0))

#define ROLE_NONE_MSK		 			0x07		//00000111		//---
#define ROLE_SENDONLY_MSK	 		0x07		//00000111		//--X
#define ROLE_HUB_MSK		 			0x07		//00000111		//-R-
#define ROLE_RELAY_MSK		 		0x07		//00000111		//-RX
#define ROLE_STANDALONE_MSK	 	0x07		//00000111		//S--
#define ROLE_TERMINUS_MSK	 		0x07		//00000111		//S-X
#define ROLE_SAMPLINGHUB_MSK 	0x07		//00000111		//SR-
#define ROLE_SPOKE_MSK		 		0x07		//00000111		//SRX

#define ROLE_SAMPLE_BIT				0x04					//00000100
#define ROLE_RECEIVE_BIT			0x02					//00000010
#define ROLE_SEND_BIT					0x01					//00000001



/*-------  OPTION INDEX PAIR DEFINITIONS  -----------------------------------
 *
 * NOTE: The Hi half of the index is the byte index.
 *		 The Lo half of the index is the bit  index.
 *
 *---------------------------------------------------------------------------*/


/* OPTPAIR INDEXES BYTE 0 OF THE OPTION ARRAY */
#define OPTPAIR_WIZ_SENDS						((uint)((0<<3)+ 0))
#define OPTPAIR_WIZ_RECEIVES					((uint)((0<<3)+ 1))
#define OPTPAIR_WIZ_SAMPLES						((uint)((0<<3)+ 2))
#define OPTPAIR_WIZ_NIY							((uint)((0<<3)+ 3))
#define OPTPAIR_STRMING							((uint)((0<<3)+ 4))
#define OPTPAIR_SHOW_SYSTAT						((uint)((0<<3)+ 5))
#define OPTPAIR_CHK_FOR_RDIO_BD					((uint)((0<<3)+ 6))
#define OPTPAIR_USE_RDIO_FOR_RAND_SEED			((uint)((0<<3)+ 7))


/* OPTPAIR INDEXES BYTE 1 OF THE OPTION ARRAY */
#define OPTPAIR_SND_RDIO_STUP_MSG				((uint)((1<<3)+ 0))
#define OPTPAIR_ALLOW_BRAIN_STUP_HBRNAT			((uint)((1<<3)+ 1))
#define OPTPAIR_SPS_ARE_ATTCHD					((uint)((1<<3)+ 2))
#define OPTPAIR_CHK_FOR_SPS_ON_STUP				((uint)((1<<3)+ 3))
#define OPTPAIR_FIX_SOIL_RDING					((uint)((1<<3)+ 4))
#define OPTPAIR_DO_HBRNAT_CHK					((uint)((1<<3)+ 5))
#define OPTPAIR_BUZ_ON							((uint)((1<<3)+ 6))
#define OPTPAIR_CHK_FOR_FRAM_ON_STUP			((uint)((1<<3)+ 7))


/* OPTPAIR INDEXES BYTE 2 OF THE OPTION ARRAY */
#define OPTPAIR_CHK_FRAM_FMT_ON_STUP			((uint)((2<<3)+ 0))
#define OPTPAIR_CHK_FOR_FLSH_ON_STUP			((uint)((2<<3)+ 1))	
#define OPTPAIR_SHOW_STS_ON_LFACTRS				((uint)((2<<3)+ 2))	
#define OPTPAIR_SHOW_LNK_SCHED					((uint)((2<<3)+ 3))	
#define OPTPAIR_UNUSED_1						((uint)((2<<3)+ 4))	
#define OPTPAIR_UNUSED_2						((uint)((2<<3)+ 5))	
#define OPTPAIR_UNUSED_3						((uint)((2<<3)+ 6))	
#define OPTPAIR_UNUSED_4						((uint)((2<<3)+ 7))	


/* OPTPAIR INDEXES BYTE 3 OF THE OPTION ARRAY */
#define OPTPAIR_RPT_SOM2_LNKS_TO_RDIO			((uint)((3<<3)+ 0))
#define OPTPAIR_RPT_SOM2_LNKS_TO_FLSH			((uint)((3<<3)+ 1))

#define OPTPAIR_RPT_SOM2_FAILS_TO_RDIO			((uint)((3<<3)+ 2))
#define OPTPAIR_RPT_SOM2_FAILS_TO_FLSH			((uint)((3<<3)+ 3))

#define OPTPAIR_RPT_ROM2_LNKS_TO_RDIO			((uint)((3<<3)+ 4))
#define OPTPAIR_RPT_ROM2_LNKS_TO_FLSH			((uint)((3<<3)+ 5))

#define OPTPAIR_RPT_ROM2_FAILS_TO_RDIO			((uint)((3<<3)+ 6))
#define OPTPAIR_RPT_ROM2_FAILS_TO_FLSH			((uint)((3<<3)+ 7))



/* OPTPAIR INDEXES BYTE 4 OF THE OPTION ARRAY */
#define OPTPAIR_RPT_SYS_RSTRTS_TO_RDIO			((uint)((4<<3)+ 0))
#define OPTPAIR_RPT_SYS_RSTRTS_TO_FLSH			((uint)((4<<3)+ 1))

#define OPTPAIR_RPT_BATT_DEAD_RSTRT_TO_RDIO		((uint)((4<<3)+ 2))
#define OPTPAIR_RPT_BATT_DEAD_RSTRT_TO_FLSH		((uint)((4<<3)+ 3))

#define OPTPAIR_RPT_BATT_VOLTS_TO_RDIO			((uint)((4<<3)+ 4))
#define OPTPAIR_RPT_BATT_VOLTS_TO_FLSH			((uint)((4<<3)+ 5))

#define OPTPAIR_RPT_TIM_CHGS_TO_RDIO			((uint)((4<<3)+ 6))
#define OPTPAIR_RPT_TIM_CHGS_TO_FLSH			((uint)((4<<3)+ 7))





/*------------- ROM OPTIONS -------------------*/


#if 0
void vMODOPT_showSingleRomOptionTblAllBits(
		uchar ucOptionTblNum
		);

void vMODOPT_showAllRomOptionTblsAllBits(
		void
		);

void vMODOPT_showAllRomOptionTblNames(
		void
		);
#endif




/*------------- FRAM OPTIONS -------------------*/

#if 0
uchar ucMODOPT_readSingleFramOptionBit(
		uchar ucOptionIdxPair	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		);

void vMODOPT_writeSingleFramOptionBit(
		uchar ucOptionIdxPair,	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		uchar ucBitVal		 	//0 = clear bit, non-zro = set bit
		);
#endif


void vMODOPT_copyAllRomOptionsToFramOptions(
		uchar ucRomOptionTblNum
		);


#if 0
void vMODOPT_showSingleFramOptionBit(
		uchar ucOptionIdxPair	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		);

void vMODOPT_showAllFramOptionBits(
		void
		);
#endif




/*------------ RAM OPTIONS --------------------*/

void vMODOPT_copyAllFramOptionsToRamOptions(
		void
		);

uchar ucMODOPT_readSingleRamOptionBit(
		uchar ucOptionIdxPair	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		);

void vMODOPT_writeSingleRamOptionBit(
		uchar ucOptionIdxPair,	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		uchar ucBitVal			//0 = clr-bit, non-zro = set-bit
		);

void vMODOPT_writeBothFramAndRamBit(
		uchar ucOptionIdxPair,	//HI NIBBLE= OptByteNum, LO NIBBLE= OptBitNum
		uchar ucBitVal			//0 = clr-bit, non-zro = set-bit
		);

void vMODOPT_showAllRamOptionBits(
		void
		);

#if 0
uchar ucMODOPT_buildTwoBitReportVal( 
		uchar ucRadioReportBitIdxPair,
		uchar ucFlashReportBitIdxPair
		);
#endif

uchar ucMODOPT_getCurRole(
		void
		);

void vMODOPT_showCurRole(
		void
		);

void vMODOPT_showCurIdentity(
		void
		);

/*------------ CONSOLE OPTIONS MODIFIER  --------------------*/

void vMODOPT_cmdModifyOptionTbls(
		void
		);

uchar ucMODOPT_isRelay(void);





#endif /* MODOPT_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
