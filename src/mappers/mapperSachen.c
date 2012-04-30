/*
 * mapperSachen.c
 *
 *  Created on: 27/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "savestate.h"

#define sa74347xChr8kSwap(bank8k)\
{\
	DBWORD bank;\
	value = bank8k;\
	controlBank(chrRom8kMax)\
	bank = value << 13;\
	chr.bank1k[0] = &chr.data[bank];\
	chr.bank1k[1] = &chr.data[bank | 0x0400];\
	chr.bank1k[2] = &chr.data[bank | 0x0800];\
	chr.bank1k[3] = &chr.data[bank | 0x0C00];\
	chr.bank1k[4] = &chr.data[bank | 0x1000];\
	chr.bank1k[5] = &chr.data[bank | 0x1400];\
	chr.bank1k[6] = &chr.data[bank | 0x1800];\
	chr.bank1k[7] = &chr.data[bank | 0x1C00];\
	sa74374x.chrRom8kBank = value;\
}

WORD prgRom32kMax, chrRom8kMax, chrRom4kMax, chrRom2kMax, chrRom1kMax;
BYTE type, shift, ored[3];

void mapInit_Sachen(BYTE model) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	chrRom8kMax = info.chrRom8kCount - 1;
	chrRom4kMax = info.chrRom4kCount - 1;
	chrRom2kMax = (info.chrRom1kCount >> 1) - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	switch (model) {
		case SA0036:
			EXTCLCPUWRMEM(Sachen_sa0036);
			break;
		case SA0037:
			EXTCLCPUWRMEM(Sachen_sa0037);

			if (info.reset >= HARD) {
				if (prgRom32kMax != 0xFFFF) {
					mapPrgRom8k(4, 0, 0);
				}
			}
			break;
		case SA8259A:
		case SA8259B:
		case SA8259C:
		case SA8259D: {
			EXTCLCPUWRMEM(Sachen_sa8259x);
			EXTCLSAVEMAPPER(Sachen_sa8259x);
			mapper.intStruct[0] = (BYTE *) &sa8259;
			mapper.intStructSize[0] = sizeof(sa8259);

			info.mapperExtendWrite = TRUE;

			if (info.reset >= HARD) {
				memset(&sa8259, 0x00, sizeof(sa8259));

				if (prgRom32kMax != 0xFFFF) {
					mapPrgRom8k(4, 0, 0);
				}
			}

			switch (model) {
				case SA8259A:
					shift = 1;
					ored[0] = 1;
					ored[1] = 0;
					ored[2] = 1;
					break;
				case SA8259B:
					shift = 0;
					ored[0] = 0;
					ored[1] = 0;
					ored[2] = 0;
					break;
				case SA8259C:
					shift = 2;
					ored[0] = 1;
					ored[1] = 2;
					ored[2] = 3;
					break;
				case SA8259D:
					if (!mapper.writeVRAM) {
						const DBWORD bank = chrRom4kMax << 12;

						chr.bank1k[4] = &chr.data[bank];
						chr.bank1k[5] = &chr.data[bank | 0x0400];
						chr.bank1k[6] = &chr.data[bank | 0x0800];
						chr.bank1k[7] = &chr.data[bank | 0x0C00];
					}
					break;
			}
			break;
		}
		case TCA01:
			EXTCLCPUWRMEM(Sachen_tca01);
			EXTCLCPURDMEM(Sachen_tca01);
			break;
		case TCU01:
			EXTCLCPUWRMEM(Sachen_tcu01);

			info.mapperExtendWrite = TRUE;

			if (info.reset >= HARD) {
				if (prgRom32kMax != 0xFFFF) {
					mapPrgRom8k(4, 0, 0);
				}
			}
			break;
		case TCU02:
			EXTCLCPUWRMEM(Sachen_tcu02);
			EXTCLCPURDMEM(Sachen_tcu02);
			EXTCLSAVEMAPPER(Sachen_tcu02);
			mapper.intStruct[0] = (BYTE *) &tcu02;
			mapper.intStructSize[0] = sizeof(tcu02);

			info.mapperExtendWrite = TRUE;

			if (info.reset >= HARD) {
				memset(&tcu02, 0x00, sizeof(tcu02));
			}
			break;
		case SA72007:
			EXTCLCPUWRMEM(Sachen_sa72007);

			info.mapperExtendWrite = TRUE;
			break;
		case SA72008:
			EXTCLCPUWRMEM(Sachen_sa72008);

			info.mapperExtendWrite = TRUE;
			break;
		case SA74374A:
		case SA74374B: {
			BYTE i;
			for (i = 0; i < LENGTH(pokeriiichr); i++) {
				if (!(memcmp(pokeriiichr[i], info.sha1sumStringChr, 40))) {
					if (i == 0) {
						/* Poker III 5-in-1 (Sachen) [!].nes */
						info.mapper = 150;
						model = SA74374B;
					} else {
						/* Poker III [!].nes */
						info.mapper = 243;
						model = SA74374A;
					}
				}
			}

			if (model == SA74374A) {
				EXTCLCPUWRMEM(Sachen_sa74374a);
			} else {
				EXTCLCPUWRMEM(Sachen_sa74374b);
			}
			EXTCLSAVEMAPPER(Sachen_sa74374x);
			mapper.intStruct[0] = (BYTE *) &sa74374x;
			mapper.intStructSize[0] = sizeof(sa74374x);

			info.mapperExtendWrite = TRUE;

			if (info.reset >= HARD) {
				memset(&sa74374x, 0x00, sizeof(sa74374x));
				mapPrgRom8k(4, 0, 0);
			}
			break;
		}
	}

	type = model;
}

void extclCpuWrMem_Sachen_sa0036(WORD address, BYTE value) {
	DBWORD bank;

	value >>= 7;
	controlBank(chrRom8kMax)
	bank = value << 13;
	chr.bank1k[0] = &chr.data[bank];
	chr.bank1k[1] = &chr.data[bank | 0x0400];
	chr.bank1k[2] = &chr.data[bank | 0x0800];
	chr.bank1k[3] = &chr.data[bank | 0x0C00];
	chr.bank1k[4] = &chr.data[bank | 0x1000];
	chr.bank1k[5] = &chr.data[bank | 0x1400];
	chr.bank1k[6] = &chr.data[bank | 0x1800];
	chr.bank1k[7] = &chr.data[bank | 0x1C00];
}

void extclCpuWrMem_Sachen_sa0037(WORD address, BYTE value) {
	/* bus conflict */
	const BYTE save = value &= prgRomRd(address);
	DBWORD bank;

	if (prgRom32kMax != 0xFFFF) {
		value >>= 3;
		controlBank(prgRom32kMax)
		mapPrgRom8k(4, 0, value);
		mapPrgRom8kUpdate();
		value = save;
	}

	controlBank(chrRom8kMax)
	bank = value << 13;
	chr.bank1k[0] = &chr.data[bank];
	chr.bank1k[1] = &chr.data[bank | 0x0400];
	chr.bank1k[2] = &chr.data[bank | 0x0800];
	chr.bank1k[3] = &chr.data[bank | 0x0C00];
	chr.bank1k[4] = &chr.data[bank | 0x1000];
	chr.bank1k[5] = &chr.data[bank | 0x1400];
	chr.bank1k[6] = &chr.data[bank | 0x1800];
	chr.bank1k[7] = &chr.data[bank | 0x1C00];
}

void extclCpuWrMem_Sachen_sa8259x(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x7FFF)) {
		return;
	}

	switch (address & 0x0101) {
		case 0x0100:
			sa8259.ctrl = value;
			return;
		case 0x0101: {
			const BYTE slot = sa8259.ctrl & 0x07;

			sa8259.reg[slot] = value;
			switch (slot) {
				case 5:
					if (prgRom32kMax != 0xFFFF) {
						controlBank(prgRom32kMax)
						mapPrgRom8k(4, 0, value);
						mapPrgRom8kUpdate();
					}
					break;
				case 7:
					if (value & 0x01) {
						mirroring_V();
					} else {
						switch ((value >> 1) & 0x03) {
							case 0:
								mirroring_V();
								break;
							case 1:
								mirroring_H();
								break;
							case 2:
								mirroring_SCR0x1_SCR1x3();
								break;
							case 3:
								mirroring_SCR0();
								break;
						}
					}
					break;
				default:
					if (!mapper.writeVRAM) {
						DBWORD bank;

						if (type == SA8259D) {

							value = sa8259.reg[0] & 0x07;
							controlBank(chrRom1kMax)
							bank = value << 10;
							chr.bank1k[0] = &chr.data[bank];

							value = (sa8259.reg[1] & 0x07) | ((sa8259.reg[4] << 4) & 0x10);
							controlBank(chrRom1kMax)
							bank = value << 10;
							chr.bank1k[1] = &chr.data[bank];

							value = (sa8259.reg[2] & 0x07) | ((sa8259.reg[4] << 3) & 0x10);
							controlBank(chrRom1kMax)
							bank = value << 10;
							chr.bank1k[2] = &chr.data[bank];

							value = (sa8259.reg[3] & 0x07) | ((sa8259.reg[4] << 2) & 0x10)
							        		| ((sa8259.reg[6] << 3) & 0x08);
							controlBank(chrRom1kMax)
							bank = value << 10;
							chr.bank1k[3] = &chr.data[bank];
						} else {
							const BYTE high = (sa8259.reg[4] << 3) & 0x38;

							value = (high | (sa8259.reg[0] & 0x07)) << shift;
							controlBank(chrRom2kMax)
							bank = value << 11;
							chr.bank1k[0] = &chr.data[bank];
							chr.bank1k[1] = &chr.data[bank | 0x0400];

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 1] & 0x07))
									<< shift) | ored[0];
							controlBank(chrRom2kMax)
							bank = value << 11;
							chr.bank1k[2] = &chr.data[bank];
							chr.bank1k[3] = &chr.data[bank | 0x0400];

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 2] & 0x07))
									<< shift) | ored[1];
							controlBank(chrRom2kMax)
							bank = value << 11;
							chr.bank1k[4] = &chr.data[bank];
							chr.bank1k[5] = &chr.data[bank | 0x0400];

							value = ((high | (sa8259.reg[(sa8259.reg[7] & 0x01) ? 0 : 3] & 0x07))
									<< shift) | ored[2];
							controlBank(chrRom2kMax)
							bank = value << 11;
							chr.bank1k[6] = &chr.data[bank];
							chr.bank1k[7] = &chr.data[bank | 0x0400];
						}
					}
					break;
			}
			return;
		}
	}
}
BYTE extclSaveMapper_Sachen_sa8259x(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, sa8259.ctrl);
	savestateEle(mode, slot, sa8259.reg);

	return (EXIT_OK);
}

void extclCpuWrMem_Sachen_tca01(WORD address, BYTE value) {
	return;
}
BYTE extclCpuRdMem_Sachen_tca01(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return (openbus);
	}

	if (address & 0x0100) {
		return ((~address & 0x003F) | (openbus & 0xC0));
	}

	return (openbus);
}

void extclCpuWrMem_Sachen_tcu01(WORD address, BYTE value) {
	if (address < 0x4100) {
		return;
	}

	if ((address & 0x0003) == 0x0002) {
		const BYTE save = value;
		DBWORD bank;

		if (prgRom32kMax != 0xFFFF) {
			value = ((value >> 6) & 0x02) | ((value >> 2) & 0x01);
			controlBank(prgRom32kMax)
			mapPrgRom8k(4, 0, value);
			mapPrgRom8kUpdate();
			value = save;
		}

		value = (value >> 3);
		controlBank(chrRom8kMax)
		bank = value << 13;
		chr.bank1k[0] = &chr.data[bank];
		chr.bank1k[1] = &chr.data[bank | 0x0400];
		chr.bank1k[2] = &chr.data[bank | 0x0800];
		chr.bank1k[3] = &chr.data[bank | 0x0C00];
		chr.bank1k[4] = &chr.data[bank | 0x1000];
		chr.bank1k[5] = &chr.data[bank | 0x1400];
		chr.bank1k[6] = &chr.data[bank | 0x1800];
		chr.bank1k[7] = &chr.data[bank | 0x1C00];
	}
}

void extclCpuWrMem_Sachen_tcu02(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x7FFF)) {
		return;
	}

	if ((address & 0x0003) == 0x0002) {
		DBWORD bank;

		tcu02.reg = (value & 0x30) | ((value + 3) & 0x0F);
		value = tcu02.reg;
		controlBank(chrRom8kMax)
		bank = value << 13;
		chr.bank1k[0] = &chr.data[bank];
		chr.bank1k[1] = &chr.data[bank | 0x0400];
		chr.bank1k[2] = &chr.data[bank | 0x0800];
		chr.bank1k[3] = &chr.data[bank | 0x0C00];
		chr.bank1k[4] = &chr.data[bank | 0x1000];
		chr.bank1k[5] = &chr.data[bank | 0x1400];
		chr.bank1k[6] = &chr.data[bank | 0x1800];
		chr.bank1k[7] = &chr.data[bank | 0x1C00];
	}
}
BYTE extclCpuRdMem_Sachen_tcu02(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return (openbus);
	}

	if ((address & 0x0003) == 0x0000) {
		return (tcu02.reg | 0x40);
	}

	return (openbus);
}
BYTE extclSaveMapper_Sachen_tcu02(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, tcu02.reg);

	return (EXIT_OK);
}

void extclCpuWrMem_Sachen_sa72007(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	if (address & 0x0100) {
		DBWORD bank;

		value >>= 7;
		controlBank(chrRom8kMax)
		bank = value << 13;
		chr.bank1k[0] = &chr.data[bank];
		chr.bank1k[1] = &chr.data[bank | 0x0400];
		chr.bank1k[2] = &chr.data[bank | 0x0800];
		chr.bank1k[3] = &chr.data[bank | 0x0C00];
		chr.bank1k[4] = &chr.data[bank | 0x1000];
		chr.bank1k[5] = &chr.data[bank | 0x1400];
		chr.bank1k[6] = &chr.data[bank | 0x1800];
		chr.bank1k[7] = &chr.data[bank | 0x1C00];
	}
}

void extclCpuWrMem_Sachen_sa72008(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	{
		const BYTE save = value;
		DBWORD bank;

		if (prgRom32kMax != 0xFFFF) {
			value >>= 2;
			controlBank(prgRom32kMax)
			mapPrgRom8k(4, 0, value);
			mapPrgRom8kUpdate();
			value = save;
		}

		controlBank(chrRom8kMax)
		bank = value << 13;
		chr.bank1k[0] = &chr.data[bank];
		chr.bank1k[1] = &chr.data[bank | 0x0400];
		chr.bank1k[2] = &chr.data[bank | 0x0800];
		chr.bank1k[3] = &chr.data[bank | 0x0C00];
		chr.bank1k[4] = &chr.data[bank | 0x1000];
		chr.bank1k[5] = &chr.data[bank | 0x1400];
		chr.bank1k[6] = &chr.data[bank | 0x1800];
		chr.bank1k[7] = &chr.data[bank | 0x1C00];
	}
}

void extclCpuWrMem_Sachen_sa74374a(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	switch (address & 0x4101) {
		case 0x4100:
			sa74374x.reg = value & 0x07;
			return;
		case 0x4101: {
			switch (sa74374x.reg) {
				case 0:
					mapPrgRom8k(4, 0, 0);
					mapPrgRom8kUpdate();

					sa74347xChr8kSwap(3)
					break;
				case 2:
					sa74347xChr8kSwap((sa74374x.chrRom8kBank & 0x07) | ((value << 3) & 0x08))
					break;
				case 4:
					sa74347xChr8kSwap((sa74374x.chrRom8kBank & 0x0E) | (value & 0x01))
					break;
				case 5:
					value &= 0x01;
					controlBank(prgRom32kMax)
					mapPrgRom8k(4, 0, value);
					mapPrgRom8kUpdate();
					break;
				case 6:
					sa74347xChr8kSwap((sa74374x.chrRom8kBank & 0x09) | ((value << 1) & 0x06))
					break;
				case 7: {
					switch (value & 0x03) {
						case 0:
							mirroring_V();
							break;
						case 1:
							mirroring_H();
							break;
						case 2:
							mirroring_SCR0x1_SCR1x3();
							break;
						case 3:
							mirroring_SCR0();
							break;
					}
					break;
				}
			}
			return;
		}
	}
}
void extclCpuWrMem_Sachen_sa74374b(WORD address, BYTE value) {
	if ((address < 0x4100) || (address > 0x5FFF)) {
		return;
	}

	switch (address & 0x4101) {
		case 0x4100:
			sa74374x.reg = value & 0x07;
			return;
		case 0x4101: {
			switch (sa74374x.reg) {
				case 2: {
					const BYTE save = value;
					sa74347xChr8kSwap((sa74374x.chrRom8kBank & 0x07) | ((value << 3) & 0x08))

					value = save & 0x01;
					controlBank(prgRom32kMax)
					mapPrgRom8k(4, 0, value);
					mapPrgRom8kUpdate();
					break;
				}
				case 4:
					sa74347xChr8kSwap((sa74374x.chrRom8kBank & 0x0B) | ((value << 2) & 0x04))
					break;
				case 5:
					value &= 0x07;
					controlBank(prgRom32kMax)
					mapPrgRom8k(4, 0, value);
					mapPrgRom8kUpdate();
					break;
				case 6:
					sa74347xChr8kSwap((sa74374x.chrRom8kBank & 0x0C) | (value & 0x03))
					break;
				case 7: {
					switch ((value >> 1) & 0x03) {
						case 0:
							mirroring_V();
							break;
						case 1:
							mirroring_H();
							break;
						case 2:
							mirroring_SCR0x1_SCR1x3();
							break;
						case 3:
							mirroring_SCR0();
							break;
					}
					break;
				}
			}
			return;
		}
	}
}
BYTE extclSaveMapper_Sachen_sa74374x(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, sa74374x.reg);
	savestateEle(mode, slot, sa74374x.chrRom8kBank);

	return (EXIT_OK);
}
