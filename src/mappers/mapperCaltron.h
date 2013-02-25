/*
 * mapperCaltron.h
 *
 *  Created on: 16/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERCALTRON_H_
#define MAPPERCALTRON_H_

#include "common.h"

struct _caltron {
	BYTE reg;
} caltron;

void mapInit_Caltron(void);
void extcl_cpu_wr_mem_Caltron(WORD address, BYTE value);
BYTE extcl_save_mapper_Caltron(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPERCALTRON_H_ */
