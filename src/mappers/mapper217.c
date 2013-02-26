/*
 * mapper217.c
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

#define m217chr1k(vl)\
	if (m217.reg[1] & 0x08) {\
		bank = ((m217.reg[1] << 8) & 0x0300) | vl;\
	} else {\
		bank = ((m217.reg[1] << 8) & 0x0300) | (((m217.reg[1] << 3) & 0x80) | (vl & 0x7F));\
	}
#define m217prg8k(vl)\
	if (m217.reg[1] & 0x08) {\
		value = ((m217.reg[1] << 5) & 0x60) | (vl & 0x1F);\
	} else {\
		value = ((m217.reg[1] << 5) & 0x60) | ((vl & 0x0F) | (m217.reg[1] & 0x10));\
	}
#define m217prg8kupdate()\
{\
	BYTE i;\
	for (i = 0; i < 4; i++) {\
		m217prg8k(m217.prg8kBank[i])\
		control_bank(prgRom8kMax)\
		map_prg_rom_8k(1, i, value);\
	}\
}
#define m217_8000()\
{\
	const BYTE chrRomCfgOld = mmc3.chrRomCfg;\
	const BYTE prgRomCfgOld = mmc3.prgRomCfg;\
	mmc3.bankToUpdate = value & 0x07;\
	mmc3.prgRomCfg = (value & 0x40) >> 5;\
	mmc3.chrRomCfg = (value & 0x80) >> 5;\
	/*\
	 * se il tipo di configurazione della chr cambia,\
	 * devo swappare i primi 4 banchi con i secondi\
	 * quattro.\
	 */\
	if (mmc3.chrRomCfg != chrRomCfgOld) {\
		swapChrBank1k(0, 4)\
		swapChrBank1k(1, 5)\
		swapChrBank1k(2, 6)\
		swapChrBank1k(3, 7)\
	}\
	if (mmc3.prgRomCfg != prgRomCfgOld) {\
		WORD p0 = mapper.rom_map_to[0];\
		WORD p2 = mapper.rom_map_to[2];\
		mapper.rom_map_to[0] = p2;\
		mapper.rom_map_to[2] = p0;\
		/*\
		 * prgRomCfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
		 * prgRomCfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
		 */\
		m217prg8k(prgRom8kBeforeLast)\
		control_bank(prgRom8kMax)\
		map_prg_rom_8k(1, mmc3.prgRomCfg ^ 0x02, value);\
		map_prg_rom_8k_update();\
		m217.prg8kBank[0] = mapper.rom_map_to[0];\
		m217.prg8kBank[1] = mapper.rom_map_to[1];\
		m217.prg8kBank[2] = mapper.rom_map_to[2];\
		m217.prg8kBank[3] = mapper.rom_map_to[3];\
	}\
}
#define m217_8001()\
{\
	WORD bank;\
	switch (mmc3.bankToUpdate) {\
		case 0:\
			m217chr1k(value)\
			bank &= 0xFFE;\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[mmc3.chrRomCfg] = &chr.data[bank << 10];\
			chr.bank_1k[mmc3.chrRomCfg | 0x01] = &chr.data[(bank + 1) << 10];\
			return;\
		case 1:\
			m217chr1k(value)\
			bank &= 0xFFE;\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[mmc3.chrRomCfg | 0x02] = &chr.data[bank << 10];\
			chr.bank_1k[mmc3.chrRomCfg | 0x03] = &chr.data[(bank + 1) << 10];\
			return;\
		case 2:\
			m217chr1k(value)\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[mmc3.chrRomCfg ^ 0x04] = &chr.data[bank << 10];\
			return;\
		case 3:\
			m217chr1k(value)\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[(mmc3.chrRomCfg ^ 0x04) | 0x01] = &chr.data[bank << 10];\
			return;\
		case 4:\
			m217chr1k(value)\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[(mmc3.chrRomCfg ^ 0x04) | 0x02] = &chr.data[bank << 10];\
			return;\
		case 5:\
			m217chr1k(value)\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[(mmc3.chrRomCfg ^ 0x04) | 0x03] = &chr.data[bank << 10];\
			return;\
		case 6:\
			m217prg8k(value)\
			control_bank(prgRom8kMax)\
			map_prg_rom_8k(1, mmc3.prgRomCfg, value);\
			map_prg_rom_8k_update();\
			m217.prg8kBank[mmc3.prgRomCfg] = mapper.rom_map_to[mmc3.prgRomCfg];\
			return;\
		case 7:\
			m217prg8k(value)\
			control_bank(prgRom8kMax)\
			map_prg_rom_8k(1, 1, value);\
			map_prg_rom_8k_update();\
			m217.prg8kBank[1] = mapper.rom_map_to[1];\
			return;\
	}\
}

WORD prgRom16kMax, prgRom8kMax, prgRom8kBeforeLast, chrRom1kMax;

void map_init_217(void) {
	prgRom16kMax = info.prg_rom_16k_count - 1;
	prgRom8kMax = info.prg_rom_8k_count - 1;
	prgRom8kBeforeLast = info.prg_rom_8k_count - 2;
	chrRom1kMax = info.chr_rom_1k_count - 1;

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
	m217.prg8kBank[0] = 0;
	m217.prg8kBank[1] = 1;
	m217.prg8kBank[2] = prgRom8kBeforeLast;
	m217.prg8kBank[3] = prgRom8kMax;

	info.mapper_extend_wr = TRUE;

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
					if (m217.reg[3] && (!(m217.reg[0] & 0x80) || (mmc3.bankToUpdate < 6))) {
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
				m217prg8kupdate()
			} else {
				value = (m217.reg[0] & 0x0F) | ((m217.reg[1] << 4) & 0x30);
				control_bank(prgRom16kMax)
				map_prg_rom_8k(2, 0, value);
				map_prg_rom_8k(2, 2, value);
			}
			map_prg_rom_8k_update();
			return;
		case 0x5001:
			if (m217.reg[1] != value) {
				m217.reg[1] = value;
				m217prg8kupdate()
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
	save_slot_ele(mode, slot, m217.prg8kBank);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
