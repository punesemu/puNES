/*
 * mapper50.c
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu.h"
#include "save_slot.h"

WORD prgRom8kMax, chrRom1kMax;
BYTE *prg6000;

void map_init_50(void) {
	prgRom8kMax = info.prg_rom_8k_count - 1;
	chrRom1kMax = info.chr_rom_1k_count - 1;

	EXTCL_CPU_WR_MEM(50);
	EXTCL_CPU_RD_MEM(50);
	EXTCL_SAVE_MAPPER(50);
	EXTCL_CPU_EVERY_CYCLE(50);
	mapper.internal_struct[0] = (BYTE *) &m50;
	mapper.internal_struct_size[0] = sizeof(m50);

	if (info.reset >= HARD) {
		memset(&m50, 0x00, sizeof(m50));

		mapper.rom_map_to[2] = 0;
	}

	prg6000 = &prg.rom[prgRom8kMax << 13];

	mapper.rom_map_to[0] = 8;
	mapper.rom_map_to[1] = 9;
	mapper.rom_map_to[3] = 11;

	info.mapper_extend_wr = TRUE;
}
void extcl_cpu_wr_mem_50(WORD address, BYTE value) {
	if ((address <= 0x5FFF) && ((address & 0x0060) == 0x0020)) {
		if (address & 0x0100) {
			if (!(m50.enabled = value & 0x01)) {
				m50.count = 0;
				irq.high &= ~EXTIRQ;
			}
			return;
		}

		value = (value & 0x08) | ((value << 2) & 0x04) | ((value >> 1) & 0x03);
		control_bank(prgRom8kMax)
		map_prg_rom_8k(1, 2, value);
		map_prg_rom_8k_update();
		return;
	}
}
BYTE extcl_cpu_rd_mem_50(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_50(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m50.enabled);
	save_slot_ele(mode, slot, m50.count);
	save_slot_ele(mode, slot, m50.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_50(void) {
	if (m50.delay && !(--m50.delay)) {
		irq.high |= EXTIRQ;
	}

	if (m50.enabled && (++m50.count == 0x1000)) {
		m50.delay = 1;
	}
}
