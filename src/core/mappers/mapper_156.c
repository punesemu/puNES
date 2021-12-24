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

INLINE static void chr_setup_156(void);

struct _m156 {
	struct _m156_chr {
		BYTE low[8];
		BYTE high[8];
	} chr;
} m156;

void map_init_156(void) {
	EXTCL_CPU_WR_MEM(156);
	EXTCL_SAVE_MAPPER(156);
	mapper.internal_struct[0] = (BYTE *)&m156;
	mapper.internal_struct_size[0] = sizeof(m156);

	if (info.reset >= HARD) {
		memset(&m156, 0x00, sizeof(m156));
		map_prg_rom_8k(2, 0, 0);
	}

	mirroring_SCR0();
}
void extcl_cpu_wr_mem_156(WORD address, BYTE value) {
	switch (address) {
		case 0xC000:
		case 0xC001:
		case 0xC002:
		case 0xC003:
			m156.chr.low[address & 0x0003] = value;
			chr_setup_156();
			return;
		case 0xC004:
		case 0xC005:
		case 0xC006:
		case 0xC007:
			m156.chr.high[address & 0x0003] = value;
			chr_setup_156();
			return;
		case 0xC008:
		case 0xC009:
		case 0xC00A:
		case 0xC00B:
			m156.chr.low[(address & 0x0003) + 4] = value;
			chr_setup_156();
			return;
		case 0xC00C:
		case 0xC00D:
		case 0xC00E:
		case 0xC00F:
			m156.chr.high[(address & 0x0003) + 4] = value;
			chr_setup_156();
			return;
		case 0xC010:
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0xC014:
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
	}
}
BYTE extcl_save_mapper_156(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m156.chr.low);
	save_slot_ele(mode, slot, m156.chr.high);

	return (EXIT_OK);
}

INLINE static void chr_setup_156(void) {
	WORD value;
	BYTE i;

	for (i = 0; i < 8; i++) {
		value = (m156.chr.high[i] << 8) | m156.chr.low[i];
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
