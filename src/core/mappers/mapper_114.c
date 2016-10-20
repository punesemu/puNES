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

#define m114_prg_rom_backup()\
	m114.prg_rom_bank[0] = mapper.rom_map_to[0];\
	m114.prg_rom_bank[1] = mapper.rom_map_to[1];\
	m114.prg_rom_bank[2] = mapper.rom_map_to[2];\
	m114.prg_rom_bank[3] = mapper.rom_map_to[3]
#define m114_prg_rom_restore()\
	mapper.rom_map_to[0] = m114.prg_rom_bank[0];\
	mapper.rom_map_to[1] = m114.prg_rom_bank[1];\
	mapper.rom_map_to[2] = m114.prg_rom_bank[2];\
	mapper.rom_map_to[3] = m114.prg_rom_bank[3]

void map_init_114(void) {
	EXTCL_CPU_WR_MEM(114);
	EXTCL_SAVE_MAPPER(114);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &m114;
	mapper.internal_struct_size[0] = sizeof(m114);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&m114, 0x00, sizeof(m114));
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));

		{
			BYTE i;

			for (i = 0; i < 4; i++) {
				m114.prg_rom_bank[i] = mapper.rom_map_to[i];
			}
		}
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_114(WORD address, BYTE value) {
	if (address < 0x5000) {
		return;
	}

	switch (address & 0xE001) {
		case 0x4000:
		case 0x4001:
		case 0x6000:
		case 0x6001:
			m114.prg_rom_switch = value >> 7;
			if (m114.prg_rom_switch) {
				control_bank_with_AND(0x1F, info.prg.rom[0].max.banks_16k)
				map_prg_rom_8k(2, 0, value);
				map_prg_rom_8k(2, 2, value);
				map_prg_rom_8k_update();
			} else {
				m114_prg_rom_restore();
				map_prg_rom_8k_update();
			}
			return;
		case 0x8000:
		case 0x8001:
			extcl_cpu_wr_mem_MMC3(0xA000, value);
			return;
		case 0xA000:
			value = (value & 0xC0) | vlu114[value & 0x07];
			m114.mmc3_ctrl_change = TRUE;
			extcl_cpu_wr_mem_MMC3(0x8000, value);
			if (m114.prg_rom_switch) {
				const BYTE prg_rom_cfg = (value & 0x40) >> 5;

				if (mmc3.prg_rom_cfg != prg_rom_cfg) {
					BYTE p0 = m114.prg_rom_bank[0];
					BYTE p2 = m114.prg_rom_bank[2];
					m114.prg_rom_bank[0] = p2;
					m114.prg_rom_bank[2] = p0;
					m114.prg_rom_bank[prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;
				}
			} else {
				m114_prg_rom_backup();
			}
			return;
		case 0xA001:
			irqA12.latch = value;
			return;
		case 0xC000:
			if (m114.mmc3_ctrl_change && (!m114.prg_rom_switch || (mmc3.bank_to_update < 6))) {
				m114.mmc3_ctrl_change = FALSE;
				extcl_cpu_wr_mem_MMC3(0x8001, value);
				m114_prg_rom_backup();
			}
			return;
		case 0xC001:
			irqA12.reload = TRUE;
			irqA12.counter = 0;
			return;
		case 0xE000:
			irqA12.enable = FALSE;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE001:
			irqA12.enable = TRUE;
			return;
	}
}
BYTE extcl_save_mapper_114(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m114.prg_rom_switch);
	save_slot_ele(mode, slot, m114.mmc3_ctrl_change);
	if (save_slot.version < 6) {
		if (mode == SAVE_SLOT_READ) {
			BYTE old_prg_rom_bank[4], i;

			save_slot_ele(mode, slot, old_prg_rom_bank)

			for (i = 0; i < 4; i++) {
				m114.prg_rom_bank[i] = old_prg_rom_bank[i];
			}
		} else if (mode == SAVE_SLOT_COUNT) {
			save_slot.tot_size[slot] += sizeof(BYTE) * 4;
		}
	} else {
		save_slot_ele(mode, slot, m114.prg_rom_bank);
	}
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
