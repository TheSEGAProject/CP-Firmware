#include "serial.h"
#include "SD_Card.h"
#include "SD_Card_Testing.h"

extern SD_R1 g_r1Response;
unsigned char ucaTestArray[SD_CARD_BLOCKLEN];

void SD_Testing_Fill(unsigned char ucByte)
{
	int i;
	for(i=0; i<SD_CARD_BLOCKLEN; i++)
		ucaTestArray[i] = ucByte;
}

void SD_Testing_FillInc()
{
	int i;
	for(i=0; i<SD_CARD_BLOCKLEN; i++)
	{
		ucaTestArray[i] = (unsigned char)(i%0xFF);
	}
}

unsigned long SD_Testing_Basic()
{
	unsigned long i, clearByte;
	unsigned long j, k;
	unsigned int blocks[] = { 50504, 52, 326, 1016, 50555 };
	uchar ucRetVal;
	ulong ulFailCount = 0;

	vSD_PowerOn();

	//Initialize SD card and write to the next block
	if (ucSD_Init() == SD_FAILED)
		return SD_FAILED;

	//for(i=1; i<ulSD_GetCapacity(); i += 1)
	for(i=1; i<16; i += 1)
	{
		//Fill the test buffer with increasing values
		SD_Testing_FillInc();

		k = 0;
		// If the write is good then read
		while (k++ < 20 && SD_Write_Block(ucaTestArray,i) == SD_FAILED){
			if (k == 20)
				ucRetVal = SD_FAILED;
		}

		//Init and read
		SD_Testing_Fill(0x00);

		k = 0;
		// If the read is good then verify
		while (k++ < 20 && SD_Read_Block(ucaTestArray, i) == SD_FAILED){
			if (k == 20)
				ucRetVal = SD_FAILED;
		}

		//Validate Buffer
		for (j = 0; j < SD_CARD_BLOCKLEN; j++) {
			if (ucaTestArray[j] != (unsigned char) (j % 0xFF))
				ucRetVal =  SD_FAILED;
		}

		k = 0;
		//Init and clear
		while (k++ < 20 && SD_Erase_Block(i) == SD_FAILED){
			if (k == 20)
				ucRetVal = SD_FAILED;
		}

		//Init and read
		SD_Testing_Fill(0x00);
		k = 0;
		while (k++ < 20 && SD_Read_Block(ucaTestArray, i) == SD_FAILED){
			if (k == 20)
				ucRetVal = SD_FAILED;
		}

		//Check buffer
		clearByte = ucaTestArray[0];
		if(clearByte != 0x00 && clearByte != 0xFF)
			ucRetVal = SD_FAILED;
		for(j=1; j<SD_CARD_BLOCKLEN; j++)
		{
			if( ucaTestArray[j] != clearByte )
				ucRetVal = SD_FAILED;
		}

		if (ucRetVal == SD_FAILED){
			ulFailCount++;
			ucRetVal = SD_SUCCESS;

			vSD_PowerOff();
			__delay_cycles(16000000);
			vSD_PowerOn();
			ucSD_Init();
		}
	}

	vSD_PowerOff();

	return ulFailCount;
}

void SD_Testing_EnumerateResponse()
{
	vSERIAL_sout("Response (0x", 12);
	vSERIAL_HB8out(g_r1Response.uiRaw);
	vSERIAL_sout("): ", 3);

	if(g_r1Response.flags.ucStartBit == 1)       vSERIAL_sout("START_BIT ", 10);
	if(g_r1Response.flags.ucParameterError == 1) vSERIAL_sout("PARAM_ERR ", 10);
	if(g_r1Response.flags.ucIllegalCommand == 1) vSERIAL_sout("ILLEGAL_CMD ", 12);
	if(g_r1Response.flags.ucIdleState == 1)      vSERIAL_sout("IDLE_STATE ", 11);
	if(g_r1Response.flags.ucEraseSeqError == 1)  vSERIAL_sout("ERASE_SEQ_ERR ", 14);
	if(g_r1Response.flags.ucCRCError == 1)       vSERIAL_sout("CRC_ERR ", 8);
	if(g_r1Response.flags.ucAddressError == 1)   vSERIAL_sout("ADDR_ERR ", 9);
	vSERIAL_crlf();
}

void SD_Testing_ShowBlock()
{
	int i,j;
	for(i=0; i<32; i++)
	{
		for(j=0; j<16; j++)
		{
			vSERIAL_sout("0x", 2);
			vSERIAL_HB8out(ucaTestArray[i*16+j]);
			vSERIAL_sout(" ", 1);
		}
		vSERIAL_crlf();
	}
}
