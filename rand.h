
/******************************  RAND.H  *************************************
*
* RAND number header file
*
*
*
*
* V1.00 09/17/2002 wzr
*	started
*
******************************************************************************/

#ifndef RAND_H_INCLUDED
	#define RAND_H_INCLUDED

/*****************************  NOTE  ****************************************
* Data from the random number generator is found in a 3 byte array as follows:
* 
* ucRAND_NUM[RAND_HI]
* ucRAND_NUM[RAND_MD]
* ucRAND_NUM[RAND_LO]
*
******************************************************************************/

 #define BLANK_SEED_VAL		0x000000

 #define RAND_NUM_SIZE 3

 #define RAND_HI 0
 #define RAND_MD 1
 #define RAND_LO 2


 usl uslRAND_getNewSeed(	//stuff a new seed into rand array (and ret val too)
		void
		);

 void vRAND_stuffFullSysSeed(		//STUFF FULL VALUE
		usl uslRandNum				//Seed to stuff
		);

 usl uslRAND_getFullSysSeed(	//RET: NO ROLL, RET FULL VALUE
		void
		);

 usl uslRAND_getRolledFullSysSeed(	//RET: YES ROLL, RET FULL VALUE
		void
		);


// change! needed
 void vRAND_getNextNum(		// this was an assembler routine need to decide how you want to handle this. 
		void
		);



 uchar ucRAND_getMidSysSeed( //RET: NO ROLL, RET MID BYTE
		void
		);

 uchar ucRAND_getRolledMidSysSeed( //RET: YES ROLL, RET MID BYTE
		void
		);





 usl uslRAND_getRolledFullForeignSeed(  //RET: ROLL THIS NUM, RET FULL VALUE
		usl uslForeignRandNum
		);

 uchar ucRAND_getMidForeignSeed(  //RET: ROLL THIS NUM, RET MID BYTE
		usl uslForeignRandNum
		);




#endif /* RAND_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */

