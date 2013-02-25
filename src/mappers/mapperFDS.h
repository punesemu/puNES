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

void map_init_FDS(void);
void extcl_cpu_every_cycle_FDS(void);
void extcl_apu_tick_FDS(void);

#endif /* MAPPERFDS_H_ */
