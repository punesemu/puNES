/*
 * mapper_Whirlwind.c
 *
 *  Created on: 25/set/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

void map_init_Whirlwind(void) {
	EXTCL_CPU_WR_MEM(Whirlwind);
	EXTCL_CPU_RD_MEM(Whirlwind);
	EXTCL_SAVE_MAPPER(Whirlwind);
	mapper.internal_struct[0] = (BYTE *) &whirlwind;
	mapper.internal_struct_size[0] = sizeof(whirlwind);

	info.prg.ram.banks_8k_plus = FALSE;

	if (info.reset >= HARD) {
		memset(&whirlwind, 0x00, sizeof(whirlwind));

		map_prg_rom_8k(4, 0, info.prg.rom.max.banks_32k);
	}
}
void extcl_cpu_wr_mem_Whirlwind(WORD address, BYTE value) {
	if (address == 0x8FFF) {
		control_bank(info.prg.rom.max.banks_8k)
		whirlwind.prg_ram = value << 13;
	}
}
BYTE extcl_cpu_rd_mem_Whirlwind(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg_chip_byte(0, whirlwind.prg_ram + (address - 0x6000)));
}
BYTE extcl_save_mapper_Whirlwind(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, whirlwind.prg_ram);

	return (EXIT_OK);
}
