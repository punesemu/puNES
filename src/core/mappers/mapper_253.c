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
#include "cpu.h"
#include "save_slot.h"

INLINE static void m253_update_chr(void);

struct _m253 {
	BYTE disabled_vram;
	WORD chr_map_high[8];
	BYTE chr_map[8];
	struct _m253_irq {
		BYTE active;
		WORD prescaler;
		WORD count;
		WORD reload;
	} irq;
} m253;

void map_init_253(void) {
	EXTCL_CPU_WR_MEM(253);
	EXTCL_SAVE_MAPPER(253);
	EXTCL_WR_CHR(253);
	EXTCL_CPU_EVERY_CYCLE(253);
	mapper.internal_struct[0] = (BYTE *) &m253;
	mapper.internal_struct_size[0] = sizeof(m253);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&m253, 0x00, sizeof(m253));

		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			m253.chr_map[i] = i;
		}
	}

	map_chr_ram_extra_init(0x800);
	m253_update_chr();
}
void extcl_cpu_wr_mem_253(WORD address, BYTE value) {
	switch (address) {
		case 0x8010:
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0xA010:
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0x9400:
			switch (value & 0x03) {
				case 0:
					mirroring_V();
					return;
				case 1:
					mirroring_H();
					return;
				case 2:
					mirroring_SCR0();
					return;
				case 3:
					mirroring_SCR1();
					return;
			}
			return;
		case 0xF000:
			m253.irq.reload = (m253.irq.reload & 0xF0) | (value & 0x0F);
			return;
		case 0xF004:
			m253.irq.reload = (value << 4) | (m253.irq.reload & 0x0F);
			return;
		case 0xF008:
			m253.irq.active = value & 0x02;
			if (m253.irq.active) {
				m253.irq.prescaler = 0;
				m253.irq.count = m253.irq.reload;
			}
			irq.high &= ~EXT_IRQ;
			return;
		default:
			if ((address >= 0xB000) && (address <= 0xE00C)) {
				BYTE i, shift;

				i = ((((address & 0x08) | (address >> 8)) >> 3) + 2) & 0x07;
				shift = address & 0x04;

				m253.chr_map[i] = (m253.chr_map[i] & (0xF0 >> shift)) | ((value & 0x0F) << shift);

				if (shift) {
					m253.chr_map_high[i] = (value << 4) & 0x0F00;
				}

				if (!i) {
					if (m253.chr_map[i] == 0xC8) {
						m253.disabled_vram = FALSE;
					} else if (m253.chr_map[i] == 0x88) {
						m253.disabled_vram = TRUE;
					}
				}

				m253_update_chr();
			}
			return;
	}
}
BYTE extcl_save_mapper_253(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m253.disabled_vram);
	save_slot_ele(mode, slot, m253.chr_map_high);
	save_slot_ele(mode, slot, m253.chr_map);
	save_slot_ele(mode, slot, m253.irq.active);
	save_slot_ele(mode, slot, m253.irq.prescaler);
	save_slot_ele(mode, slot, m253.irq.count);
	save_slot_ele(mode, slot, m253.irq.reload);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE);

	if (mode == SAVE_SLOT_READ) {
		m253_update_chr();
	}

	return (EXIT_OK);
}
void extcl_wr_chr_253(WORD address, BYTE value) {
	BYTE i = address >> 10;

	if (((m253.chr_map[i] == 4) || (m253.chr_map[i] == 5)) && (m253.disabled_vram == FALSE)) {
		chr.bank_1k[i][address & 0x3FF] = value;
	}
}
void extcl_cpu_every_cycle_253(void) {
	if (!m253.irq.active) {
		return;
	}

	if (m253.irq.prescaler < 338) {
		m253.irq.prescaler += 3;
		return;
	}
	m253.irq.prescaler -= 338;

	if (m253.irq.count != 0xFF) {
		m253.irq.count++;
		return;
	}

	m253.irq.count = m253.irq.reload;
	irq.delay = TRUE;
	irq.high |= EXT_IRQ;
}

INLINE static void m253_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8 ; i++) {
		value = m253.chr_map_high[i] | m253.chr_map[i];

		if (((m253.chr_map[i] == 4) || (m253.chr_map[i] == 5)) && (m253.disabled_vram == FALSE)) {
			chr.bank_1k[i] = &chr.extra.data[(value & 0x0001) << 10];
		} else {
			control_bank(info.chr.rom[0].max.banks_1k)
			chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
		}
	}
}
