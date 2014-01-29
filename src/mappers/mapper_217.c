/*
 * mapper_217.c
 *
 *  Created on: 23/mar/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

#define m217_chr_1k(vl)\
	if (m217.reg[1] & 0x08) {\
		bank = ((m217.reg[1] << 8) & 0x0300) | vl;\
	} else {\
		bank = ((m217.reg[1] << 8) & 0x0300) | (((m217.reg[1] << 3) & 0x80) | (vl & 0x7F));\
	}
#define m217_prg_8k(vl)\
	if (m217.reg[1] & 0x08) {\
		value = ((m217.reg[1] << 5) & 0x60) | (vl & 0x1F);\
	} else {\
		value = ((m217.reg[1] << 5) & 0x60) | ((vl & 0x0F) | (m217.reg[1] & 0x10));\
	}
#define m217_prg_8k_update()\
{\
	BYTE i;\
	for (i = 0; i < 4; i++) {\
		m217_prg_8k(m217.prg_8k_bank[i])\
		control_bank(info.prg.rom.max.banks_8k)\
		map_prg_rom_8k(1, i, value);\
	}\
}
#define m217_8000()\
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
		WORD p0 = mapper.rom_map_to[0];\
		WORD p2 = mapper.rom_map_to[2];\
		mapper.rom_map_to[0] = p2;\
		mapper.rom_map_to[2] = p0;\
		/*\
		 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
		 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
		 */\
		m217_prg_8k(prg_rom_8k_before_last)\
		control_bank(info.prg.rom.max.banks_8k)\
		map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, value);\
		map_prg_rom_8k_update();\
		m217.prg_8k_bank[0] = mapper.rom_map_to[0];\
		m217.prg_8k_bank[1] = mapper.rom_map_to[1];\
		m217.prg_8k_bank[2] = mapper.rom_map_to[2];\
		m217.prg_8k_bank[3] = mapper.rom_map_to[3];\
	}\
}
#define m217_8001()\
{\
	WORD bank;\
	switch (mmc3.bank_to_update) {\
		case 0:\
			m217_chr_1k(value)\
			bank &= 0xFFE;\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[mmc3.chr_rom_cfg] = &chr.data[bank << 10];\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x01] = &chr.data[(bank + 1) << 10];\
			return;\
		case 1:\
			m217_chr_1k(value)\
			bank &= 0xFFE;\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x02] = &chr.data[bank << 10];\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x03] = &chr.data[(bank + 1) << 10];\
			return;\
		case 2:\
			m217_chr_1k(value)\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[mmc3.chr_rom_cfg ^ 0x04] = &chr.data[bank << 10];\
			return;\
		case 3:\
			m217_chr_1k(value)\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = &chr.data[bank << 10];\
			return;\
		case 4:\
			m217_chr_1k(value)\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = &chr.data[bank << 10];\
			return;\
		case 5:\
			m217_chr_1k(value)\
			_control_bank(bank, info.chr.rom.max.banks_1k)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = &chr.data[bank << 10];\
			return;\
		case 6:\
			m217_prg_8k(value)\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);\
			map_prg_rom_8k_update();\
			m217.prg_8k_bank[mmc3.prg_rom_cfg] = mapper.rom_map_to[mmc3.prg_rom_cfg];\
			return;\
		case 7:\
			m217_prg_8k(value)\
			control_bank(info.prg.rom.max.banks_8k)\
			map_prg_rom_8k(1, 1, value);\
			map_prg_rom_8k_update();\
			m217.prg_8k_bank[1] = mapper.rom_map_to[1];\
			return;\
	}\
}

WORD prg_rom_8k_before_last;

void map_init_217(void) {
	prg_rom_8k_before_last = info.prg.rom.banks_8k - 2;

	EXTCL_CPU_WR_MEM(217);
	EXTCL_SAVE_MAPPER(217);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &m217;
	mapper.internal_struct_size[0] = sizeof(m217);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));
		m217.reg[0] = 0x00;
		m217.reg[1] = 0xFF;
		m217.reg[2] = 0x03;
	}

	m217.reg[3] = FALSE;
	m217.prg_8k_bank[0] = 0;
	m217.prg_8k_bank[1] = 1;
	m217.prg_8k_bank[2] = prg_rom_8k_before_last;
	m217.prg_8k_bank[3] = info.prg.rom.max.banks_8k;

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_217(WORD address, BYTE value) {
	if (address > 0x7FFF) {
		switch (address & 0xE001) {
			case 0x8000:
				if (!m217.reg[2]) {
					m217_8000()
				} else {
					extcl_cpu_wr_mem_MMC3(0xC000, value);
				}
				return;
			case 0x8001: {
				if (!m217.reg[2]) {
					m217_8001()
				} else {
					static const BYTE security[8] = { 0, 6, 3, 7, 5, 2, 4, 1};
					BYTE save = value;

					m217.reg[3] = TRUE;
					value = (save & 0xC0) | security[save & 0x07];

					m217_8000()
				}
				return;
			}
			case 0xA000:
				if (!m217.reg[2]) {
					if (value & 0x01) {
						mirroring_H();
					} else {
						mirroring_V();
					}
				} else {
					if (m217.reg[3] && (!(m217.reg[0] & 0x80) || (mmc3.bank_to_update < 6))) {
						m217.reg[3] = FALSE;
						m217_8001()
					}
				}
				return;
			case 0xA001:
				if (!m217.reg[2]) {
					extcl_cpu_wr_mem_MMC3(address, value);
				} else {
					if (value & 0x01) {
						mirroring_H();
					} else {
						mirroring_V();
					}
				}
				return;
		}
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	}

	switch (address) {
		case 0x5000:
			m217.reg[0] = value;

			if (!(m217.reg[0] & 0x80)) {
				m217_prg_8k_update()
			} else {
				value = (m217.reg[0] & 0x0F) | ((m217.reg[1] << 4) & 0x30);
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 0, value);
				map_prg_rom_8k(2, 2, value);
			}
			map_prg_rom_8k_update();
			return;
		case 0x5001:
			if (m217.reg[1] != value) {
				m217.reg[1] = value;
				m217_prg_8k_update()
				map_prg_rom_8k_update();
			}
			return;
		case 0x5007:
			m217.reg[2] = value;
			return;
	}
}
BYTE extcl_save_mapper_217(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m217.reg);
	save_slot_ele(mode, slot, m217.prg_8k_bank);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
