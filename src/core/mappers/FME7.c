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
#include "cpu.h"
#include "save_slot.h"

void (*FME7_prg_fix)(void);
void (*FME7_prg_swap)(WORD address, WORD value);
void (*FME7_chr_fix)(void);
void (*FME7_chr_swap)(WORD address, WORD value);
void (*FME7_wram_fix)(void);
void (*FME7_wram_swap)(WORD value);
void (*FME7_mirroring_fix)(void);

INLINE static void fme7_square_tick(_square_fme7 *square);

_fme7 fme7;

// promemoria
//void map_init_FME7(BYTE revision) {
//	EXTCL_AFTER_MAPPER_INIT(FME7);
//	EXTCL_CPU_WR_MEM(FME7);
//	EXTCL_SAVE_MAPPER(FME7);
//	EXTCL_CPU_EVERY_CYCLE(FME7);
//	EXTCL_APU_TICK(FME7);
//}

void extcl_after_mapper_init_FME7(void) {
	FME7_prg_fix();
	FME7_chr_fix();
	FME7_wram_fix();
	FME7_mirroring_fix();
}
void extcl_cpu_wr_mem_FME7(WORD address, BYTE value) {
	int index = 0;

	switch (address & 0xE000) {
		case 0x8000:
			fme7.reg = value;
			return;
		case 0xA000:
			index = fme7.reg & 0x0F;
			switch (index) {
				case 0x00:
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				case 0x06:
				case 0x07:
					fme7.chr[index] = value;
					FME7_chr_fix();
					return;
				case 0x08:
				case 0x09:
				case 0x0A:
				case 0x0B:
					fme7.prg[index & 0x03] = value;
					FME7_prg_fix();
					FME7_wram_fix();
					return;
				case 0x0C:
					fme7.mirroring = value;
					FME7_mirroring_fix();
					return;
				case 0x0D:
					fme7.irq.control = value;
					irq.high &= ~EXT_IRQ;
					return;
				case 0x0E:
					fme7.irq.count = (fme7.irq.count & 0xFF00) | value;
					return;
				case 0x0F:
					fme7.irq.count = (fme7.irq.count & 0x00FF) | (value << 8);
					return;
				default:
					return;
			}
			return;
		case 0xC000:
			fme7.snd.reg = value;
			return;
		case 0xE000:
			index = fme7.snd.reg & 0x0F;
			switch (index) {
				case 0x00:
				case 0x02:
				case 0x04:
					index >>= 1;
					fme7.snd.square[index].frequency = (fme7.snd.square[index].frequency & 0x0F00) | value;
					return;
				case 0x01:
				case 0x03:
				case 0x05:
					index >>= 1;
					fme7.snd.square[index].frequency = (fme7.snd.square[index].frequency & 0x00FF) | ((value & 0x0F) << 8);
					return;
				case 0x07:
					fme7.snd.square[0].disable = value & 0x01;
					fme7.snd.square[1].disable = value & 0x02;
					fme7.snd.square[2].disable = value & 0x04;
					return;
				case 0x08:
				case 0x09:
				case 0x0A:
					index &= 0x03;
					fme7.snd.square[index].volume = value & 0x0F;
					return;
			}
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_FME7(BYTE mode, BYTE slot, FILE *fp) {
	int i = 0;

	save_slot_ele(mode, slot, fme7.reg);
	save_slot_ele(mode, slot, fme7.prg);
	save_slot_ele(mode, slot, fme7.chr);
	save_slot_ele(mode, slot, fme7.mirroring);
	save_slot_ele(mode, slot, fme7.irq.control);
	save_slot_ele(mode, slot, fme7.irq.count);
	save_slot_ele(mode, slot, fme7.snd.reg);
	for (i = 0; i < (int)LENGTH(fme7.snd.square); i++) {
		save_slot_ele(mode, slot, fme7.snd.square[i].disable);
		save_slot_ele(mode, slot, fme7.snd.square[i].step);
		save_slot_ele(mode, slot, fme7.snd.square[i].frequency);
		save_slot_ele(mode, slot, fme7.snd.square[i].timer);
		save_slot_ele(mode, slot, fme7.snd.square[i].volume);
		save_slot_ele(mode, slot, fme7.snd.square[i].output);
	}

	if (mode == SAVE_SLOT_READ) {
		FME7_wram_fix();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_FME7(void) {
	// nell'FME7 l'IRQ viene generato quando il contatore passa da 0x0000 a 0xFFFF.
	// Nella vecchia gestione utilizzavo il solito delay di un ciclo, ma a quanto pare
	// se lo genero quando a 0x0000, proprio per il famigerato delay con cui gira
	// l'emulatore compenso il fatto di non generarlo a 0xFFFF. Facendo cosi'
	// supero i test M69_P128K_C64K_S8K.nes e M69_P128K_C64K_W8K.nes del set
	// holydiverbatman-bin-0.01.7z
	if (!(fme7.irq.control & 0x80)) {
		return;
	}

	if (!(--fme7.irq.count) && (fme7.irq.control & 0x01)) {
		irq.high |= EXT_IRQ;
	}
}
void extcl_apu_tick_FME7(void) {
	fme7_square_tick(&fme7.snd.square[0]);
	fme7_square_tick(&fme7.snd.square[1]);
	fme7_square_tick(&fme7.snd.square[2]);
}

void init_NSF_FME7(void) {
	memset(&fme7, 0x00, sizeof(fme7));

	fme7.snd.square[0].timer = 1;
	fme7.snd.square[1].timer = 1;
	fme7.snd.square[2].timer = 1;
}
void init_FME7(void) {
	if (info.reset >= HARD) {
		memset(&fme7, 0x00, sizeof(fme7));

		fme7.prg[0] = 0;
		fme7.prg[1] = 0;
		fme7.prg[2] = 0x01;
		fme7.prg[3] = 0xFE;

		fme7.chr[0] = 0;
		fme7.chr[1] = 1;
		fme7.chr[2] = 2;
		fme7.chr[3] = 3;
		fme7.chr[4] = 4;
		fme7.chr[5] = 5;
		fme7.chr[6] = 6;
		fme7.chr[7] = 7;
	}

	fme7.snd.square[0].timer = 1;
	fme7.snd.square[1].timer = 1;
	fme7.snd.square[2].timer = 1;

	FME7_prg_fix = prg_fix_FME7_base;
	FME7_prg_swap = prg_swap_FME7_base;
	FME7_chr_fix = chr_fix_FME7_base;
	FME7_chr_swap = chr_swap_FME7_base;
	FME7_wram_fix = wram_fix_FME7_base;
	FME7_wram_swap = wram_swap_FME7_base;
	FME7_mirroring_fix = mirroring_fix_FME7_base;
}
void prg_fix_FME7_base(void) {
	FME7_prg_swap(0x8000, fme7.prg[1]);
	FME7_prg_swap(0xA000, fme7.prg[2]);
	FME7_prg_swap(0xC000, fme7.prg[3]);
	FME7_prg_swap(0xE000, ~0);
}
void prg_swap_FME7_base(WORD address, WORD value) {
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_fix_FME7_base(void) {
	FME7_chr_swap(0x0000, fme7.chr[0]);
	FME7_chr_swap(0x0400, fme7.chr[1]);
	FME7_chr_swap(0x0800, fme7.chr[2]);
	FME7_chr_swap(0x0C00, fme7.chr[3]);
	FME7_chr_swap(0x1000, fme7.chr[4]);
	FME7_chr_swap(0x1400, fme7.chr[5]);
	FME7_chr_swap(0x1800, fme7.chr[6]);
	FME7_chr_swap(0x1C00, fme7.chr[7]);
}
void chr_swap_FME7_base(WORD address, WORD value) {
	map_chr_rom_1k(address, value);
}
void wram_fix_FME7_base(void) {
	switch (fme7.prg[0] & 0xC0) {
		default:
		case 0x00:
		case 0x80:
			cpu.prg_ram_rd_active = TRUE;
			cpu.prg_ram_wr_active = FALSE;
			FME7_wram_swap(fme7.prg[0] & 0x3F);
			return;
		case 0x40:
		case 0xC0:
			cpu.prg_ram_rd_active = fme7.prg[0] >> 7;
			cpu.prg_ram_wr_active = cpu.prg_ram_rd_active;
			FME7_wram_swap(fme7.prg[0] & 0x3F);
			return;
	}
}
void wram_swap_FME7_base(WORD value) {
	prg.ram_plus_8k = NULL;
	if (fme7.prg[0] & 0x40) {
		if (info.prg.ram.banks_8k_plus) {
			control_bank(info.prg.ram.banks_8k_plus - 1)
			prg.ram_plus_8k = &prg.ram_plus[value << 13];
		}
	} else {
		control_bank_with_AND(0x3F, info.prg.rom.max.banks_8k)
		prg.ram_plus_8k = prg_pnt(value << 13);
	}
}
void mirroring_fix_FME7_base(void) {
	switch (fme7.mirroring & 0x03) {
		default:
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

INLINE static void fme7_square_tick(_square_fme7 *square) {
	square->output = 0;
	if (--square->timer == 0) {
		square->step = (square->step + 1) & 0x1F;
		square->timer = square->frequency + 1;
		fme7.clocked = TRUE;
	}
	if (!square->disable) {
		// square->output = -square->volume * ((square->step & 0x10) ? 2 : -2);
		square->output = square->volume * ((square->step & 0x10) ? 1 : 0);
	}
}
