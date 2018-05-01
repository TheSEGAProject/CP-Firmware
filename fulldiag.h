
/***************************  FULLDIAG.H  ****************************************
*
* Header for FULLDIAG routine pkg
*
*
*
* V1.00 12/28/2004 wzr
*	started
*
******************************************************************************/

#ifndef FULLDIAG_H_INCLUDED
	#define FULLDIAG_H_INCLUDED


 /* KEY RETURN VALUES FOR ACTION KEYS */
 #define DIAG_ACTION_KEY_NONE					0
 #define DIAG_ACTION_KEY_PLAIN_KEY				1
 #define DIAG_ACTION_KEY_ERR					1 //same as plain key
 #define DIAG_ACTION_KEY_TOGGLE_HALT_ON_ERR		2
 #define DIAG_ACTION_KEY_TOGGLE_LOOP_FLAG		3
 #define DIAG_ACTION_KEY_TOGGLE_MUTE_FLAG		4
 #define DIAG_ACTION_KEY_NEXT_TEST_FLAG			5
 #define DIAG_ACTION_KEY_PREV_TEST_FLAG			6
 #define DIAG_ACTION_KEY_QUIT					7
 #define DIAG_ACTION_KEY_STOP					8
 #define DIAG_ACTION_KEY_TELL					9
 #define DIAG_ACTION_KEY_EXIT					10




 /* ROUTINE DEFINITIONS */

 void vFULLDIAG_run_module(
		void
		);

 void vFULLDIAG_deployment_diagnostic(void);

#endif /* FULLDIAG_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
