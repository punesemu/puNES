/*
 * mappers.c
 *
 *  Created on: 11/mag/2010
 *      Author: fhorse
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "mappers.h"
#include "mem_map.h"
#include "irqA12.h"
#include "irql2f.h"
#include "tas.h"
#include "text.h"
#include "uncompress.h"

BYTE map_init(WORD mapper_type) {
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

	switch (mapper_type) {
		case 0:
			map_init_0();
			break;
		case 1:
			map_init_MMC1();
			break;
		case 2:
			if (info.mapper_type == UNLROM) {
				map_init_UxROM(UNLROM);
			} else if (info.mapper_type == UNROM_BK2) {
				map_init_UxROM(UNROM_BK2);
			} else {
				map_init_UxROM(UXROM);

			}
			break;
		case 3:
			map_init_CNROM(CNROM);
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
			switch (info.mapper_type) {
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
			map_init_VRC4(info.mapper_type == DEFAULT ? VRC4A : info.mapper_type);
			break;
		case 22:
			map_init_VRC2(VRC2A);
			break;
		case 23:
			if (info.mapper_type == VRC4BMC) {
				map_init_VRC4BMC();
			} else if (info.mapper_type == VRC4E) {
				map_init_VRC4(VRC4E);
			} else {
				map_init_VRC2(VRC2B);
			}
			break;
		case 24:
			map_init_VRC6(VRC6A);
			break;
		case 25:
			map_init_VRC4(info.mapper_type == DEFAULT ? VRC4B : info.mapper_type);
			break;
		case 26:
			map_init_VRC6(VRC6B);
			break;
		case 28:
			map_init_28();
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
		case 37:
			map_init_37();
			break;
		case 38:
			map_init_74x138x161();
			break;
		case 41:
			map_init_Caltron();
			break;
		case 42:
			map_init_42();
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
			map_init_52();
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
			if (info.mapper_type == MAP60_VT5201) {
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
			if (info.mapper_type == X1005B) {
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
			if (info.mapper_type == VRC7A) {
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
			if (info.mapper_type == TKSROM) {
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
		case 132:
			map_init_Txc(T22211A);
			break;
		case 133:
			map_init_Sachen(SA72008);
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
		case 152:
			map_init_74x161x161x32(IC74X161X161X32B);
			break;
		case 154:
			map_init_Namco(N3453);
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
		case 163:
			map_init_163();
			break;
		case 164:
			map_init_164();
			break;
		case 165:
			map_init_Waixing(SH2);
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
			map_init_176();
			break;
		case 177:
			if (info.mapper_type != DEFAULT) {
				/* questa e' la mappers 179 in nestopia */
				map_init_Hen(info.mapper_type);
			} else {
				map_init_Hen(HEN_177);
			}
			break;
		case 178:
			if (info.mapper_type != DEFAULT) {
				map_init_178(info.mapper_type);
			} else {
				map_init_178(MAP178);
			}
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
			map_init_CNROM(CNROM_CNFL);
			break;
		case 186:
			map_init_186();
			break;
		case 189:
			map_init_Txc(TXCTW);
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
		/*
		 * la mapper 207 la tratto come mapper 80
		 * case 207:
		 */
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
			info.mapper_type = BF9096;
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
		default:
			text_add_line_info(1, "[yellow]Mapper %d not supported", mapper_type);
			fprintf(stderr, "Mapper not supported\n");
			EXTCL_CPU_WR_MEM(0);
			break;
		/* casi speciali */
		case FDS_MAPPER:
			map_init_FDS();
			break;
		case GAMEGENIE_MAPPER:
			map_init_GameGenie();
			break;
	}
	map_prg_rom_8k_update();
	map_prg_ram_init();
	if (map_chr_ram_init()) {
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
void map_quit(void) {
	/* se c'e' della PRG Ram battery packed la salvo in un file */
	if (info.prg_ram_bat_banks) {
		char prg_ram_file[1024], *last_dot;
		FILE *fp;

		/* copio il nome del file nella variabile */
		if (info.uncompress_rom == TRUE) {
			strcpy(prg_ram_file, uncomp.compress_archive);
			dirname(prg_ram_file);
			strcat(prg_ram_file, "/");
			strcat(prg_ram_file, basename(uncomp.uncompress_file));
		} else {
			strcpy(prg_ram_file, info.rom_file);
		}
		/* rintraccio l'ultimo '.' nel nome */
		last_dot = strrchr(prg_ram_file, '.');
		/* elimino l'estensione */
		*last_dot = 0x00;
		/* aggiungo l'estensione prb */
		strcat(prg_ram_file, ".prb");
		/* apro il file */
		fp = fopen(prg_ram_file, "wb");
		if (fp) {
			if (extcl_battery_io) {
				extcl_battery_io(WR_BAT, fp);
			} else {
				mapper_wr_battery_default();
			}
			/* chiudo */
			fclose(fp);
		}
	}

	info.mapper = 0;
	info.mapper_type = 0;
	info.mapper_extend_wr = 0;
	info.mapper_extend_rd = 0;
	info.id = 0;
	memset(info.sha1sum, 0, sizeof(info.sha1sum));
	memset(info.sha1sum_string, 0, sizeof(info.sha1sum_string));
	memset(info.sha1sum_chr, 0, sizeof(info.sha1sum_chr));
	memset(info.sha1sum_string_chr, 0, sizeof(info.sha1sum_string_chr));
	info.chr_rom_8k_count = 0;
	info.chr_rom_4k_count = 0;
	info.chr_rom_1k_count = 0;
	info.prg_rom_16k_count = 0;
	info.prg_rom_8k_count = 0;
	info.prg_ram_plus_8k_count = 0;
	info.prg_ram_bat_banks = 0;
	info.prg_ram_bat_start = DEFAULT;

	/* PRG */
	if (prg.rom) {
		free(prg.rom);
	}
	if (prg.ram) {
		free(prg.ram);
	}
	if (prg.ram_plus) {
		free(prg.ram_plus);
	}
	prg.rom = NULL;
	memset(prg.rom_8k, 0, sizeof(prg.rom_8k));
	prg.ram = NULL;
	prg.ram_plus = NULL;
	prg.ram_plus_8k = NULL;
	prg.ram_battery = NULL;

	/* CHR */
	if (chr.data) {
		free(chr.data);
	}
	chr.data = NULL;
	memset(chr.bank_1k, 0, sizeof(chr.bank_1k));

	mirroring_V();

	mapper.write_vram = FALSE;
}
void map_prg_rom_8k(BYTE banks_8k, BYTE at, BYTE value) {
	BYTE a;

	for (a = 0; a < banks_8k; ++a) {
		mapper.rom_map_to[at + a] = ((value * banks_8k) + a);
	}
}
void map_prg_rom_8k_reset(void) {
	mapper.rom_map_to[0] = 0;
	mapper.rom_map_to[1] = 1;
	mapper.rom_map_to[2] = info.prg_rom_8k_count - 2;
	mapper.rom_map_to[3] = info.prg_rom_8k_count - 1;
}
void map_prg_rom_8k_update(void) {
	BYTE i;

	for (i = 0; i < 4; ++i) {
		prg.rom_8k[i] = &prg.rom[mapper.rom_map_to[i] << 13];
	}
}
void map_prg_ram_init(void) {
	/*
	 * se non ci sono stati settaggi particolari della mapper
	 * e devono esserci banchi di PRG Ram extra allora li assegno.
	 */
	if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) && info.prg_ram_plus_8k_count
	        && !prg.ram_plus) {
		/* alloco la memoria necessaria */
		prg.ram_plus = (BYTE *) malloc(prg_ram_plus_size());
		/* inizializzo */
		memset(prg.ram_plus, 0x00, prg_ram_plus_size());
		/* gli 8k iniziali */
		prg.ram_plus_8k = &prg.ram_plus[0];
		/* controllo se la rom ha una RAM PRG battery packed */
		if (info.prg_ram_bat_banks && !tas.type) {
			char prg_ram_file[1024], *last_dot;
			FILE *fp;

			/* copio il nome del file nella variabile */
			if (info.uncompress_rom == TRUE) {
				strcpy(prg_ram_file, uncomp.compress_archive);
				dirname(prg_ram_file);
				strcat(prg_ram_file, "/");
				strcat(prg_ram_file, basename(uncomp.uncompress_file));
			} else {
				strncpy(prg_ram_file, info.rom_file, sizeof(prg_ram_file));
			}

			/* rintraccio l'ultimo '.' nel nome */
			last_dot = strrchr(prg_ram_file, '.');
			/* elimino l'estensione */
			*last_dot = 0x00;
			/* aggiungo l'estensione prb */
			strcat(prg_ram_file, ".prb");
			/* provo ad aprire il file */
			fp = fopen(prg_ram_file, "rb");
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
	}
	if (info.trainer) {
		BYTE *here = prg.ram;

		if (prg.ram_plus) {
			here = prg.ram_plus;
		}

		memcpy(here + 0x1000, &trainer.data, sizeof(trainer.data));
	}
}
BYTE map_chr_ram_init(void) {
	if (((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) && mapper.write_vram) {
		if (chr.data) {
			free(chr.data);
		}
		/* alloco la CHR Rom */
		if (!(chr.data = (BYTE *) malloc(chr_ram_size()))) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}
		chr_bank_1k_reset()
		memset(chr.data, 0x00, chr_ram_size());
	}

	return (EXIT_OK);
}
