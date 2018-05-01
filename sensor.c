


/**************************  SENSOR.C  *****************************************
*
* Routines to show SENSORs during events
*
*
* V1.00 10/04/2003 wzr
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
/*lint -e752 */		/* local declarator not referenced */
/*lint -e758 */		/* global union not referenced */
/*lint -e768 */		/* global struct member not referenced */



#include "std.h"			//standard defines
#include "hal/config.h"			//system configuration description file
#include "serial.h"			//serial IO port stuff


#define L_JUSTIFY				0
#define R_JUSTIFY				1


#define FIRST_NAME_BLK_SIZE		33
#define SECOND_NAME_BLK_SIZE 	32
#define THIRD_NAME_BLK_SIZE		12

#define SENSOR_NAME_LEN			 6



const char *cpaSensorName1[FIRST_NAME_BLK_SIZE] =
	{
	"  NONE",		//	 0 //blank
	"  FAK1",		//	 1
	"  FAK2",		//	 2
	"SDcodV",		//	 3 //SD code version
	"SDmsgV",		//	 4 //SD msg version
	"   TTY",		//	 5
	"  EMsg",		//	 6
	"   TC1",		//	 7
	"   TC2",		//	 8
	"   TC3",		//	 9
	"   TC4",		//	10
	"   LT1",		//	11 //+Reading (uint)
	"   LT2",		//	12 //+Reading (uint)
	"   LT3",		//	13 //+Reading (uint)
	"   LT4",		//	14 //+Reading (uint)
	"SlWet1",		//	15 //+Reading (uint)
	"SlWet2",		//	16 //+Reading (uint)
	"   GP1",		//	17 //+Reading (uint)
	"   GP2",		//	18 //+Reading (uint)
	"Owire0",		//	19
	"Batt-V",		//	20 //+Reading (uint)
	"SD-Msg",		//	21 //+msg number
	"BR-Msg",		//	22 //+msg number
	"SOM2Lk",		//	23 //+SN of SOM2 link
	"SOM2In",		//	24 //+SOM2 Link msg reason
	"Tchg_H",		//	25 //+New Time HI uint
	"Tchg_L",		//	26 //+New Time LO uint
	"ROM2Lk",		//	27 //+SN of ROM2 link
	"ROM2In",		//	28 //+ROM2 Link msg reason
	" K_TC1",		//	29 //+Reading (uint)
	" K_TC2",		//	30 //+Reading (uint)
	" K_TC3",		//	31 //+Reading (uint)
	" K_TC4"		//	32 //+Reading (uint)
	};


const char *cpaSensorName2[SECOND_NAME_BLK_SIZE] =
	{
	" WSmin",		//	33 //+HI number part (int)
	"WSMinL",		//	34 //+decimal number part (uint)
	" WSave",		//	35 //+HI number part (int)
	"WSaveL",		//	36 //+decimal number part (uint)
	" WSmax",		//	37 //+HI number part (int)
	"WSmaxL",		//	38 //+decimal number part (uint)
	" WDmin",		//	39 //+HI number part (int)
	"WDminL",		//	40 //+decimal number part (uint)
	" WDave",		//	41 //+HI number part (int)
	"WDaveL",		//	42 //+decimal number part (uint)
	" WDmax",		//	43 //+HI number part (int)
	"WDmaxL",		//	44 //+decimal number part (uint)
	"  AirP",		//	45 //+HI number part (int)
	"AirP_L",		//	46 //+decimal number part (uint)
	"  AirT",		//	47 //+HI number part (int)
	"AirT_L",		//	48 //+decimal number part (uint)
	" InsdT",		//	49 //+HI number part (int)
	"InsdTL",		//	50 //+decimal number part (uint)
	" RlHum",		//	51 //+HI number part (int)
	"RlHumL",		//	52 //+decimal number part (uint)
	" RnAcc",		//	53 //+HI number part (int)
	"RnAccL",		//	54 //+decimal number part (uint)
	" RnDur",		//	55 //+HI number part (int)
	"RnDurL",		//	56 //+decimal number part (uint)
	" RnInt",		//  57 //+HI number part (int)
	"RnIntL",		//  58 //+decimal number part (uint)
	" HlAcc",		//	59 //+HI number part (int)
	"HlAccL",		//	60 //+decimal number part (uint)
	" HlDur",		//	61 //+HI number part (int)
	"HlDurL",		//	62 //+decimal number part (uint)
	" HlInt",		//  63 //+HI number part (int)
	"HlIntL"		//  64 //+decimal number part (uint)
	};


const char *cpaSensorName3[THIRD_NAME_BLK_SIZE] =
	{
	" HeatT",		//  65 //+HI number part (int)
	"HeatTL",		//  66 //+decimal number part (uint)
	" HeatV",		//  67 //+HI number part (int)
	"HeatVL",		//  68 //+decimal number part (uint)
	"  SrcV",		//  69 //+HI number part (int)
	"SrcV_L",		//  70 //+decimal number part (uint)
	"  RefV",		//	71 //+HI number part (int)
	"RefV_L",		//	72 //+decimal number part (uint)
	"  SAP1",		//  73 //+Reading (uint)
	"  SAP2",		//  74 //+Reading (uint)
	"  SAP3",		//  75 //+Reading (uint)
	"  SAP4"		//  76 //+Reading (uint)
	};





/*****************************  CODE STARTS HERE  ****************************/




/****************  vSENSOR_showJustifiedStr()  *******************************
*
*
*
******************************************************************************/
void vSENSOR_showJustifiedStr(
		const char *cpSrcPtr,
		uchar ucJustifyFlg
		)
	{
//	uchar ucii;
//	uchar ucjj;
//	char cChar;
//
//	if(ucJustifyFlg)
//		{
//		vSERIAL_rom_sout(cpSrcPtr);
//		return;
//		}
//
//	/* DO LEFT JUSTIFY PRINTING */
//	for(ucii=0,ucjj=0;  ucii<SENSOR_NAME_LEN;  ucii++)
//		{
//		cChar = *(cpSrcPtr+ucii);
//		if(cChar == ' ') continue;
//		vSERIAL_bout((uchar)cChar);
//		ucjj++;
//		}/* END: for(ucii) */
//
//	/* FINISH WITH SPACES */
//	for(ucii=ucjj; ucii<SENSOR_NAME_LEN; ucii++)
//		{
//		vSERIAL_bout(' ');
//		}/* END: for(ucii) */

	return;

	}/* END: vSENSOR_showJustifiedStr() */






/****************** vSENSOR_showSensorName()  ****************************************
*
*
*
******************************************************************************/
void vSENSOR_showSensorName(
		uchar ucSensorNum,
		uchar ucJustifyFlg		//L_JUSTIFY, R_JUSTIFY
		)
	{
//	const char *cpStrPtr;
//
//	if(ucSensorNum < FIRST_NAME_BLK_SIZE)
//		{
//		cpStrPtr = cpaSensorName1[ucSensorNum];
//		goto ssn_exit;
//		}
//
//	ucSensorNum -= FIRST_NAME_BLK_SIZE;
//	if(ucSensorNum < SECOND_NAME_BLK_SIZE)
//		{
//		cpStrPtr = cpaSensorName2[ucSensorNum];
//		goto ssn_exit;
//		}
//
//	ucSensorNum -= SECOND_NAME_BLK_SIZE;
//	if(ucSensorNum < THIRD_NAME_BLK_SIZE)
//		{
//		cpStrPtr = cpaSensorName3[ucSensorNum];
//		goto ssn_exit;
//		}
//
//	vSERIAL_sout(" --- ", 5);
//	vSERIAL_sout("(", 1);
//	vSERIAL_UIV8out(ucSensorNum);
//	vSERIAL_sout(")", 1);
//	return;
//
//ssn_exit:
//	vSENSOR_showJustifiedStr(cpStrPtr, ucJustifyFlg);
	return;

	}/* END: vSENSOR_showSensorName() */



/*-------------------------------  MODULE END  ------------------------------*/
