/*
 * mapper_116.c
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

WORD prg_rom_32k_max, prg_rom_16k_max, prg_rom_8k_max, prg_rom_8k_before_last;
WORD chr_rom_1k_max, chr_rom_2k_max, chr_rom_4k_max;

#define m116_update_prg_8k(bnk, vl)\
	tmp = vl;\
	_control_bank(tmp, prg_rom_8k_max)\
	map_prg_rom_8k(1, bnk, tmp)
#define m116_update_prg_16k(bnk, vl)\
	tmp = vl;\
	_control_bank(tmp, prg_rom_16k_max)\
	map_prg_rom_8k(2, bnk, tmp)
#define m116_update_prg_32k(vl)\
	tmp = vl;\
	_control_bank(tmp, prg_rom_32k_max)\
	map_prg_rom_8k(4, 0, tmp)

#define m116_update_chr_1k(bnk, vl)\
	bank = vl;\
	_control_bank(bank, chr_rom_1k_max)\
	chr.bank_1k[bnk] = &chr.data[bank << 10]
#define m116_update_chr_2k(bnk, vl)\
	bank = vl;\
	_control_bank(bank, chr_rom_2k_max)\
	bank <<= 11;\
	chr.bank_1k[bnk       ] = &chr.data[bank         ];\
	chr.bank_1k[bnk | 0x01] = &chr.data[bank | 0x0400]
#define m116_update_chr_4k(bnk, vl)\
	bank = vl;\
	_control_bank(bank, chr_rom_4k_max)\
	bank <<= 12;\
	chr.bank_1k[bnk       ] = &chr.data[bank         ];\
	chr.bank_1k[bnk | 0x01] = &chr.data[bank | 0x0400];\
	chr.bank_1k[bnk | 0x02] = &chr.data[bank | 0x0800];\
	chr.bank_1k[bnk | 0x03] = &chr.data[bank | 0x0C00]

#define m116_A_update_prg_mode0()\
{\
	BYTE tmp;\
	m116_update_prg_8k(0, m116.mode0.prg[0]);\
	m116_update_prg_8k(1, m116.mode0.prg[1]);\
	m116_update_prg_8k(2, 0x1E);\
	m116_update_prg_8k(3, 0x1F);\
	map_prg_rom_8k_update();\
}
#define m116_A_update_prg_mode1()\
{\
	BYTE tmp, i = (m116.mode1.ctrl >> 5) & 0x02;\
	m116_update_prg_8k(0, m116.mode1.banks[6 + i]);\
	m116_update_prg_8k(1, m116.mode1.banks[6 + 1]);\
	m116_update_prg_8k(2, m116.mode1.banks[6 + (i ^ 2)]);\
	m116_update_prg_8k(3, m116.mode1.banks[6 + 3]);\
	map_prg_rom_8k_update();\
}
#define m116_A_update_prg_mode2()\
{\
	BYTE tmp, bank = m116.mode2.reg[3] & 0x0F;\
	if (m116.mode2.reg[0] & 0x08) {\
		m116_update_prg_16k(0, (m116.mode2.reg[0] & 0x04) ? bank : 0);\
		m116_update_prg_16k(2, (m116.mode2.reg[0] & 0x04) ? 0x0F : bank);\
	} else {\
		m116_update_prg_32k(bank >> 1);\
	}\
	map_prg_rom_8k_update();\
}
#define m116_A_update_prg()\
	switch(m116.mode & 0x03) {\
		case 0:\
			m116_A_update_prg_mode0()\
			break;\
		case 1:\
			m116_A_update_prg_mode1()\
			break;\
		case 2:\
			m116_A_update_prg_mode2()\
			break;\
	}
#define m116_A_update_chr_mode0()\
{\
	DBWORD bank;\
	WORD base = (m116.mode & 0x04) << 6;\
	m116_update_chr_1k(0, base | m116.mode0.chr[0]);\
	m116_update_chr_1k(1, base | m116.mode0.chr[1]);\
	m116_update_chr_1k(2, base | m116.mode0.chr[2]);\
	m116_update_chr_1k(3, base | m116.mode0.chr[3]);\
	m116_update_chr_1k(4, base | m116.mode0.chr[4]);\
	m116_update_chr_1k(5, base | m116.mode0.chr[5]);\
	m116_update_chr_1k(6, base | m116.mode0.chr[6]);\
	m116_update_chr_1k(7, base | m116.mode0.chr[7]);\
}
#define m116_A_update_chr_mode1()\
{\
	DBWORD bank;\
	WORD base = (m116.mode & 0x04) << 6;\
	BYTE swap = (m116.mode1.ctrl & 0x80) >> 5;\
	m116_update_chr_2k((swap | 0x00), (base >> 1) | m116.mode1.banks[0]);\
	m116_update_chr_2k((swap | 0x02), (base >> 1) | m116.mode1.banks[1]);\
	swap ^= 0x04;\
	m116_update_chr_1k((swap | 0x00), base | m116.mode1.banks[2]);\
	m116_update_chr_1k((swap | 0x01), base | m116.mode1.banks[3]);\
	m116_update_chr_1k((swap | 0x02), base | m116.mode1.banks[4]);\
	m116_update_chr_1k((swap | 0x03), base | m116.mode1.banks[5]);\
}
#define m116_A_update_chr_mode2()\
{\
	DBWORD bank;\
	m116_update_chr_4k(0, (m116.mode2.reg[0] & 0x10) ? m116.mode2.reg[1] :\
			m116.mode2.reg[1] & 0x1E);\
	m116_update_chr_4k(4, (m116.mode2.reg[0] & 0x10) ? m116.mode2.reg[2] :\
			m116.mode2.reg[1] | 0x01);\
}
#define m116_A_update_chr()\
	switch(m116.mode & 0x03) {\
		case 0:\
			m116_A_update_chr_mode0()\
			break;\
		case 1:\
			m116_A_update_chr_mode1()\
			break;\
		case 2:\
			m116_A_update_chr_mode2()\
			break;\
	}
#define m116_A_update_mirroring_mode0()\
	if (m116.mode0.nmt & 0x01) {\
		mirroring_H();\
	} else {\
		mirroring_V();\
	}
#define m116_A_update_mirroring_mode1()\
	if (m116.mode1.nmt & 0x01) {\
		mirroring_H();\
	} else {\
		mirroring_V();\
	}
#define m116_A_update_mirroring_mode2()\
	switch (m116.mode2.reg[0] & 0x03) {\
		case 0:\
			mirroring_SCR0();\
			break;\
		case 1:\
			mirroring_SCR1();\
			break;\
		case 2:\
			mirroring_V();\
			break;\
		default:\
			mirroring_H();\
			break;\
	}
#define m116_A_update_mirroring()\
	switch(m116.mode & 0x03) {\
		case 0:\
			m116_A_update_mirroring_mode0()\
			break;\
		case 1:\
			m116_A_update_mirroring_mode1()\
			break;\
		case 2:\
			m116_A_update_mirroring_mode2()\
			break;\
		default:\
			break;\
	}

#define m116_B_chr_1k(a)\
	m116.chr_map[a] = value;\
	if (!(m116.mode & 0x02)) {\
		chr.bank_1k[a] = &chr.data[value << 10];\
	}
#define m116_B_swap_chr_bank_1k(src, dst)\
{\
	BYTE *chr_bank_1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chr_bank_1k;\
	WORD map = m116.chr_map[src];\
	m116.chr_map[src] = m116.chr_map[dst];\
	m116.chr_map[dst] = map;\
}
#define m116_B_8000()\
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
	if ((mmc3.chr_rom_cfg != chr_rom_cfg_old) && !(m116.mode & 0x02)) {\
		m116_B_swap_chr_bank_1k(0, 4)\
		m116_B_swap_chr_bank_1k(1, 5)\
		m116_B_swap_chr_bank_1k(2, 6)\
		m116_B_swap_chr_bank_1k(3, 7)\
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
		map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, prg_rom_8k_before_last);\
		map_prg_rom_8k_update();\
	}\
}
#define m116_B_8001()\
{\
	switch (mmc3.bank_to_update) {\
		case 0:\
			control_bank_with_AND(0xFE, chr_rom_1k_max)\
			m116_B_chr_1k(mmc3.chr_rom_cfg)\
			value++;\
			m116_B_chr_1k(mmc3.chr_rom_cfg | 0x01)\
			return;\
		case 1:\
			control_bank_with_AND(0xFE, chr_rom_1k_max)\
			m116_B_chr_1k(mmc3.chr_rom_cfg | 0x02)\
			value++;\
			m116_B_chr_1k(mmc3.chr_rom_cfg | 0x03)\
			return;\
		case 2:\
			control_bank(chr_rom_1k_max)\
			m116_B_chr_1k(mmc3.chr_rom_cfg ^ 0x04)\
			return;\
		case 3:\
			control_bank(chr_rom_1k_max)\
			m116_B_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x01)\
			return;\
		case 4:\
			control_bank(chr_rom_1k_max)\
			m116_B_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x02)\
			return;\
		case 5:\
			control_bank(chr_rom_1k_max)\
			m116_B_chr_1k((mmc3.chr_rom_cfg ^ 0x04) | 0x03)\
			return;\
	}\
}

#define m116_C_chr_1k(a, vl)\
{\
	static const BYTE modes[4] = { 5, 5, 3, 1 };\
	bank = vl | ((m116.mode << modes[(a >> 1) ^ ((mmc3.chr_rom_cfg >> 1) & 0x02)]) & 0x100);\
}
#define m116_C_swap_chr_bank_1k(src, dst)\
{\
	BYTE *chr_bank_1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chr_bank_1k;\
	WORD map = m116.chr_map[src];\
	m116.chr_map[src] = m116.chr_map[dst];\
	m116.chr_map[dst] = map;\
}
#define m116_C_8000()\
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
		m116_C_swap_chr_bank_1k(0, 4)\
		m116_C_swap_chr_bank_1k(1, 5)\
		m116_C_swap_chr_bank_1k(2, 6)\
		m116_C_swap_chr_bank_1k(3, 7)\
	}\
	if (mmc3.prg_rom_cfg != prg_rom_cfg_old) {\
		WORD p0 = mapper.rom_map_to[0];\
		WORD p2 = mapper.rom_map_to[2];\
		mapper.rom_map_to[0] = p2;\
		mapper.rom_map_to[2] = p0;\
		p0 = m116.prg_map[0];\
		p2 = m116.prg_map[2];\
		m116.prg_map[0] = p2;\
		m116.prg_map[2] = p0;\
		m116.prg_map[mmc3.prg_rom_cfg ^ 0x02] = prg_rom_8k_before_last;\
		/*\
		 * prg_rom_cfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
		 * prg_rom_cfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
		 */\
		map_prg_rom_8k(1, mmc3.prg_rom_cfg ^ 0x02, prg_rom_8k_before_last);\
		map_prg_rom_8k_update();\
	}\
}
#define m116_C_8001()\
{\
	WORD bank;\
	BYTE slot;\
	switch (mmc3.bank_to_update) {\
		case 0:\
			slot = mmc3.chr_rom_cfg;\
			m116.chr_map[slot] = value;\
			m116.chr_map[slot + 1] = value + 1;\
			m116_C_chr_1k(slot, value);\
			bank &= 0xFFE;\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			chr.bank_1k[slot + 1] = &chr.data[(bank + 1) << 10];\
			return;\
		case 1:\
			slot = mmc3.chr_rom_cfg | 0x02;\
			m116.chr_map[slot] = value;\
			m116.chr_map[slot + 1] = value + 1;\
			m116_C_chr_1k(slot, value);\
			bank &= 0xFFE;\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			chr.bank_1k[slot + 1] = &chr.data[(bank + 1) << 10];\
			return;\
		case 2:\
			slot = mmc3.chr_rom_cfg ^ 0x04;\
			m116.chr_map[slot] = value;\
			m116_C_chr_1k(slot, value);\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			return;\
		case 3:\
			slot = (mmc3.chr_rom_cfg ^ 0x04) | 0x01;\
			m116.chr_map[slot] = value;\
			m116_C_chr_1k(slot, value);\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			return;\
		case 4:\
			slot = (mmc3.chr_rom_cfg ^ 0x04) | 0x02;\
			m116.chr_map[slot] = value;\
			m116_C_chr_1k(slot, value);\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			return;\
		case 5:\
			slot = (mmc3.chr_rom_cfg ^ 0x04) | 0x03;\
			m116.chr_map[slot] = value;\
			m116_C_chr_1k(slot, value);\
			_control_bank(bank, chr_rom_1k_max)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			return;\
		case 6:\
			control_bank(prg_rom_8k_max)\
			m116.prg_map[mmc3.prg_rom_cfg] = value;\
			map_prg_rom_8k(1, mmc3.prg_rom_cfg, value);\
			map_prg_rom_8k_update();\
			return;\
		case 7:\
			control_bank(prg_rom_8k_max)\
			m116.prg_map[1] = value;\
			map_prg_rom_8k(1, 1, value);\
			map_prg_rom_8k_update();\
			return;\
	}\
}

void map_init_116(void) {
	prg_rom_32k_max = (info.prg_rom_16k_count >> 1) - 1;
	prg_rom_16k_max = info.prg_rom_16k_count - 1;
	prg_rom_8k_before_last = info.prg_rom_8k_count - 2;
	prg_rom_8k_max = info.prg_rom_8k_count - 1;
	chr_rom_4k_max = info.chr_rom_4k_count - 1;
	chr_rom_2k_max = (info.chr_rom_1k_count >> 1) - 1;
	chr_rom_1k_max = info.chr_rom_1k_count - 1;

	switch (info.mapper_type) {
		default:
		case M116_TYPE_A:
			EXTCL_CPU_WR_MEM(116_type_A);
			EXTCL_SAVE_MAPPER(116_type_A);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *) &m116;
			mapper.internal_struct_size[0] = sizeof(m116);

			if (info.reset >= HARD) {
				BYTE i;

				memset(&irqA12, 0x00, sizeof(irqA12));

				m116.mode = 0;

				m116.mode0.prg[0] = 0x00;
				m116.mode0.prg[1] = 0x01;
				m116.mode0.nmt = 0;
				for (i = 0; i < 8; ++i) {
					m116.mode0.chr[i] = i;
				}

				m116.mode1.ctrl = 0;
				m116.mode1.nmt = 0;
				m116.mode1.banks[0] = 0x00;
				m116.mode1.banks[1] = 0x01;
				m116.mode1.banks[2] = 0x04;
				m116.mode1.banks[3] = 0x05;
				m116.mode1.banks[4] = 0x06;
				m116.mode1.banks[5] = 0x07;
				m116.mode1.banks[6] = 0x3C;
				m116.mode1.banks[7] = 0x3D;
				m116.mode1.banks[8] = 0xFE;
				m116.mode1.banks[9] = 0xFF;

				m116.mode2.buffer = 0;
				m116.mode2.shifter = 0;
				m116.mode2.reg[0] = 0x04 | 0x08;
				m116.mode2.reg[1] = 0;
				m116.mode2.reg[2] = 0;
				m116.mode2.reg[3] = 0;
			}

			m116_A_update_prg()
			m116_A_update_chr()
			m116_A_update_mirroring()

			info.mapper_extend_wr = TRUE;

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case M116_TYPE_B:
			EXTCL_CPU_WR_MEM(116_type_B);
			EXTCL_SAVE_MAPPER(116_type_B);
			EXTCL_WR_CHR(116_type_B);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *) &m116;
			mapper.internal_struct_size[0] = sizeof(m116);
			mapper.internal_struct[1] = (BYTE *) &mmc3;
			mapper.internal_struct_size[1] = sizeof(mmc3);

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				memset(&irqA12, 0x00, sizeof(irqA12));
				memset(&m116, 0x00, sizeof(m116));

				{
					BYTE i;

					chr_bank_1k_reset()

					for (i = 0; i < 8; i++) {
						m116.chr_map[i] = i;
					}
				}
			}

			info.mapper_extend_wr = TRUE;

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case M116_TYPE_C:
			EXTCL_CPU_WR_MEM(116_type_C);
			EXTCL_SAVE_MAPPER(116_type_C);
			EXTCL_CPU_EVERY_CYCLE(MMC3);
			EXTCL_PPU_000_TO_34X(MMC3);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(MMC3);
			mapper.internal_struct[0] = (BYTE *) &m116;
			mapper.internal_struct_size[0] = sizeof(m116);
			mapper.internal_struct[1] = (BYTE *) &mmc3;
			mapper.internal_struct_size[1] = sizeof(mmc3);

			if (info.reset >= HARD) {
				memset(&mmc3, 0x00, sizeof(mmc3));
				memset(&irqA12, 0x00, sizeof(irqA12));
				memset(&m116, 0x00, sizeof(m116));

				{
					BYTE i;

					map_prg_rom_8k_reset();
					chr_bank_1k_reset()

					for (i = 0; i < 4; i++) {
						m116.prg_map[i] = mapper.rom_map_to[i];
						if (i < 2) {
							m116.mode0.prg[i] = 0;
						} else {
							m116.mode0.prg[i] = mapper.rom_map_to[i];
						}
					}

					for (i = 0; i < 8; i++) {
						m116.chr_map[i] = i;
						m116.mode0.chr[i] = 0;
					}
				}
			}

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;

	}
}

void extcl_cpu_wr_mem_116_type_A(WORD address, BYTE value) {
	BYTE reg, mode;

	if (address < 0x4100) {
		return;
	}

	if ((address < 0x6000) && (address & 0x0100)) {
		if (m116.mode != value) {
			m116.mode = value;
			if ((value & 0x01) != 1) {
				irqA12.enable = FALSE;
				irq.high &= ~EXT_IRQ;
			}
			m116_A_update_prg()
			m116_A_update_chr()
			m116_A_update_mirroring()
		}
		return;
	}

	if (address < 0x8000) {
		return;
	}

	reg = (address & 0xF000) >> 12;
	mode = m116.mode & 0x03;

	/* VCR2 mode */
	if (mode == 0) {
		switch (reg) {
			case 0x08:
			case 0x0A:
				value &= 0x1F;
				reg = (address >> 13) & 0x01;
				if (m116.mode0.prg[reg] != value) {
					m116.mode0.prg[reg] = value;
					m116_A_update_prg_mode0()
				}
				return;
			case 0x09:
				value &= 0x01;
				if (m116.mode0.nmt != value) {
					m116.mode0.nmt = value;
					m116_A_update_mirroring_mode0()
				}
				return;
			case 0x0B:
			case 0x0C:
			case 0x0D:
			case 0x0E:
				value = (value & 0x0F) << ((address << 1) & 0x04);
				reg = (((address - 0xB000) >> 11) & 0x06) | (address & 0x01);
				if (m116.mode0.chr[reg] != value) {
					m116.mode0.chr[reg] = value;
					m116_A_update_chr_mode0()
				}
				return;
			case 0x0F:
				return;
		}
	}
	/* MMC3 mode */
	if (mode == 1) {
		switch (reg) {
			case 0x08:
			case 0x09:
				if (address & 0x0001) {
					reg = m116.mode1.ctrl & 0x07;
					if (reg < 2) {
						value >>= 1;
					}
					if (m116.mode1.banks[reg] != value) {
						m116.mode1.banks[reg] = value;
						if (reg < 6) {
							m116_A_update_chr_mode1()
						} else {
							m116_A_update_prg_mode1()
						}
					}
				} else {
					reg = m116.mode1.ctrl ^ value;
					m116.mode1.ctrl = value;
					if (reg & 0x40) {
						m116_A_update_prg_mode1()
					}
					if (reg & (0x80 | 0x07)) {
						m116_A_update_chr_mode1()
					}
				}
				return;
			case 0x0A:
			case 0x0B:
				if (!(address & 0x0001)) {
					if (m116.mode1.nmt != value) {
						m116.mode1.nmt = value;
						m116_A_update_mirroring_mode1()
					}
				}
				return;
			case 0x0C:
			case 0x0D:
				if (address & 0x0001) {
					irqA12.reload = TRUE;
					irqA12.counter = 0;
				} else {
					irqA12.latch = value;
				}
				return;
			case 0x0E:
			case 0x0F:
				if (address & 0x0001) {
					irqA12.enable = TRUE;
				} else {
					irqA12.enable = FALSE;
					irq.high &= ~EXT_IRQ;
				}
				return;
		}
	}
	/* MMC1 mode */
	if (mode == 2) {
		switch (reg) {
			case 0x08:
			case 0x09:
			case 0x0A:
			case 0x0B:
			case 0x0C:
			case 0x0D:
			case 0x0E:
			case 0x0F:
				if (!(value & 0x80)) {
					m116.mode2.buffer |= (value & 0x01) << m116.mode2.shifter++;
					if (m116.mode2.shifter != 5) {
						return;
					}
					m116.mode2.shifter = 0;
					value = m116.mode2.buffer;
					m116.mode2.buffer = 0;

					reg = (address >> 13) & 0x03;

					if (m116.mode2.reg[reg] != value) {
						m116.mode2.reg[reg] = value;

						m116_A_update_prg_mode2();
						m116_A_update_chr_mode2();
						m116_A_update_mirroring_mode2();
					}
				} else {
					m116.mode2.buffer = 0;
					m116.mode2.shifter = 0;

					if ((m116.mode2.reg[0] & (0x04 | 0x08)) != (0x04 | 0x08)) {
						m116.mode2.reg[0] |= (0x04 | 0x08);

						m116_A_update_prg_mode2();
						m116_A_update_chr_mode2();
						m116_A_update_mirroring_mode2();
					}
				}
				return;
		}
	}
}
BYTE extcl_save_mapper_116_type_A(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m116.mode);

	save_slot_ele(mode, slot, m116.mode0.chr);
	save_slot_ele(mode, slot, m116.mode0.prg);
	save_slot_ele(mode, slot, m116.mode0.nmt);
	save_slot_ele(mode, slot, m116.mode0.padding);

	save_slot_ele(mode, slot, m116.mode1.banks);
	save_slot_ele(mode, slot, m116.mode1.ctrl);
	save_slot_ele(mode, slot, m116.mode1.nmt);

	save_slot_ele(mode, slot, m116.mode2.reg);
	save_slot_ele(mode, slot, m116.mode2.buffer);
	save_slot_ele(mode, slot, m116.mode2.shifter);
	save_slot_ele(mode, slot, m116.mode2.padding);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_116_type_B(WORD address, BYTE value) {
	BYTE save = value;

	if (address == 0x4100) {
		if (m116.mode != value) {
			m116.mode = value;
			if (value & 0x02) {
				chr.bank_1k[0] = &m116.chr_ram[0 << 10];
				chr.bank_1k[1] = &m116.chr_ram[1 << 10];
				chr.bank_1k[2] = &m116.chr_ram[2 << 10];
				chr.bank_1k[3] = &m116.chr_ram[3 << 10];
				chr.bank_1k[4] = &m116.chr_ram[4 << 10];
				chr.bank_1k[5] = &m116.chr_ram[5 << 10];
				chr.bank_1k[6] = &m116.chr_ram[6 << 10];
				chr.bank_1k[7] = &m116.chr_ram[7 << 10];
			} else {
				chr.bank_1k[0] = &chr.data[m116.chr_map[0] << 10];
				chr.bank_1k[1] = &chr.data[m116.chr_map[1] << 10];
				chr.bank_1k[2] = &chr.data[m116.chr_map[2] << 10];
				chr.bank_1k[3] = &chr.data[m116.chr_map[3] << 10];
				chr.bank_1k[4] = &chr.data[m116.chr_map[4] << 10];
				chr.bank_1k[5] = &chr.data[m116.chr_map[5] << 10];
				chr.bank_1k[6] = &chr.data[m116.chr_map[6] << 10];
				chr.bank_1k[7] = &chr.data[m116.chr_map[7] << 10];
			}
		}
		return;
	}

	if (address < 0x8000) {
		return;
	}

	switch (address & 0xE001) {
		case 0x8000:
			m116_B_8000()
			return;
		case 0x8001:
			m116_B_8001()
			break;
	}
	extcl_cpu_wr_mem_MMC3(address, save);
}
BYTE extcl_save_mapper_116_type_B(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m116.mode);
	save_slot_ele(mode, slot, m116.chr_map);
	save_slot_ele(mode, slot, m116.chr_ram);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if ((mode == SAVE_SLOT_READ) && (m116.mode & 0x02)) {
		BYTE i;

		for (i = 0; i < 8; i++) {
			chr.bank_1k[i] = &m116.chr_ram[i << 10];
		}
	}

	return (EXIT_OK);
}
void extcl_wr_chr_116_type_B(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (m116.mode & 0x02) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

void extcl_cpu_wr_mem_116_type_C(WORD address, BYTE value) {
	if (((address & 0xA131) == 0xA131) && (m116.mode != value)) {
		BYTE i;

		m116.mode = value;

		for (i = 0; i < 4; i++) {
			if (m116.mode & 0x02) {
				map_prg_rom_8k(1, i, m116.prg_map[i]);
			} else {
				map_prg_rom_8k(1, i, m116.mode0.prg[i]);
			}
		}
		map_prg_rom_8k_update();

		for (i = 0; i < 8; i++) {
			if (m116.mode & 0x02) {
				WORD bank;
				BYTE value = m116.chr_map[i];

				m116_C_chr_1k(i, value);
				chr.bank_1k[i] = &chr.data[bank << 10];
			} else {
				chr.bank_1k[i] = &chr.data[m116.mode0.chr[i] << 10];
			}
		}

		if (!(m116.mode & 0x02)) {
			if (m116.mode0.nmt & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
		}
	}

	if (m116.mode & 0x02) {
		switch (address & 0xE001) {
			case 0x8000:
				m116_C_8000()
				return;
			case 0x8001:
				m116_C_8001()
				return;
			case 0xA000:
				if (m116.mode0.nmt & 0x01) {
					mirroring_V();
				} else {
					mirroring_H();
				}
				return;
		}
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	} else if ((address >= 0xB000) && (address <= 0xE003)) {
		const BYTE offset = (address << 2) & 0x04;

		address = ((((address & 0x02) | (address >> 10)) >> 1) + 2) & 0x07;

		m116.mode0.chr[address] = (m116.mode0.chr[address] & (0xF0 >> offset))
		        | ((value & 0x0F) << offset);

		_control_bank(m116.mode0.chr[address], chr_rom_1k_max)
		chr.bank_1k[address] = &chr.data[m116.mode0.chr[address] << 10];
		return;
	} else {
		switch (address & 0xF003) {
			case 0x8000:
				control_bank(prg_rom_8k_max)
				if (m116.mode0.prg[0] != value) {
					m116.mode0.prg[0] = value;
					map_prg_rom_8k(1, 0, value);
					map_prg_rom_8k_update();
				}
				return;
			case 0x9000:
				if (m116.mode0.nmt != value) {
					m116.mode0.nmt = value;
					if (m116.mode0.nmt & 0x01) {
						mirroring_H();
					} else {
						mirroring_V();
					}
				}
				return;
			case 0xA000:
				control_bank(prg_rom_8k_max)
				if (m116.mode0.prg[1] != value) {
					m116.mode0.prg[1] = value;
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
				}
				return;
		}
	}
}
BYTE extcl_save_mapper_116_type_C(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m116.mode);
	save_slot_ele(mode, slot, m116.chr_map);
	save_slot_ele(mode, slot, m116.prg_map);
	save_slot_ele(mode, slot, m116.mode0.chr);
	save_slot_ele(mode, slot, m116.mode0.prg);
	save_slot_ele(mode, slot, m116.mode0.nmt);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
