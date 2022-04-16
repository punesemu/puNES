/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

INLINE static void boy_update_prg(void);
INLINE static void boy_update_chr(void);

#define boy_swap_chr_1k(a, b)\
	chr1k = boy.chr_map[b];\
	boy.chr_map[b] = boy.chr_map[a];\
	boy.chr_map[a] = chr1k
#define boy_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		WORD chr1k;\
		boy_swap_chr_1k(0, 4);\
		boy_swap_chr_1k(1, 5);\
		boy_swap_chr_1k(2, 6);\
		boy_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = boy.prg_map[0];\
		mapper.rom_map_to[0] = boy.prg_map[2];\
		boy.prg_map[0] = mapper.rom_map_to[0];\
		boy.prg_map[2] = mapper.rom_map_to[2];\
		boy.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom[0].max.banks_8k_before_last;\
	}
#define boy_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			value &= 0xFE;\
			boy.chr_map[mmc3.chr_rom_cfg] = value;\
			boy.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			value &= 0xFE;\
			boy.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			boy.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			boy.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			boy.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			boy.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			boy.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			boy.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			boy.prg_map[1] = value;\
			break;\
	}

struct _boy {
	BYTE reg[4];
	WORD prg_map[4];
	WORD chr_map[8];
} boy;

void map_init_BOY(void) {
	EXTCL_AFTER_MAPPER_INIT(BOY);
	EXTCL_CPU_WR_MEM(BOY);
	EXTCL_SAVE_MAPPER(BOY);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&boy;
	mapper.internal_struct_size[0] = sizeof(boy);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&boy, 0x00, sizeof(boy));
	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));

	if (mapper.write_vram == TRUE) {
		info.chr.rom[0].banks_8k = 32;
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_after_mapper_init_BOY(void) {
	BYTE i;

	map_prg_rom_8k_reset();
	map_chr_bank_1k_reset();

	for (i = 0; i < 8; i++) {
		if (i < 4) {
			boy.prg_map[i] = mapper.rom_map_to[i];
		}
		boy.chr_map[i] = i;
	}

	boy_update_prg();
	boy_update_chr();
}
void extcl_cpu_wr_mem_BOY(WORD address, BYTE value) {
	BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
	BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

	switch (address & 0xF001) {
		case 0x6000:
		case 0x6001:
		case 0x7000:
		case 0x7001:
			if ((boy.reg[3] & 0x90) != 0x80) {
				boy.reg[address & 0x03] = value;
				boy_update_prg();
				boy_update_chr();
			}
			return;
		case 0x8000:
		case 0x9000:
			extcl_cpu_wr_mem_MMC3(address, value);
			boy_8000()
			boy_update_prg();
			boy_update_chr();
			return;
		case 0x8001:
		case 0x9001:
			extcl_cpu_wr_mem_MMC3(address, value);
			boy_8001()
			boy_update_prg();
			boy_update_chr();
			return;
		default:
			extcl_cpu_wr_mem_MMC3(address, value);
			return;
	}
}
BYTE extcl_save_mapper_BOY(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, boy.reg);
	save_slot_ele(mode, slot, boy.prg_map);
	save_slot_ele(mode, slot, boy.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void boy_update_prg(void) {
	BYTE i;
	WORD value;
	WORD mask = ((0x3F | (boy.reg[1] & 0x40) |
			((boy.reg[1] & 0x20) << 2)) ^
			((boy.reg[0] & 0x40) >> 2)) ^
			((boy.reg[1] & 0x80) >> 2);
	WORD base = ((boy.reg[0] & 0x07) >> 0) |
			((boy.reg[1] & 0x10) >> 1) |
			((boy.reg[1] & 0x0C) << 2) |
			((boy.reg[0] & 0x30) << 2);

	for (i = 0; i < 4; i++) {
		value = boy.prg_map[i];

		if ((boy.reg[3] & 0x40) && (value >= 0xFE) && !mmc3.prg_rom_cfg) {
			switch (i) {
				case 1:
					if (mmc3.prg_rom_cfg) {
						value = 0;
					}
					break;
				case 2:
					if (!mmc3.prg_rom_cfg) {
						value = 0;
					}
					break;
				case 3:
					value = 0;
					break;
			}
		}
		if (!(boy.reg[3] & 0x10)) {
			value = (((base << 4) & ~mask)) | (value & mask);
		} else {
			BYTE emask;

			mask &= 0xF0;
			if ((((boy.reg[1] & 0x02) != 0))) {
				emask = (boy.reg[3] & 0x0C) | (i & 0x02);
			} else {
				emask = boy.reg[3] & 0x0E;
			}
			value = ((base << 4) & ~mask) | (value & mask) | emask | (i & 0x01);
		}
		control_bank(info.prg.rom[0].max.banks_8k)
		map_prg_rom_8k(1, i, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void boy_update_chr(void) {
	BYTE i, mask = 0xFF ^ (boy.reg[0] & 0x80);
	WORD value;

	for (i = 0; i < 8; i++) {
		value = boy.chr_map[i];
		if (boy.reg[3] & 0x10) {
			if (boy.reg[3] & 0x40) {
				switch (mmc3.chr_rom_cfg ^ i) {
					case 1:
					case 3:
						value &= 0x7F;
						break;
				}
			}
			value = ((value & 0x80) & mask) |
					(((boy.reg[0] & 0x08) << 4) & ~mask) |
					((boy.reg[2] & 0x0F) << 3) | i;
		} else {
			if (boy.reg[3] & 0x40) {
				switch (mmc3.chr_rom_cfg ^ i) {
					case 0:
						value = boy.chr_map[0];
						break;
					case 2:
						value = boy.chr_map[2];
						break;
					case 1:
					case 3:
						value = 0;
						break;
				}
			}
			value = (value & mask) | (((boy.reg[0] & 0x08) << 4) & ~mask);
		}
		control_bank(info.chr.rom[0].max.banks_1k)
		chr.bank_1k[i] = chr_chip_byte_pnt(0, value << 10);
	}
}
