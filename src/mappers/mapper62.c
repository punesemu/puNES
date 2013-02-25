/*
 * mapper62.c
 *
 *  Created on: 22/apr/2012
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"

WORD prgRom32kMax, prgRom16kMax, chrRom8kMax;

void map_init_62(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom16kMax = info.prg_rom_16k_count - 1;
	chrRom8kMax = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(62);

	if (info.reset >= HARD) {
		extcl_cpu_wr_mem_62(0x8000, 0x00);
	}
}
void extcl_cpu_wr_mem_62(WORD address, BYTE value) {
	DBWORD bank;

	if (address & 0x0080) {
		mirroring_H();
	} else  {
		mirroring_V();
	}

	/*
	 * workaround per far funzionare correttamente "Fancy Mario"
	 * della rom "Super 700-in-1 [p1][!].nes" che non utilizza ne il mirroring
	 * verticale ne quello orizzontale.
	 */
	if ((info.mapper_type == SUPER700IN1) && (address == 0x8790)) {
		mirroring_FSCR();
	}

	value = ((address & 0x1F) << 2) | (value & 0x03);
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

	if (address & 0x0020) {
		value = (address & 0x40) | ((address >> 8) & 0x3F);
		control_bank(prgRom16kMax)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = ((address & 0x40) | ((address >> 8) & 0x3F)) >> 1;
		control_bank(prgRom32kMax)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();
}
