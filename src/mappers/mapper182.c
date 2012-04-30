/*
 * mapper182.c
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

WORD prgRom8kMax, prgRom8kBeforeLast, chrRom2kMax, chrRom1kMax;

void mapInit_182(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	prgRom8kBeforeLast = info.prgRom8kCount - 2;
	chrRom2kMax = (info.chrRom1kCount >> 1) - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(182);
	EXTCLSAVEMAPPER(MMC3);
	EXTCLCPUEVERYCYCLE(MMC3);
	EXTCLPPU000TO34X(MMC3);
	EXTCLPPU000TO255(MMC3);
	EXTCLPPU256TO319(MMC3);
	EXTCLPPU320TO34X(MMC3);
	EXTCL2006UPDATE(MMC3);
	mapper.intStruct[0] = (BYTE *) &mmc3;
	mapper.intStructSize[0] = sizeof(mmc3);

	if (info.reset >= HARD) {
		memset(&mmc3, 0x00, sizeof(mmc3));
		memset(&irqA12, 0x00, sizeof(irqA12));
	}

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extclCpuWrMem_182(WORD address, BYTE value) {
	switch (address & 0xE001) {
		case 0x8001:
			extclCpuWrMem_MMC3(0xA000, value);
			return;
		case 0xA000:
			extclCpuWrMem_MMC3(0x8000, value);
			return;
		case 0xC000: {
			switch (mmc3.bankToUpdate) {
				case 0: {
					DBWORD bank;

					value >>= 1;
					controlBank(chrRom2kMax)
					bank = value << 11;
					chr.bank1k[mmc3.chrRomCfg] = &chr.data[bank];
					chr.bank1k[mmc3.chrRomCfg | 0x01] = &chr.data[bank | 0x0400];
					break;
				}
				case 1:
					controlBank(chrRom1kMax)
					chr.bank1k[(mmc3.chrRomCfg ^ 0x04) | 0x01] = &chr.data[value << 10];
					break;
				case 2: {
					DBWORD bank;

					value >>= 1;
					controlBank(chrRom2kMax)
					bank = value << 11;
					chr.bank1k[mmc3.chrRomCfg | 0x02] = &chr.data[bank];
					chr.bank1k[mmc3.chrRomCfg | 0x03] = &chr.data[bank | 0x0400];
					break;
				}
				case 3:
					controlBank(chrRom1kMax)
					chr.bank1k[(mmc3.chrRomCfg ^ 0x04) | 0x03] = &chr.data[value << 10];
					break;
				case 4:
					controlBank(prgRom8kMax)
					mapPrgRom8k(1, mmc3.prgRomCfg, value);
					mapPrgRom8kUpdate();
					break;
				case 5:
					controlBank(prgRom8kMax)
					mapPrgRom8k(1, 1, value);
					mapPrgRom8kUpdate();
					break;
				case 6:
					controlBank(chrRom1kMax)
					chr.bank1k[mmc3.chrRomCfg ^ 0x04] = &chr.data[value << 10];
					break;
				case 7:
					controlBank(chrRom1kMax)
					chr.bank1k[(mmc3.chrRomCfg ^ 0x04) | 0x02] = &chr.data[value << 10];
					break;
			}
			return;
		}
		case 0xC001:
			irqA12.latch = value;
			irqA12.reload = TRUE;
			irqA12.counter = 0;
			return;
		case 0xE000:
		case 0xE001:
			extclCpuWrMem_MMC3(address, value);
			return;
	}
}
