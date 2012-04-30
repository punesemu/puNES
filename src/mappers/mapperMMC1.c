/*
 * mapperMMC1.c
 *
 *  Created on: 4/lug/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "cpu6502.h"
#include "savestate.h"

static void INLINE ctrlReg_MMC1(void);
static void INLINE swapPrgRom_MMC1(void);
static void INLINE swapChr0_MMC1(void);
static void INLINE swapChr1_MMC1(void);

enum {
	CTRL = 0,
	CHR0 = 1,
	CHR1 = 2,
	PRG0 = 3
};

#define chrReg(reg)\
	value = reg;\
	switch (info.mapperType) {\
		case SNROM:\
			/*\
			 * 4bit0\
			 * -----\
			 * ExxxC\
			 * |   |\
			 * |   +- Select 4 KB CHR RAM bank at PPU $0000 (ignored in 8 KB mode)\
			 * +----- PRG RAM disable (0: enable, 1: open bus)\
			 */\
			cpu.prgRamRdActive = (reg & 0x10 ? FALSE : TRUE);\
			cpu.prgRamWrActive = cpu.prgRamRdActive;\
			value &= 0x01;\
			break;\
		case SOROM: {\
			BYTE bankPrgRam = (reg & 0x08) >> 3;\
			prg.ramPlus8k = &prg.ramPlus[bankPrgRam * 0x2000];\
			mmc1.prgUpper = reg & 0x10;\
			value &= 0x01;\
			break;\
		}\
		case SUROM:\
			mmc1.prgUpper = reg & 0x10;\
			value &= 0x01;\
			break;\
		case SXROM: {\
			BYTE bankPrgRam = (reg & 0x0C) >> 2;\
			prg.ramPlus8k = &prg.ramPlus[bankPrgRam * 0x2000];\
			mmc1.prgUpper = reg & 0x10;\
			value &= 0x01;\
			break;\
		}\
		default:\
			value &= 0x1F;\
			break;\
	}

WORD prgRom16kMax, chrRom8kMax, chrRom4kMax;

void mapInit_MMC1(void) {
	prgRom16kMax = info.prgRom16kCount - 1;
	chrRom8kMax = info.chrRom8kCount - 1;
	chrRom4kMax = info.chrRom4kCount - 1;

	EXTCLCPUWRMEM(MMC1);
	EXTCLSAVEMAPPER(MMC1);
	mapper.intStruct[0] = (BYTE *) &mmc1;
	mapper.intStructSize[0] = sizeof(mmc1);

	if (info.reset >= HARD) {
		memset(&mmc1, 0x00, sizeof(mmc1));
		mmc1.ctrl = 0x0C;
		mmc1.prgMode = 3;
		mmc1.chr1 = 1;
	}

	switch (info.mapperType) {
		case SNROM:
			/* SUROM usa 8k di PRG Ram */
			info.prgRamPlus8kCount = 1;
			break;
		case SOROM:
			/* SOROM usa 16k di PRG Ram */
			info.prgRamPlus8kCount = 2;
			break;
		case SXROM:
			/* SXROM usa 32k di PRG Ram */
			info.prgRamPlus8kCount = 4;
			break;
	}
}
void extclCpuWrMem_MMC1(WORD address, BYTE value) {
	/*
	 * se nel tick precedente e' stato fatto un reset e
	 * sono in presenza di una doppia scrittura da parte
	 * di un'istruzione (tipo l'INC), allora l'MMC1 non
	 * la considera. Roms interessate:
	 * Advanced Dungeons & Dragons - Hillsfar
	 * Bill & Ted's Excellent Video Game Adventure
	 * Snow Brothers
	 */
	if (mmc1.reset) {
		/* azzero il flag */
		mmc1.reset = FALSE;
		/* esco se necessario */
		if (cpu.dblWr) {
			return;
		}
	}
	/*
	 * A program's reset code will reset the mapper
	 * first by writing a value of $80 through $FF
	 * to any address in $8000-$FFFF.
	 */
	if (value & 0x80) {
		/* indico che e' stato fatto un reset */
		mmc1.reset = TRUE;
		/* azzero posizione e registro temporaneo */
		mmc1.pos = mmc1.reg = 0;
		/*
		 * reset shift register and write
		 * Control with (Control OR $0C).
		 */
		mmc1.ctrl |= 0x0C;
		/* reinizializzo tutto */
		ctrlReg_MMC1();
		/*
		 * locking PRG ROM at $C000-$FFFF
		 * to the last 16k bank.
		 */
		mapPrgRom8k(2, 2, mmc1.prgUpper | (prgRom16kMax & 0x0F));
		mapPrgRom8kUpdate();
		return;
	}

	mmc1.reg |= ((value & 0x01) << mmc1.pos);

	if (mmc1.pos++ == 4) {
		BYTE reg = (address >> 13) & 0x03;

		switch (reg) {
			case CTRL:
				mmc1.ctrl = mmc1.reg;
				ctrlReg_MMC1();
				break;
			case CHR0:
				mmc1.chr0 = mmc1.reg;
				swapChr0_MMC1();
				break;
			case CHR1:
				mmc1.chr1 = mmc1.reg;
				swapChr1_MMC1();
				break;
			case PRG0:
				mmc1.prg0 = mmc1.reg;
				cpu.prgRamRdActive = (reg & 0x10 ? FALSE : TRUE);
				cpu.prgRamWrActive = cpu.prgRamRdActive;
				break;
		}
		swapPrgRom_MMC1();
		/* azzero posizione e registro temporaneo */
		mmc1.pos = mmc1.reg = 0;
	}
}
BYTE extclSaveMapper_MMC1(BYTE mode, BYTE slot, FILE *fp) {
	savestateEle(mode, slot, mmc1.reg);
	savestateEle(mode, slot, mmc1.pos);
	savestateEle(mode, slot, mmc1.prgMode);
	savestateEle(mode, slot, mmc1.chrMode);
	savestateEle(mode, slot, mmc1.ctrl);
	savestateEle(mode, slot, mmc1.chr0);
	savestateEle(mode, slot, mmc1.chr1);
	savestateEle(mode, slot, mmc1.prg0);
	savestateEle(mode, slot, mmc1.reset);
	savestateEle(mode, slot, mmc1.prgUpper);
	return (EXIT_OK);
}

static void INLINE ctrlReg_MMC1(void) {
	mmc1.prgMode = (mmc1.ctrl & 0x0C) >> 2;
	mmc1.chrMode = (mmc1.ctrl & 0x10) >> 4;
	switch (mmc1.ctrl & 0x03) {
		case 0x00:
			mirroring_SCR0();
			break;
		case 0x01:
			mirroring_SCR1();
			break;
		case 0x02:
			mirroring_V();
			break;
		case 0x03:
			mirroring_H();
			break;
	}
	swapChr0_MMC1();
	swapChr1_MMC1();
}
static void INLINE swapPrgRom_MMC1(void) {
	BYTE value = mmc1.prg0;

	switch (mmc1.prgMode) {
		case 0:
		case 1: {
			BYTE bank;
			controlBankWithAND(0x0E, prgRom16kMax)
			bank = mmc1.prgUpper | value;
			/* switch 32k at $8000, ignoring low bit of bank number */
			mapPrgRom8k(2, 0, bank);
			mapPrgRom8k(2, 2, bank + 1);
			break;
		}
		case 2:
			controlBankWithAND(0x0F, prgRom16kMax)
			/* fix first 16k bank at $8000 and switch 16 KB bank at $C000 */
			mapPrgRom8k(2, 0, mmc1.prgUpper);
			mapPrgRom8k(2, 2, mmc1.prgUpper | value);
			break;
		case 3:
			controlBankWithAND(0x0F, prgRom16kMax)
			/* fix last 16k bank at $C000 and switch 16 KB bank at $8000 */
			mapPrgRom8k(2, 0, mmc1.prgUpper | value);
			mapPrgRom8k(2, 2, mmc1.prgUpper | (prgRom16kMax & 0x0F));
			break;
	}
	mapPrgRom8kUpdate();
}
static void INLINE swapChr0_MMC1(void) {
	DBWORD value;

	chrReg(mmc1.chr0)

	/* 4k mode */
	if (mmc1.chrMode) {
		controlBank(chrRom4kMax)
		value <<= 12;
		chr.bank1k[0] = &chr.data[value];
		chr.bank1k[1] = &chr.data[value | 0x0400];
		chr.bank1k[2] = &chr.data[value | 0x0800];
		chr.bank1k[3] = &chr.data[value | 0x0C00];
		return;
	}

	/* 8k mode */

	/*
	 * se ho solo della CHR ram allora
	 * non posso switchare niente.
	 */
	if (mapper.writeVRAM) {
		return;
	}

	controlBankWithAND(0x1E, chrRom4kMax)
	value <<= 12;
	chr.bank1k[0] = &chr.data[value];
	chr.bank1k[1] = &chr.data[value | 0x0400];
	chr.bank1k[2] = &chr.data[value | 0x0800];
	chr.bank1k[3] = &chr.data[value | 0x0C00];
	chr.bank1k[4] = &chr.data[value | 0x1000];
	chr.bank1k[5] = &chr.data[value | 0x1400];
	chr.bank1k[6] = &chr.data[value | 0x1800];
	chr.bank1k[7] = &chr.data[value | 0x1C00];
}
static void INLINE swapChr1_MMC1(void) {
	if (mmc1.chrMode) {
		DBWORD value;

		chrReg(mmc1.chr1)

		controlBank(chrRom4kMax)
		value <<= 12;
		chr.bank1k[4] = &chr.data[value];
		chr.bank1k[5] = &chr.data[value | 0x0400];
		chr.bank1k[6] = &chr.data[value | 0x0800];
		chr.bank1k[7] = &chr.data[value | 0x0C00];
	}
}
