/*
 * mapper57.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

WORD prgRom32kMax, prgRom16kMax, chrRom8kMax;

void map_init_57(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom16kMax = info.prg_rom_16k_count - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(57);
	EXTCL_SAVE_MAPPER(57);
	mapper.internal_struct[0] = (BYTE *) &m57;
	mapper.internal_struct_size[0] = sizeof(m57);

	if (info.reset >= HARD) {
		memset(&m57, 0x00, sizeof(m57));

		extcl_cpu_wr_mem_57(0x8800, 0x00);
	}
}
void extcl_cpu_wr_mem_57(WORD address, BYTE value) {
	DBWORD bank;

	if (address & 0x0800) {
		m57.reg[0] = value;

		if (m57.reg[0] & 0x08) {
			mirroring_H();
		} else  {
			mirroring_V();
		}

		if (m57.reg[0] & 0x10) {
			value = (m57.reg[0] & 0xC0) >> 6;
			control_bank(prgRom32kMax)
			map_prg_rom_8k(4, 0, value);
		} else {
			value = (m57.reg[0] & 0xE0) >> 5;
			control_bank(prgRom16kMax)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}

		map_prg_rom_8k_update();
	} else {
		m57.reg[1] = value;
	}

	value = (m57.reg[1] & 0x07) | (m57.reg[0] & 0x07) | ((m57.reg[1] & 0x40) >> 3);
	control_bank(chrRom8kMax)
	bank = value << 13;
	chr.bank_1k[0] = &chr.data[bank];
	chr.bank_1k[1] = &chr.data[bank | 0x0400];
	chr.bank_1k[2] = &chr.data[bank | 0x0800];
	chr.bank_1k[3] = &chr.data[bank | 0x0C00];
	chr.bank_1k[4] = &chr.data[bank | 0x1000];
	chr.bank_1k[5] = &chr.data[bank | 0x1400];
	chr.bank_1k[6] = &chr.data[bank | 0x1800];
	chr.bank_1k[7] = &chr.data[bank | 0x1C00];
}
BYTE extcl_save_mapper_57(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m57.reg);

	return (EXIT_OK);
}
