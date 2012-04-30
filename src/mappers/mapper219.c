/*
 * mapper219.c
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

#define m219Chr1k(a, b)\
	value = m219.reg[2] | ((save >> 1) & a);\
	controlBank(chrRom1kMax)\
	chr.bank1k[b] = &chr.data[value << 10]

WORD prgRom8kMax, chrRom1kMax;

void mapInit_219(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(219);
	EXTCLSAVEMAPPER(219);
	EXTCLCPUEVERYCYCLE(MMC3);
	EXTCLPPU000TO34X(MMC3);
	EXTCLPPU000TO255(MMC3);
	EXTCLPPU256TO319(MMC3);
	EXTCLPPU320TO34X(MMC3);
	EXTCL2006UPDATE(MMC3);
	mapper.intStruct[0] = (BYTE *) &m219;
	mapper.intStructSize[0] = sizeof(m219);

	if (info.reset >= HARD) {
		memset(&m219, 0x00, sizeof(m219));
	}

	memset(&irqA12, 0x00, sizeof(irqA12));

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extclCpuWrMem_219(WORD address, BYTE value) {
	if (address >= 0xA000) {
		extclCpuWrMem_MMC3(address, value);
		return;
	}

	/* intercetto cio' che mi interessa */
	switch (address & 0x03) {
		case 0:
			m219.reg[0] = 0;
			m219.reg[1] = value;
			return;
		case 1: {
			BYTE save = value;

			value = m219.reg[0] - 0x23;

			if (value < 4) {
				value ^= 0x03;
				controlBank(prgRom8kMax)
				mapPrgRom8k(2, 0, value);

				value = ((value >> 5) & 0x01) | ((value >> 3) & 0x02) | ((value >> 1) & 0x04)
				        		| ((value << 1) & 0x08);
				controlBank(prgRom8kMax)
				mapPrgRom8k(2, 2, value);

				mapPrgRom8kUpdate();
			}

			switch (m219.reg[1]) {
				case 0x08:
				case 0x0A:
				case 0x0E:
				case 0x12:
				case 0x16:
				case 0x1A:
				case 0x1E:
					m219.reg[2] = save << 4;
					return;
				case 0x09:
					m219Chr1k(0x0E, 0);
					return;
				case 0x0B:
					m219Chr1k(0x01, 1);
					return;
				case 0x0C:
				case 0x0D:
					m219Chr1k(0x0E, 2);
					return;
				case 0x0F:
					m219Chr1k(0x01, 3);
					return;
				case 0x10:
				case 0x11:
					m219Chr1k(0x0F, 4);
					return;
				case 0x14:
				case 0x15:
					m219Chr1k(0x0F, 5);
					return;
				case 0x18:
				case 0x19:
					m219Chr1k(0x0F, 6);
					return;
				case 0x1C:
				case 0x1D:
					m219Chr1k(0x0F, 7);
					return;
			}

			return;
		}
		case 2:
			m219.reg[0] = value;
			m219.reg[1] = 0;
			return;
	}
}
BYTE extclSaveMapper_219(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m219.reg);

	return (EXIT_OK);
}
