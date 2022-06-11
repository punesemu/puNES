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

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

INLINE static void ctrl_reg_MMC1(void);
INLINE static void swap_prg_rom_MMC1(void);
INLINE static void swap_chr0_MMC1(void);
INLINE static void swap_chr1_MMC1(void);

enum MMC1_regs { CTRL, CHR0, CHR1, PRG0 };

#define chr_reg(reg)\
	value = reg;\
	switch (info.mapper.submapper) {\
		case SNROM:\
			/*\
			 * 4bit0\
			 * -----\
			 * ExxxC\
			 * |   |\
			 * |   +- Select 4 KB CHR RAM bank at PPU $0000 (ignored in 8 KB mode)\
			 * +----- PRG RAM disable (0: enable, 1: open bus)\
			 */\
			cpu.prg_ram_rd_active = (reg & 0x10 ? FALSE : TRUE);\
			cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;\
			value &= 0x01;\
			break;\
		case SOROM: {\
			BYTE bank = (reg & 0x08) >> 3;\
			prg.ram_plus_8k = &prg.ram_plus[bank * 0x2000];\
			mmc1.prg_upper = (info.prg.rom.banks_16k > 0xF) ? reg & 0x10 : 0;\
			value &= 0x01;\
			break;\
		}\
		case SUROM:\
			mmc1.prg_upper = (info.prg.rom.banks_16k > 0xF) ? reg & 0x10 : 0;\
			value &= 0x01;\
			break;\
		case SXROM: {\
			BYTE bank = (reg & 0x0C) >> 2;\
			prg.ram_plus_8k = &prg.ram_plus[bank * 0x2000];\
			mmc1.prg_upper = (info.prg.rom.banks_16k > 0xF) ? reg & 0x10 : 0;\
			value &= 0x01;\
			break;\
		}\
		case FARIDSLROM:\
			value |= mmc1.chr_upper;\
			break;\
		case MAP111:\
			value &= 0x3F;\
			break;\
		case MAP297:\
		case MAP374:\
		case MAP404:\
		value = mmc1.chr_upper | (value & 0x1F);\
			break;\
		default:\
			value &= 0x1F;\
			break;\
	}

// NTF2 System Cart (U) [!].nes
static BYTE dipswitch_mmc1[] = { 0x07, 0xFD, 0x03, 0xFE };
_mmc1 mmc1;
struct _mmc1tmp {
	BYTE index;
	BYTE dipswitch;
} mmc1tmp;

void map_init_MMC1(void) {
	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_SAVE_MAPPER(MMC1);
	mapper.internal_struct[0] = (BYTE *)&mmc1;
	mapper.internal_struct_size[0] = sizeof(mmc1);

	// NTF2 System Cart (U) [!].nes
	if (info.crc32.prg == 0xAF8F7059) {
		EXTCL_CPU_RD_RAM(MMC1);

		if (info.reset == RESET) {
			mmc1tmp.index = (mmc1tmp.index + 1) & 0x03;
		} else if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
			mmc1tmp.index = 0;
		}
		mmc1tmp.dipswitch = dipswitch_mmc1[mmc1tmp.index];
	}

	if (info.reset >= HARD) {
		memset(&mmc1, 0x00, sizeof(mmc1));
		mmc1.ctrl = 0x0C;
		mmc1.prg_mode = 3;
	}
	mmc1.chr1 = 1;

	if (info.mapper.submapper == DEFAULT) {
		if (((info.prg.rom.banks_8k == 16) || (info.prg.rom.banks_8k == 32)
			|| (info.prg.rom.banks_8k == 64)) && (info.chr.rom.banks_8k <= 1)
			&& ((info.prg.ram.banks_8k_plus == 4) || (info.prg.ram.bat.banks == 4))) {
			info.mapper.submapper = SXROM;
		} else if (info.prg.rom.banks_8k <= 32) {
			if (info.chr.rom.banks_8k <= 1) {
				info.mapper.submapper = SNROM;
			}
		} else {
			info.mapper.submapper = SUROM;
		}
	}

	mmc1.prg_mask = 0x0F;

	switch (info.mapper.submapper) {
		case SNROM:
			/* SUROM usa 8k di PRG Ram */
			info.prg.ram.banks_8k_plus = 1;
			break;
		case SOROM:
			/* SOROM usa 16k di PRG Ram */
			info.prg.ram.banks_8k_plus = 2;
			break;
		case SXROM:
			/* SXROM usa 32k di PRG Ram */
			info.prg.ram.banks_8k_plus = 4;
			break;
		case SKROM:
			/* SKROM usa 8k di PRG Ram */
			info.prg.ram.banks_8k_plus = 1;
			cpu.prg_ram_wr_active = cpu.prg_ram_rd_active = TRUE;
			break;
		default:
			break;
	}
}
void extcl_cpu_wr_mem_MMC1(WORD address, BYTE value) {
	/*
	 * se nel tick precedente e' stato fatto un reset e
	 * sono in presenza di una doppia scrittura da parte
	 * di un'istruzione (tipo l'INC), allora l'MMC1 non
	 * la considera. Roms interessate:
	 * Advanced Dungeons & Dragons - Hillsfar
	 * Bill & Ted's Excellent Video Game Adventure
	 * Snow Brothers
	 */
	if (mmc1.reset) {
		/* azzero il flag */
		mmc1.reset = FALSE;
		/* esco se necessario */
		if (cpu.double_wr) {
			return;
		}
	}
	/*
	 * A program's reset code will reset the mapper
	 * first by writing a value of $80 through $FF
	 * to any address in $8000-$FFFF.
	 */
	if (value & 0x80) {
		/* indico che e' stato fatto un reset */
		mmc1.reset = TRUE;
		/* azzero posizione e registro temporaneo */
		mmc1.pos = mmc1.reg = 0;
		/*
		 * reset shift register and write
		 * Control with (Control OR $0C).
		 */
		mmc1.ctrl |= 0x0C;
		/* reinizializzo tutto */
		ctrl_reg_MMC1();
		/*
		 * locking PRG ROM at $C000-$FFFF
		 * to the last 16k bank.
		 */
		value = mmc1.prg_upper | mmc1.prg_mask;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
		map_prg_rom_8k_update();
		return;
	}

	if (info.mapper.submapper == MAP111) {
		mmc1.reg = value;
		mmc1.pos = 4;
	} else {
		mmc1.reg |= ((value & 0x01) << mmc1.pos);
	}

	if (mmc1.pos++ == 4) {
		BYTE reg = (address >> 13) & 0x03;

		switch (reg) {
			case CTRL:
				mmc1.ctrl = mmc1.reg;
				ctrl_reg_MMC1();
				break;
			case CHR0:
				mmc1.chr0 = mmc1.reg;
				swap_chr0_MMC1();
				break;
			case CHR1:
				mmc1.chr1 = mmc1.reg;
				swap_chr1_MMC1();
				break;
			case PRG0:
				mmc1.prg0 = mmc1.reg;

				if (info.mapper.submapper == SKROM) {
					break;
				}

				cpu.prg_ram_rd_active = (mmc1.prg0 & 0x10 ? FALSE : TRUE);
				cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
				break;
		}
		swap_prg_rom_MMC1();
		/* azzero posizione e registro temporaneo */
		mmc1.pos = mmc1.reg = 0;
	}
}
BYTE extcl_cpu_rd_ram_MMC1(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x1000) && (address <= 0x1FFF)) {
		return (mmc1tmp.dipswitch);
	}
	return (openbus);
}
BYTE extcl_save_mapper_MMC1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc1.reg);
	save_slot_ele(mode, slot, mmc1.pos);
	save_slot_ele(mode, slot, mmc1.prg_mode);
	save_slot_ele(mode, slot, mmc1.chr_mode);
	save_slot_ele(mode, slot, mmc1.ctrl);
	save_slot_ele(mode, slot, mmc1.chr0);
	save_slot_ele(mode, slot, mmc1.chr1);
	save_slot_ele(mode, slot, mmc1.prg0);
	save_slot_ele(mode, slot, mmc1.reset);
	save_slot_ele(mode, slot, mmc1.prg_upper);
	if (save_slot.version >= 27) {
		save_slot_ele(mode, slot, mmc1.chr_upper);
	}

	return (EXIT_OK);
}

INLINE static void ctrl_reg_MMC1(void) {
	mmc1.prg_mode = (mmc1.ctrl & 0x0C) >> 2;
	mmc1.chr_mode = (mmc1.ctrl & 0x10) >> 4;
	switch (mmc1.ctrl & 0x03) {
		case 0x00:
			mirroring_SCR0();
			break;
		case 0x01:
			mirroring_SCR1();
			break;
		case 0x02:
			mirroring_V();
			break;
		case 0x03:
			mirroring_H();
			break;
	}
	swap_chr0_MMC1();
	swap_chr1_MMC1();
}
INLINE static void swap_prg_rom_MMC1(void) {
	BYTE value = mmc1.prg0;

	/* SEROM, SHROM, SH1ROM use a fixed 32k PRG ROM with no banking support */
	if (info.mapper.submapper == SEROM) {
		return;
	}

	switch (mmc1.prg_mode) {
		case 0:
		case 1:
			/* switch 32k at $8000, ignoring low bit of bank number */
			value = (mmc1.prg_upper | (value & mmc1.prg_mask)) >> 1;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			break;
		case 2:
			/* fix first 16k bank at $8000 and switch 16 KB bank at $C000 */
			value = mmc1.prg_upper | (value & mmc1.prg_mask);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			value = mmc1.prg_upper;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			break;
		case 3:
			/* fix last 16k bank at $C000 and switch 16 KB bank at $8000 */
			value = mmc1.prg_upper | (value & mmc1.prg_mask);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value = mmc1.prg_upper | mmc1.prg_mask;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
	}
	map_prg_rom_8k_update();
}
INLINE static void swap_chr0_MMC1(void) {
	DBWORD value;

	chr_reg(mmc1.chr0)

	/* 4k mode */
	if (mmc1.chr_mode) {
		control_bank(info.chr.rom.max.banks_4k)
		value <<= 12;
		chr.bank_1k[0] = chr_pnt(value);
		chr.bank_1k[1] = chr_pnt(value | 0x0400);
		chr.bank_1k[2] = chr_pnt(value | 0x0800);
		chr.bank_1k[3] = chr_pnt(value | 0x0C00);
		return;
	}

	/* 8k mode */

	/*
	 * se ho solo della CHR ram allora
	 * non posso switchare niente.
	 */
	if (mapper.write_vram) {
		return;
	}

	//control_bank_with_AND(0x1E, info.chr.rom.max.banks_4k)
	value >>= 1;
	control_bank(info.chr.rom.max.banks_8k)
	value <<= 13;
	chr.bank_1k[0] = chr_pnt(value);
	chr.bank_1k[1] = chr_pnt(value | 0x0400);
	chr.bank_1k[2] = chr_pnt(value | 0x0800);
	chr.bank_1k[3] = chr_pnt(value | 0x0C00);
	chr.bank_1k[4] = chr_pnt(value | 0x1000);
	chr.bank_1k[5] = chr_pnt(value | 0x1400);
	chr.bank_1k[6] = chr_pnt(value | 0x1800);
	chr.bank_1k[7] = chr_pnt(value | 0x1C00);
}
INLINE static void swap_chr1_MMC1(void) {
	if (mmc1.chr_mode) {
		DBWORD value;

		chr_reg(mmc1.chr1)

		control_bank(info.chr.rom.max.banks_4k)
		value <<= 12;
		chr.bank_1k[4] = chr_pnt(value);
		chr.bank_1k[5] = chr_pnt(value | 0x0400);
		chr.bank_1k[6] = chr_pnt(value | 0x0800);
		chr.bank_1k[7] = chr_pnt(value | 0x0C00);
	}
}
