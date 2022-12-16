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
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

INLINE static BYTE output_Txc_t22211ab(void);
INLINE static void chr_fix_Txc_t22211x(void);
INLINE static BYTE reverse_D0D5_Txc_t22211c(BYTE value);

struct _t22211x {
	BYTE reg[4];
	BYTE RRR;
} t22211x;
struct _txctmp {
	BYTE type;
	BYTE Jin_Gwok_132;
} txctmp;

void map_init_Txc(BYTE model) {
	switch (model) {
		case TXCTW:
			EXTCL_CPU_WR_MEM(Txc_tw);
			EXTCL_SAVE_MAPPER(MMC3);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *)&mmc3;
			mapper.internal_struct_size[0] = sizeof(mmc3);

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				/* sembra proprio che parta in questa modalita' */
				mmc3.prg_rom_cfg = 0x02;
				map_prg_rom_8k(4, 0, 0);
			}

			memset(&irqA12, 0x00, sizeof(irqA12));

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case T22211A:
		case T22211B:
			EXTCL_CPU_WR_MEM(Txc_t22211ab);
			EXTCL_CPU_RD_MEM(Txc_t22211ab);
			EXTCL_SAVE_MAPPER(Txc_t22211x);
			mapper.internal_struct[0] = (BYTE *)&t22211x;
			mapper.internal_struct_size[0] = sizeof(t22211x);

			memset(&t22211x, 0x00, sizeof(t22211x));

			info.mapper.extend_wr = TRUE;

			extcl_cpu_wr_mem_Txc_t22211ab(0x8000, 0x00);

			// Jin Gwok Sei Chuen Saang (Ch) [U][!].unf
			// 戰國四川省 (Zhànguó Sìchuān Shěng, original version of AVE's Tiles of Fate) is set to Mapper 132 in GoodNES 3.23b.
			// That ROM image is actually a mapper hack with the PRG-ROM code unmodified but the CHR-ROM banks rearranged to work
			// as Mapper 132; the correct mapper is INES Mapper 173. That mapper hack only works on certain
			// emulators' implementation of Mapper 132, not on the above implementation based on studying the circuit board.
			txctmp.Jin_Gwok_132 = info.crc32.total == 0x2A5F4C5A;
			break;
		case T22211C:
			EXTCL_CPU_WR_MEM(Txc_t22211c);
			EXTCL_CPU_RD_MEM(Txc_t22211c);
			EXTCL_SAVE_MAPPER(Txc_t22211x);
			mapper.internal_struct[0] = (BYTE *)&t22211x;
			mapper.internal_struct_size[0] = sizeof(t22211x);

			memset(&t22211x, 0x00, sizeof(t22211x));
			t22211x.reg[1] = 0x20;

			info.mapper.extend_wr = TRUE;

			extcl_cpu_wr_mem_Txc_t22211c(0x8000, 0x00);
			break;
	}

	txctmp.type = model;
}

void extcl_cpu_wr_mem_Txc_tw(WORD address, BYTE value) {
	if (address < 0x4120) {
		return;
	}

	if (address >= 0x8000) {
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	}

	value = (value >> 4) | value;
	control_bank(info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_Txc_t22211ab(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		switch (address & 0x0103) {
			case 0x0100:
				t22211x.reg[0] = value;
				if (t22211x.reg[3] & 0x01) {
					t22211x.RRR = (t22211x.RRR + 1) & 0x07;
				} else {
					BYTE PPP = t22211x.reg[2] & 0x07;

					t22211x.RRR = (t22211x.reg[1] & 0x01) ? ~PPP : PPP;
				}
				break;
			case 0x0101:
				t22211x.reg[1] = value;
				if (txctmp.type == T22211B) {
					chr_fix_Txc_t22211x();
				}
				break;
			case 0x0102:
				t22211x.reg[2] = value;
				break;
			case 0x0103:
				t22211x.reg[3] = value;
				break;
		}
	}

	if (address < 0x8000) {
		return;
	}

	value = (t22211x.RRR >> 2) & 0x01;
	control_bank(info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	chr_fix_Txc_t22211x();
}
BYTE extcl_cpu_rd_mem_Txc_t22211ab(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (address == 0x4100) {
		return ((openbus & 0xF0) | output_Txc_t22211ab());
	}
	return (openbus);
}

void extcl_cpu_wr_mem_Txc_t22211c(WORD address, BYTE value) {
	if ((address >= 0x4000) && (address <= 0x4FFF)) {
		switch (address & 0x0103) {
			case 0x0100:
				//Write $4100:
				// When Mode==1: Bits 0-3 of Register incremented by one, bits 4-5 unaffected.
				// When Mode==0: Bits 0-5 of Register := Input, bits 0-3 being inverted if Invert==1.
				t22211x.reg[0] = value;

				if (t22211x.reg[3] & 0x20) {
					t22211x.RRR = (t22211x.RRR & ~0x0F) | ((t22211x.RRR + 1) & 0x0F);
				} else {
					BYTE tmp = t22211x.reg[2] & 0x0F;

					t22211x.RRR = (t22211x.reg[2] & 0x30) | ((t22211x.reg[1] & 0x20) ? ~tmp : tmp);
				}
				break;
			case 0x0101:
				// Write $4101: Invert := Written value bit 5.
				t22211x.reg[1] = value;
				break;
			case 0x0102:
				// Write $4102: Input := Written value bits 0-5. Note that the bit order D0-D5 is reversed.
				t22211x.reg[2] = reverse_D0D5_Txc_t22211c(value);
				break;
			case 0x0103:
				// Write $4103: Mode := Written value bit 5.
				t22211x.reg[3] = value;
				break;
		}
	}

	if (address < 0x8000) {
		return;
	}

	chr_fix_Txc_t22211x();

	// When writing to $8000-$FFFF, nametable mirroring is changed to
	// Horizontal if Invert was clear, and to Vertical if Invert was set.
	if (t22211x.reg[1] & 0x20) {
		mirroring_V();
	} else {
		mirroring_H();
	}
}
BYTE extcl_cpu_rd_mem_Txc_t22211c(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x4100) && (address <= 0x4103)) {
		// Read $4100-$4103: [..RR RRRR]: Read Register. Bits 4-5 are inverted if Invert==1.
		// Bits 6-7 are open bus. Note that the bit order D0-D5 is reversed.
		return ((openbus & 0xC0) | reverse_D0D5_Txc_t22211c(t22211x.RRR));
	}
	return (openbus);
}

BYTE extcl_save_mapper_Txc_t22211x(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, t22211x.reg);
	save_slot_ele(mode, slot, t22211x.RRR);

	return (EXIT_OK);
}

INLINE static BYTE output_Txc_t22211ab(void) {
	return (((t22211x.reg[2] & 0x08) ^ ((t22211x.reg[1] & 0x01) << 3)) | t22211x.RRR);
}
INLINE static BYTE reverse_D0D5_Txc_t22211c(BYTE value) {
	BYTE count = sizeof(value) * 8 - 1;
	BYTE reversed = value, save = value;

	value <<= 2;
	value >>= 1;
	while (value) {
		reversed <<= 1;
		reversed |= value & 0x01;
		value >>= 1;
		count--;
	}
	reversed <<= count;
	return (reversed |= save &  0xC0);
}
INLINE static void chr_fix_Txc_t22211x(void) {
	DBWORD bank;

	if (txctmp.type == T22211B) {
		bank = (output_Txc_t22211ab() & 0x01) | (~(t22211x.reg[1] & 0x01) << 1);
	} else {
		bank = (txctmp.Jin_Gwok_132 ? t22211x.reg[2] : t22211x.RRR) & 0x03;
	}

	_control_bank(bank, info.chr.rom.max.banks_8k)
	bank = bank << 13;

	chr.bank_1k[0] = chr_pnt(bank);
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);
	chr.bank_1k[4] = chr_pnt(bank | 0x1000);
	chr.bank_1k[5] = chr_pnt(bank | 0x1400);
	chr.bank_1k[6] = chr_pnt(bank | 0x1800);
	chr.bank_1k[7] = chr_pnt(bank | 0x1C00);
}
