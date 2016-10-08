/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "info.h"
#include "ppu.h"
#include "cpu.h"
#include "irqA12.h"
#include "save_slot.h"

#define chr_rom_1k_update(slot, mask, shift)\
	value = (tf1201.chr_rom_bank[slot] & mask) | ((value & 0x0F) << shift);\
	control_bank(info.chr.rom[0].max.banks_1k)\
	chr.bank_1k[slot] = chr_chip_byte_pnt(0, value << 10);\
	tf1201.chr_rom_bank[slot] = value

void map_init_TF1201(void) {
	EXTCL_CPU_WR_MEM(TF1201);
	EXTCL_SAVE_MAPPER(TF1201);
	EXTCL_CPU_EVERY_CYCLE(TF1201);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	//EXTCL_UPDATE_R2006(MMC3);
	EXTCL_IRQ_A12_CLOCK(TF1201);
	mapper.internal_struct[0] = (BYTE *) &tf1201;
	mapper.internal_struct_size[0] = sizeof(tf1201);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&tf1201, 0x00, sizeof(tf1201));

		for (i = 0; i < 8; i++) {
			tf1201.chr_rom_bank[i] = i;
		}
	}

	memset(&irqA12, 0x00, sizeof(irqA12));
	irqA12.present = TRUE;
}
void extcl_cpu_wr_mem_TF1201(WORD address, BYTE value) {
	WORD tmp = (address & 0xF000);

	if ((tmp == 0x8000) || (tmp == 0xA000)) {
		address &= 0xF000;
	} else {
		address = (address & 0xF003) | ((address & 0x000C) >> 2);
	}

	switch (address) {
		case 0x8000:
			control_bank_with_AND(0x1F, info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, tf1201.swap_mode, value);
			map_prg_rom_8k(1, 0x02 >> tf1201.swap_mode, info.prg.rom[0].max.banks_8k_before_last);
			map_prg_rom_8k_update();
			return;
		case 0xA000:
			control_bank_with_AND(0x1F, info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x9000:
			switch (value & 0x01) {
				case 0:
					mirroring_V();
					break;
				case 1:
					mirroring_H();
					break;
			}
			return;
		case 0x9001:
			value = (value & 0x03) ? 0x02 : 0x00;
			if (tf1201.swap_mode != value) {
				WORD swap = mapper.rom_map_to[0];

				mapper.rom_map_to[0] = mapper.rom_map_to[2];
				mapper.rom_map_to[2] = swap;
				map_prg_rom_8k_update();
				tf1201.swap_mode = value;
			}
			return;
		case 0xB000:
			chr_rom_1k_update(0, 0xF0, 0);
			return;
		case 0xB001:
			chr_rom_1k_update(1, 0xF0, 0);
			return;
		case 0xB002:
			chr_rom_1k_update(0, 0x0F, 4);
			return;
		case 0xB003:
			chr_rom_1k_update(1, 0x0F, 4);
			return;
		case 0xC000:
			chr_rom_1k_update(2, 0xF0, 0);
			return;
		case 0xC001:
			chr_rom_1k_update(3, 0xF0, 0);
			return;
		case 0xC002:
			chr_rom_1k_update(2, 0x0F, 4);
			return;
		case 0xC003:
			chr_rom_1k_update(3, 0x0F, 4);
			return;
		case 0xD000:
			chr_rom_1k_update(4, 0xF0, 0);
			return;
		case 0xD001:
			chr_rom_1k_update(5, 0xF0, 0);
			return;
		case 0xD002:
			chr_rom_1k_update(4, 0x0F, 4);
			return;
		case 0xD003:
			chr_rom_1k_update(5, 0x0F, 4);
			return;
		case 0xE000:
			chr_rom_1k_update(6, 0xF0, 0);
			return;
		case 0xE001:
			chr_rom_1k_update(7, 0xF0, 0);
			return;
		case 0xE002:
			chr_rom_1k_update(6, 0x0F, 4);
			return;
		case 0xE003:
			chr_rom_1k_update(7, 0x0F, 4);
			return;
		case 0xF000:
			irqA12.counter = (irqA12.counter & 0xF0) | (value & 0x0F);
			return;
		case 0xF001:
		case 0xF003:
			irqA12.enable = value & 0x02;
			irq.high &= ~EXT_IRQ;

			if ((ppu.frame_y > ppu_sclines.vint) && (ppu.screen_y < SCR_LINES)) {
				irqA12.counter -= 8;
			}
			return;
		case 0xF002:
			irqA12.counter = (irqA12.counter & 0x0F) | ((value & 0x0F) << 4);
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_TF1201(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, tf1201.chr_rom_bank);
	save_slot_ele(mode, slot, tf1201.swap_mode);

	return (EXIT_OK);
}
void extcl_irq_A12_clock_TF1201(void) {
	if (!irqA12.enable) {
		return;
	}

	if (++irqA12.counter == 237) {
		irqA12.delay = 1;
	}
}
void extcl_cpu_every_cycle_TF1201(void) {
	if (irqA12.delay && !(--irqA12.delay)) {
		irq.high |= EXT_IRQ;
	}
}
