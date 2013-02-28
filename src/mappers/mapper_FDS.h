/*
 * mapper_FDS.h
 *
 *  Created on: 29/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER_FDS_H_
#define MAPPER_FDS_H_

#include "common.h"

enum { FDS_MAPPER = 0x1000 };

void map_init_FDS(void);
void extcl_cpu_every_cycle_FDS(void);
void extcl_apu_tick_FDS(void);

#endif /* MAPPER_FDS_H_ */
