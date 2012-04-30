/*
 * mapper114.c
 *
 *  Created on: 8/ott/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "irqA12.h"
#include "savestate.h"

#define m114prgRomBackup()\
	m114.prgRomBank[0] = mapper.romMapTo[0];\
	m114.prgRomBank[1] = mapper.romMapTo[1];\
	m114.prgRomBank[2] = mapper.romMapTo[2];\
	m114.prgRomBank[3] = mapper.romMapTo[3]
#define m114prgRomRestore()\
	mapper.romMapTo[0] = m114.prgRomBank[0];\
	mapper.romMapTo[1] = m114.prgRomBank[1];\
	mapper.romMapTo[2] = m114.prgRomBank[2];\
	mapper.romMapTo[3] = m114.prgRomBank[3]

static const BYTE vlu114[8] = {0, 3, 1, 5, 6, 7, 2, 4};

WORD prgRom16kMax, prgRom8kMax, prgRom8kBeforeLast, chrRom2kMax, chrRom1kMax;

void mapInit_114(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	prgRom8kBeforeLast = info.prgRom8kCount - 2;
	chrRom2kMax = (info.chrRom1kCount >> 1) - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(114);
	EXTCLSAVEMAPPER(114);
	EXTCLCPUEVERYCYCLE(MMC3);
	EXTCLPPU000TO34X(MMC3);
	EXTCLPPU000TO255(MMC3);
	EXTCLPPU256TO319(MMC3);
	EXTCLPPU320TO34X(MMC3);
	EXTCL2006UPDATE(MMC3);
	mapper.intStruct[0] = (BYTE *) &m114;
	mapper.intStructSize[0] = sizeof(m114);
	mapper.intStruct[1] = (BYTE *) &mmc3;
	mapper.intStructSize[1] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&m114, 0x00, sizeof(m114));
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));

		{
			BYTE i;

			for (i = 0; i < 4; i++) {
				m114.prgRomBank[i] = mapper.romMapTo[i];
			}
		}
	}

	info.mapperExtendWrite = TRUE;

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extclCpuWrMem_114(WORD address, BYTE value) {
	if (address < 0x5000) {
		return;
	}

	switch (address & 0xE001) {
		case 0x4000:
		case 0x4001:
		case 0x6000:
		case 0x6001:
			m114.prgRomSwitch = value >> 7;
			if (m114.prgRomSwitch) {
				controlBankWithAND(0x1F, prgRom16kMax)
				mapPrgRom8k(2, 0, value);
				mapPrgRom8k(2, 2, value);
				mapPrgRom8kUpdate();
			} else {
				m114prgRomRestore();
				mapPrgRom8kUpdate();
			}
			return;
		case 0x8000:
		case 0x8001:
			extclCpuWrMem_MMC3(0xA000, value);
			return;
		case 0xA000:
		case 0xA001:
			value = (value & 0xC0) | vlu114[value & 0x07];
			m114.mmc3CtrlChange = TRUE;
			extclCpuWrMem_MMC3(0x8000, value);
			if (m114.prgRomSwitch) {
				const BYTE prgRomCfg = (value & 0x40) >> 5;

				if (mmc3.prgRomCfg != prgRomCfg) {
					BYTE p0 = m114.prgRomBank[0];
					BYTE p2 = m114.prgRomBank[2];
					m114.prgRomBank[0] = p2;
					m114.prgRomBank[2] = p0;
					m114.prgRomBank[prgRomCfg ^ 0x02] = prgRom8kBeforeLast;
				}
			} else {
				m114prgRomBackup();
			}
			return;
		case 0xC000:
		case 0xC001:
			if (m114.mmc3CtrlChange && (!m114.prgRomSwitch || (mmc3.bankToUpdate < 6))) {
				m114.mmc3CtrlChange = FALSE;
				extclCpuWrMem_MMC3(0x8001, value);
				m114prgRomBackup();
			}
			return;
		case 0xE000:
			irqA12.enable = FALSE;
			irq.high &= ~EXTIRQ;
			return;
		case 0xE001:
			irqA12.enable = TRUE;
			irqA12.latch = value;
			irqA12.reload = TRUE;
			irqA12.counter = 0;
			return;
	}
}
BYTE extclSaveMapper_114(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m114.prgRomSwitch);
	savestateEle(mode, slot, m114.mmc3CtrlChange);
	if (savestate.version < 6) {
		if (mode == SSREAD) {
			BYTE old_prgRomBank[4], i;

			savestateEle(mode, slot, old_prgRomBank)

			for (i = 0; i < 4; i++) {
				m114.prgRomBank[i] = old_prgRomBank[i];
			}
		} else if (mode == SSCOUNT) {
			savestate.totSize[slot] += sizeof(BYTE) * 4;
		}
	} else {
		savestateEle(mode, slot, m114.prgRomBank);
	}
	extclSaveMapper_MMC3(mode, slot, fp);

	return (EXIT_OK);
}
