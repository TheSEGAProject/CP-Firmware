/*
 * PMM.h
 *
 *  Created on: May 13, 2014
 *      Author: cp397
 */

#ifndef PMM_H_
#define PMM_H_

/* LOW VOLTAGE DETECT DEFINITIONS */
#define BATT_LOW_V300	SVSMHRRL_6
#define BATT_LOW_V270	SVSMHRRL_5
#define BATT_LOW_V240	SVSMHRRL_4
#define BATT_LOW_V226	SVSMHRRL_3
#define BATT_LOW_V214	SVSMHRRL_2
#define BATT_LOW_V194	SVSMHRRL_1
#define BATT_LOW_V174	SVSMHRRL_0


unsigned char ucPMM_chkLowVoltage(//2=Ok Voltage, 1=Bad Detector, 0=Low Voltage
		unsigned char ucLowVoltLevel /* LOW_V300, LOW_V194 ... (see defines) */
		);

void vPMM_Reset(void);

void vPMM_SetCoreVoltage(unsigned char ucMCLKFreq);

#endif /* PMM_H_ */
