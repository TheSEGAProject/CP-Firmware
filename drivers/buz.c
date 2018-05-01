

/**************************  BUZ.C  *****************************************
*
* This all needs change! to correct the sound
* 
* Routines to handle low level BUZ control.
*
*
* V1.00 10/14/2003 wzr
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



#include <msp430x54x.h>			//processor reg description
#include "../std.h"				//standard definitions
#include "../diag.h"			//diagnostic defines
#include "../hal/config.h" 		//system configuration description file
#include "../delay.h"			//timer delay routine
#include "../serial.h" 			//serial IO port stuff
#include "buz.h"				//buzzer control code
#include "../MODOPT.h"			//Modify Options routines


extern volatile union						//ucDiagFlagByte1
  {
	uchar byte;
	struct
	 {
	 unsigned DIAG_mute_bit:1;					//bit 0 ;1=MUTE, 0=SOUND
	 unsigned DIAG_test_repeat_looping_bit:1;	//bit 1 ;1=LOOPING, 0=NOT LOOPING
	 unsigned DIAG_only_a_single_test_bit:1;	//bit 2	;1=ONLY A SINGLE, 0=ALL
	 unsigned DIAG_only_a_single_group_bit:1;	//bit 3 ;1=ONLY A GROUP, 0=ALL
	 unsigned DIAG_exit_out_to_top_bit:1;		//bit 4 ;1=EXIT OUT, 0=NO EXIT OUT
	 unsigned DIAG_exit_out_to_prev_test_bit:1;	//bit 5 ;1=GOTO PREV, 0=NO GOTO PREV
	 unsigned DIAG_exit_out_to_next_test_bit:1;	//bit 6 ;1=GOTO NEXT, 0=NO GOTO NEXT
	 unsigned DIAG_quit_out_to_return_bit:1;	//bit 7 ;1=TOTO RETURN, 0=NO QUIT
	 }diagFlagByte1_STRUCT;

  }ucDiagFlagByte1;






	/* NOTE DEFINITIONS */
	#define NOTE_NONE		0


	/* NOTE DEFINITIONS */
//-------------------------------------  LO OCTAVE
	#define NOTE_LO_C			659

	#define TEMP_MULTIPLIER_D1	(1.059463 * 100000.0)
	#define TEMP_MULTIPLIER_L1	((long)TEMP_MULTIPLIER_D1)
	#define TEMP_VAL_L1			(NOTE_LO_C * TEMP_MULTIPLIER_L1)
	#define NOTE_LO_C_SHARP 	((uint) (TEMP_VAL_L1 / 100000L))

	#define TEMP_MULTIPLIER_D2	((1.059463 * TEMP_MULTIPLIER_D1))
	#define TEMP_MULTIPLIER_L2	((long)TEMP_MULTIPLIER_D2)
	#define TEMP_VAL_L2			(NOTE_LO_C * TEMP_MULTIPLIER_L2)
	#define NOTE_LO_D 			((uint) (TEMP_VAL_L2 / 100000L))

	#define TEMP_MULTIPLIER_D3	((1.059463 * TEMP_MULTIPLIER_D2))
	#define TEMP_MULTIPLIER_L3	((long)TEMP_MULTIPLIER_D3)
	#define TEMP_VAL_L3			(NOTE_LO_C * TEMP_MULTIPLIER_L3)
	#define NOTE_LO_D_SHARP 	((uint) (TEMP_VAL_L3 / 100000L))

	#define TEMP_MULTIPLIER_D4	((1.059463 * TEMP_MULTIPLIER_D3))
	#define TEMP_MULTIPLIER_L4	((long)TEMP_MULTIPLIER_D4)
	#define TEMP_VAL_L4			(NOTE_LO_C * TEMP_MULTIPLIER_L4)
	#define NOTE_LO_E 			((uint) (TEMP_VAL_L4 / 100000L))

	#define TEMP_MULTIPLIER_D5	((1.059463 * TEMP_MULTIPLIER_D4))
	#define TEMP_MULTIPLIER_L5	((long)TEMP_MULTIPLIER_D5)
	#define TEMP_VAL_L5			(NOTE_LO_C * TEMP_MULTIPLIER_L5)
	#define NOTE_LO_F 			((uint) (TEMP_VAL_L5 / 100000L))

	#define TEMP_MULTIPLIER_D6	((1.059463 * TEMP_MULTIPLIER_D5))
	#define TEMP_MULTIPLIER_L6	((long)TEMP_MULTIPLIER_D6)
	#define TEMP_VAL_L6			(NOTE_LO_C * TEMP_MULTIPLIER_L6)
	#define NOTE_LO_F_SHARP 	((uint) (TEMP_VAL_L6 / 100000L))

	#define TEMP_MULTIPLIER_D7	((1.059463 * TEMP_MULTIPLIER_D6))
	#define TEMP_MULTIPLIER_L7	((long)TEMP_MULTIPLIER_D7)
	#define TEMP_VAL_L7			(NOTE_LO_C * TEMP_MULTIPLIER_L7)
	#define NOTE_LO_G 			((uint) (TEMP_VAL_L7 / 100000L))

	#define TEMP_MULTIPLIER_D8	((1.059463 * TEMP_MULTIPLIER_D7))
	#define TEMP_MULTIPLIER_L8	((long)TEMP_MULTIPLIER_D8)
	#define TEMP_VAL_L8			(NOTE_LO_C * TEMP_MULTIPLIER_L8)
	#define NOTE_LO_G_SHARP 	((uint) (TEMP_VAL_L8 / 100000L))

	#define TEMP_MULTIPLIER_D9	((1.059463 * TEMP_MULTIPLIER_D8))
	#define TEMP_MULTIPLIER_L9	((long)TEMP_MULTIPLIER_D9)
	#define TEMP_VAL_L9			(NOTE_LO_C * TEMP_MULTIPLIER_L9)
	#define NOTE_LO_A 			((uint) (TEMP_VAL_L9 / 100000L))

	#define TEMP_MULTIPLIER_D10	((1.059463 * TEMP_MULTIPLIER_D9))
	#define TEMP_MULTIPLIER_L10	((long)TEMP_MULTIPLIER_D10)
	#define TEMP_VAL_L10		(NOTE_LO_C * TEMP_MULTIPLIER_L10)
	#define NOTE_LO_A_SHARP 	((uint) (TEMP_VAL_L10 / 100000L))

	#define TEMP_MULTIPLIER_D11	((1.059463 * TEMP_MULTIPLIER_D10))
	#define TEMP_MULTIPLIER_L11	((long)TEMP_MULTIPLIER_D11)
	#define TEMP_VAL_L11		(NOTE_LO_C * TEMP_MULTIPLIER_L11)
	#define NOTE_LO_B 			((uint) (TEMP_VAL_L11 / 100000L))

//-------------------------------------  MIDDLE OCTAVE
	#define TEMP_MULTIPLIER_D12	((1.059463 * TEMP_MULTIPLIER_D11))
	#define TEMP_MULTIPLIER_L12	((long)TEMP_MULTIPLIER_D12)
	#define TEMP_VAL_L12		(NOTE_LO_C * TEMP_MULTIPLIER_L12)
	#define NOTE_C				((uint) (TEMP_VAL_L12 / 100000L))

	#define TEMP_MULTIPLIER_D13	((1.059463 * TEMP_MULTIPLIER_D12))
	#define TEMP_MULTIPLIER_L13	((long)TEMP_MULTIPLIER_D13)
	#define TEMP_VAL_L13		(NOTE_LO_C * TEMP_MULTIPLIER_L13)
	#define NOTE_C_SHARP		((uint) (TEMP_VAL_L13 / 100000L))

	#define TEMP_MULTIPLIER_D14	((1.059463 * TEMP_MULTIPLIER_D13))
	#define TEMP_MULTIPLIER_L14	((long)TEMP_MULTIPLIER_D14)
	#define TEMP_VAL_L14		(NOTE_LO_C * TEMP_MULTIPLIER_L14)
	#define NOTE_D 				((uint) (TEMP_VAL_L14 / 100000L))

	#define TEMP_MULTIPLIER_D15	((1.059463 * TEMP_MULTIPLIER_D14))
	#define TEMP_MULTIPLIER_L15	((long)TEMP_MULTIPLIER_D15)
	#define TEMP_VAL_L15		(NOTE_LO_C * TEMP_MULTIPLIER_L15)
	#define NOTE_D_SHARP		((uint) (TEMP_VAL_L15 / 100000L))

	#define TEMP_MULTIPLIER_D16	((1.059463 * TEMP_MULTIPLIER_D15))
	#define TEMP_MULTIPLIER_L16	((long)TEMP_MULTIPLIER_D16)
	#define TEMP_VAL_L16		(NOTE_LO_C * TEMP_MULTIPLIER_L16)
	#define NOTE_E 				((uint) (TEMP_VAL_L16 / 100000L))

	#define TEMP_MULTIPLIER_D17	((1.059463 * TEMP_MULTIPLIER_D16))
	#define TEMP_MULTIPLIER_L17	((long)TEMP_MULTIPLIER_D17)
	#define TEMP_VAL_L17		(NOTE_LO_C * TEMP_MULTIPLIER_L17)
	#define NOTE_F 				((uint) (TEMP_VAL_L17 / 100000L))

	#define TEMP_MULTIPLIER_D18	((1.059463 * TEMP_MULTIPLIER_D17))
	#define TEMP_MULTIPLIER_L18	((long)TEMP_MULTIPLIER_D18)
	#define TEMP_VAL_L18		(NOTE_LO_C * TEMP_MULTIPLIER_L18)
	#define NOTE_F_SHARP		((uint) (TEMP_VAL_L18 / 100000L))

	#define TEMP_MULTIPLIER_D19	((1.059463 * TEMP_MULTIPLIER_D18))
	#define TEMP_MULTIPLIER_L19	((long)TEMP_MULTIPLIER_D19)
	#define TEMP_VAL_L19		(NOTE_LO_C * TEMP_MULTIPLIER_L19)
	#define NOTE_G 				((uint) (TEMP_VAL_L19 / 100000L))

	#define TEMP_MULTIPLIER_D20	((1.059463 * TEMP_MULTIPLIER_D19))
	#define TEMP_MULTIPLIER_L20	((long)TEMP_MULTIPLIER_D20)
	#define TEMP_VAL_L20		(NOTE_LO_C * TEMP_MULTIPLIER_L20)
	#define NOTE_G_SHARP		((uint) (TEMP_VAL_L20 / 100000L))

	#define TEMP_MULTIPLIER_D21	((1.059463 * TEMP_MULTIPLIER_D20))
	#define TEMP_MULTIPLIER_L21	((long)TEMP_MULTIPLIER_D21)
	#define TEMP_VAL_L21		(NOTE_LO_C * TEMP_MULTIPLIER_L21)
	#define NOTE_A 				((uint) (TEMP_VAL_L21 / 100000L))

	#define TEMP_MULTIPLIER_D22	((1.059463 * TEMP_MULTIPLIER_D21))
	#define TEMP_MULTIPLIER_L22	((long)TEMP_MULTIPLIER_D22)
	#define TEMP_VAL_L22		(NOTE_LO_C * TEMP_MULTIPLIER_L22)
	#define NOTE_A_SHARP		((uint) (TEMP_VAL_L22 / 100000L))

	#define TEMP_MULTIPLIER_D23	((1.059463 * TEMP_MULTIPLIER_D22))
	#define TEMP_MULTIPLIER_L23	((long)TEMP_MULTIPLIER_D23)
	#define TEMP_VAL_L23		(NOTE_LO_C * TEMP_MULTIPLIER_L23)
	#define NOTE_B 				((uint) (TEMP_VAL_L23 / 100000L))

//-----------------------------------  HI OCTAVE

	#define TEMP_MULTIPLIER_D24	((1.059463 * TEMP_MULTIPLIER_D23))
	#define TEMP_MULTIPLIER_L24	((long)TEMP_MULTIPLIER_D24)
	#define TEMP_VAL_L24		(NOTE_LO_C * TEMP_MULTIPLIER_L24)
	#define NOTE_HI_C			((uint) (TEMP_VAL_L24 / 100000L))

	#define TEMP_MULTIPLIER_D25	((1.059463 * TEMP_MULTIPLIER_D24))
	#define TEMP_MULTIPLIER_L25	((long)TEMP_MULTIPLIER_D25)
	#define TEMP_VAL_L25		(NOTE_LO_C * TEMP_MULTIPLIER_L25)
	#define NOTE_HI_C_SHARP		((uint) (TEMP_VAL_L25 / 100000L))

	#define TEMP_MULTIPLIER_D26	((1.059463 * TEMP_MULTIPLIER_D25))
	#define TEMP_MULTIPLIER_L26	((long)TEMP_MULTIPLIER_D26)
	#define TEMP_VAL_L26		(NOTE_LO_C * TEMP_MULTIPLIER_L26)
	#define NOTE_HI_D 			((uint) (TEMP_VAL_L26 / 100000L))

	#define TEMP_MULTIPLIER_D27	((1.059463 * TEMP_MULTIPLIER_D26))
	#define TEMP_MULTIPLIER_L27	((long)TEMP_MULTIPLIER_D27)
	#define TEMP_VAL_L27		(NOTE_LO_C * TEMP_MULTIPLIER_L27)
	#define NOTE_HI_D_SHARP		((uint) (TEMP_VAL_L27 / 100000L))

	#define TEMP_MULTIPLIER_D28	((1.059463 * TEMP_MULTIPLIER_D27))
	#define TEMP_MULTIPLIER_L28	((long)TEMP_MULTIPLIER_D28)
	#define TEMP_VAL_L28		(NOTE_LO_C * TEMP_MULTIPLIER_L28)
	#define NOTE_HI_E 			((uint) (TEMP_VAL_L28 / 100000L))

	#define TEMP_MULTIPLIER_D29	((1.059463 * TEMP_MULTIPLIER_D28))
	#define TEMP_MULTIPLIER_L29	((long)TEMP_MULTIPLIER_D29)
	#define TEMP_VAL_L29		(NOTE_LO_C * TEMP_MULTIPLIER_L29)
	#define NOTE_HI_F 			((uint) (TEMP_VAL_L29 / 100000L))

	#define TEMP_MULTIPLIER_D30	((1.059463 * TEMP_MULTIPLIER_D29))
	#define TEMP_MULTIPLIER_L30	((long)TEMP_MULTIPLIER_D30)
	#define TEMP_VAL_L30		(NOTE_LO_C * TEMP_MULTIPLIER_L30)
	#define NOTE_HI_F_SHARP		((uint) (TEMP_VAL_L30 / 100000L))

	#define TEMP_MULTIPLIER_D31	((1.059463 * TEMP_MULTIPLIER_D30))
	#define TEMP_MULTIPLIER_L31	((long)TEMP_MULTIPLIER_D31)
	#define TEMP_VAL_L31		(NOTE_LO_C * TEMP_MULTIPLIER_L31)
	#define NOTE_HI_G 			((uint) (TEMP_VAL_L31 / 100000L))

	#define TEMP_MULTIPLIER_D32	((1.059463 * TEMP_MULTIPLIER_D31))
	#define TEMP_MULTIPLIER_L32	((long)TEMP_MULTIPLIER_D32)
	#define TEMP_VAL_L32		(NOTE_LO_C * TEMP_MULTIPLIER_L32)
	#define NOTE_HI_G_SHARP		((uint) (TEMP_VAL_L32 / 100000L))

	#define TEMP_MULTIPLIER_D33	((1.059463 * TEMP_MULTIPLIER_D32))
	#define TEMP_MULTIPLIER_L33	((long)TEMP_MULTIPLIER_D33)
	#define TEMP_VAL_L33		(NOTE_LO_C * TEMP_MULTIPLIER_L33)
	#define NOTE_HI_A 			((uint) (TEMP_VAL_L33 / 100000L))

	#define TEMP_MULTIPLIER_D34	((1.059463 * TEMP_MULTIPLIER_D33))
	#define TEMP_MULTIPLIER_L34	((long)TEMP_MULTIPLIER_D34)
	#define TEMP_VAL_L34		(NOTE_LO_C * TEMP_MULTIPLIER_L34)
	#define NOTE_HI_A_SHARP		((uint) (TEMP_VAL_L34 / 100000L))

	#define TEMP_MULTIPLIER_D35	((1.059463 * TEMP_MULTIPLIER_D34))
	#define TEMP_MULTIPLIER_L35	((long)TEMP_MULTIPLIER_D35)
	#define TEMP_VAL_L35		(NOTE_LO_C * TEMP_MULTIPLIER_L35)
	#define NOTE_HI_B 			((uint) (TEMP_VAL_L35 / 100000L))

//------------------------------------- XHI OCTAVE

	#define TEMP_MULTIPLIER_D36	((1.059463 * TEMP_MULTIPLIER_D35))
	#define TEMP_MULTIPLIER_L36	((long)TEMP_MULTIPLIER_D36)
	#define TEMP_VAL_L36		(NOTE_LO_C * TEMP_MULTIPLIER_L36)
	#define NOTE_XHI_C 			((uint) (TEMP_VAL_L36 / 100000L))


	/* NOTE DURATIONS */
	#define EIGHTH_NOTE		 500	//duration in 100us tics
	#define QUARTER_NOTE	1000
	#define HALF_NOTE		2000
	#define WHOLE_NOTE		4000




	/* PACKED COMBINATIONS OF NOTES AND DURATIONS */

	#define W0		"\xC1"		//whole rest
	#define H0		"\x81"		//half rest
	#define Q0		"\x41"		//quarter rest
	#define E0		"\x01"		//eighth rest

	#define WMC	 	"\xC2"		//whole middle C
	#define HMC	 	"\x82"		//half middle C
	#define QMC	 	"\x42"		//quarter middle C
	#define EMC	 	"\x02"		//eighth middle C

	#define WMCS	"\xC3"		//whole middle C sharp
	#define HMCS	"\x83"		//half middle C sharp
	#define QMCS	"\x43"		//quarter middle C sharp
	#define EMCS 	"\x03"		//eighth middle C sharp

	#define WMD	 	"\xC4"		//whole middle D
	#define HMD	 	"\x84"		//half middle D
	#define QMD	 	"\x44"		//quarter middle D
	#define EMD  	"\x04"		//eighth middle D

	#define WMDS	"\xC5"		//whole middle D sharp
	#define HMDS	"\x85"		//half middle D sharp
	#define QMDS	"\x45"		//quarter middle D sharp
	#define EMDS 	"\x05"		//eighth middle D sharp

	#define WME		"\xC6"		//whole middle E
	#define HME		"\x86"		//half middle E
	#define QME		"\x46"		//quarter middle E
	#define EME 	"\x06"		//eighth middle E
				
	#define WMF		"\xC7"		//whole middle F
	#define HMF		"\x87"		//half middle F
	#define QMF		"\x47"		//quarter middle F
	#define EMF 	"\x07"		//eighth middle F

	#define WMFS	"\xC8"		//whole middle F sharp
	#define HMFS	"\x88"		//half middle F sharp
	#define QMFS	"\x48"		//quarter middle F sharp
	#define EMFS 	"\x08"		//eighth middle F sharp

	#define WMG		"\xC9"		//whole middle G
	#define HMG		"\x89"		//half middle G
	#define QMG		"\x49"		//quarter middle G
	#define EMG 	"\x09"		//eighth middle G

	#define WMGS	"\xCA"		//whole middle G sharp
	#define HMGS	"\x8A"		//half middle G sharp
	#define QMGS	"\x4A"		//quarter middle G sharp
	#define EMGS 	"\x0A"		//eighth middle G sharp

	#define WMA		"\xCB"		//whole middle A
	#define HMA		"\x8B"		//half middle A
	#define QMA		"\x4B"		//quarter middle A
	#define EMA 	"\x0B"		//eighth middle A

	#define WMAS	"\xCC"		//whole middle A sharp
	#define HMAS	"\x8C"		//half middle A sharp
	#define QMAS	"\x4C"		//quarter middle A sharp
	#define EMAS 	"\x0C"		//eighth middle A sharp

	#define WMB		"\xCD"		//whole middle B
	#define HMB		"\x8D"		//half middle B
	#define QMB		"\x4D"		//quarter middle B
	#define EMB 	"\x0D"		//eighth middle B


//---------------------------------------------  HI OCTAVE

	#define WHC	 	"\xCE"		//whole high C
	#define HHC	 	"\x8E"		//half high C
	#define QHC	 	"\x4E"		//quarter high C
	#define EHC	 	"\x0E"		//eighth high C

	#define WHCS	"\xCF"		//whole high C sharp
	#define HHCS	"\x8F"		//half high C sharp
	#define QHCS	"\x4F"		//quarter high C sharp
	#define EHCS 	"\x0F"		//eighth high C sharp

	#define WHD	 	"\xD0"		//whole high D
	#define HHD	 	"\x90"		//half high D
	#define QHD	 	"\x50"		//quarter high D
	#define EHD  	"\x10"		//eighth high D

	#define WHDS	"\xD1"		//whole high D sharp
	#define HHDS	"\x91"		//half high D sharp
	#define QHDS	"\x51"		//quarter high D sharp
	#define EHDS 	"\x11"		//eighth high D sharp

	#define WHE		"\xD2"		//whole high E
	#define HHE		"\x92"		//half high E
	#define QHE		"\x52"		//quarter high E
	#define EHE 	"\x12"		//eighth high E
				
	#define WHF		"\xD3"		//whole high F
	#define HHF		"\x93"		//half high F
	#define QHF		"\x53"		//quarter high F
	#define EHF 	"\x13"		//eighth high F

	#define WHFS	"\xD4"		//whole high F sharp
	#define HHFS	"\x94"		//half high F sharp
	#define QHFS	"\x54"		//quarter high F sharp
	#define EHFS 	"\x14"		//eighth high F sharp

	#define WHG		"\xD5"		//whole high G
	#define HHG		"\x95"		//half high G
	#define QHG		"\x55"		//quarter high G
	#define EHG 	"\x15"		//eighth high G

	#define WHGS	"\xD6"		//whole high G sharp
	#define HHGS	"\x96"		//half high G sharp
	#define QHGS	"\x56"		//quarter high G sharp
	#define EHGS 	"\x16"		//eighth high G sharp

	#define WHA		"\xD7"		//whole high A
	#define HHA		"\x97"		//half high A
	#define QHA		"\x57"		//quarter high A
	#define EHA 	"\x17"		//eighth high A

	#define WHAS	"\xD8"		//whole high A sharp
	#define HHAS	"\x98"		//half high A sharp
	#define QHAS	"\x58"		//quarter high A sharp
	#define EHAS 	"\x18"		//eighth high A sharp

	#define WHB		"\xD9"		//whole high B
	#define HHB		"\x99"		//half high B
	#define QHB		"\x59"		//quarter high B
	#define EHB 	"\x19"		//eighth high B


//---------------------------------------------  LO OCTAVE

	#define WLC	 	"\xDA"		//whole low C
	#define HLC	 	"\x9A"		//half low C
	#define QLC	 	"\x5A"		//quarter low C
	#define ELC	 	"\x1A"		//eighth low C

	#define WLCS	"\xDB"		//whole low C sharp
	#define HLCS	"\x9B"		//half low C sharp
	#define QLCS	"\x5B"		//quarter low C sharp
	#define ELCS 	"\x1B"		//eighth low C sharp

	#define WLD	 	"\xDC"		//whole low D
	#define HLD	 	"\x9C"		//half low D
	#define QLD	 	"\x5C"		//quarter low D
	#define ELD  	"\x1C"		//eighth low D

	#define WLDS	"\xDD"		//whole low D sharp
	#define HLDS	"\x9D"		//half low D sharp
	#define QLDS	"\x5D"		//quarter low D sharp
	#define ELDS 	"\x1D"		//eighth low D sharp

	#define WLE		"\xDE"		//whole low E
	#define HLE		"\x9E"		//half low E
	#define QLE		"\x5E"		//quarter low E
	#define ELE 	"\x1E"		//eighth low E
				
	#define WLF		"\xDF"		//whole low F
	#define HLF		"\x9F"		//half low F
	#define QLF		"\x5F"		//quarter low F
	#define ELF 	"\x1F"		//eighth low F

	#define WLFS	"\xE0"		//whole low F sharp
	#define HLFS	"\xA0"		//half low F sharp
	#define QLFS	"\x60"		//quarter low F sharp
	#define ELFS 	"\x20"		//eighth low F sharp

	#define WLG		"\xE1"		//whole low G
	#define HLG		"\xA1"		//half low G
	#define QLG		"\x61"		//quarter low G
	#define ELG 	"\x21"		//eighth low G

	#define WLGS	"\xE2"		//whole low G sharp
	#define HLGS	"\xA2"		//half low G sharp
	#define QLGS	"\x62"		//quarter low G sharp
	#define ELGS 	"\x22"		//eighth low G sharp

	#define WLA		"\xE3"		//whole low A
	#define HLA		"\xA3"		//half low A
	#define QLA		"\x63"		//quarter low A
	#define ELA 	"\x23"		//eighth low A

	#define WLAS	"\xE4"		//whole low A sharp
	#define HLAS	"\xA4"		//half low A sharp
	#define QLAS	"\x64"		//quarter low A sharp
	#define ELAS 	"\x24"		//eighth low A sharp

	#define WLB		"\xE5"		//whole low B
	#define HLB		"\xA5"		//half low B
	#define QLB		"\x65"		//quarter low B
	#define ELB 	"\x25"		//eighth low B



/*************************  NOTE TABLES  **************************************
*
* PARALLEL TABLES STARTING WITH NOTE_C AND PROGRESSING UP TO B
* index of table is the index assigned to each note 
* first table is the frequency of the note 
* second table is the duration for the note
*
*****************************************************************************/


const uint uiNote_freq_tbl[0x26] = {		//lint !e103 !e17 !e10

	NOTE_NONE,			//0: Blank place holder for ASCIZ

	NOTE_NONE,			//1: The real note none

	NOTE_C,				//2
	NOTE_C_SHARP,		//3

	NOTE_D,				//4
	NOTE_D_SHARP,		//5

	NOTE_E,				//6

	NOTE_F,				//7
	NOTE_F_SHARP,		//8:

	NOTE_G,				//9:
	NOTE_G_SHARP,		//A:

	NOTE_A,				//B:
	NOTE_A_SHARP,		//C:

	NOTE_B,				//D:

	//-------------------------  HI OCTAVE
	NOTE_HI_C,				//E
	NOTE_HI_C_SHARP,		//F

	NOTE_HI_D,				//10
	NOTE_HI_D_SHARP,		//11

	NOTE_HI_E,				//12

	NOTE_HI_F,				//13
	NOTE_HI_F_SHARP,		//14

	NOTE_HI_G,				//15
	NOTE_HI_G_SHARP,		//16

	NOTE_HI_A,				//17
	NOTE_HI_A_SHARP,		//18

	NOTE_HI_B,				//19

	//-------------------------  LO OCTAVE

	NOTE_LO_C,				//1A
	NOTE_LO_C_SHARP,		//1B

	NOTE_LO_D,				//1C
	NOTE_LO_D_SHARP,		//1D

	NOTE_LO_E,				//1E

	NOTE_LO_F,				//1F
	NOTE_LO_F_SHARP,		//20

	NOTE_LO_G,				//21
	NOTE_LO_G_SHARP,		//22

	NOTE_LO_A,				//23
	NOTE_LO_A_SHARP,		//24

	NOTE_LO_B				//25


	};


const uint uiNote_duration_tbl[4] = {
	EIGHTH_NOTE,		//0 -  500
	QUARTER_NOTE,		//1 - 1000
	HALF_NOTE,			//2 - 2000
	WHOLE_NOTE			//3 - 4000
	};




/*********************  vBUZ()  **********************************************
*
* Vibrate the buzzer
*
* The SMCLK tics at a rate of 4 MHz.
* The input clk frequency to timer SMCLK.
* We further step down the input freq to timer by dividing by 4 then 6 for a
* total input freq of SMCLK/24.
* Computing the width of a timer tic using the values from above we get
*
* 		TimerA0_input_tic_rate = 24/4000000 sec/tic = 6 us/tic
*
* Timer A0 is a 16 bit timer so at the minimum value of a 50% duty cycle
* PWM would be a TA0CCR0 value)period) of 2,
* and a TA0CCR3 value (duty cycle)  of 1.
* This would give a maximum frequency output of:
*
*		Maximum_freq = 1/(2Tics * 6 us/Tic) = 1/(12 us) = 333333 Hz
*
* A TimerA0 maximum value at a 50% duty cycle would be:
* PWM would be a TA0CCR0 value (period) of 65534,
* and a TA0CCR3 value  (duty cycle)  of 32767.
* This would give a minimum frequency output of:
*
*		Minimum_freq = 1/(65534Tics * 6us/Tic) = 1/(1625.6us) = 2.54 Hz
*
* The buzzer that we are using has a range up to 35000 Hz.
* 
* The following section comments are remaining from the PIC and therefore make 
* little sense in terms of range and the choices made because of it. The note 
* selection has not been ported beyond function and will need tuning if you 
* care to.
* 
* *****************************************************************************
*
* The standard note of A is 440 Hz (American Concert Pitch) and is
* therefore out of range of the current timer settings.
*
* An octave doubles the frequency and we want to get 3 octaves into the freq
* range that we can handle. We must accomodate a base freq that can go up to
* 8 times higher and still be within our limits.
* 
* Choosing 880 Hz as LO 'A' as our base note will give us
* 659 Hz as the LO 'C' note and a top of 5274 Hz as our hi C note.
*
* We will use an equally tempered scale so that each note is a uniform
* distance in freq from the previous note namely the 12th root of 2 = 1.05946.
* Where there are 12 keys from C to B inclusive.
*
* The definitions in BUZ.h reflect the above thinking.
*
******************************************************************************/

void vBUZ(
		uint uiFreq,
		uint uiDuration_in_100us
		)
	{
	uchar ucTimerCount;
	uchar ucDutyCycleCount;
	long lInterval_in_10ns;
	long lTimerCount_x_1;

	#ifdef FULLDIAG_ENABLED
	if(ucDiagFlagByte1.diagFlagByte1_STRUCT.DIAG_mute_bit) return;
	#endif


	/* COMPUTE THE TIMER COUNT REQUIRED TO GET THE PROPER FREQ */
	lInterval_in_10ns = (SMCLK_RATE*10)/((long)uiFreq);	//(SMCLK_RATE) / desired_freq
	lTimerCount_x_1   = lInterval_in_10ns/240;			//lInterval_in_10ns / (24 * 10)
	ucTimerCount      = (uchar)lTimerCount_x_1; 		//count as uchar

   #if 0
	vSERIAL_sout("TimrCnt = ", 10);
	vSERIAL_HB8out(ucTimerCount);
	vSERIAL_crlf();
   #endif

	/* COMPUTE THE TIMER COUNT FOR THE DUTY CYCLE */
	ucDutyCycleCount = ucTimerCount/2;

   #if 0
	vSERIAL_sout("DtyCycl= ", 9);
	vSERIAL_HB8out(ucDutyCycleCount);
	vSERIAL_crlf();
   #endif

	TA0CTL = 0x0000;
	TA0CCTL3 = 0x0000;
	TA0CCTL4 = 0x0000;

	/*SETUP THE TA0 CLK SELECT */
	TA0CTL = TASSEL_2;

	/* SETUP PERIOD VALUE FOR TIMER 4 */
	TA0CCR0 = ucTimerCount;		//PERIOD = preset time = rollover time

	/* SETUP DUTY CYCLE FOR PWM */
	TA0CCR3 = ucDutyCycleCount;	//DUTY CYCLE = UP part of PERIOD = 50%

	/* SET THE BUZZER PIN DIRECTION */
	BUZZER_DIR_PORT |= BUZZER_BIT;
	BUZZER_SEL_PORT	|= BUZZER_BIT;

	/* SET PRESCALE AND TURN ON CLK */
	TA0EX0 = 0x0006;     			// SMCLK_RATE/6
	TA0CTL |= 0x90;					// (SMCLK_RATE/6)/4 = SMCLK_RATE/24
    //line above originally TA0CTL |= 0x90; this clears TASSEL bit 
	/* SET TA0 MODE TO COMPARE, RESET/SET MODE*/
	TA0CCTL3 = 0x00E0;

	// Wait the duration in low-power mode
	vDELAY_LPMWait1us((ulong)uiDuration_in_100us * 100L, 1);

	/* SHUT OFF THE TIMER */
	TA0CTL &= ~0x0030;

	//leave output bit in HI state to save pwr
	TA0CCTL3 = 0x04; 	

	/* SET THE BUZZER PIN DIRECTION */
	BUZZER_DIR_PORT |= BUZZER_BIT;
	BUZZER_SEL_PORT	&= ~BUZZER_BIT;
	BUZZER_OUT_PORT |= BUZZER_BIT;

	// Reset the expansion register to 0
	TA0CTL = 0x0000;
	TA0CCTL3 = 0x0000;
	TA0CCTL4 = 0x0000;
	TA0EX0 = 0x0000;

	return;

	}/* END: vBUZ() */

/*********************  vBUZ_blink_buzzer()  **********************************
*
* Blink the buzzer a number of times constant frequency at NOTE_C
*
******************************************************************************/

void vBUZ_blink_buzzer(
		uchar ucCount
		)
	{
	uchar ucc;

	#define BUZ_LEN_IN_100US		625
	#define BUZ_GAP_IN_100US		1125

	for(ucc=0;  ucc<ucCount;  ucc++)
		{//NOTE_C originally
		vBUZ(NOTE_LO_A,BUZ_LEN_IN_100US);			
		vDELAY_wait100usTic(BUZ_GAP_IN_100US);
		}/* END: for(ucc) */

	}/* END: vBUZ_blink_buzzer() */


/*********************  vBUZ_play_tune()  ************************************
*
* Routine to play a tune on the buzzer (adjusts freq, duration, and gap);
*
*****************************************************************************/

void vBUZ_play_tune(
		const char *cpTunePtr,		//lint !e125
		uint uiFixedDuration	//for sliding notes
		)
	{
	uchar ucTuneSymbol;
	uchar ucDurationIdx;
	uchar ucFreqIdx;
	uint uiDuration;

	while(*cpTunePtr)
		{
		/* LOAD THE NEXT TUNE SUMBOL */
		ucTuneSymbol = ((uchar)*cpTunePtr++);		

		/* DECOMPRESS THE TUNE SYMBOL */
		ucDurationIdx = ucTuneSymbol >> 6;		
		if(uiFixedDuration == 0)
			uiDuration = uiNote_duration_tbl[ucDurationIdx];
		else
			uiDuration = uiFixedDuration;

		ucFreqIdx = ucTuneSymbol & 0x3F;

		#if 0
		vSERIAL_sout("Sym=", 4);
		vSERIAL_HB8out(ucTuneSymbol);
		vSERIAL_sout("  Dur=", 6);
		vSERIAL_HB8out(ucDurationIdx);
		vSERIAL_sout("  Freq=", 7);
		vSERIAL_HB8out(ucFreqIdx);
		vSERIAL_sout("  FreqVal=", 10);
		vSERIAL_UI16out(uiNote_freq_tbl[ucFreqIdx]);
		vSERIAL_rom_sout("  DurVal=", 9);
		vSERIAL_UI16out(uiNote_duration_tbl[ucDurationIdx]);
		vSERIAL_crlf();
		#endif

		vBUZ(uiNote_freq_tbl[ucFreqIdx], uiDuration);

		}/* END: while() */

	return;

	}/* END: vBUZ_play_tune() */







/*********************  vBUZ_tune_mary()  ************************************
*
* Mary had a little lamb (NOT USED)
*
*****************************************************************************/

void vBUZ_tune_mary(
		void
		)
	{

	vBUZ_play_tune(
	WMFS WME WMD WME WMFS WMFS WMFS W0 WME WME WME W0 WMFS \
	WMA WMA W0 WMFS WME WMD WME WMFS WMFS WMFS WMFS WME WME WMFS WME WMD W0,
	0
	);

	}/* END: vBUZ_tune_mary() */





/*********************  vBUZ_tune_imperial()  *******************************
*
* Plays when system has succeeded in coming up from hibernation.
*
*****************************************************************************/

void vBUZ_tune_imperial(
		void
		)
	{

	vBUZ_play_tune(
	HMC E0 EMC E0 EMC H0 WMG W0,
	0
	);

	}/* END: vBUZ_tune_imperial() */





/*********************  vBUZ_tune_TaDah_TaDah()  **********************************
*
* played when the system has joined the net.
*
*****************************************************************************/

void vBUZ_tune_TaDah_TaDah(
		void
		)
	{

	vBUZ_play_tune(
			EMC E0 HMG,
	0
	);

	}/* END: vBUZ_tune_TaDah_TaDah() */

/*********************  vBUZ_tune_Blip()  **********************************
*
* played when the system has joined the net.
*
*****************************************************************************/

void vBUZ_tune_Blip(void)
	{
	vBUZ_play_tune(EMC, 0);

	}/* END: vBUZ_tune_Blip() */




/*********************  vBUZ_tune_bad_news_1()  *****************************
*
* Sound 
*
*****************************************************************************/

void vBUZ_tune_bad_news_1(
		void
		)
	{

	vBUZ_play_tune(
	WMD Q0 WMCS W0 WMD Q0 WMCS W0,
	0
	);

	}/* END: vBUZ_tune_bad_news_1() */





/*********************  vBUZ_tune_bad_news_2()  ***********************
*
* Sound 
*
*****************************************************************************/

void vBUZ_tune_bad_news_2(
		void
		)
	{

	vBUZ_play_tune(
		WMCS Q0 WMC W0 WMCS Q0 WMC W0,
		0
		);

	}/* END: vBUZ_tune_bad_news_2() */






/*********************  vBUZ_tune_bad_news_3()  ***********************
*
* Sound 
*
*****************************************************************************/

void vBUZ_tune_bad_news_3(
		void
		)
	{

	vBUZ_play_tune(
		HMDS Q0 HMDS Q0 HMDS Q0 WMC W0,
		0
		);

	}/* END: vBUZ_tune_bad_news_3() */




/*********************  vBUZ_entire_note_scale()  ***********************
*
* Sound 
*
*****************************************************************************/

void vBUZ_entire_note_scale(
		void
		)
	{

	vBUZ_play_tune(
	ELC Q0 ELCS Q0 ELD Q0 ELDS Q0 ELE Q0 ELF Q0 ELFS Q0 ELG Q0 ELGS Q0 ELA Q0 ELAS Q0 ELB Q0 \
	EMC Q0 EMCS Q0 EMD Q0 EMDS Q0 EME Q0 EMF Q0 EMFS Q0 EMG Q0 EMGS Q0 EMA Q0 EMAS Q0 EMB Q0 \
	EHC Q0 EHCS Q0 EHD Q0 EHDS Q0 EHE Q0 EHF Q0 EHFS Q0 EHG Q0 EHGS Q0 EHA Q0 EHAS Q0 EHB Q0,
	0
	);

	}/* END: vBUZ_entire_note_scale() */



/*********************  vBUZ_morrie()  **************************************
*
* Sound played if FRAM is not working.
*
*****************************************************************************/

void vBUZ_morrie(
		void
		)
	{

	vBUZ_play_tune(
	ELC ELCS ELD ELDS ELE ELF ELFS ELG ELGS ELA ELAS ELB
	EMC EMCS EMD EMDS EME EMF EMFS EMG EMGS EMA EMAS EMB \
	EHC EHCS EHD EHDS EHE EHF EHFS EHG EHGS EHA EHAS EHB,
	100
	);

	}/* END: vBUZ_morrie() */





/*********************  vBUZ_raspberry()  ***********************************
*
* Sound used when radio is not working
*
*****************************************************************************/

void vBUZ_raspberry(
		void
		)
	{

	vBUZ_play_tune(
	ELC ELCS ELD ELDS ELE ELF ELFS ELG ELGS ELA ELAS ELB \
	EMC EMCS EMD EMDS EME EMF EMFS EMG EMGS EMA EMAS EMB,
	10
	);

	}/* END: vBUZ_raspberry() */





/*********************  vBUZ_raygun_up()  ***********************************
*
* Sound used when radio board is not plugged in.
*
*****************************************************************************/

void vBUZ_raygun_up(
		void
		)
	{

	vBUZ_play_tune(
	ELC ELCS ELD ELDS ELE ELF ELFS ELG ELGS ELA ELAS ELB \
	EMC EMCS EMD EMDS EME EMF EMFS EMG EMGS EMA EMAS EMB,
	50
	);

	}/* END: vBUZ_raygun_up() */





/*********************  vBUZ_raygun_down()  ***********************************
*
* Sound 
*
*****************************************************************************/

void vBUZ_raygun_down(
		void
		)
	{

	vBUZ_play_tune(
	EMB EMAS EMA EMGS EMG EMFS EMF EME EMDS EMD EMCS EMC \
	ELB ELAS ELA ELGS ELG ELFS ELF ELE ELDS ELD ELCS ELC,
	50
	);

	}/* END: vBUZ_raygun_down() */






/*********************  vBUZ_scale_down()  ***********************************
*
* Sound 
*
*****************************************************************************/

void vBUZ_scale_down(
		void
		)
	{

	vBUZ_play_tune(
	EMB EMAS EMA EMGS EMG EMFS EMF EME EMDS EMD EMCS EMC \
	ELB ELAS ELA ELGS ELG ELFS ELF ELE ELDS ELD ELCS ELC,
	0
	);

	}/* END: vBUZ_scale_down() */





/*********************  vBUZ_scale_up_and_down()  ***********************************
*
* sound 
*
*****************************************************************************/

void vBUZ_scale_up_and_down(
		void
		)
	{

	vBUZ_play_tune(
	ELC ELCS ELD ELDS ELE ELF ELFS ELG ELGS ELA ELAS ELB \
	ELB ELAS ELA ELGS ELG ELFS ELF ELE ELDS ELD ELCS ELC,
	0
	);

	}/* END: vBUZ_scale_up_and_down() */




/*********************  vBUZ_test_successful()  ***********************************
*
* sound 
*
*****************************************************************************/

void vBUZ_test_successful(
		void
		)
	{

	vBUZ_play_tune(
	QMC Q0 QMC Q0 QMC Q0 QMF Q0 QHC W0,
	0
	);

	}/* END: vBUZ_test_successful() */






/*********************  vBUZ_buggs()  ***************************************
*
* sound 
*
*****************************************************************************/

void vBUZ_buggs(
		void
		)
	{

	vBUZ_play_tune(
	ELA E0 EMCS E0 EMDS E0 WMFS E0 QMDS W0 \
	ELA E0 EMCS E0 EMDS E0 WMFS E0 QMDS W0 \
	EMCS E0 EMCS E0 EMCS E0 EMCS E0 EMCS E0 EMCS E0 EMCS E0 EMCS E0 EMCS E0 EMCS E0 W0, 
	0
	);

	}/* END: vBUZ_buggs() */



/*-------------------------------  MODULE END  ------------------------------*/
