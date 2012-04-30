/*
 * mapper215.c
 *
 *  Created on: 20/mar/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "irqA12.h"
#include "savestate.h"

#define m215chr1k(vl)\
	if (m215.reg[1] & 0x04) {\
		bank = (vl | 0x100);\
	} else {\
		bank = (vl & 0x7F) | ((m215.reg[1] << 3) & 0x80);\
	}
#define m215prg8k(vl)\
	if (m215.reg[1] & 0x08) {\
		value = (vl & 0x1F) | 0x20;\
	} else {\
		value = (vl & 0x0F) | (m215.reg[1] & 0x10);\
	}
#define m215chr1kupdate()\
{\
	BYTE i;\
	for (i = 0; i < 8; i++) {\
		WORD bank;\
		m215chr1k(((chr.bank1k[i] - chr.data) >> 10))\
		_controlBank(bank, chrRom1kMax)\
		chr.bank1k[i] = &chr.data[bank << 10];\
	}\
}
#define m215prg8kupdate()\
	if (!(m215.reg[0] & 0x80)) {\
		BYTE i;\
		for (i = 0; i < 4; i++) {\
			m215prg8k(m215.prg8kBank[i]);\
			controlBank(prgRom8kMax)\
			mapPrgRom8k(1, i, value);\
		}\
	} else {\
		value = (m215.reg[0] & 0x0F) | (m215.reg[1] & 0x10);\
		controlBank(prgRom16kMax)\
		mapPrgRom8k(2, 0, value);\
		mapPrgRom8k(2, 2, value);\
	}\
	mapPrgRom8kUpdate()
#define m215_8000()\
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
		if (!(m215.reg[0] & 0x80)) {\
			WORD p0 = mapper.romMapTo[0];\
			WORD p2 = mapper.romMapTo[2];\
			mapper.romMapTo[0] = p2;\
			mapper.romMapTo[2] = p0;\
			/*\
			 * prgRomCfg 0x00 : $C000 - $DFFF fisso al penultimo banco\
			 * prgRomCfg 0x02 : $8000 - $9FFF fisso al penultimo banco\
			 */\
			m215prg8k(prgRom8kBeforeLast)\
			controlBank(prgRom8kMax)\
			mapPrgRom8k(1, mmc3.prgRomCfg ^ 0x02, value);\
			mapPrgRom8kUpdate();\
			m215.prg8kBank[0] = mapper.romMapTo[0];\
			m215.prg8kBank[1] = mapper.romMapTo[1];\
			m215.prg8kBank[2] = mapper.romMapTo[2];\
			m215.prg8kBank[3] = mapper.romMapTo[3];\
		}\
	}\
}
#define m215_8001()\
{\
	WORD bank;\
	switch (mmc3.bankToUpdate) {\
		case 0:\
			m215chr1k(value)\
			bank &= 0xFFE;\
			_controlBank(bank, chrRom1kMax)\
			chr.bank1k[mmc3.chrRomCfg] = &chr.data[bank << 10];\
			chr.bank1k[mmc3.chrRomCfg | 0x01] = &chr.data[(bank + 1) << 10];\
			return;\
		case 1:\
			m215chr1k(value)\
			bank &= 0xFFE;\
			_controlBank(bank, chrRom1kMax)\
			chr.bank1k[mmc3.chrRomCfg | 0x02] = &chr.data[bank << 10];\
			chr.bank1k[mmc3.chrRomCfg | 0x03] = &chr.data[(bank + 1) << 10];\
			return;\
		case 2:\
			m215chr1k(value)\
			_controlBank(bank, chrRom1kMax)\
			chr.bank1k[mmc3.chrRomCfg ^ 0x04] = &chr.data[bank << 10];\
			return;\
		case 3:\
			m215chr1k(value)\
			_controlBank(bank, chrRom1kMax)\
			chr.bank1k[(mmc3.chrRomCfg ^ 0x04) | 0x01] = &chr.data[bank << 10];\
			return;\
		case 4:\
			m215chr1k(value)\
			_controlBank(bank, chrRom1kMax)\
			chr.bank1k[(mmc3.chrRomCfg ^ 0x04) | 0x02] = &chr.data[bank << 10];\
			return;\
		case 5:\
			m215chr1k(value)\
			_controlBank(bank, chrRom1kMax)\
			chr.bank1k[(mmc3.chrRomCfg ^ 0x04) | 0x03] = &chr.data[bank << 10];\
			return;\
		case 6:\
			if (!(m215.reg[0] & 0x80)) {\
				m215prg8k(value)\
				controlBank(prgRom8kMax)\
				mapPrgRom8k(1, mmc3.prgRomCfg, value);\
				mapPrgRom8kUpdate();\
				m215.prg8kBank[mmc3.prgRomCfg] = mapper.romMapTo[mmc3.prgRomCfg];\
			}\
			return;\
		case 7:\
			if (!(m215.reg[0] & 0x80)) {\
				m215prg8k(value)\
				controlBank(prgRom8kMax)\
				mapPrgRom8k(1, 1, value);\
				mapPrgRom8kUpdate();\
				m215.prg8kBank[1] = mapper.romMapTo[1];\
			}\
			return;\
	}\
}

WORD prgRom16kMax, prgRom8kMax, prgRom8kBeforeLast, chrRom1kMax;

void mapInit_215(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	prgRom8kBeforeLast = info.prgRom8kCount - 2;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(215);
	EXTCLSAVEMAPPER(215);
	EXTCLCPUEVERYCYCLE(MMC3);
	EXTCLPPU000TO34X(MMC3);
	EXTCLPPU000TO255(MMC3);
	EXTCLPPU256TO319(MMC3);
	EXTCLPPU320TO34X(MMC3);
	EXTCL2006UPDATE(MMC3);
	mapper.intStruct[0] = (BYTE *) &m215;
	mapper.intStructSize[0] = sizeof(m215);
	mapper.intStruct[1] = (BYTE *) &mmc3;
	mapper.intStructSize[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	m215.reg[0] = 0x00;
	m215.reg[1] = 0xFF;
	m215.reg[2] = 0x04;
	m215.reg[3] = FALSE;
	m215.prg8kBank[0] = 0;
	m215.prg8kBank[1] = 1;
	m215.prg8kBank[2] = prgRom8kBeforeLast;
	m215.prg8kBank[3] = prgRom8kMax;

	info.mapperExtendWrite = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extclCpuWrMem_215(WORD address, BYTE value) {
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
					if (m215.reg[3] && (!(m215.reg[0] & 0x80) || (mmc3.bankToUpdate < 6))) {
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
					extclCpuWrMem_MMC3(address, value);
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
					extclCpuWrMem_MMC3(address, value);
				} else {
					extclCpuWrMem_MMC3(0xE001, value);
				}
				return;
			case 0xE001:
				if (!m215.reg[2]) {
					extclCpuWrMem_MMC3(address, value);
				} else {
					extclCpuWrMem_MMC3(0xC000, value);
					extclCpuWrMem_MMC3(0xC001, value);
				}
				return;
		}
		extclCpuWrMem_MMC3(address, value);
		return;
	}

	if ((info.id == M215MK3E) && ((address & 0xF000) == 0x6000)) {
		return;
	}

	switch (address) {
		case 0x5000:
		case 0x6000:
			if (m215.reg[0] != value) {
				m215.reg[0] = value;
				m215prg8kupdate();
			}
			return;
		case 0x5001:
		case 0x6001:
			if (m215.reg[1] != value) {
				m215.reg[1] = value;
				m215chr1kupdate()
			}
			return;
		case 0x5007:
		case 0x6007:
			mmc3.bankToUpdate = 0;
			mmc3.prgRomCfg = 0;
			mmc3.chrRomCfg = 0;

			if (m215.reg[2] != value) {
				m215.reg[2] = value;
				m215prg8kupdate();
				m215chr1kupdate()
			}
			return;
	}
}
BYTE extclSaveMapper_215(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m215.reg);
	savestateEle(mode, slot, m215.prg8kBank);
	extclSaveMapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
