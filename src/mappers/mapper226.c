/*
 * mapper226.c
 *
 *  Created on: 08/feb/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

WORD prgRom32kMax, prgRom16kMax;

void map_init_226(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom16kMax = info.prg_rom_16k_count - 1;

	EXTCL_CPU_WR_MEM(226);
	EXTCL_SAVE_MAPPER(226);
	mapper.internal_struct[0] = (BYTE *) &m226;
	mapper.internal_struct_size[0] = sizeof(m226);

	if (info.reset >= HARD) {
		map_prg_rom_8k(4, 0, 0);
		memset(&m226, 0x00, sizeof(m226));
	}
}
void extcl_cpu_wr_mem_226(WORD address, BYTE value) {
	BYTE bank;

	m226.reg[address & 0x0001] = value;

	bank = ((m226.reg[0] >> 1) & 0x0F) | ((m226.reg[0] >> 3) & 0x10) |
			((m226.reg[1] << 5) & 0x20);

	if (m226.reg[0] & 0x20) {
		value = (bank << 1) | (m226.reg[0] & 0x01);
		control_bank(prgRom16kMax)
		map_prg_rom_8k(2, 0, value);
		map_prg_rom_8k(2, 2, value);
	} else {
		value = bank;
		control_bank(prgRom32kMax)
		map_prg_rom_8k(4, 0, value);
    }
	map_prg_rom_8k_update();

	if (m226.reg[0] & 0x40) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
BYTE extcl_save_mapper_226(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m226.reg);

	return (EXIT_OK);
}
