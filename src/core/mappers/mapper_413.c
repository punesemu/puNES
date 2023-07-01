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
#include "irqA12.h"
#include "save_slot.h"

INLINE static void prg_fix_413(void);
INLINE static void wram_fix_413(void);
INLINE static void chr_fix_413(void);

struct _m413 {
	BYTE reg[4];
	struct _m413_serial {
		BYTE control;
		uint32_t address;
	} serial;
} m413;

void map_init_413(void) {
	EXTCL_AFTER_MAPPER_INIT(413);
	EXTCL_CPU_WR_MEM(413);
	EXTCL_CPU_RD_MEM(413);
	EXTCL_SAVE_MAPPER(413);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	EXTCL_IRQ_A12_CLOCK(413);
	mapper.internal_struct[0] = (BYTE *)&m413;
	mapper.internal_struct_size[0] = sizeof(m413);
	mapper.internal_struct[1] = (BYTE *)&irqA12;
	mapper.internal_struct_size[1] = sizeof(irqA12);

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		memmap_prg_region_init(S4K);
	}

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m413, 0x00, sizeof(m413));

	info.mapper.extend_rd = TRUE;

	irqA12.present = TRUE;
}
void extcl_after_mapper_init_413(void) {
	prg_fix_413();
	wram_fix_413();
	chr_fix_413();
}
void extcl_cpu_wr_mem_413(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			irqA12.latch = value;
			break;
		case 0x9000:
			irqA12.counter = 0;
			break;
		case 0xA000:
		case 0xB000:
			irqA12.enable = (address & 0x1000) != 0;
			if (!irqA12.enable) {
				irq.high &= ~EXT_IRQ;
			}
			break;
		case 0xC000:
			m413.serial.address = (m413.serial.address << 1) | (value >> 7);
			break;
		case 0xD000:
			m413.serial.control = value;
			break;
		case 0xE000:
		case 0xF000:
			m413.reg[value >> 6] = value & 0x3F;
			prg_fix_413();
			wram_fix_413();
			chr_fix_413();
			break;
	}
}
BYTE extcl_cpu_rd_mem_413(WORD address, UNUSED(BYTE openbus)) {
	switch (address & 0xF800) {
		case 0x4800:
		case 0xC000:
		case 0xC800:
			if (m413.serial.control & 0x02) {
				return (miscrom_byte(m413.serial.address++ & (miscrom_size() - 1)));
			} else {
				return (miscrom_byte(m413.serial.address & (miscrom_size() - 1)));
			}
		default:
			return (address >= 0x8000 ? prgrom_rd(address) : wram_rd(address));
	}
}
BYTE extcl_save_mapper_413(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m413.reg);
	save_slot_ele(mode, slot, m413.serial.address);
	save_slot_ele(mode, slot, m413.serial.control);

	return (EXIT_OK);
}
void extcl_irq_A12_clock_413(void) {
	irqA12.counter = !irqA12.counter ? irqA12.latch : irqA12.counter - 1;
	if (!irqA12.counter && irqA12.enable) {
		irq.high |= EXT_IRQ;
	}
}

INLINE static void prg_fix_413(void) {
	memmap_auto_8k(MMCPU(0x8000), m413.reg[1]);
	memmap_auto_8k(MMCPU(0xA000), m413.reg[2]);
	memmap_disable_4k(MMCPU(0xC000));
	memmap_auto_4k(MMCPU(0xD000), 7);
	memmap_auto_8k(MMCPU(0xE000), 4);
}
INLINE static void wram_fix_413(void) {
	memmap_prgrom_4k(MMCPU(0x5000), 1);
	memmap_prgrom_8k(MMCPU(0x6000), m413.reg[0]);
}
INLINE static void chr_fix_413(void) {
	memmap_auto_4k(MMPPU(0x0000), m413.reg[3]);
	memmap_auto_4k(MMPPU(0x1000), 0xFD);
}
