/*
 * mapper_VRC4.h
 *
 *  Created on: 10/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_VRC4_H_
#define MAPPER_VRC4_H_

#include "common.h"

enum { VRC4A, VRC4B, VRC4C, VRC4D, VRC4E, VRC4BMC };

struct _vrc4 {
	BYTE chr_rom_bank[8];
	BYTE swap_mode;
	BYTE irq_enabled;
	BYTE irq_reload;
	BYTE irq_mode;
	BYTE irq_acknowledge;
	BYTE irq_count;
	WORD irq_prescaler;
} vrc4;

void map_init_VRC4(BYTE revision);
void extcl_cpu_wr_mem_VRC4(WORD address, BYTE value);
BYTE extcl_save_mapper_VRC4(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_VRC4(void);

void map_init_VRC4BMC(void);
void extcl_cpu_wr_mem_VRC4BMC(WORD address, BYTE value);

#endif /* MAPPER_VRC4_H_ */
