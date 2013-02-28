/*
 * mapper120.c
 *
 *  Created on: 29/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

WORD prg_rom_8k_max;

void map_init_120(void) {
	prg_rom_8k_max = info.prg_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(120);
	EXTCL_CPU_RD_MEM(120);
	EXTCL_SAVE_MAPPER(120);
	mapper.internal_struct[0] = (BYTE *) &m120;
	mapper.internal_struct_size[0] = sizeof(m120);

	info.mapper_extend_wr = TRUE;
	cpu.prg_ram_wr_active = FALSE;
	cpu.prg_ram_rd_active = FALSE;

	if (info.reset >= HARD) {
		memset(&m120, 0x00, sizeof(m120));
		m120.prg_ram_rd = &prg.rom[0];
		map_prg_rom_8k(4, 0, 2);
	}
}
void extcl_cpu_wr_mem_120(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if ((address & 0xE3C0) == 0x41C0) {
		control_bank_with_AND(0x07, prg_rom_8k_max)
		m120.prg_ram_rd = &prg.rom[value << 13];
	}
}
BYTE extcl_cpu_rd_mem_120(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m120.prg_ram_rd[address & 0x1FFF]);
}
BYTE extcl_save_mapper_120(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_pos(mode, slot, prg.rom, m120.prg_ram_rd);

	return (EXIT_OK);
}
