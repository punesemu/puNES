/*
 * mapper116.c
 *
 *  Created on: 24/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "irqA12.h"
#include "savestate.h"

WORD prgRom32kMax, prgRom16kMax, prgRom8kMax, prgRom8kBeforeLast;
WORD chrRom1kMax, chrRom2kMax, chrRom4kMax;

#define m116UpdatePrg8k(bnk, vl)\
	tmp = vl;\
	_control_bank(tmp, prgRom8kMax)\
	map_prg_rom_8k(1, bnk, tmp)
#define m116UpdatePrg16k(bnk, vl)\
	tmp = vl;\
	_control_bank(tmp, prgRom16kMax)\
	map_prg_rom_8k(2, bnk, tmp)
#define m116UpdatePrg32k(vl)\
	tmp = vl;\
	_control_bank(tmp, prgRom32kMax)\
	map_prg_rom_8k(4, 0, tmp)

#define m116UpdateChr1k(bnk, vl)\
	bank = vl;\
	_control_bank(bank, chrRom1kMax)\
	chr.bank_1k[bnk] = &chr.data[bank << 10]
#define m116UpdateChr2k(bnk, vl)\
	bank = vl;\
	_control_bank(bank, chrRom2kMax)\
	bank <<= 11;\
	chr.bank_1k[bnk       ] = &chr.data[bank         ];\
	chr.bank_1k[bnk | 0x01] = &chr.data[bank | 0x0400]
#define m116UpdateChr4k(bnk, vl)\
	bank = vl;\
	_control_bank(bank, chrRom4kMax)\
	bank <<= 12;\
	chr.bank_1k[bnk       ] = &chr.data[bank         ];\
	chr.bank_1k[bnk | 0x01] = &chr.data[bank | 0x0400];\
	chr.bank_1k[bnk | 0x02] = &chr.data[bank | 0x0800];\
	chr.bank_1k[bnk | 0x03] = &chr.data[bank | 0x0C00]

#define m116_A_UpdatePrgMode0()\
{\
	BYTE tmp;\
	m116UpdatePrg8k(0, m116.mode0.prg[0]);\
	m116UpdatePrg8k(1, m116.mode0.prg[1]);\
	m116UpdatePrg8k(2, 0x1E);\
	m116UpdatePrg8k(3, 0x1F);\
	map_prg_rom_8k_update();\
}
#define m116_A_UpdatePrgMode1()\
{\
	BYTE tmp, i = (m116.mode1.ctrl >> 5) & 0x02;\
	m116UpdatePrg8k(0, m116.mode1.banks[6 + i]);\
	m116UpdatePrg8k(1, m116.mode1.banks[6 + 1]);\
	m116UpdatePrg8k(2, m116.mode1.banks[6 + (i ^ 2)]);\
	m116UpdatePrg8k(3, m116.mode1.banks[6 + 3]);\
	map_prg_rom_8k_update();\
}
#define m116_A_UpdatePrgMode2()\
{\
	BYTE tmp, bank = m116.mode2.reg[3] & 0x0F;\
	if (m116.mode2.reg[0] & 0x08) {\
		m116UpdatePrg16k(0, (m116.mode2.reg[0] & 0x04) ? bank : 0);\
		m116UpdatePrg16k(2, (m116.mode2.reg[0] & 0x04) ? 0x0F : bank);\
	} else {\
		m116UpdatePrg32k(bank >> 1);\
	}\
	map_prg_rom_8k_update();\
}
#define m116_A_UpdatePrg()\
	switch(m116.mode & 0x03) {\
		case 0:\
			m116_A_UpdatePrgMode0()\
			break;\
		case 1:\
			m116_A_UpdatePrgMode1()\
			break;\
		case 2:\
			m116_A_UpdatePrgMode2()\
			break;\
	}
#define m116_A_UpdateChrMode0()\
{\
	DBWORD bank;\
	WORD base = (m116.mode & 0x04) << 6;\
	m116UpdateChr1k(0, base | m116.mode0.chr[0]);\
	m116UpdateChr1k(1, base | m116.mode0.chr[1]);\
	m116UpdateChr1k(2, base | m116.mode0.chr[2]);\
	m116UpdateChr1k(3, base | m116.mode0.chr[3]);\
	m116UpdateChr1k(4, base | m116.mode0.chr[4]);\
	m116UpdateChr1k(5, base | m116.mode0.chr[5]);\
	m116UpdateChr1k(6, base | m116.mode0.chr[6]);\
	m116UpdateChr1k(7, base | m116.mode0.chr[7]);\
}
#define m116_A_UpdateChrMode1()\
{\
	DBWORD bank;\
	WORD base = (m116.mode & 0x04) << 6;\
	BYTE swap = (m116.mode1.ctrl & 0x80) >> 5;\
	m116UpdateChr2k((swap | 0x00), (base >> 1) | m116.mode1.banks[0]);\
	m116UpdateChr2k((swap | 0x02), (base >> 1) | m116.mode1.banks[1]);\
	swap ^= 0x04;\
	m116UpdateChr1k((swap | 0x00), base | m116.mode1.banks[2]);\
	m116UpdateChr1k((swap | 0x01), base | m116.mode1.banks[3]);\
	m116UpdateChr1k((swap | 0x02), base | m116.mode1.banks[4]);\
	m116UpdateChr1k((swap | 0x03), base | m116.mode1.banks[5]);\
}
#define m116_A_UpdateChrMode2()\
{\
	DBWORD bank;\
	m116UpdateChr4k(0, (m116.mode2.reg[0] & 0x10) ? m116.mode2.reg[1] :\
			m116.mode2.reg[1] & 0x1E);\
	m116UpdateChr4k(4, (m116.mode2.reg[0] & 0x10) ? m116.mode2.reg[2] :\
			m116.mode2.reg[1] | 0x01);\
}
#define m116_A_UpdateChr()\
	switch(m116.mode & 0x03) {\
		case 0:\
			m116_A_UpdateChrMode0()\
			break;\
		case 1:\
			m116_A_UpdateChrMode1()\
			break;\
		case 2:\
			m116_A_UpdateChrMode2()\
			break;\
	}
#define m116_A_UpdateMirroringMode0()\
	if (m116.mode0.nmt & 0x01) {\
		mirroring_H();\
	} else {\
		mirroring_V();\
	}
#define m116_A_UpdateMirroringMode1()\
	if (m116.mode1.nmt & 0x01) {\
		mirroring_H();\
	} else {\
		mirroring_V();\
	}
#define m116_A_UpdateMirroringMode2()\
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
#define m116_A_UpdateMirroring()\
	switch(m116.mode & 0x03) {\
		case 0:\
			m116_A_UpdateMirroringMode0()\
			break;\
		case 1:\
			m116_A_UpdateMirroringMode1()\
			break;\
		case 2:\
			m116_A_UpdateMirroringMode2()\
			break;\
		default:\
			break;\
	}

#define m116_B_chr1k(a)\
	m116.chrmap[a] = value;\
	if (!(m116.mode & 0x02)) {\
		chr.bank_1k[a] = &chr.data[value << 10];\
	}
#define m116_B_swapChrBank1k(src, dst)\
{\
	BYTE *chrBank1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chrBank1k;\
	WORD map = m116.chrmap[src];\
	m116.chrmap[src] = m116.chrmap[dst];\
	m116.chrmap[dst] = map;\
}
#define m116_B_8000()\
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
	if ((mmc3.chrRomCfg != chrRomCfgOld) && !(m116.mode & 0x02)) {\
		m116_B_swapChrBank1k(0, 4)\
		m116_B_swapChrBank1k(1, 5)\
		m116_B_swapChrBank1k(2, 6)\
		m116_B_swapChrBank1k(3, 7)\
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
		map_prg_rom_8k(1, mmc3.prgRomCfg ^ 0x02, prgRom8kBeforeLast);\
		map_prg_rom_8k_update();\
	}\
}
#define m116_B_8001()\
{\
	switch (mmc3.bankToUpdate) {\
		case 0:\
			control_bank_with_AND(0xFE, chrRom1kMax)\
			m116_B_chr1k(mmc3.chrRomCfg)\
			value++;\
			m116_B_chr1k(mmc3.chrRomCfg | 0x01)\
			return;\
		case 1:\
			control_bank_with_AND(0xFE, chrRom1kMax)\
			m116_B_chr1k(mmc3.chrRomCfg | 0x02)\
			value++;\
			m116_B_chr1k(mmc3.chrRomCfg | 0x03)\
			return;\
		case 2:\
			control_bank(chrRom1kMax)\
			m116_B_chr1k(mmc3.chrRomCfg ^ 0x04)\
			return;\
		case 3:\
			control_bank(chrRom1kMax)\
			m116_B_chr1k((mmc3.chrRomCfg ^ 0x04) | 0x01)\
			return;\
		case 4:\
			control_bank(chrRom1kMax)\
			m116_B_chr1k((mmc3.chrRomCfg ^ 0x04) | 0x02)\
			return;\
		case 5:\
			control_bank(chrRom1kMax)\
			m116_B_chr1k((mmc3.chrRomCfg ^ 0x04) | 0x03)\
			return;\
	}\
}

#define m116_C_chr1k(a, vl)\
{\
	static const BYTE modes[4] = { 5, 5, 3, 1 };\
	bank = vl | ((m116.mode << modes[(a >> 1) ^ ((mmc3.chrRomCfg >> 1) & 0x02)]) & 0x100);\
}
#define m116_C_swapChrBank1k(src, dst)\
{\
	BYTE *chrBank1k = chr.bank_1k[src];\
	chr.bank_1k[src] = chr.bank_1k[dst];\
	chr.bank_1k[dst] = chrBank1k;\
	WORD map = m116.chrmap[src];\
	m116.chrmap[src] = m116.chrmap[dst];\
	m116.chrmap[dst] = map;\
}
#define m116_C_8000()\
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
		m116_C_swapChrBank1k(0, 4)\
		m116_C_swapChrBank1k(1, 5)\
		m116_C_swapChrBank1k(2, 6)\
		m116_C_swapChrBank1k(3, 7)\
	}\
	if (mmc3.prgRomCfg != prgRomCfgOld) {\
		WORD p0 = mapper.rom_map_to[0];\
		WORD p2 = mapper.rom_map_to[2];\
		mapper.rom_map_to[0] = p2;\
		mapper.rom_map_to[2] = p0;\
		p0 = m116.prgmap[0];\
		p2 = m116.prgmap[2];\
		m116.prgmap[0] = p2;\
		m116.prgmap[2] = p0;\
		m116.prgmap[mmc3.prgRomCfg ^ 0x02] = prgRom8kBeforeLast;\
		/*\
		 * prgRomCfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
		 * prgRomCfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
		 */\
		map_prg_rom_8k(1, mmc3.prgRomCfg ^ 0x02, prgRom8kBeforeLast);\
		map_prg_rom_8k_update();\
	}\
}
#define m116_C_8001()\
{\
	WORD bank;\
	BYTE slot;\
	switch (mmc3.bankToUpdate) {\
		case 0:\
			slot = mmc3.chrRomCfg;\
			m116.chrmap[slot] = value;\
			m116.chrmap[slot + 1] = value + 1;\
			m116_C_chr1k(slot, value);\
			bank &= 0xFFE;\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			chr.bank_1k[slot + 1] = &chr.data[(bank + 1) << 10];\
			return;\
		case 1:\
			slot = mmc3.chrRomCfg | 0x02;\
			m116.chrmap[slot] = value;\
			m116.chrmap[slot + 1] = value + 1;\
			m116_C_chr1k(slot, value);\
			bank &= 0xFFE;\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			chr.bank_1k[slot + 1] = &chr.data[(bank + 1) << 10];\
			return;\
		case 2:\
			slot = mmc3.chrRomCfg ^ 0x04;\
			m116.chrmap[slot] = value;\
			m116_C_chr1k(slot, value);\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			return;\
		case 3:\
			slot = (mmc3.chrRomCfg ^ 0x04) | 0x01;\
			m116.chrmap[slot] = value;\
			m116_C_chr1k(slot, value);\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			return;\
		case 4:\
			slot = (mmc3.chrRomCfg ^ 0x04) | 0x02;\
			m116.chrmap[slot] = value;\
			m116_C_chr1k(slot, value);\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			return;\
		case 5:\
			slot = (mmc3.chrRomCfg ^ 0x04) | 0x03;\
			m116.chrmap[slot] = value;\
			m116_C_chr1k(slot, value);\
			_control_bank(bank, chrRom1kMax)\
			chr.bank_1k[slot] = &chr.data[bank << 10];\
			return;\
		case 6:\
			control_bank(prgRom8kMax)\
			m116.prgmap[mmc3.prgRomCfg] = value;\
			map_prg_rom_8k(1, mmc3.prgRomCfg, value);\
			map_prg_rom_8k_update();\
			return;\
		case 7:\
			control_bank(prgRom8kMax)\
			m116.prgmap[1] = value;\
			map_prg_rom_8k(1, 1, value);\
			map_prg_rom_8k_update();\
			return;\
	}\
}

void map_init_116(void) {
	prgRom32kMax = (info.prg_rom_16k_count >> 1) - 1;
	prgRom16kMax = info.prg_rom_16k_count - 1;
	prgRom8kBeforeLast = info.prg_rom_8k_count - 2;
	prgRom8kMax = info.prg_rom_8k_count - 1;
	chrRom4kMax = info.chr_rom_4k_count - 1;
	chrRom2kMax = (info.chr_rom_1k_count >> 1) - 1;
	chrRom1kMax = info.chr_rom_1k_count - 1;

	switch (info.mapper_type) {
		default:
		case M116TYPEA:
			EXTCL_CPU_WR_MEM(116_TypeA);
			EXTCL_SAVE_MAPPER(116_TypeA);
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

			m116_A_UpdatePrg()
			m116_A_UpdateChr()
			m116_A_UpdateMirroring()

			info.mapper_extend_wr = TRUE;

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case M116TYPEB:
			EXTCL_CPU_WR_MEM(116_TypeB);
			EXTCL_SAVE_MAPPER(116_TypeB);
			EXTCL_WR_CHR(116_TypeB);
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
						m116.chrmap[i] = i;
					}
				}
			}

			info.mapper_extend_wr = TRUE;

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;
		case M116TYPEC:
			EXTCL_CPU_WR_MEM(116_TypeC);
			EXTCL_SAVE_MAPPER(116_TypeC);
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
						m116.prgmap[i] = mapper.rom_map_to[i];
						if (i < 2) {
							m116.mode0.prg[i] = 0;
						} else {
							m116.mode0.prg[i] = mapper.rom_map_to[i];
						}
					}

					for (i = 0; i < 8; i++) {
						m116.chrmap[i] = i;
						m116.mode0.chr[i] = 0;
					}
				}
			}

			irqA12.present = TRUE;
			irqA12_delay = 1;
			break;

	}
}

void extcl_cpu_wr_mem_116_TypeA(WORD address, BYTE value) {
	BYTE reg, mode;

	if (address < 0x4100) {
		return;
	}

	if ((address < 0x6000) && (address & 0x0100)) {
		if (m116.mode != value) {
			m116.mode = value;
			if ((value & 0x01) != 1) {
				irqA12.enable = FALSE;
				irq.high &= ~EXTIRQ;
			}
			m116_A_UpdatePrg()
			m116_A_UpdateChr()
			m116_A_UpdateMirroring()
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
					m116_A_UpdatePrgMode0()
				}
				return;
			case 0x09:
				value &= 0x01;
				if (m116.mode0.nmt != value) {
					m116.mode0.nmt = value;
					m116_A_UpdateMirroringMode0()
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
					m116_A_UpdateChrMode0()
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
							m116_A_UpdateChrMode1()
						} else {
							m116_A_UpdatePrgMode1()
						}
					}
				} else {
					reg = m116.mode1.ctrl ^ value;
					m116.mode1.ctrl = value;
					if (reg & 0x40) {
						m116_A_UpdatePrgMode1()
					}
					if (reg & (0x80 | 0x07)) {
						m116_A_UpdateChrMode1()
					}
				}
				return;
			case 0x0A:
			case 0x0B:
				if (!(address & 0x0001)) {
					if (m116.mode1.nmt != value) {
						m116.mode1.nmt = value;
						m116_A_UpdateMirroringMode1()
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
					irq.high &= ~EXTIRQ;
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

						m116_A_UpdatePrgMode2();
						m116_A_UpdateChrMode2();
						m116_A_UpdateMirroringMode2();
					}
				} else {
					m116.mode2.buffer = 0;
					m116.mode2.shifter = 0;

					if ((m116.mode2.reg[0] & (0x04 | 0x08)) != (0x04 | 0x08)) {
						m116.mode2.reg[0] |= (0x04 | 0x08);

						m116_A_UpdatePrgMode2();
						m116_A_UpdateChrMode2();
						m116_A_UpdateMirroringMode2();
					}
				}
				return;
		}
	}
}
BYTE extcl_save_mapper_116_TypeA(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m116.mode);

	savestateEle(mode, slot, m116.mode0.chr);
	savestateEle(mode, slot, m116.mode0.prg);
	savestateEle(mode, slot, m116.mode0.nmt);
	savestateEle(mode, slot, m116.mode0.padding);

	savestateEle(mode, slot, m116.mode1.banks);
	savestateEle(mode, slot, m116.mode1.ctrl);
	savestateEle(mode, slot, m116.mode1.nmt);

	savestateEle(mode, slot, m116.mode2.reg);
	savestateEle(mode, slot, m116.mode2.buffer);
	savestateEle(mode, slot, m116.mode2.shifter);
	savestateEle(mode, slot, m116.mode2.padding);

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_116_TypeB(WORD address, BYTE value) {
	BYTE save = value;

	if (address == 0x4100) {
		if (m116.mode != value) {
			m116.mode = value;
			if (value & 0x02) {
				chr.bank_1k[0] = &m116.chrRam[0 << 10];
				chr.bank_1k[1] = &m116.chrRam[1 << 10];
				chr.bank_1k[2] = &m116.chrRam[2 << 10];
				chr.bank_1k[3] = &m116.chrRam[3 << 10];
				chr.bank_1k[4] = &m116.chrRam[4 << 10];
				chr.bank_1k[5] = &m116.chrRam[5 << 10];
				chr.bank_1k[6] = &m116.chrRam[6 << 10];
				chr.bank_1k[7] = &m116.chrRam[7 << 10];
			} else {
				chr.bank_1k[0] = &chr.data[m116.chrmap[0] << 10];
				chr.bank_1k[1] = &chr.data[m116.chrmap[1] << 10];
				chr.bank_1k[2] = &chr.data[m116.chrmap[2] << 10];
				chr.bank_1k[3] = &chr.data[m116.chrmap[3] << 10];
				chr.bank_1k[4] = &chr.data[m116.chrmap[4] << 10];
				chr.bank_1k[5] = &chr.data[m116.chrmap[5] << 10];
				chr.bank_1k[6] = &chr.data[m116.chrmap[6] << 10];
				chr.bank_1k[7] = &chr.data[m116.chrmap[7] << 10];
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
BYTE extcl_save_mapper_116_TypeB(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m116.mode);
	savestateEle(mode, slot, m116.chrmap);
	savestateEle(mode, slot, m116.chrRam);
	extcl_save_mapper_MMC3(mode, slot, fp);

	if ((mode == SSREAD) && (m116.mode & 0x02)) {
		BYTE i;

		for (i = 0; i < 8; i++) {
			chr.bank_1k[i] = &m116.chrRam[i << 10];
		}
	}

	return (EXIT_OK);
}
void extcl_wr_chr_116_TypeB(WORD address, BYTE value) {
	const BYTE slot = address >> 10;

	if (m116.mode & 0x02) {
		chr.bank_1k[slot][address & 0x3FF] = value;
	}
}

void extcl_cpu_wr_mem_116_TypeC(WORD address, BYTE value) {
	if (((address & 0xA131) == 0xA131) && (m116.mode != value)) {
		BYTE i;

		m116.mode = value;

		for (i = 0; i < 4; i++) {
			if (m116.mode & 0x02) {
				map_prg_rom_8k(1, i, m116.prgmap[i]);
			} else {
				map_prg_rom_8k(1, i, m116.mode0.prg[i]);
			}
		}
		map_prg_rom_8k_update();

		for (i = 0; i < 8; i++) {
			if (m116.mode & 0x02) {
				WORD bank;
				BYTE value = m116.chrmap[i];

				m116_C_chr1k(i, value);
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

		_control_bank(m116.mode0.chr[address], chrRom1kMax)
		chr.bank_1k[address] = &chr.data[m116.mode0.chr[address] << 10];
		return;
	} else {
		switch (address & 0xF003) {
			case 0x8000:
				control_bank(prgRom8kMax)
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
				control_bank(prgRom8kMax)
				if (m116.mode0.prg[1] != value) {
					m116.mode0.prg[1] = value;
					map_prg_rom_8k(1, 1, value);
					map_prg_rom_8k_update();
				}
				return;
		}
    }
}
BYTE extcl_save_mapper_116_TypeC(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m116.mode);
	savestateEle(mode, slot, m116.chrmap);
	savestateEle(mode, slot, m116.prgmap);
	savestateEle(mode, slot, m116.mode0.chr);
	savestateEle(mode, slot, m116.mode0.prg);
	savestateEle(mode, slot, m116.mode0.nmt);
	extcl_save_mapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
