/*
 * mapper_ColorDreams.h
 *
 *  Created on: 11/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPER_COLORDREAMS_H_
#define MAPPER_COLORDREAMS_H_

#include "common.h"

enum {
	CD_NO_CONFLCT,
	BAD_KING_NEPT,
};

void map_init_ColorDreams(void);
void extcl_cpu_wr_mem_ColorDreams(WORD address, BYTE value);

#endif /* MAPPER_COLORDREAMS_H_ */
