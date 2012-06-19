/*
 * mapperTaito.c
 *
 *  Created on: 17/lug/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "memmap.h"
#include "mappers.h"
#include "irqA12.h"
#include "savestate.h"

WORD prgRom8kMax, chrRom2kMax, chrRom1kMax;
BYTE type;

void mapInit_Taito(BYTE model) {
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom2kMax = (info.chrRom1kCount >> 1) - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	switch (model) {
		case TC0190FMC: {
			switch (info.mapperType) {
				case TC0190FMCPAL16R4:
					EXTCLCPUWRMEM(Taito_TC0190FMCPAL16R4);
					EXTCLCPUEVERYCYCLE(MMC3);
					EXTCLPPU000TO34X(Taito_TC0190FMCPAL16R4);
					EXTCLPPU000TO255(Taito_TC0190FMCPAL16R4);
					EXTCLPPU256TO319(Taito_TC0190FMCPAL16R4);
					EXTCLPPU320TO34X(Taito_TC0190FMCPAL16R4);
					EXTCL2006UPDATE(MMC3);

					if (info.reset >= HARD) {
						memset(&irqA12, 0x00, sizeof(irqA12));
					}
					irqA12.present = TRUE;
					irqA12_delay = 7;

					mirroring_V();
					break;
				default:
					EXTCLCPUWRMEM(Taito_TC0190FMC);
					break;
			}
			break;
		}
		case X1005A:
		case X1005B:
			EXTCLCPUWRMEM(Taito_X1005);
			EXTCLCPURDMEM(Taito_X1005);
			EXTCLSAVEMAPPER(Taito_X1005);
			EXTCLBATTERYIO(Taito_X1005);
			mapper.intStruct[0] = (BYTE *) &taitoX1005;
			mapper.intStructSize[0] = sizeof(taitoX1005);

			info.mapperExtendWrite = TRUE;

			if (info.reset > HARD) {
				memset(&taitoX1005, 0x00, sizeof(taitoX1005));
			} else if (info.reset == HARD) {
				taitoX1005.enable = 0;
			}

			if (model == X1005A) {
				mirroring_H();
			} else {
				mirroring_SCR0();
			}

			if (info.id == X1005NOBAT) {
				info.prgRamBatBanks = FALSE;
			} else {
				info.prgRamBatBanks = TRUE;
			}

			break;
		case X1017:
			EXTCLCPUWRMEM(Taito_X1017);
			EXTCLSAVEMAPPER(Taito_X1017);
			mapper.intStruct[0] = (BYTE *) &taitoX1017;
			mapper.intStructSize[0] = sizeof(taitoX1017);

			info.mapperExtendWrite = TRUE;

			info.prgRamPlus8kCount = 1;
			info.prgRamBatBanks = 1;

			break;
	}

	type = model;
}

void extclCpuWrMem_Taito_TC0190FMC(WORD address, BYTE value) {
	DBWORD bank;

	switch (address & 0xF003) {
		case 0x8000:
			if (value & 0x40) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			controlBankWithAND(0x3F, prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x8001:
			controlBankWithAND(0x3F, prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0x8002:
			controlBank(chrRom2kMax)
			bank = value << 11;
			chr.bank1k[0] = &chr.data[bank];
			chr.bank1k[1] = &chr.data[bank | 0x0400];
			return;
		case 0x8003:
			controlBank(chrRom2kMax)
			bank = value << 11;
			chr.bank1k[2] = &chr.data[bank];
			chr.bank1k[3] = &chr.data[bank | 0x0400];
			return;
		case 0xA000:
			controlBank(chrRom1kMax)
			bank = value << 10;
			chr.bank1k[4] = &chr.data[bank];
			return;
		case 0xA001:
			controlBank(chrRom1kMax)
			bank = value << 10;
			chr.bank1k[5] = &chr.data[bank];
			return;
		case 0xA002:
			controlBank(chrRom1kMax)
			bank = value << 10;
			chr.bank1k[6] = &chr.data[bank];
			return;
		case 0xA003:
			controlBank(chrRom1kMax)
			bank = value << 10;
			chr.bank1k[7] = &chr.data[bank];
			return;
	}
}

void extclCpuWrMem_Taito_TC0190FMCPAL16R4(WORD address, BYTE value) {
	DBWORD bank;

	switch (address & 0xF003) {
		case 0x8000:
			controlBankWithAND(0x3F, prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x8001:
			controlBankWithAND(0x3F, prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0x8002:
			controlBank(chrRom2kMax)
			bank = value << 11;
			chr.bank1k[0] = &chr.data[bank];
			chr.bank1k[1] = &chr.data[bank | 0x0400];
			return;
		case 0x8003:
			controlBank(chrRom2kMax)
			bank = value << 11;
			chr.bank1k[2] = &chr.data[bank];
			chr.bank1k[3] = &chr.data[bank | 0x0400];
			return;
		case 0xA000:
			controlBank(chrRom1kMax)
			bank = value << 10;
			chr.bank1k[4] = &chr.data[bank];
			return;
		case 0xA001:
			controlBank(chrRom1kMax)
			bank = value << 10;
			chr.bank1k[5] = &chr.data[bank];
			return;
		case 0xA002:
			controlBank(chrRom1kMax)
			bank = value << 10;
			chr.bank1k[6] = &chr.data[bank];
			return;
		case 0xA003:
			controlBank(chrRom1kMax)
			bank = value << 10;
			chr.bank1k[7] = &chr.data[bank];
			return;
		case 0xC000:
			irqA12.latch = (0x100 - value) & 0xFF;
			return;
		case 0xC001:
			irqA12.reload = TRUE;
			irqA12.counter = 0;
			return;
		case 0xC002:
			irqA12.enable = TRUE;
			return;
		case 0xC003:
			irqA12.enable = FALSE;
			irq.high &= ~EXTIRQ;
			return;
		case 0xE000:
			if (value & 0x40) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
	}
}
void extclPPU000to34x_Taito_TC0190FMCPAL16R4(void) {
	irqA12_RS();
}
void extclPPU000to255_Taito_TC0190FMCPAL16R4(void) {
	if (r2001.visible) {
		irqA12_SB();
	}
}
void extclPPU256to319_Taito_TC0190FMCPAL16R4(void) {
	irqA12_BS();
}
void extclPPU320to34x_Taito_TC0190FMCPAL16R4(void) {
	irqA12_SB();
}

void extclCpuWrMem_Taito_X1005(WORD address, BYTE value) {
	if ((address < 0x7EF0) || (address > 0x7FFF)) {
		return;
	}

	switch (address & 0xFFFE) {
		case 0x7EF0: {
			const BYTE slot = (address & 0x0001) << 1;
			DBWORD bank;

			if (type == X1005B) {
				if (value & 0x80) {
					mirroring_SCR1();
				} else {
					mirroring_SCR0();
				}
			}
			value >>= 1;
			controlBank(chrRom2kMax)
			bank = value << 11;
			chr.bank1k[slot] = &chr.data[bank];
			chr.bank1k[slot | 0x01] = &chr.data[bank | 0x0400];
			return;
		}
		case 0x7EF2:
		case 0x7EF4:
			if (type == X1005B) {
				if (value & 0x80) {
					mirroring_SCR1();
				} else {
					mirroring_SCR0();
				}
			}
			controlBank(chrRom1kMax)
			chr.bank1k[(address & 0x0007) + 2] = &chr.data[value << 10];
			return;
		case 0x7EF6:
			if (type == X1005A) {
				if (value & 0x01) {
					mirroring_V();
				} else {
					mirroring_H();
				}
			}
			return;
		case 0x7EF8:
			taitoX1005.enable = value;
			return;
		case 0x7EFA:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x7EFC:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0x7EFE:
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 2, value);
			mapPrgRom8kUpdate();
			return;
		default:
			if (taitoX1005.enable == 0xA3) {
				taitoX1005.ram[address & 0x007F] = value;
			}
			return;
	}
}
BYTE extclCpuRdMem_Taito_X1005(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x7EF8) || (address > 0x7FFF)) {
		return (openbus);
	}

	if ((address & 0xFFFE) == 0x7FE8) {
		return (taitoX1005.enable);
	}

	if ((address & 0xFF00) == 0x7F00) {
		if (taitoX1005.enable == 0xA3) {
			return (taitoX1005.ram[address & 0x007F]);
		}
		return (0x7F);
	}

	return (openbus);
}
BYTE extclSaveMapper_Taito_X1005(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, taitoX1005.ram);
	savestateEle(mode, slot, taitoX1005.enable);

	return (EXIT_OK);
}
void extclBatteryIO_Taito_X1005(BYTE mode, FILE *fp) {
	if (!fp) {
		return;
	}

	if (info.prgRamBatBanks) {
		if (mode == WRBAT) {
			if (fwrite(&taitoX1005.ram[0], LENGTH(taitoX1005.ram), 1, fp) < 1) {
				fprintf(stderr, "error on write battery memory\n");
			}
		} else {
			if (fread(&taitoX1005.ram[0], LENGTH(taitoX1005.ram), 1, fp) < 1) {
				fprintf(stderr, "error on read battery memory\n");
			}
		}
	}
}

void extclCpuWrMem_Taito_X1017(WORD address, BYTE value) {
	if ((address < 0x7EF0) || (address > 0x7EFF)) {
		return;
	}

	switch (address & 0x7FFF) {
		case 0x7EF0:
		case 0x7EF1: {
			BYTE slot = address & 0x0001;

			value >>= 1;
			controlBank(chrRom2kMax)
			if (taitoX1017.chr[slot] != value) {
				const BYTE tmp = (taitoX1017.control & 0x02) << 1;
				const BYTE chr1k = slot << 1;
				const DBWORD bank = value << 11;

				chr.bank1k[chr1k | tmp] = &chr.data[bank];
				chr.bank1k[(chr1k + 1) | tmp] = &chr.data[bank | 0x0400];
				taitoX1017.chr[slot] = value;
			}
			return;
		}
		case 0x7EF2:
		case 0x7EF3:
		case 0x7EF4:
		case 0x7EF5: {
			BYTE slot = address & 0x0007;

			controlBank(chrRom1kMax)
			if (taitoX1017.chr[slot] != value) {
				const DBWORD bank = value << 10;

				if (taitoX1017.control & 0x02) {
					chr.bank1k[slot - 2] = &chr.data[bank];
				} else {
					chr.bank1k[slot + 2] = &chr.data[bank];
				}
				taitoX1017.chr[slot] = value;
			}
			return;
		}
		case 0x7EF6:
			if (taitoX1017.control != value) {
				if (value & 0x01) {
					mirroring_V();
				} else {
					mirroring_H();
				}
				if (value & 0x02) {
					DBWORD bank;

					chr.bank1k[0] = &chr.data[taitoX1017.chr[2] << 10];
					chr.bank1k[1] = &chr.data[taitoX1017.chr[3] << 10];
					chr.bank1k[2] = &chr.data[taitoX1017.chr[4] << 10];
					chr.bank1k[3] = &chr.data[taitoX1017.chr[5] << 10];
					bank = taitoX1017.chr[0] << 11;
					chr.bank1k[4] = &chr.data[bank];
					chr.bank1k[5] = &chr.data[bank | 0x0400];
					bank = taitoX1017.chr[1] << 11;
					chr.bank1k[6] = &chr.data[bank];
					chr.bank1k[7] = &chr.data[bank | 0x0400];
				} else {
					DBWORD bank;

					bank = taitoX1017.chr[0] << 11;
					chr.bank1k[0] = &chr.data[bank];
					chr.bank1k[1] = &chr.data[bank | 0x0400];
					bank = taitoX1017.chr[1] << 11;
					chr.bank1k[2] = &chr.data[bank];
					chr.bank1k[3] = &chr.data[bank | 0x0400];
					chr.bank1k[4] = &chr.data[taitoX1017.chr[2] << 10];
					chr.bank1k[5] = &chr.data[taitoX1017.chr[3] << 10];
					chr.bank1k[6] = &chr.data[taitoX1017.chr[4] << 10];
					chr.bank1k[7] = &chr.data[taitoX1017.chr[5] << 10];
				}
				taitoX1017.control = value;
			}
			return;
		case 0x7EFA:
			value >>= 2;
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 0, value);
			mapPrgRom8kUpdate();
			return;
		case 0x7EFB:
			value >>= 2;
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 1, value);
			mapPrgRom8kUpdate();
			return;
		case 0x7EFC:
			value >>= 2;
			controlBank(prgRom8kMax)
			mapPrgRom8k(1, 2, value);
			mapPrgRom8kUpdate();
			return;
	}
}
BYTE extclSaveMapper_Taito_X1017(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, taitoX1017.chr);
	savestateEle(mode, slot, taitoX1017.control);

	return (EXIT_OK);
}
