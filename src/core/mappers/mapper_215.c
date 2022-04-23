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

#define m215_chr_1k(vl)\
	if (m215.reg[1] & 0x04) {\
		bank = (vl | 0x100);\
	} else {\
		bank = (vl & 0x7F) | ((m215.reg[1] << 3) & 0x80);\
	}
#define m215_prg_8k(vl)\
	if (m215.reg[1] & 0x08) {\
		value = (vl & 0x1F) | 0x20;\
	} else {\
		value = (vl & 0x0F) | (m215.reg[1] & 0x10);\
	}
#define m215_chr_1k_update()\
{\
	BYTE i;\
	for (i = 0; i < 8; i++) {\
		WORD bank;\
		m215_chr_1k(((chr.bank_1k[i] - chr_rom()) >> 10))\
		_control_bank(bank, info.chr.rom.max.banks_1k)\
		chr.bank_1k[i] = chr_pnt(bank << 10);\
	}\
}
#define m215_prg_8k_update()\
	if (!(m215.reg[0] & 0x80)) {\
		BYTE i;\
		for (i = 0; i < 4; i++) {\
			m215_prg_8k(m215.prg_8k_bank[i]);\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, i, value);\
		}\
	} else {\
		value = (m215.reg[0] & 0x0F) | (m215.reg[1] & 0x10);\
		control_bank(info.prg.rom.max.banks_16k)\
		map_prg_rom_8k(2, 0, value);\
		map_prg_rom_8k(2, 2, value);\
	}\
	map_prg_rom_8k_update()
#define m215_8000()\
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
		swap_chr_bank_1k(0, 4)\
		swap_chr_bank_1k(1, 5)\
		swap_chr_bank_1k(2, 6)\
		swap_chr_bank_1k(3, 7)\
	}\
	if (mmc3.prg_rom_cfg != prg_rom_cfg_old) {\
		if (!(m215.reg[0] & 0x80)) {\
			WORD p0 = mapper.rom_map_to[0];\
			WORD p2 = mapper.rom_map_to[2];\
			mapper.rom_map_to[0] = p2;\
			mapper.rom_map_to[2] = p0;\
			/*\
			 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
			 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
			 */\
			m215_prg_8k(info.prg.rom.max.banks_8k_before_last)\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, value);\
			map_prg_rom_8k_update();\
			m215.prg_8k_bank[0] = mapper.rom_map_to[0];\
			m215.prg_8k_bank[1] = mapper.rom_map_to[1];\
			m215.prg_8k_bank[2] = mapper.rom_map_to[2];\
			m215.prg_8k_bank[3] = mapper.rom_map_to[3];\
		}\
	}\
}
#define m215_8001()\
{\
	WORD bank;\
	switch (mmc3.bank_to_update) {\
		case 0:\
			m215_chr_1k(value)\
			bank &= 0xFFE;\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[mmc3.chr_rom_cfg] = chr_pnt(bank << 10);\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x01] = chr_pnt((bank + 1) << 10);\
			return;\
		case 1:\
			m215_chr_1k(value)\
			bank &= 0xFFE;\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x02] = chr_pnt(bank << 10);\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x03] = chr_pnt((bank + 1) << 10);\
			return;\
		case 2:\
			m215_chr_1k(value)\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[mmc3.chr_rom_cfg ^ 0x04] = chr_pnt(bank << 10);\
			return;\
		case 3:\
			m215_chr_1k(value)\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = chr_pnt(bank << 10);\
			return;\
		case 4:\
			m215_chr_1k(value)\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = chr_pnt(bank << 10);\
			return;\
		case 5:\
			m215_chr_1k(value)\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = chr_pnt(bank << 10);\
			return;\
		case 6:\
			if (!(m215.reg[0] & 0x80)) {\
				m215_prg_8k(value)\
				control_bank(info.prg.rom.max.banks_8k)\
				map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);\
				map_prg_rom_8k_update();\
				m215.prg_8k_bank[mmc3.prg_rom_cfg] = mapper.rom_map_to[mmc3.prg_rom_cfg];\
			}\
			return;\
		case 7:\
			if (!(m215.reg[0] & 0x80)) {\
				m215_prg_8k(value)\
				control_bank(info.prg.rom.max.banks_8k)\
				map_prg_rom_8k(1, 1, value);\
				map_prg_rom_8k_update();\
				m215.prg_8k_bank[1] = mapper.rom_map_to[1];\
			}\
			return;\
	}\
}

struct _m215 {
	BYTE reg[4];
	WORD prg_8k_bank[4];
} m215;

void map_init_215(void) {
	EXTCL_CPU_WR_MEM(215);
	EXTCL_SAVE_MAPPER(215);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&m215;
	mapper.internal_struct_size[0] = sizeof(m215);
	mapper.internal_struct[1] = (BYTE *)&mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	m215.reg[0] = 0x00;
	m215.reg[1] = 0xFF;
	m215.reg[2] = 0x04;
	m215.reg[3] = FALSE;
	m215.prg_8k_bank[0] = 0;
	m215.prg_8k_bank[1] = 1;
	m215.prg_8k_bank[2] = info.prg.rom.max.banks_8k_before_last;
	m215.prg_8k_bank[3] = info.prg.rom.max.banks_8k;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_215(WORD address, BYTE value) {
	if (address > 0x7FFF) {
		switch (address & 0xE001) {
			case 0x8000:
				if (!m215.reg[2]) {
					m215_8000()
				}
				return;
			case 0x8001:
				if (!m215.reg[2]) {
					m215_8001()
				} else {
					if (m215.reg[3] && (!(m215.reg[0] & 0x80) || (mmc3.bank_to_update < 6))) {
						m215.reg[3] = FALSE;
						m215_8001()
					}
				}
				return;
			case 0xA000:
				if (!m215.reg[2]) {
					if (value & 0x01) {
						mirroring_H();
					} else {
						mirroring_V();
					}
				} else {
					static const BYTE security[8] = { 0, 2, 5, 3, 6, 1, 7, 4 };
					BYTE save = value;

					m215.reg[3] = TRUE;
					value = (save & 0xC0) | security[save & 0x07];

					m215_8000()
				}
				return;
			case 0xC000:
				if (!m215.reg[2]) {
					extcl_cpu_wr_mem_MMC3(address, value);
				} else {
					if (((value >> 7) | value) & 0x01) {
						mirroring_H();
					} else {
						mirroring_V();
					}
				}
				return;
			case 0xC001:
				if (!m215.reg[2]) {
					extcl_cpu_wr_mem_MMC3(address, value);
				} else {
					extcl_cpu_wr_mem_MMC3(0xE001, value);
				}
				return;
			case 0xE001:
				if (!m215.reg[2]) {
					extcl_cpu_wr_mem_MMC3(address, value);
				} else {
					extcl_cpu_wr_mem_MMC3(0xC000, value);
					extcl_cpu_wr_mem_MMC3(0xC001, value);
				}
				return;
		}
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	}

	if ((info.id == M215_MK3E) && ((address & 0xF000) == 0x6000)) {
		return;
	}

	switch (address) {
		case 0x5000:
		case 0x6000:
			if (m215.reg[0] != value) {
				m215.reg[0] = value;
				m215_prg_8k_update();
			}
			return;
		case 0x5001:
		case 0x6001:
			if (m215.reg[1] != value) {
				m215.reg[1] = value;
				m215_chr_1k_update()
			}
			return;
		case 0x5007:
		case 0x6007:
			mmc3.bank_to_update = 0;
			mmc3.prg_rom_cfg = 0;
			mmc3.chr_rom_cfg = 0;

			if (m215.reg[2] != value) {
				m215.reg[2] = value;
				m215_prg_8k_update();
				m215_chr_1k_update()
			}
			return;
	}
}
BYTE extcl_save_mapper_215(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m215.reg);
	save_slot_ele(mode, slot, m215.prg_8k_bank);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
