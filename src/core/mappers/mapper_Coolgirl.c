/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

// https://github.com/ClusterM/fceux/blob/coolgirl/src/boards/coolgirl.cpp

#include <string.h>
#include <stdlib.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "ppu.h"
#include "tas.h"
#include "save_slot.h"

enum _coolgirl_misc {
	SAVE_FLASH_SIZE = 1024 * 1024 * 8,
	SAVE_FLASH_MAX_8K = 1024 -1,
	SAVE_FLASH_MAX_16K = 512 -1,
	SAVE_FLASH_MAX_32K = 256 -1,
	CFI_SIZE = 1024 * 32
};

INLINE static void state_fix_Coolgirl(void);
INLINE static void prg_fix_Coolgirl(void);
INLINE static DBWORD prg_mapped_Coolgirl(BYTE bank);
INLINE static BYTE prg_mapped_is_flash_Coolgirl(DBWORD mapped);
INLINE static void prg_swap_8k_Coolgirl(BYTE flash, BYTE at, DBWORD value);
INLINE static void prg_swap_16k_Coolgirl(BYTE flash, BYTE at, DBWORD value);
INLINE static void prg_swap_32k_Coolgirl(BYTE flash, DBWORD value);
INLINE static void chr_fix_Coolgirl(void);
INLINE static void mirroring_fix_Coolgirl(void);
INLINE static void flash_write_Coolgirl(WORD address, BYTE value);

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
INLINE static void mapper02_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper03_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper04_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper05_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper07_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper08_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper09_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper10_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper11_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper13_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper14_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper16_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper17_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper18_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper19_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper20_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper21_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper22_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper23_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper24_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper25_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper26_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper29_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper30_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper34_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper35_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper36_cpu_wr_high(WORD address, BYTE value);
INLINE static void mapper37_cpu_wr_high(WORD address, BYTE value);

INLINE static BYTE mapper06_cpu_rd(WORD address, BYTE openbus);
INLINE static BYTE mapper13_cpu_rd(WORD address, BYTE openbus);
INLINE static BYTE mapper15_cpu_rd(WORD address, BYTE openbus);
INLINE static BYTE mapper35_cpu_rd(WORD address, BYTE openbus);

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
struct _coolgirl {
	BYTE mapper;
	BYTE fscreen;
	BYTE mirroring;
	BYTE flags;
	BYTE lock;
	BYTE cfi_mode;
	BYTE map_rom_on_6000;

	struct _coolgirl_wram {
		BYTE enabled;
		BYTE page;
	} wram;
	struct _coolgirl_flash {
		BYTE can_write;
		BYTE state;
		WORD buffer_a[10];
		BYTE buffer_v[10];
	} flash;
	struct _coolgirl_prg {
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
	struct _coolgirl_chr {
		WORD a, b, c, d, e, f, g, h;
		BYTE can_write;
		BYTE mode;
		BYTE mask;
	} chr;
	// for MMC1
	struct _coolgirl_mmc1 {
		//static uint64 lreset = 0;
		BYTE load_register;
	} mmc1;
	// for MMC2/MMC4
	struct _coolgirl_mmc24 {
		BYTE latch0;
		BYTE latch1;
	} mmc24;
	// for MMC3
	struct _coolgirl_mmc3 {
		BYTE internal;
		// for MMC3 scanline-based interrupts, counts A12 rises after long A12 falls
		struct _coolgirl_mmc3_irq {
			BYTE enabled;
			BYTE counter;
			BYTE latch;
			BYTE reload;
		} irq;
	} mmc3;
	// for MMC5
	struct _coolgirl_mmc5 {
		// for MMC5 scanline-based interrupts, counts dummy PPU reads
		struct _coolgirl_mmc5_irq {
			BYTE enabled;
			BYTE line;
			BYTE out;
		} irq;
		struct _coolgirl_mmc5_ppu {
			BYTE rendering;
			WORD scanline;
		} ppu;
	} mmc5;
	// for VRC3
	struct _coolgirl_vrc3 {
		// for VRC3 CPU-based interrupts
		struct _coolgirl_vrc3_irq {
			WORD value;
			BYTE control;
			WORD latch;
		} irq;
	} vrc3;
	// for VRC4
	struct _coolgirl_vrc4 {
		// for VRC4 CPU-based interrupts
		struct _coolgirl_vrc4_irq {
			BYTE value;
			BYTE control;
			BYTE latch;
			BYTE prescaler;
			BYTE prescaler_counter;
		} irq;
	} vrc4;
	// for mapper #69 Sunsoft FME-7
	struct _coolgirl_mapper69 {
		BYTE internal;
		struct _coolgirl_mapper69_irq {
			BYTE enabled;
			BYTE counter_enabled;
			WORD value;
		} irq;
	} mapper69;
	// for mapper #112
	struct _coolgirl_mapper112 {
		BYTE internal;
	} mapper112;
	// for mapper #163
	struct _coolgirl_mapper163 {
		BYTE latch;
		BYTE r0;
		BYTE r1;
		BYTE r2;
		BYTE r3;
		BYTE r4;
		BYTE r5;
	} mapper163;
	// for mapper #83
	struct _coolgirl_mapper83 {
		struct _coolgirl_mapper83_irq {
			BYTE enabled_latch;
			BYTE enabled;
			WORD counter;
		} irq;
	} mapper83;
	// for mapper #90
	struct _coolgirl_mapper90 {
		BYTE mul1;
		BYTE mul2;
		BYTE xor;
	} mapper90;
	// for mapper #18
	struct _coolgirl_mapper18 {
		struct _coolgirl_mapper18_irq {
				WORD value;
				BYTE control;
				WORD latch;
		} irq;
	} mapper18;
	// for mapper #65
	struct _coolgirl_mapper65 {
		struct _coolgirl_mapper65_irq {
			BYTE enabled;
			WORD value;
			WORD latch;
		} irq;
	} mapper65;
	// for mapper #42 (only Baby Mario)
	struct _coolgirl_mapper42 {
		struct _coolgirl_mapper42_irq {
			BYTE enabled;
			WORD value;
		} irq;
	} mapper42;
	// for mapper #67
	struct _coolgirl_mapper67 {
		struct _coolgirl_mapper67_irq {
			BYTE enabled;
			BYTE latch;
			WORD counter;
		} irq;
	} mapper67;
} coolgirl;
struct _coolgirltmp {
	BYTE *save_flash;
	BYTE *cfi;
	BYTE *prg_6000;
} coolgirltmp;

void map_init_Coolgirl(void) {
	EXTCL_AFTER_MAPPER_INIT(Coolgirl);
	EXTCL_MAPPER_QUIT(Coolgirl);
	EXTCL_CPU_WR_MEM(Coolgirl);
	EXTCL_CPU_RD_MEM(Coolgirl);
	EXTCL_SAVE_MAPPER(Coolgirl);
	EXTCL_BATTERY_IO(Coolgirl);
	EXTCL_CPU_EVERY_CYCLE(Coolgirl);
	EXTCL_WR_CHR(Coolgirl);
	EXTCL_PPU_000_TO_34X(Coolgirl);
	EXTCL_AFTER_RD_CHR(Coolgirl);
	EXTCL_UPDATE_R2006(Coolgirl);
	mapper.internal_struct[0] = (BYTE *)&coolgirl;
	mapper.internal_struct_size[0] = sizeof(coolgirl);

	memset(&coolgirl, 0x00, sizeof(coolgirl));

	coolgirl.prg.mask = 0xF8; // 11111000, 128KB
	coolgirl.prg.a = 0;
	coolgirl.prg.b = 1;
	coolgirl.prg.c = 0xFE;
	coolgirl.prg.d = 0xFF;

	coolgirl.chr.a = 0;
	coolgirl.chr.b = 1;
	coolgirl.chr.c = 2;
	coolgirl.chr.d = 3;
	coolgirl.chr.e = 4;
	coolgirl.chr.f = 5;
	coolgirl.chr.g = 6;
	coolgirl.chr.h = 7;

	// non ha CHR ROM ma solo CHR RAM
	info.chr.rom.banks_8k = 64;

	// 32k di PRG RAM
	info.prg.ram.banks_8k_plus = 4;
	info.prg.ram.bat.banks = 4;

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		coolgirltmp.save_flash = (BYTE *)malloc(SAVE_FLASH_SIZE);
		memset(coolgirltmp.save_flash, 0x00, SAVE_FLASH_SIZE);

		coolgirltmp.cfi = (BYTE *)malloc(CFI_SIZE);
		memset(coolgirltmp.cfi, 0x00, CFI_SIZE);

		for (int i = 0; i < 0x8000; i += sizeof(cfi_data)) {
			memcpy(&coolgirltmp.cfi[i], cfi_data, sizeof(cfi_data));
		}
	}

	info.mapper.extend_wr = info.mapper.extend_rd = TRUE;
}
void extcl_after_mapper_init_Coolgirl(void) {
	state_fix_Coolgirl();
}
void extcl_mapper_quit_Coolgirl(void) {
	if (coolgirltmp.save_flash) {
		free(coolgirltmp.save_flash);
		coolgirltmp.save_flash = NULL;
	}
	if (coolgirltmp.cfi) {
		free(coolgirltmp.cfi);
		coolgirltmp.cfi = NULL;
	}
}
void extcl_cpu_wr_mem_Coolgirl(WORD address, BYTE value) {
	if (address < 0x8000) {
		if ((address >= 0x5000) && (address < 0x5FFF) && !coolgirl.lock) {
			mapper00_cpu_wr_low(address, value);
		}
		switch (coolgirl.mapper) {
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
		state_fix_Coolgirl();
		return;
	}

	// writing flash
	if (coolgirl.flash.can_write) {
		flash_write_Coolgirl(address, value);
		return;
	}

	switch (coolgirl.mapper) {
		case 1:
			mapper01_cpu_wr_high(address, value);
			break;
		case 2:
			mapper02_cpu_wr_high(address, value);
			break;
		case 3:
			mapper03_cpu_wr_high(address, value);
			break;
		case 4:
			mapper04_cpu_wr_high(address, value);
			break;
		case 5:
			mapper05_cpu_wr_high(address, value);
			break;
		case 7:
			mapper07_cpu_wr_high(address, value);
			break;
		case 8:
			mapper08_cpu_wr_high(address, value);
			break;
		case 9:
			mapper09_cpu_wr_high(address, value);
			break;
		case 10:
			mapper10_cpu_wr_high(address, value);
			break;
		case 11:
			mapper11_cpu_wr_high(address, value);
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
			mapper18_cpu_wr_high(address, value);
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
			mapper30_cpu_wr_high(address, value);
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
			mapper37_cpu_wr_high(address, value);
			break;
	}
	state_fix_Coolgirl();
}
BYTE extcl_cpu_rd_mem_Coolgirl(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	// Mapper #36 is assigned to TXC's PCB 01-22000-400
	if ((coolgirl.mapper == 29) && ((address & 0xE100) == 0x4100)) {
		return ((coolgirl.prg.a & 0x0C) << 2);
	}

	switch (address & 0xF000) {
		case 0x5000:
			switch (coolgirl.mapper) {
				case 0:
					return (0);
				case 6:
					return (mapper06_cpu_rd(address, openbus));
				case 13:
					return (mapper13_cpu_rd(address, openbus));
				case 15:
					return (mapper15_cpu_rd(address, openbus));
				case 35:
					return (mapper35_cpu_rd(address, openbus));
			}
			break;
		case 0x6000:
		case 0x7000:
			if (coolgirl.map_rom_on_6000) {
				return (coolgirltmp.prg_6000[address & 0x1FFF]);
			}
			//if (coolgirl.wram.enabled) {
			//	return (prg.ram_plus_8k[address & 0x1FFF]);
			//}
			break;
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (coolgirl.cfi_mode) {
				return (coolgirltmp.cfi[address & 0x7FFF]);
			}
			break;
	}
	return (openbus);
}
BYTE extcl_save_mapper_Coolgirl(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, coolgirl.mapper);
	save_slot_ele(mode, slot, coolgirl.fscreen);
	save_slot_ele(mode, slot, coolgirl.mirroring);
	save_slot_ele(mode, slot, coolgirl.flags);
	save_slot_ele(mode, slot, coolgirl.lock);
	save_slot_ele(mode, slot, coolgirl.cfi_mode);
	save_slot_ele(mode, slot, coolgirl.map_rom_on_6000);

	save_slot_ele(mode, slot, coolgirl.wram.enabled);
	save_slot_ele(mode, slot, coolgirl.wram.page);

	save_slot_ele(mode, slot, coolgirl.flash.can_write);
	save_slot_ele(mode, slot, coolgirl.flash.state);
	save_slot_ele(mode, slot, coolgirl.flash.buffer_a);
	save_slot_ele(mode, slot, coolgirl.flash.buffer_v);

	save_slot_ele(mode, slot, coolgirl.prg.b6000);
	save_slot_ele(mode, slot, coolgirl.prg.a);
	save_slot_ele(mode, slot, coolgirl.prg.b);
	save_slot_ele(mode, slot, coolgirl.prg.c);
	save_slot_ele(mode, slot, coolgirl.prg.d);
	save_slot_ele(mode, slot, coolgirl.prg.b6000_mapped);
	save_slot_ele(mode, slot, coolgirl.prg.a_mapped);
	save_slot_ele(mode, slot, coolgirl.prg.b_mapped);
	save_slot_ele(mode, slot, coolgirl.prg.c_mapped);
	save_slot_ele(mode, slot, coolgirl.prg.d_mapped);
	save_slot_ele(mode, slot, coolgirl.prg.mode);
	save_slot_ele(mode, slot, coolgirl.prg.base);
	save_slot_ele(mode, slot, coolgirl.prg.mask);

	save_slot_ele(mode, slot, coolgirl.chr.can_write);
	save_slot_ele(mode, slot, coolgirl.chr.a);
	save_slot_ele(mode, slot, coolgirl.chr.b);
	save_slot_ele(mode, slot, coolgirl.chr.c);
	save_slot_ele(mode, slot, coolgirl.chr.d);
	save_slot_ele(mode, slot, coolgirl.chr.e);
	save_slot_ele(mode, slot, coolgirl.chr.f);
	save_slot_ele(mode, slot, coolgirl.chr.g);
	save_slot_ele(mode, slot, coolgirl.chr.h);
	save_slot_ele(mode, slot, coolgirl.chr.mode);
	save_slot_ele(mode, slot, coolgirl.chr.mask);

	save_slot_ele(mode, slot, coolgirl.mmc1.load_register);

	save_slot_ele(mode, slot, coolgirl.mmc24.latch0);
	save_slot_ele(mode, slot, coolgirl.mmc24.latch1);

	save_slot_ele(mode, slot, coolgirl.mmc3.internal);
	save_slot_ele(mode, slot, coolgirl.mmc3.irq.enabled);
	save_slot_ele(mode, slot, coolgirl.mmc3.irq.counter);
	save_slot_ele(mode, slot, coolgirl.mmc3.irq.latch);
	save_slot_ele(mode, slot, coolgirl.mmc3.irq.reload);

	save_slot_ele(mode, slot, coolgirl.mmc5.irq.enabled);
	save_slot_ele(mode, slot, coolgirl.mmc5.irq.line);
	save_slot_ele(mode, slot, coolgirl.mmc5.irq.out);
	save_slot_ele(mode, slot, coolgirl.mmc5.ppu.rendering);
	save_slot_ele(mode, slot, coolgirl.mmc5.ppu.scanline);

	save_slot_ele(mode, slot, coolgirl.vrc3.irq.value);
	save_slot_ele(mode, slot, coolgirl.vrc3.irq.control);
	save_slot_ele(mode, slot, coolgirl.vrc3.irq.latch);

	save_slot_ele(mode, slot, coolgirl.vrc4.irq.value);
	save_slot_ele(mode, slot, coolgirl.vrc4.irq.control);
	save_slot_ele(mode, slot, coolgirl.vrc4.irq.latch);
	save_slot_ele(mode, slot, coolgirl.vrc4.irq.prescaler);
	save_slot_ele(mode, slot, coolgirl.vrc4.irq.prescaler_counter);

	save_slot_ele(mode, slot, coolgirl.mapper69.internal);
	save_slot_ele(mode, slot, coolgirl.mapper69.irq.enabled);
	save_slot_ele(mode, slot, coolgirl.mapper69.irq.counter_enabled);
	save_slot_ele(mode, slot, coolgirl.mapper69.irq.value);

	save_slot_ele(mode, slot, coolgirl.mapper112.internal);

	save_slot_ele(mode, slot, coolgirl.mapper163.latch);
	save_slot_ele(mode, slot, coolgirl.mapper163.r0);
	save_slot_ele(mode, slot, coolgirl.mapper163.r1);
	save_slot_ele(mode, slot, coolgirl.mapper163.r2);
	save_slot_ele(mode, slot, coolgirl.mapper163.r3);
	save_slot_ele(mode, slot, coolgirl.mapper163.r4);
	save_slot_ele(mode, slot, coolgirl.mapper163.r5);

	save_slot_ele(mode, slot, coolgirl.mapper83.irq.enabled_latch);
	save_slot_ele(mode, slot, coolgirl.mapper83.irq.enabled);
	save_slot_ele(mode, slot, coolgirl.mapper83.irq.counter);

	save_slot_ele(mode, slot, coolgirl.mapper90.mul1);
	save_slot_ele(mode, slot, coolgirl.mapper90.mul2);
	save_slot_ele(mode, slot, coolgirl.mapper90.xor);

	save_slot_ele(mode, slot, coolgirl.mapper18.irq.value);
	save_slot_ele(mode, slot, coolgirl.mapper18.irq.control);
	save_slot_ele(mode, slot, coolgirl.mapper18.irq.latch);

	save_slot_ele(mode, slot, coolgirl.mapper65.irq.enabled);
	save_slot_ele(mode, slot, coolgirl.mapper65.irq.value);
	save_slot_ele(mode, slot, coolgirl.mapper65.irq.latch);

	save_slot_ele(mode, slot, coolgirl.mapper42.irq.enabled);
	save_slot_ele(mode, slot, coolgirl.mapper42.irq.value);

	save_slot_ele(mode, slot, coolgirl.mapper67.irq.enabled);
	save_slot_ele(mode, slot, coolgirl.mapper67.irq.latch);
	save_slot_ele(mode, slot, coolgirl.mapper67.irq.counter);

	if (mode == SAVE_SLOT_READ) {
		state_fix_Coolgirl();
	}

	return (EXIT_OK);
}
void extcl_battery_io_Coolgirl(BYTE mode, FILE *fp) {
	if (!fp || (tas.type != NOTAS)) {
		return;
	}

	if (mode == WR_BAT) {
		map_bat_wr_default(fp);
		if (fwrite(coolgirltmp.save_flash, SAVE_FLASH_SIZE, 1, fp) < 1) {
			//fprintf(stderr, "error on write flash chip\n");
		}
	} else {
		map_bat_rd_default(fp);
		if (fread(coolgirltmp.save_flash, SAVE_FLASH_SIZE, 1, fp) < 1) {
			//fprintf(stderr, "error on read flash chip\n");
		}
	}
}
void extcl_cpu_every_cycle_Coolgirl(void) {
	switch (coolgirl.mapper) {
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
void extcl_ppu_000_to_34x_Coolgirl(void) {
	coolgirl.mmc5.ppu.rendering = !ppu.vblank && r2001.visible && (ppu.screen_y < SCR_ROWS);
	coolgirl.mmc5.ppu.scanline = ppu.screen_y;

	if ((ppu.frame_x == 260) && coolgirl.mmc5.ppu.rendering) {
		// for MMC3 and MMC3-based
		if (coolgirl.mmc3.irq.reload || !coolgirl.mmc3.irq.counter) {
			coolgirl.mmc3.irq.counter = coolgirl.mmc3.irq.latch;
			coolgirl.mmc3.irq.reload = 0;
		} else {
			coolgirl.mmc3.irq.counter--;
		}
		if (!coolgirl.mmc3.irq.counter && coolgirl.mmc3.irq.enabled) {
			irq.high |= EXT_IRQ;
		}

		// for MMC5
		if ((coolgirl.mmc5.irq.line == (ppu.screen_y + 1)) && coolgirl.mmc5.irq.enabled) {
			coolgirl.mmc5.irq.out = 1;
			irq.high |= EXT_IRQ;
		}

		// for mapper #163
		if (coolgirl.mapper == 6) {
			if (ppu.screen_y == 239) {
				coolgirl.mapper163.latch = 0;
				chr_fix_Coolgirl();
			} else if (ppu.screen_y == 127) {
				coolgirl.mapper163.latch = 1;
				chr_fix_Coolgirl();
			}
		}
	}
}
void extcl_wr_chr_Coolgirl(WORD address, BYTE value) {
	if (coolgirl.chr.can_write) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}
void extcl_after_rd_chr_Coolgirl(WORD address) {
	if (coolgirl.mapper == 17) {
		switch (address & 0xFFF0) {
			case 0x0FD0:
				coolgirl.mmc24.latch0 = 0;
				chr_fix_Coolgirl();
				break;
			case 0x0FE0:
				coolgirl.mmc24.latch0 = 1;
				chr_fix_Coolgirl();
				break;
			case 0x1FD0:
				coolgirl.mmc24.latch1 = 0;
				chr_fix_Coolgirl();
				break;
			case 0x1FE0:
				coolgirl.mmc24.latch1 = 1;
				chr_fix_Coolgirl();
				break;
			default:
				return;
		}
	}
}
void extcl_update_r2006_Coolgirl(WORD new_r2006, UNUSED(WORD old_r2006)) {
	extcl_after_rd_chr_Coolgirl(new_r2006);
}

INLINE static void state_fix_Coolgirl(void) {
	prg_fix_Coolgirl();
	chr_fix_Coolgirl();
	mirroring_fix_Coolgirl();
}
INLINE static void prg_fix_Coolgirl(void) {
	BYTE a_is_flash, b_is_flash, c_is_flash, d_is_flash;
	DBWORD bank;

	coolgirl.prg.b6000_mapped = prg_mapped_Coolgirl(coolgirl.prg.b6000);
	coolgirl.prg.a_mapped = prg_mapped_Coolgirl(coolgirl.prg.a);
	coolgirl.prg.b_mapped = prg_mapped_Coolgirl(coolgirl.prg.b);
	coolgirl.prg.c_mapped = prg_mapped_Coolgirl(coolgirl.prg.c);
	coolgirl.prg.d_mapped = prg_mapped_Coolgirl(coolgirl.prg.d);
	a_is_flash = prg_mapped_is_flash_Coolgirl(coolgirl.prg.a_mapped);
	b_is_flash = prg_mapped_is_flash_Coolgirl(coolgirl.prg.b_mapped);
	c_is_flash = prg_mapped_is_flash_Coolgirl(coolgirl.prg.c_mapped);
	d_is_flash = prg_mapped_is_flash_Coolgirl(coolgirl.prg.d_mapped);

	if (!coolgirl.cfi_mode) {
		switch (coolgirl.prg.mode & 0x07) {
			default:
			case 0:
				bank = coolgirl.prg.a_mapped >> 1;
				prg_swap_16k_Coolgirl(a_is_flash, 0, bank);
				bank = coolgirl.prg.c_mapped >> 1;
				prg_swap_16k_Coolgirl(c_is_flash, 2, bank);
				break;
			case 1:
				bank = coolgirl.prg.c_mapped >> 1;
				prg_swap_16k_Coolgirl(c_is_flash, 0, bank);
				bank = coolgirl.prg.a_mapped >> 1;
				prg_swap_16k_Coolgirl(a_is_flash, 2, bank);
				break;
			case 4:
				bank = coolgirl.prg.a_mapped;
				prg_swap_8k_Coolgirl(a_is_flash, 0, bank);
				bank = coolgirl.prg.b_mapped;
				prg_swap_8k_Coolgirl(b_is_flash, 1, bank);
				bank = coolgirl.prg.c_mapped;
				prg_swap_8k_Coolgirl(c_is_flash, 2, bank);
				bank = coolgirl.prg.d_mapped;
				prg_swap_8k_Coolgirl(d_is_flash, 3, bank);
				break;
			case 5:
				bank = coolgirl.prg.c_mapped;
				prg_swap_8k_Coolgirl(c_is_flash, 0, bank);
				bank = coolgirl.prg.b_mapped;
				prg_swap_8k_Coolgirl(b_is_flash, 1, bank);
				bank = coolgirl.prg.a_mapped;
				prg_swap_8k_Coolgirl(a_is_flash, 2, bank);
				bank = coolgirl.prg.d_mapped;
				prg_swap_8k_Coolgirl(d_is_flash, 3, bank);
				break;
			case 6:
				bank = coolgirl.prg.b_mapped >> 2;
				prg_swap_32k_Coolgirl(b_is_flash, bank);
				break;
			case 7:
				bank = coolgirl.prg.a_mapped >> 2;
				prg_swap_32k_Coolgirl(a_is_flash, bank);
				break;
		}
	}

	if (coolgirl.map_rom_on_6000) {
		bank = coolgirl.prg.b6000_mapped;
		_control_bank(bank, info.prg.rom.max.banks_8k)
		coolgirltmp.prg_6000 = prg_pnt(bank << 13);
	} else if (coolgirl.wram.enabled) {
		prg.ram_plus_8k = &prg.ram_plus[coolgirl.wram.page << 13];
	}
}
INLINE static DBWORD prg_mapped_Coolgirl(BYTE bank) {
	return ((coolgirl.prg.base << 1) | (bank & (((~coolgirl.prg.mask & 0x7F) << 1) | 1)));
}
INLINE static BYTE prg_mapped_is_flash_Coolgirl(DBWORD mapped) {
	return (mapped >= 0x20000 - SAVE_FLASH_SIZE / 1024 / 8);
}
INLINE static void prg_swap_8k_Coolgirl(BYTE flash, BYTE at, DBWORD value) {
	BYTE *pnt;

	if (flash) {
		pnt = coolgirltmp.save_flash;
		control_bank(SAVE_FLASH_MAX_8K)
	} else {
		pnt = prg_rom();
		control_bank(info.prg.rom.max.banks_8k)
	}
	map_prg_rom_8k(1, at, value);
	prg.rom_8k[at] = pnt + (mapper.rom_map_to[at] << 13);
}
INLINE static void prg_swap_16k_Coolgirl(BYTE flash, BYTE at, DBWORD value) {
	BYTE *pnt;

	if (flash) {
		pnt = coolgirltmp.save_flash;
		control_bank(SAVE_FLASH_MAX_16K)
	} else {
		pnt = prg_rom();
		control_bank(info.prg.rom.max.banks_16k)
	}
	map_prg_rom_8k(2, at, value);
	prg.rom_8k[at] = pnt + (mapper.rom_map_to[at] << 13);
	at++;
	prg.rom_8k[at] = pnt + (mapper.rom_map_to[at] << 13);
}
INLINE static void prg_swap_32k_Coolgirl(BYTE flash, DBWORD value) {
	BYTE *pnt;

	if (flash) {
		pnt = coolgirltmp.save_flash;
		control_bank(SAVE_FLASH_MAX_32K)
	} else {
		pnt = prg_rom();
		control_bank(info.prg.rom.max.banks_32k)
	}
	map_prg_rom_8k(4, 0, value);
	prg.rom_8k[0] = pnt + (mapper.rom_map_to[0] << 13);
	prg.rom_8k[1] = pnt + (mapper.rom_map_to[1] << 13);
	prg.rom_8k[2] = pnt + (mapper.rom_map_to[2] << 13);
	prg.rom_8k[3] = pnt + (mapper.rom_map_to[3] << 13);
}
INLINE static void chr_fix_Coolgirl(void) {
	BYTE chr_shift = (coolgirl.mapper == 24) && (coolgirl.flags & 0x02);
	DBWORD mask = (((((~coolgirl.chr.mask & 0x1F) + 1) * 0x2000) / 0x400) - 1) >> chr_shift;
	DBWORD bank[7];

	switch (coolgirl.chr.mode & 0x07) {
		default:
		case 0:
			bank[0] = (coolgirl.chr.a >> 3 >> chr_shift) & (mask >> 3);
			_control_bank(bank[0], info.chr.rom.max.banks_8k)
			bank[0] <<= 3;
			bank[1] = bank[0] | 1;
			bank[2] = bank[0] | 2;
			bank[3] = bank[0] | 3;
			bank[4] = bank[0] | 4;
			bank[5] = bank[0] | 5;
			bank[6] = bank[0] | 6;
			bank[7] = bank[0] | 7;
			break;
		case 1:
			bank[0] = (coolgirl.mapper163.latch >> 2 >> chr_shift) & (mask >> 2);
			_control_bank(bank[0], info.chr.rom.max.banks_4k)
			bank[0] <<= 2;
			bank[4] = bank[0];
			bank[1] = bank[5] = bank[0] | 1;
			bank[2] = bank[6] = bank[0] | 2;
			bank[3] = bank[7] = bank[0] | 3;
			break;
		case 2:
			bank[0] = (coolgirl.chr.a >> 1 >> chr_shift) & (mask >> 1);
			_control_bank(bank[0], info.chr.rom.max.banks_2k)
			bank[0] <<= 1;
			bank[1] = bank[0] | 1;

			bank[2] = (coolgirl.chr.c >> 1 >> chr_shift) & (mask >> 1);
			_control_bank(bank[2], info.chr.rom.max.banks_2k)
			bank[2] <<= 1;
			bank[3] = bank[2] | 1;

			bank[4] = (coolgirl.chr.e >> chr_shift) & mask;
			_control_bank(bank[4], info.chr.rom.max.banks_1k)

			bank[5] = (coolgirl.chr.f >> chr_shift) & mask;
			_control_bank(bank[5], info.chr.rom.max.banks_1k)

			bank[6] = (coolgirl.chr.g >> chr_shift) & mask;
			_control_bank(bank[6], info.chr.rom.max.banks_1k)

			bank[7] = (coolgirl.chr.h >> chr_shift) & mask;
			_control_bank(bank[7], info.chr.rom.max.banks_1k)
			break;
		case 3:
			bank[0] = (coolgirl.chr.e >> chr_shift) & mask;
			_control_bank(bank[0], info.chr.rom.max.banks_1k)

			bank[1] = (coolgirl.chr.f >> chr_shift) & mask;
			_control_bank(bank[1], info.chr.rom.max.banks_1k)

			bank[2] = (coolgirl.chr.g >> chr_shift) & mask;
			_control_bank(bank[2], info.chr.rom.max.banks_1k)

			bank[3] = (coolgirl.chr.h >> chr_shift) & mask;
			_control_bank(bank[3], info.chr.rom.max.banks_1k)

			bank[4] = (coolgirl.chr.a >> 1 >> chr_shift) & (mask >> 1);
			_control_bank(bank[4], info.chr.rom.max.banks_2k)
			bank[4] <<= 1;
			bank[5] = bank[4] | 1;

			bank[6] = (coolgirl.chr.c >> 1 >> chr_shift) & (mask >> 1);
			_control_bank(bank[6], info.chr.rom.max.banks_2k)
			bank[6] <<= 1;
			bank[7] = bank[6] | 1;
			break;
		case 4:
			bank[0] = (coolgirl.chr.a >> 2 >> chr_shift) & (mask >> 2);
			_control_bank(bank[0], info.chr.rom.max.banks_4k)
			bank[0] <<= 2;
			bank[1] = bank[0] | 1;
			bank[2] = bank[0] | 2;
			bank[3] = bank[0] | 3;

			bank[4] = (coolgirl.chr.e >> 2 >> chr_shift) & (mask >> 2);
			_control_bank(bank[4], info.chr.rom.max.banks_4k)
			bank[4] <<= 2;
			bank[5] = bank[4] | 1;
			bank[6] = bank[4] | 2;
			bank[7] = bank[4] | 3;
			break;
		case 5:
			if (!coolgirl.mmc24.latch0) {
				bank[0] = (coolgirl.chr.a >> 2 >> chr_shift) & (mask >> 2);
				_control_bank(bank[0], info.chr.rom.max.banks_4k)
				bank[0] <<= 2;
				bank[1] = bank[0] | 1;
				bank[2] = bank[0] | 2;
				bank[3] = bank[0] | 3;
			} else {
				bank[0] = (coolgirl.chr.b >> 2 >> chr_shift) & (mask >> 2);
				_control_bank(bank[0], info.chr.rom.max.banks_4k)
				bank[0] <<= 2;
				bank[1] = bank[0] | 1;
				bank[2] = bank[0] | 2;
				bank[3] = bank[0] | 3;
			}
			if (!coolgirl.mmc24.latch1) {
				bank[4] = (coolgirl.chr.e >> 2 >> chr_shift) & (mask >> 2);
				_control_bank(bank[0], info.chr.rom.max.banks_4k)
				bank[4] <<= 2;
				bank[5] = bank[4] | 1;
				bank[6] = bank[4] | 2;
				bank[7] = bank[4] | 3;
			} else {
				bank[4] = (coolgirl.chr.f >> 2 >> chr_shift) & (mask >> 2);
				_control_bank(bank[0], info.chr.rom.max.banks_4k)
				bank[4] <<= 2;
				bank[5] = bank[4] | 1;
				bank[6] = bank[4] | 2;
				bank[7] = bank[4] | 3;
			}
			break;
		case 6:
			bank[0] = (coolgirl.chr.a >> 1 >> chr_shift) & (mask >> 1);
			_control_bank(bank[0], info.chr.rom.max.banks_2k)
			bank[0] <<= 1;
			bank[1] = bank[0] | 1;

			bank[2] = (coolgirl.chr.c >> 1 >> chr_shift) & (mask >> 1);
			_control_bank(bank[2], info.chr.rom.max.banks_2k)
			bank[2] <<= 1;
			bank[3] = bank[2] | 1;

			bank[4] = (coolgirl.chr.e >> 1 >> chr_shift) & (mask >> 1);
			_control_bank(bank[4], info.chr.rom.max.banks_2k)
			bank[4] <<= 1;
			bank[5] = bank[4] | 1;

			bank[6] = (coolgirl.chr.g >> 1 >> chr_shift) & (mask >> 1);
			_control_bank(bank[6], info.chr.rom.max.banks_2k)
			bank[6] <<= 1;
			bank[7] = bank[6] | 1;
			break;
		case 7:
			bank[0] = (coolgirl.chr.a >> chr_shift) & mask;
			_control_bank(bank[0], info.chr.rom.max.banks_1k)

			bank[1] = (coolgirl.chr.b >> chr_shift) & mask;
			_control_bank(bank[1], info.chr.rom.max.banks_1k)

			bank[2] = (coolgirl.chr.c >> chr_shift) & mask;
			_control_bank(bank[2], info.chr.rom.max.banks_1k)

			bank[3] = (coolgirl.chr.d >> chr_shift) & mask;
			_control_bank(bank[3], info.chr.rom.max.banks_1k)

			bank[4] = (coolgirl.chr.e >> chr_shift) & mask;
			_control_bank(bank[4], info.chr.rom.max.banks_1k)

			bank[5] = (coolgirl.chr.f >> chr_shift) & mask;
			_control_bank(bank[5], info.chr.rom.max.banks_1k)

			bank[6] = (coolgirl.chr.g >> chr_shift) & mask;
			_control_bank(bank[6], info.chr.rom.max.banks_1k)

			bank[7] = (coolgirl.chr.h >> chr_shift) & mask;
			_control_bank(bank[7], info.chr.rom.max.banks_1k)
			break;
	}
	chr.bank_1k[0] = chr_pnt(bank[0] << 10);
	chr.bank_1k[1] = chr_pnt(bank[1] << 10);
	chr.bank_1k[2] = chr_pnt(bank[2] << 10);
	chr.bank_1k[3] = chr_pnt(bank[3] << 10);
	chr.bank_1k[4] = chr_pnt(bank[4] << 10);
	chr.bank_1k[5] = chr_pnt(bank[5] << 10);
	chr.bank_1k[6] = chr_pnt(bank[6] << 10);
	chr.bank_1k[7] = chr_pnt(bank[7] << 10);
}
INLINE static void mirroring_fix_Coolgirl(void) {
	if (coolgirl.fscreen) {
		mirroring_FSCR();
	} else  {
		// Mapper #189?
		if (!((coolgirl.mapper == 20) && (coolgirl.flags & 0x01))) {
			switch (coolgirl.mirroring) {
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
INLINE static void flash_write_Coolgirl(WORD address, BYTE value) {
	if (coolgirl.flash.state < 10) {
		coolgirl.flash.buffer_a[coolgirl.flash.state] = address & 0xFFF;
		coolgirl.flash.buffer_v[coolgirl.flash.state] = value;
		coolgirl.flash.state++;

		// sector erase
		if ((coolgirl.flash.state == 6) &&
			(coolgirl.flash.buffer_a[0] == 0x0AAA) && (coolgirl.flash.buffer_v[0] == 0xAA) &&
			(coolgirl.flash.buffer_a[1] == 0x0555) && (coolgirl.flash.buffer_v[1] == 0x55) &&
			(coolgirl.flash.buffer_a[2] == 0x0AAA) && (coolgirl.flash.buffer_v[2] == 0x80) &&
			(coolgirl.flash.buffer_a[3] == 0x0AAA) && (coolgirl.flash.buffer_v[3] == 0xAA) &&
			(coolgirl.flash.buffer_a[4] == 0x0555) && (coolgirl.flash.buffer_v[4] == 0x55) &&
			(coolgirl.flash.buffer_v[5] == 0x30)) {
			DBWORD sector_address = coolgirl.prg.a_mapped * 0x2000;

			for (DBWORD i = sector_address; i < sector_address + 128 * 1024; i++) {
				coolgirltmp.save_flash[i % SAVE_FLASH_SIZE] = 0xFF;
			}
			//printf("Flash sector erased: %08x - %08x\n", sector_address, sector_address + 128 * 1024 - 1);
			coolgirl.flash.state = 0;
		}

		// writing byte
		if ((coolgirl.flash.state == 4) &&
			(coolgirl.flash.buffer_a[0] == 0x0AAA) && (coolgirl.flash.buffer_v[0] == 0xAA) &&
			(coolgirl.flash.buffer_a[1] == 0x0555) && (coolgirl.flash.buffer_v[1] == 0x55) &&
			(coolgirl.flash.buffer_a[2] == 0x0AAA) && (coolgirl.flash.buffer_v[2] == 0xA0)) {
			DBWORD sector_address = coolgirl.prg.a_mapped * 0x2000;
			DBWORD flash_addr = sector_address + (address % 0x8000);

			if (coolgirltmp.save_flash[flash_addr % SAVE_FLASH_SIZE] != 0xFF) {
				//printf("Error flash sector is not erased: %08x\n", flash_addr);
			}
			coolgirltmp.save_flash[flash_addr % SAVE_FLASH_SIZE] = value;

			if (address % 0x2000 == 0) {
				//printf("Flash sector writed: %08x\n", flash_addr);
			}
			coolgirl.flash.state = 0;
		}

		// enter CFI mode
		if ((coolgirl.flash.state == 1) &&
			(coolgirl.flash.buffer_a[0] == 0x0AAA) && (coolgirl.flash.buffer_v[0] == 0x98)) {
			coolgirl.cfi_mode = 1;
			prg_fix_Coolgirl();
			coolgirl.flash.state = 0;
			return;
		}
	}
	if (value == 0xF0) {
		coolgirl.flash.state = 0;
		coolgirl.cfi_mode = 0;
		prg_fix_Coolgirl();
	}
}

INLINE static void mapper00_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #342 - Coolgirl
	switch (address & 0x0007) {
		case 0:
			coolgirl.prg.base = (coolgirl.prg.base & 0xFF) | (value << 8);
			break;
		case 1:
			coolgirl.prg.base = (coolgirl.prg.base & 0xFF00) | value;
			break;
		case 2:
			coolgirl.chr.mask = (coolgirl.chr.mask & 0x1F) | ((value & 0x80) >> 2);
			coolgirl.prg.mask = value & 0x7F;
			break;
		case 3:
			coolgirl.prg.mode = value >> 5;
			coolgirl.chr.a = (coolgirl.chr.a & 0x07) | (value << 3);
			break;
		case 4:
			coolgirl.chr.mode = value >> 5;
			coolgirl.chr.mask = (coolgirl.chr.mask & 0x20) | (value & 0x1F);
			break;
		case 5:
			coolgirl.chr.a = (coolgirl.chr.a & 0xFF) | ((value & 0x80) << 1);
			coolgirl.prg.a = (coolgirl.prg.a & 0xC1) | ((value & 0x7C) >> 1);
			coolgirl.wram.page = value & 0x03;
			break;
		case 6:
			coolgirl.flags = value >> 5;
			coolgirl.mapper = (coolgirl.mapper & 0x20) | (value & 0x1F);
			break;
		case 7:
			coolgirl.lock = value >> 7;
			coolgirl.mapper = (coolgirl.mapper & 0x1F) | ((value & 0x40) >> 1);
			coolgirl.fscreen = (value & 0x20) >> 5;
			coolgirl.mirroring = (value & 0x18) >> 3;
			coolgirl.flash.can_write = (value & 0x04) >> 2;
			coolgirl.chr.can_write = (value & 0x02) >> 1;
			coolgirl.wram.enabled = value & 0x01;
			switch (coolgirl.mapper) {
				case 14:
					// Mapper #65 - Irem's H3001
					coolgirl.prg.b = 1;
					break;
				case 17:
					// MMC2
					coolgirl.prg.b = 0xFD;
					break;
				case 23:
					// Mapper #42
					coolgirl.map_rom_on_6000 = 1;
					break;
			}
			break;
	}
}
INLINE static void mapper06_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #163
	if (address == 0x5101) {
		if (coolgirl.mapper163.r4 && !value) {
			coolgirl.mapper163.r5 ^= 1;
		}
		coolgirl.mapper163.r4 = value;
	} else if ((address == 0x5100) && (value == 0x06)) {
		coolgirl.prg.mode = coolgirl.prg.mode & 0xFE;
		coolgirl.prg.b = 12;
	} else {
		if ((address & 0x7000) == 0x5000) {
			switch ((address & 0x0300) >> 8) {
				case 2:
					coolgirl.prg.mode |= 1;
					coolgirl.prg.a = (coolgirl.prg.a & 0x3F) | ((value & 0x03) << 6);
					coolgirl.mapper163.r0 = value;
					break;
				case 0:
					coolgirl.prg.mode |= 1;
					coolgirl.prg.a = (coolgirl.prg.a & 0xC3) | ((value & 0x0F) << 2);
					coolgirl.chr.mode = (coolgirl.chr.mode & 0xFE) | (value >> 7);
					coolgirl.mapper163.r1 = value;
					break;
				case 3:
					coolgirl.mapper163.r2 = value;
					break;
				case 1:
					coolgirl.mapper163.r3 = value;
					break;
			}
		}
	}
}
INLINE static void mapper12_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #87
	if ((address & 0x6000) == 0x6000) {
		coolgirl.chr.a = (coolgirl.chr.a & 0xE7) | ((value & 0x01) << 4) | ((value & 0x02) << 2);
	}
}
INLINE static void mapper15_cpu_wr_low(WORD address, BYTE value) {
	// MMC5 (not really)
	switch (address) {
		case 0x5105:
			if (value == 0xFF) {
				coolgirl.fscreen = 1;
			} else {
				coolgirl.fscreen = 0;
				switch (((value >> 2) & 0x01) | ((value >> 3) & 0x02)) {
					case 0:
						coolgirl.mirroring = 2;
						break;
					case 1:
						coolgirl.mirroring = 0;
						break;
					case 2:
						coolgirl.mirroring = 1;
						break;
					case 3:
						coolgirl.mirroring = 3;
						break;
				}
			}
			break;
		case 0x5115:
			coolgirl.prg.a = (value & 0x1E);
			coolgirl.prg.b = (value & 0x1E) | 1;
			break;
		case 0x5116:
			coolgirl.prg.c = value & 0x1F;
			break;
		case 0x5117:
			coolgirl.prg.d = value & 0x1F;
			break;
		case 0x5120:
			coolgirl.chr.a = value;
			break;
		case 0x5121:
			coolgirl.chr.b = value;
			break;
		case 0x5122:
			coolgirl.chr.c = value;
			break;
		case 0x5123:
			coolgirl.chr.d = value;
			break;
		case 0x5128:
			coolgirl.chr.e = value;
			break;
		case 0x5129:
			coolgirl.chr.f = value;
			break;
		case 0x512A:
			coolgirl.chr.g = value;
			break;
		case 0x512B:
			coolgirl.chr.h = value;
			break;
		case 0x5203:
			irq.high &= ~EXT_IRQ;
			coolgirl.mmc5.irq.out = 0;
			coolgirl.mmc5.irq.line = value;
			break;
		case 0x5204:
			irq.high &= ~EXT_IRQ;
			coolgirl.mmc5.irq.out = 0;
			coolgirl.mmc5.irq.enabled = (value & 0x80) >> 7;
			break;
	}
}
INLINE static void mapper20_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #189
	if ((coolgirl.flags & 0x02) &&  (address >= 0x4120)) {
		coolgirl.prg.a = (coolgirl.prg.a & 0xC3) | ((value & 0x0F) << 2) | ((value & 0xF0) >> 2);
	}
}
INLINE static void mapper27_cpu_wr_low(WORD address, BYTE value) {
	// Mappers #79 and #146 - NINA-03/06 and Sachen 3015: (flag0 = 1)
	if ((address & 0x6100) == 0x4100) {
		coolgirl.chr.a = (coolgirl.chr.a & 0xC7) | ((value & 0x07) << 3);
		coolgirl.prg.a = (coolgirl.prg.a & 0xFB) | ((value & 0x08) >> 1);
	}
}
INLINE static void mapper28_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #133
	if ((address & 0x6100) == 0x4100) {
		coolgirl.chr.a = (coolgirl.chr.a & 0xE7) | ((value & 0x03) << 3);
		coolgirl.prg.a = (coolgirl.prg.a & 0xFB) | (value & 0x04);
	}
}
INLINE static void mapper31_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #184
	if ((address & 0x6000) == 0x6000) {
		coolgirl.chr.a = (coolgirl.chr.a & 0xE3) | ((value & 0x07) << 2);
		coolgirl.chr.e = (coolgirl.chr.e & 0xE3) | ((value & 0x30) >> 2) | 0x10;
	}
}
INLINE static void mapper32_cpu_wr_low(WORD address, BYTE value) {
	// Mapper #38
	if ((address & 0x7000) == 0x7000) {
		coolgirl.prg.a = (coolgirl.prg.a & 0xF7) | ((value & 0x03) << 2);
		coolgirl.chr.a = (coolgirl.chr.a & 0xE7) | ((value & 0x0C) << 1);
	}
}

INLINE static void mapper01_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #2 - UxROM
	// flag0 - mapper #71 - for Fire Hawk only.
	// other mapper-#71 games are UxROM
	if (!(coolgirl.flags & 0x01) || ((address & 0x7000) != 0x1000)) {
		coolgirl.prg.a = (coolgirl.prg.a & 0xC1) | ((value & 0x1F) << 1);
		if (coolgirl.flags & 0x02) {
			coolgirl.mirroring = 0x02 | (value >> 7);
			coolgirl.chr.a = (coolgirl.chr.a & 0xFC) | ((value & 0x60) >> 5);
		}
	} else {
		coolgirl.mirroring = 0x02 | ((value >> 4) & 0x01);
	}
}
INLINE static void mapper02_cpu_wr_high(WORD, BYTE value) {
	// Mapper #3 - CNROM
	coolgirl.chr.a = (coolgirl.chr.a & 0x07) | ((value & 0x1F) << 3);
}
INLINE static void mapper03_cpu_wr_high(WORD, BYTE value) {
	// Mapper #78 - Holy Diver
	coolgirl.prg.a = (coolgirl.prg.a & 0xF1) | ((value & 0x07) << 1);
	coolgirl.chr.a = (coolgirl.chr.a & 0x87) | ((value & 0xF0) >> 1);
	coolgirl.mirroring = ((value >> 3) & 0x01) ^ 1;
}
INLINE static void mapper04_cpu_wr_high(WORD, BYTE value) {
	// Mapper #97 - Irem's TAM-S1
	coolgirl.prg.a = (coolgirl.prg.a & 0xC1) | ((value & 0x1F) << 1);
	coolgirl.mirroring = (value >> 7) ^ 0x01;
}
INLINE static void mapper05_cpu_wr_high(WORD, BYTE value) {
	// Mapper #93 - Sunsoft-2
	coolgirl.prg.a = (coolgirl.prg.a & 0xF1) | ((value & 0x70) >> 3);
	coolgirl.chr.can_write = value & 0x01;
}
INLINE static void mapper07_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #18
	switch (((address & 0x7000) >> 10) | (address & 0x03)) {
		case 0:
			coolgirl.prg.a = (coolgirl.prg.a & 0xF0) | (value & 0x0F);
			break;
		case 1:
			coolgirl.prg.a = (coolgirl.prg.a & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 2:
			coolgirl.prg.b = (coolgirl.prg.b & 0xF0) | (value & 0x0F);
			break;
		case 3:
			coolgirl.prg.b = (coolgirl.prg.b & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 4:
			coolgirl.prg.c = (coolgirl.prg.c & 0xF0) | (value & 0x0F);
			break;
		case 5:
			coolgirl.prg.c = (coolgirl.prg.c & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			coolgirl.chr.a = (coolgirl.chr.a & 0xF0) | (value & 0x0F);
			break;
		case 9:
			coolgirl.chr.a = (coolgirl.chr.a & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 10:
			coolgirl.chr.b = (coolgirl.chr.b & 0xF0) | (value & 0x0F);
			break;
		case 11:
			coolgirl.chr.b = (coolgirl.chr.b & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 12:
			coolgirl.chr.c = (coolgirl.chr.c & 0xF0) | (value & 0x0F);
			break;
		case 13:
			coolgirl.chr.c = (coolgirl.chr.c & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 14:
			coolgirl.chr.d = (coolgirl.chr.d & 0xF0) | (value & 0x0F);
			break;
		case 15:
			coolgirl.chr.d = (coolgirl.chr.d & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 16:
			coolgirl.chr.e = (coolgirl.chr.e & 0xF0) | (value & 0x0F);
			break;
		case 17:
			coolgirl.chr.e = (coolgirl.chr.e & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 18:
			coolgirl.chr.f = (coolgirl.chr.f & 0xF0) | (value & 0x0F);
			break;
		case 19:
			coolgirl.chr.f = (coolgirl.chr.f & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 20:
			coolgirl.chr.g = (coolgirl.chr.g & 0xF0) | (value & 0x0F);
			break;
		case 21:
			coolgirl.chr.g = (coolgirl.chr.g & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 22:
			coolgirl.chr.h = (coolgirl.chr.h & 0xF0) | (value & 0x0F);
			break;
		case 23:
			coolgirl.chr.h = (coolgirl.chr.h & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 24:
			coolgirl.mapper18.irq.latch = (coolgirl.mapper18.irq.latch & 0xFFF0) | (value & 0x0F);
			break;
		case 25:
			coolgirl.mapper18.irq.latch = (coolgirl.mapper18.irq.latch & 0xFF0F) | ((value & 0x0F) << 4);
			break;
		case 26:
			coolgirl.mapper18.irq.latch = (coolgirl.mapper18.irq.latch & 0xF0FF) | ((value & 0x0F) << 8);
			break;
		case 27:
			coolgirl.mapper18.irq.latch = (coolgirl.mapper18.irq.latch & 0x0FFF) | ((value & 0x0F) << 12);
			break;
		case 28:
			coolgirl.mapper18.irq.value = coolgirl.mapper18.irq.latch;
			irq.high &= ~EXT_IRQ;
			break;
		case 29:
			coolgirl.mapper18.irq.control = value & 0x0F;
			irq.high &= ~EXT_IRQ;
			break;
		case 30:
			switch (value & 0x03) {
				case 0:
					coolgirl.mirroring = 1;
					break;
				case 1:
					coolgirl.mirroring = 0;
					break;
				case 2:
					coolgirl.mirroring = 2;
					break;
				case 3:
					coolgirl.mirroring = 3;
					break;
			}
			break;
		case 31:
			break;
	}
}
INLINE static void mapper08_cpu_wr_high(WORD, BYTE value) {
	// Mapper #7 - AxROM, mapper #241 - BNROM
	coolgirl.prg.a = (coolgirl.prg.a & 0xC3) | ((value & 0xF) << 2);
	if (!(coolgirl.flags & 0x01)) {
		coolgirl.mirroring = 0x02 | ((value >> 4) & 0x01);
	}
}
INLINE static void mapper09_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #228 - Cheetahmen II
	coolgirl.prg.a = (coolgirl.prg.a & 0xC3) | ((address & 0x0780) >> 5);
	coolgirl.chr.a = (coolgirl.chr.a & 0x07) | ((address & 0x0007) << 5) | ((value & 0x03) << 3);
	coolgirl.mirroring = (address >> 13) & 0x01;
}
INLINE static void mapper10_cpu_wr_high(WORD, BYTE value) {
	// Mapper #11 - ColorDreams
	coolgirl.prg.a = (coolgirl.prg.a & 0xF3) | ((value & 0x03) << 2);
	coolgirl.chr.a = (coolgirl.chr.a & 0x87) | ((value & 0xF0) >> 1);
}
INLINE static void mapper11_cpu_wr_high(WORD, BYTE value) {
	// Mapper #66 - GxROM
	coolgirl.prg.a = (coolgirl.prg.a & 0xF3) | ((value & 0x30) >> 2);
	coolgirl.chr.a = (coolgirl.chr.a & 0xE7) | ((value & 0x03) << 3);
}
INLINE static void mapper13_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #90 - JY
	switch (address & 0xF000) {
		case 0x8000:
			switch (address & 0x0003) {
				case 0:
					coolgirl.prg.a = (coolgirl.prg.a & 0xC0) | (value & 0x3F);
					break;
				case 1:
					coolgirl.prg.b = (coolgirl.prg.b & 0xC0) | (value & 0x3F);
					break;
				case 2:
					coolgirl.prg.c = (coolgirl.prg.c & 0xC0) | (value & 0x3F);
					break;
				case 3:
					coolgirl.prg.d = (coolgirl.prg.d & 0xC0) | (value & 0x3F);
					break;
			}
			break;
		case 0x9000:
			switch (address & 0x0007) {
				case 0:
					coolgirl.chr.a = value;
					break;
				case 1:
					coolgirl.chr.b = value;
					break;
				case 2:
					coolgirl.chr.c = value;
					break;
				case 3:
					coolgirl.chr.d = value;
					break;
				case 4:
					coolgirl.chr.e = value;
					break;
				case 5:
					coolgirl.chr.f = value;
					break;
				case 6:
					coolgirl.chr.g = value;
					break;
				case 7:
					coolgirl.chr.h = value;
					break;
			}
			break;
		case 0xC000:
			switch (address & 0x0007) {
				case 0:
					if (value & 0x01) {
						coolgirl.mmc3.irq.enabled = 1;
					} else {
						coolgirl.mmc3.irq.enabled = 0;
						irq.high &= ~EXT_IRQ;
					}
					break;
				case 1:
					break;
				case 2:
					coolgirl.mmc3.irq.enabled = 0;
					irq.high &= ~EXT_IRQ;
					break;
				case 3:
					coolgirl.mmc3.irq.enabled = 1;
					break;
				case 4:
					break;
				case 5:
					coolgirl.mmc3.irq.latch = value ^ coolgirl.mapper90.xor;
					coolgirl.mmc3.irq.reload = 1;
					break;
				case 6:
					coolgirl.mapper90.xor = value;
					break;
				case 7:
					break;
			}
			break;
		case 0xD000:
			if ((address & 0x0003) == 0x0001) {
				coolgirl.mirroring = value & 0x03;
			}
			break;
	}

}
INLINE static void mapper14_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #65 - Irem's H3001
	switch (((address & 0x7000) >> 9) | (address & 0x0007)) {
		case 0:
			coolgirl.prg.a = (coolgirl.prg.a & 0xC0) | (value & 0x3F);
			break;
		case 9:
			coolgirl.mirroring = (value >> 7) & 0x01;
			break;
		case 11:
			coolgirl.mapper65.irq.enabled = value >> 7;
			irq.high &= ~EXT_IRQ;
			break;
		case 12:
			coolgirl.mapper65.irq.value = coolgirl.mapper65.irq.latch;
			irq.high &= ~EXT_IRQ;
			break;
		case 13:
			coolgirl.mapper65.irq.latch = (coolgirl.mapper65.irq.latch & 0x00FF) | (value << 8);
			break;
		case 14:
			coolgirl.mapper65.irq.latch = (coolgirl.mapper65.irq.latch & 0xFF00) | value;
			break;
		case 16:
			coolgirl.prg.b = (coolgirl.prg.b & 0xC0) | (value & 0x3F);
			break;
		case 24:
			coolgirl.chr.a = value;
			break;
		case 25:
			coolgirl.chr.b = value;
			break;
		case 26:
			coolgirl.chr.c = value;
			break;
		case 27:
			coolgirl.chr.d = value;
			break;
		case 28:
			coolgirl.chr.e = value;
			break;
		case 29:
			coolgirl.chr.f = value;
			break;
		case 30:
			coolgirl.chr.g = value;
			break;
		case 31:
			coolgirl.chr.h = value;
			break;
		case 32:
			coolgirl.prg.c = (coolgirl.prg.c & 0xC0) | (value & 0x3F);
			break;
	}
}
INLINE static void mapper16_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #1 - MMC1
	// flag0 - 16KB of SRAM (SOROM)
	if (value & 0x80) {
		// reset
		coolgirl.mmc1.load_register = (coolgirl.mmc1.load_register & 0xC0) | 0x20;
		coolgirl.prg.mode = 0;
		coolgirl.prg.c = (coolgirl.prg.c & 0xE0) | 0x1E;
	} else {
		coolgirl.mmc1.load_register = (coolgirl.mmc1.load_register & 0xC0) | ((value & 0x01) << 5) |
			((coolgirl.mmc1.load_register & 0x3E) >> 1);
		if (coolgirl.mmc1.load_register & 0x01) {
			switch ((address >> 13) & 0x0003) {
				case 0:
					if ((coolgirl.mmc1.load_register & 0x18) == 0x18) {
						coolgirl.prg.mode = 0;
						coolgirl.prg.c = (coolgirl.prg.c & 0xE1) | 0x1E;
					} else if ((coolgirl.mmc1.load_register & 0x18) == 0x10) {
						coolgirl.prg.mode = 1;
						coolgirl.prg.c = (coolgirl.prg.c & 0xE1);
					} else {
						coolgirl.prg.mode = 7;
					}
					if (coolgirl.mmc1.load_register & 0x20) {
						coolgirl.chr.mode = 4;
					} else {
						coolgirl.chr.mode = 0;
					}
					coolgirl.mirroring = ((coolgirl.mmc1.load_register >> 1) & 0x03) ^ 0x02;
					break;
				case 1:
					coolgirl.chr.a = (coolgirl.chr.a & 0x83) | ((coolgirl.mmc1.load_register & 0x3E) << 1);
					coolgirl.prg.a = (coolgirl.prg.a & 0xDF) | (coolgirl.mmc1.load_register & 0x20);
					coolgirl.prg.c = (coolgirl.prg.c & 0xDF) | (coolgirl.mmc1.load_register & 0x20);
					break;
				case 2:
					coolgirl.chr.e = (coolgirl.chr.e & 0x83) | ((coolgirl.mmc1.load_register & 0x3E) << 1);
					break;
				case 3:
					coolgirl.prg.a = (coolgirl.prg.a & 0xE1) | (coolgirl.mmc1.load_register & 0x1E);
					coolgirl.wram.enabled = ((coolgirl.mmc1.load_register >> 5) & 0x01) ^ 0x01;
					break;
			}
			coolgirl.mmc1.load_register = 0x20;
		}
	}
}
INLINE static void mapper17_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #9 and #10 - MMC2 and MMC4
	// flag0 - 0=MMC2, 1=MMC4
	switch ((address >> 12) & 0x0007) {
		case 2:
			if (!(coolgirl.flags & 0x01)) {
				// MMC2
				coolgirl.prg.a = (coolgirl.prg.a & 0xF0) | (value & 0x0F);
			} else {
				// MMC4
				coolgirl.prg.a = (coolgirl.prg.a & 0xE1) | ((value & 0x0F) << 1);
			}
			break;
		case 3:
			coolgirl.chr.a = (coolgirl.chr.a & 0x83) | ((value & 0x1F) << 2);
			break;
		case 4:
			coolgirl.chr.b = (coolgirl.chr.b & 0x83) | ((value & 0x1F) << 2);
			break;
		case 5:
			coolgirl.chr.e = (coolgirl.chr.e & 0x83) | ((value & 0x1F) << 2);
			break;
		case 6:
			coolgirl.chr.f = (coolgirl.chr.f & 0x83) | ((value & 0x1F) << 2);
			break;
		case 7:
			coolgirl.mirroring = value & 0x01;
			break;
	}
}
INLINE static void mapper18_cpu_wr_high(WORD, BYTE value) {
	// Mapper #152
	coolgirl.chr.a = (coolgirl.chr.a & 0x87) | ((value & 0x0F) << 3);
	coolgirl.prg.a = (coolgirl.prg.a & 0xF1) | ((value & 0x70) >> 3);
	coolgirl.mirroring = 0x02 | (value >> 7);
}
INLINE static void mapper19_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #73 - VRC3
	switch (address & 0x7000) {
		case 0x0000:
			coolgirl.vrc3.irq.latch = (coolgirl.vrc3.irq.latch & 0xFFF0) | (value & 0x0F);
			break;
		case 0x1000:
			coolgirl.vrc3.irq.latch = (coolgirl.vrc3.irq.latch & 0xFF0F) | ((value & 0x0F) << 4);
			break;
		case 0x2000:
			coolgirl.vrc3.irq.latch = (coolgirl.vrc3.irq.latch & 0xF0FF) | ((value & 0x0F) << 8);
			break;
		case 0x3000:
			coolgirl.vrc3.irq.latch = (coolgirl.vrc3.irq.latch & 0x0FFF) | ((value & 0x0F) << 12);
			break;
		case 0x4000:
			coolgirl.vrc3.irq.control = (coolgirl.vrc3.irq.control & 0xF8) | (value & 0x07);
			if (coolgirl.vrc3.irq.control & 0x02) {
				coolgirl.vrc3.irq.value = coolgirl.vrc3.irq.latch;
			}
			irq.high &= ~EXT_IRQ;
			break;
		case 0x5000:
			coolgirl.vrc3.irq.control = (coolgirl.vrc3.irq.control & 0xFD) | (coolgirl.vrc3.irq.control & 0x01) << 1;
			irq.high &= ~EXT_IRQ;
			break;
		case 0x7000:
			coolgirl.prg.a = (coolgirl.prg.a & 0xF1) | ((value & 0x07) << 1);
			break;
	}
}
INLINE static void mapper20_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #4 - MMC3/MMC6
	// flag0 - TxSROM
	// flag1 - mapper #189
	switch (((address & 0x6000) >> 12) | (address & 0x0001)) {
		case 0:
			coolgirl.mmc3.internal = (coolgirl.mmc3.internal & 0xF8) | (value & 0x07);
			if (!(coolgirl.flags & 0x02) && !(coolgirl.flags & 0x04)) {
				coolgirl.prg.mode = value & 0x40 ? 5 : 4;
			}
			if (!(coolgirl.flags & 0x04)) {
				coolgirl.chr.mode = (value & 0x80) ? 3 : 2;
			}
			break;
		case 1:
			switch (coolgirl.mmc3.internal & 0x07) {
				case 0:
					coolgirl.chr.a = value;
					break;
				case 1:
					coolgirl.chr.c = value;
					break;
				case 2:
					coolgirl.chr.e = value;
					break;
				case 3:
					coolgirl.chr.f = value;
					break;
				case 4:
					coolgirl.chr.g = value;
					break;
				case 5:
					coolgirl.chr.h = value;
					break;
				case 6:
					if (!(coolgirl.flags & 0x02)) {
						coolgirl.prg.a = value;
					}
					break;
				case 7:
					if (!(coolgirl.flags & 0x02)) {
						coolgirl.prg.b = value;
					}
					break;
			}
			// mapper #189
			if (coolgirl.flags & 0x01) {
				switch (coolgirl.mmc3.internal & 0x07) {
					case 0:
					case 1:
						if (coolgirl.chr.mode == 2) {
							const BYTE slot = (coolgirl.mmc3.internal & 0x07) << 1;

							ntbl.bank_1k[slot] = &ntbl.data[((value >> 7) ^ 0x01) << 10];
							ntbl.bank_1k[slot | 0x01] = ntbl.bank_1k[slot];
						}
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						if (coolgirl.chr.mode == 3) {
							ntbl.bank_1k[(coolgirl.mmc3.internal & 0x07) - 2] = &ntbl.data[((value >> 7) ^ 0x01) << 10];
						}
						break;
				}
			}
			break;
		case 2:
			if (!(coolgirl.flags & 0x04)) {
				coolgirl.mirroring = value & 0x01;
			}
			break;
		case 3:
			break;
		case 4:
			coolgirl.mmc3.irq.latch = value;
			break;
		case 5:
			coolgirl.mmc3.irq.reload = 1;
			break;
		case 6:
			coolgirl.mmc3.irq.enabled = 0;
			irq.high &= ~EXT_IRQ;
			break;
		case 7:
			coolgirl.mmc3.irq.enabled = 1;
			break;
	}
}
INLINE static void mapper21_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #112
	switch (address & 0xE000) {
		case 0x8000:
			coolgirl.mapper112.internal = (coolgirl.mapper112.internal & 0xF8) | (value & 0x07);
			break;
		case 0xA000:
			switch (coolgirl.mapper112.internal & 0x07) {
				case 0:
					coolgirl.prg.a = (coolgirl.prg.a & 0xC0) | (value & 0x3F);
					break;
				case 1:
					coolgirl.prg.b = (coolgirl.prg.b & 0xC0) | (value & 0x3F);
					break;
				case 2:
					coolgirl.chr.a = value;
					break;
				case 3:
					coolgirl.chr.c = value;
					break;
				case 4:
					coolgirl.chr.e = value;
					break;
				case 5:
					coolgirl.chr.f = value;
					break;
				case 6:
					coolgirl.chr.g = value;
					break;
				case 7:
					coolgirl.chr.h = value;
					break;
			}
			break;
		case 0xC000:
			break;
		case 0xE000:
			coolgirl.mirroring = value & 0x01;
			break;
	}
}
INLINE static void mapper22_cpu_wr_high(WORD address, BYTE value) {
	// Mappers #33 + #48 - Taito
	// flag0=0 - #33, flag0=1 - #48
	switch (((address & 0x6000) >> 11) | (address & 0x0003)) {
		case 0:
			coolgirl.prg.a = (coolgirl.prg.a & 0xC0) | (value & 0x3F);
			if (!(coolgirl.flags & 0x01)) {
				coolgirl.mirroring = (value >> 6) & 0x01;
			}
			break;
		case 1:
			coolgirl.prg.b = (coolgirl.prg.b & 0xC0) | (value & 0x3F);
			break;
		case 2:
			coolgirl.chr.a = value << 1;
			break;
		case 3:
			coolgirl.chr.c = value << 1;
			break;
		case 4:
			coolgirl.chr.e = value;
			break;
		case 5:
			coolgirl.chr.f = value;
			break;
		case 6:
			coolgirl.chr.g = value;
			break;
		case 7:
			coolgirl.chr.h = value;
			break;
		case 12:
			if (coolgirl.flags & 0x01) {
				coolgirl.mirroring = (value >> 6) & 0x01;
			}
			break;
		case 8:
			coolgirl.mmc3.irq.latch = value ^ 0xFF;
			break;
		case 9:
			coolgirl.mmc3.irq.reload = 1;
			break;
		case 10:
			coolgirl.mmc3.irq.enabled = 1;
			break;
		case 11:
			coolgirl.mmc3.irq.enabled = 0;
			irq.high &= ~EXT_IRQ;
			break;
	}
}
INLINE static void mapper23_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #42
	switch (((address >> 12) & 0x0004) | (address & 0x0003)) {
		case 0:
			coolgirl.chr.a = (coolgirl.chr.a & 0xE0) | ((value & 0x1F) << 3);
			break;
		case 4:
			coolgirl.prg.b6000 = (coolgirl.prg.b6000 & 0xF0) | (value & 0x0F);
			break;
		case 5:
			coolgirl.mirroring = (value >> 3) & 0x01;
			break;
		case 6:
			coolgirl.mapper42.irq.enabled = (value & 0x02) >> 1;
			if (!coolgirl.mapper42.irq.enabled) {
				coolgirl.mapper42.irq.value = 0;
				irq.high &= ~EXT_IRQ;
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
		((coolgirl.flags & 0x01 ? vrc_2b_low : vrc_2b_hi) << 1) |
		(coolgirl.flags & 0x01 ? vrc_2b_hi : vrc_2b_low)) {
		case 0:
		case 1:
		case 2:
		case 3:
			coolgirl.prg.a = (coolgirl.prg.a & 0xE0) | (value & 0x1F);
			break;
		case 4:
		case 5:
			// VRC2 - using games are usually well - behaved and only write 0 or 1 to this register,
			// but Wai Wai World in one instance writes $FF instead
			if (value != 0xFF) {
				coolgirl.mirroring = value & 0x03;
			}
			break;
		case 6:
		case 7:
			coolgirl.prg.mode = (coolgirl.prg.mode & 0xFE) | ((value >> 1) & 0x01);
			break;
		case 8:
		case 9:
		case 10:
		case 11:
			coolgirl.prg.b = (coolgirl.prg.b & 0xE0) | (value & 0x1F);
			break;
		case 12:
			coolgirl.chr.a = (coolgirl.chr.a & 0xF0) | (value & 0x0F);
			break;
		case 13:
			coolgirl.chr.a = (coolgirl.chr.a & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 14:
			coolgirl.chr.b = (coolgirl.chr.b & 0xF0) | (value & 0x0F);
			break;
		case 15:
			coolgirl.chr.b = (coolgirl.chr.b & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 16:
			coolgirl.chr.c = (coolgirl.chr.c & 0xF0) | (value & 0x0F);
			break;
		case 17:
			coolgirl.chr.c = (coolgirl.chr.c & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 18:
			coolgirl.chr.d = (coolgirl.chr.d & 0xF0) | (value & 0x0F);
			break;
		case 19:
			coolgirl.chr.d = (coolgirl.chr.d & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 20:
			coolgirl.chr.e = (coolgirl.chr.e & 0xF0) | (value & 0x0F);
			break;
		case 21:
			coolgirl.chr.e = (coolgirl.chr.e & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 22:
			coolgirl.chr.f = (coolgirl.chr.f & 0xF0) | (value & 0x0F);
			break;
		case 23:
			coolgirl.chr.f = (coolgirl.chr.f & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 24:
			coolgirl.chr.g = (coolgirl.chr.g & 0xF0) | (value & 0x0F);
			break;
		case 25:
			coolgirl.chr.g = (coolgirl.chr.g & 0x0F) | ((value & 0x0F) << 4);
			break;
		case 26:
			coolgirl.chr.h = (coolgirl.chr.h & 0xF0) | (value & 0x0F);
			break;
		case 27:
			coolgirl.chr.h = (coolgirl.chr.h & 0x0F) | ((value & 0x0F) << 4);
			break;
	}
	if ((address & 0x7000) == 0x7000) {
		switch ((((coolgirl.flags & 0x01) ? vrc_2b_low : vrc_2b_hi) << 1) | ((coolgirl.flags & 0x01) ? vrc_2b_hi : vrc_2b_low)) {
			case 0:
				coolgirl.vrc4.irq.latch = (coolgirl.vrc4.irq.latch & 0xF0) | (value & 0x0F);
				break;
			case 1:
				coolgirl.vrc4.irq.latch = (coolgirl.vrc4.irq.latch & 0x0F) | ((value & 0x0F) << 4);
				break;
			case 2:
				coolgirl.vrc4.irq.control = (coolgirl.vrc4.irq.control & 0xF8) | (value & 0x07);
				if (coolgirl.vrc4.irq.control & 0x02) {
					coolgirl.vrc4.irq.prescaler_counter = 0;
					coolgirl.vrc4.irq.prescaler = 0;
					coolgirl.vrc4.irq.value = coolgirl.vrc4.irq.latch;
				}
				irq.high &= ~EXT_IRQ;
				break;
			case 3:
				coolgirl.vrc4.irq.control = (coolgirl.vrc4.irq.control & 0xFD) | (coolgirl.vrc4.irq.control & 0x01) << 1;
				irq.high &= ~EXT_IRQ;
				break;
		}
	}
}
INLINE static void mapper25_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #69 - Sunsoft FME-7
	BYTE bank = (address & 0x6000) >> 13;

	if (bank == 0) {
		coolgirl.mapper69.internal = (coolgirl.mapper69.internal & 0xF0) | (value & 0x0F);
	} else if (bank == 1) {
		switch (coolgirl.mapper69.internal & 0x0F) {
			case 0:
				coolgirl.chr.a = value;
				break;
			case 1:
				coolgirl.chr.b = value;
				break;
			case 2:
				coolgirl.chr.c = value;
				break;
			case 3:
				coolgirl.chr.d = value;
				break;
			case 4:
				coolgirl.chr.e = value;
				break;
			case 5:
				coolgirl.chr.f = value;
				break;
			case 6:
				coolgirl.chr.g = value;
				break;
			case 7:
				coolgirl.chr.h = value;
				break;
			case 8:
				coolgirl.wram.enabled = value >> 7;
				coolgirl.map_rom_on_6000 = ((value >> 6) & 0x01) ^ 0x01;
				coolgirl.prg.b6000 = value & 0x3F;
				break;
			case 9:
				coolgirl.prg.a = (coolgirl.prg.a & 0xC0) | (value & 0x3F);
				break;
			case 10:
				coolgirl.prg.b = (coolgirl.prg.b & 0xC0) | (value & 0x3F);
				break;
			case 11:
				coolgirl.prg.c = (coolgirl.prg.c & 0xC0) | (value & 0x3F);
				break;
			case 12:
				coolgirl.mirroring = value & 0x03;
				break;
			case 13:
				coolgirl.mapper69.irq.counter_enabled = value >> 7;
				coolgirl.mapper69.irq.enabled = value & 0x01;
				irq.high &= ~EXT_IRQ;
				break;
			case 14:
				coolgirl.mapper69.irq.value = (coolgirl.mapper69.irq.value & 0xFF00) | value;
				break;
			case 15:
				coolgirl.mapper69.irq.value = (coolgirl.mapper69.irq.value & 0x00FF) | (value << 8);
				break;
		}
	}
}
INLINE static void mapper26_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #32 - Irem's G-101
	switch (address & 0xF000) {
		case 0x8000:
			coolgirl.prg.a = (coolgirl.prg.a & 0xC0) | (value & 0x3F);
			break;
		case 0x9000:
			coolgirl.prg.mode = (coolgirl.prg.mode & 0x06) | ((value >> 1) & 0x01);
			coolgirl.mirroring = value & 0x01;
			break;
		case 0xA000:
			coolgirl.prg.b = (coolgirl.prg.b & 0xC0) | (value & 0x3F);
			break;
		case 0xB000:
			switch (address & 0x0007) {
				case 0:
					coolgirl.chr.a = value;
					break;
				case 1:
					coolgirl.chr.b = value;
					break;
				case 2:
					coolgirl.chr.c = value;
					break;
				case 3:
					coolgirl.chr.d = value;
					break;
				case 4:
					coolgirl.chr.e = value;
					break;
				case 5:
					coolgirl.chr.f = value;
					break;
				case 6:
					coolgirl.chr.g = value;
					break;
				case 7:
					coolgirl.chr.h = value;
					break;
			}
			break;
	}
}
INLINE static void mapper29_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #36 is assigned to TXC's PCB 01-22000-400
	if ((address & 0x7FFE) == 0x7FFE) {
		coolgirl.prg.a = (coolgirl.prg.a & 0xC3) | ((value & 0xF0) >> 2);
		coolgirl.chr.a = (coolgirl.chr.a & 0x87) | ((value & 0x0F) << 3);
	}
}
INLINE static void mapper30_cpu_wr_high(WORD, BYTE value) {
	// Mapper #70
	coolgirl.prg.a = (coolgirl.prg.a & 0xE1) | ((value & 0xF0) >> 3);
	coolgirl.chr.a = (coolgirl.chr.a & 0x87) | ((value & 0x0F) << 3);
}
INLINE static void mapper34_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #75 - VRC1
	switch (address & 0x7000) {
		case 0x0000:
			coolgirl.prg.a = (coolgirl.prg.a & 0xF0) | (value & 0x0F);
			break;
		case 0x1000:
			coolgirl.mirroring = value & 0x01;
			coolgirl.chr.a = (coolgirl.chr.a & 0xBF) | ((value & 0x02) << 5);
			coolgirl.chr.e = (coolgirl.chr.a & 0xBF) | ((value & 0x04) << 4);
			break;
		case 0x2000:
			coolgirl.prg.b = (coolgirl.prg.b & 0xF0) | (value & 0x0F);
			break;
		case 0x4000:
			coolgirl.prg.c = (coolgirl.prg.c & 0xF0) | (value & 0x0F);
			break;
		case 0x6000:
			coolgirl.chr.a = (coolgirl.chr.a & 0xC3) | ((value & 0x0F) << 2);
			break;
		case 0x7000:
			coolgirl.chr.e = (coolgirl.chr.e & 0xC3) | ((value & 0x0F) << 2);
			break;
	}
}
INLINE static void mapper35_cpu_wr_high(WORD address, BYTE value) {
	// Mapper #83 - Cony/Yoko
	switch (address & 0x0300) {
		case 0x0100:
			coolgirl.mirroring = value & 0x03;
			coolgirl.mapper83.irq.enabled_latch = value >> 7;
			break;
		case 0x0200:
			if (!(address & 0x0001)) {
				coolgirl.mapper83.irq.counter = (coolgirl.mapper83.irq.counter & 0xFF00) | value;
				irq.high &= ~EXT_IRQ;
			} else {
				coolgirl.mapper83.irq.enabled = coolgirl.mapper83.irq.enabled_latch;
				coolgirl.mapper83.irq.counter = (coolgirl.mapper83.irq.counter & 0x00FF) | (value << 8);
			}
		break;
	case 0x0300:
		if (!(address & 0x0010))
		{
			switch (address & 0x0003) {
				case 0:
					coolgirl.prg.a = value;
					break;
				case 1:
					coolgirl.prg.b = value;
					break;
				case 2:
					coolgirl.prg.b = value;
					break;
				//case 3:
					//coolgirl.prg.b6000 = value;
					//break;
			}
		} else {
			switch (address & 0x0007) {
				case 0:
					coolgirl.chr.a = value;
					break;
				case 1:
					coolgirl.chr.b = value;
					break;
				case 2:
					coolgirl.chr.c = value;
					break;
				case 3:
					coolgirl.chr.d = value;
					break;
				case 4:
					coolgirl.chr.e = value;
					break;
				case 5:
					coolgirl.chr.f = value;
					break;
				case 6:
					coolgirl.chr.g = value;
					break;
				case 7:
					coolgirl.chr.h = value;
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
				coolgirl.chr.a = (coolgirl.chr.a & 0x81) | ((value & 0x3F) << 1);
				break;
			case 0x1000:
				coolgirl.chr.c = (coolgirl.chr.c & 0x81) | ((value & 0x3F) << 1);
				break;
			case 0x2000:
				coolgirl.chr.e = (coolgirl.chr.e & 0x81) | ((value & 0x3F) << 1);
				break;
			case 0x3000:
				coolgirl.chr.g = (coolgirl.chr.g & 0x81) | ((value & 0x3F) << 1);
				break;
			case 0x4000:
				coolgirl.mapper67.irq.latch = ~coolgirl.mapper67.irq.latch;
				if (coolgirl.mapper67.irq.latch) {
					coolgirl.mapper67.irq.counter = (coolgirl.mapper67.irq.counter & 0x00FF) | (value << 8);
				} else {
					coolgirl.mapper67.irq.counter = (coolgirl.mapper67.irq.counter & 0xFF00) | value;
				}
				break;
			case 0x5000:
				coolgirl.mapper67.irq.latch = 0;
				coolgirl.mapper67.irq.enabled = (value & 0x10) >> 4;
				break;
			case 0x6000:
				coolgirl.mirroring = value & 0x03;
				break;
			case 0x7000:
				coolgirl.chr.g = (coolgirl.chr.g & 0xE1) | ((value & 0x0F) << 1);
				break;
		}
	} else {
		// Interrupt Acknowledge ($8000)
		irq.high &= ~EXT_IRQ;
	}
}
INLINE static void mapper37_cpu_wr_high(WORD, BYTE value) {
	// Mapper #89 - Sunsoft-2 chip on the Sunsoft-3 board
	coolgirl.prg.a = (coolgirl.prg.a & 0xF1) | ((value & 0x70) >> 3);
	coolgirl.chr.a = (coolgirl.chr.a & 0x87) | ((value & 0x80) >> 1) | ((value & 0x07) << 3);
	coolgirl.mirroring = 0x02 | ((value & 0x08) >> 3);
}

INLINE static BYTE mapper06_cpu_rd(WORD address, BYTE openbus) {
	// Mapper #163
	if ((address & 0x7700) == 0x5100) {
		return (coolgirl.mapper163.r2 | coolgirl.mapper163.r0 | coolgirl.mapper163.r1 | ~coolgirl.mapper163.r3);
	}
	if ((address & 0x7700) == 0x5500) {
		return (coolgirl.mapper163.r5 & 0x01 ? coolgirl.mapper163.r2 : coolgirl.mapper163.r1);
	}
	return (openbus);
}
INLINE static BYTE mapper13_cpu_rd(WORD address, BYTE openbus) {
	// Mapper #90 - JY
	if (address == 0x5800) {
		return (coolgirl.mapper90.mul1 * coolgirl.mapper90.mul2) & 0xFF;
	}
	if (address == 0x5801) {
		return ((coolgirl.mapper90.mul1 * coolgirl.mapper90.mul2) >> 8) & 0xFF;
	}
	return (openbus);
}
INLINE static BYTE mapper15_cpu_rd(WORD address, BYTE openbus) {
	// MMC5
	if (address == 0x0204) {
		BYTE value = (coolgirl.mmc5.irq.out << 7) |
			(!coolgirl.mmc5.ppu.rendering || ((coolgirl.mmc5.ppu.scanline + 1) >= 241) ? 0 : 0x40);

		coolgirl.mmc5.irq.out = 0;
		irq.high &= ~EXT_IRQ;
		return (value);
	}
	return (openbus);
}
INLINE static BYTE mapper35_cpu_rd(WORD, BYTE) {
	return (coolgirl.flags & 0x03);
}

INLINE static void mapper07_cpu_every_cycle(void) {
	// Mapper #18
	if (coolgirl.mapper18.irq.control & 0x01) {
		BYTE carry = carry = (coolgirl.mapper18.irq.value & 0x0F) - 1;

		coolgirl.mapper18.irq.value = (coolgirl.mapper18.irq.value & 0xFFF0) | (carry & 0x0F);
		carry = (carry >> 4) & 0x01;
		if (!(coolgirl.mapper18.irq.control & 0x08)) {
			carry = ((coolgirl.mapper18.irq.value >> 4) & 0x0F) - carry;
			//SET_BITS(mapper18_irq_value, "7:4", carry, "3:0");
			coolgirl.mapper18.irq.value = (coolgirl.mapper18.irq.value & 0xFF0F) | ((carry & 0x0F) << 4);
			carry = (carry >> 4) & 0x01;
		}
		if (!(coolgirl.mapper18.irq.control & 0x0C)) {
			carry = ((coolgirl.mapper18.irq.value >> 8) & 0x0F) - carry;
			coolgirl.mapper18.irq.value = (coolgirl.mapper18.irq.value & 0xF0FF) | ((carry & 0x0F) << 8);
			carry = (carry >> 4) & 0x01;
		}
		if (!(coolgirl.mapper18.irq.control & 0x0E)) {
			carry = ((coolgirl.mapper18.irq.value >> 12) & 0x0F) - carry;
			coolgirl.mapper18.irq.value = (coolgirl.mapper18.irq.value & 0x0FFF) | ((carry & 0x0F) << 12);
			carry = (carry >> 4) & 0x01;
		}
		if (carry) {
			irq.high |= EXT_IRQ;
		}
	}
}
INLINE static void mapper14_cpu_every_cycle(void) {
	// Mapper #65 - Irem's H3001
	if (coolgirl.mapper65.irq.enabled) {
		if (coolgirl.mapper65.irq.value != 0) {
			coolgirl.mapper65.irq.value--;
			if (!coolgirl.mapper65.irq.value) {
				irq.high |= EXT_IRQ;
			}
		}
	}
}
INLINE static void mapper19_cpu_every_cycle(void) {
	// Mapper #73 - VRC3
	if (coolgirl.vrc3.irq.control & 0x02) {
		if (coolgirl.vrc3.irq.control & 0x04) {  // 8-bit mode
			coolgirl.vrc3.irq.value = (coolgirl.vrc3.irq.value & 0xFF00) | ((coolgirl.vrc3.irq.value + 1) & 0xFF);
			if ((coolgirl.vrc3.irq.value & 0xFF) == 0) {
				coolgirl.vrc3.irq.value = (coolgirl.vrc3.irq.value & 0xFF00) | (coolgirl.vrc3.irq.latch & 0xFF);
				irq.high |= EXT_IRQ;
			}
		} else { // 16-bit
			coolgirl.vrc3.irq.value += 1;
			if (coolgirl.vrc3.irq.value == 0) {
				coolgirl.vrc3.irq.value = coolgirl.vrc3.irq.latch;
				irq.high |= EXT_IRQ;
			}
		}
	}
}
INLINE static void mapper23_cpu_every_cycle(void) {
	// Mapper #42
	if (coolgirl.mapper42.irq.enabled) {
		coolgirl.mapper42.irq.value++;
		if (coolgirl.mapper42.irq.value >> 15) {
			coolgirl.mapper42.irq.value = 0;
		}
		if (((coolgirl.mapper42.irq.value >> 13) & 0x03) == 0x03) {
			irq.high |= EXT_IRQ;
		} else {
			irq.high &= ~EXT_IRQ;
		}
	}
}
INLINE static void mapper24_cpu_every_cycle(void) {
	// Mapper #23 - VRC2/4
	if (coolgirl.vrc4.irq.control & 0x02) {
		if (coolgirl.vrc4.irq.control & 0x04) {
			coolgirl.vrc4.irq.value++;
			if (coolgirl.vrc4.irq.value == 0) {
				coolgirl.vrc4.irq.value = coolgirl.vrc4.irq.latch;
				irq.high |= EXT_IRQ;
			}
		} else {
			coolgirl.vrc4.irq.prescaler++;
			if ((!(coolgirl.vrc4.irq.prescaler_counter & 0x02) && (coolgirl.vrc4.irq.prescaler == 114))
				|| ((coolgirl.vrc4.irq.prescaler_counter & 0x02) && (coolgirl.vrc4.irq.prescaler == 113))) {
				coolgirl.vrc4.irq.prescaler = 0;
				coolgirl.vrc4.irq.prescaler_counter++;
				if (coolgirl.vrc4.irq.prescaler_counter == 3) {
					coolgirl.vrc4.irq.prescaler_counter = 0;
				}
				coolgirl.vrc4.irq.value++;
				if (coolgirl.vrc4.irq.value == 0) {
					coolgirl.vrc4.irq.value = coolgirl.vrc4.irq.latch;
					irq.high |= EXT_IRQ;
				}
			}
		}
	}
}
INLINE static void mapper25_cpu_every_cycle(void) {
	// Mapper #69 - Sunsoft FME-7
	if (coolgirl.mapper69.irq.counter_enabled) {
		coolgirl.mapper69.irq.value--;
		if (coolgirl.mapper69.irq.value == 0xFFFF) {
			irq.high |= EXT_IRQ;
		}
	}
}
INLINE static void mapper35_cpu_every_cycle(void) {
	// Mapper #83 - Cony/Yoko
	if (coolgirl.mapper83.irq.enabled) {
		if (coolgirl.mapper83.irq.counter == 0) {
			irq.high |= EXT_IRQ;
		}
		coolgirl.mapper83.irq.counter--;
	}
}
INLINE static void mapper36_cpu_every_cycle(void) {
	// Mapper #67 - Sunsoft-3
	if (coolgirl.mapper67.irq.enabled) {
		coolgirl.mapper67.irq.counter--;
		if (coolgirl.mapper67.irq.counter == 0xFFFF) {
			irq.high |= EXT_IRQ;
			coolgirl.mapper67.irq.enabled = 0;
		}
	}
}
