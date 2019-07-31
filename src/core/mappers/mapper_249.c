/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#define m249_swap_chr_bank_1k(src, dst)\
{\
	BYTE *chr_bank_1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chr_bank_1k;\
	WORD map = m249.chr_map[src];\
	m249.chr_map[src] = m249.chr_map[dst];\
	m249.chr_map[dst] = map;\
}
/* fceux */
#define m249_chr_1k(vl)\
	if (m249.reg) {\
		value = (vl & 0x03) | ((vl >> 1) & 0x04) | ((vl >> 4) & 0x08)\
				| ((vl >> 2) & 0x10) | ((vl << 3) & 0x20)\
				| ((vl << 2) & 0xC0);\
	}
#define m249_prg_8k(vl)\
	if (m249.reg) {\
		if (vl < 0x20) {\
			value = (vl & 0x01) | ((vl >> 3) & 0x02) | ((vl >> 1) & 0x04)\
					| ((vl << 2) & 0x08) | ((vl << 2) & 0x10);\
		} else {\
			value = vl - 0x20;\
			value = (vl & 0x03) | ((vl >> 1) & 0x04) | ((vl >> 4) & 0x08)\
					| ((vl >> 2) & 0x10) | ((vl << 3) & 0x20)\
					| ((vl << 2) & 0xC0);\
		}\
	}
/**/
/* nestopia
#define m249_chr_1k(vl)\
	if (m249.reg) {\
		value = ((vl >> 0) & 0x03) | ((vl >> 1) & 0x04) | ((vl >> 4) & 0x08) | ((vl >> 2) & 0x10) |\
				((vl << 3) & 0x20) | ((vl << 2) & 0x40) | ((vl << 2) & 0x80);\
	}
#define m249_prg_8k(vl)\
	if (m249.reg) {\
		value = ((vl << 0) & 0x01) | ((vl >> 3) & 0x02) | ((vl >> 1) & 0x04) | ((vl << 2) & 0x18);\
	}
*/

void map_init_249(void) {
	EXTCL_CPU_WR_MEM(249);
	EXTCL_CPU_RD_MEM(249);
	EXTCL_SAVE_MAPPER(249);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &m249;
	mapper.internal_struct_size[0] = sizeof(m249);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&m249, 0x00, sizeof(m249));
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));

		{
			BYTE i;

			map_chr_bank_1k_reset();

			for (i = 0; i < 8; i++) {
				if (i < 4) {
					m249.prg_map[i] = mapper.rom_map_to[i];
				}
				m249.chr_map[i] = i;
			}
		}
	} else {
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_249(WORD address, BYTE value) {
	BYTE save = value;

	if (address == 0x5000) {
		value &= 0x02;

		if (m249.reg != value) {
			BYTE i;

			m249.reg = value;

			for (i = 0; i < 8; i++) {
				if (i < 4) {
					m249_prg_8k(m249.prg_map[i])
					control_bank(info.prg.rom[0].max.banks_8k)
					map_prg_rom_8k(1, i, value);
				}
				m249_chr_1k(m249.chr_map[i])
				control_bank(info.chr.rom[0].max.banks_1k)
				chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
			}

			map_prg_rom_8k_update();
		}

		return;
	}

	switch (address & 0xE001) {
		case 0x8000: {
			const BYTE chr_rom_cfg_old = mmc3.chr_rom_cfg;
			const BYTE prg_rom_cfg_old = mmc3.prg_rom_cfg;
			mmc3.bank_to_update = value & 0x07;
			mmc3.prg_rom_cfg = (value & 0x40) >> 5;
			mmc3.chr_rom_cfg = (value & 0x80) >> 5;
			if (mmc3.chr_rom_cfg != chr_rom_cfg_old) {
				m249_swap_chr_bank_1k(0, 4)
				m249_swap_chr_bank_1k(1, 5)
				m249_swap_chr_bank_1k(2, 6)
				m249_swap_chr_bank_1k(3, 7)
			}
			if (mmc3.prg_rom_cfg != prg_rom_cfg_old) {
				WORD p0 = mapper.rom_map_to[0];
				WORD p2 = mapper.rom_map_to[2];
				mapper.rom_map_to[0] = p2;
				mapper.rom_map_to[2] = p0;

				p0 = m249.prg_map[0];
				p2 = m249.prg_map[2];
				m249.prg_map[0] = p2;
				m249.prg_map[2] = p0;

				m249.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;
				m249_prg_8k(info.prg.rom[0].max.banks_8k_before_last);
				control_bank(info.prg.rom[0].max.banks_8k)
				map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, value);
				map_prg_rom_8k_update();
			}
			return;
		}
		case 0x8001:
			switch (mmc3.bank_to_update) {
				case 0:
					m249.chr_map[mmc3.chr_rom_cfg] = value;
					m249.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;
					m249_chr_1k(value)
					control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)
					chr.bank_1k[mmc3.chr_rom_cfg] = chr_chip_byte_pnt(0, value << 10);
					chr.bank_1k[mmc3.chr_rom_cfg | 0x01] = chr_chip_byte_pnt(0, (value + 1) << 10);
					return;
				case 1:
					m249.chr_map[mmc3.chr_rom_cfg | 0x02] = value;
					m249.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;
					m249_chr_1k(value)
					control_bank_with_AND(0xFE, info.chr.rom[0].max.banks_1k)
					chr.bank_1k[mmc3.chr_rom_cfg | 0x02] = chr_chip_byte_pnt(0, value << 10);
					chr.bank_1k[mmc3.chr_rom_cfg | 0x03] = chr_chip_byte_pnt(0, (value + 1) << 10);
					return;
				case 2:
					m249.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;
					m249_chr_1k(value)
					control_bank(info.chr.rom[0].max.banks_1k)
					chr.bank_1k[mmc3.chr_rom_cfg ^ 0x04] = chr_chip_byte_pnt(0, value << 10);
					return;
				case 3:
					m249.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;
					m249_chr_1k(value)
					control_bank(info.chr.rom[0].max.banks_1k)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = chr_chip_byte_pnt(0, value << 10);
					return;
				case 4:
					m249.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;
					m249_chr_1k(value)
					control_bank(info.chr.rom[0].max.banks_1k)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = chr_chip_byte_pnt(0, value << 10);
					return;
				case 5:
					m249.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;
					m249_chr_1k(value)
					control_bank(info.chr.rom[0].max.banks_1k)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = chr_chip_byte_pnt(0, value << 10);
					return;
				case 6:
					m249.prg_map[mmc3.prg_rom_cfg] = value;
					m249_prg_8k(value)
					control_bank(info.prg.rom[0].max.banks_8k)
					map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);
					map_prg_rom_8k_update();
					return;
				case 7:
					m249.prg_map[1] = value;
					m249_prg_8k(value)
					control_bank(info.prg.rom[0].max.banks_8k)
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
					return;
			}
			return;
	}

	extcl_cpu_wr_mem_MMC3(address, save);
}
BYTE extcl_cpu_rd_mem_249(WORD address, BYTE openbus, BYTE before) {
	if ((address >= 0x5000) && (address < 0x6000)) {
		return (before);
	}
	return (openbus);
}
BYTE extcl_save_mapper_249(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m249.reg);
	save_slot_ele(mode, slot, m249.prg_map);
	save_slot_ele(mode, slot, m249.chr_map);

	return (EXIT_OK);
}
