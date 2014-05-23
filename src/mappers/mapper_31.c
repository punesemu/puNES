/*
 * mapper_31.c
 *
 *  Created on: 29/apr/2014
 *      Author: fhorse
 */

#include <stdio.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

static void INLINE sync_31(void);

void map_init_31(void) {
	EXTCL_CPU_WR_MEM(31);
	EXTCL_CPU_RD_MEM(31);
	EXTCL_SAVE_MAPPER(31);

	memset(&m31, 0x00, sizeof(m31));
	m31.regs[7] = 0xFF;
	sync_31();

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_cpu_wr_mem_31(WORD address, BYTE value) {
	if ((address < 0x5000) && (address > 0x5FFF)) {
		return;
	}
	m31.regs[address & 0x0007] = value;
	sync_31();
}
BYTE extcl_cpu_rd_mem_31(WORD address, BYTE openbus, BYTE before) {
	if (address < 0x8000) {
		return (openbus);
	}
	return (m31.rom_4k[(address >> 12) & 0x07][address & 0x0FFF]);
}
BYTE extcl_save_mapper_31(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m31.regs);

	if (mode == SAVE_SLOT_READ) {
		sync_31();
	}

	return (EXIT_OK);
}

static void INLINE sync_31(void) {
	BYTE i;

	for (i = 0; i < 8; ++i) {
		m31.rom_4k[i] = &prg.rom[m31.regs[i] << 12];
	}
}
