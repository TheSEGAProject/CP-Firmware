/***************************  SENSOR.H  ****************************************
 *
 * Header for SENSOR routine pkg
 *
 * NOTE: See Main.h for the disk layout map.
 *
 *
 *
 * V1.00 10/04/2003 wzr
 *	started
 *
 ******************************************************************************/

#ifndef SENSOR_H_INCLUDED
#define SENSOR_H_INCLUDED

#define L_JUSTIFY				0
#define R_JUSTIFY				1

/*-------------------------  NOTE  -------------------------------------------
 *
 * If you are adding or modifying this sensor table be sure to change the
 * runtime sensor name code to match it.
 *
 **---------------------------------------------------------------------------*/

/*------------------  SENSOR LIST  --------------------------------------*/
#define SENSOR_NONE					0

#define SENSOR_FAKE_1				1
#define SENSOR_FAKE_2				2

#define SENSOR_SD_CODE_VERSION		3

#define SENSOR_SP_MSG_VERSIONS		4
#define SD_MSG_VERSION_HI		0x01
#define SD_MSG_VERSION_LO		0x01
#define SD_MSG_VERSION (((unsigned int)SD_MSG_VERSION_HI<<8) | ((unsigned int)SD_MSG_VERSION_LO))

#define SENSOR_TTY					5

#define SENSOR_UNUSED_1				6 //
#define SENSOR_TC_1					7
#define SENSOR_TC_2					8
#define SENSOR_TC_3					9
#define SENSOR_TC_4					10

#define SENSOR_LIGHT_1				11 //16 Bit Follower = Reading (uint)
#define SENSOR_LIGHT_2				12 //16 Bit Follower = Reading (uint)
#define SENSOR_LIGHT_3				13 //16 Bit Follower = Reading (uint)
#define SENSOR_LIGHT_4				14 //16 Bit Follower = Reading (uint)
#define SENSOR_SOIL_MOISTURE_1		15 //16 Bit Follower = Reading (uint)
#define SENSOR_SOIL_MOISTURE_2		16 //16 Bit Follower = Reading (uint)
#define SENSOR_GENERAL_PURP_1		17 //16 Bit Follower = Reading (uint)
#define SENSOR_GENERAL_PURP_2		18 //16 Bit Follower = Reading (uint)
#define SENSOR_ONEWIRE_0			19 //16 Bit Follower = Reading (uint)
#define SENSOR_BATT_VOLTAGE			20 //16 Bit Follower = Reading (uint)
#define SENSOR_SD_BOARD_MSG			21 //16 Bit Follower = Sub-msg number
#define SENSOR_BR_BOARD_MSG			22 //16 Bit Follower = Sub-msg number
#define FLASH_MSG_ERR_CRCERR				 1
#define FLASH_MSG_ERR_SIZTOOSMALL			 2
#define FLASH_MSG_ERR_BADSIZ2122			 3
#define FLASH_MSG_ERR_BADSIZ2425			 4
#define FLASH_MSG_ERR_BADSIZ2728			 5
#define FLASH_MSG_ERR_BADSIZ30				 6
#define FLASH_MSG_ERR_BADTYPE				 7
#define FLASH_MSG_ERR_BADSEQNUM				 8
#define FLASH_MSG_ERR_BADSRCID				 9
#define FLASH_MSG_ERR_BADAGTID				10
#define FLASH_MSG_ERR_BADDESTID				11
#define FLASH_MSG_ERR_BADTIME				12
#define FLASH_MSG_ERR_BADSENSORNUM			13
#define FLASH_MSG_ERR_BADSENSORINLASTENTRY	14
#define FLASH_MSG_ERR_RESERVED1				15
#define FLASH_MSG_ERR_RESERVED2				16
#define FLASH_MSG_ERR_RESERVED3				17
#define FLASH_MSG_ERR_RESERVED4				18
#define FLASH_MSG_ERR_RESERVED5				19
#define BRAIN_RESTART						20
#define BATTERY_LOW_RESTART					21



#define MAX_BRAIN_MSG_IDX					22

//	#define SENSOR_SOM2_LINK_INFO		23 //16 Bit Follower = SN of radio link
//	#define SENSOR_SOM2_INFO_REASON		24 //16 Bit Follower = Sub-Msg Number
//		#define SOM2_BLANK_REASON				0 //This value not used
//		#define SOM2_OM1_MISSED					1 //Appt missed
//		#define SOM2_OM1_MISSED_LINK_BROKEN		2 //Appt missed - link severed
//		#define SOM2_TIMEOUT_NONE_SENT			3 //Received OM1 timed out before OM2 could be sent
//		#define SOM2_TIMEOUT_BELOW_300MS_LIMIT	4 //Received OM1 ran out of time
//		#define SOM2_OM3_MISSED					5 //Appt made, Confirm missed
//		#define SOM2_LINK_ESTABLISHED			6 //New SOM2 link established
//		#define SOM2_LINK_REPLY_TOO_LATE		7 //Missed Linkup because clks out of sync

#define MAX_SOM2_INFO_IDX				8

#define SENSOR_TIMECHANGE_HI		25 //16 Bit Follower = New Time HI uint
#define SENSOR_TIMECHANGE_LO		26 //16 Bit Follower = New Time LO uint
#define SENSOR_ROM2_LINK_INFO		27 //16 Bit Follower = SN of radio link
#define SENSOR_ROM2_INFO_REASON		28 //16 Bit Follower = Sub_msg Number
#define ROM2_BLANK_REASON				0 //This value not used
#define ROM2_OM1_START_TIME_MISMATCH	1 //Appt missed, Time not sync'ed
#define ROM2_OM1_TOO_LATE_TO_SEND		2 //Appt missed, slot started too late
#define ROM2_MISSED						3 //Appt missed
#define ROM2_LINK_BROKEN				4 //Appt missed - link severed
#define ROM2_LINK_ESTABLISHED			5 //New ROM2 link established
#define MAX_ROM2_INFO_IDX				6

#define SENSOR_KUN_TC_1				29 //16 Bit Follower = Reading (uint)
#define SENSOR_KUN_TC_2				30 //16 Bit Follower = Reading (uint)
#define SENSOR_KUN_TC_3				31 //16 Bit Follower = Reading (uint)
#define SENSOR_KUN_TC_4				32 //16 Bit Follower = Reading (uint)
#define SENSOR_WIND_SPEED_MIN_WHOLE_VS	33 //16 Bit Follower = whole number part (int)
#define SENSOR_WIND_SPEED_MIN_FRAC_VS	34 //16 Bit Follower = decimal number part (uint)
#define SENSOR_WIND_SPEED_AVE_WHOLE_VS	35 //16 Bit Follower = whole number part (int)
#define SENSOR_WIND_SPEED_AVE_FRAC_VS	36 //16 Bit Follower = decimal number part (uint)
#define SENSOR_WIND_SPEED_MAX_WHOLE_VS	37 //16 Bit Follower = whole number part (int)
#define SENSOR_WIND_SPEED_MAX_FRAC_VS	38 //16 Bit Follower = decimal number part (uint)
#define SENSOR_WIND_DIR_MIN_WHOLE_VS	39 //16 Bit Follower = whole number part (int)
#define SENSOR_WIND_DIR_MIN_FRAC_VS		40 //16 Bit Follower = decimal number part (uint)
#define SENSOR_WIND_DIR_AVE_WHOLE_VS	41 //16 Bit Follower = whole number part (int)
#define SENSOR_WIND_DIR_AVE_FRAC_VS		42 //16 Bit Follower = decimal number part (uint)
#define SENSOR_WIND_DIR_MAX_WHOLE_VS	43 //16 Bit Follower = whole number part (int)
#define SENSOR_WIND_DIR_MAX_FRAC_VS		44 //16 Bit Follower = decimal number part (uint)

#define SENSOR_AIR_PRESS_WHOLE_VS		45 //16 Bit Follower = whole number part (int)
#define SENSOR_AIR_PRESS_FRAC_VS		46 //16 Bit Follower = decimal number part (uint)

#define SENSOR_AIR_TEMP_WHOLE_VS		47 //16 Bit Follower = whole number part (int)
#define SENSOR_AIR_TEMP_FRAC_VS			48 //16 Bit Follower = decimal number part (uint)
#define SENSOR_INTERNAL_TEMP_WHOLE_VS	49 //16 Bit Follower = whole number part (int)
#define SENSOR_INTERNAL_TEMP_FRAC_VS	50 //16 Bit Follower = decimal number part (uint)
#define SENSOR_REL_HUMID_WHOLE_VS		51 //16 Bit Follower = whole number part (int)
#define SENSOR_REL_HUMID_FRAC_VS		52 //16 Bit Follower = decimal number part (uint)

#define SENSOR_RAIN_ACC_WHOLE_VS		53 //16 Bit Follower = whole number part (int)
#define SENSOR_RAIN_ACC_FRAC_VS			54 //16 Bit Follower = decimal number part (uint)
#define SENSOR_RAIN_DUR_WHOLE_VS		55 //16 Bit Follower = whole number part (int)
#define SENSOR_RAIN_DUR_FRAC_VS			56 //16 Bit Follower = decimal number part (uint)
#define SENSOR_RAIN_INTSTY_WHOLE_VS  	57 //16 Bit Follower = whole number part (int)
#define SENSOR_RAIN_INTSTY_FRAC_VS   	58 //16 Bit Follower = decimal number part (uint)

#define SENSOR_HAIL_ACC_WHOLE_VS		59 //16 Bit Follower = whole number part (int)
#define SENSOR_HAIL_ACC_FRAC_VS			60 //16 Bit Follower = decimal number part (uint)
#define SENSOR_HAIL_DUR_WHOLE_VS		61 //16 Bit Follower = whole number part (int)
#define SENSOR_HAIL_DUR_FRAC_VS			62 //16 Bit Follower = decimal number part (uint)
#define SENSOR_HAIL_INTSTY_WHOLE_VS  	63 //16 Bit Follower = whole number part (int)
#define SENSOR_HAIL_INTSTY_FRAC_VS   	64 //16 Bit Follower = decimal number part (uint)

#define SENSOR_HEAT_TEMP_WHOLE_VS  65 //16 Bit Follower = whole number part (int)
#define SENSOR_HEAT_TEMP_FRAC_VS   66 //16 Bit Follower = decimal number part (uint)
#define SENSOR_HEAT_VOLTS_WHOLE_VS  67 //16 Bit Follower = whole number part (int)
#define SENSOR_HEAT_VOLTS_FRAC_VS   68 //16 Bit Follower = decimal number part (uint)
#define SENSOR_SUPPLY_VOLTS_WHOLE_VS  69 //16 Bit Follower = whole number part (int)
#define SENSOR_SUPPLY_VOLTS_FRAC_VS   70 //16 Bit Follower = decimal number part (uint)
#define SENSOR_REF_VOLTS_WHOLE_VS	71 //16 Bit Follower = whole number part (int)
#define SENSOR_REF_VOLTS_FRAC_VS	72 //16 Bit Follower = decimal number part (uint)
#define SENSOR_SAP_1				73
#define SENSOR_SAP_2				74
#define SENSOR_SAP_3				75
#define SENSOR_SAP_4				76

/*------------------------*/
#define SENSOR_MAX_VALUE			77

void vSENSOR_showSensorName(uchar ucSensorNum, uchar ucJustifyFlg //L_JUSTIFY, R_JUSTIFY
    );

#endif /* SENSOR_H_INCLUDED */

/* --------------------------  END of MODULE  ------------------------------- */
