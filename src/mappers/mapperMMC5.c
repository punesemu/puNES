/*
 * mapperMMC5.c
 *
 *  Created on: 26/ago/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "apu.h"
#include "ppu.h"
#include "cpu.h"
#include "irql2f.h"
#include "savestate.h"

/* PRG */
#define prg8kUpdate(slot)\
	value = mmc5.prgBank[slot];\
	if (value & 0x80) {\
		/* modalita' rom */\
		mmc5.prgRamBank[slot][0] = FALSE;\
		controlBankWithAND(0x7F, prgRom8kMax)\
		mapPrgRom8k(1, slot, value);\
	} else {\
		/* modalita' ram */\
		BYTE prgRamBank;\
		prgRamBank = prgRamAccess[prgRamMode][value & 0x07];\
		if (prgRamBank != INVALID) {\
			mmc5.prgRamBank[slot][0] = TRUE;\
			mmc5.prgRamBank[slot][1] = prgRamBank << 13;\
		}\
	}
#define prg16kUpdate()\
	value = mmc5.prgBank[1];\
	if (value & 0x80) {\
		/* modalita' rom */\
		mmc5.prgRamBank[0][0] = mmc5.prgRamBank[1][0] = FALSE;\
		value = (value & 0x7F) >> 1;\
		controlBank(prgRom16kMax)\
		mapPrgRom8k(2, 0, value);\
	} else {\
		/* modalita' ram */\
		BYTE prgRamBank;\
		prgRamBank = prgRamAccess[prgRamMode][value & 0x07];\
		if (prgRamBank != INVALID) {\
			mmc5.prgRamBank[0][0] = TRUE;\
			mmc5.prgRamBank[0][1] = (value & 0x06) << 13;\
		}\
		prgRamBank = prgRamAccess[prgRamMode][(value + 1) & 0x07];\
		if (prgRamBank != INVALID) {\
			mmc5.prgRamBank[1][0] = TRUE;\
			mmc5.prgRamBank[1][1] = (value & 0x07) << 13;\
		}\
	}
#define prgRom8kUpdate()\
	value = mmc5.prgBank[3];\
	mmc5.prgRamBank[3][0] = FALSE;\
	controlBankWithAND(0x7F, prgRom8kMax)\
	mapPrgRom8k(1, 3, value)
#define prgRom16kUpdate()\
	value = (mmc5.prgBank[3] & 0x7F) >> 1;\
	mmc5.prgRamBank[2][0] = mmc5.prgRamBank[3][0] = FALSE;\
	controlBank(prgRom16kMax)\
	mapPrgRom8k(2, 2, value)
#define prgRom32kUpdate()\
	value = (mmc5.prgBank[3] & 0x7F) >> 2;\
	mmc5.prgRamBank[0][0] = mmc5.prgRamBank[1][0] = FALSE;\
	mmc5.prgRamBank[2][0] = mmc5.prgRamBank[3][0] = FALSE;\
	controlBank(prgRom32kMax)\
	mapPrgRom8k(4, 0, value)
#define prgRamUseBank(slot)\
	prg.rom8k[slot] = &prg.ramPlus[mmc5.prgRamBank[slot][1]]
/* CHR */
#define chr8kUpdate(chrType, slot)\
	value = mmc5.chrType[slot];\
	controlBankWithAND(0x03FF, chrRom8kMax)\
	value <<= 13;\
	chr.bank1k[0] = &chr.data[value];\
	chr.bank1k[1] = &chr.data[value | 0x0400];\
	chr.bank1k[2] = &chr.data[value | 0x0800];\
	chr.bank1k[3] = &chr.data[value | 0x0C00];\
	chr.bank1k[4] = &chr.data[value | 0x1000];\
	chr.bank1k[5] = &chr.data[value | 0x1400];\
	chr.bank1k[6] = &chr.data[value | 0x1800];\
	chr.bank1k[7] = &chr.data[value | 0x1C00]
#define chr4kUpdate(chrType, slot, base)\
	value = mmc5.chrType[slot];\
	controlBankWithAND(0x03FF, chrRom4kMax)\
	value <<= 12;\
	chr.bank1k[base | 0] = &chr.data[value];\
	chr.bank1k[base | 1] = &chr.data[value | 0x0400];\
	chr.bank1k[base | 2] = &chr.data[value | 0x0800];\
	chr.bank1k[base | 3] = &chr.data[value | 0x0C00]
#define chr2kUpdate(chrType, slot, base)\
	value = mmc5.chrType[slot];\
	controlBankWithAND(0x03FF, chrRom2kMax)\
	value <<= 11;\
	chr.bank1k[base | 0] = &chr.data[value];\
	chr.bank1k[base | 1] = &chr.data[value | 0x0400]
#define chr1kUpdate(chrType, slot, base)\
	value = mmc5.chrType[slot];\
	controlBankWithAND(0x03FF, chrRom1kMax)\
	value <<= 10;\
	chr.bank1k[base] = &chr.data[value]
/* nametables **/
#define nmtUpdate(bits, slot)\
{\
	BYTE mode = (value >> bits) & 0x03;\
	mmc5.nmtMode[slot] = mode;\
	_nmtUpdate(slot)\
}
#define _nmtUpdate(slot)\
	switch (mode) {\
		case MODE0:\
			ntbl.bank1k[slot] = &ntbl.data[0];\
			break;\
		case MODE1:\
			ntbl.bank1k[slot] = &ntbl.data[0x400];\
			break;\
		case MODE2:\
			ntbl.bank1k[slot] = &mmc5.extRam[0];\
			break;\
		case MODE3:\
			ntbl.bank1k[slot] = &mmc5.fillTable[0];\
			break;\
	}

void prgSwap(void);
void useChrS(void);
void useChrB(void);

enum {
	PRGRAMNONE,
	PRGRAM8K,
	PRGRAM16K,
	PRGRAM32K,
	PRGRAM40K,
	PRGRAM64K,
	INVALID,
	X = INVALID
};
enum {
	MODE0, MODE1, MODE2, MODE3
};
enum {
	CHRS, CHRB
};
enum {
	SPLIT_LEFT, SPLIT_RIGHT = 0x40
};

const BYTE fillerAttrib[4] = {0x00, 0x55, 0xAA, 0xFF};
WORD prgRom32kMax, prgRom16kMax, prgRom8kMax;
WORD chrRom8kMax, chrRom4kMax, chrRom2kMax, chrRom1kMax;
BYTE prgRamMode;

static const BYTE prgRamAccess[6][8] = {
	{X,X,X,X,X,X,X,X},
	{0,0,0,0,X,X,X,X},
	{0,0,0,0,1,1,1,1},
	{0,1,2,3,X,X,X,X},
	{0,1,2,3,4,4,4,4},
	{0,1,2,3,4,5,6,7}
};

void mapInit_MMC5(void) {
	prgRom32kMax = (info.prgRom16kCount >> 1) - 1;
	prgRom16kMax = info.prgRom16kCount - 1;
	prgRom8kMax = info.prgRom8kCount - 1;

	chrRom8kMax = info.chrRom8kCount - 1;
	chrRom4kMax = info.chrRom4kCount - 1;
	chrRom2kMax = (info.chrRom4kCount << 1) - 1;
	chrRom1kMax = (info.chrRom4kCount << 2) - 1;

	EXTCL_CPU_WR_MEM(MMC5);
	EXTCL_CPU_RD_MEM(MMC5);
	EXTCL_SAVE_MAPPER(MMC5);
	EXTCL_PPU_256_TO_319(MMC5);
	EXTCL_PPU_320_TO_34X(MMC5);
	EXTCL_AFTER_RD_CHR(MMC5);
	EXTCL_RD_NMT(MMC5);
	EXTCL_RD_CHR(MMC5);
	EXTCL_LENGTH_CLOCK(MMC5);
	EXTCL_ENVELOPE_CLOCK(MMC5);
	EXTCL_APU_TICK(MMC5);
	mapper.intStruct[0] = (BYTE *) &mmc5;
	mapper.intStructSize[0] = sizeof(mmc5);

	if (info.reset >= HARD) {
		BYTE i;

		memset(&mmc5, 0x00, sizeof(mmc5));
		memset(&irql2f, 0x00, sizeof(irql2f));
		mmc5.prgMode = MODE3;
		mmc5.chrMode = MODE0;
		mmc5.extMode = MODE0;
		mmc5.chrLast = CHRS;

		mmc5.S3.frequency = 1;
		mmc5.S4.frequency = 1;

		irql2f.scanline = 255;
		irql2f.frameX = 339;

		for (i = 0; i < 4; ++i) {
			mmc5.prgBank[i] = 0xFF;
		}

		for (i = 0; i < 8; ++i) {
			mmc5.chrS[i] = i;
		}

		for (i = 0; i < 4; ++i) {
			mmc5.chrB[i] = i;
		}

		useChrS();
	} else {
		mmc5.S3.length.enabled = 0;
		mmc5.S3.length.value = 0;
		mmc5.S4.length.enabled = 0;
		mmc5.S4.length.value = 0;
	}

	info.mapperExtendWrite = TRUE;
	irql2f.present = TRUE;

	switch (info.mapperType) {
		case EKROM:
			info.prgRamPlus8kCount = 1;
			info.prgRamBatBanks = 1;
			prgRamMode = PRGRAM8K;
			break;
		default:
		case ELROM:
			info.prgRamPlus8kCount = FALSE;
			info.prgRamBatBanks = FALSE;
			prgRamMode = PRGRAMNONE;
			break;
		case ETROM:
			info.prgRamPlus8kCount = 2;
			info.prgRamBatBanks = 1;
			info.prgRamBatStart = 0;
			prgRamMode = PRGRAM16K;
			break;
		case EWROM:
			info.prgRamPlus8kCount = 4;
			info.prgRamBatBanks = 4;
			prgRamMode = PRGRAM32K;
			break;
	}
}
void extcl_cpu_wr_mem_MMC5(WORD address, BYTE value) {
	if (address < 0x5000) {
		return;
	}

	switch (address) {
		case 0x5000:
			square_reg0(mmc5.S3);
			return;
		case 0x5001:
			/* lo sweep non e' utilizzato */
			return;
		case 0x5002:
			square_reg2(mmc5.S3);
			return;
		case 0x5003:
			square_reg3(mmc5.S3);
			return;
		case 0x5004:
			square_reg0(mmc5.S4);
			return;
		case 0x5005:
			/* lo sweep non e' utilizzato */
			return;
		case 0x5006:
			square_reg2(mmc5.S4);
			return;
		case 0x5007:
			square_reg3(mmc5.S4);
			return;
		case 0x5010:
			mmc5.pcm.enabled = ~value & 0x01;
			mmc5.pcm.output = 0;
			if (mmc5.pcm.enabled) {
				mmc5.pcm.output = mmc5.pcm.amp;
			}
			mmc5.pcm.clocked = TRUE;
			return;
		case 0x5011:
			mmc5.pcm.amp = value;
			mmc5.pcm.output = 0;
			if (mmc5.pcm.enabled) {
				mmc5.pcm.output = mmc5.pcm.amp;
			}
			mmc5.pcm.clocked = TRUE;
			return;
		case 0x5015:
			if (!(mmc5.S3.length.enabled = value & 0x01)) {
				mmc5.S3.length.value = 0;
			}
			if (!(mmc5.S4.length.enabled = value & 0x02)) {
				mmc5.S4.length.value = 0;
			}
			return;
		case 0x5100:
			value &= 0x03;
			if (value != mmc5.prgMode) {
				mmc5.prgMode = value;
				prgSwap();
			}
			return;
		case 0x5101:
			value &= 0x03;
			if (value != mmc5.chrMode) {
				mmc5.chrMode = value;
				if ((r2000.sizeSPR != 16) || !r2001.visible || r2002.vblank) {
					if (mmc5.chrLast == CHRS) {
						useChrS();
					} else {
						useChrB();
					}
				}
			}
			return;
		case 0x5102:
		case 0x5103:
			mmc5.prgRamWrite[address & 0x0001] = value & 0x03;
			if ((mmc5.prgRamWrite[0] == 0x02) && (mmc5.prgRamWrite[1] == 0x01)) {
				cpu.prg_ram_wr_active = TRUE;
			} else {
				cpu.prg_ram_wr_active = FALSE;
			}
			return;
		case 0x5104:
			mmc5.extMode = value & 0x03;
			return;
		case 0x5105:
			nmtUpdate(0, 0)
			nmtUpdate(2, 1)
			nmtUpdate(4, 2)
			nmtUpdate(6, 3)
			return;
			/* --------------------------------- PRG bankswitching ---------------------------------*/
		case 0x5106:
			mmc5.fillTile = value;
			memset(&mmc5.fillTable[0], mmc5.fillTile, 0x3C0);
			memset(&mmc5.fillTable[0x3C0], fillerAttrib[mmc5.fillAttr], 0x40);
			return;
		case 0x5107:
			mmc5.fillAttr = value & 0x03;
			memset(&mmc5.fillTable[0x3C0], fillerAttrib[mmc5.fillAttr], 0x40);
			return;
		case 0x5113: {
			BYTE bank;
			bank = prgRamAccess[prgRamMode][value & 0x07];
			if (bank != INVALID) {
				prg.ramPlus8k = &prg.ramPlus[bank * 0x2000];
			}
			return;
		}
		case 0x5114:
		case 0x5115:
		case 0x5116:
		case 0x5117:
			address &= 0x0003;
			if (mmc5.prgBank[address] != value) {
				mmc5.prgBank[address] = value;
				prgSwap();
			}
			return;
/* --------------------------------- CHR bankswitching ---------------------------------*/
		case 0x5120:
		case 0x5121:
		case 0x5122:
		case 0x5123:
		case 0x5124:
		case 0x5125:
		case 0x5126:
		case 0x5127: {
			WORD bank = value | (mmc5.chrHigh << 2);
			address &= 0x0007;

			if ((mmc5.chrLast != CHRS) || (mmc5.chrS[address] != bank)) {
				mmc5.chrS[address] = bank;
				mmc5.chrLast = CHRS;
				if ((r2000.sizeSPR != 16) || !r2001.visible || r2002.vblank) {
					useChrS();
				}
			}
			return;
		}
		case 0x5128:
		case 0x5129:
		case 0x512A:
		case 0x512B: {
			WORD bank = value | (mmc5.chrHigh << 2);
			address &= 0x0003;

			if ((mmc5.chrLast != CHRB) || (mmc5.chrB[address] != bank)) {
				mmc5.chrB[address] = bank;
				mmc5.chrLast = CHRB;
				if ((r2000.sizeSPR != 16) || !r2001.visible || r2002.vblank) {
					useChrB();
				}
			}
			return;
		}
		case 0x5130:
			mmc5.chrHigh = (value & 0x03) << 6;
			return;
		case 0x5200:
			mmc5.split = value & 0x80;
			mmc5.splitSide = value & 0x40;
			mmc5.splitStTile = value & 0x1F;
			return;
		case 0x5201:
			if (value >= 240) {
				value -= 16;
			}
			mmc5.splitScrl = value;
			return;
		case 0x5202:
			controlBank(chrRom4kMax)
			mmc5.splitBank = value << 12;
			return;
		case 0x5203:
			irql2f.scanline = value;
			return;
		case 0x5204:
			if (value & 0x80) {
				irql2f.enable = TRUE;
				return;
			}
			irql2f.enable = FALSE;
			/* disabilito l'IRQ dell'MMC5 */
			irq.high &= ~EXTIRQ;
			return;
		case 0x5205:
			mmc5.factor[0] = value;
			mmc5.product = mmc5.factor[0] * mmc5.factor[1];
			return;
		case 0x5206:
			mmc5.factor[1] = value;
			mmc5.product = mmc5.factor[0] * mmc5.factor[1];
			return;
		default:
			if ((address >= 0x5C00) && (address < 0x6000)) {
				address &= 0x03FF;
				if (mmc5.extMode < MODE2) {
					if (!r2002.vblank && r2001.visible && (ppu.screenY < SCRLINES)) {
						mmc5.extRam[address] = value;
					} else {
						mmc5.extRam[address] = 0;
					}
					return;
				}
				if (mmc5.extMode == MODE2) {
					mmc5.extRam[address] = value;
					return;
				}
				return;
			}
			/*
			 * posso memorizzare nella PRG rom solo se il banco in cui
			 * la rom vuole scrivere punta ad un banco di PRG ram.
			 */
			if (address >= 0x8000) {
				BYTE index = ((address >> 13) & 0x03);
				if (mmc5.prgRamBank[index][0] && cpu.prg_ram_wr_active) {
					prg.rom8k[index][address & 0x1FFF] = value;
				}
			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_MMC5(WORD address, BYTE openbus, BYTE before) {
	BYTE value;

	switch (address) {
		case 0x5015:
			/* azzero la varibile d'uscita */
			value = 0;
			/*
			 * per ogni canale controllo se il length counter
			 * non e' a 0 e se si setto a 1 il bit corrispondente
			 * nel byte di ritorno.
			 */
			if (mmc5.S3.length.value) {
				value |= 0x01;
			}
			if (mmc5.S4.length.value) {
				value |= 0x02;
			}
			return (value);
		case 0x5204:
			value = irql2f.pending | irql2f.inFrame;
			irql2f.pending = FALSE;
			/* disabilito l'IRQ dell'MMC5 */
			irq.high &= ~EXTIRQ;
			return (value);
		case 0x5205:
			return (mmc5.product & 0x00FF);
		case 0x5206:
			return ((mmc5.product & 0xFF00) >> 8);
		default:
			if ((address < 0x5C00) || (address >= 0x6000)) {
				return (openbus);
			}
			if (mmc5.extMode < MODE2) {
				return (openbus);
			}
			if (mmc5.extMode == MODE2) {
				return (mmc5.extRam[address & 0x03FF]);
			}
			return (mmc5.fillTable[address & 0x03FF]);
	}
}
BYTE extcl_save_mapper_MMC5(BYTE mode, BYTE slot, FILE *fp) {
	BYTE i;

	savestateEle(mode, slot, mmc5.prgMode);
	savestateEle(mode, slot, mmc5.chrMode);
	savestateEle(mode, slot, mmc5.extMode);
	savestateEle(mode, slot, mmc5.nmtMode);
	if (mode == SSREAD) {
		for (i = 0; i < LENGTH(mmc5.nmtMode); i++) {
			BYTE mode = mmc5.nmtMode[i];
			_nmtUpdate(i)
		}
	}
	savestateEle(mode, slot, mmc5.prgRamWrite);
	savestateEle(mode, slot, mmc5.prgBank);
	for (i = 0; i < LENGTH(mmc5.prgRamBank); i++) {
		savestateInt(mode, slot, mmc5.prgRamBank[i][0]);
		savestateInt(mode, slot, mmc5.prgRamBank[i][1]);
		if ((mode == SSREAD) && (mmc5.prgRamBank[i][0])) {
			prgRamUseBank(i);
		}
	}
	savestateEle(mode, slot, mmc5.chrLast);
	savestateEle(mode, slot, mmc5.chrHigh);
	savestateEle(mode, slot, mmc5.chrS);
	savestateEle(mode, slot, mmc5.chrB);
	savestateEle(mode, slot, mmc5.extRam);
	savestateEle(mode, slot, mmc5.fillTile);
	savestateEle(mode, slot, mmc5.fillAttr);
	if (mode == SSREAD) {
		memset(&mmc5.fillTable[0], mmc5.fillTile, 0x3C0);
		memset(&mmc5.fillTable[0x3C0], fillerAttrib[mmc5.fillAttr], 0x40);
	}
	savestateEle(mode, slot, mmc5.split);
	savestateEle(mode, slot, mmc5.splitStTile);
	savestateEle(mode, slot, mmc5.splitSide);
	savestateEle(mode, slot, mmc5.splitScrl);
	savestateEle(mode, slot, mmc5.splitInReg);
	savestateEle(mode, slot, mmc5.splitX);
	savestateEle(mode, slot, mmc5.splitY);
	savestateEle(mode, slot, mmc5.splitTile);
	savestateEle(mode, slot, mmc5.splitBank);
	savestateEle(mode, slot, mmc5.factor);
	savestateEle(mode, slot, mmc5.product);

	savestateSquare(mmc5.S3, slot);
	savestateSquare(mmc5.S4, slot);

	savestateEle(mode, slot, mmc5.pcm.enabled);
	savestateEle(mode, slot, mmc5.pcm.output);
	savestateEle(mode, slot, mmc5.pcm.amp);

	savestateEle(mode, slot, mmc5.filler);

	return (EXIT_OK);
}
void extcl_ppu_256_to_319_MMC5(void) {
	if (ppu.frameX != 256) {
		return;
	};

	if ((mmc5.chrLast == CHRS) || (r2000.sizeSPR == 16)) {
		useChrS();
	} else {
		useChrB();
	}
}
void extcl_ppu_320_to_34x_MMC5(void) {
	irql2f_tick();

	if (ppu.frameX != 320) {
		return;
	};

	if (mmc5.split) {
		mmc5.splitX = 0x1F;
		if (ppu.screenY == SCRLINES - 1) {
			mmc5.splitY = mmc5.splitScrl - 1;
		} else {
			if (mmc5.splitY < 239) {
				mmc5.splitY++;
			} else {
				mmc5.splitY = 0;
			}
		}
	}

	if ((mmc5.chrLast == CHRB) || (r2000.sizeSPR == 16)) {
		useChrB();
	} else {
		useChrS();
	}
}
void extcl_after_rd_chr_MMC5(WORD address) {
	/*
	 * dopo ogni fetch del high byte del background
	 * azzero il flag con cui indico se il tile era
	 * nella regione dello split (questo indipendentemente
	 * che lo fosse realmente o meno). In questo modo sono
	 * che non rimanga settato quando in realta' non serve.
	 */
	mmc5.splitInReg = FALSE;
}
BYTE extcl_rd_chr_MMC5(WORD address) {
	BYTE value;
	uint32_t index;

	/* se non sto trattando il background esco */
	if ((address & 0xFFF7) != ppu.bckAdr) {
		return (chr.bank1k[address >> 10][address & 0x3FF]);
	}

	/* sono nella regione di split? */
	if (mmc5.split && mmc5.splitInReg) {
		return (chr.data[mmc5.splitBank + (address & 0x0FFF)]);
	}

	/* se non sono nella modalita' 1 esco normalmente */
	if (mmc5.extMode != MODE1) {
		return (chr.bank1k[address >> 10][address & 0x3FF]);
	}

	value = (mmc5.extRam[r2006.value & 0x03FF] & 0x3F);
	controlBank(chrRom4kMax)
	index = ((value + mmc5.chrHigh) << 12) + (address & 0x0FFF);
	return (chr.data[index]);
}
BYTE extcl_rd_nmt_MMC5(WORD address) {
	BYTE nmt = address >> 10;

	if ((ntbl.bank1k[nmt] == mmc5.extRam) && (mmc5.extMode > MODE1)) {
		return (0);
	}

	if (mmc5.split) {
		WORD adr = (address & 0x03FF);
		/* attributi */
		if (adr >= 0x03C0) {
			mmc5.splitX = (mmc5.splitX + 1) & 0x1F;

			mmc5.splitInReg = FALSE;

			if (((mmc5.splitSide == SPLIT_LEFT) && (mmc5.splitX <= mmc5.splitStTile))
			        || ((mmc5.splitSide == SPLIT_RIGHT) && (mmc5.splitX >= mmc5.splitStTile))) {

				mmc5.splitTile = ((mmc5.splitY & 0xF8) << 2) | mmc5.splitX;

				mmc5.splitInReg = TRUE;

				return (fillerAttrib[(mmc5.extRam[0x3C0 | (mmc5.splitTile >> 4 & 0x38)
				        | (mmc5.splitTile >> 2 & 0x7)]
				        >> ((mmc5.splitTile >> 4 & 0x4) | (mmc5.splitTile & 0x2))) & 0x3]);
			}
		}
		/* tile */
		if (mmc5.splitInReg) {
			return (mmc5.extRam[mmc5.splitTile]);
		}
	}

	if (mmc5.extMode != MODE1) {
		return (ntbl.bank1k[nmt][address & 0x3FF]);
	}

	if ((address & 0x03FF) >= 0x03C0) {
		BYTE shiftAT = (((r2006.value & 0x40) >> 4) | (r2006.value & 0x02));
		return (((mmc5.extRam[r2006.value & 0x03FF] & 0xC0) >> 6) << shiftAT);
	}
	return (ntbl.bank1k[nmt][address & 0x3FF]);
}
void extcl_length_clock_MMC5(void) {
	length_run(mmc5.S3)
	length_run(mmc5.S4)
}
void extcl_envelope_clock_MMC5(void) {
	envelope_run(mmc5.S3)
	envelope_run(mmc5.S4)
}
void extcl_apu_tick_MMC5(void) {
	square_tick(mmc5.S3, 0)
	square_tick(mmc5.S4, 0)
}

void prgSwap(void) {
	BYTE value, i;

	switch (mmc5.prgMode) {
		case MODE0:
			if (prgRom32kMax == 0xFFFF) {
				break;
			}
			prgRom32kUpdate();
			break;
		case MODE1:
			if (prgRom16kMax == 0xFFFF) {
				break;
			}
			prg16kUpdate()
			prgRom16kUpdate();
			break;
		case MODE2:
			if (prgRom16kMax != 0xFFFF) {
				prg16kUpdate()
			}
			prg8kUpdate(2)
			prgRom8kUpdate();
			break;
		case MODE3:
			prg8kUpdate(0)
			prg8kUpdate(1)
			prg8kUpdate(2)
			prgRom8kUpdate();
			break;
	}

	mapPrgRom8kUpdate();

	for (i = 0; i < 4; i++) {
		if (mmc5.prgRamBank[i][0]) {
			prgRamUseBank(i);
		} else {
			mmc5.prgRamBank[i][1] = 0;
		}
	}
}
void useChrS(void) {
	DBWORD value;

	switch (mmc5.chrMode) {
		case MODE0:
			chr8kUpdate(chrS, 7);
			break;
		case MODE1:
			chr4kUpdate(chrS, 3, 0);
			chr4kUpdate(chrS, 7, 4);
			break;
		case MODE2:
			chr2kUpdate(chrS, 1, 0);
			chr2kUpdate(chrS, 3, 2);
			chr2kUpdate(chrS, 5, 4);
			chr2kUpdate(chrS, 7, 6);
			break;
		case MODE3:
			chr1kUpdate(chrS, 0, 0);
			chr1kUpdate(chrS, 1, 1);
			chr1kUpdate(chrS, 2, 2);
			chr1kUpdate(chrS, 3, 3);
			chr1kUpdate(chrS, 4, 4);
			chr1kUpdate(chrS, 5, 5);
			chr1kUpdate(chrS, 6, 6);
			chr1kUpdate(chrS, 7, 7);
			break;
	}
}
void useChrB(void) {
	DBWORD value;

	switch (mmc5.chrMode) {
		case MODE0:
			chr8kUpdate(chrB, 3);
			break;
		case MODE1:
			chr4kUpdate(chrB, 3, 0);
			chr4kUpdate(chrB, 3, 4);
			break;
		case MODE2:
			chr2kUpdate(chrB, 1, 0);
			chr2kUpdate(chrB, 3, 2);
			chr2kUpdate(chrB, 1, 4);
			chr2kUpdate(chrB, 3, 6);
			break;
		case MODE3:
			chr1kUpdate(chrB, 0, 0);
			chr1kUpdate(chrB, 1, 1);
			chr1kUpdate(chrB, 2, 2);
			chr1kUpdate(chrB, 3, 3);
			chr1kUpdate(chrB, 0, 4);
			chr1kUpdate(chrB, 1, 5);
			chr1kUpdate(chrB, 2, 6);
			chr1kUpdate(chrB, 3, 7);
			break;
	}
}
