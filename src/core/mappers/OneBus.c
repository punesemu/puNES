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

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "info.h"
#include "irqA12.h"
#include "save_slot.h"
#include "emu.h"
#include "OneBus.h"
#include "serial_devices_interface.h"

INLINE static void chr_swap_OneBus(BYTE **banks, BYTE *base, BYTE bit4pp, BYTE extended, WORD EVA, WORD mmask, WORD mblock);
INLINE static void irq_tick_OneBus(void);

struct _onebustmp {
	BYTE *prg_6000;
	struct _onebustmp_chr {
		size_t size;
		BYTE *data;
		BYTE *low;
		BYTE *low16;
		BYTE *high;
		BYTE *high16;
		struct _onebus_chr_bank {
			struct _onebus_chr_bank_low {
				BYTE *r2007[8];
				BYTE *bg[8];
				BYTE *spr[8];
			} low;
			struct _onebus_chr_bank_high {
				BYTE *r2007[8];
				BYTE *bg[8];
				BYTE *spr[8];
			} high;
		} bank;
	} chr;
} onebustmp;
_onebus onebus;

void map_init_OneBus(void) {
	EXTCL_AFTER_MAPPER_INIT(OneBus);
	EXTCL_MAPPER_QUIT(OneBus);
	EXTCL_CPU_WR_MEM(OneBus);
	EXTCL_CPU_RD_MEM(OneBus);
	EXTCL_SAVE_MAPPER(OneBus);
	EXTCL_WR_PPU_REG(OneBus);
	EXTCL_WR_APU(OneBus);
	EXTCL_RD_APU(OneBus);
	EXTCL_RD_CHR(OneBus);
	EXTCL_CPU_EVERY_CYCLE(OneBus);
	EXTCL_IRQ_A12_CLOCK(OneBus);
	EXTCL_PPU_000_TO_34X(OneBus);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&onebus;
	mapper.internal_struct_size[0] = sizeof(onebus);

	memset(&irqA12, 0x00, sizeof(irqA12));

	if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		memset(&onebustmp, 0x00, sizeof(onebustmp));

		if (mapper.write_vram) {
			onebustmp.chr.data = prg_pnt(0);
			onebustmp.chr.size = prg_size();
		} else {
			onebustmp.chr.data = chr_pnt(0);
			onebustmp.chr.size = chr_size();
		}

		onebustmp.chr.size = emu_power_of_two(onebustmp.chr.size);

		{
			size_t size = onebustmp.chr.size >> 1;

			onebustmp.chr.low = malloc(size);
			onebustmp.chr.high = malloc(size);
			if ((info.mapper.ext_console_type == VT09) || (info.mapper.ext_console_type == VT369)) {
				onebustmp.chr.low16 = malloc(size);
				onebustmp.chr.high16 = malloc(size);
			}
		}

		{
			int i = 0;

			for (i = 0; i < (int)LENGTH(onebus.gpio); i++) {
				onebus.gpio[i] = gpio_onebus_create();
			}
		}
	}

	if (info.reset >= HARD) {
		size_t i = 0;

		for (i = 0; i < onebustmp.chr.size; i++) {
			size_t address = (i & 0x0F) | ((i >> 1) & ~0x0F);

			if (i & 0x10) {
				onebustmp.chr.high[address] = onebustmp.chr.data[i];
			} else {
				onebustmp.chr.low[address] = onebustmp.chr.data[i];
			}
		}
		if ((info.mapper.ext_console_type == VT09) || (info.mapper.ext_console_type == VT369)) {
			for (i = 0; i < onebustmp.chr.size; i++) {
				if (i & 0x01) {
					onebustmp.chr.high16[i >> 1] = onebustmp.chr.data[i];
				} else {
					onebustmp.chr.low16[i >> 1] = onebustmp.chr.data[i];
				}
			}
		}
		memset(&onebus.reg.ppu, 0x00, sizeof(onebus.reg.ppu));
		memset(&onebus.reg.cpu, 0x00, sizeof(onebus.reg.cpu));
		memset(&onebus.reg.apu, 0x00, sizeof(onebus.reg.apu));
		memset(&onebus.pcm, 0x00, sizeof(onebus.pcm));

		onebus.pcm.clock = 0xE1;
	}

	if (info.prg.ram.banks_8k_plus == 0) {
		info.prg.ram.banks_8k_plus = 1;
	}

	onebus.relative_8k = 0;
	onebus.reg.ppu[0x10] = 0x00;
	onebus.reg.ppu[0x12] = 0x04;
	onebus.reg.ppu[0x13] = 0x05;
	onebus.reg.ppu[0x14] = 0x06;
	onebus.reg.ppu[0x15] = 0x07;
	onebus.reg.ppu[0x16] = 0x00;
	onebus.reg.ppu[0x17] = 0x02;
	onebus.reg.ppu[0x18] = 0x00;
	onebus.reg.ppu[0x1A] = 0x00;
	onebus.reg.cpu[0x00] = 0x00;
	onebus.reg.cpu[0x05] = 0x00;
	onebus.reg.cpu[0x07] = 0x00;
	onebus.reg.cpu[0x08] = 0x01;
	onebus.reg.cpu[0x09] = 0xFE;
	onebus.reg.cpu[0x0A] = 0x00;
	onebus.reg.cpu[0x0B] = 0x00;
	onebus.reg.cpu[0x0F] = 0xFF;
	onebus.reg.cpu[0x60] = 0x00;
	onebus.reg.cpu[0x61] = 0x00;

	if (info.mapper.ext_console_type == VT369) {
		int i = 0;

		extcl_cpu_wr_mem_OneBus(0x4162, 0x00);

		for (i = 0; i < (int)LENGTH(onebus.gpio); i++) {
			gpio_onebus_reset(onebus.gpio[i]);
		}

		// TODO : ho implementato la scrittura $3000-$3FFF, devo implementarne la lettura
		// (oltre che tutto il resto dell'emulazione VT).
	}

	info.mapper.extend_wr = TRUE;
	info.mapper.ram_plus_op_controlled_by_mapper = TRUE;
	mapper.write_vram = FALSE;

	irqA12.present = TRUE;
}
void extcl_after_mapper_init_OneBus(void) {
	prg_fix_8k_OneBus(0x0FFF, 0);
	chr_fix_OneBus(0x7FFF, 0);
	mirroring_fix_OneBus();
}
void extcl_mapper_quit_OneBus(void) {
	int i = 0;

	for (i = 0; i < (int)LENGTH(onebus.gpio); i++) {
		gpio_onebus_reset(onebus.gpio[i]);
		gpio_onebus_free(onebus.gpio[i]);
	}
	if (onebustmp.chr.low) {
		free(onebustmp.chr.low);
		onebustmp.chr.low = NULL;
	}
	if (onebustmp.chr.high) {
		free(onebustmp.chr.high);
		onebustmp.chr.high = NULL;
	}
	if (onebustmp.chr.low16) {
		free(onebustmp.chr.low16);
		onebustmp.chr.low16 = NULL;
	}
	if (onebustmp.chr.high16) {
		free(onebustmp.chr.high16);
		onebustmp.chr.high16 = NULL;
	}
}
void extcl_cpu_wr_mem_OneBus(WORD address, BYTE value) {
	if ((address >= 0x4100) && (address <= 0x41FF)) {
		onebus.reg.cpu[address & 0x00FF] = value;
		switch (address & 0x01FF) {
			case 0x101:
				irqA12.counter = onebus.reg.cpu[0x01];
				break;
			case 0x102:
				irqA12.counter = 0;
				break;
			case 0x103:
				irqA12.enable = FALSE;
				irq.high &= ~EXT_IRQ;
				break;
			case 0x104:
				irqA12.enable = TRUE;
				break;
			case 0x140: case 0x141: case 0x142: case 0x143: case 0x144: case 0x145: case 0x146: case 0x147:
			case 0x148: case 0x149: case 0x14A: case 0x14B: case 0x14C: case 0x14D: case 0x14E: case 0x14F:
			case 0x150: case 0x151: case 0x152: case 0x153: case 0x154: case 0x155: case 0x156: case 0x157:
			case 0x158: case 0x159: case 0x15A: case 0x15B: case 0x15C: case 0x15D: case 0x15E: case 0x15F:
				if (info.mapper.ext_console_type == VT369) {
					gpio_onebus_write(onebus.gpio[(address & 0x18) >> 3], address & 0x07, value);
				}
				break;
			case 0x160:
			case 0x161:
				if (info.mapper.ext_console_type == VT369) {
					onebus.relative_8k = ((onebus.reg.cpu[0x61] & 0x0F) << 8) | onebus.reg.cpu[0x60];
				}
				break;
		}
		extcl_after_mapper_init();
		return;
	} else if ((address >= 0x6000) && (address <= 0x7FFF)) {
		if (!onebustmp.prg_6000) {
			prg.ram_plus_8k[address & 0x1FFF] = value;
		}
	} else if (address >= 0x8000) {
		if (!(onebus.reg.cpu[0x0B] & 0x08)) {
			switch (address & 0xE001) {
				case 0x8000:
					extcl_cpu_wr_mem_OneBus(0x4105, value & ~0x20);
					break;
				case 0x8001: {
					BYTE reg = onebus.reg.cpu[0x05] & 0x07;

					switch (reg) {
						case 0:
						case 1:
							extcl_wr_ppu_reg_OneBus(0x2016 + reg, &value);
							break;
						case 2:
						case 3:
						case 4:
						case 5:
							extcl_wr_ppu_reg_OneBus(0x2010 + reg, &value);
							break;
						case 6:
						case 7:
							extcl_cpu_wr_mem_OneBus(0x4101 + reg, value);
							break;
						default:
							break;
					}
					break;
				}
				case 0xA000:
					extcl_cpu_wr_mem_OneBus(0x4106, value & 0x01);
					break;
				case 0xC000:
					extcl_cpu_wr_mem_OneBus(0x4101, value);
					break;
				case 0xC001:
					extcl_cpu_wr_mem_OneBus(0x4102, value);
					break;
				case 0xE000:
					extcl_cpu_wr_mem_OneBus(0x4103, value);
					break;
				case 0xE001:
					extcl_cpu_wr_mem_OneBus(0x4104, value);
					break;
			}
		}
	}
}
BYTE extcl_cpu_rd_mem_OneBus(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x4100) && (address <= 0x4FFF)) {
		switch(address & 0x0FFF) {
			case 0x140: case 0x141: case 0x142: case 0x143: case 0x144: case 0x145: case 0x146: case 0x147:
			case 0x148: case 0x149: case 0x14A: case 0x14B: case 0x14C: case 0x14D: case 0x14E: case 0x14F:
			case 0x150: case 0x151: case 0x152: case 0x153: case 0x154: case 0x155: case 0x156: case 0x157:
			case 0x158: case 0x159: case 0x15A: case 0x15B: case 0x15D: case 0x15E: case 0x15F:
				return (info.mapper.ext_console_type == VT369 ?
					gpio_onebus_read(onebus.gpio[(address & 0x18) >> 3], address & 0x07) : 0xFF);
			case 0x15C:
				return (0x10);
			case 0x18A:
				return (0x04);
			case 0x1B7:
				return (0x04);
			case 0x1B9:
				return (0x80);
			default:
				if ((address <= 0x410D) || ((address >= 0x4160) && (address < 0x4800))) {
					return (onebus.reg.cpu[address & 0xFF]);
				} else {
					return (openbus);
				}
				break;
		}
	} else if ((address >= 0x6000) && (address <= 0x7FFF)) {
		return (onebustmp.prg_6000 ? onebustmp.prg_6000[address & 0x1FFF] : prg.ram_plus_8k[address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_OneBus(BYTE mode, BYTE slot, FILE *fp) {
	int i = 0;

	save_slot_ele(mode, slot, onebus.relative_8k);
	save_slot_ele(mode, slot, onebus.reg.cpu);
	save_slot_ele(mode, slot, onebus.reg.ppu);
	save_slot_ele(mode, slot, onebus.reg.apu);
	save_slot_ele(mode, slot, onebus.pcm.address);
	save_slot_ele(mode, slot, onebus.pcm.size);
	save_slot_ele(mode, slot, onebus.pcm.latch);
	save_slot_ele(mode, slot, onebus.pcm.clock);
	save_slot_ele(mode, slot, onebus.pcm.enable);
	save_slot_ele(mode, slot, onebus.pcm.irq);

	for (i = 0; i < (int)LENGTH(onebus.gpio); i++) {
		gpio_onebus_save_mapper(onebus.gpio[i], mode, slot, fp);
	}

	if (mode == SAVE_SLOT_READ) {
		extcl_after_mapper_init();
	}

	return (EXIT_OK);
}
BYTE extcl_wr_ppu_reg_OneBus(WORD address, BYTE *value) {
	if ((address >= 0x3000) && (info.mapper.ext_console_type == VT369)) {
		address &= 0x0FFF;
		{
			const BYTE slot = address >> 10;

			if (ntbl.writable[slot]) {
				ntbl.bank_1k[slot][address & 0x3FF] = (*value);
			}
		}
		return (TRUE);
	}
	if (address >= 0x2008) {
		if (info.mapper.ext_console_type <= VT09) {
			address &= ~0x40;
		}
		onebus.reg.ppu[address & 0x00FF] = (*value);
		extcl_after_mapper_init();
		return (TRUE);
	}
	return (FALSE);
}
BYTE extcl_wr_apu_OneBus(WORD address, BYTE *value) {
	if ((address >= 0x4000) && (address <= 0x403F)) {
		onebus.reg.apu[address & 0x3F] = (*value);
		switch (address & 0x003F) {
			case 0x12:
				if (onebus.reg.apu[0x30] & 0x10) {
					onebus.pcm.address = (*value) << 6;
				}
				break;
			case 0x13:
				if (onebus.reg.apu[0x30] & 0x10) {
					onebus.pcm.size = ((*value) << 4) + 1;
				}
				break;
			case 0x15:
				if (onebus.reg.apu[0x30] & 0x10) {
					onebus.pcm.enable = (*value) & 0x10;
					if (onebus.pcm.irq) {
						irq.high &= ~EXT_IRQ;
						onebus.pcm.irq = 0;
					}
					if (onebus.pcm.enable) {
						onebus.pcm.latch = onebus.pcm.clock;
					}
					(*value) &= 0xEF;
				}
				break;
		}
	}
	return (FALSE);
}
BYTE extcl_rd_apu_OneBus(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0x003F) {
		case 0x15:
			if (onebus.reg.apu[0x30] & 0x10) {
				openbus = (openbus & 0x7F) | onebus.pcm.irq;
			}
			break;
	}
	return (openbus);
}
BYTE extcl_rd_chr_OneBus(WORD address) {
	return (onebustmp.chr.bank.low.r2007[address >> 10][address & 0x3FF]);
}
void extcl_cpu_every_cycle_OneBus(void) {
	if (onebus.pcm.enable) {
		onebus.pcm.latch--;
		if (onebus.pcm.latch <= 0) {
			onebus.pcm.latch += onebus.pcm.clock;
			onebus.pcm.size--;
			if (onebus.pcm.size < 0) {
				onebus.pcm.irq = 0x80;
				onebus.pcm.enable = 0;
				irq.high |= EXT_IRQ;
			} else {
				WORD address = onebus.pcm.address | ((onebus.reg.apu[0x30] ^ 3) << 14);

				apu_wr_mem_mapper(0x4011, (BYTE)(cpu_rd_mem_dbg(address) >> 1));
				onebus.pcm.address++;
				onebus.pcm.address &= 0x7FFF;
			}
		}
	}
	extcl_cpu_every_cycle_MMC3();
}
void extcl_ppu_000_to_34x_OneBus(void) {
	extcl_ppu_000_to_34x_MMC3();

	if (info.mapper.ext_console_type == VT369) {
		if ((onebus.reg.cpu[0x1C] & 0x80) && (ppu.frame_y <= ppu_sclines.vint)) {
			return;
		}
		if ((onebus.reg.cpu[0x1C] & 0x20) && (ppu.frame_y == ppu_sclines.vint)) {
			return;
		}
	}
	if (((onebus.reg.cpu[0x0B] & 0x80) | (onebus.reg.ppu[0x10] & 0x02))) {
		if (((ppu.frame_y >= ppu_sclines.vint) && (ppu.screen_y < SCR_ROWS)) &&
			(ppu.frame_x == (info.mapper.ext_console_type == VT369 ? 240 : 256))) {
			irq_tick_OneBus();
		}
	}
}
void extcl_irq_A12_clock_OneBus(void) {
	if (!((onebus.reg.cpu[0x0B] & 0x80) | (onebus.reg.ppu[0x10] & 0x02))) {
		irq_tick_OneBus();
	}
}

void prg_fix_8k_OneBus(WORD mmask, WORD mblock) {
	BYTE mode = onebus.reg.cpu[0x0B] & 0x07;
	BYTE mask = mode == 0x07 ? 0xFF : 0x3F >> mode;
	BYTE swap = (onebus.reg.cpu[0x05] & 0x40) >> 5;
	WORD bank = 0, block = ((onebus.reg.cpu[0] & 0xF0) << 4) | (onebus.reg.cpu[0x0A] & ~mask);

	if ((info.mapper.ext_console_type == VT369) && (onebus.reg.cpu[0x1C] & 0x40)) {
		bank = mblock | (((block | (onebus.reg.cpu[0x12] & mask)) + onebus.relative_8k) & mmask);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		onebustmp.prg_6000 = prg_pnt(bank << 13);
	} else {
		onebustmp.prg_6000 = NULL;
	}

	bank = mblock | (((block | (onebus.reg.cpu[0x07] & mask)) + onebus.relative_8k) & mmask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0 ^ swap, bank);

	bank = mblock | (((block | (onebus.reg.cpu[0x08] & mask)) + onebus.relative_8k) & mmask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, bank);

	bank = mblock | (((block | ((onebus.reg.cpu[0x0B] & 0x40 ? onebus.reg.cpu[0x09] : 0xFE) & mask)) + onebus.relative_8k) & mmask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2 ^ swap, bank);

	bank = mblock | (((block | (0xFF & mask)) + onebus.relative_8k) & mmask);
	_control_bank(bank, info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, bank);

	map_prg_rom_8k_update();
}
void prg_fix_16k_OneBus(WORD bank0, WORD bank1, WORD mmask, WORD mblock) {
	BYTE mode = onebus.reg.cpu[0x0B] & 0x07;
	BYTE mask = mode == 0x07 ? 0xFF : 0x3F >> mode;
	WORD bank = 0, block = ((onebus.reg.cpu[0] & 0xF0) << 4) | (onebus.reg.cpu[0x0A] & ~mask);

	if ((info.mapper.ext_console_type == VT369) && (onebus.reg.cpu[0x1C] & 0x40)) {
		bank = mblock | (((block | (onebus.reg.cpu[0x12] & mask)) + onebus.relative_8k) & mmask);
		_control_bank(bank, info.prg.rom.max.banks_8k)
		onebustmp.prg_6000 = prg_pnt(bank << 13);
	} else {
		onebustmp.prg_6000 = NULL;
	}

	bank = mblock | ((block | (bank0 & mask)) & mmask);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, bank);

	bank = mblock | ((block | (bank1 & mask)) & mmask);
	_control_bank(bank, info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, bank);

	map_prg_rom_8k_update();
}
void chr_fix_OneBus(WORD mmask, WORD mblock) {
	BYTE v16ben = (onebus.reg.ppu[0x10] & 0x40) || (onebus.reg.cpu[0x2B] == 0x61);
	BYTE bk16en = onebus.reg.ppu[0x10] & 0x02;
	BYTE sp16en = onebus.reg.ppu[0x10] & 0x04;
	BYTE bkexten = onebus.reg.ppu[0x10] & 0x10;
	BYTE spexten = onebus.reg.ppu[0x10] & 0x08;
	BYTE vrwb = onebus.reg.ppu[0x18] & 0x07;
	//BYTE bkpage = onebus.reg.ppu[0x18] &0x08;

	// 0000-1FFF: 2007 CHR Low
	chr_swap_OneBus(onebustmp.chr.bank.low.r2007, v16ben ? onebustmp.chr.low16 : onebustmp.chr.low,
		bk16en || sp16en, bkexten || spexten, vrwb, mmask, mblock);

	// 4000-5FFF: 2007 CHR High
	//chr_swap_OneBus(onebustmp.chr.bank.high.r2007, v16ben ? onebustmp.chr.high16 : onebustmp.chr.high,
	//	bk16en || sp16en, bkexten || spexten, vrwb, mmask, mblock);

	// 8000-9FFF: BG CHR Low
	//chr_swap_OneBus(onebustmp.chr.bank.low.bg, v16ben ? onebustmp.chr.low16 : onebustmp.chr.low,
	//	bk16en, bkexten, bkpage ? 4 : 0, mmask, mblock);

	// A000-BFFF: SPR CHR Low
	//chr_swap_OneBus(onebustmp.chr.bank.low.spr, v16ben ? onebustmp.chr.low16 : onebustmp.chr.low,
	//	sp16en, spexten, 0, mmask, mblock);

	// C000-BFFF: BG CHR High
	//chr_swap_OneBus(onebustmp.chr.bank.high.bg, v16ben ? onebustmp.chr.high16 : onebustmp.chr.high,
	//	bk16en, bkexten, bkpage ? 4 : 0, mmask, mblock);

	// E000-FFFF: SPR CHR High
	//chr_swap_OneBus(onebustmp.chr.bank.high.spr, v16ben ? onebustmp.chr.high16 : onebustmp.chr.high,
	//	sp16en, spexten, 0, mmask, mblock);
}
void mirroring_fix_OneBus(void) {
	if (onebus.reg.cpu[0x06] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}

INLINE static void chr_swap_OneBus(BYTE **banks, BYTE *base, BYTE bit4pp, BYTE extended, WORD EVA, WORD mmask, WORD mblock) {
	static const BYTE chr_mask[8] = { 0xFF, 0x7F, 0x3F, 0x00, 0x1F, 0x0F, 0x07, 0x00 };
	BYTE mask = chr_mask[onebus.reg.ppu[0x1A] & 0x07];
	BYTE swap = (onebus.reg.cpu[0x05] & 0x80) >> 5;
	WORD block = (onebus.reg.ppu[0x1A] & 0xF8) & ~mask;
	size_t chrMask = onebustmp.chr.size - 1;
	int relative = onebus.relative_8k << 3;

	if (bit4pp) {
		relative >>= 1;
		chrMask >>= 1;
		mmask >>= 1;
		mblock >>= 1;
	} else {
		base = onebustmp.chr.data;
	}

	if (extended) {
#define chrpnt(input)\
	(((mblock | ((block | ((input) & mask) | EVA | ((onebus.reg.cpu[0] & 0x0F) << 11)) & mmask)) + relative) << 10) & chrMask
		banks[0 ^ swap] = &base[chrpnt((onebus.reg.ppu[0x16] & 0xFE))];
		banks[1 ^ swap] = &base[chrpnt((onebus.reg.ppu[0x16] | 0x01))];
		banks[2 ^ swap] = &base[chrpnt((onebus.reg.ppu[0x17] & 0xFE))];
		banks[3 ^ swap] = &base[chrpnt((onebus.reg.ppu[0x17] | 0x01))];
		banks[4 ^ swap] = &base[chrpnt(onebus.reg.ppu[0x12])];
		banks[5 ^ swap] = &base[chrpnt(onebus.reg.ppu[0x13])];
		banks[6 ^ swap] = &base[chrpnt(onebus.reg.ppu[0x14])];
		banks[7 ^ swap] = &base[chrpnt(onebus.reg.ppu[0x15])];
#undef chrpnt
	} else {
		block |= ((onebus.reg.ppu[0x18] & 0x70) << 4);
#define chrpnt(input)\
	(((mblock | ((block | ((input) & mask) | ((onebus.reg.cpu[0] & 0x0F) << 11)) & mmask)) + relative) << 10) & chrMask
		banks[0 ^ swap] = &base[chrpnt((onebus.reg.ppu[0x16] & 0xFE))];
		banks[1 ^ swap] = &base[chrpnt((onebus.reg.ppu[0x16] | 0x01))];
		banks[2 ^ swap] = &base[chrpnt((onebus.reg.ppu[0x17] & 0xFE))];
		banks[3 ^ swap] = &base[chrpnt((onebus.reg.ppu[0x17] | 0x01))];
		banks[4 ^ swap] = &base[chrpnt(onebus.reg.ppu[0x12])];
		banks[5 ^ swap] = &base[chrpnt(onebus.reg.ppu[0x13])];
		banks[6 ^ swap] = &base[chrpnt(onebus.reg.ppu[0x14])];
		banks[7 ^ swap] = &base[chrpnt(onebus.reg.ppu[0x15])];
#undef chrpnt
	}
}
INLINE static void irq_tick_OneBus(void) {
	irqA12.counter = !irqA12.counter ? onebus.reg.cpu[0x01] : irqA12.counter - 1;
	if (!irqA12.counter &&irqA12.enable && !ppu.vblank && r2001.visible) {
		if (info.mapper.ext_console_type == VT369 && (onebus.reg.cpu[0x1C] & 0x20)) {
			irqA12.delay = 24;
		} else {
			irq.high |= EXT_IRQ;
		}
	}
}
