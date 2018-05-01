/**************************  RAND.C  *****************************************
 *
 * Routines to read and write the static ram
 *
 *
 *
 *
 ******************************************************************************/

/*lint -e526 *//* function not defined */
/*lint -e657 *//* Unusual (nonportable) anonymous struct or union */
/*lint -e714 *//* symbol not referenced */
/*lint -e750 *//* local macro not referenced */
/*lint -e754 *//* local structure member not referenced */
/*lint -e755 *//* global macro not referenced */
/*lint -e757 *//* global declarator not referenced */
/*lint -e758 *//* global union not referenced */
/*lint -e768 *//* global struct member not referenced */

#include <MSP430.h>  //processor description
#include "DIAG.h"			//diagnostic definitions
#include "serial.h"	//serial comm functions
#include "STD.H"			//standard defines
#include "RAND.H"			//random number header
#include "MODOPT.h"			//Modify Options routines
#include "delay.h"
#include "buz.h" //buzzer functions
#include "ADF7020.h" //radio driver
#include "L2FRAM.h"			//level 2 fram routines
// RAND_NUM_SIZE may need to change!  short longs are not used.
extern volatile unsigned char ucRAND_NUM[RAND_NUM_SIZE];

/***********************  uslRAND_getNewSeed()  ********************************
 *
 * This routine stuffs a new seed into the rand area
 *
 *****************************************************************************/

usl uslRAND_getNewSeed( //stuff a new seed into rand array (and ret val)
    void)
{
	uint8 uii, ucCounter;
	uint8 uiLoopMax;
	uint32 ulSeed;

	//If the WiSARD uses the radio for the seed then do it
	if (ucMODOPT_readSingleRamOptionBit(OPTPAIR_USE_RDIO_FOR_RAND_SEED))
	{
		vADF7020_WakeUp();

		//Initialize the timeout counter
		ucCounter = 0;
		//Get the seed from the radio
		ulSeed = uslADF7020_GetRandomNoise();

		//If the seed is all zeros of all ones then retry
		while (ulSeed == 0x00000000 || ulSeed == 0xFFFFFFFF)
		{
			vDELAY_wait100usTic(10000);
			vBUZ_raspberry(); //Annoy listeners
			vSERIAL_sout("SlntRdio\r\n", 10); //Print out failure
			ulSeed = uslADF7020_GetRandomNoise(); //Get the seed from the radio noise

			ucCounter++; //Increment the timeout counter

			//If the CP board is intended to use its radio and the radio fails to work here
			// then the device is essentially bricked so go to sleep and save power
			if (ucCounter == 0xFF)
			{
				__bic_SR_register(GIE);
				LPM4;
			}
		}

		vRAND_stuffFullSysSeed(ulSeed);
		vADF7020_Quit();
	}
	else
	{
		vRAND_stuffFullSysSeed(0x8336F3);
	}

	/* ROLL THE RANDOM NUMBER THE SERIAL NUMBER OF TIMES */
	uiLoopMax = uiL2FRAM_getSnumLo16AsUint();
	for (uii = 0; uii < uiLoopMax; uii++)
	{
		ucRAND_getRolledMidSysSeed(); //lint !e534
	}

	return (uslRAND_getFullSysSeed());


}/* END: uslRAND_getNewSeed() */

/**********************  vRAND_stuffFullSysSeed() ***********************************
 *
 * stuff the random seed
 *
 ******************************************************************************/

void vRAND_stuffFullSysSeed( //Stuff a full seed value
    USL uslRandNum)
{
	ucRAND_NUM[RAND_HI] = (unsigned char) (uslRandNum >> 16);
	ucRAND_NUM[RAND_MD] = (unsigned char) (uslRandNum >> 8);
	ucRAND_NUM[RAND_LO] = (unsigned char) (uslRandNum);

	return;

}/* END: vRAND_stuffFullSysSeed() */

/**********************  uslRAND_getFullSysSeed() ****************************
 *
 * read the random seed
 *
 ******************************************************************************/

USL uslRAND_getFullSysSeed( //RET: unchanged seed from mem.
    void)
{
	USL uslTmp;
	//make sure uslTmp starts off in the zero state before ORing
	uslTmp = 0x00000000;

	uslTmp = (USL) ucRAND_NUM[RAND_HI];
	uslTmp <<= 8;
	uslTmp |= ucRAND_NUM[RAND_MD];
	uslTmp <<= 8;
	uslTmp |= ucRAND_NUM[RAND_LO];

	return (uslTmp);

}/* END: uslRAND_getFullSysSeed() */

///////////////////////  vRAND_getNextNum() /////////////////////////////////
//! \brief Linear feedback shift generator, random number generator
//!
//! Generates a 24-bit deterministic or pseudo-random number
//!
//!
//! \param none
//! \return none
/////////////////////////////////////////////////////////////////////////////
void vRAND_getNextNum(void)
{
	//taps used to choose the bits in the random number to be XORed together
	uint8 ucBit; //!<The values obtained by XORing with the taps
	uint32 ulTmp; //!< Temporarily stores the random number

	//Move the global Rand array into a local variable
	ulTmp = (USL) ucRAND_NUM[RAND_HI];
	ulTmp <<= 8;
	ulTmp |= ucRAND_NUM[RAND_MD];
	ulTmp <<= 8;
	ulTmp |= ucRAND_NUM[RAND_LO];

	//XOR the bits in the taps to get our new bit
	ucBit = ~((ulTmp) ^ (ulTmp >> 1) ^ (ulTmp >> 2) ^ (ulTmp >> 7)) & 0x01;

	//Reload the rand array with the new number
	ucRAND_NUM[RAND_HI] = (unsigned char) ((ulTmp >> 17) | (ucBit << 7));
	ucRAND_NUM[RAND_MD] = (unsigned char) (ulTmp >> 9);
	ucRAND_NUM[RAND_LO] = (unsigned char) (ulTmp >> 1);
}/* END: vRAND_getNextNum() */

/**********************  uslRAND_getRolledFullSysSeed() **********************
 *
 *
 *
 *
 ******************************************************************************/

USL uslRAND_getRolledFullSysSeed( //RET: rolls cur seed & rets new seed
    void)
{

	vRAND_getNextNum();

	return (uslRAND_getFullSysSeed());

}/* END: uslRAND_getRolledFullSysSeed() */

/**********************  ucRAND_getRolledMidSysSeed() *********************************
 *
 * YES ROLL, RET MID BYTE
 *
 ******************************************************************************/

unsigned char ucRAND_getRolledMidSysSeed(
//RET: rolls the current seed value and returns the middle byte
    void)
{
	USL uslTmp;

	uslTmp = uslRAND_getRolledFullSysSeed();

	return ((unsigned char) (uslTmp >> 8));

}/* END: ucRAND_getRolledMidSysSeed() */

/**********************  ucRAND_getMidSysSeed()  ********************************
 *
 * NO ROLL, RET MID BYTE
 *
 *****************************************************************************/

unsigned char ucRAND_getMidSysSeed( //RET: middle seed byte without rolling
    void)
{

	return (ucRAND_NUM[RAND_MD]);

}/* END: ucRAND_getMidSysSeed() */

/*******************  uslRAND_getRolledFullForeignSeed() **********************
 *
 * RET: NEST ROLL OF THE PASSED VALUE, and RESTORE THE SYSTEM SEED
 *
 ******************************************************************************/

USL uslRAND_getRolledFullForeignSeed( //RET: ROLL THIS NUM, RET FULL VALUE
    usl uslForeignRandSeed)
{
	usl uslRolledForeignSeed;
	usl uslSavedSysSeed;

	/* SAVE THE SYSTEM SEED */
	uslSavedSysSeed = uslRAND_getFullSysSeed();

	/* STUFF THE NEW SEED */
	vRAND_stuffFullSysSeed(uslForeignRandSeed);

	/* ROLL THE NEW SEED AND GET THE RESULT */
	uslRolledForeignSeed = uslRAND_getRolledFullSysSeed();

	/* NOW RESTORE THE SYSTEM SEED */
	vRAND_stuffFullSysSeed(uslSavedSysSeed);

	return (uslRolledForeignSeed);

}/* END: uslRAND_getRolledFullForeignSeed() */

/*******************  uslRAND_getMidForeignSeed() **********************
 *
 * RET: NEST ROLL OF THE PASSED VALUE, and RESTORE THE SYSTEM SEED
 *
 ******************************************************************************/

uchar ucRAND_getMidForeignSeed( //RET: ROLL THIS NUM, RET FULL VALUE
    usl uslForeignRandSeed)
{
	usl uslRolledForeignSeed;

	uslRolledForeignSeed = uslRAND_getRolledFullForeignSeed(uslForeignRandSeed);

	return ((uchar) (uslRolledForeignSeed >> 8));

}/* END: uslRAND_getMidForeignSeed() */

/*-------------------------------  MODULE END  ------------------------------*/
