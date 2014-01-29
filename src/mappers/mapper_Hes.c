/*
 * mapper_Hes.c
 *
 *  Created on: 26/set/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_Hes(void) {
	EXTCL_CPU_WR_MEM(Hes);

	info.mapper.extend_wr = TRUE;

	if (info.reset >= HARD) {
		if (info.prg.rom.max.banks_32k != 0xFFFF) {
			map_prg_rom_8k(4, 0, 0);
		}
	}
}
void extcl_cpu_wr_mem_Hes(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if ((address & 0x0100) == 0x0100) {
		const BYTE save = value;
		DBWORD bank;

		if (value & 0x80) {
			mirroring_V();
		} else {
			mirroring_H();
		}

		if (info.prg.rom.max.banks_32k != 0xFFFF) {
			value = (value >> 3) & 0x07;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
		}

		value = ((save >> 3) & 0x08) | (save & 0x07);
		control_bank(info.chr.rom.max.banks_8k)
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
}
