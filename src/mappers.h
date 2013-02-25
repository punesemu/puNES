/*
 * mappers.h
 *
 *  Created on: 09/mag/2010
 *      Author: fhorse
 */

#ifndef MAPPERS_H_
#define MAPPERS_H_

#include "common.h"
#include "external_calls.h"
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

#define _control_bank(val, max)\
	if (val > max) {\
		val &= max;\
	}
#define control_bank(max)\
	_control_bank(value, max)
#define control_bank_with_AND(mask, max)\
{\
	value &= mask;\
	control_bank(max)\
}
#define prg_rom_rd(address) prg.rom_8k[(address >> 13) & 0x03][address & 0x1FFF]
#define chr_bank_1k_reset()\
{\
	BYTE bank1k;\
	for (bank1k = 0; bank1k < 8; ++bank1k) {\
		chr.bank_1k[bank1k] = &chr.data[bank1k * 0x0400];\
	}\
}
#define mapper_rd_battery_default()\
{\
	BYTE bank;\
	/*\
	 * se non e' specificato da che banco di PRG ram inizia\
	 * la battery packed Ram, utilizzo sempre l'ultimo.\
	 */\
	if (info.prg_ram_bat_start == DEFAULT) {\
		bank = info.prg_ram_plus_8k_count - info.prg_ram_bat_banks;\
	} else {\
		bank = info.prg_ram_bat_start;\
	}\
	prg.ram_battery = &prg.ram_plus[bank * 0x2000];\
	if (fp) {\
		/* ne leggo il contenuto */\
		if (fread(&prg.ram_battery[0], info.prg_ram_bat_banks * 8192, 1, fp) < 1) {\
			fprintf(stderr, "error on read battery memory\n");\
		}\
	}\
}
#define mapper_wr_battery_default()\
	/* ci scrivo i dati */\
	if (fwrite(&prg.ram_battery[0], info.prg_ram_bat_banks * 8192, 1, fp) < 1) {\
		fprintf(stderr, "error on write battery memory\n");\
	}

enum { RD_BAT, WR_BAT };

typedef struct {
	BYTE mirroring;
	BYTE write_vram;
	WORD rom_map_to[4];
	BYTE *internal_struct[2];
	WORD internal_struct_size[2];
} _mapper;

_mapper mapper;

BYTE map_init(WORD mapper_type);
void map_quit(void);
void map_prg_rom_8k_reset(void);
void map_prg_rom_8k(BYTE banks_8k, BYTE at, BYTE value);
void map_prg_rom_8k_update(void);
void map_prg_ram_init(void);
BYTE map_chr_ram_init(void);

#endif /* MAPPERS_H_ */
