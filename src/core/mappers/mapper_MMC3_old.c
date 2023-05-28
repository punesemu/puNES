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

#define reload_counter()\
{\
	mmc3.irq_counter = mmc3.irq_latch;\
	/* MMC3 Rev A */\
	if (!mmc3.irq_counter && mmc3.irq_reload) {\
		mmc3.save_irq_counter = 1;\
	}\
	mmc3.irq_reload = FALSE;\
}
#define swap_chr_bank_1k(src, dst)\
{\
	BYTE *chr_bank_1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chr_bank_1k;\
}

_mmc3_old mmc3_old;

void map_init_MMC3old(void) {
	EXTCL_CPU_WR_MEM(MMC3_old);
	EXTCL_SAVE_MAPPER(MMC3_old);
	EXTCL_CPU_EVERY_CYCLE(MMC3_old);
	EXTCL_PPU_000_TO_34X(MMC3_old);
	EXTCL_PPU_000_TO_255(MMC3_old);
	EXTCL_PPU_256_TO_319(MMC3_old);
	EXTCL_PPU_320_TO_34X(MMC3_old);
	EXTCL_UPDATE_R2006(MMC3_old);
	mapper.internal_struct[0] = (BYTE *)&mmc3_old;
	mapper.internal_struct_size[0] = sizeof(mmc3_old);

	if (info.reset >= HARD) {
		memset(&mmc3_old, 0x00, sizeof(mmc3_old));
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;

	switch (info.mapper.submapper) {
		default:
		case DEFAULT:
			info.mapper.submapper = MMC3_SHARP;
			break;
		case MMC3_NEC:
			EXTCL_IRQ_A12_CLOCK(MMC3_alternate_old);
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
}
void extcl_cpu_wr_mem_MMC3_old(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000: {
			const BYTE chr_rom_cfg_old = mmc3_old.chr_rom_cfg;
			const BYTE prg_rom_cfg_old = mmc3_old.prg_rom_cfg;

			mmc3_old.bank_to_update = value & 0x07;
			mmc3_old.prg_rom_cfg = (value & 0x40) >> 5;
			mmc3_old.chr_rom_cfg = (value & 0x80) >> 5;
			/*
			 * se il tipo di configurazione della chr cambia,
			 * devo swappare i primi 4 banchi con i secondi
			 * quattro.
			 */
			if (mmc3_old.chr_rom_cfg != chr_rom_cfg_old) {
				swap_chr_bank_1k(0, 4)
				swap_chr_bank_1k(1, 5)
				swap_chr_bank_1k(2, 6)
				swap_chr_bank_1k(3, 7)
			}
			if (mmc3_old.prg_rom_cfg != prg_rom_cfg_old) {
				WORD p0 = mapper.rom_map_to[0];
				WORD p2 = mapper.rom_map_to[2];

				mapper.rom_map_to[0] = p2;
				mapper.rom_map_to[2] = p0;
				/*
				 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco
				 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco
				 */
				map_prg_rom_8k(1, mmc3_old.prg_rom_cfg ^ 0x02, info.prg.rom.max.banks_8k_before_last);
				map_prg_rom_8k_update();
			}
			break;
		}
		case 0x8001: {
			switch (mmc3_old.bank_to_update) {
				case 0:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 0 e 1
					 * chr_rom_cfg 0x04 : chr_bank_1k 4 e 5
					 */
					/*
					 * nel caso value sia dispari, visto che devo
					 * trattare 2Kb, devo considerare come primo
					 * Kb quello pari e come secondo il dispari.
					 * Per questo motivo faccio l'AND con 0xFE.
					 */
					control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)
					chr.bank_1k[mmc3_old.chr_rom_cfg] = chr_pnt(value << 10);
					chr.bank_1k[mmc3_old.chr_rom_cfg | 0x01] = chr_pnt((value + 1) << 10);
					break;
				case 1:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 2 e 3
					 * chr_rom_cfg 0x04 : chr_bank_1k 6 e 7
					 */
					/*
					 * nel caso value sia dispari, visto che devo
					 * trattare 2Kb, devo considerare come primo
					 * Kb quello pari e come secondo il dispari.
					 * Per questo motivo faccio l'AND con 0xFE.
					 */
					control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)
					chr.bank_1k[mmc3_old.chr_rom_cfg | 0x02] = chr_pnt(value << 10);
					chr.bank_1k[mmc3_old.chr_rom_cfg | 0x03] = chr_pnt((value + 1) << 10);
					break;
				case 2:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 4
					 * chr_rom_cfg 0x04 : chr_bank_1k 0
					 */
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[mmc3_old.chr_rom_cfg ^ 0x04] = chr_pnt(value << 10);
					break;
				case 3:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 5
					 * chr_rom_cfg 0x04 : chr_bank_1k 1
					 */
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[(mmc3_old.chr_rom_cfg ^ 0x04) | 0x01] = chr_pnt(value << 10);
					break;
				case 4:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 6
					 * chr_rom_cfg 0x04 : chr_bank_1k 2
					 */
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[(mmc3_old.chr_rom_cfg ^ 0x04) | 0x02] = chr_pnt(value << 10);
					break;
				case 5:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 7
					 * chr_rom_cfg 0x04 : chr_bank_1k 3
					 */
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[(mmc3_old.chr_rom_cfg ^ 0x04) | 0x03] = chr_pnt(value << 10);
					break;
				case 6:
					/*
					 * prg_rom_cfg 0x00 : $8000 - $9FFF swappable
					 * prg_rom_cfg 0x02 : $C000 - $DFFF swappable
					 */
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, mmc3_old.prg_rom_cfg, value);
					map_prg_rom_8k_update();
					break;
				case 7:
					/* $A000 - $BFFF swappable */
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
					break;
			}
			break;
		}
		case 0xA000:
			/*
			 * se e' abilitato il 4 schermi, il cambio
			 * di mirroring deve essere ignorato.
			 */
			if (info.mapper.mirroring == MIRRORING_FOURSCR) {
				break;
			}
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			break;
		case 0xA001: {
			if (info.mapper.submapper != MMC3_MMC6) {
				/*
				 * 7  bit  0
				 * ---- ----
				 * RWxx xxxx
				 * ||
				 * |+-------- Write protection (0: allow writes; 1: deny writes)
				 * +--------- Chip enable (0: disable chip; 1: enable chip)
				 */
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
			/*
			 * in "Downtown Special - Kunio-kun no Jidaigeki Dayo Zenin Shuugou! (J)"
			 * avviene uno sfarfallio dell'ultima riga dello screen perche' avviene
			 * una scrittura in questo registro nel momento esatto in cui avviene un
			 * clock irqA12_SB() facendo gia' caricare il counter con il nuovo latch
			 * cosa che invece dovrebbe avvenire nel clock successivo.
			 */
			irqA12.race.C001 = TRUE;
			irqA12.race.reload = irqA12.reload;
			irqA12.race.counter = irqA12.counter;

			irqA12.reload = TRUE;
			irqA12.counter = 0;
			break;
		case 0xE000:
			irqA12.enable = FALSE;
			/* disabilito l'IRQ dell'MMC3 */
			irq.high &= ~EXT_IRQ;
			break;
		case 0xE001:
			irqA12.enable = TRUE;
			break;
	}
}
BYTE extcl_save_mapper_MMC3_old(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc3_old.prg_ram_protect);
	save_slot_ele(mode, slot, mmc3_old.bank_to_update);
	save_slot_ele(mode, slot, mmc3_old.prg_rom_cfg);
	save_slot_ele(mode, slot, mmc3_old.chr_rom_cfg);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_MMC3_old(void) {
	if (irqA12.delay && !(--irqA12.delay)) {
		irq.high |= EXT_IRQ;
	}
}
void extcl_ppu_000_to_34x_MMC3_old(void) {
	irqA12_RS();
}
void extcl_ppu_000_to_255_MMC3_old(void) {
	if (r2001.visible) {
		irqA12_SB();
	}
}
void extcl_ppu_256_to_319_MMC3_old(void) {
	irqA12_BS();
}
void extcl_ppu_320_to_34x_MMC3_old(void) {
	irqA12_SB();
}
void extcl_update_r2006_MMC3_old(WORD new_r2006, WORD old_r2006) {
	irqA12_IO(new_r2006, old_r2006);
}
void extcl_irq_A12_clock_MMC3_alternate_old(void) {
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
