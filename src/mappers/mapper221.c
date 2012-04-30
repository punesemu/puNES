/*
 * mapper221.c
 *
 *  Created on: 14/feb/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

#define m221prg16kswap()\
	value = ((m221.reg[0] >> 1) & 0x38) | ((m221.reg[0] & 0x01) ? (m221.reg[0] & 0x80) ?\
		m221.reg[1] : (m221.reg[1] & 0x06) : m221.reg[1]);\
   	controlBank(prgRom16kMax)\
	mapPrgRom8k(2, 0, value);\
	value = ((m221.reg[0] >> 1) & 0x38) | ((m221.reg[0] & 0x01) ? (m221.reg[0] & 0x80) ?\
		0x07 : (m221.reg[1] & 0x06) | 0x1 : m221.reg[1]);\
   	controlBank(prgRom16kMax)\
	mapPrgRom8k(2, 2, value)

WORD prgRom16kMax;

void mapInit_221(void) {
	prgRom16kMax = info.prgRom16kCount - 1;

	EXTCLCPUWRMEM(221);
	EXTCLSAVEMAPPER(221);
	mapper.intStruct[0] = (BYTE *) &m221;
	mapper.intStructSize[0] = sizeof(m221);

	if (info.reset >= HARD) {
		memset(&m221, 0x00, sizeof(m221));
		{
			BYTE value;
			m221prg16kswap();
		}
	}
}
void extclCpuWrMem_221(WORD address, BYTE value) {
	BYTE reg;

	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			if (address & 0x0001) {
				mirroring_H();
			} else {
				mirroring_V();
			}

			if (m221.reg[0] == (reg = (address >> 1) & 0xFF)) {
				return;
			}
			m221.reg[0] = reg;
			m221prg16kswap();
			break;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (m221.reg[1] == (reg = address & 0x07)) {
				return;
			}
			m221.reg[1] = reg;
			m221prg16kswap();
			break;
	}
	mapPrgRom8kUpdate();
}
BYTE extclSaveMapper_221(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m221.reg);

	return (EXIT_OK);
}
