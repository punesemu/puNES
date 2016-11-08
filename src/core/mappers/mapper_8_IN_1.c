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
#include "irqA12.h"
#include "save_slot.h"

static void INLINE m8in1_update_prg(void);
static void INLINE m8in1_update_chr(void);

#define m8in1_swap_chr_1k(a, b)\
	chr1k = m8in1.chr_map[b];\
	m8in1.chr_map[b] = m8in1.chr_map[a];\
	m8in1.chr_map[a] = chr1k
#define m8in1_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		WORD chr1k;\
		m8in1_swap_chr_1k(0, 4);\
		m8in1_swap_chr_1k(1, 5);\
		m8in1_swap_chr_1k(2, 6);\
		m8in1_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = m8in1.prg_map[0];\
		mapper.rom_map_to[0] = m8in1.prg_map[2];\
		m8in1.prg_map[0] = mapper.rom_map_to[0];\
		m8in1.prg_map[2] = mapper.rom_map_to[2];\
		m8in1.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define m8in1_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			value &= 0xFE;\
			m8in1.chr_map[mmc3.chr_rom_cfg] = value;\
			m8in1.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			value &= 0xFE;\
			m8in1.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			m8in1.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			m8in1.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			m8in1.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			m8in1.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			m8in1.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			m8in1.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			m8in1.prg_map[1] = value;\
			break;\
	}

void map_init_8_IN_1(void) {
	EXTCL_CPU_WR_MEM(8_IN_1);
	EXTCL_SAVE_MAPPER(8_IN_1);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &m8in1;
	mapper.internal_struct_size[0] = sizeof(m8in1);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&m8in1, 0x00, sizeof(m8in1));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m8in1.prg_map[i] = mapper.rom_map_to[i];
			}
			m8in1.chr_map[i] = i;
		}

		m8in1_update_prg();
		m8in1_update_chr();
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_8_IN_1(WORD address, BYTE value) {
	BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
	BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

	switch (address & 0xF001) {
		case 0x9000:
		case 0x9001:
		case 0xB000:
		case 0xB001:
		case 0xD000:
		case 0xD001:
		case 0xF000:
		case 0xF001:
			m8in1.reg = value;
			m8in1_update_prg();
			m8in1_update_chr();
			return;
		case 0x8000:
			extcl_cpu_wr_mem_MMC3(address, value);
			m8in1_8000()
			m8in1_update_prg();
			m8in1_update_chr();
			return;
		case 0x8001:
			extcl_cpu_wr_mem_MMC3(address, value);
			m8in1_8001()
			m8in1_update_prg();
			m8in1_update_chr();
			return;
		default:
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
	}
}
BYTE extcl_save_mapper_8_IN_1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m8in1.reg);
	save_slot_ele(mode, slot, m8in1.prg_map);
	save_slot_ele(mode, slot, m8in1.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

static void INLINE m8in1_update_prg(void) {
	WORD value;

	if (m8in1.reg & 0x10) {
		BYTE i;

		for (i = 0; i < 4; i++) {
			value = ((m8in1.reg << 2) & 0x30) | (m8in1.prg_map[i] & 0x0F);
			control_bank(info.prg.rom[0].max.banks_8k)
			map_prg_rom_8k(1, i, value);
		}
	} else {
		value = m8in1.reg & 0x0F;
		control_bank(info.prg.rom[0].max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}
	map_prg_rom_8k_update();
}
static void INLINE m8in1_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		value = ((m8in1.reg << 5) & 0x0180) | (m8in1.chr_map[i] & 0x7F);
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
