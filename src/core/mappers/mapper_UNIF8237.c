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

INLINE static void unif8237_update_prg(void);
INLINE static void unif8237_update_chr(void);
INLINE static void unif8237a_update_prg(void);
INLINE static void unif8237a_update_chr(void);

#define unif8237_prg_8k_a(vl) value = ((unif8237.reg[1] & 0x03) << 5) | (vl & 0x0F) | bnk0
#define unif8237_prg_8k_b(vl) value = ((unif8237.reg[1] & 0x03) << 5) | (vl & 0x1F)

#define unif8237a_prg_8k_a(vl) value = ((unif8237.reg[1] & 0x03) << 5) | ((unif8237.reg[1] & 0x08) << 4) | (vl & 0x0F) | bnk0
#define unif8237a_prg_8k_b(vl) value = ((unif8237.reg[1] & 0x03) << 5) | ((unif8237.reg[1] & 0x08) << 4) | (vl & 0x1F)

#define unif8237_swap_chr_1k(a, b)\
	chr1k = unif8237.chr_map[b];\
	unif8237.chr_map[b] = unif8237.chr_map[a];\
	unif8237.chr_map[a] = chr1k
#define unif8237_8000()\
	if (mmc3.chr_rom_cfg != old_chr_rom_cfg) {\
		BYTE chr1k;\
		unif8237_swap_chr_1k(0, 4);\
		unif8237_swap_chr_1k(1, 5);\
		unif8237_swap_chr_1k(2, 6);\
		unif8237_swap_chr_1k(3, 7);\
	}\
	if (mmc3.prg_rom_cfg != old_prg_rom_cfg) {\
		mapper.rom_map_to[2] = unif8237.prg_map[0];\
		mapper.rom_map_to[0] = unif8237.prg_map[2];\
		unif8237.prg_map[0] = mapper.rom_map_to[0];\
		unif8237.prg_map[2] = mapper.rom_map_to[2];\
		unif8237.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom.max.banks_8k_before_last;\
	}
#define unif8237_8001()\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)\
			unif8237.chr_map[mmc3.chr_rom_cfg] = value;\
			unif8237.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			break;\
		case 1:\
			control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)\
			unif8237.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			unif8237.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			break;\
		case 2:\
			unif8237.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			break;\
		case 3:\
			unif8237.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			break;\
		case 4:\
			unif8237.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			break;\
		case 5:\
			unif8237.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			break;\
		case 6:\
			unif8237.prg_map[mmc3.prg_rom_cfg] = value;\
			break;\
		case 7:\
			unif8237.prg_map[1] = value;\
			break;\
	}
#define unif8237_updt_prg()\
	if (unif8237tmp.model == U8237) {\
		unif8237_update_prg();\
	} else {\
		unif8237a_update_prg();\
	}
#define unif8237_updt_prg_and_chr()\
	if (unif8237tmp.model == U8237) {\
		unif8237_update_prg();\
		unif8237_update_chr();\
	} else {\
		unif8237a_update_prg();\
		unif8237a_update_chr();\
	}

static const BYTE unif8237_reg[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 2, 6, 1, 7, 3, 4, 5 },
	{ 0, 5, 4, 1, 7, 2, 6, 3 },
	{ 0, 6, 3, 7, 5, 2, 4, 1 },
	{ 0, 2, 5, 3, 6, 1, 7, 4 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
};
static const BYTE unif8237_adr[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 3, 2, 0, 4, 1, 5, 6, 7 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 5, 0, 1, 2, 3, 7, 6, 4 },
	{ 3, 1, 0, 5, 2, 4, 6, 7 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
};
struct _unif8237 {
	BYTE reg[4];
	WORD prg_map[4];
	WORD chr_map[8];
} unif8237;
struct _unif8237tmp {
	BYTE model;
} unif8237tmp;

void map_init_UNIF8237(BYTE model) {
	EXTCL_CPU_WR_MEM(UNIF8237);
	EXTCL_SAVE_MAPPER(UNIF8237);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&unif8237;
	mapper.internal_struct_size[0] = sizeof(unif8237);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&unif8237, 0x00, sizeof(unif8237));

	unif8237tmp.model = model == DEFAULT ? U8237 : model;

	{
		BYTE i;

		map_prg_rom_8k_reset();
		map_chr_bank_1k_reset();

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				unif8237.prg_map[i] = mapper.rom_map_to[i];
			}
			unif8237.chr_map[i] = i;
		}
	}

	unif8237.reg[1] = 3;

	unif8237_updt_prg_and_chr()

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_UNIF8237(WORD address, BYTE value) {
	if (address >= 0x8000) {
		BYTE base_adr = unif8237_adr[unif8237.reg[2]][((address >> 12) & 0x06) | (address & 0x01)];
		address = (base_adr & 0x01) | ((base_adr & 0x06) << 12) | 0x8000;

		if (base_adr < 4) {
			BYTE old_prg_rom_cfg = mmc3.prg_rom_cfg;
			BYTE old_chr_rom_cfg = mmc3.chr_rom_cfg;

			if (!base_adr) {
				value = (value & 0xC0) | unif8237_reg[unif8237.reg[2]][value & 0x07];
			}

			switch (address & 0xE001) {
				case 0x8000:
					extcl_cpu_wr_mem_MMC3(address, value);
					unif8237_8000()
					unif8237_updt_prg_and_chr()
					return;
				case 0x8001:
					extcl_cpu_wr_mem_MMC3(address, value);
					unif8237_8001()
					unif8237_updt_prg_and_chr()
					return;
				default:
					extcl_cpu_wr_mem_MMC3(address, value);
					return;
			}
		} else {
			extcl_cpu_wr_mem_MMC3(address, value);
		}
	}

	if (address < 0x5000) {
		return;
	}

	switch (address) {
		case 0x5000:
			unif8237.reg[0] = value;
			unif8237_updt_prg()
			break;
		case 0x5001:
			unif8237.reg[1] = value;
			unif8237_updt_prg_and_chr()
			break;
		case 0x5007:
			unif8237.reg[2] = value;
			break;
	}
}
BYTE extcl_save_mapper_UNIF8237(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, unif8237.reg);
	save_slot_ele(mode, slot, unif8237.prg_map);
	save_slot_ele(mode, slot, unif8237.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}

INLINE static void unif8237_update_prg(void) {
	BYTE value;

	if (unif8237.reg[0] & 0x40) {
		BYTE bnk0 = (unif8237.reg[1] & 0x10);

		if (unif8237.reg[0] & 0x80) {
			BYTE bnk1 = ((unif8237.reg[1] & 0x03) << 4) | (unif8237.reg[0] & 0x07) | (bnk0 >> 1);

			if (unif8237.reg[0] & 0x20) {
				value = bnk1 >> 1;
				control_bank(info.prg.rom.max.banks_32k)
				map_prg_rom_8k(4, 0, value);
			} else {
				value = bnk1;
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 0, value);
				map_prg_rom_8k(2, 2, value);
			}
		} else {
			unif8237_prg_8k_a(unif8237.prg_map[0]);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);

			unif8237_prg_8k_a(unif8237.prg_map[1]);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);

			unif8237_prg_8k_a(unif8237.prg_map[2]);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);

			unif8237_prg_8k_a(unif8237.prg_map[3]);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, value);
		}
	} else if (unif8237.reg[0] & 0x80) {
		BYTE bnk = ((unif8237.reg[1] & 0x03) << 4) | (unif8237.reg[0] & 0x0F);

		if (unif8237.reg[0] & 0x20) {
			value = bnk >> 1;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			value = bnk;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}
	} else {
		unif8237_prg_8k_b(unif8237.prg_map[0]);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		unif8237_prg_8k_b(unif8237.prg_map[1]);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		unif8237_prg_8k_b(unif8237.prg_map[2]);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		unif8237_prg_8k_b(unif8237.prg_map[3]);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void unif8237_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		if (unif8237.reg[0] & 0x40) {
			value = ((unif8237.reg[1] & 0x0C) << 6) | (unif8237.chr_map[i] & 0x7F) | ((unif8237.reg[1] & 0x20) << 2);
		} else {
			value = ((unif8237.reg[1] & 0x0C) << 6) | unif8237.chr_map[i];
		}
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[i] = chr_pnt(value << 10);
	}
}
INLINE static void unif8237a_update_prg(void) {
	BYTE value;

	if (unif8237.reg[0] & 0x40) {
		BYTE bnk0 = (unif8237.reg[1] & 0x10);

		if (unif8237.reg[0] & 0x80) {
			BYTE bnk1 = ((unif8237.reg[1] & 0x03) << 4) | ((unif8237.reg[1] & 0x08) << 3) | (unif8237.reg[0] & 0x07) | (bnk0 >> 1);

			if (unif8237.reg[0] & 0x20) {
				value = bnk1 >> 1;
				control_bank(info.prg.rom.max.banks_32k)
				map_prg_rom_8k(4, 0, value);
			} else {
				value = bnk1;
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 0, value);
				map_prg_rom_8k(2, 2, value);
			}
		} else {
			unif8237a_prg_8k_a(unif8237.prg_map[0]);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);

			unif8237a_prg_8k_a(unif8237.prg_map[1]);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);

			unif8237a_prg_8k_a(unif8237.prg_map[2]);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);

			unif8237a_prg_8k_a(unif8237.prg_map[3]);
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, value);
		}
	} else if (unif8237.reg[0] & 0x80) {
		BYTE bnk = ((unif8237.reg[1] & 0x03) << 4) | ((unif8237.reg[1] & 0x08) << 3) | (unif8237.reg[0] & 0x0F);

		if (unif8237.reg[0] & 0x20) {
			value = bnk >> 1;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
		} else {
			value = bnk;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
		}
	} else {
		unif8237a_prg_8k_b(unif8237.prg_map[0]);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 0, value);

		unif8237a_prg_8k_b(unif8237.prg_map[1]);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 1, value);

		unif8237a_prg_8k_b(unif8237.prg_map[2]);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 2, value);

		unif8237a_prg_8k_b(unif8237.prg_map[3]);
		control_bank(info.prg.rom.max.banks_8k)
		map_prg_rom_8k(1, 3, value);
	}
	map_prg_rom_8k_update();
}
INLINE static void unif8237a_update_chr(void) {
	BYTE i;
	WORD value;

	for (i = 0; i < 8; i++) {
		if (unif8237.reg[0] & 0x40) {
			value = ((unif8237.reg[1] & 0x0E) << 7) | (unif8237.chr_map[i] & 0x7F) | ((unif8237.reg[1] & 0x20) << 2);
		} else {
			value = ((unif8237.reg[1] & 0x0E) << 7) | unif8237.chr_map[i];
		}
		control_bank(info.chr.rom.max.banks_1k)
		chr.bank_1k[i] = chr_pnt(value << 10);
	}
}
