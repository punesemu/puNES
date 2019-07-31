/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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
#include "apu.h"
#include "ppu.h"
#include "cpu.h"
#include "irql2f.h"
#include "save_slot.h"

/* PRG */
#define prg_8k_update(slot)\
	value = mmc5.prg_bank[slot];\
	if (value & 0x80) {\
		/* modalita' rom */\
		mmc5.prg_ram_bank[slot][0] = FALSE;\
		control_bank_with_AND(0x7F, info.prg.rom[0].max.banks_8k)\
		map_prg_rom_8k(1, slot, value);\
	} else {\
		/* modalita' ram */\
		BYTE bank = prg_ram_access[prg_ram_mode][value & 0x07];\
		if (bank != INVALID) {\
			mmc5.prg_ram_bank[slot][0] = TRUE;\
			mmc5.prg_ram_bank[slot][1] = bank << 13;\
		}\
	}
#define prg_16k_update()\
	value = mmc5.prg_bank[1];\
	if (value & 0x80) {\
		/* modalita' rom */\
		mmc5.prg_ram_bank[0][0] = mmc5.prg_ram_bank[1][0] = FALSE;\
		value = (value & 0x7F) >> 1;\
		control_bank(info.prg.rom[0].max.banks_16k)\
		map_prg_rom_8k(2, 0, value);\
	} else {\
		/* modalita' ram */\
		BYTE bank = prg_ram_access[prg_ram_mode][value & 0x07];\
		if (bank != INVALID) {\
			mmc5.prg_ram_bank[0][0] = TRUE;\
			mmc5.prg_ram_bank[0][1] = (value & 0x06) << 13;\
		}\
		bank = prg_ram_access[prg_ram_mode][(value + 1) & 0x07];\
		if (bank != INVALID) {\
			mmc5.prg_ram_bank[1][0] = TRUE;\
			mmc5.prg_ram_bank[1][1] = (value & 0x07) << 13;\
		}\
	}
#define prg_rom_8k_update()\
	value = mmc5.prg_bank[3];\
	mmc5.prg_ram_bank[3][0] = FALSE;\
	control_bank_with_AND(0x7F, info.prg.rom[0].max.banks_8k)\
	map_prg_rom_8k(1, 3, value)
#define prg_rom_16k_update()\
	value = (mmc5.prg_bank[3] & 0x7F) >> 1;\
	mmc5.prg_ram_bank[2][0] = mmc5.prg_ram_bank[3][0] = FALSE;\
	control_bank(info.prg.rom[0].max.banks_16k)\
	map_prg_rom_8k(2, 2, value)
#define prg_rom_32k_update()\
	value = (mmc5.prg_bank[3] & 0x7F) >> 2;\
	mmc5.prg_ram_bank[0][0] = mmc5.prg_ram_bank[1][0] = FALSE;\
	mmc5.prg_ram_bank[2][0] = mmc5.prg_ram_bank[3][0] = FALSE;\
	control_bank(info.prg.rom[0].max.banks_32k)\
	map_prg_rom_8k(4, 0, value)
#define prg_ram_use_bank(slot)\
	prg.rom_8k[slot] = &prg.ram_plus[mmc5.prg_ram_bank[slot][1]]
/* CHR */
#define chr_8k_update(chr_type, slot)\
	value = mmc5.chr_type[slot];\
	control_bank_with_AND(0x03FF, info.chr.rom[0].max.banks_8k)\
	value <<= 13;\
	chr.bank_1k[0] = chr_chip_byte_pnt(0, value);\
	chr.bank_1k[1] = chr_chip_byte_pnt(0, value | 0x0400);\
	chr.bank_1k[2] = chr_chip_byte_pnt(0, value | 0x0800);\
	chr.bank_1k[3] = chr_chip_byte_pnt(0, value | 0x0C00);\
	chr.bank_1k[4] = chr_chip_byte_pnt(0, value | 0x1000);\
	chr.bank_1k[5] = chr_chip_byte_pnt(0, value | 0x1400);\
	chr.bank_1k[6] = chr_chip_byte_pnt(0, value | 0x1800);\
	chr.bank_1k[7] = chr_chip_byte_pnt(0, value | 0x1C00)
#define chr_4k_update(chr_type, slot, base)\
	value = mmc5.chr_type[slot];\
	control_bank_with_AND(0x03FF, info.chr.rom[0].max.banks_4k)\
	value <<= 12;\
	chr.bank_1k[base | 0] = chr_chip_byte_pnt(0, value);\
	chr.bank_1k[base | 1] = chr_chip_byte_pnt(0, value | 0x0400);\
	chr.bank_1k[base | 2] = chr_chip_byte_pnt(0, value | 0x0800);\
	chr.bank_1k[base | 3] = chr_chip_byte_pnt(0, value | 0x0C00)
#define chr_2k_update(chr_type, slot, base)\
	value = mmc5.chr_type[slot];\
	control_bank_with_AND(0x03FF, info.chr.rom[0].max.banks_2k)\
	value <<= 11;\
	chr.bank_1k[base | 0] = chr_chip_byte_pnt(0, value);\
	chr.bank_1k[base | 1] = chr_chip_byte_pnt(0, value | 0x0400)
#define chr_1k_update(chr_type, slot, base)\
	value = mmc5.chr_type[slot];\
	control_bank_with_AND(0x03FF, info.chr.rom[0].max.banks_1k)\
	value <<= 10;\
	chr.bank_1k[base] = chr_chip_byte_pnt(0, value)
/* nametables **/
#define nmt_update(bits, slot)\
{\
	BYTE mode = (value >> bits) & 0x03;\
	mmc5.nmt_mode[slot] = mode;\
	_nmt_update(slot)\
}
#define _nmt_update(slot)\
	switch (mode) {\
		case MODE0:\
			ntbl.bank_1k[slot] = &ntbl.data[0];\
			break;\
		case MODE1:\
			ntbl.bank_1k[slot] = &ntbl.data[0x400];\
			break;\
		case MODE2:\
			ntbl.bank_1k[slot] = &mmc5.ext_ram[0];\
			break;\
		case MODE3:\
			ntbl.bank_1k[slot] = &mmc5.fill_table[0];\
			break;\
	}

INLINE static void prg_swap(void);
INLINE static void use_chr_s(void);
INLINE static void use_chr_b(void);

enum {
	PRG_RAM_NONE,
	PRG_RAM_8K,
	PRG_RAM_16K,
	PRG_RAM_32K,
	PRG_RAM_40K,
	PRG_RAM_64K,
	INVALID,
	X = INVALID
};
enum { MODE0, MODE1, MODE2, MODE3 };
enum { CHR_S, CHR_B };
enum { SPLIT_LEFT, SPLIT_RIGHT = 0x40 };

const BYTE filler_attrib[4] = {0x00, 0x55, 0xAA, 0xFF};
static const BYTE prg_ram_access[6][8] = {
	{X,X,X,X,X,X,X,X},
	{0,0,0,0,X,X,X,X},
	{0,0,0,0,1,1,1,1},
	{0,1,2,3,X,X,X,X},
	{0,1,2,3,4,4,4,4},
	{0,1,2,3,4,5,6,7}
};
BYTE prg_ram_mode;

void map_init_MMC5(void) {
	EXTCL_CPU_WR_MEM(MMC5);
	EXTCL_CPU_RD_MEM(MMC5);
	EXTCL_SAVE_MAPPER(MMC5);
	EXTCL_PPU_256_TO_319(MMC5);
	EXTCL_PPU_320_TO_34X(MMC5);
	EXTCL_RD_R2007(MMC5);
	EXTCL_AFTER_RD_CHR(MMC5);
	EXTCL_RD_NMT(MMC5);
	EXTCL_RD_CHR(MMC5);
	EXTCL_LENGTH_CLOCK(MMC5);
	EXTCL_ENVELOPE_CLOCK(MMC5);
	EXTCL_APU_TICK(MMC5);
	mapper.internal_struct[0] = (BYTE *) &mmc5;
	mapper.internal_struct_size[0] = sizeof(mmc5);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&mmc5, 0x00, sizeof(mmc5));
		memset(&irql2f, 0x00, sizeof(irql2f));
		mmc5.prg_mode = MODE3;
		mmc5.chr_mode = MODE0;
		mmc5.ext_mode = MODE0;
		mmc5.chr_last = CHR_S;

		mmc5.S3.frequency = 1;
		mmc5.S4.frequency = 1;

		irql2f.scanline = 255;
		irql2f.frame_x = 339;

		for (i = 0; i < 4; ++i) {
			mmc5.prg_bank[i] = 0xFF;
		}

		for (i = 0; i < 8; ++i) {
			mmc5.chr_s[i] = i;
		}

		for (i = 0; i < 4; ++i) {
			mmc5.chr_b[i] = i;
		}

		use_chr_s();
	} else {
		mmc5.S3.length.enabled = 0;
		mmc5.S3.length.value = 0;
		mmc5.S4.length.enabled = 0;
		mmc5.S4.length.value = 0;
	}

	info.mapper.extend_wr = TRUE;
	irql2f.present = TRUE;

	switch (info.mapper.submapper) {
		case EKROM:
			info.prg.ram.banks_8k_plus = 1;
			info.prg.ram.bat.banks = 1;
			prg_ram_mode = PRG_RAM_8K;
			break;
		case ELROM:
			info.prg.ram.banks_8k_plus = FALSE;
			info.prg.ram.bat.banks = FALSE;
			prg_ram_mode = PRG_RAM_NONE;
			break;
		case ETROM:
			info.prg.ram.banks_8k_plus = 2;
			info.prg.ram.bat.banks = 1;
			info.prg.ram.bat.start = 0;
			prg_ram_mode = PRG_RAM_16K;
			break;
		case EWROM:
			info.prg.ram.banks_8k_plus = 4;
			info.prg.ram.bat.banks = 4;
			prg_ram_mode = PRG_RAM_32K;
			break;
		default:
			info.prg.ram.banks_8k_plus = 8;
			info.prg.ram.bat.banks = FALSE;
			prg_ram_mode = PRG_RAM_64K;
			break;
	}
}
void map_init_NSF_MMC5(void) {
	memset(&mmc5, 0x00, sizeof(mmc5));

	mmc5.S3.frequency = 1;
	mmc5.S4.frequency = 1;
	mmc5.S3.length.enabled = 0;
	mmc5.S3.length.value = 0;
	mmc5.S4.length.enabled = 0;
	mmc5.S4.length.value = 0;
}
void extcl_cpu_wr_mem_MMC5(WORD address, BYTE value) {
	if (address < 0x5000) {
		return;
	}

	switch (address) {
		case 0x5000:
			square_reg0(mmc5.S3);
			return;
		case 0x5001:
			/* lo sweep non e' utilizzato */
			return;
		case 0x5002:
			square_reg2(mmc5.S3);
			return;
		case 0x5003:
			square_reg3(mmc5.S3);
			return;
		case 0x5004:
			square_reg0(mmc5.S4);
			return;
		case 0x5005:
			/* lo sweep non e' utilizzato */
			return;
		case 0x5006:
			square_reg2(mmc5.S4);
			return;
		case 0x5007:
			square_reg3(mmc5.S4);
			return;
		case 0x5010:
			mmc5.pcm.enabled = ~value & 0x01;
			mmc5.pcm.output = 0;
			if (mmc5.pcm.enabled) {
				mmc5.pcm.output = mmc5.pcm.amp;
			}
			mmc5.clocked = TRUE;
			return;
		case 0x5011:
			mmc5.pcm.amp = value;
			mmc5.pcm.output = 0;
			if (mmc5.pcm.enabled) {
				mmc5.pcm.output = mmc5.pcm.amp;
			}
			mmc5.clocked = TRUE;
			return;
		case 0x5015:
			if (!(mmc5.S3.length.enabled = value & 0x01)) {
				mmc5.S3.length.value = 0;
			}
			if (!(mmc5.S4.length.enabled = value & 0x02)) {
				mmc5.S4.length.value = 0;
			}
			return;
		case 0x5100:
			value &= 0x03;
			if (value != mmc5.prg_mode) {
				mmc5.prg_mode = value;
				prg_swap();
			}
			return;
		case 0x5101:
			value &= 0x03;
			mmc5.chr_mode = value;
			return;
		case 0x5102:
		case 0x5103:
			mmc5.prg_ram_write[address & 0x0001] = value & 0x03;
			if ((mmc5.prg_ram_write[0] == 0x02) && (mmc5.prg_ram_write[1] == 0x01)) {
				cpu.prg_ram_wr_active = TRUE;
			} else {
				cpu.prg_ram_wr_active = FALSE;
			}
			return;
		case 0x5104:
			mmc5.ext_mode = value & 0x03;
			return;
		case 0x5105:
			nmt_update(0, 0)
			nmt_update(2, 1)
			nmt_update(4, 2)
			nmt_update(6, 3)
			return;
/* --------------------------------- PRG bankswitching ---------------------------------*/
		case 0x5106:
			mmc5.fill_tile = value;
			memset(&mmc5.fill_table[0], mmc5.fill_tile, 0x3C0);
			memset(&mmc5.fill_table[0x3C0], filler_attrib[mmc5.fill_attr], 0x40);
			return;
		case 0x5107:
			mmc5.fill_attr = value & 0x03;
			memset(&mmc5.fill_table[0x3C0], filler_attrib[mmc5.fill_attr], 0x40);
			return;
		case 0x5113: {
			BYTE bank = prg_ram_access[prg_ram_mode][value & 0x07];

			if (bank != INVALID) {
				prg.ram_plus_8k = &prg.ram_plus[bank * 0x2000];
			}
			return;
		}
		case 0x5114:
		case 0x5115:
		case 0x5116:
		case 0x5117:
			address &= 0x0003;
			if (mmc5.prg_bank[address] != value) {
				mmc5.prg_bank[address] = value;
				prg_swap();
			}
			return;
/* --------------------------------- CHR bankswitching ---------------------------------*/
		case 0x5120:
		case 0x5121:
		case 0x5122:
		case 0x5123:
		case 0x5124:
		case 0x5125:
		case 0x5126:
		case 0x5127:
			address &= 0x0007;
			mmc5.chr_s[address] = value | (mmc5.chr_high << 2);
			mmc5.chr_last = CHR_S;
			return;
		case 0x5128:
		case 0x5129:
		case 0x512A:
		case 0x512B:
			address &= 0x0003;
			mmc5.chr_b[address] = value | (mmc5.chr_high << 2);
			mmc5.chr_last = CHR_B;
			return;
		case 0x5130:
			mmc5.chr_high = (value & 0x03) << 6;
			return;
		case 0x5200:
			mmc5.split = value & 0x80;
			mmc5.split_side = value & 0x40;
			mmc5.split_st_tile = value & 0x1F;
			return;
		case 0x5201:
			if (value >= 240) {
				value -= 16;
			}
			mmc5.split_scrl = value;
			return;
		case 0x5202:
			control_bank(info.chr.rom[0].max.banks_4k)
			mmc5.split_bank = value << 12;
			return;
		case 0x5203:
			irql2f.scanline = value;
			return;
		case 0x5204:
			if (value & 0x80) {
				irql2f.enable = TRUE;
				return;
			}
			irql2f.enable = FALSE;
			/* disabilito l'IRQ dell'MMC5 */
			irq.high &= ~EXT_IRQ;
			return;
		case 0x5205:
			mmc5.factor[0] = value;
			mmc5.product = mmc5.factor[0] * mmc5.factor[1];
			return;
		case 0x5206:
			mmc5.factor[1] = value;
			mmc5.product = mmc5.factor[0] * mmc5.factor[1];
			return;
		default:
			if ((address >= 0x5C00) && (address < 0x6000)) {
				address &= 0x03FF;
				if (mmc5.ext_mode < MODE2) {
					if (!ppu.vblank && r2001.visible && (ppu.screen_y < SCR_LINES)) {
						mmc5.ext_ram[address] = value;
					} else {
						mmc5.ext_ram[address] = 0;
					}
					return;
				}
				if (mmc5.ext_mode == MODE2) {
					mmc5.ext_ram[address] = value;
					return;
				}
				return;
			}
			/*
			 * posso memorizzare nella PRG rom solo se il banco in cui
			 * la rom vuole scrivere punta ad un banco di PRG ram.
			 */
			if (address >= 0x8000) {
				BYTE index = ((address >> 13) & 0x03);
				if (mmc5.prg_ram_bank[index][0] && cpu.prg_ram_wr_active) {
					prg.rom_8k[index][address & 0x1FFF] = value;
				}
			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_MMC5(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	BYTE value;

	switch (address) {
		case 0x5015:
			/* azzero la varibile d'uscita */
			value = 0;
			/*
			 * per ogni canale controllo se il length counter
			 * non e' a 0 e se si setto a 1 il bit corrispondente
			 * nel byte di ritorno.
			 */
			if (mmc5.S3.length.value) {
				value |= 0x01;
			}
			if (mmc5.S4.length.value) {
				value |= 0x02;
			}
			return (value);
		case 0x5204:
			value = irql2f.pending | irql2f.in_frame;
			irql2f.pending = FALSE;
			/* disabilito l'IRQ dell'MMC5 */
			irq.high &= ~EXT_IRQ;
			return (value);
		case 0x5205:
			return (mmc5.product & 0x00FF);
		case 0x5206:
			return ((mmc5.product & 0xFF00) >> 8);
		default:
			if ((address < 0x5C00) || (address >= 0x6000)) {
				return (openbus);
			}
			if (mmc5.ext_mode < MODE2) {
				return (openbus);
			}
			if (mmc5.ext_mode == MODE2) {
				return (mmc5.ext_ram[address & 0x03FF]);
			}
			return (mmc5.fill_table[address & 0x03FF]);
	}
}
BYTE extcl_save_mapper_MMC5(BYTE mode, BYTE slot, FILE *fp) {
	BYTE i;

	save_slot_ele(mode, slot, mmc5.prg_mode);
	save_slot_ele(mode, slot, mmc5.chr_mode);
	save_slot_ele(mode, slot, mmc5.ext_mode);
	save_slot_ele(mode, slot, mmc5.nmt_mode);
	if (mode == SAVE_SLOT_READ) {
		for (i = 0; i < LENGTH(mmc5.nmt_mode); i++) {
			BYTE mode = mmc5.nmt_mode[i];

			_nmt_update(i)
		}
	}
	save_slot_ele(mode, slot, mmc5.prg_ram_write);
	save_slot_ele(mode, slot, mmc5.prg_bank);
	for (i = 0; i < LENGTH(mmc5.prg_ram_bank); i++) {
		save_slot_int(mode, slot, mmc5.prg_ram_bank[i][0]);
		save_slot_int(mode, slot, mmc5.prg_ram_bank[i][1]);
		if ((mode == SAVE_SLOT_READ) && (mmc5.prg_ram_bank[i][0])) {
			prg_ram_use_bank(i);
		}
	}
	save_slot_ele(mode, slot, mmc5.chr_last);
	save_slot_ele(mode, slot, mmc5.chr_high);
	save_slot_ele(mode, slot, mmc5.chr_s);
	save_slot_ele(mode, slot, mmc5.chr_b);
	save_slot_ele(mode, slot, mmc5.ext_ram);
	save_slot_ele(mode, slot, mmc5.fill_tile);
	save_slot_ele(mode, slot, mmc5.fill_attr);
	if (mode == SAVE_SLOT_READ) {
		memset(&mmc5.fill_table[0], mmc5.fill_tile, 0x3C0);
		memset(&mmc5.fill_table[0x3C0], filler_attrib[mmc5.fill_attr], 0x40);
	}
	save_slot_ele(mode, slot, mmc5.split);
	save_slot_ele(mode, slot, mmc5.split_st_tile);
	save_slot_ele(mode, slot, mmc5.split_side);
	save_slot_ele(mode, slot, mmc5.split_scrl);
	save_slot_ele(mode, slot, mmc5.split_in_reg);
	save_slot_ele(mode, slot, mmc5.split_x);
	save_slot_ele(mode, slot, mmc5.split_y);
	save_slot_ele(mode, slot, mmc5.split_tile);
	save_slot_ele(mode, slot, mmc5.split_bank);
	save_slot_ele(mode, slot, mmc5.factor);
	save_slot_ele(mode, slot, mmc5.product);

	save_slot_square(mmc5.S3, slot);
	save_slot_square(mmc5.S4, slot);

	save_slot_ele(mode, slot, mmc5.pcm.enabled);
	save_slot_ele(mode, slot, mmc5.pcm.output);
	save_slot_ele(mode, slot, mmc5.pcm.amp);

	save_slot_ele(mode, slot, mmc5.filler);

	return (EXIT_OK);
}
void extcl_ppu_256_to_319_MMC5(void) {
	if (ppu.frame_x != 256) {
		return;
	};

	if ((r2000.size_spr == 8) || r2001.visible) {
		use_chr_s();
	} else {
		if (mmc5.chr_last == CHR_S) {
			use_chr_s();
		} else {
			use_chr_b();
		}
	}
}
void extcl_ppu_320_to_34x_MMC5(void) {
	irql2f_tick();

	if (ppu.frame_x != 320) {
		return;
	};

	if (mmc5.split) {
		mmc5.split_x = 0x1F;
		if (ppu.screen_y == SCR_LINES - 1) {
			mmc5.split_y = mmc5.split_scrl - 1;
		} else {
			if (mmc5.split_y < 239) {
				mmc5.split_y++;
			} else {
				mmc5.split_y = 0;
			}
		}
	}

	if (r2000.size_spr == 8) {
		use_chr_s();
	} else if (r2001.visible) {
		use_chr_b();
	} else  {
		if (mmc5.chr_last == CHR_S) {
			use_chr_s();
		} else {
			use_chr_b();
		}
	}
}
void extcl_rd_r2007_MMC5(void) {

	// When 8x8 sprites are used, the registers from $5120-5127 ("set A") are used for everything
	// (sprites, BG tiles, and reads from $2007 during vblank or forced blank).
	// Registers $5128-512B ("set B") are unused.

	// When 8x16 sprites are used, sprites use the "set A" banks; BG tiles use the "set B" banks;
	// reads from $2007 during vblank or forced blank are treated as sprite accesses
	// (using bank set A) if the most recently written register was $5120-5127; and treated as
	// BG accesses (using bank set B) if the most recently written register was $5128-512B


	if (r2000.size_spr == 8) {
		use_chr_s();
	} else {
		if (mmc5.chr_last == CHR_S) {
			use_chr_s();
		} else {
			use_chr_b();
		}
	}
}
void extcl_after_rd_chr_MMC5(UNUSED(WORD address)) {
	/*
	 * dopo ogni fetch del high byte del background
	 * azzero il flag con cui indico se il tile era
	 * nella regione dello split (questo indipendentemente
	 * che lo fosse realmente o meno). In questo modo sono
	 * che non rimanga settato quando in realta' non serve.
	 */
	mmc5.split_in_reg = FALSE;
}
BYTE extcl_rd_chr_MMC5(WORD address) {
	BYTE value;
	uint32_t index;

	/* se non sto trattando il background esco */
	if ((address & 0xFFF7) != ppu.bck_adr) {
		return (chr.bank_1k[address >> 10][address & 0x3FF]);
	}

	/* sono nella regione di split? */
	if (mmc5.split && mmc5.split_in_reg) {
		return (chr_chip_byte(0, mmc5.split_bank + (address & 0x0FFF)));
	}

	/* se non sono nella modalita' 1 esco normalmente */
	if (mmc5.ext_mode != MODE1) {
		return (chr.bank_1k[address >> 10][address & 0x3FF]);
	}

	value = (mmc5.ext_ram[r2006.value & 0x03FF] & 0x3F);
	control_bank(info.chr.rom[0].max.banks_4k)
	index = ((value + mmc5.chr_high) << 12) + (address & 0x0FFF);

	return (chr_chip_byte(0, index));
}
BYTE extcl_rd_nmt_MMC5(WORD address) {
	BYTE nmt = address >> 10;

	if ((ntbl.bank_1k[nmt] == mmc5.ext_ram) && (mmc5.ext_mode > MODE1)) {
		return (0);
	}

	if (mmc5.split) {
		WORD adr = (address & 0x03FF);

		/* attributi */
		if (adr >= 0x03C0) {
			mmc5.split_x = (mmc5.split_x + 1) & 0x1F;

			mmc5.split_in_reg = FALSE;

			if (((mmc5.split_side == SPLIT_LEFT) && (mmc5.split_x <= mmc5.split_st_tile))
				|| ((mmc5.split_side == SPLIT_RIGHT) && (mmc5.split_x >= mmc5.split_st_tile))) {

				mmc5.split_tile = ((mmc5.split_y & 0xF8) << 2) | mmc5.split_x;

				mmc5.split_in_reg = TRUE;

				return (filler_attrib[(mmc5.ext_ram[0x3C0 | (mmc5.split_tile >> 4 & 0x38)
					| (mmc5.split_tile >> 2 & 0x7)]
					>> ((mmc5.split_tile >> 4 & 0x4) | (mmc5.split_tile & 0x2))) & 0x3]);
			}
		}
		/* tile */
		if (mmc5.split_in_reg) {
			return (mmc5.ext_ram[mmc5.split_tile]);
		}
	}

	if (mmc5.ext_mode != MODE1) {
		return (ntbl.bank_1k[nmt][address & 0x3FF]);
	}

	if ((address & 0x03FF) >= 0x03C0) {
		BYTE shift_at = (((r2006.value & 0x40) >> 4) | (r2006.value & 0x02));

		return (((mmc5.ext_ram[r2006.value & 0x03FF] & 0xC0) >> 6) << shift_at);
	}

	return (ntbl.bank_1k[nmt][address & 0x3FF]);
}
void extcl_length_clock_MMC5(void) {
	length_run(mmc5.S3)
	length_run(mmc5.S4)
}
void extcl_envelope_clock_MMC5(void) {
	envelope_run(mmc5.S3)
	envelope_run(mmc5.S4)
}
void extcl_apu_tick_MMC5(void) {
	square_tick(mmc5.S3, 0, mmc5)
	square_tick(mmc5.S4, 0, mmc5)
}

INLINE static void prg_swap(void) {
	BYTE value, i;

	switch (mmc5.prg_mode) {
		case MODE0:
			if (info.prg.rom[0].max.banks_32k == 0xFFFF) {
				break;
			}
			prg_rom_32k_update();
			break;
		case MODE1:
			if (info.prg.rom[0].max.banks_16k == 0xFFFF) {
				break;
			}
			prg_16k_update()
			prg_rom_16k_update();
			break;
		case MODE2:
			if (info.prg.rom[0].max.banks_16k != 0xFFFF) {
				prg_16k_update()
			}
			prg_8k_update(2)
			prg_rom_8k_update();
			break;
		case MODE3:
			prg_8k_update(0)
			prg_8k_update(1)
			prg_8k_update(2)
			prg_rom_8k_update();
			break;
	}

	map_prg_rom_8k_update();

	for (i = 0; i < 4; i++) {
		if (mmc5.prg_ram_bank[i][0]) {
			prg_ram_use_bank(i);
		} else {
			mmc5.prg_ram_bank[i][1] = 0;
		}
	}
}
INLINE static void use_chr_s(void) {
	DBWORD value;

	switch (mmc5.chr_mode) {
		case MODE0:
			chr_8k_update(chr_s, 7);
			break;
		case MODE1:
			chr_4k_update(chr_s, 3, 0);
			chr_4k_update(chr_s, 7, 4);
			break;
		case MODE2:
			chr_2k_update(chr_s, 1, 0);
			chr_2k_update(chr_s, 3, 2);
			chr_2k_update(chr_s, 5, 4);
			chr_2k_update(chr_s, 7, 6);
			break;
		case MODE3:
			chr_1k_update(chr_s, 0, 0);
			chr_1k_update(chr_s, 1, 1);
			chr_1k_update(chr_s, 2, 2);
			chr_1k_update(chr_s, 3, 3);
			chr_1k_update(chr_s, 4, 4);
			chr_1k_update(chr_s, 5, 5);
			chr_1k_update(chr_s, 6, 6);
			chr_1k_update(chr_s, 7, 7);
			break;
	}
}
INLINE static void use_chr_b(void) {
	DBWORD value;

	switch (mmc5.chr_mode) {
		case MODE0:
			chr_8k_update(chr_b, 3);
			break;
		case MODE1:
			chr_4k_update(chr_b, 3, 0);
			chr_4k_update(chr_b, 3, 4);
			break;
		case MODE2:
			chr_2k_update(chr_b, 1, 0);
			chr_2k_update(chr_b, 3, 2);
			chr_2k_update(chr_b, 1, 4);
			chr_2k_update(chr_b, 3, 6);
			break;
		case MODE3:
			chr_1k_update(chr_b, 0, 0);
			chr_1k_update(chr_b, 1, 1);
			chr_1k_update(chr_b, 2, 2);
			chr_1k_update(chr_b, 3, 3);
			chr_1k_update(chr_b, 0, 4);
			chr_1k_update(chr_b, 1, 5);
			chr_1k_update(chr_b, 2, 6);
			chr_1k_update(chr_b, 3, 7);
			break;
	}
}
