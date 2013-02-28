/*
 * mapper_MMC1.c
 *
 *  Created on: 4/lug/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

static void INLINE ctrl_reg_MMC1(void);
static void INLINE swap_prg_rom_MMC1(void);
static void INLINE swap_chr0_MMC1(void);
static void INLINE swap_chr1_MMC1(void);

enum { CTRL, CHR0, CHR1, PRG0 };

#define chr_reg(reg)\
	value = reg;\
	switch (info.mapper_type) {\
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
			mmc1.prg_upper = reg & 0x10;\
			value &= 0x01;\
			break;\
		}\
		case SUROM:\
			mmc1.prg_upper = reg & 0x10;\
			value &= 0x01;\
			break;\
		case SXROM: {\
			BYTE bank = (reg & 0x0C) >> 2;\
			prg.ram_plus_8k = &prg.ram_plus[bank * 0x2000];\
			mmc1.prg_upper = reg & 0x10;\
			value &= 0x01;\
			break;\
		}\
		default:\
			value &= 0x1F;\
			break;\
	}

WORD prg_rom_16k_max, chr_rom_8k_max, chr_rom_4k_max;

void map_init_MMC1(void) {
	prg_rom_16k_max = info.prg_rom_16k_count - 1;
	chr_rom_8k_max = info.chr_rom_8k_count - 1;
	chr_rom_4k_max = info.chr_rom_4k_count - 1;

	EXTCL_CPU_WR_MEM(MMC1);
	EXTCL_SAVE_MAPPER(MMC1);
	mapper.internal_struct[0] = (BYTE *) &mmc1;
	mapper.internal_struct_size[0] = sizeof(mmc1);

	if (info.reset >= HARD) {
		memset(&mmc1, 0x00, sizeof(mmc1));
		mmc1.ctrl = 0x0C;
		mmc1.prg_mode = 3;
		mmc1.chr1 = 1;
	}

	switch (info.mapper_type) {
		case SNROM:
			/* SUROM usa 8k di PRG Ram */
			info.prg_ram_plus_8k_count = 1;
			break;
		case SOROM:
			/* SOROM usa 16k di PRG Ram */
			info.prg_ram_plus_8k_count = 2;
			break;
		case SXROM:
			/* SXROM usa 32k di PRG Ram */
			info.prg_ram_plus_8k_count = 4;
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
		map_prg_rom_8k(2, 2, mmc1.prg_upper | (prg_rom_16k_max & 0x0F));
		map_prg_rom_8k_update();
		return;
	}

	mmc1.reg |= ((value & 0x01) << mmc1.pos);

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
				cpu.prg_ram_rd_active = (reg & 0x10 ? FALSE : TRUE);
				cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
				break;
		}
		swap_prg_rom_MMC1();
		/* azzero posizione e registro temporaneo */
		mmc1.pos = mmc1.reg = 0;
	}
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

	return (EXIT_OK);
}

static void INLINE ctrl_reg_MMC1(void) {
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
static void INLINE swap_prg_rom_MMC1(void) {
	BYTE value = mmc1.prg0;

	switch (mmc1.prg_mode) {
		case 0:
		case 1: {
			BYTE bank;

			control_bank_with_AND(0x0E, prg_rom_16k_max)
			bank = mmc1.prg_upper | value;
			/* switch 32k at $8000, ignoring low bit of bank number */
			map_prg_rom_8k(2, 0, bank);
			map_prg_rom_8k(2, 2, bank + 1);
			break;
		}
		case 2:
			control_bank_with_AND(0x0F, prg_rom_16k_max)
			/* fix first 16k bank at $8000 and switch 16 KB bank at $C000 */
			map_prg_rom_8k(2, 0, mmc1.prg_upper);
			map_prg_rom_8k(2, 2, mmc1.prg_upper | value);
			break;
		case 3:
			control_bank_with_AND(0x0F, prg_rom_16k_max)
			/* fix last 16k bank at $C000 and switch 16 KB bank at $8000 */
			map_prg_rom_8k(2, 0, mmc1.prg_upper | value);
			map_prg_rom_8k(2, 2, mmc1.prg_upper | (prg_rom_16k_max & 0x0F));
			break;
	}
	map_prg_rom_8k_update();
}
static void INLINE swap_chr0_MMC1(void) {
	DBWORD value;

	chr_reg(mmc1.chr0)

	/* 4k mode */
	if (mmc1.chr_mode) {
		control_bank(chr_rom_4k_max)
		value <<= 12;
		chr.bank_1k[0] = &chr.data[value];
		chr.bank_1k[1] = &chr.data[value | 0x0400];
		chr.bank_1k[2] = &chr.data[value | 0x0800];
		chr.bank_1k[3] = &chr.data[value | 0x0C00];
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

	control_bank_with_AND(0x1E, chr_rom_4k_max)
	value <<= 12;
	chr.bank_1k[0] = &chr.data[value];
	chr.bank_1k[1] = &chr.data[value | 0x0400];
	chr.bank_1k[2] = &chr.data[value | 0x0800];
	chr.bank_1k[3] = &chr.data[value | 0x0C00];
	chr.bank_1k[4] = &chr.data[value | 0x1000];
	chr.bank_1k[5] = &chr.data[value | 0x1400];
	chr.bank_1k[6] = &chr.data[value | 0x1800];
	chr.bank_1k[7] = &chr.data[value | 0x1C00];
}
static void INLINE swap_chr1_MMC1(void) {
	if (mmc1.chr_mode) {
		DBWORD value;

		chr_reg(mmc1.chr1)

		control_bank(chr_rom_4k_max)
		value <<= 12;
		chr.bank_1k[4] = &chr.data[value];
		chr.bank_1k[5] = &chr.data[value | 0x0400];
		chr.bank_1k[6] = &chr.data[value | 0x0800];
		chr.bank_1k[7] = &chr.data[value | 0x0C00];
	}
}
