/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

static void INLINE m205_update_prg(void);
static void INLINE m205_update_chr(void);

#define m205_chr_1k(vl) value = ((m205.reg[0] & 0x30) << 3) | vl
#define m205_prg_8k(vl) value = (m205.reg[0] & 0x30) | (vl & ((m205.reg[0] & 0xC0) ? 0x0F : 0x1F))
#define m205_swap_chr_1k(a, b)\
	chr1k = m205.chr_map[b];\
	m205.chr_map[b] = m205.chr_map[a];\
	m205.chr_map[a] = chr1k
#define m205_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		BYTE chr1k;\
		m205_swap_chr_1k(0, 4);\
		m205_swap_chr_1k(1, 5);\
		m205_swap_chr_1k(2, 6);\
		m205_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = m205.prg_map[0];\
		mapper.rom_map_to[0] = m205.prg_map[2];\
		m205.prg_map[0] = mapper.rom_map_to[0];\
		m205.prg_map[2] = mapper.rom_map_to[2];\
		m205.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define m205_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			m205.chr_map[mmc3.chr_rom_cfg] = value;\
			m205.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			m205.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			m205.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			m205.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			m205.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			m205.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			m205.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			m205.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			m205.prg_map[1] = value;\
			break;\
	}

void map_init_205(void) {
	EXTCL_CPU_WR_MEM(205);
	EXTCL_SAVE_MAPPER(205);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &m205;
	mapper.internal_struct_size[0] = sizeof(m205);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m205, 0x00, sizeof(m205));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m205.prg_map[i] = mapper.rom_map_to[i];
			}
			m205.chr_map[i] = i;
		}

		m205_update_prg();
		m205_update_chr();
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_205(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
		BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

		switch (address & 0xE001) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				m205_8000()
				m205_update_prg();
				m205_update_chr();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				m205_8001()
				m205_update_prg();
				m205_update_chr();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}

	if (address < 0x6000) {
		return;
	}

	if (address < 0x7000) {
		if (m205.reg[1] == 0) {
			m205.reg[0] = (value << 4) & 0xF0;
			m205.reg[1] = address & 0x80;
			m205_update_prg();
			m205_update_chr();
		}
		return;
	}

	if (address < 0x8000) {
		if (m205.reg[1] == 0) {
			m205.reg[0] = value & 0xF0;
			m205_update_prg();
			m205_update_chr();
		}
		return;
	}
}
BYTE extcl_save_mapper_205(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m205.reg);
	save_slot_ele(mode, slot, m205.prg_map);
	save_slot_ele(mode, slot, m205.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

static void INLINE m205_update_prg(void) {
	BYTE value;

	m205_prg_8k(m205.prg_map[0]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 0, value);

	m205_prg_8k(m205.prg_map[1]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 1, value);

	m205_prg_8k(m205.prg_map[2]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 2, value);

	m205_prg_8k(m205.prg_map[3]);
	control_bank(info.prg.rom[0].max.banks_8k)
	map_prg_rom_8k(1, 3, value);

	map_prg_rom_8k_update();
}
static void INLINE m205_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		m205_chr_1k(m205.chr_map[i]);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
