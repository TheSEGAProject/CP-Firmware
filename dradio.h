
/***************************  DRADIO.H  ****************************************
*
* Header for DRADIO routine pkg
*
*
* V1.00 01/28/2005 wzr
*	started
*
******************************************************************************/

#ifndef DRADIO_H_INCLUDED
	#define DRADIO_H_INCLUDED



/* ROUTINE DEFINITIONS */

unsigned char ucDRADIO_continuousXmitTestMsg(
		uchar ucChanIdx
		);

unsigned char ucDRADIO_continuousReceiveTestMsg(
		uchar ucChanIdx
		);

uchar ucDRADIO_run_sniffer(
		uchar ucChanIdx
		);



#endif /* DRADIO_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
