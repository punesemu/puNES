/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
#include "save_slot.h"

void (*MMC1_prg_fix)(void);
void (*MMC1_prg_swap)(WORD address, WORD value);
void (*MMC1_chr_fix)(void);
void (*MMC1_chr_swap)(WORD address, WORD value);
void (*MMC1_wram_fix)(void);
void (*MMC1_wram_swap)(WORD address, WORD value);
void (*MMC1_mirroring_fix)(void);

_mmc1 mmc1;
_mmc1tmp mmc1tmp;

void extcl_after_mapper_init_MMC1(void) {
	MMC1_prg_fix();
	MMC1_chr_fix();
	MMC1_wram_fix();
	MMC1_mirroring_fix();
}
void extcl_cpu_wr_mem_MMC1(BYTE nidx, WORD address, BYTE value) {
	if (address >= 0x8000) {
		// se nel tick precedente e' stato fatto un reset e
		// sono in presenza di una doppia scrittura da parte
		// di un'istruzione (tipo l'INC), allora l'MMC1 non
		// la considera. Roms interessate:
		// Advanced Dungeons & Dragons - Hillsfar
		// Bill & Ted's Excellent Video Game Adventure
		// Snow Brothers
		if (mmc1.reset) {
			// azzero il flag
			mmc1.reset = FALSE;
			// esco se necessario
			if (nes[nidx].c.cpu.double_wr) {
				return;
			}
		}
		// A program's reset code will reset the mapper
		// first by writing a value of $80 through $FF
		// to any address in $8000-$FFFF.
		if (value & 0x80) {
			// indico che e' stato fatto un reset
			mmc1.reset = TRUE;
			// azzero posizione e registro temporaneo
			mmc1.accumulator = mmc1.shift = 0;
			// reset shift register and write
			// Control with (Control OR $0C).
			mmc1.reg[0] |= 0x0C;
			return;
		}

		mmc1.accumulator |= ((value & 0x01) << mmc1.shift);

		if (mmc1.shift++ == 4) {
			mmc1.reg[(address >> 13) & 0x03] = mmc1.accumulator;
			mmc1.accumulator = mmc1.shift = 0;
			MMC1_prg_fix();
			MMC1_chr_fix();
			MMC1_wram_fix();
			MMC1_mirroring_fix();
		}
	}
}
BYTE extcl_save_mapper_MMC1(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc1.reg);
	save_slot_ele(mode, slot, mmc1.accumulator);
	save_slot_ele(mode, slot, mmc1.shift);
	save_slot_ele(mode, slot, mmc1.reset);
	return (EXIT_OK);
}

void init_MMC1(BYTE type, BYTE reset) {
	if (reset >= HARD) {
		memset(&mmc1, 0x00, sizeof(mmc1));

		mmc1.reg[0] = 0x0C;
		mmc1tmp.type = type;
	}

	MMC1_prg_fix = prg_fix_MMC1_base;
	MMC1_prg_swap = prg_swap_MMC1_base;
	MMC1_chr_fix = chr_fix_MMC1_base;
	MMC1_chr_swap = chr_swap_MMC1_base;
	MMC1_wram_fix = wram_fix_MMC1_base;
	MMC1_wram_swap = wram_swap_MMC1_base;
	MMC1_mirroring_fix = mirroring_fix_MMC1_base;
}
void prg_fix_MMC1_base(void) {
	MMC1_prg_swap(0x8000, prg_bank_MMC1(0));
	MMC1_prg_swap(0xC000, prg_bank_MMC1(1));
}
void prg_swap_MMC1_base(WORD address, WORD value) {
	memmap_auto_16k(0, MMCPU(address), value);
}
void chr_fix_MMC1_base(void) {
	MMC1_chr_swap(0x0000, chr_bank_MMC1(0));
	MMC1_chr_swap(0x1000, chr_bank_MMC1(1));
}
void chr_swap_MMC1_base(WORD address, WORD value) {
	memmap_auto_4k(0, MMPPU(address), value);
}
void wram_fix_MMC1_base(void) {
	MMC1_wram_swap(0x6000, 0);
}
void wram_swap_MMC1_base(WORD address, WORD value) {
//	SNROM
//	CHR bank 0 (internal, $A000-$BFFF)
//	4bit0
//	-----
//	ExxxC
//	|   |
//	|   +- Select 4 KB CHR RAM bank at PPU $0000 (ignored in 8 KB mode)
//	+----- PRG RAM disable (0: enable, 1: open bus)
//	CHR bank 1 (internal, $C000-$DFFF)
//	4bit0
//	-----
//	ExxxC
//	|   |
//	|   +- Select 4 KB CHR RAM bank at PPU $1000 (ignored in 8 KB mode)
//	+----- PRG RAM disable (0: enable, 1: open bus) (ignored in 8 KB mode)
//	Both the E bit and the R bit (in standard MMC1 registers) should be clear in order for the PRG RAM to
//	be writable or readable. This bit is more "reliable" on authentic hardware as it is implemented even in
//	older boards with older MMC1's, while the R bit was only introduced later. But because the E bit wasn't
//	confirmed by the homebrew community until October 2010[4], emulators tend not to implement it.
//
//	!!!! : Se Lascio questo controllo attivo "Witch n' Wiz (World) (Digital Release) (Aftermarket) (Unl).nes" ha le
//	schermate sporche dopo la selezione dello slot di salvataggio
//	const BYTE wram_enabled = mmc1tmp.type == MMC1B
//		? (info.mapper.submapper == 0
//			? (mmc1.reg[3] | (mmc1.reg[1] | mmc1.reg[2])) & 0x10 ? FALSE : TRUE
//			: mmc1.reg[3] & 0x10 ? FALSE : TRUE)
//		: TRUE;
	const BYTE wram_enabled = mmc1tmp.type == MMC1B
		? mmc1.reg[3] & 0x10 ? FALSE : TRUE
		: TRUE;

	memmap_auto_wp_8k(0, MMCPU(address), value, wram_enabled, wram_enabled);
}
void mirroring_fix_MMC1_base(void) {
	switch (mmc1.reg[0] & 0x03) {
		case 0x00:
			mirroring_SCR0(0);
			break;
		case 0x01:
			mirroring_SCR1(0);
			break;
		case 0x02:
			mirroring_V(0);
			break;
		case 0x03:
			mirroring_H(0);
			break;
	}
}

WORD prg_bank_MMC1(int index) {
	WORD bank = mmc1.reg[0] & 0x08
		? mmc1.reg[0] & 0x04
			? (mmc1.reg[3] | (index * 0x0F))
			: (mmc1.reg[3] & (index * 0x0F))
		: (mmc1.reg[3] & ~1) | index;

	return ((mmc1.reg[3] & 0x10) && (mmc1tmp.type == MMC1A)
		? (bank & 0x07) | (mmc1.reg[3] & 0x08)
		: bank & 0x0F);
}
WORD chr_bank_MMC1(int index) {
	return (mmc1.reg[0] & 0x10 ? mmc1.reg[1 + index] : (mmc1.reg[1] & ~1) | index);
}
