/*
 * mapper183.c
 *
 *  Created on: 9/ott/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "savestate.h"

WORD prgRom8kMax, chrRom1kMax;

void mapInit_183(void) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	EXTCLCPUWRMEM(183);
	EXTCLCPURDMEM(183);
	EXTCLSAVEMAPPER(183);
	EXTCLCPUEVERYCYCLE(183);
	mapper.intStruct[0] = (BYTE *) &m183;
	mapper.intStructSize[0] = sizeof(m183);

	if (info.reset >= HARD) {
		memset(&m183, 0x00, sizeof(m183));
	}

	{
		BYTE i;
		for (i = 0; i < LENGTH(m183.chrRomBank); i++) {
			m183.chrRomBank[i] = i;
		}
	}
}
void extclCpuWrMem_183(WORD address, BYTE value) {
	switch (address & 0xF80F) {
		case 0x8800:
		case 0x8801:
		case 0x8802:
		case 0x8803:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x9800:
		case 0x9801:
		case 0x9802:
		case 0x9803: {
			switch (value & 0x03) {
				case 0:
					mirroring_V();
					break;
				case 1:
					mirroring_H();
					break;
				case 2:
					mirroring_SCR0();
					break;
				case 3:
					mirroring_SCR1();
					break;
			}
			return;
		}
		case 0xA000:
		case 0xA001:
		case 0xA002:
		case 0xA003:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 2, value);
			mapPrgRom8kUpdate();
			return;
		case 0xA800:
		case 0xA801:
		case 0xA802:
		case 0xA803:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0xF000:
		case 0xF001:
		case 0xF002:
		case 0xF003:
		case 0xF800:
		case 0xF801:
		case 0xF802:
		case 0xF803:
			m183.count = (m183.count & 0xF0) | (value & 0x0F);
			return;
		case 0xF004:
		case 0xF005:
		case 0xF006:
		case 0xF007:
		case 0xF804:
		case 0xF805:
		case 0xF806:
		case 0xF807:
			m183.count = (value << 4) | (m183.count & 0x0F);
			return;
		case 0xF008:
		case 0xF009:
		case 0xF00A:
		case 0xF00B:
		case 0xF808:
		case 0xF809:
		case 0xF80A:
		case 0xF80B:
			m183.enabled = value;
			if (!m183.enabled) {
				m183.prescaler = 0;
				irq.high &= ~EXTIRQ;
			}
			return;
		default:
			if ((address > 0xAFFF) && (address < 0xF000)) {
				const BYTE shift = address & 0x04;
				const BYTE slot = (((address - 0x3000) >> 1 | (address << 7)) & 0x1C00) >> 10;

				value = (m183.chrRomBank[slot] & (0xF0 >> shift)) | ((value & 0x0F) << shift);
				controlBank(chrRom1kMax)
				chr.bank1k[slot] = &chr.data[value << 10];
				m183.chrRomBank[slot] = value;
			}
			return;
	}
}
BYTE extclCpuRdMem_183(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg.rom[address & 0x1FFF]);
}
BYTE extclSaveMapper_183(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, m183.enabled);
	savestateEle(mode, slot, m183.prescaler);
	savestateEle(mode, slot, m183.count);
	savestateEle(mode, slot, m183.delay);
	savestateEle(mode, slot, m183.chrRomBank);

	return (EXIT_OK);
}
void extclCPUEveryCycle_183(void) {
	if (m183.delay && !(--m183.delay)) {
		irq.high |= EXTIRQ;
	}

	if (++m183.prescaler < 114) {
		return;
	}

	m183.prescaler = 0;

	if (m183.enabled && !(++m183.count)) {
		m183.delay = 1;
	}
}
