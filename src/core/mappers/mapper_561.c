/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include <stdlib.h>
#include "mappers.h"
#include "save_slot.h"

INLINE static void prg_fix_561(void);
INLINE static void chr_fix_561(void);
INLINE static void mirroring_fix_561(void);

struct _m561 {
	BYTE prg[4];
	BYTE chr;
	BYTE reg;
	BYTE reg1m;
	BYTE reg4m;
	struct _m561_irq {
		BYTE reg;
		SWORD count_fds;
		SWORD count_sgd;
	} irq;
} m561;

void map_init_561(void) {
	EXTCL_AFTER_MAPPER_INIT(561);
	EXTCL_CPU_INIT_PC(561);
	EXTCL_CPU_WR_MEM(561);
	EXTCL_SAVE_MAPPER(561);
	EXTCL_CPU_EVERY_CYCLE(561);
	map_internal_struct_init((BYTE *)&m561, sizeof(m561));

	if (info.reset >= HARD) {
		memset(&m561, 0x00, sizeof(m561));

		m561.reg1m = (info.mapper.submapper << 5) | (info.mapper.mirroring == MIRRORING_VERTICAL ? 0x01: 0x11) | 0x02;
		m561.reg4m = 0x03;
		m561.prg[0] = 0x1C;
		m561.prg[1] = 0x1D;
		m561.prg[2] = 0x1E;
		m561.prg[3] = 0x1F;
	}

	memset(&m561.irq, 0x00, sizeof(m561.irq));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_561(void) {
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if ((info.mapper.submapper == 3) && (prgrom_size() < S256K)) {
			prgrom_pnt() = realloc(prgrom_pnt(), S256K);
			prgrom_set_size(S256K);
		}
		if (chrrom_size()) {
			if (!chrrom_size() || (vram_size(0) < chrrom_size())) {
				vram_set_ram_size(0, chrrom_size());
				vram_init();
			}
			memcpy(vram_pnt(0), chrrom_pnt(), chrrom_size());
		}
	}
	prg_fix_561();
	chr_fix_561();
	mirroring_fix_561();
}
void extcl_cpu_init_pc_561(BYTE nidx) {
	if (info.reset >= HARD) {
		if (miscrom_size() >= 4) {
			WORD address = 0x7000;
			WORD init = 0x7003;
			size_t size = 512;
			BYTE *data = miscrom_pnt();

			if (miscrom_size() != size) {
				address = (miscrom_byte(1) << 8) | miscrom_byte(0);
				init = (miscrom_byte(3) << 8) | miscrom_byte(2);
				size = miscrom_size() - 4;
				data = miscrom_pnt_byte(4);
			}

			if (address < 0x2000) {
				memcpy(ram_pnt_byte(nidx, address & 0x1FFF), data, size);
			} else {
				memcpy(wram_pnt_byte(address & 0x1FFF), data, size);
			}

			if (init) {
				// JSR init
				ram_wr(nidx, 0x700, 0x20);
				ram_wr(nidx, 0x701, init & 0xFF);
				ram_wr(nidx, 0x702, init >> 8);

				// JMP ($FFFC)
				ram_wr(nidx, 0x703, 0x6C);
				ram_wr(nidx, 0x704, 0xFC);
				ram_wr(nidx, 0x705, 0xFF);

				nes[nidx].c.cpu.PC.w = 0x700;
			}
		}
		r4015.value &= 0xBF;
		nes[nidx].c.irq.high &= ~APU_IRQ;
	}
}
void extcl_cpu_wr_mem_561(BYTE nidx, WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		switch (address) {
			case 0x4024:
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return;
			case 0x4025:
				m561.irq.reg = value;
				if (m561.irq.reg & 0x42) {
					m561.irq.count_fds = 0;
				}
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return;
			case 0x4100:
				m561.irq.count_sgd = (SWORD)((m561.irq.count_sgd & 0xFF00) | value);
				if (!value) {
					m561.irq.count_sgd = value;
				}
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return;
			case 0x4101:
				m561.irq.count_sgd = (SWORD)((m561.irq.count_sgd & 0x00FF) | (value << 8));
				nes[nidx].c.irq.high &= ~EXT_IRQ;
				return;
			case 0x42FC:
			case 0x42FD:
			case 0x42FE:
			case 0x42FF:
				m561.reg1m = (value & 0xF0) | (address & 0x03);
				prg_fix_561();
				chr_fix_561();
				mirroring_fix_561();
				return;
			case 0x43FC:
			case 0x43FD:
			case 0x43FE:
			case 0x43FF:
				m561.reg4m = address & 0x01;
				m561.chr = value & 0x03;
				prg_fix_561();
				chr_fix_561();
				return;
			default:
				return;
		}
	} else if (address >= 0x8000) {
		if (m561.reg1m & 0x02) {
			m561.reg = value;
			switch (m561.reg1m >> 5) {
				case 0:
				case 2:
					m561.chr = 0;
					break;
				case 1:
				case 4:
				case 5:
					m561.chr = value & 0x03;
					break;
				case 3:
					m561.chr = (value & 0x30) >> 4;
					break;
				default:
					break;
			}
			m561.prg[(address >> 13) & 0x03] = value >> 2;
			prg_fix_561();
			chr_fix_561();
		}
	}
}
BYTE extcl_save_mapper_561(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m561.prg);
	save_slot_ele(mode, slot, m561.chr);
	save_slot_ele(mode, slot, m561.reg);
	save_slot_ele(mode, slot, m561.reg1m);
	save_slot_ele(mode, slot, m561.reg4m);
	save_slot_ele(mode, slot, m561.irq.reg);
	save_slot_ele(mode, slot, m561.irq.count_fds);
	save_slot_ele(mode, slot, m561.irq.count_sgd);
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_561(BYTE nidx) {
	m561.irq.count_fds += 3;
	while ((m561.irq.count_fds >= 448) && (m561.irq.reg & 0x80)) {
		m561.irq.count_fds -= 448;
		nes[nidx].c.irq.high |= EXT_IRQ;
	}
	if ((m561.irq.count_sgd < 0) && !++m561.irq.count_sgd) {
		nes[nidx].c.irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_561(void) {
	BYTE wr = !(m561.reg1m & 0x02);

	if (!(m561.reg4m & 0x01)) {
		memmap_auto_wp_8k(0, MMCPU(0x8000), m561.prg[0], TRUE, wr);
		memmap_auto_wp_8k(0, MMCPU(0xA000), m561.prg[1], TRUE, wr);
		memmap_auto_wp_8k(0, MMCPU(0xC000), m561.prg[2], TRUE, wr);
		memmap_auto_wp_8k(0, MMCPU(0xE000), m561.prg[3], TRUE, wr);
	} else {
		switch (m561.reg1m >> 5) {
			case 0:
				memmap_auto_wp_16k(0, MMCPU(0x8000), (m561.reg & 0x07), TRUE, wr);
				memmap_auto_wp_16k(0, MMCPU(0xC000), 0x07, TRUE, wr);
				return;
			case 1:
				memmap_auto_wp_16k(0, MMCPU(0x8000), ((m561.reg & 0x3C) >> 2), TRUE, wr);
				memmap_auto_wp_16k(0, MMCPU(0xC000), 0x07, TRUE, wr);
				return;
			case 2:
				memmap_auto_wp_16k(0, MMCPU(0x8000), (m561.reg & 0x0F), TRUE, wr);
				memmap_auto_wp_16k(0, MMCPU(0xC000), 0x0F, TRUE, wr);
				return;
			case 3:
				memmap_auto_wp_16k(0, MMCPU(0x8000), 0x0F, TRUE, wr);
				memmap_auto_wp_16k(0, MMCPU(0xC000), (m561.reg & 0x0F), TRUE, wr);
				return;
			case 4:
				memmap_auto_wp_32k(0, MMCPU(0x8000), ((m561.reg & 0x30) >> 4), TRUE, wr);
				return;
			case 5:
				memmap_auto_wp_32k(0, MMCPU(0x8000), 0x03, TRUE, wr);
				return;
			case 6:
				memmap_auto_wp_8k(0, MMCPU(0x8000), (m561.reg & 0x0F), TRUE, wr);
				memmap_auto_wp_8k(0, MMCPU(0xA000), (m561.reg >> 4), TRUE, wr);
				memmap_auto_wp_16k(0, MMCPU(0xC000), 0x07, TRUE, wr);
				return;
			case 7:
				memmap_auto_wp_8k(0, MMCPU(0x8000), (m561.reg & 0x0E), TRUE, wr);
				memmap_auto_wp_8k(0, MMCPU(0xA000), ((m561.reg >> 4) | 0x01), TRUE, wr);
				memmap_auto_wp_16k(0, MMCPU(0xC000), 0x07, TRUE, wr);
				return;
		}
	}
}
INLINE static void chr_fix_561(void) {
	memmap_vram_wp_8k(0, MMPPU(0x0000), m561.chr, TRUE, !(m561.reg1m >= 0x80));
}
INLINE static void mirroring_fix_561(void) {
	switch (m561.reg1m & 0x11) {
		case 0x00:
			mirroring_SCR0(0);
			return;
		case 0x01:
			mirroring_V(0);
			return;
		case 0x10:
			mirroring_SCR1(0);
			return;
		case 0x11:
			mirroring_H(0);
			return;
	}
}
