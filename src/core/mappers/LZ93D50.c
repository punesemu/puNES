/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "cpu.h"
#include "save_slot.h"

void (*LZ93D50_prg_fix)(void);
void (*LZ93D50_prg_swap)(WORD address, WORD value);
void (*LZ93D50_chr_fix)(void);
void (*LZ93D50_chr_swap)(WORD address, WORD value);
void (*LZ93D50_wram_fix)(void);
void (*LZ93D50_mirroring_fix)(void);

_lz93d50 lz93d50;
struct lz93d50tmp {
	heeprom_i2c *eeprom;
	WORD start;
	WORD end;
} lz93d50tmp;

// promemoria
//void map_init_LZ93D50(void) {
//	EXTCL_AFTER_MAPPER_INIT(LZ93D50);
//	EXTCL_CPU_WR_MEM(LZ93D50);
//	EXTCL_CPU_RD_MEM(LZ93D50); // se e' presente la eeprom
//	EXTCL_SAVE_MAPPER(LZ93D50);
//	EXTCL_CPU_EVERY_CYCLE(LZ93D50);
//}

void init_LZ93D50(BYTE include_wram) {
	if (info.reset >= HARD) {
		memset(&lz93d50, 0x00, sizeof(lz93d50));

		lz93d50.chr[1] = 1;
		lz93d50.chr[2] = 2;
		lz93d50.chr[3] = 3;
		lz93d50.chr[4] = 4;
		lz93d50.chr[5] = 5;
		lz93d50.chr[6] = 6;
		lz93d50.chr[7] = 7;
	}

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		lz93d50tmp.eeprom = NULL;
	}

	if (include_wram) {
		lz93d50tmp.start = 0x6000;
		lz93d50tmp.end = 0xFFFF;
		info.mapper.extend_wr = TRUE;
	} else {
		lz93d50tmp.start = 0x8000;
		lz93d50tmp.end = 0xFFFF;
	}

	LZ93D50_prg_fix = prg_fix_LZ93D50_base;
	LZ93D50_prg_swap = prg_swap_LZ93D50_base;
	LZ93D50_chr_fix = chr_fix_LZ93D50_base;
	LZ93D50_chr_swap = chr_swap_LZ93D50_base;
	LZ93D50_wram_fix = wram_fix_LZ93D50_base;
	LZ93D50_mirroring_fix = mirroring_fix_LZ93D50_base;
}
void init_eeprom_LZ93D50(heeprom_i2c *eeprom) {
	lz93d50tmp.eeprom = eeprom;
}
void extcl_after_mapper_init_LZ93D50(void) {
	LZ93D50_prg_fix();
	LZ93D50_chr_fix();
	LZ93D50_mirroring_fix();
}
void extcl_cpu_wr_mem_LZ93D50(WORD address, BYTE value) {
	if ((address >= lz93d50tmp.start) && (address <= lz93d50tmp.end)) {
		switch (address & 0x0F) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
				lz93d50.chr[address & 0x07] = value;
				LZ93D50_chr_fix();
				return;
			case 0x08:
				lz93d50.prg = value;
				LZ93D50_prg_fix();
				return;
			case 0x09:
				lz93d50.mirroring = value;
				LZ93D50_mirroring_fix();
				return;
			case 0x0A:
				lz93d50.irq.enabled = value & 0x01;
				lz93d50.irq.count = lz93d50.irq.reload;
				if (lz93d50.irq.enabled && !lz93d50.irq.count) {
					lz93d50.irq.delay = 1;
				} else {
					irq.high &= ~EXT_IRQ;
				}
				return;
			case 0x0B:
				lz93d50.irq.reload = (lz93d50.irq.reload & 0xFF00) | value;
				return;
			case 0x0C:
				lz93d50.irq.reload = (lz93d50.irq.reload & 0x00FF) | (value << 8);
				return;
			case 0x0D:
				if (lz93d50tmp.eeprom) {
					eeprom_i2c_set_pins(lz93d50tmp.eeprom, FALSE, ((value & 0x20) >> 5), ((value & 0x40) >> 6));
				} else {
					lz93d50.wram_enabled = (value & 0x20) >> 5;
				}
				return;
			default:
				return;
		}
	}
}
BYTE extcl_cpu_rd_mem_LZ93D50(WORD address, BYTE openbus) {
	if ((address >= 0x6000) && (address <= 0x7FFF) && lz93d50tmp.eeprom) {
		return (eeprom_i2c_get_data(lz93d50tmp.eeprom) * 0x10) | (openbus & 0xEF);
	}
	return (openbus);
}
BYTE extcl_save_mapper_LZ93D50(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, lz93d50.prg);
	save_slot_ele(mode, slot, lz93d50.chr);
	save_slot_ele(mode, slot, lz93d50.mirroring);
	save_slot_ele(mode, slot, lz93d50.irq.enabled);
	save_slot_ele(mode, slot, lz93d50.irq.count);
	save_slot_ele(mode, slot, lz93d50.irq.delay);
	return (lz93d50tmp.eeprom
		? eeprom_i2c_save_mapper(lz93d50tmp.eeprom, mode, slot, fp)
		: EXIT_OK);
}
void extcl_cpu_every_cycle_LZ93D50(void) {
	if (lz93d50.irq.delay && !(--lz93d50.irq.delay)) {
		irq.high |= EXT_IRQ;
	}
	if (lz93d50.irq.enabled) {
		if (lz93d50.irq.count && !(--lz93d50.irq.count)) {
			lz93d50.irq.delay = 1;
		}
	}
}

void prg_fix_LZ93D50_base(void) {
	LZ93D50_prg_swap(0x8000, lz93d50.prg);
	LZ93D50_prg_swap(0xC000, 0xFF);
}
void prg_swap_LZ93D50_base(WORD address, WORD value) {
	memmap_auto_16k(MMCPU(address), value);
}
void chr_fix_LZ93D50_base(void) {
	LZ93D50_chr_swap(0x0000, lz93d50.chr[0]);
	LZ93D50_chr_swap(0x0400, lz93d50.chr[1]);
	LZ93D50_chr_swap(0x0800, lz93d50.chr[2]);
	LZ93D50_chr_swap(0x0C00, lz93d50.chr[3]);
	LZ93D50_chr_swap(0x1000, lz93d50.chr[4]);
	LZ93D50_chr_swap(0x1400, lz93d50.chr[5]);
	LZ93D50_chr_swap(0x1800, lz93d50.chr[6]);
	LZ93D50_chr_swap(0x1C00, lz93d50.chr[7]);
}
void chr_swap_LZ93D50_base(WORD address, WORD value) {
	memmap_auto_1k(MMPPU(address), value);
}
void wram_fix_LZ93D50_base(void) {
	BYTE enable = lz93d50.wram_enabled || miscrom_size();

	memmap_auto_wp_8k(MMCPU(0x6000), 0, enable, enable);
}
void mirroring_fix_LZ93D50_base(void) {
	switch (lz93d50.mirroring & 0x03) {
		case 0:
			mirroring_V();
			break;
		case 1:
			mirroring_H();
			break;
		case 2:
			mirroring_SCR0();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}
}
