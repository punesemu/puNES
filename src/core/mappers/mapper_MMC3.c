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

void (*MMC3_prg_fix)(BYTE value);
void (*MMC3_prg_swap)(WORD address, WORD value);
void (*MMC3_chr_fix)(BYTE value);
void (*MMC3_chr_swap)(WORD address, WORD value);
void (*MMC3_mirroring_fix)(void);

_mmc3 mmc3;

void map_init_MMC3(void) {
	EXTCL_AFTER_MAPPER_INIT(MMC3);
	EXTCL_CPU_WR_MEM(MMC3);
	EXTCL_SAVE_MAPPER(MMC3);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	if (info.reset >= HARD) {
		init_MMC3();
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	switch (info.mapper.submapper) {
		default:
		case DEFAULT:
			info.mapper.submapper = MMC3_SHARP;
			break;
		case MMC3_NEC:
			EXTCL_IRQ_A12_CLOCK(MMC3_NEC);
			break;
	}

	if (info.id == SMB2JSMB1) {
		info.prg.ram.banks_8k_plus = 1;
	}

	if (info.id == SMB2EREZA) {
		info.prg.ram.bat.banks = FALSE;
	}

	if (info.id == RADRACER2) {
		mirroring_FSCR();
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_MMC3(void) {
	MMC3_prg_fix(mmc3.bank_to_update);
	MMC3_chr_fix(mmc3.bank_to_update);
	MMC3_mirroring_fix();
}
void extcl_cpu_wr_mem_MMC3(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000:
			if ((value & 0x40) != (mmc3.bank_to_update & 0x40)) {
				MMC3_prg_fix(value);
			}
			if ((value & 0x80) != (mmc3.bank_to_update & 0x80)) {
				MMC3_chr_fix(value);
			}
			mmc3.bank_to_update = value;
			return;
		case 0x8001: {
			WORD cbase = (mmc3.bank_to_update & 0x80) << 5;

			mmc3.reg[mmc3.bank_to_update & 0x07] = value;

			switch (mmc3.bank_to_update & 0x07) {
				case 0:
					MMC3_chr_swap(cbase ^ 0x0000, value & (~1));
					MMC3_chr_swap(cbase ^ 0x0400, value | 1);
					return;
				case 1:
					MMC3_chr_swap(cbase ^ 0x0800, value & (~1));
					MMC3_chr_swap(cbase ^ 0x0C00, value | 1);
					return;
				case 2:
					MMC3_chr_swap(cbase ^ 0x1000, value);
					return;
				case 3:
					MMC3_chr_swap(cbase ^ 0x1400, value);
					return;
				case 4:
					MMC3_chr_swap(cbase ^ 0x1800, value);
					return;
				case 5:
					MMC3_chr_swap(cbase ^ 0x1C00, value);
					return;
				case 6:
					if (mmc3.bank_to_update & 0x40) {
						MMC3_prg_swap(0xC000, value);
					} else {
						MMC3_prg_swap(0x8000, value);
					}
					return;
				case 7:
					MMC3_prg_swap(0xA000, value);
					return;
			}
			return;
		}
		case 0xA000:
			mmc3.mirroring = value;
			MMC3_mirroring_fix();
			break;
		case 0xA001: {
			if (info.mapper.submapper != MMC3_MMC6) {
				// 7  bit  0
				// ---- ----
				// RWxx xxxx
				// ||
				// |+-------- Write protection (0: allow writes; 1: deny writes)
				// +--------- Chip enable (0: disable chip; 1: enable chip)
				switch ((value & 0xC0) >> 6) {
					case 0x00:
					case 0x01:
						cpu.prg_ram_rd_active = cpu.prg_ram_wr_active = FALSE;
						break;
					case 0x02:
						cpu.prg_ram_rd_active = cpu.prg_ram_wr_active = TRUE;
						break;
					case 0x03:
						cpu.prg_ram_rd_active = TRUE;
						cpu.prg_ram_wr_active = FALSE;
						break;
				}
			}
			break;
		}
		case 0xC000:
			irqA12.latch = value;
			break;
		case 0xC001:
			// in "Downtown Special - Kunio-kun no Jidaigeki Dayo Zenin Shuugou! (J)"
			// avviene uno sfarfallio dell'ultima riga dello screen perche' avviene
			// una scrittura in questo registro nel momento esatto in cui avviene un
			// clock irqA12_SB() facendo gia' caricare il counter con il nuovo latch
			// cosa che invece dovrebbe avvenire nel clock successivo.
			irqA12.race.C001 = TRUE;
			irqA12.race.reload = irqA12.reload;
			irqA12.race.counter = irqA12.counter;

			irqA12.reload = TRUE;
			irqA12.counter = 0;
			break;
		case 0xE000:
			irqA12.enable = FALSE;
			// disabilito l'IRQ dell'MMC3
			irq.high &= ~EXT_IRQ;
			break;
		case 0xE001:
			irqA12.enable = TRUE;
			break;
	}
}
BYTE extcl_save_mapper_MMC3(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc3.bank_to_update);
	save_slot_ele(mode, slot, mmc3.reg);
	save_slot_ele(mode, slot, mmc3.mirroring);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_MMC3(void) {
	if (irqA12.delay && !(--irqA12.delay)) {
		irq.high |= EXT_IRQ;
	}
}
void extcl_ppu_000_to_34x_MMC3(void) {
	irqA12_RS();
}
void extcl_ppu_000_to_255_MMC3(void) {
	if (r2001.visible) {
		irqA12_SB();
	}
}
void extcl_ppu_256_to_319_MMC3(void) {
	irqA12_BS();
}
void extcl_ppu_320_to_34x_MMC3(void) {
	irqA12_SB();
}
void extcl_update_r2006_MMC3(WORD new_r2006, WORD old_r2006) {
	irqA12_IO(new_r2006, old_r2006);
}
void extcl_irq_A12_clock_MMC3_NEC(void) {
	if (!irqA12.counter) {
		irqA12.counter = irqA12.latch;
		irqA12.reload = FALSE;
	} else {
		irqA12.counter--;
	}
	if (!irqA12.counter && irqA12.enable) {
		irq.high |= EXT_IRQ;
	}
}

void init_MMC3(void) {
	memset(&mmc3, 0x00, sizeof(mmc3));

	mmc3.reg[0] = 0;
	mmc3.reg[1] = 2;
	mmc3.reg[2] = 4;
	mmc3.reg[3] = 5;
	mmc3.reg[4] = 6;
	mmc3.reg[5] = 7;
	mmc3.reg[6] = 0;
	mmc3.reg[7] = 1;

	MMC3_prg_fix = prg_fix_MMC3;
	MMC3_prg_swap = prg_swap_MMC3;
	MMC3_chr_fix = chr_fix_MMC3;
	MMC3_chr_swap = chr_swap_MMC3;
	MMC3_mirroring_fix = mirroring_fix_MMC3;
}
void prg_fix_MMC3(BYTE value) {
	if (value & 0x40) {
		MMC3_prg_swap(0x8000, ~1);
		MMC3_prg_swap(0xC000, mmc3.reg[6]);
	} else {
		MMC3_prg_swap(0x8000, mmc3.reg[6]);
		MMC3_prg_swap(0xC000, ~1);
	}
	MMC3_prg_swap(0xA000, mmc3.reg[7]);
	MMC3_prg_swap(0xE000, ~0);
}
void prg_swap_MMC3(WORD address, WORD value) {
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, (address >> 13) & 0x03, value);
	map_prg_rom_8k_update();
}
void chr_fix_MMC3(BYTE value) {
	WORD cbase = (value & 0x80) << 5;

	MMC3_chr_swap(cbase ^ 0x0000, mmc3.reg[0] & (~1));
	MMC3_chr_swap(cbase ^ 0x0400, mmc3.reg[0] |   1);
	MMC3_chr_swap(cbase ^ 0x0800, mmc3.reg[1] & (~1));
	MMC3_chr_swap(cbase ^ 0x0C00, mmc3.reg[1] |   1);
	MMC3_chr_swap(cbase ^ 0x1000, mmc3.reg[2]);
	MMC3_chr_swap(cbase ^ 0x1400, mmc3.reg[3]);
	MMC3_chr_swap(cbase ^ 0x1800, mmc3.reg[4]);
	MMC3_chr_swap(cbase ^ 0x1C00, mmc3.reg[5]);
}
void chr_swap_MMC3(WORD address, WORD value) {
	control_bank(info.chr.rom.max.banks_1k)
	chr.bank_1k[address >> 10] = chr_pnt(value << 10);
}
void mirroring_fix_MMC3(void) {
	// se e' abilitato il 4 schermi, il cambio
	// di mirroring deve essere ignorato.
	if (mapper.mirroring == MIRRORING_FOURSCR) {
		return;
	}
	if (mmc3.mirroring & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
