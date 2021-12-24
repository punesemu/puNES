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
#include "irqA12.h"
#include "save_slot.h"

INLINE static void m187_update_prg(void);
INLINE static void m187_update_chr(void);

#define m187_swap_chr_1k(a, b)\
	chr1k = m187.chr_map[b];\
	m187.chr_map[b] = m187.chr_map[a];\
	m187.chr_map[a] = chr1k
#define m187_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		BYTE chr1k;\
		m187_swap_chr_1k(0, 4);\
		m187_swap_chr_1k(1, 5);\
		m187_swap_chr_1k(2, 6);\
		m187_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = m187.prg_map[0];\
		mapper.rom_map_to[0] = m187.prg_map[2];\
		m187.prg_map[0] = mapper.rom_map_to[0];\
		m187.prg_map[2] = mapper.rom_map_to[2];\
		m187.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define m187_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			m187.chr_map[mmc3.chr_rom_cfg] = value;\
			m187.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			m187.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			m187.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			m187.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			m187.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			m187.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			m187.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			m187.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			m187.prg_map[1] = value;\
			break;\
	}

struct _m187 {
	BYTE reg[8];
	WORD prg_map[4];
	WORD chr_map[8];
} m187;
static const BYTE vlu187[4] = { 0x83, 0x83, 0x42, 0x00 };

void map_init_187(void) {
	EXTCL_CPU_WR_MEM(187);
	EXTCL_CPU_RD_MEM(187);
	EXTCL_SAVE_MAPPER(187);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m187;
	mapper.internal_struct_size[0] = sizeof(m187);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&m187, 0x00, sizeof(m187));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m187.prg_map[i] = mapper.rom_map_to[i];
			}
			m187.chr_map[i] = i;
		}
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_187(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
		BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

		switch (address) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				m187_8000()
				m187.reg[1] = 1;
				m187_update_prg();
				m187_update_chr();
				return;
			case 0x8001:
				if (m187.reg[1]) {
					extcl_cpu_wr_mem_MMC3(address, value);
					m187_8001()
					m187_update_prg();
					m187_update_chr();
				}
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}

	if ((address == 0x5000) || (address == 0x6000)) {
		m187.reg[0] = value;
		m187_update_prg();
		m187_update_chr();
	}
}
BYTE extcl_cpu_rd_mem_187(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x5000) || (address > 0x5FFF)) {
		return (openbus);
	}

	return (vlu187[m187.reg[0] & 0x03]);
}
BYTE extcl_save_mapper_187(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m187.reg);
	save_slot_ele(mode, slot, m187.prg_map);
	save_slot_ele(mode, slot, m187.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void m187_update_prg(void) {
	BYTE value;

	if (m187.reg[0] & 0x80) {
		value = m187.reg[0] & 0x1F;

		if (m187.reg[0] & 0x20) {
			if (m187.reg[0] & 0x40) {
				value = value >> 2;
			} else {
				value = value >> 1;
			}
			control_bank(info.prg.rom[0].max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			control_bank(info.prg.rom[0].max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}
	} else {
		value = m187.prg_map[0];
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		value = m187.prg_map[1];
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		value = m187.prg_map[2];
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		value = m187.prg_map[3];
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void m187_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		value = m187.chr_map[i];
		if ((i & 0x04) == mmc3.chr_rom_cfg) {
			value = value | 0x100;
		}
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
