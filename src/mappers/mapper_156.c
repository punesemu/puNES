/*
 * mapper_156.c
 *
 *  Created on: 4/ott/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

WORD prg_rom_16k_max, chr_rom_1k_max;

void map_init_156(void) {
	prg_rom_16k_max = info.prg.rom.banks_16k - 1;
	chr_rom_1k_max = info.chr.rom.banks_1k - 1;

	EXTCL_CPU_WR_MEM(156);

	mirroring_SCR0();
}
void extcl_cpu_wr_mem_156(WORD address, BYTE value) {
	switch (address) {
		case 0xC000:
		case 0xC001:
		case 0xC002:
		case 0xC003:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[address & 0x0007] = &chr.data[value << 10];
			return;
		case 0xC008:
		case 0xC009:
		case 0xC00A:
		case 0xC00B:
			control_bank(chr_rom_1k_max)
			chr.bank_1k[(address & 0x000F) - 4] = &chr.data[value << 10];
			return;
		case 0xC010:
			control_bank(prg_rom_16k_max)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
