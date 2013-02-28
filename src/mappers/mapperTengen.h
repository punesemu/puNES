/*
 * mapperTengen.h
 *
 *  Created on: 17/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERTENGEN_H_
#define MAPPERTENGEN_H_

#include "common.h"

enum { TRAMBO = 3, T800037, NOCNTPLUS };

struct _tengen_rambo {
	BYTE prg_mode;
	BYTE chr_mode;
	BYTE reg_index;
	BYTE chr[8];
	BYTE prg[4];
	BYTE irq_mode;
	BYTE irq_delay;
	BYTE irq_prescaler;
} tengen_rambo;

void map_init_Tengen(BYTE model);

void extcl_cpu_wr_mem_Tengen_Rambo(WORD address, BYTE value);
BYTE extcl_save_mapper_Tengen_Rambo(BYTE mode, BYTE slot, FILE *fp);
void extcl_ppu_000_to_255_Tengen_Rambo(void);
void extcl_ppu_256_to_319_Tengen_Rambo(void);
void extcl_ppu_320_to_34x_Tengen_Rambo(void);
void extcl_irq_A12_clock_Tengen_Rambo(void);
void extcl_cpu_every_cycle_Tengen_Rambo(void);

#endif /* MAPPERTENGEN_H_ */
