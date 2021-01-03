/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#define rex_dbz_swap_chr_rom_bank_1k(slot1, slot2)\
{\
	WORD tmp = rex_dbz.chr_rom_bank[slot1];\
	rex_dbz.chr_rom_bank[slot1] = rex_dbz.chr_rom_bank[slot2];\
	rex_dbz.chr_rom_bank[slot2] = tmp;\
}
#define rex_dbz_chr_1k_update(slot)\
{\
	WORD tmp = (rex_dbz.chr_high << ((slot >= 4) ? 4 : 8)) & 0x0100;\
	chr.bank_1k[slot] = chr_chip_byte_pnt(0, (tmp | rex_dbz.chr_rom_bank[slot]) << 10);\
}
#define rex_dbz_intercept_8001(slot, val)\
{\
	BYTE bank = slot;\
	rex_dbz.chr_rom_bank[bank] = val;\
	rex_dbz_chr_1k_update(bank)\
}
#define rex_dbz_chr_update()\
{\
	BYTE i;\
	for (i = 0; i < 8 ; i++) {\
		rex_dbz_chr_1k_update(i)\
	}\
}

struct _rex_dbz {
	WORD chr_rom_bank[8];
	BYTE chr_high;
} rex_dbz;

void map_init_Rex(BYTE model) {
	if (model == DBZ) {
		EXTCL_CPU_WR_MEM(Rex_dbz);
		EXTCL_CPU_RD_MEM(Rex_dbz);
		EXTCL_SAVE_MAPPER(Rex_dbz);
		EXTCL_CPU_EVERY_CYCLE(MMC3);
		EXTCL_PPU_000_TO_34X(MMC3);
		EXTCL_PPU_000_TO_255(MMC3);
		EXTCL_PPU_256_TO_319(MMC3);
		EXTCL_PPU_320_TO_34X(MMC3);
		EXTCL_UPDATE_R2006(MMC3);
		mapper.internal_struct[0] = (BYTE *) &rex_dbz;
		mapper.internal_struct_size[0] = sizeof(rex_dbz);
		mapper.internal_struct[1] = (BYTE *) &mmc3;
		mapper.internal_struct_size[1] = sizeof(mmc3);

		if (info.reset >= HARD) {
			BYTE i;

			memset(&rex_dbz, 0x00, sizeof(rex_dbz));
			memset(&mmc3, 0x00, sizeof(mmc3));
			memset(&irqA12, 0x00, sizeof(irqA12));

			for (i = 0; i < 8; i++) {
				rex_dbz.chr_rom_bank[i] = i;
			}
		} else {
			memset(&irqA12, 0x00, sizeof(irqA12));
		}

		info.mapper.extend_wr = TRUE;

		irqA12.present = TRUE;
		irqA12_delay = 1;
	}
}
void extcl_cpu_wr_mem_Rex_dbz(WORD address, BYTE value) {
	WORD adr = address & 0xE001;

	/* intercetto cio' che mi interessa */
	if ((address >= 0x4100) && (address < 0x8000)) {
		if (rex_dbz.chr_high != value) {
			rex_dbz.chr_high = value;
			rex_dbz_chr_update()
		}
		return;
	} else if (adr == 0x8000) {
		BYTE chr_rom_cfg = (value & 0x80) >> 5;

		if (mmc3.chr_rom_cfg != chr_rom_cfg) {
			rex_dbz_swap_chr_rom_bank_1k(0, 4)
			rex_dbz_swap_chr_rom_bank_1k(1, 5)
			rex_dbz_swap_chr_rom_bank_1k(2, 6)
			rex_dbz_swap_chr_rom_bank_1k(3, 7)

			mmc3.chr_rom_cfg = chr_rom_cfg;
		}
	} else if (adr == 0x8001) {
		if (mmc3.bank_to_update <= 5) {
			switch (mmc3.bank_to_update) {
				case 0:
					rex_dbz_intercept_8001(mmc3.chr_rom_cfg, value)
					rex_dbz_intercept_8001(mmc3.chr_rom_cfg | 0x01, value + 1)
					break;
				case 1:
					rex_dbz_intercept_8001(mmc3.chr_rom_cfg | 0x02, value)
					rex_dbz_intercept_8001(mmc3.chr_rom_cfg | 0x03, value + 1)
					break;
				case 2:
					rex_dbz_intercept_8001(mmc3.chr_rom_cfg ^ 0x04, value)
					break;
				case 3:
					rex_dbz_intercept_8001((mmc3.chr_rom_cfg ^ 0x04) | 0x01, value)
					break;
				case 4:
					rex_dbz_intercept_8001((mmc3.chr_rom_cfg ^ 0x04) | 0x02, value)
					break;
				case 5:
					rex_dbz_intercept_8001((mmc3.chr_rom_cfg ^ 0x04) | 0x03, value)
					break;
			}
			return;
		}
	}
	extcl_cpu_wr_mem_MMC3(address, value);
}
BYTE extcl_cpu_rd_mem_Rex_dbz(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if ((address >= 0x4100) && (address < 0x6000)) {
		/* TODO:
		 * se disabilito questo return ed avvio la rom,
		 * il testo non sara' piu' in cinese ma in inglese.
		 */
		return (0x01);
	}

	return (openbus);
}
BYTE extcl_save_mapper_Rex_dbz(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, rex_dbz.chr_rom_bank);
	save_slot_ele(mode, slot, rex_dbz.chr_high);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
