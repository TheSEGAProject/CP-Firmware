/*
 * gs.h
 *
 *  Created on: Jul 9, 2014
 *      Author: cp397
 */

#ifndef GS_H_
#define GS_H_

void vGS_ReportToGardenServer(void);
void vGS_SendRTS(void);
unsigned char ucGS_WaitForRTR(void);
void vGS_SynchGardenServer(void);
void vGS_Init(void);
#endif /* GS_H_ */
