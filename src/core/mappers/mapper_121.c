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
#include "irqA12.h"
#include "save_slot.h"

INLINE static void m121_update_reg(void);
INLINE static void m121_update_prg(void);
INLINE static void m121_update_chr(void);

#define m121_swap_chr_1k(a, b)\
	chr1k = m121.chr_map[b];\
	m121.chr_map[b] = m121.chr_map[a];\
	m121.chr_map[a] = chr1k

#define m121_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		BYTE chr1k;\
		m121_swap_chr_1k(0, 4);\
		m121_swap_chr_1k(1, 5);\
		m121_swap_chr_1k(2, 6);\
		m121_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = m121.prg_map[0];\
		mapper.rom_map_to[0] = m121.prg_map[2];\
		m121.prg_map[0] = mapper.rom_map_to[0];\
		m121.prg_map[2] = mapper.rom_map_to[2];\
		m121.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define m121_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			m121.chr_map[mmc3.chr_rom_cfg] = value;\
			m121.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)\
			m121.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			m121.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			m121.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			m121.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			m121.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			m121.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			m121.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			m121.prg_map[1] = value;\
			break;\
	}

struct _m121 {
	BYTE reg[8];
	WORD prg_map[4];
	WORD chr_map[8];
} m121;
static const BYTE vlu121[4] = { 0x83, 0x83, 0x42, 0x00 };

void map_init_121(void) {
	EXTCL_CPU_WR_MEM(121);
	EXTCL_CPU_RD_MEM(121);
	EXTCL_SAVE_MAPPER(121);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &m121;
	mapper.internal_struct_size[0] = sizeof(m121);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&m121, 0x00, sizeof(m121));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m121.prg_map[i] = mapper.rom_map_to[i];
			}
			m121.chr_map[i] = i;
		}
	}

	if (info.reset >= HARD) {
		m121.reg[3] = 0x80;
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_121(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
		BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

		switch (address & 0xE003) {
			case 0x8000:
				extcl_cpu_wr_mem_MMC3(address, value);
				m121_8000()
				m121_update_prg();
				m121_update_chr();
				return;
			case 0x8001:
				extcl_cpu_wr_mem_MMC3(address, value);
				m121_8001()
				m121.reg[6] = ((value & 0x01) << 5) | ((value & 0x02) << 3) | ((value & 0x04) << 1)
							| ((value & 0x08) >> 1) | ((value & 0x10) >> 3) | ((value & 0x20) >> 5);
				if (!m121.reg[7]) {
					m121_update_reg();
				}
				m121_update_prg();
				m121_update_chr();
				return;
			case 0x8003:
				extcl_cpu_wr_mem_MMC3(0x8000, value);
				m121_8000()
				m121.reg[5] = value;
				m121_update_reg();
				m121_update_prg();
				return;
			default:
				extcl_cpu_wr_mem_MMC3(address, value);
				return;
		}
	}

	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m121.reg[4] = vlu121[value & 0x03];
		if ((address & 0x5180) == 0x5180) {
			m121.reg[3] = value;
			m121_update_prg();
			m121_update_chr();
		}
	}
}
BYTE extcl_cpu_rd_mem_121(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address < 0x5000) || (address > 0x5FFF)) {
		return (openbus);
	}

	return (m121.reg[4]);
}
BYTE extcl_save_mapper_121(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m121.reg);
	save_slot_ele(mode, slot, m121.prg_map);
	save_slot_ele(mode, slot, m121.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void m121_update_reg(void) {
	switch (m121.reg[5] & 0x3F) {
		case 0x20:
			m121.reg[7] = 1;
			m121.reg[0] = m121.reg[6];
			break;
		case 0x29:
			m121.reg[7] = 1;
			m121.reg[0] = m121.reg[6];
			break;
		case 0x26:
			m121.reg[7] = 0;
			m121.reg[0] = m121.reg[6];
			break;
		case 0x2B:
			m121.reg[7] = 1;
			m121.reg[0] = m121.reg[6];
			break;
		case 0x2C:
			m121.reg[7] = 1;
			if (m121.reg[6]) {
				m121.reg[0] = m121.reg[6];
			}
			break;
		case 0x3C:
		case 0x3F:
			m121.reg[7] = 1;
			m121.reg[0] = m121.reg[6];
			break;
		case 0x28:
			m121.reg[7] = 0;
			m121.reg[1] = m121.reg[6];
			break;
		case 0x2A:
			m121.reg[7] = 0;
			m121.reg[2] = m121.reg[6];
			break;
		case 0x2F:
			break;
		default:
			m121.reg[5] = 0;
			break;
	}
}
INLINE static void m121_update_prg(void) {
	BYTE value;

	if (m121.reg[5] & 0x3F) {
		value = (m121.prg_map[mmc3.prg_rom_cfg] & 0x1F) | ((m121.reg[3] & 0x80) >> 2);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);

		value = m121.reg[2] | ((m121.reg[3] & 0x80) >> 2);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		value = m121.reg[1] | ((m121.reg[3] & 0x80) >> 2);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		value = m121.reg[0] | ((m121.reg[3] & 0x80) >> 2);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	} else {
		value = (m121.prg_map[0] & 0x1F) | ((m121.reg[3] & 0x80) >> 2);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		value = (m121.prg_map[1] & 0x1F) | ((m121.reg[3] & 0x80) >> 2);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		value = (m121.prg_map[2] & 0x1F) | ((m121.reg[3] & 0x80) >> 2);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		value = (m121.prg_map[3] & 0x1F) | ((m121.reg[3] & 0x80) >> 2);
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void m121_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		value = m121.chr_map[i];

		if (prg_chip_size(0) == chr_chip_size(0)) {
			value = value | ((m121.reg[3] & 0x80) << 1);
		} else if ((i & 0x04) == mmc3.chr_rom_cfg) {
			value = value | 0x100;
		}
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
