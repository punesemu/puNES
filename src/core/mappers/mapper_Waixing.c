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

#define waixing_swap_chr_bank_1k(src, dst)\
{\
	BYTE *chr_bank_1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chr_bank_1k;\
	WORD map = waixing.chr_map[src];\
	waixing.chr_map[src] = waixing.chr_map[dst];\
	waixing.chr_map[dst] = map;\
}
#define waixing_8000(function)\
{\
	const BYTE chr_rom_cfg_old = mmc3.chr_rom_cfg;\
	const BYTE prg_rom_cfg_old = mmc3.prg_rom_cfg;\
	function;\
	mmc3.prg_rom_cfg = (value & 0x40) >> 5;\
	mmc3.chr_rom_cfg = (value & 0x80) >> 5;\
	/*\
	 * se il tipo di configurazione della chr cambia,\
	 * devo swappare i primi 4 banchi con i secondi\
	 * quattro.\
	 */\
	if (mmc3.chr_rom_cfg != chr_rom_cfg_old) {\
		waixing_swap_chr_bank_1k(0, 4)\
		waixing_swap_chr_bank_1k(1, 5)\
		waixing_swap_chr_bank_1k(2, 6)\
		waixing_swap_chr_bank_1k(3, 7)\
	}\
	if (mmc3.prg_rom_cfg != prg_rom_cfg_old) {\
		WORD p0 = mapper.rom_map_to[0];\
		WORD p2 = mapper.rom_map_to[2];\
		mapper.rom_map_to[0] = p2;\
		mapper.rom_map_to[2] = p0;\
		/*\
		 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
		 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
		 */\
		map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, info.prg.rom.max.banks_8k_before_last);\
		map_prg_rom_8k_update();\
	}\
}

#define waixing_type_ACDE_chr_1k(a)\
	if ((value >= waixingtmp.min) && (value <= waixingtmp.max)) {\
		chr.bank_1k[a] = &chr.extra.data[(value - waixingtmp.min) << 10];\
	} else {\
		chr.bank_1k[a] = chr_pnt(value << 10);\
	}
#define waixing_type_ACDE_8001()\
{\
	switch (mmc3.bank_to_update) {\
		case 0:\
			waixing.chr_map[mmc3.chr_rom_cfg] = value;\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)\
			waixing_type_ACDE_chr_1k(mmc3.chr_rom_cfg)\
			value++;\
			waixing_type_ACDE_chr_1k(mmc3.chr_rom_cfg | 0x01)\
			return;\
		case 1:\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)\
			waixing_type_ACDE_chr_1k(mmc3.chr_rom_cfg | 0x02)\
			value++;\
			waixing_type_ACDE_chr_1k(mmc3.chr_rom_cfg | 0x03)\
			return;\
		case 2:\
			waixing.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_ACDE_chr_1k(mmc3.chr_rom_cfg ^ 0x04)\
			return;\
		case 3:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_ACDE_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x01)\
			return;\
		case 4:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_ACDE_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x02)\
			return;\
		case 5:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_ACDE_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x03)\
			return;\
	}\
}

#define waixing_type_B_chr_1k(a)\
	if (save & 0x80) {\
		chr.bank_1k[a] = &chr.extra.data[value << 10];\
	} else {\
		chr.bank_1k[a] = chr_pnt(value << 10);\
	}
#define waixing_type_B_8001()\
{\
	switch (mmc3.bank_to_update) {\
		case 0:\
			waixing.chr_map[mmc3.chr_rom_cfg] = save;\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x01] = save + 1;\
			control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)\
			waixing_type_B_chr_1k(mmc3.chr_rom_cfg)\
			value++;\
			waixing_type_B_chr_1k(mmc3.chr_rom_cfg | 0x01)\
			return;\
		case 1:\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x02] = save;\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x03] = save + 1;\
			control_bank_with_AND(0xFE, info.chr.rom.max.banks_1k)\
			waixing_type_B_chr_1k(mmc3.chr_rom_cfg | 0x02)\
			value++;\
			waixing_type_B_chr_1k(mmc3.chr_rom_cfg | 0x03)\
			return;\
		case 2:\
			waixing.chr_map[mmc3.chr_rom_cfg ^ 0x04] = save;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_B_chr_1k(mmc3.chr_rom_cfg ^ 0x04)\
			return;\
		case 3:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = save;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_B_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x01)\
			return;\
		case 4:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = save;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_B_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x02)\
			return;\
		case 5:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = save;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_B_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x03)\
			return;\
	}\
}

#define waixing_type_G_chr_1k(a)\
	if (save < 8) {\
		chr.bank_1k[a] = &chr.extra.data[save << 10];\
	} else {\
		chr.bank_1k[a] = chr_pnt(value << 10);\
	}
#define Waixing_type_G_8001()\
{\
	switch (mmc3.bank_to_update) {\
		case 0:\
			waixing.chr_map[0] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_G_chr_1k(0)\
			return;\
		case 1:\
			waixing.chr_map[2] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_G_chr_1k(2)\
			return;\
		case 2:\
			waixing.chr_map[4] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_G_chr_1k(4)\
			return;\
		case 3:\
			waixing.chr_map[5] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_G_chr_1k(5)\
			return;\
		case 4:\
			waixing.chr_map[6] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_G_chr_1k(6)\
			return;\
		case 5:\
			waixing.chr_map[7] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_G_chr_1k(7)\
			return;\
		case 6:\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, 0, value);\
			map_prg_rom_8k_update();\
			return;\
		case 7:\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, 1, value);\
			map_prg_rom_8k_update();\
			return;\
		case 8:\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, 2, value);\
			map_prg_rom_8k_update();\
			return;\
		case 9:\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, 3, value);\
			map_prg_rom_8k_update();\
			return;\
		case 10:\
			waixing.chr_map[1] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_G_chr_1k(1)\
			return;\
		case 11:\
			waixing.chr_map[3] = value;\
			control_bank(info.chr.rom.max.banks_1k)\
			waixing_type_G_chr_1k(3)\
			return;\
	}\
}

#define waixing_type_H_chr_1k(a)\
	if (mapper.write_vram) {\
		chr.bank_1k[a] = &chr.extra.data[(value & 0x07) << 10];\
	} else {\
		control_bank(info.chr.rom.max.banks_1k)\
		chr.bank_1k[a] = chr_pnt(value << 10);\
	}
#define waixing_type_H_prg_8k(vl)\
	value = (vl & 0x3F) | ((waixing.ctrl[0] & 0x02) << 5)
#define waixing_type_H_prg_8k_update()\
{\
	BYTE i;\
	for (i = 0; i < 4; i++) {\
		waixing_type_H_prg_8k(waixing.prg_map[i]);\
		control_bank(info.prg.rom.max.banks_8k)\
		map_prg_rom_8k(1, i, value);\
	}\
	map_prg_rom_8k_update();\
}
#define waixing_type_H_8000()\
{\
	const BYTE chr_rom_cfg_old = mmc3.chr_rom_cfg;\
	const BYTE prg_rom_cfg_old = mmc3.prg_rom_cfg;\
	mmc3.bank_to_update = value & 0x07;\
	mmc3.prg_rom_cfg = (value & 0x40) >> 5;\
	mmc3.chr_rom_cfg = (value & 0x80) >> 5;\
	/*\
	 * se il tipo di configurazione della chr cambia,\
	 * devo swappare i primi 4 banchi con i secondi\
	 * quattro.\
	 */\
	if (mmc3.chr_rom_cfg != chr_rom_cfg_old) {\
		waixing_swap_chr_bank_1k(0, 4)\
		waixing_swap_chr_bank_1k(1, 5)\
		waixing_swap_chr_bank_1k(2, 6)\
		waixing_swap_chr_bank_1k(3, 7)\
	}\
	if (mmc3.prg_rom_cfg != prg_rom_cfg_old) {\
		WORD p0 = mapper.rom_map_to[0];\
		WORD p2 = mapper.rom_map_to[2];\
		mapper.rom_map_to[0] = p2;\
		mapper.rom_map_to[2] = p0;\
		p0 = waixing.prg_map[0];\
		p2 = waixing.prg_map[2];\
		waixing.prg_map[0] = p2;\
		waixing.prg_map[2] = p0;\
		waixing.prg_map[mmc3.prg_rom_cfg ^ 0x02] = info.prg.rom.max.banks_8k_before_last;\
		/*\
		 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
		 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
		 */\
		waixing_type_H_prg_8k(info.prg.rom.max.banks_8k_before_last);\
		control_bank(info.prg.rom.max.banks_8k)\
		map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, value);\
		map_prg_rom_8k_update();\
	}\
}
#define waixing_type_H_8001()\
{\
	switch (mmc3.bank_to_update) {\
		case 0:\
			value &= 0xFE;\
			waixing.chr_map[mmc3.chr_rom_cfg] = value;\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			waixing_type_H_chr_1k(mmc3.chr_rom_cfg)\
			value++;\
			waixing_type_H_chr_1k(mmc3.chr_rom_cfg | 0x01)\
			if (mapper.write_vram) {\
				waixing.ctrl[0] = save;\
				waixing_type_H_prg_8k_update()\
			}\
			return;\
		case 1:\
			value &= 0xFE;\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			waixing.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			waixing_type_H_chr_1k(mmc3.chr_rom_cfg | 0x02)\
			value++;\
			waixing_type_H_chr_1k(mmc3.chr_rom_cfg | 0x03)\
			waixing_type_H_prg_8k_update()\
			return;\
		case 2:\
			waixing.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			waixing_type_H_chr_1k(mmc3.chr_rom_cfg ^ 0x04)\
			waixing_type_H_prg_8k_update()\
			return;\
		case 3:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			waixing_type_H_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x01)\
			waixing_type_H_prg_8k_update()\
			return;\
		case 4:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			waixing_type_H_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x02)\
			waixing_type_H_prg_8k_update()\
			return;\
		case 5:\
			waixing.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			waixing_type_H_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x03)\
			waixing_type_H_prg_8k_update()\
			return;\
		case 6:\
			waixing.prg_map[mmc3.prg_rom_cfg] = value;\
			waixing_type_H_prg_8k(value);\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);\
			map_prg_rom_8k_update();\
			return;\
		case 7:\
			waixing.prg_map[1] = value;\
			waixing_type_H_prg_8k(value);\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, 1, value);\
			map_prg_rom_8k_update();\
			return;\
	}\
}

#define waixing_SH2_chr_4k(a, v)\
{\
	if (!v) {\
		waixing.ctrl[a >> 2] = 0;\
		chr.bank_1k[a] = &chr.extra.data[0];\
		chr.bank_1k[a | 0x01] = &chr.extra.data[0x0400];\
		chr.bank_1k[a | 0x02] = &chr.extra.data[0x0800];\
		chr.bank_1k[a | 0x03] = &chr.extra.data[0x0C00];\
	} else {\
		DBWORD bank = v << 12;\
		waixing.ctrl[a >> 2] = 1;\
		chr.bank_1k[a] = chr_pnt(bank);\
		chr.bank_1k[a | 0x01] = chr_pnt(bank | 0x0400);\
		chr.bank_1k[a | 0x02] = chr_pnt(bank | 0x0800);\
		chr.bank_1k[a | 0x03] = chr_pnt(bank | 0x0C00);\
	}\
}
#define waixing_SH2_PPUFD()\
	if (waixing.reg == 0xFD) {\
		waixing_SH2_chr_4k(0, waixing.chr_map[0])\
		waixing_SH2_chr_4k(4, waixing.chr_map[2])\
	}
#define waixing_SH2_PPUFE()\
	if (waixing.reg == 0xFE) {\
		waixing_SH2_chr_4k(0, waixing.chr_map[1])\
		waixing_SH2_chr_4k(4, waixing.chr_map[4])\
	}
#define waixing_SH2_PPU(a)\
	if ((a & 0x1FF0) == 0x1FD0) {\
		waixing.reg = 0xFD;\
		waixing_SH2_PPUFD()\
	} else if ((a & 0x1FF0) == 0x1FE0) {\
		waixing.reg = 0xFE;\
		waixing_SH2_PPUFE()\
	}
#define waixing_SH2_8000()\
{\
	const BYTE prg_rom_cfg_old = mmc3.prg_rom_cfg;\
	mmc3.bank_to_update = value & 0x07;\
	mmc3.prg_rom_cfg = (value & 0x40) >> 5;\
	mmc3.chr_rom_cfg = (value & 0x80) >> 5;\
	if (mmc3.prg_rom_cfg != prg_rom_cfg_old) {\
		WORD p0 = mapper.rom_map_to[0];\
		WORD p2 = mapper.rom_map_to[2];\
		mapper.rom_map_to[0] = p2;\
		mapper.rom_map_to[2] = p0;\
		/*\
		 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
		 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
		 */\
		map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, info.prg.rom.max.banks_8k_before_last);\
		map_prg_rom_8k_update();\
	}\
}
#define waixing_SH2_8001()\
{\
	switch (mmc3.bank_to_update) {\
		case 0:\
			waixing.chr_map[0] = value >> 2;\
			_control_bank(waixing.chr_map[0], info.chr.rom.max.banks_4k)\
			waixing_SH2_PPUFD()\
			return;\
		case 1:\
			waixing.chr_map[1] = value >> 2;\
			_control_bank(waixing.chr_map[1], info.chr.rom.max.banks_4k)\
			waixing_SH2_PPUFE()\
			return;\
		case 2:\
			waixing.chr_map[2] = value >> 2;\
			_control_bank(waixing.chr_map[2], info.chr.rom.max.banks_4k)\
			waixing_SH2_PPUFD()\
			return;\
		case 3:\
			return;\
		case 4:\
			waixing.chr_map[4] = value >> 2;\
			_control_bank(waixing.chr_map[4], info.chr.rom.max.banks_4k)\
			waixing_SH2_PPUFE()\
			return;\
		case 5:\
			return;\
	}\
}

struct _waixing {
	WORD prg_map[4];
	WORD chr_map[8];
	BYTE reg;
	WORD ctrl[8];
} waixing;
struct _waixingtmp {
	BYTE min;
	BYTE max;
} waixingtmp;

void map_init_Waixing(BYTE model) {
	switch (model) {
		case WPSX:
			EXTCL_CPU_WR_MEM(Waixing_PSx);

			map_prg_rom_8k(4, 0, 0);
			break;
		case WTB:
			EXTCL_CPU_WR_MEM(Waixing_type_B);
			EXTCL_SAVE_MAPPER(Waixing_type_B);
			EXTCL_WR_CHR(Waixing_type_B);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *)&waixing;
			mapper.internal_struct_size[0] = sizeof(waixing);
			mapper.internal_struct[1] = (BYTE *)&mmc3;
			mapper.internal_struct_size[1] = sizeof(mmc3);

			/* utilizza 0x2000 di CHR RAM extra */
			map_chr_ram_extra_init(0x2000);

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				memset(&irqA12, 0x00, sizeof(irqA12));
				memset(&waixing, 0x00, sizeof(waixing));
				map_chr_ram_extra_reset();

				{
					BYTE i;

					map_chr_bank_1k_reset();

					for (i = 0; i < 8; i++) {
						waixing.chr_map[i] = i;
					}
				}
			}

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case WTA:
		case WTC:
		case WTD:
		case WTE:
			if (model == WTA) {
				waixingtmp.min = 0x08;
				waixingtmp.max = 0x09;
			} else if (model == WTC) {
				waixingtmp.min = 0x08;
				waixingtmp.max = 0x0B;
			} else if (model == WTD) {
				waixingtmp.min = 0x00;
				waixingtmp.max = 0x01;
			} else if (model == WTE) {
				waixingtmp.min = 0x00;
				waixingtmp.max = 0x03;
			}
			EXTCL_CPU_WR_MEM(Waixing_type_ACDE);
			EXTCL_SAVE_MAPPER(Waixing_type_ACDE);
			EXTCL_WR_CHR(Waixing_type_ACDE);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *)&waixing;
			mapper.internal_struct_size[0] = sizeof(waixing);
			mapper.internal_struct[1] = (BYTE *)&mmc3;
			mapper.internal_struct_size[1] = sizeof(mmc3);

			/* utilizza 0x2000 di CHR RAM extra */
			map_chr_ram_extra_init(0x2000);

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				memset(&irqA12, 0x00, sizeof(irqA12));
				memset(&waixing, 0x00, sizeof(waixing));
				map_chr_ram_extra_reset();

				{
					BYTE i;

					map_chr_bank_1k_reset();

					for (i = 0; i < 8; i++) {
						waixing.chr_map[i] = i;

						if ((waixing.chr_map[i] >= waixingtmp.min) && (waixing.chr_map[i] <= waixingtmp.max)) {
							chr.bank_1k[i] = &chr.extra.data[(waixing.chr_map[i] - waixingtmp.min) << 10];
						}
					}
				}
			}

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case WTG:
			EXTCL_CPU_WR_MEM(Waixing_type_G);
			EXTCL_SAVE_MAPPER(Waixing_type_G);
			EXTCL_WR_CHR(Waixing_type_G);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *)&waixing;
			mapper.internal_struct_size[0] = sizeof(waixing);
			mapper.internal_struct[1] = (BYTE *)&mmc3;
			mapper.internal_struct_size[1] = sizeof(mmc3);

			/* utilizza 0x2000 di CHR RAM extra */
			map_chr_ram_extra_init(0x2000);

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				memset(&irqA12, 0x00, sizeof(irqA12));
				memset(&waixing, 0x00, sizeof(waixing));
				map_chr_ram_extra_reset();

				{
					BYTE i;

					map_chr_bank_1k_reset();

					for (i = 0; i < 8; i++) {
						waixing.chr_map[i] = i;

						if (waixing.chr_map[i] < 8) {
							chr.bank_1k[i] = &chr.extra.data[waixing.chr_map[i] << 10];
						}
					}
				}
			}

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case WTH:
			EXTCL_CPU_WR_MEM(Waixing_type_H);
			EXTCL_SAVE_MAPPER(Waixing_type_H);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *)&waixing;
			mapper.internal_struct_size[0] = sizeof(waixing);
			mapper.internal_struct[1] = (BYTE *)&mmc3;
			mapper.internal_struct_size[1] = sizeof(mmc3);

			/* utilizza 0x2000 di CHR RAM extra */
			map_chr_ram_extra_init(0x2000);

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				memset(&irqA12, 0x00, sizeof(irqA12));
				memset(&waixing, 0x00, sizeof(waixing));
				map_chr_ram_extra_reset();

				{
					BYTE i, value;

					map_prg_rom_8k_reset();
					map_chr_bank_1k_reset();

					for (i = 0; i < 8; i++) {
						if (i < 4) {
							waixing.prg_map[i] = mapper.rom_map_to[i];
						}

						waixing.chr_map[i] = i;

						if (mapper.write_vram) {
							chr.bank_1k[i] = &chr.extra.data[waixing.chr_map[i] << 10];
						}
					}

					waixing_type_H_prg_8k_update()
				}
			}

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case SH2:
			EXTCL_CPU_WR_MEM(Waixing_SH2);
			EXTCL_SAVE_MAPPER(Waixing_SH2);
			EXTCL_AFTER_RD_CHR(Waixing_SH2);
			EXTCL_UPDATE_R2006(Waixing_SH2);
			EXTCL_WR_CHR(Waixing_SH2);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			mapper.internal_struct[0] = (BYTE *)&waixing;
			mapper.internal_struct_size[0] = sizeof(waixing);
			mapper.internal_struct[1] = (BYTE *)&mmc3;
			mapper.internal_struct_size[1] = sizeof(mmc3);

			/* utilizza 0x2000 di CHR RAM extra */
			map_chr_ram_extra_init(0x2000);

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				memset(&irqA12, 0x00, sizeof(irqA12));
				memset(&waixing, 0x00, sizeof(waixing));
				map_chr_ram_extra_reset();

				waixing.reg = 0xFD;

				waixing.ctrl[0] = 1;
				waixing.ctrl[1] = 1;

				map_prg_rom_8k_reset();
				map_chr_bank_1k_reset();

				waixing.chr_map[0] = 0;
				waixing.chr_map[1] = 0;
				waixing.chr_map[2] = 0;
				waixing.chr_map[4] = 0;

				waixing_SH2_PPUFD();
			}

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
	}
}

void extcl_cpu_wr_mem_Waixing_PSx(WORD address, BYTE value) {
	BYTE swap = value >> 7;

	if (value & 0x40) {
		mirroring_H();
	} else {
		mirroring_V();
	}

	value &= 0x3F;

	switch (address & 0x1FFF) {
		case 0x0000:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value++;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
		case 0x0001:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, info.prg.rom.max.banks_16k);
			break;
		case 0x0002:
			value = (value << 1) | swap;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k(1, 3, value);
			break;
		case 0x0003:
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k(2, 2, value);
			break;
	}
	map_prg_rom_8k_update();
}

void extcl_cpu_wr_mem_Waixing_type_ACDE(WORD address, BYTE value) {
	BYTE save = value;

	switch (address & 0xE001) {
		case 0x8000:
			waixing_8000(mmc3.bank_to_update = value & 0x07)
			return;
		case 0x8001:
			waixing_type_ACDE_8001()
			break;
		case 0xA000:
			switch (value & 0x03) {
				case 0:
					mirroring_V();
					return;
				case 1:
					mirroring_H();
					return;
				case 2:
					mirroring_SCR0();
					return;
				case 3:
					mirroring_SCR1();
					return;
			}
			return;
		case 0xA001:
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, save);
}
BYTE extcl_save_mapper_Waixing_type_ACDE(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, waixing.chr_map);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE)
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		BYTE i;

		for (i = 0; i < 8; i++) {
			if ((waixing.chr_map[i] >= waixingtmp.min) && (waixing.chr_map[i] <= waixingtmp.max)) {
				chr.bank_1k[i] = &chr.extra.data[(waixing.chr_map[i] - waixingtmp.min) << 10];
			}
		}
	}

	return (EXIT_OK);
}
void extcl_wr_chr_Waixing_type_ACDE(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if ((waixing.chr_map[slot] >= waixingtmp.min) && (waixing.chr_map[slot] <= waixingtmp.max)) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

void extcl_cpu_wr_mem_Waixing_type_B(WORD address, BYTE value) {
	BYTE save = value;

	switch (address & 0xE001) {
		case 0x8000:
			waixing_8000(mmc3.bank_to_update = value & 0x07)
			return;
		case 0x8001:
			waixing_type_B_8001()
			break;
		case 0xA001:
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, save);
}
BYTE extcl_save_mapper_Waixing_type_B(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, waixing.chr_map);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE)
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		BYTE i;

		for (i = 0; i < 8; i++) {
			if (waixing.chr_map[i] & 0x80) {
				BYTE value = waixing.chr_map[i];

				control_bank(info.chr.rom.max.banks_1k)
				chr.bank_1k[i] = &chr.extra.data[value << 10];
			}
		}
	}

	return (EXIT_OK);
}
void extcl_wr_chr_Waixing_type_B(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (waixing.chr_map[slot] & 0x80) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

void extcl_cpu_wr_mem_Waixing_type_G(WORD address, BYTE value) {
	BYTE save = value;

	switch (address & 0xE001) {
		case 0x8000:
			waixing_8000(mmc3.bank_to_update = value & 0x0F)
			return;
		case 0x8001:
			Waixing_type_G_8001()
			return;
		case 0xA000:
			switch (value & 0x03) {
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
			return;
		case 0xA001:
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, save);
}
BYTE extcl_save_mapper_Waixing_type_G(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, waixing.chr_map);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE)
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		BYTE i;

		for (i = 0; i < 8; i++) {
			if (waixing.chr_map[i] < 8) {
				chr.bank_1k[i] = &chr.extra.data[waixing.chr_map[i] << 10];
			}
		}
	}

	return (EXIT_OK);
}
void extcl_wr_chr_Waixing_type_G(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (waixing.chr_map[slot] < 8) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

void extcl_cpu_wr_mem_Waixing_type_H(WORD address, BYTE value) {
	BYTE save = value;

	switch (address & 0xE001) {
		case 0x8000:
			waixing_type_H_8000()
			return;
		case 0x8001:
			waixing_type_H_8001()
			return;
	}
	extcl_cpu_wr_mem_MMC3(address, save);
}
BYTE extcl_save_mapper_Waixing_type_H(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, waixing.prg_map);
	save_slot_ele(mode, slot, waixing.chr_map);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE)
	save_slot_ele(mode, slot, waixing.ctrl);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if ((mode == SAVE_SLOT_READ) && mapper.write_vram) {
		BYTE i;

		for (i = 0; i < 8; i++) {
			chr.bank_1k[i] = &chr.extra.data[waixing.chr_map[i] << 10];
		}
	}

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Waixing_SH2(WORD address, BYTE value) {
	BYTE save = value;

	switch (address & 0xE001) {
		case 0x8000:
			waixing_SH2_8000()
			return;
		case 0x8001:
			waixing_SH2_8001()
			break;
	}
	extcl_cpu_wr_mem_MMC3(address, save);
}
BYTE extcl_save_mapper_Waixing_SH2(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, waixing.chr_map);
	save_slot_mem(mode, slot, chr.extra.data, chr.extra.size, FALSE)
	save_slot_ele(mode, slot, waixing.reg);
	save_slot_ele(mode, slot, waixing.ctrl);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if (mode == SAVE_SLOT_READ) {
		waixing_SH2_PPUFD()
		waixing_SH2_PPUFE()
	}

	return (EXIT_OK);
}
void extcl_after_rd_chr_Waixing_SH2(WORD address) {
	waixing_SH2_PPU(address)
}
void extcl_update_r2006_Waixing_SH2(WORD new_r2006, WORD old_r2006) {
	/* questo e' per l'MMC3 */
	irqA12_IO(new_r2006, old_r2006);

	if (new_r2006 >= 0x2000) {
		return;
	}

	waixing_SH2_PPU(new_r2006)
}
void extcl_wr_chr_Waixing_SH2(WORD address, BYTE value) {
	if (!waixing.ctrl[address >> 12]) {
		chr.bank_1k[address >> 10][address & 0x3FF] = value;
	}
}
