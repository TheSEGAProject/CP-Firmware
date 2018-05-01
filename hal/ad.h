  
/***************************  AD.H  ****************************************
*
* A/D header file
*
* V1.00 09/23/2002 wzr
*	started
*
******************************************************************************/

#ifndef AD_H_INCLUDED
	#define AD_H_INCLUDED

	/* A/D CHANNEL NAMES */
	#define BATT_AD_CHAN		0
	#define INTERNAL_TEMP		10


	/* CHANNEL SELECT WAIT FLAG TYPES */
	#define YES_WAIT	1
	#define NO_WAIT		0

	/* CHANNEL SELECT FLAG TYPES */
	#define YES_SELECT_CHAN		1
	#define  NO_SELECT_CHAN		0


	/* ROUTINE DEFINITIONS */

	void vAD_init(
		void
		);
		
	void vAD_setup_analog_bits(
		unsigned char ucChanNum
		);

	void vAD_select_chan(
		unsigned char ucChanNum,		//chan num (0-15)
		unsigned char ucWaitType		//YES_WAIT, NO_WAIT
		);

	unsigned int uiAD_read(
		unsigned char ucChanNum,		//(0 - 15)
		unsigned char ucChanSelectFlag	//(YES_SELECT_CHAN, NO_SELECT_CHAN)
		);

	unsigned int uiAD_full_init_setup_read_and_shutdown(
		unsigned char ucChanNum		//(0 - 15)
		);

	void vAD_clearout_analog_bits(
		unsigned char ucChanNum		//(0 - 15)
		);

	void vAD_quit(
		void
		);

	unsigned int uiAD_CalibrateTemp(unsigned int uiTempRaw);

	#endif /* AD_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
