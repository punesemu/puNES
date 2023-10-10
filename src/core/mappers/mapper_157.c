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
#include "save_slot.h"
#include "gui.h"
#include "detach_barcode.h"

void prg_swap_lz93d50_157(WORD address, WORD value);
void chr_fix_lz93d50_157(void);

struct _m157 {
	BYTE e0_data[256];
	BYTE e1_latch;
} m157;
struct _m157tmp {
	heeprom_i2c *e0;
	heeprom_i2c *e1;
} m157tmp;

void map_init_157(void) {
	EXTCL_AFTER_MAPPER_INIT(157);
	EXTCL_MAPPER_QUIT(157);
	EXTCL_CPU_WR_MEM(157);
	EXTCL_CPU_RD_MEM(157);
	EXTCL_SAVE_MAPPER(157);
	EXTCL_CPU_EVERY_CYCLE(157);
	EXTCL_BATTERY_IO(157);
	mapper.internal_struct[0] = (BYTE *)&lz93d50;
	mapper.internal_struct_size[0] = sizeof(lz93d50);

	if (info.reset >= HARD) {
		memset(&m157, 0x00, sizeof(m157));
	}

	init_detach_barcode(info.reset);

	init_LZ93D50(FALSE, info.reset);
	LZ93D50_prg_swap = prg_swap_lz93d50_157;
	LZ93D50_chr_fix = chr_fix_lz93d50_157;

	info.mapper.force_battery_io = TRUE;
}
void extcl_after_mapper_init_157(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m157tmp.e0 = eeprom_24c02_create(0, &m157.e0_data[0]);
		m157tmp.e1 = (info.format != NES_2_0) || wram_nvram_size()
			? eeprom_24c01_create(0, wram_pnt())
			: NULL;
	}
	extcl_after_mapper_init_LZ93D50();
}
void extcl_mapper_quit_157(void) {
	if (m157tmp.e0) {
		eeprom_i2c_free(m157tmp.e0);
		m157tmp.e0 = NULL;
	}
	if (m157tmp.e1) {
		eeprom_i2c_free(m157tmp.e1);
		m157tmp.e1 = NULL;
	}
}
void extcl_cpu_wr_mem_157(BYTE nidx, WORD address, BYTE value) {
	if (address >= 0x8000) {
		switch (address & 0x0F) {
			case 0x00:
				m157.e1_latch = (m157.e1_latch & 0xDF) | ((value & 0x08) << 2);
				if (m157tmp.e1) {
					eeprom_i2c_set_pins(m157tmp.e1, FALSE, ((m157.e1_latch & 0x20) >> 5), ((m157.e1_latch & 0x40) >> 6));
				}
				return;
			case 0x0D:
				m157.e1_latch = (m157.e1_latch & 0xBF) | (value & 0x40);
				if (m157tmp.e0) {
					BYTE new_clock = value & 0x80 ? TRUE : (value & 0x20) >> 5;
					BYTE new_data =  value & 0x80 ? TRUE : (value & 0x40) >> 6;

					eeprom_i2c_set_pins(m157tmp.e0, FALSE, new_clock, new_data);
				}
				if (m157tmp.e1) {
					BYTE new_clock = value & 0x80 ? TRUE : (m157.e1_latch & 0x20) >> 5;
					BYTE new_data =  value & 0x80 ? TRUE : (m157.e1_latch & 0x40) >> 6;

					eeprom_i2c_set_pins(m157tmp.e1, FALSE, new_clock, new_data);
				}
				return;
			default:
				extcl_cpu_wr_mem_LZ93D50(nidx, address, value);
				return;
		}
	}
}
BYTE extcl_cpu_rd_mem_157(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x6000) && (address <= 0x7FFF)) {
		BYTE value = 0x10;

		if (m157tmp.e0) {
			value &= eeprom_i2c_get_data(m157tmp.e0) * 0x10;
		}
		if (m157tmp.e1) {
			value &= eeprom_i2c_get_data(m157tmp.e1) * 0x10;
		}
		value |= (detach_barcode.out & 0x08);
		value |= (openbus & 0xE7);
		return (value);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_157(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m157.e0_data);
	save_slot_ele(mode, slot, m157.e1_latch);
	if (detach_barcode_save_mapper(mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	if (eeprom_i2c_save_mapper(m157tmp.e0, mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	if (eeprom_i2c_save_mapper(m157tmp.e1, mode, slot, fp) == EXIT_ERROR) return (EXIT_ERROR);
	return (extcl_save_mapper_LZ93D50(mode, slot, fp));
}
void extcl_cpu_every_cycle_157(BYTE nidx) {
	extcl_cpu_every_cycle_LZ93D50(nidx);
	detach_barcode.count++;
	if (detach_barcode.count >= 1000) {
		detach_barcode.count -= 1000;
		detach_barcode.out = detach_barcode.data[detach_barcode.pos] == 0xFF
			? 0
			: (detach_barcode.data[detach_barcode.pos++] ^ 0x01) << 3;
	}
}
void extcl_battery_io_157(BYTE mode, FILE *fp) {
	if (m157tmp.e0) {
		if (mode == WR_BAT) {
			if (fwrite(&m157.e0_data, sizeof(m157.e0_data), 1, fp) < 1) {
				log_error(uL("mapper_446;error on write e0 I2C chip"));
			}
		} else {
			if (fread(&m157.e0_data, sizeof(m157.e0_data), 1, fp) < 1) {
				log_error(uL("mapper_446;error on read e0 I2C chip"));
			}
		}
	}
}

void prg_swap_lz93d50_157(WORD address, WORD value) {
	prg_swap_LZ93D50_base(address, (value & 0x0F));
}
void chr_fix_lz93d50_157(void) {
	memmap_auto_8k(0, MMPPU(0x0000), 0);
}
