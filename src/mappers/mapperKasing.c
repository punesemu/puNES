/*
 * mapperKasing.c
 *
 *  Created on: 28/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

#define kasing_swap_prg_rom_32k()\
	value = kasing.prg_high;\
	control_bank(prg_rom_32k_max)\
	map_prg_rom_8k(4, 0, value)
#define kasing_intercept_8001_prg(slot)\
	if (kasing.prg_mode) {\
		kasing_swap_prg_rom_32k();\
	} else {\
		control_bank(prg_rom_8k_max)\
		map_prg_rom_8k(1, slot, value);\
	}\
	map_prg_rom_8k_update()
#define kasing_swap_chr_rom_bank_1k(slot1, slot2)\
{\
	WORD tmp = kasing.chr_rom_bank[slot1];\
	kasing.chr_rom_bank[slot1] = kasing.chr_rom_bank[slot2];\
	kasing.chr_rom_bank[slot2] = tmp;\
}
#define kasing_chr_1k_update(slot)\
{\
	WORD tmp = (kasing.chr_high << 8) & 0x0100;\
	chr.bank_1k[slot] = &chr.data[(tmp | kasing.chr_rom_bank[slot]) << 10];\
}
#define kasing_intercept_8001_chr(slot, val)\
{\
	BYTE bank = slot;\
	kasing.chr_rom_bank[bank] = val;\
	kasing_chr_1k_update(bank)\
}
#define kasing_chr_update()\
{\
	BYTE i;\
	for (i = 0; i < 8 ; i++) {\
		kasing_chr_1k_update(i)\
	}\
}

WORD prg_rom_32k_max, prg_rom_8k_max, prg_rom_8k_before_last, chr_rom_1k_max;

void map_init_Kasing(void) {
	prg_rom_32k_max = (info.prg_rom_16k_count >> 1) - 1;
	prg_rom_8k_max = info.prg_rom_8k_count - 1;
	prg_rom_8k_before_last = info.prg_rom_8k_count - 2;
	chr_rom_1k_max = info.chr_rom_1k_count - 1;

	EXTCL_CPU_WR_MEM(Kasing);
	EXTCL_SAVE_MAPPER(Kasing);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &kasing;
	mapper.internal_struct_size[0] = sizeof(kasing);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&kasing, 0x00, sizeof(kasing));
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));

		for (i = 0; i < 8; i++) {
			kasing.chr_rom_bank[i] = i;
		}

	} else {
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	info.mapper_extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_Kasing(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}

	/* intercetto cio' che mi interessa */
	if (address == 0x6000) {
		const BYTE prg_mode_old = kasing.prg_mode;
		const BYTE prg_high_old = kasing.prg_high;

		kasing.prg_high = (value & 0x7F) >> 1;
		kasing.prg_mode = value & 0x80;

		if ((kasing.prg_high != prg_high_old) || (kasing.prg_mode != prg_mode_old)) {
			if (kasing.prg_mode) {
				kasing.prg_rom_bank[0] = mapper.rom_map_to[0];
				kasing.prg_rom_bank[1] = mapper.rom_map_to[1];
				kasing.prg_rom_bank[2] = mapper.rom_map_to[2];
				kasing.prg_rom_bank[3] = mapper.rom_map_to[3];
				kasing_swap_prg_rom_32k();
			} else {
				mapper.rom_map_to[0] = kasing.prg_rom_bank[0];
				mapper.rom_map_to[1] = kasing.prg_rom_bank[1];
				mapper.rom_map_to[2] = kasing.prg_rom_bank[2];
				mapper.rom_map_to[3] = kasing.prg_rom_bank[3];
			}
			map_prg_rom_8k_update();
		}
		return;
	} else if (address == 0x6001) {
		if (kasing.chr_high != value) {
			kasing.chr_high = value;
			kasing_chr_update()
		}
		return;
	}

	if (address < 0x8000) {
		return;
	}

	address &= 0xE001;

	if (address == 0x8000) {
		BYTE prg_rom_cfg = (value & 0x40) >> 5;
		BYTE chr_rom_cfg = (value & 0x80) >> 5;

		if (mmc3.chr_rom_cfg != chr_rom_cfg) {
			mmc3.chr_rom_cfg = chr_rom_cfg;
			kasing_swap_chr_rom_bank_1k(0, 4)
			kasing_swap_chr_rom_bank_1k(1, 5)
			kasing_swap_chr_rom_bank_1k(2, 6)
			kasing_swap_chr_rom_bank_1k(3, 7)
		}
		if (mmc3.prg_rom_cfg != prg_rom_cfg) {
			/* intercetto solo se il prg_mode non e' a zero */
			if (kasing.prg_mode) {
				mmc3.prg_rom_cfg = prg_rom_cfg;
				kasing_swap_prg_rom_32k();
				map_prg_rom_8k_update();
			}
		}
	} else if (address == 0x8001) {
		switch (mmc3.bank_to_update) {
			case 0:
				kasing_intercept_8001_chr(mmc3.chr_rom_cfg, value)
				kasing_intercept_8001_chr(mmc3.chr_rom_cfg | 0x01, value + 1)
				break;
			case 1:
				kasing_intercept_8001_chr(mmc3.chr_rom_cfg | 0x02, value)
				kasing_intercept_8001_chr(mmc3.chr_rom_cfg | 0x03, value + 1)
				break;
			case 2:
				kasing_intercept_8001_chr(mmc3.chr_rom_cfg ^ 0x04, value)
				break;
			case 3:
				kasing_intercept_8001_chr((mmc3.chr_rom_cfg ^ 0x04) | 0x01, value)
				break;
			case 4:
				kasing_intercept_8001_chr((mmc3.chr_rom_cfg ^ 0x04) | 0x02, value)
				break;
			case 5:
				kasing_intercept_8001_chr((mmc3.chr_rom_cfg ^ 0x04) | 0x03, value)
				break;
			case 6:
				kasing_intercept_8001_prg(mmc3.prg_rom_cfg);
				break;
			case 7:
				kasing_intercept_8001_prg(1);
				break;
		}
		return;
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_save_mapper_Kasing(BYTE mode, BYTE slot, FILE *fp) {
	if (save_slot.version >= 6) {
		save_slot_ele(mode, slot, kasing.prg_mode);
	}
	save_slot_ele(mode, slot, kasing.prg_high);
	if (save_slot.version < 6) {
		if (mode == SAVE_SLOT_READ) {
			BYTE old_prg_rom_bank[4], i;

			save_slot_ele(mode, slot, old_prg_rom_bank)

			for (i = 0; i < 4; i++) {
				kasing.prg_rom_bank[i] = old_prg_rom_bank[i];
			}
		} else if (mode == SAVE_SLOT_COUNT) {
			save_slot.tot_size[slot] += sizeof(BYTE) * 4;
		}
	} else {
		save_slot_ele(mode, slot, kasing.prg_rom_bank);
	}
	save_slot_ele(mode, slot, kasing.chr_high);
	save_slot_ele(mode, slot, kasing.chr_rom_bank);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
