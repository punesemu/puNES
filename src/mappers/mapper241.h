/*
 * mapper241.h
 *
 *  Created on: 24/mar/2012
 *      Author: fhorse
 */

#ifndef MAPPER241_H_
#define MAPPER241_H_

#include "common.h"

void map_init_241(void);
void extcl_cpu_wr_mem_241(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_241(WORD address, BYTE openbus, BYTE before);

#endif /* MAPPER241_H_ */
