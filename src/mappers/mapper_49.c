/*
 * mapper_49.c
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

#define m49_chr_1k(vl) bank = ((m49.reg << 1) & 0x180) | (vl & 0x7F)
#define m49_prg_8k(vl) value = ((m49.reg >> 2) & 0x30) | (vl & 0x0F)
#define m49_chr_1k_update()\
{\
	BYTE i;\
	for (i = 0; i < 8; i++) {\
		WORD bank;\
		m49_chr_1k(m49.chr_map[i]);\
		_control_bank(bank, chr_rom_1k_max)\
		chr.bank_1k[i] = &chr.data[bank << 10];\
	}\
}
#define m49_prg_8k_update()\
{\
	BYTE i;\
	for (i = 0; i < 4; i++) {\
		m49_prg_8k(m49.prg_map[i]);\
		control_bank(prg_rom_8k_max)\
		map_prg_rom_8k(1, i, value);\
	}\
	map_prg_rom_8k_update();\
}
#define m49_swap_chr_bank_1k(src, dst)\
{\
	BYTE *chr_bank_1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chr_bank_1k;\
	WORD map = m49.chr_map[src];\
	m49.chr_map[src] = m49.chr_map[dst];\
	m49.chr_map[dst] = map;\
}
#define m49_8000()\
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
		m49_swap_chr_bank_1k(0, 4)\
		m49_swap_chr_bank_1k(1, 5)\
		m49_swap_chr_bank_1k(2, 6)\
		m49_swap_chr_bank_1k(3, 7)\
	}\
	if ((m49.reg & 0x01) && (mmc3.prg_rom_cfg != prg_rom_cfg_old)) {\
		WORD p0 = mapper.rom_map_to[0];\
		WORD p2 = mapper.rom_map_to[2];\
		mapper.rom_map_to[0] = p2;\
		mapper.rom_map_to[2] = p0;\
		p0 = m49.prg_map[0];\
		p2 = m49.prg_map[2];\
		m49.prg_map[0] = p2;\
		m49.prg_map[2] = p0;\
		/*\
		 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
		 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
		 */\
		m49.prg_map[mmc3.prg_rom_cfg ^ 0x02] = prg_rom_8k_before_last;\
		m49_prg_8k(prg_rom_8k_before_last);\
		control_bank(prg_rom_8k_max)\
		map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, value);\
		map_prg_rom_8k_update();\
	}\
}
#define m49_8001()\
{\
	WORD bank;\
	switch (mmc3.bank_to_update) {\
		case 0:\
			m49.chr_map[mmc3.chr_rom_cfg] = value;\
			m49.chr_map[mmc3.chr_rom_cfg | 0x01] = value + 1;\
			m49_chr_1k(value);\
			bank &= 0xFFE;\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[mmc3.chr_rom_cfg] = &chr.data[bank << 10];\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x01] = &chr.data[(bank + 1) << 10];\
			return;\
		case 1:\
			m49.chr_map[mmc3.chr_rom_cfg | 0x02] = value;\
			m49.chr_map[mmc3.chr_rom_cfg | 0x03] = value + 1;\
			m49_chr_1k(value);\
			bank &= 0xFFE;\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x02] = &chr.data[bank << 10];\
			chr.bank_1k[mmc3.chr_rom_cfg | 0x03] = &chr.data[(bank + 1) << 10];\
			return;\
		case 2:\
			m49.chr_map[mmc3.chr_rom_cfg ^ 0x04] = value;\
			m49_chr_1k(value);\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[mmc3.chr_rom_cfg ^ 0x04] = &chr.data[bank << 10];\
			return;\
		case 3:\
			m49.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = value;\
			m49_chr_1k(value);\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x01] = &chr.data[bank << 10];\
			return;\
		case 4:\
			m49.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = value;\
			m49_chr_1k(value);\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x02] = &chr.data[bank << 10];\
			return;\
		case 5:\
			m49.chr_map[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = value;\
			m49_chr_1k(value);\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[(mmc3.chr_rom_cfg ^ 0x04) | 0x03] = &chr.data[bank << 10];\
			return;\
		case 6:\
			if (m49.reg & 0x01) {\
				m49.prg_map[mmc3.prg_rom_cfg] = value;\
				m49_prg_8k(value);\
				control_bank(prg_rom_8k_max)\
				map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);\
				map_prg_rom_8k_update();\
			}\
			return;\
		case 7:\
			if (m49.reg & 0x01) {\
				m49.prg_map[1] = value;\
				m49_prg_8k(value);\
				control_bank(prg_rom_8k_max)\
				map_prg_rom_8k(1, 1, value);\
				map_prg_rom_8k_update();\
			}\
			return;\
	}\
}

WORD prg_rom_32k_max, prg_rom_8k_max, prg_rom_8k_before_last, chr_rom_1k_max;

void map_init_49(void) {
	prg_rom_32k_max = (info.prg.rom.banks_16k >> 1) - 1;
	prg_rom_8k_max = info.prg.rom.banks_8k - 1;
	prg_rom_8k_before_last = info.prg.rom.banks_8k - 2;
	chr_rom_1k_max = info.chr.rom.banks_1k - 1;

	EXTCL_CPU_WR_MEM(49);
	EXTCL_SAVE_MAPPER(49);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &m49;
	mapper.internal_struct_size[0] = sizeof(m49);
	mapper.internal_struct[1] = (BYTE *) &mmc3;
	mapper.internal_struct_size[1] = sizeof(mmc3);

	memset(&mmc3, 0x00, sizeof(mmc3));
	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&m49, 0x00, sizeof(m49));

	{
		BYTE value, i;

		map_prg_rom_8k_reset();
		chr_bank_1k_reset()

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m49.prg_map[i] = mapper.rom_map_to[i];
			}
			m49.chr_map[i] = i;
		}

		m49_prg_8k_update()
		m49_chr_1k_update()
	}

	info.mapper.extend_wr = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_49(WORD address, BYTE value) {
	if (address > 0x7FFF) {
		switch (address & 0xE001) {
			case 0x8000:
				m49_8000()
				return;
			case 0x8001:
				m49_8001()
				return;
		}
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	}

	if (cpu.prg_ram_wr_active && (address >= 0x6000)) {
		if (m49.reg != value) {
			m49.reg = value;

			if (m49.reg & 0x01) {
				m49_prg_8k_update()
			} else {
				value = (m49.reg >> 4) & 0x03;
				control_bank(prg_rom_32k_max)
				map_prg_rom_8k(4, 0, value);
				map_prg_rom_8k_update();
			}
			m49_chr_1k_update()
		}
	}
}
BYTE extcl_save_mapper_49(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m49.reg);
	save_slot_ele(mode, slot, m49.prg_map);
	save_slot_ele(mode, slot, m49.chr_map);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
