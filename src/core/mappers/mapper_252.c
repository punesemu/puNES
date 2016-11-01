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
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

#define m252_chr_extra_1k(a)\
	chr.bank_1k[a] = &chr.extra.data[(m252.chr_map[a] & 0x01) << 10];

static void INLINE m252_update_chr_extra(void);

void map_init_252(void) {
	EXTCL_CPU_WR_MEM(252);
	EXTCL_SAVE_MAPPER(252);
	EXTCL_WR_CHR(252);
	EXTCL_CPU_EVERY_CYCLE(252);
	mapper.internal_struct[0] = (BYTE *) &m252;
	mapper.internal_struct_size[0] = sizeof(m252);

	if (info.reset >= HARD) {
        BYTE i;

		memset(&m252, 0x00, sizeof(m252));

		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			m252.chr_map[i] = i;
		}
	}

	map_chr_ram_extra_init(0x800);
	m252_update_chr_extra();
}
void extcl_cpu_wr_mem_252(WORD address, BYTE value) {
	switch (address & 0xF00C) {
		case 0x8000:
		case 0x8004:
		case 0x8008:
		case 0x800C:
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0xA000:
		case 0xA004:
		case 0xA008:
		case 0xA00C:
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0xB000:
		case 0xB004:
		case 0xB008:
		case 0xB00C:
		case 0xC000:
		case 0xC004:
		case 0xC008:
		case 0xC00C:
		case 0xD000:
		case 0xD004:
		case 0xD008:
		case 0xD00C:
		case 0xE000:
		case 0xE004:
		case 0xE008:
		case 0xE00C: {
			BYTE i, shift;

			i = ((((address & 0x08) | (address >> 8)) >> 3) + 2) & 0x07;
			shift = address & 0x04;

			value = (m252.chr_map[i] & (0xF0 >> shift)) | ((value & 0x0F) << shift);
			control_bank(info.chr.rom[0].max.banks_1k)
			m252.chr_map[i] = value;

			if ((value == 6) || (value == 7)) {
				m252_chr_extra_1k(i);
			} else {
				chr.bank_1k[i] = chr_chip_byte_pnt(0, m252.chr_map[i] << 10);
			}
			return;
		}
		case 0xF000:
			m252.irq.reload = (m252.irq.reload & 0xF0) | (value & 0x0F);
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF004:
			m252.irq.reload = (value << 4) | (m252.irq.reload & 0x0F);
			irq.high &= ~EXT_IRQ;
			return;
		case 0xF008:
			m252.irq.prescaler = 0;
			m252.irq.active = value & 0x02;
			m252.irq.count = m252.irq.reload;
			irq.high &= ~EXT_IRQ;
			return;
	}
}
BYTE extcl_save_mapper_252(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m252.chr_map);
	save_slot_ele(mode, slot, m252.irq.active);
	save_slot_ele(mode, slot, m252.irq.prescaler);
	save_slot_ele(mode, slot, m252.irq.count);
	save_slot_ele(mode, slot, m252.irq.reload);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE);

	if (mode == SAVE_SLOT_READ) {
		m252_update_chr_extra();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_252(WORD address, BYTE value) {
	BYTE i = address >> 10;

	if ((m252.chr_map[i] == 6) || (m252.chr_map[i] == 7)) {
		chr.bank_1k[i][address & 0x3FF] = value;
	}
}
void extcl_cpu_every_cycle_252(void) {
	if (m252.irq.active) {
		m252.irq.prescaler += 3;
		if (m252.irq.prescaler >= 341) {
			m252.irq.prescaler -= 341;
			m252.irq.count++;
			if (m252.irq.count & 0x100) {
				irq.high |= EXT_IRQ;
				m252.irq.count = m252.irq.reload;
			}
		}
	}
}

static void INLINE m252_update_chr_extra(void) {
	BYTE i;

	for (i = 0; i < 8 ; i++) {
		if ((m252.chr_map[i] == 6) || (m252.chr_map[i] == 7)) {
			m252_chr_extra_1k(i);
		}
	}
}
