/*
 * mapperWhirlwind.c
 *
 *  Created on: 25/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "save_slot.h"

WORD prgRom32kMax, prgRom8kMax;

void map_init_Whirlwind(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom8kMax = info.prg_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(Whirlwind);
	EXTCL_CPU_RD_MEM(Whirlwind);
	EXTCL_SAVE_MAPPER(Whirlwind);
	mapper.internal_struct[0] = (BYTE *) &whirlwind;
	mapper.internal_struct_size[0] = sizeof(whirlwind);

	info.prg_ram_plus_8k_count = FALSE;

	if (info.reset >= HARD) {
		memset(&whirlwind, 0x00, sizeof(whirlwind));

		map_prg_rom_8k(4, 0, prgRom32kMax);
	}
}
void extcl_cpu_wr_mem_Whirlwind(WORD address, BYTE value) {
	if (address == 0x8FFF) {
		control_bank(prgRom8kMax)
		whirlwind.prgRam = value << 13;
	}
}
BYTE extcl_cpu_rd_mem_Whirlwind(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg.rom[whirlwind.prgRam + (address - 0x6000)]);
}
BYTE extcl_save_mapper_Whirlwind(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, whirlwind.prgRam);

	return (EXIT_OK);
}
