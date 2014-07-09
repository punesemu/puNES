/*
 * mapper_MMC3.c
 *
 *  Created on: 24/feb/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mem_map.h"
#include "mappers.h"
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

void map_init_MMC3(void) {
	EXTCL_CPU_WR_MEM(MMC3);
	EXTCL_SAVE_MAPPER(MMC3);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &mmc3;
	mapper.internal_struct_size[0] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;

	switch (info.mapper.submapper) {
		case NAMCO3413:
		case NAMCO3414:
		case NAMCO3415:
		case NAMCO3416:
		case NAMCO3417:
		case NAMCO3451:
			mirroring_V();
			break;
		case MMC3_ALTERNATE:
			EXTCL_IRQ_A12_CLOCK(MMC3_alternate);
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
void extcl_cpu_wr_mem_MMC3(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000: {
			const BYTE chr_rom_cfg_old = mmc3.chr_rom_cfg;
			const BYTE prg_rom_cfg_old = mmc3.prg_rom_cfg;

			mmc3.bank_to_update = value & 0x07;
			mmc3.prg_rom_cfg = (value & 0x40) >> 5;
			mmc3.chr_rom_cfg = (value & 0x80) >> 5;
			/*
			 * se il tipo di configurazione della chr cambia,
			 * devo swappare i primi 4 banchi con i secondi
			 * quattro.
			 */
			if (mmc3.chr_rom_cfg != chr_rom_cfg_old) {
				swap_chr_bank_1k(0, 4)
				swap_chr_bank_1k(1, 5)
				swap_chr_bank_1k(2, 6)
				swap_chr_bank_1k(3, 7)
			}
			if (mmc3.prg_rom_cfg != prg_rom_cfg_old) {
				WORD p0 = mapper.rom_map_to[0];
				WORD p2 = mapper.rom_map_to[2];

				mapper.rom_map_to[0] = p2;
				mapper.rom_map_to[2] = p0;
				/*
				 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco
				 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco
				 */
				map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, info.prg.rom.max.banks_8k_before_last);
				map_prg_rom_8k_update();
			}
			break;
		}
		case 0x8001: {
			switch (mmc3.bank_to_update) {
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
					chr.bank_1k[mmc3.chr_rom_cfg] = &chr.data[value << 10];
					chr.bank_1k[mmc3.chr_rom_cfg | 0x01] = &chr.data[(value + 1) << 10];
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
					chr.bank_1k[mmc3.chr_rom_cfg | 0x02] = &chr.data[value << 10];
					chr.bank_1k[mmc3.chr_rom_cfg | 0x03] = &chr.data[(value + 1) << 10];
					break;
				case 2:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 4
					 * chr_rom_cfg 0x04 : chr_bank_1k 0
					 */
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[mmc3.chr_rom_cfg ^ 0x04] = &chr.data[value << 10];
					break;
				case 3:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 5
					 * chr_rom_cfg 0x04 : chr_bank_1k 1
					 */
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = &chr.data[value << 10];
					break;
				case 4:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 6
					 * chr_rom_cfg 0x04 : chr_bank_1k 2
					 */
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = &chr.data[value << 10];
					break;
				case 5:
					/*
					 * chr_rom_cfg 0x00 : chr_bank_1k 7
					 * chr_rom_cfg 0x04 : chr_bank_1k 3
					 */
					control_bank(info.chr.rom.max.banks_1k)
					chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = &chr.data[value << 10];
					break;
				case 6:
					/*
					 * prg_rom_cfg 0x00 : $8000 - $9FFF swappable
					 * prg_rom_cfg 0x02 : $C000 - $DFFF swappable
					 */
					control_bank(info.prg.rom.max.banks_8k)
					map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);
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
			if (mapper.mirroring == MIRRORING_FOURSCR) {
				break;
			}
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			break;
		case 0xA001: {
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
			break;
		}
		case 0xC000:
			irqA12.latch = value;
			break;
		case 0xC001:
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
BYTE extcl_save_mapper_MMC3(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc3.prg_ram_protect);
	save_slot_ele(mode, slot, mmc3.bank_to_update);
	save_slot_ele(mode, slot, mmc3.prg_rom_cfg);
	save_slot_ele(mode, slot, mmc3.chr_rom_cfg);

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
void extcl_update_r2006_MMC3(WORD old_r2006) {
	irqA12_IO(old_r2006);
}
void extcl_irq_A12_clock_MMC3_alternate(void) {
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
