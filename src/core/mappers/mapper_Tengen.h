/*
 * mapper_Tengen.h
 *
 *  Created on: 17/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_TENGEN_H_
#define MAPPER_TENGEN_H_

#include "common.h"

enum { TRAMBO, T800037, NOCNTPLUS };

struct _tengen_rambo {
	BYTE prg_mode;
	BYTE chr_mode;
	BYTE reg_index;
	BYTE chr[8];
	BYTE prg[4];
	BYTE irq_mode;
	BYTE irq_delay;
	BYTE irq_prescaler;
	BYTE irq_force_clock;
} tengen_rambo;

void map_init_Tengen(BYTE model);

void extcl_cpu_wr_mem_Tengen_Rambo(WORD address, BYTE value);
BYTE extcl_save_mapper_Tengen_Rambo(BYTE mode, BYTE slot, FILE *fp);
void extcl_ppu_000_to_34x_Tengen_Rambo(void);
void extcl_update_r2006_Tengen_Rambo(WORD new_r2006, WORD old_r2006);
void extcl_irq_A12_clock_Tengen_Rambo(void);
void extcl_cpu_every_cycle_Tengen_Rambo(void);

#endif /* MAPPER_TENGEN_H_ */
