/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

INLINE static void prg_setup_178(void);

struct _m178 {
	BYTE reg[3];
	BYTE prg_mode;
} m178;
struct _m178tmp {
	BYTE model;
} m178tmp;

void map_init_178(BYTE type) {
	EXTCL_CPU_WR_MEM(178);
	EXTCL_SAVE_MAPPER(178);
	mapper.internal_struct[0] = (BYTE *)&m178;
	mapper.internal_struct_size[0] = sizeof(m178);

	if (info.reset >= HARD) {
		memset(&m178, 0x00, sizeof(m178));
		map_prg_rom_8k(4, 0, 0);
	}

	info.prg.ram.banks_8k_plus = 4;

	info.mapper.extend_wr = TRUE;

	m178tmp.model = type;
}
void extcl_cpu_wr_mem_178(WORD address, BYTE value) {
	switch (address) {
		case 0x4800:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			m178.prg_mode = (value & 0x06) >> 1;
			prg_setup_178();
			return;
		case 0x4801:
			if (m178tmp.model == M178EC32IN1) {
				m178.reg[1] = value;
			} else {
				m178.reg[0] = value;
			}
			prg_setup_178();
			return;
		case 0x4802:
			if (m178tmp.model == M178EC32IN1) {
				m178.reg[0] = value;
			} else {
				m178.reg[1] = value;
			}
			prg_setup_178();
			return;
		case 0x4803:
			m178.reg[2] = value & 0x03;
			prg.ram_plus_8k = &prg.ram_plus[m178.reg[2] << 13];
			return;
	}
}
BYTE extcl_save_mapper_178(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m178.reg);
	save_slot_ele(mode, slot, m178.prg_mode);

	return (EXIT_OK);
}

INLINE static void prg_setup_178(void) {
	DBWORD value;

	if (m178tmp.model == M178EC32IN1) {
		value = (m178.reg[1] << 3) | ((m178.reg[0] & 0x07) << 1);
	} else {
		value = (m178.reg[1] << 3) | (m178.reg[0] & 0x07);
	}

	switch (m178.prg_mode) {
		case 0:
			value >>= 1;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			break;
		case 1:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			if (m178tmp.model == M178EC32IN1) {
				value = (m178.reg[1] << 3) | 0x07;
			} else {
				value = (m178.reg[1] << 3) | 0x07;
			}
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
		case 2:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
			break;
		case 3:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			if (m178tmp.model == M178EC32IN1) {
				value = (m178.reg[1] << 3) | 0x06 | ((m178.reg[0] & 0x01) << 1);
			} else {
				value = (m178.reg[1] << 3) | 0x06 | (m178.reg[0] & 0x01);
			}
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
	}
	map_prg_rom_8k_update();
}
