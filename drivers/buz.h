/***************************  BUZ.H  ****************************************
*
* Header for BUZ routine pkg
*
*
*
* V1.00 5/11/2003 wzr
*	started
*
******************************************************************************/

#ifndef BUZ_H_INCLUDED
	#define BUZ_H_INCLUDED

	/* ROUTINE DEFINITIONS */
	void vBUZ(
		uint uiFreq,
		uint uiDuration_in_100us
		);

	void vBUZ_tune_Blip(void);

	void vBUZ_blink_buzzer(
		unsigned char ucCount
		);

	void vBUZ_play_tune(
		const char *cTunePtr,		
		uint uiFixedDuration			//for sliding notes
		);

	void vBUZ_tune_mary(				//not used
		void
		);

	void vBUZ_tune_imperial(			//system wakeup from hibernation
		void
		);

	void vBUZ_tune_TaDah_TaDah(			//this unit has an inbound lnk with net
		void
		);

	void vBUZ_tune_bad_news_1(			//battery is low (some diagnostics)
		void
		);

	void vBUZ_tune_bad_news_2(			//FRAM is not Formatted (& Some Diagnostics)
		void
		);

	void vBUZ_tune_bad_news_3(			//  (some diagnostics)
		void
		);

	void vBUZ_entire_note_scale(		//not used
		void
		);

	void vBUZ_morrie(					//FRAM is not working
		void
		);

	void vBUZ_raspberry(				//radio not working
		void
		);

	void vBUZ_raygun_up(				//radio is not plugged in
		void
		);

	void vBUZ_raygun_down(				//FLASH is not working
		void
		);

	void vBUZ_scale_down(				//SD board failures
		void
		);

	void vBUZ_scale_up_and_down(
		void
		);

	void vBUZ_test_successful(
		void
		);

	void vBUZ_buggs(
		void
		);

		
#endif /* BUZ_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
