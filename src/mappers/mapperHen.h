/*
 * mapperHen.h
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#ifndef MAPPERHEN_H_
#define MAPPERHEN_H_

#include "common.h"

enum {
	HEN_177 = 2,
	HEN_XJZB = 3,
	HEN_FANKONG = 4
};

void map_init_Hen(BYTE model);

void extcl_cpu_wr_mem_Hen_177(WORD address, BYTE value);

void extcl_cpu_wr_mem_Hen_xjzb(WORD address, BYTE value);

#endif /* MAPPERHEN_H_ */
