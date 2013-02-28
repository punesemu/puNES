/*
 * mapperWhirlwind.c
 *
 *  Created on: 25/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

WORD prg_rom_32k_max, prg_rom_8k_max;

void map_init_Whirlwind(void) {
	prg_rom_32k_max = (info.prg_rom_16k_count >> 1) - 1;
	prg_rom_8k_max = info.prg_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(Whirlwind);
	EXTCL_CPU_RD_MEM(Whirlwind);
	EXTCL_SAVE_MAPPER(Whirlwind);
	mapper.internal_struct[0] = (BYTE *) &whirlwind;
	mapper.internal_struct_size[0] = sizeof(whirlwind);

	info.prg_ram_plus_8k_count = FALSE;

	if (info.reset >= HARD) {
		memset(&whirlwind, 0x00, sizeof(whirlwind));

		map_prg_rom_8k(4, 0, prg_rom_32k_max);
	}
}
void extcl_cpu_wr_mem_Whirlwind(WORD address, BYTE value) {
	if (address == 0x8FFF) {
		control_bank(prg_rom_8k_max)
		whirlwind.prg_ram = value << 13;
	}
}
BYTE extcl_cpu_rd_mem_Whirlwind(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg.rom[whirlwind.prg_ram + (address - 0x6000)]);
}
BYTE extcl_save_mapper_Whirlwind(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, whirlwind.prg_ram);

	return (EXIT_OK);
}
