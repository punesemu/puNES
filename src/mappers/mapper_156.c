/*
 * mapper_156.c
 *
 *  Created on: 4/ott/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "mem_map.h"

void map_init_156(void) {
	EXTCL_CPU_WR_MEM(156);

	mirroring_SCR0();
}
void extcl_cpu_wr_mem_156(WORD address, BYTE value) {
	switch (address) {
		case 0xC000:
		case 0xC001:
		case 0xC002:
		case 0xC003:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[address & 0x0007] = &chr.data[value << 10];
			return;
		case 0xC008:
		case 0xC009:
		case 0xC00A:
		case 0xC00B:
			control_bank(info.chr.rom.max.banks_1k)
			chr.bank_1k[(address & 0x000F) - 4] = &chr.data[value << 10];
			return;
		case 0xC010:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
	}
}
