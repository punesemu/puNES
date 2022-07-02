/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MAPPERS_H_
#define MAPPERS_H_

#include "common.h"
#include "external_calls.h"
#include "unif.h"
#include "mappers/mapper_NSF.h"
/* INES/NES2.0 */
#include "mappers/mapper_0.h"
#include "mappers/mapper_MMC1.h"
#include "mappers/mapper_MMC2andMMC4.h"
#include "mappers/mapper_MMC3.h"
#include "mappers/mapper_MMC5.h"
#include "mappers/mapper_UxROM.h"
#include "mappers/mapper_CNROM.h"
#include "mappers/mapper_AxROM.h"
#include "mappers/mapper_ColorDreams.h"
#include "mappers/mapper_Coolboy.h"
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
#include "mappers/mapper_Rcm.h"
#include "mappers/mapper_217.h"
#include "mappers/mapper_222.h"
#include "mappers/mapper_240.h"
#include "mappers/mapper_241.h"
#include "mappers/mapper_244.h"
#include "mappers/mapper_37.h"
#include "mappers/mapper_40.h"
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
#include "mappers/mapper_JYASIC.h"
#include "mappers/mapper_83.h"
#include "mappers/mapper_28.h"
#include "mappers/mapper_42.h"
#include "mappers/mapper_91.h"
#include "mappers/mapper_105.h"
#include "mappers/mapper_31.h"
#include "mappers/mapper_BMCFK23C.h"
#include "mappers/mapper_134.h"
#include "mappers/mapper_43.h"
#include "mappers/mapper_187.h"
#include "mappers/mapper_FS304.h"
#include "mappers/mapper_196.h"
#include "mappers/mapper_252.h"
#include "mappers/mapper_253.h"
#include "mappers/mapper_250.h"
#include "mappers/mapper_254.h"
#include "mappers/mapper_197.h"
#include "mappers/mapper_188.h"
#include "mappers/mapper_168.h"
#include "mappers/mapper_167.h"
#include "mappers/mapper_166.h"
#include "mappers/mapper_SC_127.h"
#include "mappers/mapper_190.h"
#include "mappers/mapper_36.h"
#include "mappers/mapper_375.h"
#include "mappers/mapper_512.h"
#include "mappers/mapper_185.h"
#include "mappers/mapper_413.h"
#include "mappers/mapper_59.h"
#include "mappers/mapper_63.h"
#include "mappers/mapper_101.h"
#include "mappers/mapper_103.h"
#include "mappers/mapper_106.h"
#include "mappers/mapper_CHEAPOCABRA.h"
#include "mappers/mapper_126.h"
#include "mappers/mapper_198.h"
#include "mappers/mapper_218.h"
#include "mappers/mapper_237.h"
#include "mappers/mapper_267.h"
#include "mappers/mapper_269.h"
#include "mappers/mapper_399.h"
#include "mappers/mapper_288.h"
#include "mappers/mapper_297.h"
#include "mappers/mapper_541.h"
#include "mappers/mapper_428.h"
#include "mappers/mapper_353.h"
#include "mappers/mapper_356.h"
#include "mappers/mapper_357.h"
#include "mappers/mapper_359.h"
#include "mappers/mapper_360.h"
#include "mappers/mapper_361.h"
#include "mappers/mapper_372.h"
#include "mappers/mapper_374.h"
#include "mappers/mapper_377.h"
#include "mappers/mapper_380.h"
#include "mappers/mapper_381.h"
#include "mappers/mapper_382.h"
#include "mappers/mapper_389.h"
#include "mappers/mapper_393.h"
#include "mappers/mapper_394.h"
#include "mappers/mapper_396.h"
#include "mappers/mapper_398.h"
#include "mappers/mapper_400.h"
#include "mappers/mapper_401.h"
#include "mappers/mapper_403.h"
#include "mappers/mapper_404.h"
#include "mappers/mapper_406.h"
#include "mappers/mapper_409.h"
#include "mappers/mapper_410.h"
#include "mappers/mapper_411.h"
#include "mappers/mapper_412.h"
#include "mappers/mapper_414.h"
#include "mappers/mapper_415.h"
#include "mappers/mapper_416.h"
#include "mappers/mapper_417.h"
#include "mappers/mapper_420.h"
#include "mappers/mapper_429.h"
#include "mappers/mapper_431.h"
#include "mappers/mapper_432.h"
#include "mappers/mapper_433.h"
#include "mappers/mapper_FFESMC.h"
#include "mappers/mapper_29.h"
#include "mappers/mapper_516.h"
#include "mappers/mapper_559.h"
#include "mappers/mapper_457.h"
#include "mappers/mapper_452.h"
#include "mappers/mapper_447.h"
#include "mappers/mapper_455.h"
#include "mappers/mapper_437.h"
#include "mappers/mapper_456.h"
#include "mappers/mapper_434.h"
/* UNIF */
#include "mappers/mapper_A65AS.h"
#include "mappers/mapper_Malee.h"
#include "mappers/mapper_TF1201.h"
#include "mappers/mapper_EH8813A.h"
#include "mappers/mapper_BMC11160.h"
#include "mappers/mapper_BMCG146.h"
#include "mappers/mapper_BMC12IN1.h"
#include "mappers/mapper_BMC411120C.h"
#include "mappers/mapper_T262.h"
#include "mappers/mapper_BS5.h"
#include "mappers/mapper_UNIF8157.h"
#include "mappers/mapper_BMC830118C.h"
#include "mappers/mapper_UNIF8237.h"
#include "mappers/mapper_BMCNTD03.h"
#include "mappers/mapper_BMCGhostbusters63in1.h"
#include "mappers/mapper_BMC64IN1NOREPEAT.h"
#include "mappers/mapper_BMC70IN1.h"
#include "mappers/mapper_H2288.h"
#include "mappers/mapper_KOF97.h"
#include "mappers/mapper_UNIF603_5052.h"
#include "mappers/mapper_CITYFIGHT.h"
#include "mappers/mapper_UNIF43272.h"
#include "mappers/mapper_AC08.h"
#include "mappers/mapper_KS7013B.h"
#include "mappers/mapper_YOKO.h"
#include "mappers/mapper_SA_9602B.h"
#include "mappers/mapper_CC_21.h"
#include "mappers/mapper_LH32.h"
#include "mappers/mapper_SL1632.h"
#include "mappers/mapper_SHERO.h"
#include "mappers/mapper_UNIFSMB2J.h"
#include "mappers/mapper_AX5705.h"
#include "mappers/mapper_GS_20xx.h"
#include "mappers/mapper_KS7012.h"
#include "mappers/mapper_KS7037.h"
#include "mappers/mapper_KS7057.h"
#include "mappers/mapper_KS7016.h"
#include "mappers/mapper_KS7017.h"
#include "mappers/mapper_LH10.h"
#include "mappers/mapper_KS7032.h"
#include "mappers/mapper_RT_01.h"
#include "mappers/mapper_MALISB.h"
#include "mappers/mapper_BOY.h"
#include "mappers/mapper_8_IN_1.h"
#include "mappers/mapper_BMCHP898F.h"
#include "mappers/mapper_UNIF158B.h"
#include "mappers/mapper_BMC810544CA1.h"
#include "mappers/mapper_KS7031.h"
#include "mappers/mapper_DRAGONFIGHTER.h"
#include "mappers/mapper_EDU2000.h"
#include "mappers/mapper_DREAMTECH01.h"
#include "mappers/mapper_OneBus.h"
#include "mappers/mapper_BMC190IN1.h"
#include "mappers/mapper_K3033.h"
#include "mappers/mapper_BMC830425C.h"
#include "mappers/mapper_FARIDSLROM8IN1.h"
#include "mappers/mapper_FARIDUNROM8IN1.h"
#include "mappers/mapper_RESETTXROM.h"
#include "mappers/mapper_LH51.h"
#include "mappers/mapper_60311C.h"
#include "mappers/mapper_WS.h"
#include "mappers/mapper_AX40G.h"
#include "mappers/mapper_891227.h"
#include "mappers/mapper_BJ56.h"
#include "mappers/mapper_L6IN1.h"
#include "mappers/mapper_BTL900218.h"
#include "mappers/mapper_GN26.h"
#include "mappers/mapper_KS7021A.h"
#include "mappers/mapper_TH21311.h"
#include "mappers/mapper_HP2018A.h"
#include "mappers/mapper_RESETNROMXIN1.h"
#include "mappers/mapper_BMC830134C.h"
#include "mappers/mapper_K3071.h"
#include "mappers/mapper_KS106C.h"
#include "mappers/mapper_TJ03.h"
#include "mappers/mapper_SA005A.h"
#include "mappers/mapper_K3046.h"
#include "mappers/mapper_K3036.h"
#include "mappers/mapper_K3006.h"
#include "mappers/mapper_F15.h"
#include "mappers/mapper_CTC12IN1.h"
#include "mappers/mapper_CTC09.h"
#include "mappers/mapper_80013B.h"
#include "mappers/mapper_1024CA1.h"
#include "mappers/mapper_831128C.h"
#include "mappers/mapper_Coolgirl.h"
#include "mappers/mapper_DRIPGAME.h"
#include "mappers/mapper_KONAMIQTAI.h"
#include "mappers/mapper_22026.h"
#include "mappers/mapper_KS7030.h"
#include "mappers/mapper_3DBLOCK.h"

#define _control_bank(val, max)\
	if (val > max) {\
		val &= max;\
	}
#define control_bank(max)\
	_control_bank(value, max)
#define _control_bank_with_AND(val, mask, max)\
{\
	val &= mask;\
	_control_bank(val, max)\
}
#define control_bank_with_AND(mask, max)\
	_control_bank_with_AND(value, mask, max)
#define prg_rom_rd(address) prg.rom_8k[(address >> 13) & 0x03][address & 0x1FFF]

enum mappers_op_battery { RD_BAT, WR_BAT };

typedef struct _mapper {
	BYTE mirroring;
	BYTE write_vram;
	WORD rom_map_to[4];
	BYTE *internal_struct[10];
	WORD internal_struct_size[10];
	BYTE trainer[512];
	struct _misc_roms {
		size_t size;
		BYTE *data;
	} misc_roms;
} _mapper;

extern _mapper mapper;

BYTE map_init(void);
void map_quit(void);
BYTE map_prg_malloc(size_t size, BYTE set_value, BYTE init_chip0_rom);
void map_prg_rom_8k(BYTE banks_8k, BYTE at, WORD value);
void map_prg_rom_8k_reset(void);
void map_prg_rom_8k_update(void);
void map_prg_ram_init(void);
BYTE map_prg_ram_malloc(WORD size);
void map_prg_ram_memset(void);
void map_prg_ram_battery_save(void);
void map_prg_ram_battery_load(void);
BYTE map_chr_malloc(size_t size, BYTE set_value, BYTE init_chip0_rom);
void map_chr_bank_1k_reset(void);
BYTE map_chr_ram_init(void);
BYTE map_chr_ram_extra_init(uint32_t size);
void map_chr_ram_extra_reset(void);
BYTE map_misc_malloc(size_t size, BYTE set_value);
void map_set_banks_max_prg(void);
void map_set_banks_max_chr(void);
void map_bat_wr_default(FILE *fp) ;
void map_bat_rd_default(FILE *fp);

#endif /* MAPPERS_H_ */
