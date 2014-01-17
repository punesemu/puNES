/*
 * mapper_46.c
 *
 *  Created on: 20/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

WORD prg_rom_32k_max, chr_rom_8k_max;

void map_init_46(void) {
	prg_rom_32k_max = (info.prg.rom.banks_16k >> 1) - 1;
	chr_rom_8k_max = info.chr.rom.banks_8k - 1;

	EXTCL_CPU_WR_MEM(46);
	EXTCL_SAVE_MAPPER(46);
	mapper.internal_struct[0] = (BYTE *) &m46;
	mapper.internal_struct_size[0] = sizeof(m46);

	if (info.reset >= HARD) {
		memset(&m46, 0x00, sizeof(m46));

		map_prg_rom_8k(4, 0, 0);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_46(WORD address, BYTE value) {
	BYTE save = value;
	DBWORD bank;

	if (address >= 0x8000) {
		value = (m46.prg & 0x1E) | (save & 0x01);
		control_bank(prg_rom_32k_max)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
		m46.prg = value;

		value = (m46.chr & 0x78) | ((save >> 4) & 0x07);
		control_bank(chr_rom_8k_max)
		bank = value << 13;
		chr.bank_1k[0] = &chr.data[bank];
		chr.bank_1k[1] = &chr.data[bank | 0x0400];
		chr.bank_1k[2] = &chr.data[bank | 0x0800];
		chr.bank_1k[3] = &chr.data[bank | 0x0C00];
		chr.bank_1k[4] = &chr.data[bank | 0x1000];
		chr.bank_1k[5] = &chr.data[bank | 0x1400];
		chr.bank_1k[6] = &chr.data[bank | 0x1800];
		chr.bank_1k[7] = &chr.data[bank | 0x1C00];
		m46.chr = value;

		return;
	}

	if (address >= 0x6000) {
		value = (m46.prg & 0x01) | ((save << 1) & 0x1E);
		control_bank(prg_rom_32k_max)
		map_prg_rom_8k(4, 0, value);
		map_prg_rom_8k_update();
		m46.prg = value;

		value = (m46.chr & 0x07) | ((save >> 1) & 0x78);
		control_bank(chr_rom_8k_max)
		bank = value << 13;
		chr.bank_1k[0] = &chr.data[bank];
		chr.bank_1k[1] = &chr.data[bank | 0x0400];
		chr.bank_1k[2] = &chr.data[bank | 0x0800];
		chr.bank_1k[3] = &chr.data[bank | 0x0C00];
		chr.bank_1k[4] = &chr.data[bank | 0x1000];
		chr.bank_1k[5] = &chr.data[bank | 0x1400];
		chr.bank_1k[6] = &chr.data[bank | 0x1800];
		chr.bank_1k[7] = &chr.data[bank | 0x1C00];
		m46.chr = value;

		return;
	}
}
BYTE extcl_save_mapper_46(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m46.prg);
	save_slot_ele(mode, slot, m46.chr);
	return (EXIT_OK);
}
