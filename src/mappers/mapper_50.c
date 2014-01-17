/*
 * mapper_50.c
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

WORD prg_rom_8k_max, chr_rom_1k_max;
BYTE *prg_6000;

void map_init_50(void) {
	prg_rom_8k_max = info.prg.rom.banks_8k - 1;
	chr_rom_1k_max = info.chr.rom.banks_1k - 1;

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

	prg_6000 = &prg.rom[prg_rom_8k_max << 13];

	mapper.rom_map_to[0] = 8;
	mapper.rom_map_to[1] = 9;
	mapper.rom_map_to[3] = 11;

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_50(WORD address, BYTE value) {
	if ((address <= 0x5FFF) && ((address & 0x0060) == 0x0020)) {
		if (address & 0x0100) {
			if (!(m50.enabled = value & 0x01)) {
				m50.count = 0;
				irq.high &= ~EXT_IRQ;
			}
			return;
		}

		value = (value & 0x08) | ((value << 2) & 0x04) | ((value >> 1) & 0x03);
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 2, value);
		map_prg_rom_8k_update();
		return;
	}
}
BYTE extcl_cpu_rd_mem_50(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_50(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m50.enabled);
	save_slot_ele(mode, slot, m50.count);
	save_slot_ele(mode, slot, m50.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_50(void) {
	if (m50.delay && !(--m50.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (m50.enabled && (++m50.count == 0x1000)) {
		m50.delay = 1;
	}
}
