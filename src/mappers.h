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
#include "mappers/mapper_0.h"
#include "mappers/mapper_MMC1.h"
#include "mappers/mapper_MMC2andMMC4.h"
#include "mappers/mapper_MMC3.h"
#include "mappers/mapper_MMC5.h"
#include "mappers/mapper_UxROM.h"
#include "mappers/mapper_CNROM.h"
#include "mappers/mapper_AxROM.h"
#include "mappers/mapper_ColorDreams.h"
#include "mappers/mapper_Camerica.h"
#include "mappers/mapper_CPROM.h"
#include "mappers/mapper_BxROM.h"
#include "mappers/mapper_GxROM.h"
#include "mappers/mapper_Agci.h"
#include "mappers/mapper_Jaleco.h"
#include "mappers/mapper_Taito.h"
#include "mappers/mapper_VRC1.h"
#include "mappers/mapper_VRC2.h"
#include "mappers/mapper_VRC3.h"
#include "mappers/mapper_VRC4.h"
#include "mappers/mapper_Rex.h"
#include "mappers/mapper_Waixing.h"
#include "mappers/mapper_Irem.h"
#include "mappers/mapper_Namco.h"
#include "mappers/mapper_74x138x161.h"
#include "mappers/mapper_74x161x161x32.h"
#include "mappers/mapper_Caltron.h"
#include "mappers/mapper_Tengen.h"
#include "mappers/mapper_Sunsoft.h"
#include "mappers/mapper_Ave.h"
#include "mappers/mapper_Bandai.h"
#include "mappers/mapper_Vs.h"
#include "mappers/mapper_Magic.h"
#include "mappers/mapper_Whirlwind.h"
#include "mappers/mapper_Ntdec.h"
#include "mappers/mapper_Hes.h"
#include "mappers/mapper_Sachen.h"
#include "mappers/mapper_Kasing.h"
#include "mappers/mapper_Futuremedia.h"
#include "mappers/mapper_TxROM.h"
#include "mappers/mapper_120.h"
#include "mappers/mapper_Txc.h"
#include "mappers/mapper_208.h"
#include "mappers/mapper_121.h"
#include "mappers/mapper_156.h"
#include "mappers/mapper_Kaiser.h"
#include "mappers/mapper_176.h"
#include "mappers/mapper_Hen.h"
#include "mappers/mapper_178.h"
#include "mappers/mapper_182.h"
#include "mappers/mapper_114.h"
#include "mappers/mapper_183.h"
#include "mappers/mapper_186.h"
#include "mappers/mapper_VRC6.h"
#include "mappers/mapper_VRC7.h"
#include "mappers/mapper_Active.h"
#include "mappers/mapper_229.h"
#include "mappers/mapper_230.h"
#include "mappers/mapper_231.h"
#include "mappers/mapper_225.h"
#include "mappers/mapper_226.h"
#include "mappers/mapper_227.h"
#include "mappers/mapper_233.h"
#include "mappers/mapper_235.h"
#include "mappers/mapper_221.h"
#include "mappers/mapper_219.h"
#include "mappers/mapper_200.h"
#include "mappers/mapper_201.h"
#include "mappers/mapper_202.h"
#include "mappers/mapper_203.h"
#include "mappers/mapper_204.h"
#include "mappers/mapper_205.h"
#include "mappers/mapper_212.h"
#include "mappers/mapper_213.h"
#include "mappers/mapper_214.h"
#include "mappers/mapper_215.h"
#include "mappers/mapper_Rcm.h"
#include "mappers/mapper_217.h"
#include "mappers/mapper_222.h"
#include "mappers/mapper_240.h"
#include "mappers/mapper_241.h"
#include "mappers/mapper_244.h"
#include "mappers/mapper_37.h"
#include "mappers/mapper_44.h"
#include "mappers/mapper_45.h"
#include "mappers/mapper_46.h"
#include "mappers/mapper_47.h"
#include "mappers/mapper_49.h"
#include "mappers/mapper_50.h"
#include "mappers/mapper_51.h"
#include "mappers/mapper_52.h"
#include "mappers/mapper_53.h"
#include "mappers/mapper_57.h"
#include "mappers/mapper_58.h"
#include "mappers/mapper_60.h"
#include "mappers/mapper_61.h"
#include "mappers/mapper_62.h"
#include "mappers/mapper_242.h"
#include "mappers/mapper_246.h"
#include "mappers/mapper_116.h"
#include "mappers/mapper_FDS.h"
#include "mappers/mapper_GameGenie.h"
#include "mappers/mapper_163.h"
#include "mappers/mapper_164.h"
#include "mappers/mapper_249.h"
#include "mappers/mapper_90_209_211.h"
#include "mappers/mapper_83.h"
#include "mappers/mapper_28.h"
#include "mappers/mapper_42.h"

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

struct _trainer {
	BYTE data[512];
} trainer;

_mapper mapper;

BYTE map_init(WORD mapper_type);
void map_quit(void);
void map_prg_rom_8k_reset(void);
void map_prg_rom_8k(BYTE banks_8k, BYTE at, BYTE value);
void map_prg_rom_8k_update(void);
void map_prg_ram_init(void);
BYTE map_chr_ram_init(void);

#endif /* MAPPERS_H_ */
