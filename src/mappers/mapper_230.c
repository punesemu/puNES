/*
 * mapper_230.c
 *
 *  Created on: 05/feb/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

WORD prg_rom_16k_max;

void map_init_230(void) {
	prg_rom_16k_max = info.prg_rom_16k_count - 1;

	EXTCL_CPU_WR_MEM(230);
	EXTCL_SAVE_MAPPER(230);
	mapper.internal_struct[0] = (BYTE *) &m230;
	mapper.internal_struct_size[0] = sizeof(m230);

	if (info.reset >= HARD) {
		m230.mode = 0;
	} else {
		m230.mode ^= 1;
	}

	if (m230.mode) {
		map_prg_rom_8k(2, 0, 0);
		map_prg_rom_8k(2, 2, 7);

		mirroring_V();
	} else {
		map_prg_rom_8k(2, 0, 8);
		map_prg_rom_8k(2, 2, prg_rom_16k_max);
	}
}
void extcl_cpu_wr_mem_230(WORD address, BYTE value) {
	BYTE save = value;

	if (!m230.mode) {
		value = (save & 0x1F) + 0x08;
		control_bank(prg_rom_16k_max)
		map_prg_rom_8k(2, 0, value);

		value |= ((~save >> 5) & 0x01);
		control_bank(prg_rom_16k_max)
		map_prg_rom_8k(2, 2, value);

		if (save & 0x40) {
			mirroring_V();
		} else {
			mirroring_H();
		}
	} else {
		control_bank_with_AND(0x07, prg_rom_16k_max)
		map_prg_rom_8k(2, 0, value);
	}
	map_prg_rom_8k_update();
}
BYTE extcl_save_mapper_230(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m230.mode);

	return (EXIT_OK);
}
