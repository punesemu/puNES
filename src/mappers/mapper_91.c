/*
 * mapper_91.c
 *
 *  Created on: 03/gen/2014
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"
#include "cpu.h"
#include "ppu.h"

WORD prg_rom_8k_max, chr_rom_2k_max;

void map_init_91(void) {
	prg_rom_8k_max = info.prg_rom_8k_count - 1;
	chr_rom_2k_max = (info.chr_rom_1k_count >> 1) - 1;

	EXTCL_CPU_WR_MEM(91);
	EXTCL_SAVE_MAPPER(91);
	EXTCL_PPU_256_TO_319(91);
	mapper.internal_struct[0] = (BYTE *) &m91;
	mapper.internal_struct_size[0] = sizeof(m91);

	memset(&m91, 0x00, sizeof(m91));

	info.mapper_extend_wr = TRUE;
}
void extcl_cpu_wr_mem_91(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}
	if (address <= 0x6FFF) {
		DBWORD bank;

		control_bank(chr_rom_2k_max)
		bank = value << 11;

		switch (address & 0x0003) {
			case 0:
				chr.bank_1k[0] = &chr.data[bank];
				chr.bank_1k[1] = &chr.data[bank | 0x0400];
				return;
			case 1:
				chr.bank_1k[2] = &chr.data[bank];
				chr.bank_1k[3] = &chr.data[bank | 0x0400];
				return;
			case 2:
				chr.bank_1k[4] = &chr.data[bank];
				chr.bank_1k[5] = &chr.data[bank | 0x0400];
				return;
			case 3:
				chr.bank_1k[6] = &chr.data[bank];
				chr.bank_1k[7] = &chr.data[bank | 0x0400];
				return;
		}
	}
	if (address < 0x7FFF) {
		switch (address & 0x0003) {
			case 0:
				control_bank(prg_rom_8k_max)
				map_prg_rom_8k(1, 0, value);
				map_prg_rom_8k_update();
				return;
			case 1:
				control_bank(prg_rom_8k_max)
				map_prg_rom_8k(1, 1, value);
				map_prg_rom_8k_update();
				return;
			case 2:
				m91.irq.active = 0;
				m91.irq.count = 0;
				irq.high &= ~EXT_IRQ;
				return;
			case 3:
				m91.irq.active = 1;
				irq.high &= ~EXT_IRQ;
				return;
		}
	}
}
BYTE extcl_save_mapper_91(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m91.irq.active);
	save_slot_ele(mode, slot, m91.irq.count);

	return (EXIT_OK);
}
void extcl_ppu_256_to_319_91(void) {
	if (ppu.frame_x != 319) {
		return;
	}

	if (m91.irq.active && (m91.irq.count < 8)) {
		m91.irq.count++;
		if (m91.irq.count >= 8) {
			irq.high |= EXT_IRQ;
		}
	}
}
