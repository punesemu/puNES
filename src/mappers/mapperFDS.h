/*
 * mapperFDS.h
 *
 *  Created on: 29/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPERFDS_H_
#define MAPPERFDS_H_

#include "common.h"

enum { FDS_MAPPER = 0x1000 };

void mapInit_FDS(void);
void extclCPUEveryCycle_FDS(void);
void extclApuTick_FDS(void);

#endif /* MAPPERFDS_H_ */
