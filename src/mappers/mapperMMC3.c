/*
 * mapperMMC3.c
 *
 *  Created on: 24/feb/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "memmap.h"
#include "mappers.h"
#include "irqA12.h"
#include "savestate.h"

#define reloadCounter()\
{\
	mmc3.irqCounter = mmc3.irqLatch;\
	/* MMC3 Rev A */\
	if (!mmc3.irqCounter && mmc3.irqReload) {\
		mmc3.saveIrqCounter = 1;\
	}\
	mmc3.irqReload = FALSE;\
}

WORD prgRom8kMax, prgRom8kBeforeLast, chrRom1kMax;

void map_init_MMC3(void) {
	prgRom8kMax = info.prg_rom_8k_count - 1;
	prgRom8kBeforeLast = info.prg_rom_8k_count - 2;
	chrRom1kMax = info.chr_rom_1k_count - 1;

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

	switch (info.mapper_type) {
		case NAMCO3413:
		case NAMCO3414:
		case NAMCO3415:
		case NAMCO3416:
		case NAMCO3417:
		case NAMCO3451:
			mirroring_V();
			break;
	}

	if (info.id == SMB2JSMB1) {
		info.prg_ram_plus_8k_count = 1;
	}

	if (info.id == SMB2EREZA) {
		info.prg_ram_bat_banks = FALSE;
	}

	if (info.id == RADRACER2) {
		mirroring_FSCR();
	}
}
void extcl_cpu_wr_mem_MMC3(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8000: {
			const BYTE chrRomCfgOld = mmc3.chrRomCfg;
			const BYTE prgRomCfgOld = mmc3.prgRomCfg;

			mmc3.bankToUpdate = value & 0x07;
			mmc3.prgRomCfg = (value & 0x40) >> 5;
			mmc3.chrRomCfg = (value & 0x80) >> 5;
			/*
			 * se il tipo di configurazione della chr cambia,
			 * devo swappare i primi 4 banchi con i secondi
			 * quattro.
			 */
			if (mmc3.chrRomCfg != chrRomCfgOld) {
				swapChrBank1k(0, 4)
				swapChrBank1k(1, 5)
				swapChrBank1k(2, 6)
				swapChrBank1k(3, 7)
			}
			if (mmc3.prgRomCfg != prgRomCfgOld) {
				WORD p0 = mapper.rom_map_to[0];
				WORD p2 = mapper.rom_map_to[2];
				mapper.rom_map_to[0] = p2;
				mapper.rom_map_to[2] = p0;
				/*
				 * prgRomCfg 0x00 : $C000 - $DFFF fisso al penultimo banco
				 * prgRomCfg 0x02 : $8000 - $9FFF fisso al penultimo banco
				 */
				map_prg_rom_8k(1, mmc3.prgRomCfg ^ 0x02, prgRom8kBeforeLast);
				map_prg_rom_8k_update();
			}
			break;
		}
		case 0x8001: {
			switch (mmc3.bankToUpdate) {
				case 0:
					/*
					 * chrRomCfg 0x00 : chrBank1k 0 e 1
					 * chrRomCfg 0x04 : chrBank1k 4 e 5
					 */
					/*
					 * nel caso value sia dispari, visto che devo
					 * trattare 2Kb, devo considerare come primo
					 * Kb quello pari e come secondo il dispari.
					 * Per questo motivo faccio l'AND con 0xFE.
					 */
					control_bank_with_AND(0xFE, chrRom1kMax)
					chr.bank_1k[mmc3.chrRomCfg] = &chr.data[value << 10];
					chr.bank_1k[mmc3.chrRomCfg | 0x01] = &chr.data[(value + 1) << 10];
					break;
				case 1:
					/*
					 * chrRomCfg 0x00 : chrBank1k 2 e 3
					 * chrRomCfg 0x04 : chrBank1k 6 e 7
					 */
					/*
					 * nel caso value sia dispari, visto che devo
					 * trattare 2Kb, devo considerare come primo
					 * Kb quello pari e come secondo il dispari.
					 * Per questo motivo faccio l'AND con 0xFE.
					 */
					control_bank_with_AND(0xFE, chrRom1kMax)
					chr.bank_1k[mmc3.chrRomCfg | 0x02] = &chr.data[value << 10];
					chr.bank_1k[mmc3.chrRomCfg | 0x03] = &chr.data[(value + 1) << 10];
					break;
				case 2:
					/*
					 * chrRomCfg 0x00 : chrBank1k 4
					 * chrRomCfg 0x04 : chrBank1k 0
					 */
					control_bank(chrRom1kMax)
					chr.bank_1k[mmc3.chrRomCfg ^ 0x04] = &chr.data[value << 10];
					break;
				case 3:
					/*
					 * chrRomCfg 0x00 : chrBank1k 5
					 * chrRomCfg 0x04 : chrBank1k 1
					 */
					control_bank(chrRom1kMax)
					chr.bank_1k[(mmc3.chrRomCfg ^ 0x04) | 0x01] = &chr.data[value << 10];
					break;
				case 4:
					/*
					 * chrRomCfg 0x00 : chrBank1k 6
					 * chrRomCfg 0x04 : chrBank1k 2
					 */
					control_bank(chrRom1kMax)
					chr.bank_1k[(mmc3.chrRomCfg ^ 0x04) | 0x02] = &chr.data[value << 10];
					break;
				case 5:
					/*
					 * chrRomCfg 0x00 : chrBank1k 7
					 * chrRomCfg 0x04 : chrBank1k 3
					 */
					control_bank(chrRom1kMax)
					chr.bank_1k[(mmc3.chrRomCfg ^ 0x04) | 0x03] = &chr.data[value << 10];
					break;
				case 6:
					/*
					 * prgRomCfg 0x00 : $8000 - $9FFF swappable
					 * prgRomCfg 0x02 : $C000 - $DFFF swappable
					 */
					control_bank(prgRom8kMax)
					map_prg_rom_8k(1, mmc3.prgRomCfg, value);
					map_prg_rom_8k_update();
					break;
				case 7:
					/* $A000 - $BFFF swappable */
					control_bank(prgRom8kMax)
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
			irq.high &= ~EXTIRQ;
			break;
		case 0xE001:
			irqA12.enable = TRUE;
			break;
	}
}
BYTE extcl_save_mapper_MMC3(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, mmc3.prgRamProtect);
	save_slot_ele(mode, slot, mmc3.bankToUpdate);
	save_slot_ele(mode, slot, mmc3.prgRomCfg);
	save_slot_ele(mode, slot, mmc3.chrRomCfg);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_MMC3(void) {
	if (irqA12.delay && !(--irqA12.delay)) {
		irq.high |= EXTIRQ;
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
