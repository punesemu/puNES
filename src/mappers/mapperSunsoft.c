/*
 * mapperSunsoft.c
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "savestate.h"

#define mirroring(data)\
	switch (data & 0x03) {\
		case 0:\
			mirroring_V();\
			break;\
		case 1:\
			mirroring_H();\
			break;\
		case 2:\
			mirroring_SCR0();\
			break;\
		case 3:\
			mirroring_SCR1();\
			break;\
		}
#define chrRom2kSwap(slot)\
{\
	DBWORD bank;\
	controlBank(chrRom2kMax)\
	bank = value << 11;\
	chr.bank1k[slot] = &chr.data[bank];\
	chr.bank1k[slot + 1] = &chr.data[bank | 0x400];\
}
#define sunsoftS4mirroring()\
	if (!s4.mode) {\
		switch (s4.mirroring & 0x03) {\
		case 0:\
			mirroring_V();\
			break;\
		case 1:\
			mirroring_H();\
			break;\
		case 2:\
			mirroring_SCR0();\
			break;\
		case 3:\
			mirroring_SCR1();\
			break;\
		}\
	} else {\
		switch (s4.mirroring & 0x03) {\
		case 0:\
			ntbl.bank1k[0] = ntbl.bank1k[2] = &chr.data[s4.chrNmt[0]];\
			ntbl.bank1k[1] = ntbl.bank1k[3] = &chr.data[s4.chrNmt[1]];\
			break;\
		case 1:\
			ntbl.bank1k[0] = ntbl.bank1k[1] = &chr.data[s4.chrNmt[0]];\
			ntbl.bank1k[2] = ntbl.bank1k[3] = &chr.data[s4.chrNmt[1]];\
			break;\
		case 2:\
			ntbl.bank1k[0] = ntbl.bank1k[1] = &chr.data[s4.chrNmt[0]];\
			ntbl.bank1k[2] = ntbl.bank1k[3] = &chr.data[s4.chrNmt[0]];\
			break;\
		case 3:\
			ntbl.bank1k[0] = ntbl.bank1k[1] = &chr.data[s4.chrNmt[1]];\
			ntbl.bank1k[2] = ntbl.bank1k[3] = &chr.data[s4.chrNmt[1]];\
			break;\
		}\
	}
#define fm7SquareTick(sq)\
	fm7.square[sq].output = 0;\
	if (--fm7.square[sq].timer == 0) {\
		fm7.square[sq].step = (fm7.square[sq].step + 1) & 0x1F;\
		fm7.square[sq].timer = fm7.square[sq].frequency + 1;\
		fm7.square[sq].clocked = TRUE;\
	}\
	if (!fm7.square[sq].disable) {\
		/*fm7.square[sq].output = -fm7.square[sq].volume * ((fm7.square[sq].step & 0x10) ? 2 : -2);*/\
		fm7.square[sq].output = fm7.square[sq].volume * ((fm7.square[sq].step & 0x10) ? 1 : 0);\
	}

WORD prgRom16kMax, prgRom8kMax, chrRom8kMax, chrRom4kMax, chrRom2kMax, chrRom1kMax;
BYTE type;

void mapInit_Sunsoft(BYTE model) {
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;
	chrRom4kMax = info.chrRom4kCount - 1;
	chrRom2kMax = (info.chrRom1kCount >> 1) - 1;
	chrRom1kMax = info.chrRom1kCount - 1;

	switch (model) {
		case SUN1:
			EXTCL_CPU_WR_MEM(Sunsoft_S1);
			info.mapperExtendWrite = TRUE;
			break;
		case SUN2A:
		case SUN2B:
			EXTCL_CPU_WR_MEM(Sunsoft_S2);
			break;
		case SUN3:
			EXTCL_CPU_WR_MEM(Sunsoft_S3);
			EXTCL_SAVE_MAPPER(Sunsoft_S3);
			EXTCL_CPU_EVERY_CYCLE(Sunsoft_S3);
			mapper.intStruct[0] = (BYTE *) &s3;
			mapper.intStructSize[0] = sizeof(s3);

			if (info.reset >= HARD) {
				memset(&s3, 0x00, sizeof(s3));
			}
			break;
		case SUN4:
			EXTCL_CPU_WR_MEM(Sunsoft_S4);
			EXTCL_SAVE_MAPPER(Sunsoft_S4);
			mapper.intStruct[0] = (BYTE *) &s4;
			mapper.intStructSize[0] = sizeof(s4);

			if (info.reset >= HARD) {
				memset(&s4, 0x00, sizeof(s4));
				s4.chrNmt[0] = 0x80 << 10;
				s4.chrNmt[1] = 0x80 << 10;
			}

			if (info.id == MAHARAJA) {
				info.prgRamPlus8kCount = 1;
				info.prgRamBatBanks = 1;
			}
			break;
		case FM7:
			EXTCL_CPU_WR_MEM(Sunsoft_FM7);
			EXTCL_CPU_RD_MEM(Sunsoft_FM7);
			EXTCL_SAVE_MAPPER(Sunsoft_FM7);
			EXTCL_CPU_EVERY_CYCLE(Sunsoft_FM7);
			EXTCL_APU_TICK(Sunsoft_FM7);
			mapper.intStruct[0] = (BYTE *) &fm7;
			mapper.intStructSize[0] = sizeof(fm7);

			if (info.reset >= HARD) {
				memset(&fm7, 0x00, sizeof(fm7));
			}

			info.prgRamPlus8kCount = 1;

			if ((info.id == BARCODEWORLD) || (info.id == DODGEDANPEI2)) {
				info.prgRamBatBanks = 1;
			}

			fm7.square[0].timer = 1;
			fm7.square[1].timer = 1;
			fm7.square[2].timer = 1;
			break;
	}

	type = model;
}

void extcl_cpu_wr_mem_Sunsoft_S1(WORD address, BYTE value) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return;
	}

	{
		const BYTE save = value;
		DBWORD bank;

		controlBankWithAND(0x0F, chrRom4kMax)
		bank = value << 12;
		chr.bank1k[0] = &chr.data[bank];
		chr.bank1k[1] = &chr.data[bank | 0x0400];
		chr.bank1k[2] = &chr.data[bank | 0x0800];
		chr.bank1k[3] = &chr.data[bank | 0x0C00];

		value = save >> 4;
		controlBank(chrRom4kMax)
		bank = value << 12;
		chr.bank1k[4] = &chr.data[bank];
		chr.bank1k[5] = &chr.data[bank | 0x0400];
		chr.bank1k[6] = &chr.data[bank | 0x0800];
		chr.bank1k[7] = &chr.data[bank | 0x0C00];

	}
}

void extcl_cpu_wr_mem_Sunsoft_S2(WORD address, BYTE value) {
	const BYTE save = value;
	DBWORD bank;

	if (type == SUN2B) {
		if (value & 0x08) {
			mirroring_SCR1();
		} else {
			mirroring_SCR0();
		}
	}

	value = (save >> 4) & 0x07;
	controlBank(prgRom16kMax)
	mapPrgRom8k(2, 0, value);
	mapPrgRom8kUpdate();

	value = ((save & 0x80) >> 4) | (save & 0x07);
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

void extcl_cpu_wr_mem_Sunsoft_S3(WORD address, BYTE value) {
	switch (address & 0xF800) {
		case 0x8800:
			chrRom2kSwap(0)
			return;
		case 0x9800:
			chrRom2kSwap(2)
			return;
		case 0xA800:
			chrRom2kSwap(4)
			return;
		case 0xB800:
			chrRom2kSwap(6)
			return;
		case 0xC000:
		case 0xC800:
			if (s3.toggle ^= 1) {
				s3.count = (s3.count & 0x00FF) | (value << 8);
			} else {
				s3.count = (s3.count & 0xFF00) | value;
			}
			return;
		case 0xD800:
			s3.toggle = 0;
			s3.enable = value & 0x10;
			irq.high &= ~EXTIRQ;
			return;
		case 0xE800:
			mirroring(value)
			return;
		case 0xF800:
			controlBank(prgRom16kMax)
			mapPrgRom8k(2, 0, value);
			mapPrgRom8kUpdate();
			return;
	}
}
BYTE extcl_save_mapper_Sunsoft_S3(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, s3.enable);
	savestateEle(mode, slot, s3.toggle);
	savestateEle(mode, slot, s3.count);
	savestateEle(mode, slot, s3.delay);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Sunsoft_S3(void) {
	if (s3.delay && !(--s3.delay)) {
		irq.high |= EXTIRQ;
	}

	if (s3.enable && s3.count && !(--s3.count)) {
		s3.enable = FALSE;
		s3.count = 0xFFFF;
		s3.delay = 1;
	}
}

void extcl_cpu_wr_mem_Sunsoft_S4(WORD address, BYTE value) {
	switch (address & 0xF000) {
		case 0x8000:
			chrRom2kSwap(0)
			return;
		case 0x9000:
			chrRom2kSwap(2)
			return;
		case 0xA000:
			chrRom2kSwap(4)
			return;
		case 0xB000:
			chrRom2kSwap(6)
			return;
		case 0xC000:
			s4.chrNmt[0] = (value | 0x80) << 10;
			sunsoftS4mirroring()
			return;
		case 0xD000:
			s4.chrNmt[1] = (value | 0x80) << 10;
			sunsoftS4mirroring()
			return;
		case 0xE000:
			s4.mode = value & 0x10;
			s4.mirroring = value & 0x03;
			sunsoftS4mirroring()
			return;
		case 0xF000:
			controlBank(prgRom16kMax)
			mapPrgRom8k(2, 0, value);
			mapPrgRom8kUpdate();
			return;
	}
}
BYTE extcl_save_mapper_Sunsoft_S4(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, s4.chrNmt);
	savestateEle(mode, slot, s4.mirroring);
	savestateEle(mode, slot, s4.mode);
	if ((mode == SSREAD) && s4.mode) {
		sunsoftS4mirroring()
	}

	return (EXIT_OK);
}

void extcl_cpu_wr_mem_Sunsoft_FM7(WORD address, BYTE value) {
	switch (address & 0xE000) {
		case 0x4000:
			if (cpu.prgRamWrActive) {
				return;
			}
			prg.ram[address & 0x1FFF] = value;
			return;
		case 0x6000:
			return;
		case 0x8000:
			fm7.address = value;
			return;
		case 0xA000: {
			const BYTE bank = fm7.address & 0x0F;

			switch (bank) {
				case 0x00:
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				case 0x06:
				case 0x07:
					controlBank(chrRom1kMax)
					chr.bank1k[bank] = &chr.data[value << 10];
					return;
				case 0x08: {
					fm7.prgRamMode = value & 0x40;
					fm7.prgRamEnable = value & 0x80;
					switch (value & 0xC0) {
						case 0x00:
						case 0x80:
							cpu.prgRamRdActive = TRUE;
							cpu.prgRamWrActive = FALSE;
							controlBankWithAND(0x3F, prgRom8kMax)
							fm7.prgRamAddress = value << 13;
							prg.ramPlus8k = &prg.rom[fm7.prgRamAddress];
							return;
						case 0x40:
							cpu.prgRamRdActive = FALSE;
							cpu.prgRamWrActive = TRUE;
							prg.ramPlus8k = &prg.ramPlus[0];
							return;
						case 0xC0:
							cpu.prgRamRdActive = TRUE;
							cpu.prgRamWrActive = TRUE;
							prg.ramPlus8k = &prg.ramPlus[0];
							return;
					}
					return;
				}
				case 0x09:
				case 0x0A:
				case 0x0B:
					controlBank(prgRom8kMax)
					mapPrgRom8k(1, (bank & 0x03) - 1, value);
					mapPrgRom8kUpdate();
					return;
				case 0x0C:
					mirroring(value)
					return;
				case 0x0D:
					fm7.irqEnableTrig = value & 0x01;
					fm7.irqEnableCount = value & 0x80;
					if (!fm7.irqEnableCount) {
						irq.high &= ~EXTIRQ;
					}
					return;
				case 0x0E:
					fm7.irqCount = (fm7.irqCount & 0xFF00) | value;
					return;
				case 0x0F:
					fm7.irqCount = (fm7.irqCount & 0x00FF) | (value << 8);
					return;
			}
			return;
		}
		case 0xC000:
			fm7.sndReg = value & 0x0F;
			return;
		case 0xE000:
			switch (fm7.sndReg) {
				case 0x00:
				case 0x02:
				case 0x04: {
					BYTE index = fm7.sndReg >> 1;
					fm7.square[index].frequency = (fm7.square[index].frequency & 0x0F00) | value;
					return;
				}
				case 0x01:
				case 0x03:
				case 0x05: {
					BYTE index = fm7.sndReg >> 1;
					fm7.square[index].frequency = (fm7.square[index].frequency & 0x00FF)
					        | ((value & 0x0F) << 8);
					return;
				}
				case 0x07:
					fm7.square[0].disable = value & 0x01;
					fm7.square[1].disable = value & 0x02;
					fm7.square[2].disable = value & 0x04;
					return;
				case 0x08:
				case 0x09:
				case 0x0A: {
					BYTE index = fm7.sndReg & 0x03;
					fm7.square[index].volume = value & 0x0F;
					return;
				}

			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_Sunsoft_FM7(WORD address, BYTE openbus, BYTE before) {
	if (fm7.prgRamEnable) {
		return (openbus);
	}

	if (address < 0x6000) {
		return (prg.ram[address & 0x1FFF]);
	}

	return (openbus);
}
BYTE extcl_save_mapper_Sunsoft_FM7(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, fm7.address);
	savestateEle(mode, slot, fm7.prgRamEnable);
	savestateEle(mode, slot, fm7.prgRamMode);
	savestateEle(mode, slot, fm7.prgRamAddress);
	if ((mode == SSREAD) && !fm7.prgRamMode) {
		prg.ramPlus8k = &prg.rom[fm7.prgRamAddress];
	}
	savestateEle(mode, slot, fm7.irqEnableTrig);
	savestateEle(mode, slot, fm7.irqEnableCount);
	savestateEle(mode, slot, fm7.irqCount);
	savestateEle(mode, slot, fm7.irqDelay);

	/*
	 * nelle versioni 1 e 2 dei files di save non salvavo
	 * i dati delle snd square perche' non avevo ancora
	 * implementato la loro emulazione.
	 */
	if (savestate.version > 2) {
		BYTE i;

		for (i = 0; i < LENGTH(fm7.square); i++) {
			savestateEle(mode, slot, fm7.square[i].disable);
			savestateEle(mode, slot, fm7.square[i].step);
			savestateEle(mode, slot, fm7.square[i].frequency);
			savestateEle(mode, slot, fm7.square[i].timer);
			savestateEle(mode, slot, fm7.square[i].volume);
			savestateEle(mode, slot, fm7.square[i].output);
		}
	}
	return (EXIT_OK);
}
void extcl_cpu_every_cycle_Sunsoft_FM7(void) {
	if (fm7.irqDelay && !(--fm7.irqDelay)) {
		irq.high |= EXTIRQ;
	}

	if (!fm7.irqEnableCount) {
		return;
	}

	if (!(--fm7.irqCount) && fm7.irqEnableTrig) {
		fm7.irqDelay = 1;
	}
}
void extcl_apu_tick_Sunsoft_FM7(void) {
	fm7SquareTick(0)
	fm7SquareTick(1)
	fm7SquareTick(2)
}
