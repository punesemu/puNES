/*
 * mapperAxROM.h
 *
 *  Created on: 01/mar/2011
 *      Author: fhorse
 */

#ifndef MAPPERAXROM_H_
#define MAPPERAXROM_H_

#include "common.h"

enum { AMROM = 2, BAD_INES_WWFWE = 3, BBCARUNL = 4 };

void map_init_AxROM(void);
void extcl_cpu_wr_mem_AxROM(WORD address, BYTE value);

#endif /* MAPPERAXROM_H_ */
