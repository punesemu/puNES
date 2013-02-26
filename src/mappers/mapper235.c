/*
 * mapper235.c
 *
 *  Created on: 11/feb/2011
 *      Author: fhorse
 */

#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

static const BYTE slots[4][4][2] = {
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x00, 1 }, { 0x00, 1 } },
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x20, 0 }, { 0x00, 1 } },
	{ { 0x00, 0 }, { 0x00, 1 }, { 0x20, 0 }, { 0x40, 0 } },
	{ { 0x00, 0 }, { 0x20, 0 }, { 0x40, 0 }, { 0x60, 0 } }
};

WORD prgRom32kMax, prgRom16kMax;
BYTE type;

void map_init_235(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom16kMax = info.prg_rom_16k_count - 1;

	switch (info.prg_rom_16k_count) {
		case 64:
			type = 0;
			break;
		case 128:
			type = 1;
			break;
		case 192:
			type = 2;
			break;
		case 256:
		default:
			type = 3;
			break;
	}

	EXTCL_CPU_WR_MEM(235);
	if (type != 3) {
		EXTCL_CPU_RD_MEM(235);
		EXTCL_SAVE_MAPPER(235);
		mapper.internal_struct[0] = (BYTE *) &m235;
		mapper.internal_struct_size[0] = sizeof(m235);

		info.mapper_extend_rd = TRUE;
	}

	if (info.reset >= HARD) {
        m235.openbus = 0;
        extcl_cpu_wr_mem_235(0x8000, 0x00);
	}
}
void extcl_cpu_wr_mem_235(WORD address, BYTE value) {
	BYTE bank = slots[type][(address >> 8) & 0x03][0] | (address & 0x1F);
	m235.openbus = slots[type][(address >> 8) & 0x03][1];

	if (address & 0x0800) {
		value = (bank << 1) | ((address >> 12) & 0x01);
    	control_bank(prgRom16kMax)
		map_prg_rom_8k(2, 0, value);
    	map_prg_rom_8k(2, 2, value);
	} else {
		value = bank;
		control_bank(prgRom32kMax)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();

	if (address & 0x0400) {
		mirroring_SCR0();
	} else if (address & 0x2000) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
BYTE extcl_cpu_rd_mem_235(WORD address, BYTE openbus, BYTE before) {
	if (!m235.openbus || (address < 0x8000)) {
		return (openbus);
	}

	return (address >> 8);
}
BYTE extcl_save_mapper_235(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m235.openbus);

	return (EXIT_OK);
}
