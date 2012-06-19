/*
 * mappers.h
 *
 *  Created on: 09/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPERS_H_
#define MAPPERS_H_

#include "common.h"
#include "externalcalls.h"
#include "mappers/mapper0.h"
#include "mappers/mapperMMC1.h"
#include "mappers/mapperMMC2andMMC4.h"
#include "mappers/mapperMMC3.h"
#include "mappers/mapperMMC5.h"
#include "mappers/mapperUxROM.h"
#include "mappers/mapperCNROM.h"
#include "mappers/mapperAxROM.h"
#include "mappers/mapperColorDreams.h"
#include "mappers/mapperCamerica.h"
#include "mappers/mapperCPROM.h"
#include "mappers/mapperBxROM.h"
#include "mappers/mapperGxROM.h"
#include "mappers/mapperAgci.h"
#include "mappers/mapperJaleco.h"
#include "mappers/mapperTaito.h"
#include "mappers/mapperVRC1.h"
#include "mappers/mapperVRC2.h"
#include "mappers/mapperVRC3.h"
#include "mappers/mapperVRC4.h"
#include "mappers/mapperRex.h"
#include "mappers/mapperWaixing.h"
#include "mappers/mapperIrem.h"
#include "mappers/mapperNamco.h"
#include "mappers/mapper74x138x161.h"
#include "mappers/mapper74x161x161x32.h"
#include "mappers/mapperCaltron.h"
#include "mappers/mapperTengen.h"
#include "mappers/mapperSunsoft.h"
#include "mappers/mapperAve.h"
#include "mappers/mapperBandai.h"
#include "mappers/mapperVs.h"
#include "mappers/mapperMagic.h"
#include "mappers/mapperWhirlwind.h"
#include "mappers/mapperNtdec.h"
#include "mappers/mapperHes.h"
#include "mappers/mapperSachen.h"
#include "mappers/mapperKasing.h"
#include "mappers/mapperFuturemedia.h"
#include "mappers/mapperTxROM.h"
#include "mappers/mapper120.h"
#include "mappers/mapperTxc.h"
#include "mappers/mapper208.h"
#include "mappers/mapper121.h"
#include "mappers/mapper156.h"
#include "mappers/mapperKaiser.h"
#include "mappers/mapper176.h"
#include "mappers/mapperHen.h"
#include "mappers/mapper178.h"
#include "mappers/mapper182.h"
#include "mappers/mapper114.h"
#include "mappers/mapper183.h"
#include "mappers/mapper186.h"
#include "mappers/mapperVRC6.h"
#include "mappers/mapperVRC7.h"
#include "mappers/mapperActive.h"
#include "mappers/mapper229.h"
#include "mappers/mapper230.h"
#include "mappers/mapper231.h"
#include "mappers/mapper225.h"
#include "mappers/mapper226.h"
#include "mappers/mapper227.h"
#include "mappers/mapper233.h"
#include "mappers/mapper235.h"
#include "mappers/mapper221.h"
#include "mappers/mapper219.h"
#include "mappers/mapper200.h"
#include "mappers/mapper201.h"
#include "mappers/mapper202.h"
#include "mappers/mapper203.h"
#include "mappers/mapper204.h"
#include "mappers/mapper205.h"
#include "mappers/mapper212.h"
#include "mappers/mapper213.h"
#include "mappers/mapper214.h"
#include "mappers/mapper215.h"
#include "mappers/mapperRcm.h"
#include "mappers/mapper217.h"
#include "mappers/mapper222.h"
#include "mappers/mapper240.h"
#include "mappers/mapper241.h"
#include "mappers/mapper244.h"
#include "mappers/mapper37.h"
#include "mappers/mapper44.h"
#include "mappers/mapper45.h"
#include "mappers/mapper46.h"
#include "mappers/mapper47.h"
#include "mappers/mapper49.h"
#include "mappers/mapper50.h"
#include "mappers/mapper51.h"
#include "mappers/mapper52.h"
#include "mappers/mapper53.h"
#include "mappers/mapper57.h"
#include "mappers/mapper58.h"
#include "mappers/mapper60.h"
#include "mappers/mapper61.h"
#include "mappers/mapper62.h"
#include "mappers/mapper242.h"
#include "mappers/mapper246.h"
#include "mappers/mapper116.h"
#include "mappers/mapperFDS.h"
#include "mappers/mapperGameGenie.h"

#define _controlBank(val, max)\
	if (val > max) {\
		val &= max;\
	}
#define controlBank(max)\
	_controlBank(value, max)
#define controlBankWithAND(mask, max)\
{\
	value &= mask;\
	controlBank(max)\
}
#define prgRomRd(address) prg.rom8k[(address >> 13) & 0x03][address & 0x1FFF]
#define chrBank1kReset()\
{\
	BYTE bank1k;\
	for (bank1k = 0; bank1k < 8; ++bank1k) {\
		chr.bank1k[bank1k] = &chr.data[bank1k * 0x0400];\
	}\
}
#define mapperRdBatteryDefault()\
{\
	BYTE bank;\
	/*\
	 * se non e' specificato da che banco di PRG ram inizia\
	 * la battery packed Ram, utilizzo sempre l'ultimo.\
	 */\
	if (info.prgRamBatStart == DEFAULT) {\
		bank = info.prgRamPlus8kCount - info.prgRamBatBanks;\
	} else {\
		bank = info.prgRamBatStart;\
	}\
	prg.ramBattery = &prg.ramPlus[bank * 0x2000];\
	if (fp) {\
		/* ne leggo il contenuto */\
		if (fread(&prg.ramBattery[0], info.prgRamBatBanks * 8192, 1, fp) < 1) {\
			fprintf(stderr, "error on read battery memory\n");\
		}\
	}\
}
#define mapperWrBatteryDefault()\
	/* ci scrivo i dati */\
	if (fwrite(&prg.ramBattery[0], info.prgRamBatBanks * 8192, 1, fp) < 1) {\
		fprintf(stderr, "error on write battery memory\n");\
	}

enum { RDBAT, WRBAT };

typedef struct {
	BYTE mirroring;
	BYTE writeVRAM;
	WORD romMapTo[4];
	BYTE *intStruct[2];
	WORD intStructSize[2];
} _mapper;

_mapper mapper;

BYTE mapInit(WORD mapperType);
void mapQuit(void);
void mapPrgRom8kReset(void);
void mapPrgRom8k(BYTE banks8k, BYTE at, BYTE value);
void mapPrgRom8kUpdate(void);
void mapPrgRamInit(void);
BYTE mapChrRamInit(void);

#endif /* MAPPERS_H_ */
