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

INLINE static void prg_fix_517(void);

struct _m517 {
	BYTE reg;
	struct _m517_adc {
		int data;
		int high;
		int low;
		BYTE state;
	} adc;
} m517;

void map_init_517(void) {
	EXTCL_AFTER_MAPPER_INIT(517);
	EXTCL_CPU_WR_MEM(517);
	EXTCL_CPU_RD_MEM(517);
	EXTCL_SAVE_MAPPER(517);
	map_internal_struct_init((BYTE *)&m517, sizeof(m517));

	if (info.reset >= HARD) {
		memset(&m517, 0x00, sizeof(m517));
	}

	m517.adc.data = 0;
	m517.adc.state = 0;
}
void extcl_after_mapper_init_517(void) {
	prg_fix_517();
}
void extcl_cpu_wr_mem_517(UNUSED(BYTE nidx), WORD address, BYTE value) {
	if ((address >= 0x8000) && (address <= 0x8FFF)) {
//		TODO : gestione microfono
//		m517.adc.data = <microphone> * 63.0;
		m517.adc.data = 0.0 * 63.0;
		m517.adc.high = m517.adc.data >> 2;
		m517.adc.low = 0x40 - m517.adc.high - ((m517.adc.data & 0x03) << 2);
		m517.adc.state = 0;
	}
	m517.reg = value;
	prg_fix_517();
}
BYTE extcl_cpu_rd_mem_517(BYTE nidx, WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x6000) && (address <= 0x6FFF)) {
		BYTE result = 0;

		if (address == 0x6000) {
			switch (m517.adc.state) {
				case 0:
					m517.adc.state = 1;
					result = 0;
					break;
				case 1:
					m517.adc.state = 2;
					result = 1;
					break;
				case 2:
					if (m517.adc.low > 0) {
						m517.adc.low--;
						result = 1;
					} else {
						m517.adc.state = 0;
						result = 0;
					}
					break;
			}
		} else {
			result = m517.adc.high-- > 0 ? 0 : 1;
		}
		return (result);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_517(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m517.reg);
	save_slot_ele(mode, slot, m517.adc.data);
	save_slot_ele(mode, slot, m517.adc.high);
	save_slot_ele(mode, slot, m517.adc.low);
	save_slot_ele(mode, slot, m517.adc.state);
	return (EXIT_OK);
}

INLINE static void prg_fix_517(void) {
	memmap_auto_16k(0, MMCPU(0x8000), m517.reg);
	memmap_auto_16k(0, MMCPU(0xC000), 0xFF);
}
