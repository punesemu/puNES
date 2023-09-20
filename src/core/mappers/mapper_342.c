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

// Thx to guys of fceux
// https://github.com/ClusterM/fceux/blob/m342/src/boards/m342.cpp

#include <string.h>
#include <stdlib.h>
#include "mappers.h"
#include "cpu.h"
#include "ppu.h"
#include "tas.h"
#include "save_slot.h"
#include "gui.h"

enum _m342_misc {
	SAVE_FLASH_SIZE = S8M,
	CFI_SIZE = S32K
};

INLINE static void state_fix_342(void);
INLINE static void prg_fix_342(void);
INLINE static DBWORD prg_mapped_342(BYTE bank);
INLINE static BYTE prg_mapped_is_flash_342(DBWORD mapped);
INLINE static void prg_swap_8k_342(BYTE flash, WORD address, DBWORD value);
INLINE static void prg_swap_16k_342(BYTE flash, WORD address, DBWORD value);
INLINE static void prg_swap_32k_342(BYTE flash, DBWORD value);
INLINE static void chr_fix_342(void);
INLINE static void wram_fix_342(void);
INLINE static void mirroring_fix_342(void);
INLINE static void flash_write_342(WORD address, BYTE value);

INLINE static void mapper00_cpu_wr_low(WORD address, BYTE value);
INLINE static void mapper06_cpu_wr_low(WORD address, BYTE value);
INLINE static void mapper12_cpu_wr_low(WORD address, BYTE value);
INLINE static void mapper15_cpu_wr_low(WORD address, BYTE value);
INLINE static void mapper20_cpu_wr_low(WORD address, BYTE value);
INLINE static void mapper27_cpu_wr_low(WORD address, BYTE value);
INLINE static void mapper28_cpu_wr_low(WORD address, BYTE value);
INLINE static void mapper31_cpu_wr_low(WORD address, BYTE value);
INLINE static void mapper32_cpu_wr_low(WORD address, BYTE value);

INLINE static void mapper01_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper02_cpu_wr_high(BYTE value);
INLINE static void mapper03_cpu_wr_high(BYTE value);
INLINE static void mapper04_cpu_wr_high(BYTE value);
INLINE static void mapper05_cpu_wr_high(BYTE value);
INLINE static void mapper07_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper08_cpu_wr_high(BYTE value);
INLINE static void mapper09_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper10_cpu_wr_high(BYTE value);
INLINE static void mapper11_cpu_wr_high(BYTE value);
INLINE static void mapper13_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper14_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper16_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper17_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper18_cpu_wr_high(BYTE value);
INLINE static void mapper19_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper20_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper21_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper22_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper23_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper24_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper25_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper26_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper29_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper30_cpu_wr_high(BYTE value);
INLINE static void mapper34_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper35_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper36_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper37_cpu_wr_high(BYTE value);

INLINE static BYTE mapper06_cpu_rd(WORD address, BYTE openbus);
INLINE static BYTE mapper13_cpu_rd(WORD address, BYTE openbus);
INLINE static BYTE mapper15_cpu_rd(WORD address, BYTE openbus);
INLINE static BYTE mapper35_cpu_rd(void);

INLINE static void mapper07_cpu_every_cycle(void);
INLINE static void mapper14_cpu_every_cycle(void);
INLINE static void mapper19_cpu_every_cycle(void);
INLINE static void mapper23_cpu_every_cycle(void);
INLINE static void mapper24_cpu_every_cycle(void);
INLINE static void mapper25_cpu_every_cycle(void);
INLINE static void mapper35_cpu_every_cycle(void);
INLINE static void mapper36_cpu_every_cycle(void);

const BYTE cfi_data[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x51, 0x51, 0x52, 0x52, 0x59, 0x59, 0x02, 0x02, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x27, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06,
	0x06, 0x06, 0x09, 0x09, 0x13, 0x13, 0x03, 0x03, 0x05, 0x05, 0x03, 0x03, 0x02, 0x02, 0x1E, 0x1E,
	0x02, 0x02, 0x00, 0x00, 0x06, 0x06, 0x00, 0x00, 0x01, 0x01, 0xFF, 0xFF, 0x03, 0x03, 0x00, 0x00,
	0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x50, 0x50, 0x52, 0x52, 0x49, 0x49, 0x31, 0x31, 0x33, 0x33, 0x14, 0x14, 0x02, 0x02, 0x01, 0x01,
	0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0xB5, 0xB5, 0xC5, 0xC5, 0x05, 0x05,
	0x01, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
struct _m342 {
	BYTE mapper;
	BYTE fscreen;
	BYTE mirroring;
	BYTE flags;
	BYTE lock;
	BYTE cfi_mode;
	BYTE map_rom_on_6000;

	struct _m342_wram {
		BYTE enabled;
		BYTE page;
	} wram;
	struct _m342_flash {
		BYTE can_write;
		BYTE state;
		WORD buffer_a[10];
		BYTE buffer_v[10];
	} flash;
	struct _m342_prg {
		WORD b6000, a, b, c, d;
		DBWORD b6000_mapped;
		DBWORD a_mapped;
		DBWORD b_mapped;
		DBWORD c_mapped;
		DBWORD d_mapped;
		BYTE mode;
		WORD base;
		BYTE mask;
	} prg;
	struct _m342_chr {
		WORD a, b, c, d, e, f, g, h;
		BYTE can_write;
		BYTE mode;
		BYTE mask;
	} chr;
	// for MMC1
	struct _m342_mmc1 {
		//static uint64 lreset = 0;
		BYTE load_register;
	} mmc1;
	// for MMC2/MMC4
	struct _m342_mmc24 {
		BYTE latch0;
		BYTE latch1;
	} mmc24;
	// for MMC3
	struct _m342_mmc3 {
		BYTE internal;
		// for MMC3 scanline-based interrupts, counts A12 rises after long A12 falls
		struct _m342_mmc3_irq {
			BYTE enabled;
			BYTE counter;
			BYTE latch;
			BYTE reload;
		} irq;
	} mmc3;
	// for MMC5
	struct _m342_mmc5 {
		// for MMC5 scanline-based interrupts, counts dummy PPU reads
		struct _m342_mmc5_irq {
			BYTE enabled;
			BYTE line;
			BYTE out;
		} irq;
		struct _m342_mmc5_ppu {
			BYTE rendering;
			WORD scanline;
		} ppu;
	} mmc5;
	// for VRC3
	struct _m342_vrc3 {
		// for VRC3 CPU-based interrupts
		struct _m342_vrc3_irq {
			WORD value;
			BYTE control;
			WORD latch;
		} irq;
	} vrc3;
	// for VRC4
	struct _m342_vrc4 {
		// for VRC4 CPU-based interrupts
		struct _m342_vrc4_irq {
			BYTE value;
			BYTE control;
			BYTE latch;
			BYTE prescaler;
			BYTE prescaler_counter;
		} irq;
	} vrc4;
	// for mapper #69 Sunsoft FME-7
	struct _m342_mapper69 {
		BYTE internal;
		struct _m342_mapper69_irq {
			BYTE enabled;
			BYTE counter_enabled;
			WORD value;
		} irq;
	} mapper69;
	// for mapper #112
	struct _m342_mapper112 {
		BYTE internal;
	} mapper112;
	// for mapper #163
	struct _m342_mapper163 {
		BYTE latch;
		BYTE r0;
		BYTE r1;
		BYTE r2;
		BYTE r3;
		BYTE r4;
		BYTE r5;
	} mapper163;
	// for mapper #83
	struct _m342_mapper83 {
		struct _m342_mapper83_irq {
			BYTE enabled_latch;
			BYTE enabled;
			WORD counter;
		} irq;
	} mapper83;
	// for mapper #90
	struct _m342_mapper90 {
		BYTE mul1;
		BYTE mul2;
		BYTE xor;
	} mapper90;
	// for mapper #18
	struct _m342_mapper18 {
		struct _m342_mapper18_irq {
				WORD value;
				BYTE control;
				WORD latch;
		} irq;
	} mapper18;
	// for mapper #65
	struct _m342_mapper65 {
		struct _m342_mapper65_irq {
			BYTE enabled;
			WORD value;
			WORD latch;
		} irq;
	} mapper65;
	// for mapper #42 (only Baby Mario)
	struct _m342_mapper42 {
		struct _m342_mapper42_irq {
			BYTE enabled;
			WORD value;
		} irq;
	} mapper42;
	// for mapper #67
	struct _m342_mapper67 {
		struct _m342_mapper67_irq {
			BYTE enabled;
			BYTE latch;
			WORD counter;
		} irq;
	} mapper67;
} m342;
struct _m342tmp {
	BYTE *save_flash;
	BYTE *cfi;
} m342tmp;

void map_init_342(void) {
	EXTCL_AFTER_MAPPER_INIT(342);
	EXTCL_MAPPER_QUIT(342);
	EXTCL_CPU_WR_MEM(342);
	EXTCL_CPU_RD_MEM(342);
	EXTCL_SAVE_MAPPER(342);
	EXTCL_BATTERY_IO(342);
	EXTCL_CPU_EVERY_CYCLE(342);
	EXTCL_PPU_000_TO_34X(342);
	EXTCL_AFTER_RD_CHR(342);
	EXTCL_UPDATE_R2006(342);
	mapper.internal_struct[0] = (BYTE *)&m342;
	mapper.internal_struct_size[0] = sizeof(m342);

	memset(&m342, 0x00, sizeof(m342));

	m342.prg.mask = 0xF8; // 11111000, 128KB
	m342.prg.a = 0;
	m342.prg.b = 1;
	m342.prg.c = 0xFE;
	m342.prg.d = 0xFF;

	m342.chr.a = 0;
	m342.chr.b = 1;
	m342.chr.c = 2;
	m342.chr.d = 3;
	m342.chr.e = 4;
	m342.chr.f = 5;
	m342.chr.g = 6;
	m342.chr.h = 7;

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		m342tmp.save_flash = (BYTE *)malloc(SAVE_FLASH_SIZE);
		memset(m342tmp.save_flash, 0x00, SAVE_FLASH_SIZE);

		m342tmp.cfi = (BYTE *)malloc(CFI_SIZE);
		memset(m342tmp.cfi, 0x00, CFI_SIZE);

		for (int i = 0; i < S32K; i += sizeof(cfi_data)) {
			memcpy(&m342tmp.cfi[i], cfi_data, sizeof(cfi_data));
		}
	}

	info.mapper.force_battery_io = TRUE;
	info.mapper.extend_wr = TRUE;
	info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_342(void) {
	state_fix_342();
}
void extcl_mapper_quit_342(void) {
	if (m342tmp.save_flash) {
		free(m342tmp.save_flash);
		m342tmp.save_flash = NULL;
	}
	if (m342tmp.cfi) {
		free(m342tmp.cfi);
		m342tmp.cfi = NULL;
	}
}
void extcl_cpu_wr_mem_342(WORD address, BYTE value) {
	if (address < 0x8000) {
		if ((address >= 0x5000) && (address < 0x5FFF) && !m342.lock) {
			mapper00_cpu_wr_low(address, value);
		}
		switch (m342.mapper) {
			case 6:
				mapper06_cpu_wr_low(address, value);
				break;
			case 12:
				mapper12_cpu_wr_low(address, value);
				break;
			case 13:
				// Mapper #90 - JY
				/*
				if (mapper == 0b001101)
				{
					switch (A)
					{
					case 0x5800: mul1 = V; break;
					case 0x5801: mul2 = V; break;
					}
				}
				*/
				break;
			case 15:
				mapper15_cpu_wr_low(address, value);
				break;
			case 20:
				mapper20_cpu_wr_low(address, value);
				break;
			case 27:
				mapper27_cpu_wr_low(address, value);
				break;
			case 28:
				mapper28_cpu_wr_low(address, value);
				break;
			case 31:
				mapper31_cpu_wr_low(address, value);
				break;
			case 32:
				mapper32_cpu_wr_low(address, value);
				break;
		}
		state_fix_342();
		return;
	}

	// writing flash
	if (m342.flash.can_write) {
		flash_write_342(address, value);
		return;
	}

	switch (m342.mapper) {
		case 1:
			mapper01_cpu_wr_high(address, value);
			break;
		case 2:
			mapper02_cpu_wr_high(value);
			break;
		case 3:
			mapper03_cpu_wr_high(value);
			break;
		case 4:
			mapper04_cpu_wr_high(value);
			break;
		case 5:
			mapper05_cpu_wr_high(value);
			break;
		case 7:
			mapper07_cpu_wr_high(address, value);
			break;
		case 8:
			mapper08_cpu_wr_high(value);
			break;
		case 9:
			mapper09_cpu_wr_high(address, value);
			break;
		case 10:
			mapper10_cpu_wr_high(value);
			break;
		case 11:
			mapper11_cpu_wr_high(value);
			break;
		case 13:
			mapper13_cpu_wr_high(address, value);
			break;
		case 14:
			mapper14_cpu_wr_high(address, value);
			break;
		case 16:
			mapper16_cpu_wr_high(address, value);
			break;
		case 17:
			mapper17_cpu_wr_high(address, value);
			break;
		case 18:
			mapper18_cpu_wr_high(value);
			break;
		case 19:
			mapper19_cpu_wr_high(address, value);
			break;
		case 20:
			mapper20_cpu_wr_high(address, value);
			break;
		case 21:
			mapper21_cpu_wr_high(address, value);
			break;
		case 22:
			mapper22_cpu_wr_high(address, value);
			break;
		case 23:
			mapper23_cpu_wr_high(address, value);
			break;
		case 24:
			mapper24_cpu_wr_high(address, value);
			break;
		case 25:
			mapper25_cpu_wr_high(address, value);
			break;
		case 26:
			mapper26_cpu_wr_high(address, value);
			break;
		case 29:
			mapper29_cpu_wr_high(address, value);
			break;
		case 30:
			mapper30_cpu_wr_high(value);
			break;
		case 34:
			mapper34_cpu_wr_high(address, value);
			break;
		case 35:
			mapper35_cpu_wr_high(address, value);
			break;
		case 36:
			mapper36_cpu_wr_high(address, value);
			break;
		case 37:
			mapper37_cpu_wr_high(value);
			break;
	}
	state_fix_342();
}
BYTE extcl_cpu_rd_mem_342(WORD address, BYTE openbus) {
	// Mapper #36 is assigned to TXC's PCB 01-22000-400
	if ((m342.mapper == 29) && ((address & 0xE100) == 0x4100)) {
		return ((m342.prg.a & 0x0C) << 2);
	}
	switch (address & 0xF000) {
		case 0x5000:
			switch (m342.mapper) {
				case 0:
					return (0);
				case 6:
					return (mapper06_cpu_rd(address, openbus));
				case 13:
					return (mapper13_cpu_rd(address, openbus));
				case 15:
					return (mapper15_cpu_rd(address, openbus));
				case 35:
					return (mapper35_cpu_rd());
			}
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (m342.cfi_mode) {
				return (m342tmp.cfi[address & 0x7FFF]);
			}
			return (prgrom_rd(address));
	}
	return (wram_rd(address));
}
BYTE extcl_save_mapper_342(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m342.mapper);
	save_slot_ele(mode, slot, m342.fscreen);
	save_slot_ele(mode, slot, m342.mirroring);
	save_slot_ele(mode, slot, m342.flags);
	save_slot_ele(mode, slot, m342.lock);
	save_slot_ele(mode, slot, m342.cfi_mode);
	save_slot_ele(mode, slot, m342.map_rom_on_6000);

	save_slot_ele(mode, slot, m342.wram.enabled);
	save_slot_ele(mode, slot, m342.wram.page);

	save_slot_ele(mode, slot, m342.flash.can_write);
	save_slot_ele(mode, slot, m342.flash.state);
	save_slot_ele(mode, slot, m342.flash.buffer_a);
	save_slot_ele(mode, slot, m342.flash.buffer_v);

	save_slot_ele(mode, slot, m342.prg.b6000);
	save_slot_ele(mode, slot, m342.prg.a);
	save_slot_ele(mode, slot, m342.prg.b);
	save_slot_ele(mode, slot, m342.prg.c);
	save_slot_ele(mode, slot, m342.prg.d);
	save_slot_ele(mode, slot, m342.prg.b6000_mapped);
	save_slot_ele(mode, slot, m342.prg.a_mapped);
	save_slot_ele(mode, slot, m342.prg.b_mapped);
	save_slot_ele(mode, slot, m342.prg.c_mapped);
	save_slot_ele(mode, slot, m342.prg.d_mapped);
	save_slot_ele(mode, slot, m342.prg.mode);
	save_slot_ele(mode, slot, m342.prg.base);
	save_slot_ele(mode, slot, m342.prg.mask);

	save_slot_ele(mode, slot, m342.chr.can_write);
	save_slot_ele(mode, slot, m342.chr.a);
	save_slot_ele(mode, slot, m342.chr.b);
	save_slot_ele(mode, slot, m342.chr.c);
	save_slot_ele(mode, slot, m342.chr.d);
	save_slot_ele(mode, slot, m342.chr.e);
	save_slot_ele(mode, slot, m342.chr.f);
	save_slot_ele(mode, slot, m342.chr.g);
	save_slot_ele(mode, slot, m342.chr.h);
	save_slot_ele(mode, slot, m342.chr.mode);
	save_slot_ele(mode, slot, m342.chr.mask);

	save_slot_ele(mode, slot, m342.mmc1.load_register);

	save_slot_ele(mode, slot, m342.mmc24.latch0);
	save_slot_ele(mode, slot, m342.mmc24.latch1);

	save_slot_ele(mode, slot, m342.mmc3.internal);
	save_slot_ele(mode, slot, m342.mmc3.irq.enabled);
	save_slot_ele(mode, slot, m342.mmc3.irq.counter);
	save_slot_ele(mode, slot, m342.mmc3.irq.latch);
	save_slot_ele(mode, slot, m342.mmc3.irq.reload);

	save_slot_ele(mode, slot, m342.mmc5.irq.enabled);
	save_slot_ele(mode, slot, m342.mmc5.irq.line);
	save_slot_ele(mode, slot, m342.mmc5.irq.out);
	save_slot_ele(mode, slot, m342.mmc5.ppu.rendering);
	save_slot_ele(mode, slot, m342.mmc5.ppu.scanline);

	save_slot_ele(mode, slot, m342.vrc3.irq.value);
	save_slot_ele(mode, slot, m342.vrc3.irq.control);
	save_slot_ele(mode, slot, m342.vrc3.irq.latch);

	save_slot_ele(mode, slot, m342.vrc4.irq.value);
	save_slot_ele(mode, slot, m342.vrc4.irq.control);
	save_slot_ele(mode, slot, m342.vrc4.irq.latch);
	save_slot_ele(mode, slot, m342.vrc4.irq.prescaler);
	save_slot_ele(mode, slot, m342.vrc4.irq.prescaler_counter);

	save_slot_ele(mode, slot, m342.mapper69.internal);
	save_slot_ele(mode, slot, m342.mapper69.irq.enabled);
	save_slot_ele(mode, slot, m342.mapper69.irq.counter_enabled);
	save_slot_ele(mode, slot, m342.mapper69.irq.value);

	save_slot_ele(mode, slot, m342.mapper112.internal);

	save_slot_ele(mode, slot, m342.mapper163.latch);
	save_slot_ele(mode, slot, m342.mapper163.r0);
	save_slot_ele(mode, slot, m342.mapper163.r1);
	save_slot_ele(mode, slot, m342.mapper163.r2);
	save_slot_ele(mode, slot, m342.mapper163.r3);
	save_slot_ele(mode, slot, m342.mapper163.r4);
	save_slot_ele(mode, slot, m342.mapper163.r5);

	save_slot_ele(mode, slot, m342.mapper83.irq.enabled_latch);
	save_slot_ele(mode, slot, m342.mapper83.irq.enabled);
	save_slot_ele(mode, slot, m342.mapper83.irq.counter);

	save_slot_ele(mode, slot, m342.mapper90.mul1);
	save_slot_ele(mode, slot, m342.mapper90.mul2);
	save_slot_ele(mode, slot, m342.mapper90.xor);

	save_slot_ele(mode, slot, m342.mapper18.irq.value);
	save_slot_ele(mode, slot, m342.mapper18.irq.control);
	save_slot_ele(mode, slot, m342.mapper18.irq.latch);

	save_slot_ele(mode, slot, m342.mapper65.irq.enabled);
	save_slot_ele(mode, slot, m342.mapper65.irq.value);
	save_slot_ele(mode, slot, m342.mapper65.irq.latch);

	save_slot_ele(mode, slot, m342.mapper42.irq.enabled);
	save_slot_ele(mode, slot, m342.mapper42.irq.value);

	save_slot_ele(mode, slot, m342.mapper67.irq.enabled);
	save_slot_ele(mode, slot, m342.mapper67.irq.latch);
	save_slot_ele(mode, slot, m342.mapper67.irq.counter);

	return (EXIT_OK);
}
void extcl_battery_io_342(BYTE mode, FILE *fp) {
	if (mode == WR_BAT) {
		if (fwrite(m342tmp.save_flash, SAVE_FLASH_SIZE, 1, fp) < 1) {
			log_error(uL("342;error on write flash chip"));
		}
	} else {
		if (fread(m342tmp.save_flash, SAVE_FLASH_SIZE, 1, fp) < 1) {
			log_error(uL("342;error on read flash chip"));
		}
	}
}
void extcl_cpu_every_cycle_342(void) {
	switch (m342.mapper) {
		case 07:
			mapper07_cpu_every_cycle();
			break;
		case 14:
			mapper14_cpu_every_cycle();
			break;
		case 19:
			mapper19_cpu_every_cycle();
			break;
		case 24:
			mapper24_cpu_every_cycle();
			break;
		case 23:
			mapper23_cpu_every_cycle();
			break;
		case 25:
			mapper25_cpu_every_cycle();
			break;
		case 35:
			mapper35_cpu_every_cycle();
			break;
		case 36:
			mapper36_cpu_every_cycle();
			break;
	}
}
void extcl_ppu_000_to_34x_342(void) {
	m342.mmc5.ppu.rendering = !ppudata.ppu.vblank && ppudata.r2001.visible && (ppudata.ppu.screen_y < SCR_ROWS);
	m342.mmc5.ppu.scanline = ppudata.ppu.screen_y;

	if ((ppudata.ppu.frame_x == 260) && m342.mmc5.ppu.rendering) {
		// for MMC3 and MMC3-based
		if (m342.mmc3.irq.reload || !m342.mmc3.irq.counter) {
			m342.mmc3.irq.counter = m342.mmc3.irq.latch;
			m342.mmc3.irq.reload = 0;
		} else {
			m342.mmc3.irq.counter--;
		}
		if (!m342.mmc3.irq.counter && m342.mmc3.irq.enabled) {
			cpudata.irq.high |= EXT_IRQ;
		}

		// for MMC5
		if ((m342.mmc5.irq.line == (ppudata.ppu.screen_y + 1)) && m342.mmc5.irq.enabled) {
			m342.mmc5.irq.out = 1;
			cpudata.irq.high |= EXT_IRQ;
		}

		// for mapper #163
		if (m342.mapper == 6) {
			if (ppudata.ppu.screen_y == 239) {
				m342.mapper163.latch = 0;
				chr_fix_342();
			} else if (ppudata.ppu.screen_y == 127) {
				m342.mapper163.latch = 1;
				chr_fix_342();
			}
		}
	}
}
void extcl_after_rd_chr_342(WORD address) {
	if (m342.mapper == 17) {
		switch (address & 0xFFF0) {
			case 0x0FD0:
				m342.mmc24.latch0 = 0;
				chr_fix_342();
				break;
			case 0x0FE0:
				m342.mmc24.latch0 = 1;
				chr_fix_342();
				break;
			case 0x1FD0:
				m342.mmc24.latch1 = 0;
				chr_fix_342();
				break;
			case 0x1FE0:
				m342.mmc24.latch1 = 1;
				chr_fix_342();
				break;
			default:
				return;
		}
	}
}
void extcl_update_r2006_342(WORD new_r2006, UNUSED(WORD old_r2006)) {
	extcl_after_rd_chr_342(new_r2006);
}

INLINE static void state_fix_342(void) {
	prg_fix_342();
	chr_fix_342();
	wram_fix_342();
	mirroring_fix_342();
}
INLINE static void prg_fix_342(void) {
	BYTE a_is_flash = FALSE, b_is_flash = FALSE, c_is_flash = FALSE, d_is_flash = FALSE;
	DBWORD bank = 0;

	m342.prg.a_mapped = prg_mapped_342(m342.prg.a);
	m342.prg.b_mapped = prg_mapped_342(m342.prg.b);
	m342.prg.c_mapped = prg_mapped_342(m342.prg.c);
	m342.prg.d_mapped = prg_mapped_342(m342.prg.d);
	a_is_flash = prg_mapped_is_flash_342(m342.prg.a_mapped);
	b_is_flash = prg_mapped_is_flash_342(m342.prg.b_mapped);
	c_is_flash = prg_mapped_is_flash_342(m342.prg.c_mapped);
	d_is_flash = prg_mapped_is_flash_342(m342.prg.d_mapped);

	if (!m342.cfi_mode) {
		switch (m342.prg.mode & 0x07) {
			default:
			case 0:
				bank = m342.prg.a_mapped >> 1;
				prg_swap_16k_342(a_is_flash, 0x8000, bank);
				bank = m342.prg.c_mapped >> 1;
				prg_swap_16k_342(c_is_flash, 0xC000, bank);
				return;
			case 1:
				bank = m342.prg.c_mapped >> 1;
				prg_swap_16k_342(c_is_flash, 0x8000, bank);
				bank = m342.prg.a_mapped >> 1;
				prg_swap_16k_342(a_is_flash, 0xC000, bank);
				return;
			case 4:
				bank = m342.prg.a_mapped;
				prg_swap_8k_342(a_is_flash, 0x8000, bank);
				bank = m342.prg.b_mapped;
				prg_swap_8k_342(b_is_flash, 0xA000, bank);
				bank = m342.prg.c_mapped;
				prg_swap_8k_342(c_is_flash, 0xC000, bank);
				bank = m342.prg.d_mapped;
				prg_swap_8k_342(d_is_flash, 0xE000, bank);
				return;
			case 5:
				bank = m342.prg.c_mapped;
				prg_swap_8k_342(c_is_flash, 0x8000, bank);
				bank = m342.prg.b_mapped;
				prg_swap_8k_342(b_is_flash, 0xA000, bank);
				bank = m342.prg.a_mapped;
				prg_swap_8k_342(a_is_flash, 0xC000, bank);
				bank = m342.prg.d_mapped;
				prg_swap_8k_342(d_is_flash, 0xE000, bank);
				return;
			case 6:
				bank = m342.prg.b_mapped >> 2;
				prg_swap_32k_342(b_is_flash, bank);
				return;
			case 7:
				bank = m342.prg.a_mapped >> 2;
				prg_swap_32k_342(a_is_flash, bank);
				return;
		}
	}
}
INLINE static DBWORD prg_mapped_342(BYTE bank) {
	return ((m342.prg.base << 1) | (bank & (((~m342.prg.mask & 0x7F) << 1) | 1)));
}
INLINE static BYTE prg_mapped_is_flash_342(DBWORD mapped) {
	return (mapped >= 0x20000 - SAVE_FLASH_SIZE / 1024 / 8);
}
INLINE static void prg_swap_8k_342(BYTE flash, WORD address, DBWORD value) {
	if (flash) {
		memmap_other_8k(MMCPU(address), value, m342tmp.save_flash, SAVE_FLASH_SIZE, TRUE, FALSE);
	} else {
		memmap_auto_8k(MMCPU(address), value);
	}
}
INLINE static void prg_swap_16k_342(BYTE flash, WORD address, DBWORD value) {
	if (flash) {
		memmap_other_16k(MMCPU(address), value, m342tmp.save_flash, SAVE_FLASH_SIZE, TRUE, FALSE);
	} else {
		memmap_auto_16k(MMCPU(address), value);
	}
}
INLINE static void prg_swap_32k_342(BYTE flash, DBWORD value) {
	if (flash) {
		memmap_other_32k(MMCPU(0x8000), value, m342tmp.save_flash, SAVE_FLASH_SIZE, TRUE, FALSE);
	} else {
		memmap_auto_32k(MMCPU(0x8000), value);
	}
}
INLINE static void chr_fix_342(void) {
	BYTE chr_shift = (m342.mapper == 24) && (m342.flags & 0x02);
	DBWORD mask = (((((~m342.chr.mask & 0x1F) + 1) * 0x2000) / 0x400) - 1) >> chr_shift;
	DBWORD bank[8];

	switch (m342.chr.mode & 0x07) {
		default:
		case 0:
			bank[0] = ((m342.chr.a >> 3 >> chr_shift) & (mask >> 3)) << 3;
			bank[1] = bank[0] | 1;
			bank[2] = bank[0] | 2;
			bank[3] = bank[0] | 3;
			bank[4] = bank[0] | 4;
			bank[5] = bank[0] | 5;
			bank[6] = bank[0] | 6;
			bank[7] = bank[0] | 7;
			break;
		case 1:
			bank[0] = ((m342.mapper163.latch >> 2 >> chr_shift) & (mask >> 2)) << 2;
			bank[4] = bank[0];
			bank[1] = bank[5] = bank[0] | 1;
			bank[2] = bank[6] = bank[0] | 2;
			bank[3] = bank[7] = bank[0] | 3;
			break;
		case 2:
			bank[0] = ((m342.chr.a >> 1 >> chr_shift) & (mask >> 1)) << 1;
			bank[1] = bank[0] | 1;

			bank[2] = ((m342.chr.c >> 1 >> chr_shift) & (mask >> 1)) << 1;
			bank[3] = bank[2] | 1;

			bank[4] = (m342.chr.e >> chr_shift) & mask;
			bank[5] = (m342.chr.f >> chr_shift) & mask;
			bank[6] = (m342.chr.g >> chr_shift) & mask;
			bank[7] = (m342.chr.h >> chr_shift) & mask;
			break;
		case 3:
			bank[0] = (m342.chr.e >> chr_shift) & mask;
			bank[1] = (m342.chr.f >> chr_shift) & mask;
			bank[2] = (m342.chr.g >> chr_shift) & mask;
			bank[3] = (m342.chr.h >> chr_shift) & mask;

			bank[4] = ((m342.chr.a >> 1 >> chr_shift) & (mask >> 1)) << 1;
			bank[5] = bank[4] | 1;

			bank[6] = ((m342.chr.c >> 1 >> chr_shift) & (mask >> 1)) << 1;
			bank[7] = bank[6] | 1;
			break;
		case 4:
			bank[0] = ((m342.chr.a >> 2 >> chr_shift) & (mask >> 2)) << 2;
			bank[1] = bank[0] | 1;
			bank[2] = bank[0] | 2;
			bank[3] = bank[0] | 3;

			bank[4] = ((m342.chr.e >> 2 >> chr_shift) & (mask >> 2)) << 2;
			bank[5] = bank[4] | 1;
			bank[6] = bank[4] | 2;
			bank[7] = bank[4] | 3;
			break;
		case 5:
			if (!m342.mmc24.latch0) {
				bank[0] = ((m342.chr.a >> 2 >> chr_shift) & (mask >> 2)) << 2;
				bank[1] = bank[0] | 1;
				bank[2] = bank[0] | 2;
				bank[3] = bank[0] | 3;
			} else {
				bank[0] = ((m342.chr.b >> 2 >> chr_shift) & (mask >> 2)) << 2;
				bank[1] = bank[0] | 1;
				bank[2] = bank[0] | 2;
				bank[3] = bank[0] | 3;
			}
			if (!m342.mmc24.latch1) {
				bank[4] = ((m342.chr.e >> 2 >> chr_shift) & (mask >> 2)) << 2;
				bank[5] = bank[4] | 1;
				bank[6] = bank[4] | 2;
				bank[7] = bank[4] | 3;
			} else {
				bank[4] = ((m342.chr.f >> 2 >> chr_shift) & (mask >> 2)) << 2;
				bank[5] = bank[4] | 1;
				bank[6] = bank[4] | 2;
				bank[7] = bank[4] | 3;
			}
			break;
		case 6:
			bank[0] = ((m342.chr.a >> 1 >> chr_shift) & (mask >> 1)) << 1;
			bank[1] = bank[0] | 1;

			bank[2] = ((m342.chr.c >> 1 >> chr_shift) & (mask >> 1)) << 1;
			bank[3] = bank[2] | 1;

			bank[4] = ((m342.chr.e >> 1 >> chr_shift) & (mask >> 1)) << 1;
			bank[5] = bank[4] | 1;

			bank[6] = ((m342.chr.g >> 1 >> chr_shift) & (mask >> 1)) << 1;
			bank[7] = bank[6] | 1;
			break;
		case 7:
			bank[0] = (m342.chr.a >> chr_shift) & mask;
			bank[1] = (m342.chr.b >> chr_shift) & mask;
			bank[2] = (m342.chr.c >> chr_shift) & mask;
			bank[3] = (m342.chr.d >> chr_shift) & mask;
			bank[4] = (m342.chr.e >> chr_shift) & mask;
			bank[5] = (m342.chr.f >> chr_shift) & mask;
			bank[6] = (m342.chr.g >> chr_shift) & mask;
			bank[7] = (m342.chr.h >> chr_shift) & mask;
			break;
	}
	memmap_vram_wp_1k(MMPPU(0x0000), bank[0], TRUE, m342.chr.can_write);
	memmap_vram_wp_1k(MMPPU(0x0400), bank[1], TRUE, m342.chr.can_write);
	memmap_vram_wp_1k(MMPPU(0x0800), bank[2], TRUE, m342.chr.can_write);
	memmap_vram_wp_1k(MMPPU(0x0C00), bank[3], TRUE, m342.chr.can_write);
	memmap_vram_wp_1k(MMPPU(0x1000), bank[4], TRUE, m342.chr.can_write);
	memmap_vram_wp_1k(MMPPU(0x1400), bank[5], TRUE, m342.chr.can_write);
	memmap_vram_wp_1k(MMPPU(0x1800), bank[6], TRUE, m342.chr.can_write);
	memmap_vram_wp_1k(MMPPU(0x1C00), bank[7], TRUE, m342.chr.can_write);
}
INLINE static void wram_fix_342(void) {
	m342.prg.b6000_mapped = prg_mapped_342(m342.prg.b6000);

	if (m342.map_rom_on_6000) {
		memmap_prgrom_8k(MMCPU(0x6000), m342.prg.b6000_mapped);
	} else if (m342.wram.enabled) {
		memmap_auto_8k(MMCPU(0x6000), m342.wram.page);
	}
}

INLINE static void mirroring_fix_342(void) {
	if (m342.fscreen) {
		mirroring_FSCR();
	} else  {
		// Mapper #189?
		if (!((m342.mapper == 20) && (m342.flags & 0x01))) {
			switch (m342.mirroring) {
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
	}
}
INLINE static void flash_write_342(WORD address, BYTE value) {
	if (m342.flash.state < 10) {
		m342.flash.buffer_a[m342.flash.state] = address & 0xFFF;
		m342.flash.buffer_v[m342.flash.state] = value;
		m342.flash.state++;

		// sector erase
		if ((m342.flash.state == 6) &&
			(m342.flash.buffer_a[0] == 0x0AAA) && (m342.flash.buffer_v[0] == 0xAA) &&
			(m342.flash.buffer_a[1] == 0x0555) && (m342.flash.buffer_v[1] == 0x55) &&
			(m342.flash.buffer_a[2] == 0x0AAA) && (m342.flash.buffer_v[2] == 0x80) &&
			(m342.flash.buffer_a[3] == 0x0AAA) && (m342.flash.buffer_v[3] == 0xAA) &&
			(m342.flash.buffer_a[4] == 0x0555) && (m342.flash.buffer_v[4] == 0x55) &&
			(m342.flash.buffer_v[5] == 0x30)) {
			DBWORD sector_address = m342.prg.a_mapped * 0x2000;

			for (DBWORD i = sector_address; i < sector_address + 128 * 1024; i++) {
				m342tmp.save_flash[i % SAVE_FLASH_SIZE] = 0xFF;
			}
			//printf("Flash sector erased: %08x - %08x\n", sector_address, sector_address + 128 * 1024 - 1);
			m342.flash.state = 0;
		}

		// writing byte
		if ((m342.flash.state == 4) &&
			(m342.flash.buffer_a[0] == 0x0AAA) && (m342.flash.buffer_v[0] == 0xAA) &&
			(m342.flash.buffer_a[1] == 0x0555) && (m342.flash.buffer_v[1] == 0x55) &&
			(m342.flash.buffer_a[2] == 0x0AAA) && (m342.flash.buffer_v[2] == 0xA0)) {
			DBWORD sector_address = m342.prg.a_mapped * 0x2000;
			DBWORD flash_addr = sector_address + (address % 0x8000);

			if (m342tmp.save_flash[flash_addr % SAVE_FLASH_SIZE] != 0xFF) {
				//printf("Error flash sector is not erased: %08x\n", flash_addr);
			}
			m342tmp.save_flash[flash_addr % SAVE_FLASH_SIZE] = value;

			if (address % 0x2000 == 0) {
				//printf("Flash sector writed: %08x\n", flash_addr);
			}
			m342.flash.state = 0;
		}

		// enter CFI mode
		if ((m342.flash.state == 1) &&
			(m342.flash.buffer_a[0] == 0x0AAA) && (m342.flash.buffer_v[0] == 0x98)) {
			m342.cfi_mode = 1;
			prg_fix_342();
			m342.flash.state = 0;
			return;
		}
	}
	if (value == 0xF0) {
		m342.flash.state = 0;
		m342.cfi_mode = 0;
		prg_fix_342();
	}
}

INLINE static void mapper00_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #342 - 342
	switch (address & 0x0007) {
		case 0:
			m342.prg.base = (m342.prg.base & 0xFF) | (value << 8);
			break;
		case 1:
			m342.prg.base = (m342.prg.base & 0xFF00) | value;
			break;
		case 2:
			m342.chr.mask = (m342.chr.mask & 0x1F) | ((value & 0x80) >> 2);
			m342.prg.mask = value & 0x7F;
			break;
		case 3:
			m342.prg.mode = value >> 5;
			m342.chr.a = (m342.chr.a & 0x07) | (value << 3);
			break;
		case 4:
			m342.chr.mode = value >> 5;
			m342.chr.mask = (m342.chr.mask & 0x20) | (value & 0x1F);
			break;
		case 5:
			m342.chr.a = (m342.chr.a & 0xFF) | ((value & 0x80) << 1);
			m342.prg.a = (m342.prg.a & 0xC1) | ((value & 0x7C) >> 1);
			m342.wram.page = value & 0x03;
			break;
		case 6:
			m342.flags = value >> 5;
			m342.mapper = (m342.mapper & 0x20) | (value & 0x1F);
			break;
		case 7:
			m342.lock = value >> 7;
			m342.mapper = (m342.mapper & 0x1F) | ((value & 0x40) >> 1);
			m342.fscreen = (value & 0x20) >> 5;
			m342.mirroring = (value & 0x18) >> 3;
			m342.flash.can_write = (value & 0x04) >> 2;
			m342.chr.can_write = (value & 0x02) >> 1;
			m342.wram.enabled = value & 0x01;
			switch (m342.mapper) {
				case 14:
					// Mapper #65 - Irem's H3001
					m342.prg.b = 1;
					break;
				case 17:
					// MMC2
					m342.prg.b = 0xFD;
					break;
				case 23:
					// Mapper #42
					m342.map_rom_on_6000 = 1;
					break;
			}
			break;
	}
}
INLINE static void mapper06_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #163
	if (address == 0x5101) {
		if (m342.mapper163.r4 && !value) {
			m342.mapper163.r5 ^= 1;
		}
		m342.mapper163.r4 = value;
	} else if ((address == 0x5100) && (value == 0x06)) {
		m342.prg.mode = m342.prg.mode & 0xFE;
		m342.prg.b = 12;
	} else {
		if ((address & 0x7000) == 0x5000) {
			switch ((address & 0x0300) >> 8) {
				case 2:
					m342.prg.mode |= 1;
					m342.prg.a = (m342.prg.a & 0x3F) | ((value & 0x03) << 6);
					m342.mapper163.r0 = value;
					break;
				case 0:
					m342.prg.mode |= 1;
					m342.prg.a = (m342.prg.a & 0xC3) | ((value & 0x0F) << 2);
					m342.chr.mode = (m342.chr.mode & 0xFE) | (value >> 7);
					m342.mapper163.r1 = value;
					break;
				case 3:
					m342.mapper163.r2 = value;
					break;
				case 1:
					m342.mapper163.r3 = value;
					break;
			}
		}
	}
}
INLINE static void mapper12_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #87
	if ((address & 0x6000) == 0x6000) {
		m342.chr.a = (m342.chr.a & 0xE7) | ((value & 0x01) << 4) | ((value & 0x02) << 2);
	}
}
INLINE static void mapper15_cpu_wr_low(WORD address, BYTE value) {
	// MMC5 (not really)
	switch (address) {
		case 0x5105:
			if (value == 0xFF) {
				m342.fscreen = 1;
			} else {
				m342.fscreen = 0;
				switch (((value >> 2) & 0x01) | ((value >> 3) & 0x02)) {
					case 0:
						m342.mirroring = 2;
						break;
					case 1:
						m342.mirroring = 0;
						break;
					case 2:
						m342.mirroring = 1;
						break;
					case 3:
						m342.mirroring = 3;
						break;
				}
			}
			break;
		case 0x5115:
			m342.prg.a = (value & 0x1E);
			m342.prg.b = (value & 0x1E) | 1;
			break;
		case 0x5116:
			m342.prg.c = value & 0x1F;
			break;
		case 0x5117:
			m342.prg.d = value & 0x1F;
			break;
		case 0x5120:
			m342.chr.a = value;
			break;
		case 0x5121:
			m342.chr.b = value;
			break;
		case 0x5122:
			m342.chr.c = value;
			break;
		case 0x5123:
			m342.chr.d = value;
			break;
		case 0x5128:
			m342.chr.e = value;
			break;
		case 0x5129:
			m342.chr.f = value;
			break;
		case 0x512A:
			m342.chr.g = value;
			break;
		case 0x512B:
			m342.chr.h = value;
			break;
		case 0x5203:
			cpudata.irq.high &= ~EXT_IRQ;
			m342.mmc5.irq.out = 0;
			m342.mmc5.irq.line = value;
			break;
		case 0x5204:
			cpudata.irq.high &= ~EXT_IRQ;
			m342.mmc5.irq.out = 0;
			m342.mmc5.irq.enabled = (value & 0x80) >> 7;
			break;
		default:
			break;
	}
}
INLINE static void mapper20_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #189
	if ((m342.flags & 0x02) &&  (address >= 0x4120)) {
		m342.prg.a = (m342.prg.a & 0xC3) | ((value & 0x0F) << 2) | ((value & 0xF0) >> 2);
	}
}
INLINE static void mapper27_cpu_wr_low(WORD address, BYTE value) {
	// Mappers #79 and #146 - NINA-03/06 and Sachen 3015: (flag0 = 1)
	if ((address & 0x6100) == 0x4100) {
		m342.chr.a = (m342.chr.a & 0xC7) | ((value & 0x07) << 3);
		m342.prg.a = (m342.prg.a & 0xFB) | ((value & 0x08) >> 1);
	}
}
INLINE static void mapper28_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #133
	if ((address & 0x6100) == 0x4100) {
		m342.chr.a = (m342.chr.a & 0xE7) | ((value & 0x03) << 3);
		m342.prg.a = (m342.prg.a & 0xFB) | (value & 0x04);
	}
}
INLINE static void mapper31_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #184
	if ((address & 0x6000) == 0x6000) {
		m342.chr.a = (m342.chr.a & 0xE3) | ((value & 0x07) << 2);
		m342.chr.e = (m342.chr.e & 0xE3) | ((value & 0x30) >> 2) | 0x10;
	}
}
INLINE static void mapper32_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #38
	if ((address & 0x7000) == 0x7000) {
		m342.prg.a = (m342.prg.a & 0xF7) | ((value & 0x03) << 2);
		m342.chr.a = (m342.chr.a & 0xE7) | ((value & 0x0C) << 1);
	}
}

INLINE static void mapper01_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #2 - UxROM
	// flag0 - mapper #71 - for Fire Hawk only.
	// other mapper-#71 games are UxROM
	if (!(m342.flags & 0x01) || ((address & 0x7000) != 0x1000)) {
		m342.prg.a = (m342.prg.a & 0xC1) | ((value & 0x1F) << 1);
		if (m342.flags & 0x02) {
			m342.mirroring = 0x02 | (value >> 7);
			m342.chr.a = (m342.chr.a & 0xFC) | ((value & 0x60) >> 5);
		}
	} else {
		m342.mirroring = 0x02 | ((value >> 4) & 0x01);
	}
}
INLINE static void mapper02_cpu_wr_high(BYTE value) {
	// Mapper #3 - CNROM
	m342.chr.a = (m342.chr.a & 0x07) | ((value & 0x1F) << 3);
}
INLINE static void mapper03_cpu_wr_high(BYTE value) {
	// Mapper #78 - Holy Diver
	m342.prg.a = (m342.prg.a & 0xF1) | ((value & 0x07) << 1);
	m342.chr.a = (m342.chr.a & 0x87) | ((value & 0xF0) >> 1);
	m342.mirroring = ((value >> 3) & 0x01) ^ 1;
}
INLINE static void mapper04_cpu_wr_high(BYTE value) {
	// Mapper #97 - Irem's TAM-S1
	m342.prg.a = (m342.prg.a & 0xC1) | ((value & 0x1F) << 1);
	m342.mirroring = (value >> 7) ^ 0x01;
}
INLINE static void mapper05_cpu_wr_high(BYTE value) {
	// Mapper #93 - Sunsoft-2
	m342.prg.a = (m342.prg.a & 0xF1) | ((value & 0x70) >> 3);
	m342.chr.can_write = value & 0x01;
}
INLINE static void mapper07_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #18
	switch (((address & 0x7000) >> 10) | (address & 0x03)) {
		case 0:
			m342.prg.a = (m342.prg.a & 0xF0) | (value & 0x0F);
			break;
		case 1:
			m342.prg.a = (m342.prg.a & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 2:
			m342.prg.b = (m342.prg.b & 0xF0) | (value & 0x0F);
			break;
		case 3:
			m342.prg.b = (m342.prg.b & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 4:
			m342.prg.c = (m342.prg.c & 0xF0) | (value & 0x0F);
			break;
		case 5:
			m342.prg.c = (m342.prg.c & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			m342.chr.a = (m342.chr.a & 0xF0) | (value & 0x0F);
			break;
		case 9:
			m342.chr.a = (m342.chr.a & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 10:
			m342.chr.b = (m342.chr.b & 0xF0) | (value & 0x0F);
			break;
		case 11:
			m342.chr.b = (m342.chr.b & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 12:
			m342.chr.c = (m342.chr.c & 0xF0) | (value & 0x0F);
			break;
		case 13:
			m342.chr.c = (m342.chr.c & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 14:
			m342.chr.d = (m342.chr.d & 0xF0) | (value & 0x0F);
			break;
		case 15:
			m342.chr.d = (m342.chr.d & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 16:
			m342.chr.e = (m342.chr.e & 0xF0) | (value & 0x0F);
			break;
		case 17:
			m342.chr.e = (m342.chr.e & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 18:
			m342.chr.f = (m342.chr.f & 0xF0) | (value & 0x0F);
			break;
		case 19:
			m342.chr.f = (m342.chr.f & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 20:
			m342.chr.g = (m342.chr.g & 0xF0) | (value & 0x0F);
			break;
		case 21:
			m342.chr.g = (m342.chr.g & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 22:
			m342.chr.h = (m342.chr.h & 0xF0) | (value & 0x0F);
			break;
		case 23:
			m342.chr.h = (m342.chr.h & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 24:
			m342.mapper18.irq.latch = (m342.mapper18.irq.latch & 0xFFF0) | (value & 0x0F);
			break;
		case 25:
			m342.mapper18.irq.latch = (m342.mapper18.irq.latch & 0xFF0F) | ((value & 0x0F) << 4);
			break;
		case 26:
			m342.mapper18.irq.latch = (m342.mapper18.irq.latch & 0xF0FF) | ((value & 0x0F) << 8);
			break;
		case 27:
			m342.mapper18.irq.latch = (m342.mapper18.irq.latch & 0x0FFF) | ((value & 0x0F) << 12);
			break;
		case 28:
			m342.mapper18.irq.value = m342.mapper18.irq.latch;
			cpudata.irq.high &= ~EXT_IRQ;
			break;
		case 29:
			m342.mapper18.irq.control = value & 0x0F;
			cpudata.irq.high &= ~EXT_IRQ;
			break;
		case 30:
			switch (value & 0x03) {
				case 0:
					m342.mirroring = 1;
					break;
				case 1:
					m342.mirroring = 0;
					break;
				case 2:
					m342.mirroring = 2;
					break;
				case 3:
					m342.mirroring = 3;
					break;
			}
			break;
		case 31:
			break;
	}
}
INLINE static void mapper08_cpu_wr_high(BYTE value) {
	// Mapper #7 - AxROM, mapper #241 - BNROM
	m342.prg.a = (m342.prg.a & 0xC3) | ((value & 0xF) << 2);
	if (!(m342.flags & 0x01)) {
		m342.mirroring = 0x02 | ((value >> 4) & 0x01);
	}
}
INLINE static void mapper09_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #228 - Cheetahmen II
	m342.prg.a = (m342.prg.a & 0xC3) | ((address & 0x0780) >> 5);
	m342.chr.a = (m342.chr.a & 0x07) | ((address & 0x0007) << 5) | ((value & 0x03) << 3);
	m342.mirroring = (address >> 13) & 0x01;
}
INLINE static void mapper10_cpu_wr_high(BYTE value) {
	// Mapper #11 - ColorDreams
	m342.prg.a = (m342.prg.a & 0xF3) | ((value & 0x03) << 2);
	m342.chr.a = (m342.chr.a & 0x87) | ((value & 0xF0) >> 1);
}
INLINE static void mapper11_cpu_wr_high(BYTE value) {
	// Mapper #66 - GxROM
	m342.prg.a = (m342.prg.a & 0xF3) | ((value & 0x30) >> 2);
	m342.chr.a = (m342.chr.a & 0xE7) | ((value & 0x03) << 3);
}
INLINE static void mapper13_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #90 - JY
	switch (address & 0xF000) {
		case 0x8000:
			switch (address & 0x0003) {
				case 0:
					m342.prg.a = (m342.prg.a & 0xC0) | (value & 0x3F);
					break;
				case 1:
					m342.prg.b = (m342.prg.b & 0xC0) | (value & 0x3F);
					break;
				case 2:
					m342.prg.c = (m342.prg.c & 0xC0) | (value & 0x3F);
					break;
				case 3:
					m342.prg.d = (m342.prg.d & 0xC0) | (value & 0x3F);
					break;
			}
			break;
		case 0x9000:
			switch (address & 0x0007) {
				case 0:
					m342.chr.a = value;
					break;
				case 1:
					m342.chr.b = value;
					break;
				case 2:
					m342.chr.c = value;
					break;
				case 3:
					m342.chr.d = value;
					break;
				case 4:
					m342.chr.e = value;
					break;
				case 5:
					m342.chr.f = value;
					break;
				case 6:
					m342.chr.g = value;
					break;
				case 7:
					m342.chr.h = value;
					break;
			}
			break;
		case 0xC000:
			switch (address & 0x0007) {
				case 0:
					if (value & 0x01) {
						m342.mmc3.irq.enabled = 1;
					} else {
						m342.mmc3.irq.enabled = 0;
						cpudata.irq.high &= ~EXT_IRQ;
					}
					break;
				case 1:
					break;
				case 2:
					m342.mmc3.irq.enabled = 0;
					cpudata.irq.high &= ~EXT_IRQ;
					break;
				case 3:
					m342.mmc3.irq.enabled = 1;
					break;
				case 4:
					break;
				case 5:
					m342.mmc3.irq.latch = value ^ m342.mapper90.xor;
					m342.mmc3.irq.reload = 1;
					break;
				case 6:
					m342.mapper90.xor = value;
					break;
				case 7:
					break;
			}
			break;
		case 0xD000:
			if ((address & 0x0003) == 0x0001) {
				m342.mirroring = value & 0x03;
			}
			break;
	}

}
INLINE static void mapper14_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #65 - Irem's H3001
	switch (((address & 0x7000) >> 9) | (address & 0x0007)) {
		case 0:
			m342.prg.a = (m342.prg.a & 0xC0) | (value & 0x3F);
			break;
		case 9:
			m342.mirroring = (value >> 7) & 0x01;
			break;
		case 11:
			m342.mapper65.irq.enabled = value >> 7;
			cpudata.irq.high &= ~EXT_IRQ;
			break;
		case 12:
			m342.mapper65.irq.value = m342.mapper65.irq.latch;
			cpudata.irq.high &= ~EXT_IRQ;
			break;
		case 13:
			m342.mapper65.irq.latch = (m342.mapper65.irq.latch & 0x00FF) | (value << 8);
			break;
		case 14:
			m342.mapper65.irq.latch = (m342.mapper65.irq.latch & 0xFF00) | value;
			break;
		case 16:
			m342.prg.b = (m342.prg.b & 0xC0) | (value & 0x3F);
			break;
		case 24:
			m342.chr.a = value;
			break;
		case 25:
			m342.chr.b = value;
			break;
		case 26:
			m342.chr.c = value;
			break;
		case 27:
			m342.chr.d = value;
			break;
		case 28:
			m342.chr.e = value;
			break;
		case 29:
			m342.chr.f = value;
			break;
		case 30:
			m342.chr.g = value;
			break;
		case 31:
			m342.chr.h = value;
			break;
		case 32:
			m342.prg.c = (m342.prg.c & 0xC0) | (value & 0x3F);
			break;
	}
}
INLINE static void mapper16_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #1 - MMC1
	// flag0 - 16KB of SRAM (SOROM)
	if (value & 0x80) {
		// reset
		m342.mmc1.load_register = (m342.mmc1.load_register & 0xC0) | 0x20;
		m342.prg.mode = 0;
		m342.prg.c = (m342.prg.c & 0xE0) | 0x1E;
	} else {
		m342.mmc1.load_register = (m342.mmc1.load_register & 0xC0) | ((value & 0x01) << 5) |
			((m342.mmc1.load_register & 0x3E) >> 1);
		if (m342.mmc1.load_register & 0x01) {
			switch ((address >> 13) & 0x0003) {
				case 0:
					if ((m342.mmc1.load_register & 0x18) == 0x18) {
						m342.prg.mode = 0;
						m342.prg.c = (m342.prg.c & 0xE1) | 0x1E;
					} else if ((m342.mmc1.load_register & 0x18) == 0x10) {
						m342.prg.mode = 1;
						m342.prg.c = (m342.prg.c & 0xE1);
					} else {
						m342.prg.mode = 7;
					}
					if (m342.mmc1.load_register & 0x20) {
						m342.chr.mode = 4;
					} else {
						m342.chr.mode = 0;
					}
					m342.mirroring = ((m342.mmc1.load_register >> 1) & 0x03) ^ 0x02;
					break;
				case 1:
					m342.chr.a = (m342.chr.a & 0x83) | ((m342.mmc1.load_register & 0x3E) << 1);
					m342.prg.a = (m342.prg.a & 0xDF) | (m342.mmc1.load_register & 0x20);
					m342.prg.c = (m342.prg.c & 0xDF) | (m342.mmc1.load_register & 0x20);
					break;
				case 2:
					m342.chr.e = (m342.chr.e & 0x83) | ((m342.mmc1.load_register & 0x3E) << 1);
					break;
				case 3:
					m342.prg.a = (m342.prg.a & 0xE1) | (m342.mmc1.load_register & 0x1E);
					m342.wram.enabled = ((m342.mmc1.load_register >> 5) & 0x01) ^ 0x01;
					break;
			}
			m342.mmc1.load_register = 0x20;
		}
	}
}
INLINE static void mapper17_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #9 and #10 - MMC2 and MMC4
	// flag0 - 0=MMC2, 1=MMC4
	switch ((address >> 12) & 0x0007) {
		case 2:
			if (!(m342.flags & 0x01)) {
				// MMC2
				m342.prg.a = (m342.prg.a & 0xF0) | (value & 0x0F);
			} else {
				// MMC4
				m342.prg.a = (m342.prg.a & 0xE1) | ((value & 0x0F) << 1);
			}
			break;
		case 3:
			m342.chr.a = (m342.chr.a & 0x83) | ((value & 0x1F) << 2);
			break;
		case 4:
			m342.chr.b = (m342.chr.b & 0x83) | ((value & 0x1F) << 2);
			break;
		case 5:
			m342.chr.e = (m342.chr.e & 0x83) | ((value & 0x1F) << 2);
			break;
		case 6:
			m342.chr.f = (m342.chr.f & 0x83) | ((value & 0x1F) << 2);
			break;
		case 7:
			m342.mirroring = value & 0x01;
			break;
	}
}
INLINE static void mapper18_cpu_wr_high(BYTE value) {
	// Mapper #152
	m342.chr.a = (m342.chr.a & 0x87) | ((value & 0x0F) << 3);
	m342.prg.a = (m342.prg.a & 0xF1) | ((value & 0x70) >> 3);
	m342.mirroring = 0x02 | (value >> 7);
}
INLINE static void mapper19_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #73 - VRC3
	switch (address & 0x7000) {
		case 0x0000:
			m342.vrc3.irq.latch = (m342.vrc3.irq.latch & 0xFFF0) | (value & 0x0F);
			break;
		case 0x1000:
			m342.vrc3.irq.latch = (m342.vrc3.irq.latch & 0xFF0F) | ((value & 0x0F) << 4);
			break;
		case 0x2000:
			m342.vrc3.irq.latch = (m342.vrc3.irq.latch & 0xF0FF) | ((value & 0x0F) << 8);
			break;
		case 0x3000:
			m342.vrc3.irq.latch = (m342.vrc3.irq.latch & 0x0FFF) | ((value & 0x0F) << 12);
			break;
		case 0x4000:
			m342.vrc3.irq.control = (m342.vrc3.irq.control & 0xF8) | (value & 0x07);
			if (m342.vrc3.irq.control & 0x02) {
				m342.vrc3.irq.value = m342.vrc3.irq.latch;
			}
			cpudata.irq.high &= ~EXT_IRQ;
			break;
		case 0x5000:
			m342.vrc3.irq.control = (m342.vrc3.irq.control & 0xFD) | (m342.vrc3.irq.control & 0x01) << 1;
			cpudata.irq.high &= ~EXT_IRQ;
			break;
		case 0x7000:
			m342.prg.a = (m342.prg.a & 0xF1) | ((value & 0x07) << 1);
			break;
	}
}
INLINE static void mapper20_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #4 - MMC3/MMC6
	// flag0 - TxSROM
	// flag1 - mapper #189
	switch (((address & 0x6000) >> 12) | (address & 0x0001)) {
		case 0:
			m342.mmc3.internal = (m342.mmc3.internal & 0xF8) | (value & 0x07);
			if (!(m342.flags & 0x02) && !(m342.flags & 0x04)) {
				m342.prg.mode = value & 0x40 ? 5 : 4;
			}
			if (!(m342.flags & 0x04)) {
				m342.chr.mode = (value & 0x80) ? 3 : 2;
			}
			break;
		case 1:
			switch (m342.mmc3.internal & 0x07) {
				case 0:
					m342.chr.a = value;
					break;
				case 1:
					m342.chr.c = value;
					break;
				case 2:
					m342.chr.e = value;
					break;
				case 3:
					m342.chr.f = value;
					break;
				case 4:
					m342.chr.g = value;
					break;
				case 5:
					m342.chr.h = value;
					break;
				case 6:
					if (!(m342.flags & 0x02)) {
						m342.prg.a = value;
					}
					break;
				case 7:
					if (!(m342.flags & 0x02)) {
						m342.prg.b = value;
					}
					break;
			}
			// mapper #189
			if (m342.flags & 0x01) {
				switch (m342.mmc3.internal & 0x07) {
					case 0:
					case 1:
						if (m342.chr.mode == 2) {
							WORD adr = 0x2000 | ((m342.mmc3.internal & 0x07) << 11);

							memmap_nmt_1k(MMPPU(adr | 0x0000), ((value >> 7) ^ 0x01));
							memmap_nmt_1k(MMPPU(adr | 0x0400), ((value >> 7) ^ 0x01));

							memmap_nmt_1k(MMPPU(0x1000 | adr | 0x0000), ((value >> 7) ^ 0x01));
							memmap_nmt_1k(MMPPU(0x1000 | adr | 0x0400), ((value >> 7) ^ 0x01));
						}
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						if (m342.chr.mode == 3) {
							WORD adr = 0x2000 | (((m342.mmc3.internal & 0x07) - 2) << 10);

							memmap_nmt_1k(MMPPU(adr), ((value >> 7) ^ 0x01));
							memmap_nmt_1k(MMPPU(0x1000 | adr), ((value >> 7) ^ 0x01));
						}
						break;
				}
			}
			break;
		case 2:
			if (!(m342.flags & 0x04)) {
				m342.mirroring = value & 0x01;
			}
			break;
		case 3:
			break;
		case 4:
			m342.mmc3.irq.latch = value;
			break;
		case 5:
			m342.mmc3.irq.reload = 1;
			break;
		case 6:
			m342.mmc3.irq.enabled = 0;
			cpudata.irq.high &= ~EXT_IRQ;
			break;
		case 7:
			m342.mmc3.irq.enabled = 1;
			break;
	}
}
INLINE static void mapper21_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #112
	switch (address & 0xE000) {
		case 0x8000:
			m342.mapper112.internal = (m342.mapper112.internal & 0xF8) | (value & 0x07);
			break;
		case 0xA000:
			switch (m342.mapper112.internal & 0x07) {
				case 0:
					m342.prg.a = (m342.prg.a & 0xC0) | (value & 0x3F);
					break;
				case 1:
					m342.prg.b = (m342.prg.b & 0xC0) | (value & 0x3F);
					break;
				case 2:
					m342.chr.a = value;
					break;
				case 3:
					m342.chr.c = value;
					break;
				case 4:
					m342.chr.e = value;
					break;
				case 5:
					m342.chr.f = value;
					break;
				case 6:
					m342.chr.g = value;
					break;
				case 7:
					m342.chr.h = value;
					break;
			}
			break;
		case 0xC000:
			break;
		case 0xE000:
			m342.mirroring = value & 0x01;
			break;
	}
}
INLINE static void mapper22_cpu_wr_high(WORD address, BYTE value) {
	// Mappers #33 + #48 - Taito
	// flag0=0 - #33, flag0=1 - #48
	switch (((address & 0x6000) >> 11) | (address & 0x0003)) {
		case 0:
			m342.prg.a = (m342.prg.a & 0xC0) | (value & 0x3F);
			if (!(m342.flags & 0x01)) {
				m342.mirroring = (value >> 6) & 0x01;
			}
			break;
		case 1:
			m342.prg.b = (m342.prg.b & 0xC0) | (value & 0x3F);
			break;
		case 2:
			m342.chr.a = value << 1;
			break;
		case 3:
			m342.chr.c = value << 1;
			break;
		case 4:
			m342.chr.e = value;
			break;
		case 5:
			m342.chr.f = value;
			break;
		case 6:
			m342.chr.g = value;
			break;
		case 7:
			m342.chr.h = value;
			break;
		case 12:
			if (m342.flags & 0x01) {
				m342.mirroring = (value >> 6) & 0x01;
			}
			break;
		case 8:
			m342.mmc3.irq.latch = value ^ 0xFF;
			break;
		case 9:
			m342.mmc3.irq.reload = 1;
			break;
		case 10:
			m342.mmc3.irq.enabled = 1;
			break;
		case 11:
			m342.mmc3.irq.enabled = 0;
			cpudata.irq.high &= ~EXT_IRQ;
			break;
	}
}
INLINE static void mapper23_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #42
	switch (((address >> 12) & 0x0004) | (address & 0x0003)) {
		case 0:
			m342.chr.a = (m342.chr.a & 0xE0) | ((value & 0x1F) << 3);
			break;
		case 4:
			m342.prg.b6000 = (m342.prg.b6000 & 0xF0) | (value & 0x0F);
			break;
		case 5:
			m342.mirroring = (value >> 3) & 0x01;
			break;
		case 6:
			m342.mapper42.irq.enabled = (value & 0x02) >> 1;
			if (!m342.mapper42.irq.enabled) {
				m342.mapper42.irq.value = 0;
				cpudata.irq.high &= ~EXT_IRQ;
			}
			break;
	}
}
INLINE static void mapper24_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #23 - VRC2/4
	// flag0 - switches A0 and A1 lines. 0=A0,A1 like VRC2b (mapper #23), 1=A1,A0 like VRC2a(#22), VRC2c(#25)
	// flag1 - divides CHR bank select by two (mapper #22, VRC2a)
	BYTE vrc_2b_hi = ((address >> 1) & 0x01) | ((address >> 3) & 0x01) | ((address >> 5) & 0x01) | ((address >> 7) & 0x01);
	BYTE vrc_2b_low = (address & 0x01) | ((address >> 2) & 0x01) | ((address >> 4) & 0x01) | ((address >> 6) & 0x01);

	switch (((address >> 10) & 0x1C) |
		((m342.flags & 0x01 ? vrc_2b_low : vrc_2b_hi) << 1) |
		(m342.flags & 0x01 ? vrc_2b_hi : vrc_2b_low)) {
		case 0:
		case 1:
		case 2:
		case 3:
			m342.prg.a = (m342.prg.a & 0xE0) | (value & 0x1F);
			break;
		case 4:
		case 5:
			// VRC2 - using games are usually well - behaved and only write 0 or 1 to this register,
			// but Wai Wai World in one instance writes $FF instead
			if (value != 0xFF) {
				m342.mirroring = value & 0x03;
			}
			break;
		case 6:
		case 7:
			m342.prg.mode = (m342.prg.mode & 0xFE) | ((value >> 1) & 0x01);
			break;
		case 8:
		case 9:
		case 10:
		case 11:
			m342.prg.b = (m342.prg.b & 0xE0) | (value & 0x1F);
			break;
		case 12:
			m342.chr.a = (m342.chr.a & 0xF0) | (value & 0x0F);
			break;
		case 13:
			m342.chr.a = (m342.chr.a & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 14:
			m342.chr.b = (m342.chr.b & 0xF0) | (value & 0x0F);
			break;
		case 15:
			m342.chr.b = (m342.chr.b & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 16:
			m342.chr.c = (m342.chr.c & 0xF0) | (value & 0x0F);
			break;
		case 17:
			m342.chr.c = (m342.chr.c & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 18:
			m342.chr.d = (m342.chr.d & 0xF0) | (value & 0x0F);
			break;
		case 19:
			m342.chr.d = (m342.chr.d & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 20:
			m342.chr.e = (m342.chr.e & 0xF0) | (value & 0x0F);
			break;
		case 21:
			m342.chr.e = (m342.chr.e & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 22:
			m342.chr.f = (m342.chr.f & 0xF0) | (value & 0x0F);
			break;
		case 23:
			m342.chr.f = (m342.chr.f & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 24:
			m342.chr.g = (m342.chr.g & 0xF0) | (value & 0x0F);
			break;
		case 25:
			m342.chr.g = (m342.chr.g & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 26:
			m342.chr.h = (m342.chr.h & 0xF0) | (value & 0x0F);
			break;
		case 27:
			m342.chr.h = (m342.chr.h & 0x0F) | ((value & 0x0F) << 4);
			break;
	}
	if ((address & 0x7000) == 0x7000) {
		switch ((((m342.flags & 0x01) ? vrc_2b_low : vrc_2b_hi) << 1) | ((m342.flags & 0x01) ? vrc_2b_hi : vrc_2b_low)) {
			case 0:
				m342.vrc4.irq.latch = (m342.vrc4.irq.latch & 0xF0) | (value & 0x0F);
				break;
			case 1:
				m342.vrc4.irq.latch = (m342.vrc4.irq.latch & 0x0F) | ((value & 0x0F) << 4);
				break;
			case 2:
				m342.vrc4.irq.control = (m342.vrc4.irq.control & 0xF8) | (value & 0x07);
				if (m342.vrc4.irq.control & 0x02) {
					m342.vrc4.irq.prescaler_counter = 0;
					m342.vrc4.irq.prescaler = 0;
					m342.vrc4.irq.value = m342.vrc4.irq.latch;
				}
				cpudata.irq.high &= ~EXT_IRQ;
				break;
			case 3:
				m342.vrc4.irq.control = (m342.vrc4.irq.control & 0xFD) | (m342.vrc4.irq.control & 0x01) << 1;
				cpudata.irq.high &= ~EXT_IRQ;
				break;
		}
	}
}
INLINE static void mapper25_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #69 - Sunsoft FME-7
	BYTE bank = (address & 0x6000) >> 13;

	if (bank == 0) {
		m342.mapper69.internal = (m342.mapper69.internal & 0xF0) | (value & 0x0F);
	} else if (bank == 1) {
		switch (m342.mapper69.internal & 0x0F) {
			case 0:
				m342.chr.a = value;
				break;
			case 1:
				m342.chr.b = value;
				break;
			case 2:
				m342.chr.c = value;
				break;
			case 3:
				m342.chr.d = value;
				break;
			case 4:
				m342.chr.e = value;
				break;
			case 5:
				m342.chr.f = value;
				break;
			case 6:
				m342.chr.g = value;
				break;
			case 7:
				m342.chr.h = value;
				break;
			case 8:
				m342.wram.enabled = value >> 7;
				m342.map_rom_on_6000 = ((value >> 6) & 0x01) ^ 0x01;
				m342.prg.b6000 = value & 0x3F;
				break;
			case 9:
				m342.prg.a = (m342.prg.a & 0xC0) | (value & 0x3F);
				break;
			case 10:
				m342.prg.b = (m342.prg.b & 0xC0) | (value & 0x3F);
				break;
			case 11:
				m342.prg.c = (m342.prg.c & 0xC0) | (value & 0x3F);
				break;
			case 12:
				m342.mirroring = value & 0x03;
				break;
			case 13:
				m342.mapper69.irq.counter_enabled = value >> 7;
				m342.mapper69.irq.enabled = value & 0x01;
				cpudata.irq.high &= ~EXT_IRQ;
				break;
			case 14:
				m342.mapper69.irq.value = (m342.mapper69.irq.value & 0xFF00) | value;
				break;
			case 15:
				m342.mapper69.irq.value = (m342.mapper69.irq.value & 0x00FF) | (value << 8);
				break;
		}
	}
}
INLINE static void mapper26_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #32 - Irem's G-101
	switch (address & 0xF000) {
		case 0x8000:
			m342.prg.a = (m342.prg.a & 0xC0) | (value & 0x3F);
			break;
		case 0x9000:
			m342.prg.mode = (m342.prg.mode & 0x06) | ((value >> 1) & 0x01);
			m342.mirroring = value & 0x01;
			break;
		case 0xA000:
			m342.prg.b = (m342.prg.b & 0xC0) | (value & 0x3F);
			break;
		case 0xB000:
			switch (address & 0x0007) {
				case 0:
					m342.chr.a = value;
					break;
				case 1:
					m342.chr.b = value;
					break;
				case 2:
					m342.chr.c = value;
					break;
				case 3:
					m342.chr.d = value;
					break;
				case 4:
					m342.chr.e = value;
					break;
				case 5:
					m342.chr.f = value;
					break;
				case 6:
					m342.chr.g = value;
					break;
				case 7:
					m342.chr.h = value;
					break;
			}
			break;
	}
}
INLINE static void mapper29_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #36 is assigned to TXC's PCB 01-22000-400
	if ((address & 0x7FFE) == 0x7FFE) {
		m342.prg.a = (m342.prg.a & 0xC3) | ((value & 0xF0) >> 2);
		m342.chr.a = (m342.chr.a & 0x87) | ((value & 0x0F) << 3);
	}
}
INLINE static void mapper30_cpu_wr_high(BYTE value) {
	// Mapper #70
	m342.prg.a = (m342.prg.a & 0xE1) | ((value & 0xF0) >> 3);
	m342.chr.a = (m342.chr.a & 0x87) | ((value & 0x0F) << 3);
}
INLINE static void mapper34_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #75 - VRC1
	switch (address & 0x7000) {
		case 0x0000:
			m342.prg.a = (m342.prg.a & 0xF0) | (value & 0x0F);
			break;
		case 0x1000:
			m342.mirroring = value & 0x01;
			m342.chr.a = (m342.chr.a & 0xBF) | ((value & 0x02) << 5);
			m342.chr.e = (m342.chr.a & 0xBF) | ((value & 0x04) << 4);
			break;
		case 0x2000:
			m342.prg.b = (m342.prg.b & 0xF0) | (value & 0x0F);
			break;
		case 0x4000:
			m342.prg.c = (m342.prg.c & 0xF0) | (value & 0x0F);
			break;
		case 0x6000:
			m342.chr.a = (m342.chr.a & 0xC3) | ((value & 0x0F) << 2);
			break;
		case 0x7000:
			m342.chr.e = (m342.chr.e & 0xC3) | ((value & 0x0F) << 2);
			break;
	}
}
INLINE static void mapper35_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #83 - Cony/Yoko
	switch (address & 0x0300) {
		case 0x0100:
			m342.mirroring = value & 0x03;
			m342.mapper83.irq.enabled_latch = value >> 7;
			break;
		case 0x0200:
			if (!(address & 0x0001)) {
				m342.mapper83.irq.counter = (m342.mapper83.irq.counter & 0xFF00) | value;
				cpudata.irq.high &= ~EXT_IRQ;
			} else {
				m342.mapper83.irq.enabled = m342.mapper83.irq.enabled_latch;
				m342.mapper83.irq.counter = (m342.mapper83.irq.counter & 0x00FF) | (value << 8);
			}
		break;
	case 0x0300:
		if (!(address & 0x0010))
		{
			switch (address & 0x0003) {
				case 0:
					m342.prg.a = value;
					break;
				case 1:
					m342.prg.b = value;
					break;
				case 2:
					m342.prg.b = value;
					break;
				//case 3:
					//m342.prg.b6000 = value;
					//break;
			}
		} else {
			switch (address & 0x0007) {
				case 0:
					m342.chr.a = value;
					break;
				case 1:
					m342.chr.b = value;
					break;
				case 2:
					m342.chr.c = value;
					break;
				case 3:
					m342.chr.d = value;
					break;
				case 4:
					m342.chr.e = value;
					break;
				case 5:
					m342.chr.f = value;
					break;
				case 6:
					m342.chr.g = value;
					break;
				case 7:
					m342.chr.h = value;
					break;
			}
		}
		break;
	}
}
INLINE static void mapper36_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #67 - Sunsoft-3
	if (address & 0x0800) {
		switch (address & 0x7000) {
			case 0x0000:
				m342.chr.a = (m342.chr.a & 0x81) | ((value & 0x3F) << 1);
				break;
			case 0x1000:
				m342.chr.c = (m342.chr.c & 0x81) | ((value & 0x3F) << 1);
				break;
			case 0x2000:
				m342.chr.e = (m342.chr.e & 0x81) | ((value & 0x3F) << 1);
				break;
			case 0x3000:
				m342.chr.g = (m342.chr.g & 0x81) | ((value & 0x3F) << 1);
				break;
			case 0x4000:
				m342.mapper67.irq.latch = ~m342.mapper67.irq.latch;
				if (m342.mapper67.irq.latch) {
					m342.mapper67.irq.counter = (m342.mapper67.irq.counter & 0x00FF) | (value << 8);
				} else {
					m342.mapper67.irq.counter = (m342.mapper67.irq.counter & 0xFF00) | value;
				}
				break;
			case 0x5000:
				m342.mapper67.irq.latch = 0;
				m342.mapper67.irq.enabled = (value & 0x10) >> 4;
				break;
			case 0x6000:
				m342.mirroring = value & 0x03;
				break;
			case 0x7000:
				m342.prg.a = (m342.prg.a & 0xE1) | ((value & 0x0F) << 1);
				break;
		}
	} else {
		// Interrupt Acknowledge ($8000)
		cpudata.irq.high &= ~EXT_IRQ;
	}
}
INLINE static void mapper37_cpu_wr_high(BYTE value) {
	// Mapper #89 - Sunsoft-2 chip on the Sunsoft-3 board
	m342.prg.a = (m342.prg.a & 0xF1) | ((value & 0x70) >> 3);
	m342.chr.a = (m342.chr.a & 0x87) | ((value & 0x80) >> 1) | ((value & 0x07) << 3);
	m342.mirroring = 0x02 | ((value & 0x08) >> 3);
}

INLINE static BYTE mapper06_cpu_rd(WORD address, BYTE openbus) {
	// Mapper #163
	if ((address & 0x7700) == 0x5100) {
		return (m342.mapper163.r2 | m342.mapper163.r0 | m342.mapper163.r1 | ~m342.mapper163.r3);
	}
	if ((address & 0x7700) == 0x5500) {
		return (m342.mapper163.r5 & 0x01 ? m342.mapper163.r2 : m342.mapper163.r1);
	}
	return (openbus);
}
INLINE static BYTE mapper13_cpu_rd(WORD address, BYTE openbus) {
	// Mapper #90 - JY
	if (address == 0x5800) {
		return (m342.mapper90.mul1 * m342.mapper90.mul2) & 0xFF;
	}
	if (address == 0x5801) {
		return ((m342.mapper90.mul1 * m342.mapper90.mul2) >> 8) & 0xFF;
	}
	return (openbus);
}
INLINE static BYTE mapper15_cpu_rd(WORD address, BYTE openbus) {
	// MMC5
	if (address == 0x0204) {
		BYTE value = (m342.mmc5.irq.out << 7) |
			(!m342.mmc5.ppu.rendering || ((m342.mmc5.ppu.scanline + 1) >= 241) ? 0 : 0x40);

		m342.mmc5.irq.out = 0;
		cpudata.irq.high &= ~EXT_IRQ;
		return (value);
	}
	return (openbus);
}
INLINE static BYTE mapper35_cpu_rd(void) {
	return (m342.flags & 0x03);
}

INLINE static void mapper07_cpu_every_cycle(void) {
	// Mapper #18
	if (m342.mapper18.irq.control & 0x01) {
		BYTE carry = carry = (m342.mapper18.irq.value & 0x0F) - 1;

		m342.mapper18.irq.value = (m342.mapper18.irq.value & 0xFFF0) | (carry & 0x0F);
		carry = (carry >> 4) & 0x01;
		if (!(m342.mapper18.irq.control & 0x08)) {
			carry = ((m342.mapper18.irq.value >> 4) & 0x0F) - carry;
			//SET_BITS(mapper18_irq_value, "7:4", carry, "3:0");
			m342.mapper18.irq.value = (m342.mapper18.irq.value & 0xFF0F) | ((carry & 0x0F) << 4);
			carry = (carry >> 4) & 0x01;
		}
		if (!(m342.mapper18.irq.control & 0x0C)) {
			carry = ((m342.mapper18.irq.value >> 8) & 0x0F) - carry;
			m342.mapper18.irq.value = (m342.mapper18.irq.value & 0xF0FF) | ((carry & 0x0F) << 8);
			carry = (carry >> 4) & 0x01;
		}
		if (!(m342.mapper18.irq.control & 0x0E)) {
			carry = ((m342.mapper18.irq.value >> 12) & 0x0F) - carry;
			m342.mapper18.irq.value = (m342.mapper18.irq.value & 0x0FFF) | ((carry & 0x0F) << 12);
			carry = (carry >> 4) & 0x01;
		}
		if (carry) {
			cpudata.irq.high |= EXT_IRQ;
		}
	}
}
INLINE static void mapper14_cpu_every_cycle(void) {
	// Mapper #65 - Irem's H3001
	if (m342.mapper65.irq.enabled) {
		if (m342.mapper65.irq.value != 0) {
			m342.mapper65.irq.value--;
			if (!m342.mapper65.irq.value) {
				cpudata.irq.high |= EXT_IRQ;
			}
		}
	}
}
INLINE static void mapper19_cpu_every_cycle(void) {
	// Mapper #73 - VRC3
	if (m342.vrc3.irq.control & 0x02) {
		if (m342.vrc3.irq.control & 0x04) {  // 8-bit mode
			m342.vrc3.irq.value = (m342.vrc3.irq.value & 0xFF00) | ((m342.vrc3.irq.value + 1) & 0xFF);
			if ((m342.vrc3.irq.value & 0xFF) == 0) {
				m342.vrc3.irq.value = (m342.vrc3.irq.value & 0xFF00) | (m342.vrc3.irq.latch & 0xFF);
				cpudata.irq.high |= EXT_IRQ;
			}
		} else { // 16-bit
			m342.vrc3.irq.value += 1;
			if (m342.vrc3.irq.value == 0) {
				m342.vrc3.irq.value = m342.vrc3.irq.latch;
				cpudata.irq.high |= EXT_IRQ;
			}
		}
	}
}
INLINE static void mapper23_cpu_every_cycle(void) {
	// Mapper #42
	if (m342.mapper42.irq.enabled) {
		m342.mapper42.irq.value++;
		if (m342.mapper42.irq.value >> 15) {
			m342.mapper42.irq.value = 0;
		}
		if (((m342.mapper42.irq.value >> 13) & 0x03) == 0x03) {
			cpudata.irq.high |= EXT_IRQ;
		} else {
			cpudata.irq.high &= ~EXT_IRQ;
		}
	}
}
INLINE static void mapper24_cpu_every_cycle(void) {
	// Mapper #23 - VRC2/4
	if (m342.vrc4.irq.control & 0x02) {
		if (m342.vrc4.irq.control & 0x04) {
			m342.vrc4.irq.value++;
			if (m342.vrc4.irq.value == 0) {
				m342.vrc4.irq.value = m342.vrc4.irq.latch;
				cpudata.irq.high |= EXT_IRQ;
			}
		} else {
			m342.vrc4.irq.prescaler++;
			if ((!(m342.vrc4.irq.prescaler_counter & 0x02) && (m342.vrc4.irq.prescaler == 114))
				|| ((m342.vrc4.irq.prescaler_counter & 0x02) && (m342.vrc4.irq.prescaler == 113))) {
				m342.vrc4.irq.prescaler = 0;
				m342.vrc4.irq.prescaler_counter++;
				if (m342.vrc4.irq.prescaler_counter == 3) {
					m342.vrc4.irq.prescaler_counter = 0;
				}
				m342.vrc4.irq.value++;
				if (m342.vrc4.irq.value == 0) {
					m342.vrc4.irq.value = m342.vrc4.irq.latch;
					cpudata.irq.high |= EXT_IRQ;
				}
			}
		}
	}
}
INLINE static void mapper25_cpu_every_cycle(void) {
	// Mapper #69 - Sunsoft FME-7
	if (m342.mapper69.irq.counter_enabled) {
		m342.mapper69.irq.value--;
		if (m342.mapper69.irq.value == 0xFFFF) {
			cpudata.irq.high |= EXT_IRQ;
		}
	}
}
INLINE static void mapper35_cpu_every_cycle(void) {
	// Mapper #83 - Cony/Yoko
	if (m342.mapper83.irq.enabled) {
		if (m342.mapper83.irq.counter == 0) {
			cpudata.irq.high |= EXT_IRQ;
		}
		m342.mapper83.irq.counter--;
	}
}
INLINE static void mapper36_cpu_every_cycle(void) {
	// Mapper #67 - Sunsoft-3
	if (m342.mapper67.irq.enabled) {
		m342.mapper67.irq.counter--;
		if (m342.mapper67.irq.counter == 0xFFFF) {
			cpudata.irq.high |= EXT_IRQ;
			m342.mapper67.irq.enabled = 0;
		}
	}
}
