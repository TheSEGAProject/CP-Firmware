/*
 * ucs.h
 *
 *  Created on: Mar 20, 2013
 *      Author: cp397
 */

#ifndef UCS_H_
#define UCS_H_

//! \brief Clock frequencies for the digitally controlled oscillator
//! \def DCO_1MHZ
//! \def DCO_16MHZ
#define DCO_1MHZ		1
#define DCO_16MHZ		16

//UCS function declarations
void vUCS_InitializeClock(void);
void vClock_SetFrequency(unsigned char ucFrequency);

#endif /* UCS_H_ */
