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
#include "cpu.h"
#include "save_slot.h"

INLINE static void prg_fix_083(void);
INLINE static void chr_fix_083(void);
INLINE static void wram_fix_083(void);
INLINE static void mirroring_fix_083(void);

struct _m083 {
	BYTE mode;
	BYTE outer;
	BYTE low[4];
	BYTE prg[4];
	BYTE chr[8];
	struct _m083_irq {
		BYTE active;
		WORD count;
	} irq;
} m083;
struct _m083tmp {
	BYTE prg_mask;
	BYTE chr_mode;
	WORD dip_mask;
	BYTE use_wram;
} m083tmp;

void map_init_083(void) {
	EXTCL_AFTER_MAPPER_INIT(083);
	EXTCL_CPU_WR_MEM(083);
	EXTCL_CPU_RD_MEM(083);
	EXTCL_SAVE_MAPPER(083);
	EXTCL_CPU_EVERY_CYCLE(083);
	mapper.internal_struct[0] = (BYTE *)&m083;
	mapper.internal_struct_size[0] = sizeof(m083);

	if (info.reset >= HARD) {
		memset(&m083, 0x00, sizeof(m083));

		m083.prg[0] = 0xFC;
		m083.prg[1] = 0xFD;
		m083.prg[2] = 0xFE;
		m083.prg[3] = 0xFF;

		m083.chr[0] = 0;
		m083.chr[1] = 1;
		m083.chr[2] = 2;
		m083.chr[3] = 3;
		m083.chr[4] = 4;
		m083.chr[5] = 5;
		m083.chr[6] = 6;
		m083.chr[7] = 7;

		m083.mode = 0x10;
	}

	if (info.mapper.id == 83) {
		if (info.format != NES_2_0) {
			if (chrrom_size() >= S1M) {
				info.mapper.submapper = 2;
				wram_set_nvram_size(S32K);
			} else if (chrrom_size() >= S512K) {
				info.mapper.submapper = 1;
			}
		}
		m083tmp.prg_mask = 0x1F;
		m083tmp.use_wram = info.mapper.submapper == 2;
		m083tmp.chr_mode = info.mapper.submapper;
		m083tmp.dip_mask = 0x100;
	} else if (info.mapper.id == 264) {
		m083tmp.prg_mask = 0x0F;
		m083tmp.use_wram = FALSE;
		m083tmp.chr_mode = 1;
		m083tmp.dip_mask = 0x400;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_083(void) {
	prg_fix_083();
	chr_fix_083();
	wram_fix_083();
	mirroring_fix_083();
}
void extcl_cpu_wr_mem_083(WORD address, BYTE value) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		m083.low[address & 0x03] = value;
	} else if (address >= 0x8000) {
		if (info.mapper.id == 264) {
			address = ((address & 0xF00) >> 2) | (address & 0x003F);
		}
		switch (address & 0x0300) {
			case 0x0000:
				m083.outer = value;
				prg_fix_083();
				chr_fix_083();
				return;
			case 0x0100:
				m083.mode = value;
				prg_fix_083();
				wram_fix_083();
				mirroring_fix_083();
				return;
			case 0x0200:
				if (address & 0x01) {
					m083.irq.active = m083.mode & 0x80;
					m083.irq.count = (m083.irq.count & 0x00FF) | (value << 8);
				} else {
					m083.irq.count = (m083.irq.count & 0xFF00) | value;
					nes.c.irq.high &= ~EXT_IRQ;
				}
				return;
			case 0x0300: {
				BYTE index = address & 0x1F;

				if (index <= 0x0F) {
					m083.prg[index & 0x03] = value;
					prg_fix_083();
					wram_fix_083();
				} else if (index <= 0x17) {
					m083.chr[index & 0x07] = value;
					chr_fix_083();
				}
				return;
			}
		}
	}
}
BYTE extcl_cpu_rd_mem_083(WORD address, UNUSED(BYTE openbus)) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return (address & m083tmp.dip_mask ? m083.low[address & 0x03] : dipswitch.value);
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_083(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m083.mode);
	save_slot_ele(mode, slot, m083.outer);
	save_slot_ele(mode, slot, m083.low);
	save_slot_ele(mode, slot, m083.prg);
	save_slot_ele(mode, slot, m083.chr);
	save_slot_ele(mode, slot, m083.irq.active);
	save_slot_ele(mode, slot, m083.irq.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_083(void) {
	if (m083.irq.active && m083.irq.count) {
		m083.irq.count = (m083.mode & 0x40 ? m083.irq.count - 1 : m083.irq.count + 1);
		if (!m083.irq.count) {
			nes.c.irq.high |= EXT_IRQ;
			m083.irq.active = FALSE;
		}
	}
}

INLINE static void prg_fix_083(void) {
	switch (m083.mode & 0x18) {
		case 0x00:
			memmap_auto_16k(MMCPU(0x8000), m083.outer);
			memmap_auto_16k(MMCPU(0xC000), (m083.outer | (m083tmp.prg_mask >> 1)));
			return;
		case 0x08:
			memmap_auto_32k(MMCPU(0x8000), (m083.outer >> 1));
			return;
		default: {
			WORD base = (m083.outer << 1) & ~m083tmp.prg_mask;

			memmap_auto_8k(MMCPU(0x8000), (base | (m083.prg[0] & m083tmp.prg_mask)));
			memmap_auto_8k(MMCPU(0xA000), (base | (m083.prg[1] & m083tmp.prg_mask)));
			memmap_auto_8k(MMCPU(0xC000), (base | (m083.prg[2] & m083tmp.prg_mask)));
			memmap_auto_8k(MMCPU(0xE000), (base | (0x1F & m083tmp.prg_mask)));
			return;
		}
	}
}
INLINE static void chr_fix_083(void) {
	switch (m083tmp.chr_mode) {
		default:
			memmap_chrrom_1k(MMPPU(0x0000), m083.chr[0]);
			memmap_chrrom_1k(MMPPU(0x0400), m083.chr[1]);
			memmap_chrrom_1k(MMPPU(0x0800), m083.chr[2]);
			memmap_chrrom_1k(MMPPU(0x0C00), m083.chr[3]);
			memmap_chrrom_1k(MMPPU(0x1000), m083.chr[4]);
			memmap_chrrom_1k(MMPPU(0x1400), m083.chr[5]);
			memmap_chrrom_1k(MMPPU(0x1800), m083.chr[6]);
			memmap_chrrom_1k(MMPPU(0x1C00), m083.chr[7]);
			return;
		case 1:
			memmap_chrrom_2k(MMPPU(0x0000), m083.chr[0]);
			memmap_chrrom_2k(MMPPU(0x0800), m083.chr[1]);
			memmap_chrrom_2k(MMPPU(0x1000), m083.chr[6]);
			memmap_chrrom_2k(MMPPU(0x1800), m083.chr[7]);
			return;
		case 2: {
			WORD base = (m083.outer & 0x30) << 4;

			memmap_chrrom_1k(MMPPU(0x0000), (base | m083.chr[0]));
			memmap_chrrom_1k(MMPPU(0x0400), (base | m083.chr[1]));
			memmap_chrrom_1k(MMPPU(0x0800), (base | m083.chr[2]));
			memmap_chrrom_1k(MMPPU(0x0C00), (base | m083.chr[3]));
			memmap_chrrom_1k(MMPPU(0x1000), (base | m083.chr[4]));
			memmap_chrrom_1k(MMPPU(0x1400), (base | m083.chr[5]));
			memmap_chrrom_1k(MMPPU(0x1800), (base | m083.chr[6]));
			memmap_chrrom_1k(MMPPU(0x1C00), (base | m083.chr[7]));
			return;
		}
	}
}
INLINE static void wram_fix_083(void) {
	if (m083tmp.use_wram) {
		memmap_wram_8k(MMCPU(0x6000), (m083tmp.prg_mask >> 6));
	} else if (m083.mode & 0x20) {
		memmap_prgrom_8k(MMCPU(0x6000), m083.prg[3]);
	} else {
		memmap_disable_8k(MMCPU(0x6000));
	}
}
INLINE static void mirroring_fix_083(void) {
	switch (m083.mode & 0x03) {
		case 0:
			mirroring_V();
			break;
		case 1:
			mirroring_H();
			break;
		case 2:
			mirroring_SCR0();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}
}
