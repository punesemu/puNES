/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "irqA12.h"
#include "irql2f.h"
#include "tas.h"
#include "uncompress.h"
#include "unif.h"
#include "gui.h"

_trainer trainer;
_mapper mapper;

BYTE map_init(void) {
	BYTE i;
	/*
	 * di default la routine di salvataggio
	 * di una possibile struttura interna
	 * di dati della mapper e' NULL.
	 */
	for (i = 0; i < LENGTH(mapper.internal_struct); i++) {
		mapper.internal_struct[i] = 0;
	}
	/* disabilito gli accessori */
	irqA12.present = FALSE;
	irql2f.present = FALSE;
	/* disabilito tutte le chiamate relative alle mappers */
	extcl_init();

	switch (info.mapper.id) {
		case 0:
			map_init_0();
			break;
		case 1:
			map_init_MMC1();
			break;
		case 2:
			if (info.mapper.submapper == UNLROM) {
				map_init_UxROM(UNLROM);
			} else if (info.mapper.submapper == UNROM_BK2) {
				map_init_UxROM(UNROM_BK2);
			} else {
				map_init_UxROM(UXROM);
			}
			break;
		case 3:
			map_init_CNROM();
			break;
		case 4:
			map_init_MMC3();
			break;
		case 5:
			map_init_MMC5();
			break;
		case 7:
			map_init_AxROM();
			break;
			/* per MMC2 e MMC4 uso le stesse routine */
		case 9:
		case 10:
			map_init_MMC2and4();
			break;
		case 11:
			map_init_ColorDreams();
			break;
		case 12:
			map_init_Rex(DBZ);
			break;
		case 13:
			map_init_CPROM();
			break;
		case 15:
			map_init_Waixing(WPSX);
			break;
		case 16: {
			switch (info.mapper.submapper) {
				case E24C02:
					map_init_Bandai(E24C02);
					break;
				case DATACH:
					map_init_Bandai(DATACH);
					break;
				default:
					map_init_Bandai(FCGx);
					break;
			}
			break;
		}
		case 18:
			map_init_Jaleco(SS8806);
			break;
		case 19:
			map_init_Namco(N163);
			break;
		case 21:
			map_init_VRC4(info.mapper.submapper == DEFAULT ? VRC4A : info.mapper.submapper);
			break;
		case 22:
			map_init_VRC2(VRC2A);
			break;
		case 23:
			if (info.mapper.submapper == VRC4BMC) {
				map_init_VRC4BMC();
			} else if (info.mapper.submapper == VRC4T230) {
				map_init_VRC4T230();
			} else if (info.mapper.submapper == VRC4E) {
				map_init_VRC4(VRC4E);
			} else {
				map_init_VRC2(VRC2B);
			}
			break;
		case 24:
			map_init_VRC6(VRC6A);
			break;
		case 25:
			map_init_VRC4(info.mapper.submapper == DEFAULT ? VRC4B : info.mapper.submapper);
			break;
		case 26:
			map_init_VRC6(VRC6B);
			break;
		case 27:
			map_init_VRC4(VRC4UNL);
			break;
		case 28:
			map_init_28();
			break;
		case 30:
			map_init_UxROM(UNROM_BK2);
			break;
		case 31:
			map_init_31();
			break;
		case 32:
			map_init_Irem(G101);
			break;
		case 33:
			map_init_Taito(TC0190FMC);
			break;
		case 34:
			map_init_BxROM();
			break;
		case 35:
			map_init_SC_127();
			break;
		case 36:
			map_init_36();
			break;
		case 37:
			map_init_37();
			break;
		case 38:
			map_init_74x138x161();
			break;
		case 40:
			map_init_40();
			break;
		case 41:
			map_init_Caltron();
			break;
		case 42:
			map_init_42();
			break;
		case 43:
			map_init_43();
			break;
		case 44:
			map_init_44();
			break;
		case 45:
			map_init_45();
			break;
		case 46:
			map_init_46();
			break;
		case 47:
			map_init_47();
			break;
		case 49:
			map_init_49();
			break;
		case 50:
			map_init_50();
			break;
		case 51:
			map_init_51();
			break;
		case 52:
			map_init_52(info.mapper.submapper);
			break;
		case 53:
			map_init_53();
			break;
		case 56:
			map_init_Kaiser(KS202);
			break;
		case 57:
			map_init_57();
			break;
		case 58:
			map_init_58();
			break;
		case 60:
			if (info.mapper.submapper == MAP60_VT5201) {
				map_init_60_vt5201();
			} else {
				map_init_60();
			}
			break;
		case 61:
			map_init_61();
			break;
		case 62:
			map_init_62();
			break;
		case 64:
			map_init_Tengen(TRAMBO);
			break;
		case 65:
			map_init_Irem(H3000);
			break;
		case 66:
			map_init_GxROM();
			break;
		case 67:
			map_init_Sunsoft(SUN3);
			break;
		case 68:
			map_init_Sunsoft(SUN4);
			break;
		case 69:
			map_init_Sunsoft(FM7);
			break;
		case 70:
			map_init_74x161x161x32(IC74X161X161X32A);
			break;
		case 71:
			map_init_Camerica();
			break;
		case 72:
			map_init_Jaleco(JF17);
			break;
		case 73:
			map_init_VRC3();
			break;
		case 74:
			map_init_Waixing(WTA);
			break;
		case 75:
		case 151:
			map_init_VRC1();
			break;
		case 76:
			map_init_Namco(N3446);
			break;
		case 77:
			map_init_Irem(LROG017);
			break;
		case 78:
			map_init_Jaleco(JF16);
			break;
		case 79:
			map_init_Ave(NINA06);
			break;
		case 80:
			if (info.mapper.submapper == X1005B) {
				map_init_Taito(X1005B);
			} else {
				map_init_Taito(X1005A);
			}
			break;
		case 82:
			map_init_Taito(X1017);
			break;
		case 83:
			map_init_83();
			break;
		case 85:
			if (info.mapper.submapper == VRC7UNL) {
				map_init_VRC7UNL();
			} else if (info.mapper.submapper == VRC7A) {
				map_init_VRC7(VRC7A);
			} else {
				map_init_VRC7(VRC7B);
			}
			break;
		case 86:
			map_init_Jaleco(JF13);
			break;
		case 87:
			map_init_Jaleco(JF05);
			break;
		case 88:
			map_init_Namco(N3433);
			break;
		case 89:
			map_init_Sunsoft(SUN2B);
			break;
		case 90:
			map_init_90_209_211(MAP90);
			break;
		case 91:
			map_init_91();
			break;
		case 92:
			map_init_Jaleco(JF19);
			break;
		case 93:
			map_init_Sunsoft(SUN2A);
			break;
		case 94:
			map_init_UxROM(UNL1XROM);
			break;
		case 95:
			map_init_Namco(N3425);
			break;
		case 96:
			map_init_Bandai(B161X02X74);
			break;
		case 97:
			map_init_Irem(TAMS1);
			break;
		case 99:
			map_init_Vs();
			break;
		case 105:
			map_init_105();
			break;
		case 107:
			map_init_Magic();
			break;
		case 108:
			map_init_Whirlwind();
			break;
		case 112:
			map_init_Ntdec(ASDER);
			break;
		case 113:
			map_init_Hes();
			break;
		case 114:
			map_init_114();
			break;
		case 115:
			map_init_Kasing();
			break;
		case 116:
			map_init_116();
			break;
		case 117:
			map_init_Futuremedia();
			break;
		case 118:
			if (info.mapper.submapper == TKSROM) {
				map_init_TxROM(TKSROM);
			} else {
				map_init_TxROM(TLSROM);
			}
			break;
		case 119:
			map_init_TxROM(TQROM);
			break;
		case 120:
			map_init_120();
			break;
		case 121:
			map_init_121();
			break;
		case 123:
			map_init_H2288();
			break;
		case 132:
			map_init_Txc(T22211A);
			break;
		case 133:
			map_init_Sachen(SA72008);
			break;
		case 134:
			map_init_134();
			break;
		case 136:
			map_init_Sachen(TCU02);
			break;
		case 137:
			map_init_Sachen(SA8259D);
			break;
		case 138:
			map_init_Sachen(SA8259B);
			break;
		case 139:
			map_init_Sachen(SA8259C);
			break;
		case 140:
			map_init_Jaleco(JF11);
			break;
		case 141:
			map_init_Sachen(SA8259A);
			break;
		case 142:
			map_init_Kaiser(KS7032);
			break;
		case 143:
			map_init_Sachen(TCA01);
			break;
		case 144:
			map_init_Agci();
			break;
		case 145:
			map_init_Sachen(SA72007);
			break;
		case 146:
			map_init_Ave(NINA06);
			break;
		case 147:
			map_init_Sachen(TCU01);
			break;
		case 148:
			map_init_Sachen(SA0037);
			break;
		case 149:
			map_init_Sachen(SA0036);
			break;
		case 150:
			map_init_Sachen(SA74374B);
			break;
		// la mapper 151 la tratto come la 75
		case 152:
			map_init_74x161x161x32(IC74X161X161X32B);
			break;
		case 153:
			/* Famicom Jump II - Saikyou no 7 Nin (J) [!].nes */
			map_init_Bandai(FCGx);
			break;
		case 154:
			map_init_Namco(N3453);
			break;
		case 155:
			info.mapper.submapper = SKROM;
			map_init_MMC1();
			break;
		case 156:
			map_init_156();
			break;
		case 158:
			map_init_Tengen(T800037);
			break;
		case 159:
			map_init_Bandai(E24C01);
			break;
		case 162:
			map_init_FS304();
			break;
		case 163:
			map_init_163();
			break;
		case 164:
			map_init_164();
			break;
		case 165:
			map_init_Waixing(SH2);
			break;
		case 166:
			map_init_166();
			break;
		case 167:
			map_init_167();
			break;
		case 168:
			map_init_168();
			break;
		case 171:
			map_init_Kaiser(KS7058);
			break;
		case 172:
			map_init_Txc(T22211B);
			break;
		case 173:
			map_init_Txc(T22211C);
			break;
		case 175:
			map_init_Kaiser(KS7022);
			break;
		case 176:
			if (info.id == NOBMCFK23C) {
				map_init_176();
			} else {
				map_init_BMCFK23C();
			}
			break;
		case 177:
			if (info.mapper.submapper != DEFAULT) {
				/* questa e' la mappers 179 in nestopia */
				map_init_Hen(info.mapper.submapper);
			} else {
				map_init_Hen(HEN_177);
			}
			break;
		case 178:
			map_init_178(info.mapper.submapper);
			break;
		case 180:
			map_init_UxROM(UNROM180);
			break;
		case 182:
			map_init_182();
			break;
		case 183:
			map_init_183();
			break;
		case 184:
			map_init_Sunsoft(SUN1);
			break;
		case 185:
			info.mapper.submapper = CNROM_CNFL;
			map_init_CNROM();
			break;
		case 186:
			map_init_186();
			break;
		case 187:
			map_init_187();
			break;
		case 188:
			map_init_188();
			break;
		case 189:
			map_init_Txc(TXCTW);
			break;
		case 190:
			map_init_190();
			break;
		case 191:
			map_init_Waixing(WTB);
			break;
		case 192:
			map_init_Waixing(WTC);
			break;
		case 193:
			map_init_Ntdec(FHERO);
			break;
		case 194:
			map_init_Waixing(WTD);
			break;
		case 195:
			map_init_Waixing(WTE);
			break;
		case 196:
			map_init_196();
			break;
		case 197:
			map_init_197();
			break;
		case 199:
			map_init_Waixing(WTG);
			break;
		case 200:
			map_init_200();
			break;
		case 201:
			map_init_201();
			break;
		case 202:
			map_init_202();
			break;
		case 203:
			map_init_203();
			break;
		case 204:
			map_init_204();
			break;
		case 205:
			map_init_205();
			break;
		case 206:
			map_init_Namco(N3416);
			break;
		// la mapper 207 la tratto come mapper 80
		case 208:
			map_init_208();
			break;
		case 209:
			map_init_90_209_211(MAP209);
			break;
		case 211:
			map_init_90_209_211(MAP211);
			break;
		case 212:
			map_init_212();
			break;
		case 213:
			map_init_213();
			break;
		case 214:
			map_init_214();
			break;
		case 215:
			map_init_215();
			break;
		case 216:
			map_init_Rcm(GS2015);
			break;
		case 217:
			map_init_217();
			break;
		case 219:
			map_init_219();
			break;
		case 221:
			map_init_221();
			break;
		case 222:
			map_init_222();
			break;
		case 225:
			map_init_225();
			break;
		case 226:
			map_init_226();
			break;
		case 227:
			map_init_227();
			break;
		case 228:
			map_init_Active();
			break;
		case 229:
			map_init_229();
			break;
		case 230:
			map_init_230();
			break;
		case 231:
			map_init_231();
			break;
		case 232:
			info.mapper.submapper = BF9096;
			map_init_Camerica();
			break;
		case 233:
			map_init_233();
			break;
		case 234:
			map_init_Ave(D1012);
			break;
		case 235:
			map_init_235();
			break;
		case 240:
			map_init_240();
			break;
		case 241:
			map_init_241();
			break;
		case 242:
			map_init_242();
			break;
		case 243:
			map_init_Sachen(SA74374A);
			break;
		case 244:
			map_init_244();
			break;
		case 245:
			map_init_Waixing(WTH);
			break;
		case 246:
			map_init_246();
			break;
		case 249:
			map_init_249();
			break;
		case 250:
			map_init_250();
			break;
		case 252:
			map_init_252();
			break;
		case 253:
			map_init_253();
			break;
		case 254:
			map_init_254();
			break;
		default:
			gui_overlay_info_append_msg_precompiled(11, NULL);
			fprintf(stderr, "Mapper not supported\n");
			EXTCL_CPU_WR_MEM(0);
			break;
		/* casi speciali */
		case NSF_MAPPER:
			map_init_NSF();
			break;
		case FDS_MAPPER:
			map_init_FDS();
			break;
		case GAMEGENIE_MAPPER:
			map_init_GameGenie();
			break;
		case UNIF_MAPPER:
			switch (unif.internal_mapper) {
				case 0:
					// A65AS
					map_init_A65AS();
					break;
				case 1:
					// MARIO1-MALEE2
					map_init_malee();
					break;
				case 2:
					// TF1021
					map_init_TF1201();
					break;
				case 3:
					// EH8813A
					map_init_EH8813A();
					break;
				case 4:
					// 11160
					map_init_BMC11160();
					break;
				case 5:
					// G-146
					map_init_BMCG146();
					break;
				case 6:
					// 12-IN-1
					map_init_BMC12IN1();
					break;
				case 7:
					// 411120-C
					map_init_BMC411120C();
					break;
				case 8:
					// T-262
					map_init_T262();
					break;
				case 9:
					// BS-5
					map_init_BS5();
					break;
				case 10:
					// 8157
					map_init_UNIF8157();
					break;
				case 11:
					// 830118C
					map_init_BMC830118C();
					break;
				case 12:
					// 8237
					map_init_UNIF8237(U8237);
					break;
				case 13:
					// 8237A
					map_init_UNIF8237(U8237A);
					break;
				case 14:
					// NTD-03
					map_init_BMCNTD03();
					break;
				case 15:
					// Ghostbusters63in1
					map_init_BMCGHOSTBUSTERS63IN1();
					break;
				case 16:
					// 64in1NoRepeat
					map_init_BMC64IN1NOREPEAT();
					break;
				case 17:
					// 70in1
					map_init_BMC70IN1(BMC70IN1);
					break;
				case 18:
					// 70in1
					map_init_BMC70IN1(BMC70IN1B);
					break;
				case 19:
					// KS7032
					map_init_KS7032();
					break;
				case 20:
					// KOF97
					map_init_KOF97();
					break;
				case 21:
					// 603-5052
					map_init_UNIF603_5052();
					break;
				case 22:
					// CITYFIGHT
					map_init_CITYFIGHT();
					break;
				case 23:
					// BB
					map_init_BB();
					break;
				case 24:
					// 43272
					map_init_UNIF43272();
					break;
				case 25:
					// AC-08
					map_init_AC08();
					break;
				case 26:
					// KS7013B
					map_init_KS7013B();
					break;
				case 27:
					// MTECH01
					map_init_MTECH01();
					break;
				case 28:
					// YOKO
					map_init_YOKO();
					break;
				case 29:
					// SA-9602B
					map_init_SA_9602B();
					break;
				case 30:
					// CC-21
					map_init_CC_21();
					break;
				case 31:
					// LH32
					map_init_LH32();
					break;
				case 32:
					// NovelDiamond9999999in1
					map_init_NovelDiamond();
					break;
				case 33:
					// SL1632
					map_init_SL1632();
					break;
				case 34:
					// SHERO
					map_init_SHERO();
					break;
				case 35:
					// SMB2J
					map_init_UNIFSMB2J();
					break;
				case 36:
					// AX5705
					map_init_AX5705();
					break;
				case 37:
					// GS-2004
					map_init_GS_2004();
					break;
				case 38:
					// GS-2013
					map_init_GS_2013();
					break;
				case 39:
					// KS7012
					map_init_KS7012();
					break;
				case 40:
					// KS7037
					map_init_KS7037();
					break;
				case 41:
					// KS7057
					map_init_KS7057();
					break;
				case 42:
					// KS7016
					map_init_KS7016();
					break;
				case 43:
					// KS7017
					map_init_KS7017();
					break;
				case 44:
					// LH10
					map_init_LH10();
					break;
				case 45:
					// RT-01
					map_init_RT_01();
					break;
				case 46:
					// MALISB
					map_init_MALISB();
					break;
				case 47:
					// BOY
					map_init_BOY();
					break;
				case 48:
					// 8-IN-1
					map_init_8_IN_1();
					break;
				case 49:
					// HP898F
					map_init_BMCHP898F();
					break;
				case 50:
					// 158B
					map_init_UNIF158B();
					break;
				case 51:
					// 810544-C-A1
					map_init_BMC810544CA1();
					break;
				case 52:
					// KS7031
					map_init_KS7031();
					break;
				case 53:
					// DRAGONFIGHTER
					map_init_DRAGONFIGHTER();
					break;
				case 54:
					// Super24in1SC03
					map_init_Super24in1();
					break;
				case 55:
					// EDU2000
					map_init_EDU2000();
					break;
				case 56:
					// DREAMTECH01
					map_init_DREAMTECH01();
					break;
			}
			break;
	}
	map_prg_rom_8k_update();
	map_prg_ram_init();
	if (map_chr_ram_init()) {
		return (EXIT_ERROR);
	}
	if (extcl_after_mapper_init) {
		extcl_after_mapper_init();
	}
	return (EXIT_OK);
}
void map_quit(void) {
	map_prg_ram_battery_save();

	info.id = 0;
	memset(&info.mapper, 0x00, sizeof(info.mapper));
	memset(&info.sha1sum, 0x00, sizeof(info.sha1sum));
	memset(&info.chr, 0x00, sizeof(info.chr));
	memset(&info.prg, 0x00, sizeof(info.prg));
	info.prg.ram.bat.start = DEFAULT;

	/* PRG */
	{
		BYTE i;

		for (i = 0; i < MAX_CHIPS; i++) {
			if (prg_chip(i)) {
				free(prg_chip(i));
				prg_chip(i) = NULL;
				prg_chip_size(i) = 0;
			}
		}
	}
	memset(&info.prg, 0x00, sizeof(info.prg));

	if (prg.ram.data) {
		free(prg.ram.data);
	}
	memset(&prg.ram, 0x00, sizeof(prg.ram));

	if (prg.ram_plus) {
		free(prg.ram_plus);
	}
	memset(prg.rom_8k, 0x00, sizeof(prg.rom_8k));
	prg.ram_plus = NULL;
	prg.ram_plus_8k = NULL;
	prg.ram_battery = NULL;

	/* CHR */
	{
		BYTE i;

		for (i = 0; i < MAX_CHIPS; i++) {
			if (chr_chip(i)) {
				free(chr_chip(i));
				chr_chip(i) = NULL;
				chr_chip_size(i) = 0;
			}
		}
	}
	memset(&info.chr, 0x00, sizeof(info.chr));

	/* CHR extra */
	if (chr.extra.data) {
		free(chr.extra.data);
		chr.extra.size = 0;
	}
	chr.extra.data = NULL;

	memset(chr.bank_1k, 0, sizeof(chr.bank_1k));

	mirroring_V();

	mapper.write_vram = FALSE;
}

BYTE map_prg_chip_malloc(BYTE index, size_t size, BYTE set_value) {
	if (prg_chip(index)) {
		free(prg_chip(index));
		prg_chip(index) = NULL;
	}

	prg_chip_size(index) = size;
	info.prg.chips++;

	if ((prg_chip(index) = (BYTE *)malloc(size))) {
		memset(prg_chip(index), set_value, size);
	} else {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
BYTE map_prg_chip_rd_byte(BYTE index, BYTE openbus, WORD address, WORD mask) {
	if (prg_chip(index)) {
		return (prg_chip_byte(1, address & mask));
	}
	return (openbus);
}
void map_prg_rom_8k_chip(BYTE banks_8k, BYTE at, WORD value, WORD chip) {
	BYTE a;

	/* se cerco di switchare 32k ma ho solo un banco da 16k esco */
	if ((banks_8k == 4) && (info.prg.rom[chip].banks_16k <= 1)) {
		return;
	}

	for (a = 0; a < banks_8k; ++a) {
		prg.rom_chip[at + a] = chip;
		mapper.rom_map_to[at + a] = ((value * banks_8k) + a);
	}
}
void map_prg_rom_8k_reset(void) {
	prg.rom_chip[0] = 0;
	prg.rom_chip[1] = 0;
	prg.rom_chip[2] = 0;
	prg.rom_chip[3] = 0;

	mapper.rom_map_to[0] = 0;
	mapper.rom_map_to[1] = 1;
	mapper.rom_map_to[2] = info.prg.rom[prg.rom_chip[2]].banks_8k - 2;
	mapper.rom_map_to[3] = info.prg.rom[prg.rom_chip[3]].banks_8k - 1;
}
void map_prg_rom_8k_update(void) {
	BYTE i;

	for (i = 0; i < 4; ++i) {
		prg.rom_8k[i] = prg_chip_byte_pnt(prg.rom_chip[i], mapper.rom_map_to[i] << 13);
	}
}
void map_prg_ram_init(void) {
	/*
	 * se non ci sono stati settaggi particolari della mapper
	 * e devono esserci banchi di PRG Ram extra allora li assegno.
	 */
	if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) && info.prg.ram.banks_8k_plus && !prg.ram_plus) {
		/* alloco la memoria necessaria */
		prg.ram_plus = (BYTE *)malloc(prg_ram_plus_size());
		/* inizializzo */
		memset(prg.ram_plus, 0x00, prg_ram_plus_size());
		/* gli 8k iniziali */
		prg.ram_plus_8k = &prg.ram_plus[0];
		/* controllo se la rom ha una RAM PRG battery packed */
		if (info.prg.ram.bat.banks && (tas.type == NOTAS)) {
			uTCHAR prg_ram_file[LENGTH_FILE_NAME_LONG], basename[255], *fl, *last_dot;
			FILE *fp;

			fl = info.rom.file;

			gui_utf_basename(fl, basename, usizeof(basename));
			usnprintf(prg_ram_file, usizeof(prg_ram_file), uL("" uPERCENTs PRB_FOLDER "/" uPERCENTs), info.base_folder, basename);

			/* rintraccio l'ultimo '.' nel nome */
			if ((last_dot = ustrrchr(prg_ram_file, uL('.')))) {
				/* elimino l'estensione */
				(*last_dot) = 0x00;
			}
			/* aggiungo l'estensione prb */
			ustrcat(prg_ram_file, uL(".prb"));
			/* provo ad aprire il file */
			fp = ufopen(prg_ram_file, uL("rb"));
			if (extcl_battery_io) {
				extcl_battery_io(RD_BAT, fp);
			} else {
				mapper_rd_battery_default();
			}
			/* chiudo il file */
			if (fp) {
				fclose(fp);
			}
		}
	} else if ((info.reset >= HARD) && info.prg.ram.banks_8k_plus) {
		int i;

		for (i = 0; i < info.prg.ram.banks_8k_plus; i++) {
			if (info.prg.ram.bat.banks && (i >= info.prg.ram.bat.start) &&
				(i < (info.prg.ram.bat.start + info.prg.ram.bat.banks))) {
				continue;
			}
			memset(prg.ram_plus + (i * 0x2000), 0x00, 0x2000);
		}
	}

	if (info.trainer) {
		BYTE *here = prg.ram.data;

		if (prg.ram_plus) {
			here = prg.ram_plus;
		}

		memcpy(here + 0x1000, &trainer.data, sizeof(trainer.data));
	}
}
BYTE map_prg_ram_malloc(WORD size) {
	prg.ram.size = size;

	if (!(prg.ram.data = (BYTE *)malloc(prg.ram.size))) {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void map_prg_ram_memset(void) {
	int value = 0x00;

	if (info.mapper.id == FDS_MAPPER) {
		value = 0xEA;
	}

	memset(prg.ram.data, value, prg.ram.size);
}
void map_prg_ram_battery_save(void) {
	/* se c'e' della PRG Ram battery packed la salvo in un file */
	if (info.prg.ram.bat.banks) {
		uTCHAR prg_ram_file[LENGTH_FILE_NAME_LONG], basename[255], *fl, *last_dot;
		FILE *fp;

		fl = info.rom.file;

		gui_utf_basename(fl, basename, usizeof(basename));
		usnprintf(prg_ram_file, usizeof(prg_ram_file), uL("" uPERCENTs PRB_FOLDER "/" uPERCENTs), info.base_folder, basename);

		/* rintraccio l'ultimo '.' nel nome */
		if ((last_dot = ustrrchr(prg_ram_file, uL('.')))) {
			/* elimino l'estensione */
			(*last_dot) = 0x00;
		}
		/* aggiungo l'estensione prb */
		ustrcat(prg_ram_file, uL(".prb"));
		/* apro il file */
		fp = ufopen(prg_ram_file, uL("wb"));
		if (fp) {
			if (extcl_battery_io) {
				extcl_battery_io(WR_BAT, fp);
			} else {
				mapper_wr_battery_default();
			}

			/* forzo la scrittura del file */
			fflush(fp);

			/* chiudo */
			fclose(fp);
		}
	}
}
BYTE map_chr_chip_malloc(BYTE index, size_t size, BYTE set_value) {
	if (chr_chip(index)) {
		free(chr_chip(index));
		chr_chip(index) = NULL;
	}

	chr_chip_size(index) = size;
	info.chr.chips++;

	if ((chr_chip(index) = (BYTE *)malloc(size))) {
		memset(chr_chip(index), set_value, size);
	} else {
		fprintf(stderr, "Out of memory\n");
		return (EXIT_ERROR);
	}

	return (EXIT_OK);
}
void map_chr_bank_1k_reset(void) {
	BYTE bank1k, bnk;

	for (bank1k = 0; bank1k < 8; ++bank1k) {
		chr.rom_chip[bank1k] = 0;

		bnk = bank1k;
		_control_bank(bnk, info.chr.rom[0].max.banks_1k)
		chr.bank_1k[bank1k] = chr_chip_byte_pnt(0, bnk * 0x0400);
	}
}
BYTE map_chr_ram_init(void) {
	if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (mapper.write_vram) {
			if (chr_chip(0)) {
				free(chr_chip(0));
			}
			/* alloco la CHR Rom */
			if (map_chr_chip_malloc(0, chr_ram_size(), 0x00) == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			map_chr_bank_1k_reset();
		}
	}

	return (EXIT_OK);
}
BYTE map_chr_ram_extra_init(uint32_t size) {
	if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP))) {
		if (chr.extra.data) {
			free(chr.extra.data);
			chr.extra.size = 0;
		}
		/* alloco la CHR Ram extra */
		if (!(chr.extra.data = (BYTE *)malloc(size))) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}
		chr.extra.size = size;
		memset(chr.extra.data, 0x00, chr.extra.size);
	}

	return (EXIT_OK);
}
void map_chr_ram_extra_reset(void) {
	if (chr.extra.data) {
		memset(chr.extra.data, 0x00, chr.extra.size);
	}
}
void map_set_banks_max_prg(BYTE chip) {
	info.prg.rom[chip].max.banks_32k = (info.prg.rom[chip].banks_16k == 1) ? 0 :
		((info.prg.rom[chip].banks_16k >> 1) ? (info.prg.rom[chip].banks_16k >> 1) - 1 : 0);
	info.prg.rom[chip].max.banks_16k =
		info.prg.rom[chip].banks_16k ? info.prg.rom[chip].banks_16k - 1 : 0;
	info.prg.rom[chip].max.banks_8k =
		info.prg.rom[chip].banks_8k ? info.prg.rom[chip].banks_8k - 1 : 0;
	info.prg.rom[chip].max.banks_8k_before_last =
		(info.prg.rom[chip].banks_8k > 1) ? info.prg.rom[chip].banks_8k - 2 : 0;
	info.prg.rom[chip].max.banks_4k =
		((info.prg.rom[chip].banks_8k << 1) != 0) ? (info.prg.rom[chip].banks_8k << 1) - 1 : 0;
	info.prg.rom[chip].max.banks_2k =
		((info.prg.rom[chip].banks_8k << 2) != 0) ? (info.prg.rom[chip].banks_8k << 2) - 1 : 0;
}
void map_set_banks_max_chr(BYTE chip) {
	info.chr.rom[chip].max.banks_8k =
		info.chr.rom[chip].banks_8k ? info.chr.rom[chip].banks_8k - 1 : 0;
	info.chr.rom[chip].max.banks_4k =
		info.chr.rom[chip].banks_4k ? info.chr.rom[chip].banks_4k - 1 : 0;
	info.chr.rom[chip].max.banks_2k =
		((info.chr.rom[chip].banks_1k >> 1) != 0) ? (info.chr.rom[chip].banks_1k >> 1) - 1 : 0;
	info.chr.rom[chip].max.banks_1k =
		info.chr.rom[chip].banks_1k ? info.chr.rom[chip].banks_1k - 1 : 0;
}
