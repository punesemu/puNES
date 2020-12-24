/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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
#include "info.h"
#include "mem_map.h"
#include "save_slot.h"

INLINE static void m53_update_6000(void);

struct _m53 {
	BYTE reg[2];
	BYTE prg_6000;
} m53;
struct _m53tmp {
	BYTE *prg_6000;
} m53tmp;

void map_init_53(void) {
	EXTCL_CPU_WR_MEM(53);
	EXTCL_CPU_RD_MEM(53);
	EXTCL_SAVE_MAPPER(53);
	mapper.internal_struct[0] = (BYTE *) &m53;
	mapper.internal_struct_size[0] = sizeof(m53);

	if (info.reset >= HARD) {
		memset(&m53, 0x00, sizeof(m53));

		extcl_cpu_wr_mem_53(0x6000, 0x00);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_53(WORD address, BYTE value) {
	BYTE chip, tmp;

	if (address < 0x6000) {
		return;
	}

	if (address >= 0x8000) {
		m53.reg[1] = value;
	} else {
		m53.reg[0] = value;

		if (m53.reg[0] & 0x20) {
			mirroring_H();
		} else  {
			mirroring_V();
		}
	}

	m53_update_6000();

	if (m53.reg[0] & 0x10) {
		if (info.prg.chips > 1) {
			chip = (m53.reg[0] >> 2) & 0x03;
			_control_bank(chip, info.prg.max_chips)

			tmp = (m53.reg[0] << 3) & 0x18;

			value = tmp | (m53.reg[1] & 0x07);
			control_bank(info.prg.rom[chip].max.banks_16k)
			map_prg_rom_8k_chip(2, 0, value, chip);

			value = tmp | 0x07;
			control_bank(info.prg.rom[chip].max.banks_16k)
			map_prg_rom_8k_chip(2, 2, value, chip);
		} else {
			tmp = (m53.reg[0] << 3) & 0x78;

			value = (tmp | (m53.reg[1] & 0x07)) + 2;
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);

			value = (tmp | 0x07) + 2;
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 2, value);
		}
	} else if (info.prg.chips == 5) {
		map_prg_rom_8k_chip(4, 0, 0, 4);
	} else {
		map_prg_rom_8k(4, 0, 0);
	}
	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_53(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (m53tmp.prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_53(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m53.reg);
	save_slot_ele(mode, slot, m53.prg_6000);

	if (mode == SAVE_SLOT_READ) {
		m53_update_6000();
	}

	return (EXIT_OK);
}

INLINE static void m53_update_6000(void) {
	BYTE chip;

	if (info.prg.chips > 1) {
		chip = (m53.reg[0] >> 2) & 0x03;
		_control_bank(chip, info.prg.max_chips)

		m53.prg_6000 = ((m53.reg[0] << 4) & 0x30) | 0x0F;
		_control_bank(m53.prg_6000, info.prg.rom[chip].max.banks_8k)
		m53tmp.prg_6000 = prg_chip_byte_pnt(chip, m53.prg_6000 << 13);
	} else {
		m53.prg_6000 = (((m53.reg[0] << 4) & 0xF0) | 0x0F) + 4;
		_control_bank(m53.prg_6000, info.prg.rom[0].max.banks_8k)
		m53tmp.prg_6000 = prg_chip_byte_pnt(0, m53.prg_6000 << 13);
	}
}
