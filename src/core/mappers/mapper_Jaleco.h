/*
 * mapper_Jaleco.h
 *
 *  Created on: 16/lug/2011
 *      Author: fhorse
 */

#ifndef MAPPER_JALECO_H_
#define MAPPER_JALECO_H_

#include "common.h"

enum {
	JF05,
	JF11,
	JF13,
	JF16,
	JF17,
	JF19,
	SS8806,
	HOLYDIVER
};
enum {
	JAJAMARU,
	MEZASETOPPRO,
};

struct _ss8806 {
	BYTE chr_rom_bank[8];
	BYTE enabled;
	WORD mask;
	WORD reload;
	WORD count;
	BYTE delay;
} ss8806;

void map_init_Jaleco(BYTE model);

void extcl_cpu_wr_mem_Jaleco_JF05(WORD address, BYTE value);

void extcl_cpu_wr_mem_Jaleco_JF11(WORD address, BYTE value);

void extcl_cpu_wr_mem_Jaleco_JF13(WORD address, BYTE value);

void extcl_cpu_wr_mem_Jaleco_JF16(WORD address, BYTE value);

void extcl_cpu_wr_mem_Jaleco_JF17(WORD address, BYTE value);

void extcl_cpu_wr_mem_Jaleco_JF19(WORD address, BYTE value);

void extcl_cpu_wr_mem_Jaleco_SS8806(WORD address, BYTE value);
BYTE extcl_save_mapper_Jaleco_SS8806(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Jaleco_SS8806(void);

#endif /* MAPPER_JALECO_H_ */
