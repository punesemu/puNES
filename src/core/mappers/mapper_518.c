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
#include "ppu.h"
#include "save_slot.h"

// TODO : aggiungere l'emulazione del floppy disk controller

INLINE static void prg_fix_518(void);
INLINE static void wram_fix_518(void);
INLINE static void mirroring_fix_518(void);

_m518 m518;

void map_init_518(void) {
	EXTCL_AFTER_MAPPER_INIT(518);
	EXTCL_CPU_WR_MEM(518);
	EXTCL_CPU_RD_MEM(518);
	EXTCL_SAVE_MAPPER(518);
	EXTCL_RD_CHR(518);
	EXTCL_RD_NMT(518);
	EXTCL_CPU_EVERY_CYCLE(518);
	mapper.internal_struct[0] = (BYTE *)&m518;
	mapper.internal_struct_size[0] = sizeof(m518);

	memset(&m518, 0x00, sizeof(m518));

	info.mapper.extend_wr = TRUE;
}
void extcl_after_mapper_init_518(void) {
	prg_fix_518();
	wram_fix_518();
	mirroring_fix_518();
}
void extcl_cpu_wr_mem_518(UNUSED(BYTE nidx), WORD address, BYTE value) {
	switch (address & 0xFF00) {
		case 0x5000:
			m518.reg[0] = value;
			prg_fix_518();
			break;
		case 0x5200:
			m518.reg[1] = value;
			prg_fix_518();
			mirroring_fix_518();
			break;
		case 0x5300:
			m518.dac.out = value;
			m518.dac.status = 0x00;
			break;
	}
}
BYTE extcl_cpu_rd_mem_518(BYTE nidx, WORD address, BYTE openbus) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		return ((address & 0xFF00) == 0x5300 ? m518.dac.status : openbus);
	}
	return (wram_rd(nidx, address));
}
BYTE extcl_save_mapper_518(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m518.reg);
	save_slot_ele(mode, slot, m518.chr_bank);
	save_slot_ele(mode, slot, m518.dac.out);
	save_slot_ele(mode, slot, m518.dac.status);
	save_slot_ele(mode, slot, m518.dac.count);

	return (EXIT_OK);
}
BYTE extcl_rd_chr_518(BYTE nidx, WORD address) {
	return ((address < 0x1000) && (m518.reg[1] & 0x02)
		? chr_rd(nidx, (m518.chr_bank << 10) | (address & 0xFFF))
		: chr_rd(nidx, address));
}
BYTE extcl_rd_nmt_518(BYTE nidx, WORD address) {
	BYTE slot = (address & 0x0FFF) >> 10;

	m518.chr_bank = ((slot >> (m518.reg[1] & 0x01)) & 0x01) << 2;
	return (nmt_rd(nidx, address));
}
void extcl_cpu_every_cycle_518(UNUSED(BYTE nidx)) {
	m518.dac.count++;
	if (m518.dac.count == 1789772 / 11025) {
		m518.dac.count = 0;
		m518.dac.status = 0x80 | (m518.dac.out & 0x7F);
	}
}

INLINE static void prg_fix_518(void) {
	if (m518.reg[0] & 0x80) {
		if (m518.reg[1] & 0x04) {
			memmap_wram_32k(0, MMCPU(0x8000), (m518.reg[0] & 0x03));
		} else {
			memmap_wram_16k(0, MMCPU(0x8000), (m518.reg[0] & 0x07));
			memmap_auto_16k(0, MMCPU(0xC000), 0);
		}
	} else {
		if (m518.reg[1] & 0x04) {
			memmap_auto_32k(0, MMCPU(0x8000), m518.reg[0]);
		} else {
			memmap_auto_16k(0, MMCPU(0x8000), m518.reg[0]);
			memmap_auto_16k(0, MMCPU(0xC000), 0);
		}
	}
}
INLINE static void wram_fix_518(void) {
	memmap_auto_8k(0, MMCPU(0x6000), 16);
}
INLINE static void mirroring_fix_518(void) {
	if (m518.reg[1] & 0x01) {
		mirroring_H(0);
	} else {
		mirroring_V(0);
	}
}
