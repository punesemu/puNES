/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MAPPER_CAMERICA_H_
#define MAPPER_CAMERICA_H_

#include "common.h"

enum {
	BF9093,
	BF9096,
	BF9097,
	GOLDENFIVE,
	PEGASUS4IN1
};

void map_init_Camerica(void);
void extcl_cpu_wr_mem_Camerica_BF9093(WORD address, BYTE value);
void extcl_cpu_wr_mem_Camerica_BF9096(WORD address, BYTE value);
void extcl_cpu_wr_mem_Camerica_BF9097(WORD address, BYTE value);
void extcl_cpu_wr_mem_Camerica_GoldenFive(WORD address, BYTE value);

#endif /* MAPPER_CAMERICA_H_ */
