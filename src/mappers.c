/*
 * mappers.c
 *
 *  Created on: 11/mag/2010
 *      Author: fhorse
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mappers.h"
#include "memmap.h"
#include "irqA12.h"
#include "irql2f.h"
#include "tas.h"
#include "sdltext.h"

BYTE mapInit(WORD mapperType) {
	BYTE i;
	/*
	 * di default la routine di salvataggio
	 * di una possibile struttura interna
	 * di dati della mapper e' NULL.
	 */
	for (i = 0; i < LENGTH(mapper.intStruct); i++) {
		mapper.intStruct[i] = 0;
	}
	/* disabilito gli accessori */
	irqA12.present = FALSE;
	irql2f.present = FALSE;
	/* disabilito tutte le chiamate relative alle mappers */
	extclInit();

	switch (mapperType) {
		case 0:
			mapInit_0();
			break;
		case 1:
			mapInit_MMC1();
			break;
		case 2:
			if (info.mapperType == UNLROM) {
				mapInit_UxROM(UNLROM);
			} else {
				mapInit_UxROM(UXROM);
			}
			break;
		case 3:
			mapInit_CNROM(CNROM);
			break;
		case 4:
			mapInit_MMC3();
			break;
		case 5:
			mapInit_MMC5();
			break;
		case 7:
			mapInit_AxROM();
			break;
			/* per MMC2 e MMC4 uso le stesse routine */
		case 9:
		case 10:
			mapInit_MMC2and4();
			break;
		case 11:
			mapInit_ColorDreams();
			break;
		case 12:
			mapInit_Rex(DBZ);
			break;
		case 13:
			mapInit_CPROM();
			break;
		case 15:
			mapInit_Waixing(WPSX);
			break;
		case 16: {
			switch (info.mapperType) {
				case E24C02:
					mapInit_Bandai(E24C02);
					break;
				case DATACH:
					mapInit_Bandai(DATACH);
					break;
				default:
					mapInit_Bandai(FCGx);
					break;
			}
			break;
		}
		case 18:
			mapInit_Jaleco(SS8806);
			break;
		case 19:
			mapInit_Namco(N163);
			break;
		case 21:
			mapInit_VRC4(info.mapperType == DEFAULT ? VRC4A : info.mapperType);
			break;
		case 22:
			mapInit_VRC2(VRC2A);
			break;
		case 23:
			if (info.mapperType == VRC4BMC) {
				mapInit_VRC4BMC();
			} else if (info.mapperType == VRC4E) {
				mapInit_VRC4(VRC4E);
			} else {
				mapInit_VRC2(VRC2B);
			}
			break;
		case 24:
			mapInit_VRC6(VRC6A);
			break;
		case 25:
			mapInit_VRC4(info.mapperType == DEFAULT ? VRC4B : info.mapperType);
			break;
		case 26:
			mapInit_VRC6(VRC6B);
			break;
		case 32:
			mapInit_Irem(G101);
			break;
		case 33:
			mapInit_Taito(TC0190FMC);
			break;
		case 34:
			mapInit_BxROM();
			break;
		case 37:
			mapInit_37();
			break;
		case 38:
			mapInit_74x138x161();
			break;
		case 41:
			mapInit_Caltron();
			break;
		case 44:
			mapInit_44();
			break;
		case 45:
			mapInit_45();
			break;
		case 46:
			mapInit_46();
			break;
		case 47:
			mapInit_47();
			break;
		case 49:
			mapInit_49();
			break;
		case 50:
			mapInit_50();
			break;
		case 51:
			mapInit_51();
			break;
		case 52:
			mapInit_52();
			break;
		case 53:
			mapInit_53();
			break;
		case 56:
			mapInit_Kaiser(KS202);
			break;
		case 57:
			mapInit_57();
			break;
		case 58:
			mapInit_58();
			break;
		case 60:
			if (info.mapperType == M60VT5201) {
				mapInit_60_vt5201();
			} else {
				mapInit_60();
			}
			break;
		case 61:
			mapInit_61();
			break;
		case 62:
			mapInit_62();
			break;
		case 64:
			mapInit_Tengen(TRAMBO);
			break;
		case 65:
			mapInit_Irem(H3000);
			break;
		case 66:
			mapInit_GxROM();
			break;
		case 67:
			mapInit_Sunsoft(SUN3);
			break;
		case 68:
			mapInit_Sunsoft(SUN4);
			break;
		case 69:
			mapInit_Sunsoft(FM7);
			break;
		case 70:
			mapInit_74x161x161x32(IC74X161X161X32A);
			break;
		case 71:
			mapInit_Camerica();
			break;
		case 72:
			mapInit_Jaleco(JF17);
			break;
		case 73:
			mapInit_VRC3();
			break;
		case 74:
			mapInit_Waixing(WTA);
			break;
		case 75:
			mapInit_VRC1();
			break;
		case 76:
			mapInit_Namco(N3446);
			break;
		case 77:
			mapInit_Irem(LROG017);
			break;
		case 78:
			mapInit_Jaleco(JF16);
			break;
		case 79:
			mapInit_Ave(NINA06);
			break;
		case 80:
			if (info.mapperType == X1005B) {
				mapInit_Taito(X1005B);
			} else {
				mapInit_Taito(X1005A);
			}
			break;
		case 82:
			mapInit_Taito(X1017);
			break;
		case 85:
			if (info.mapperType == VRC7A) {
				mapInit_VRC7(VRC7A);
			} else {
				mapInit_VRC7(VRC7B);
			}
			break;
		case 86:
			mapInit_Jaleco(JF13);
			break;
		case 87:
			mapInit_Jaleco(JF05);
			break;
		case 88:
			mapInit_Namco(N3433);
			break;
		case 89:
			mapInit_Sunsoft(SUN2B);
			break;
		case 92:
			mapInit_Jaleco(JF19);
			break;
		case 93:
			mapInit_Sunsoft(SUN2A);
			break;
		case 94:
			mapInit_UxROM(UNL1XROM);
			break;
		case 95:
			mapInit_Namco(N3425);
			break;
		case 96:
			mapInit_Bandai(B161X02X74);
			break;
		case 97:
			mapInit_Irem(TAMS1);
			break;
		case 99:
			mapInit_Vs();
			break;
		case 107:
			mapInit_Magic();
			break;
		case 108:
			mapInit_Whirlwind();
			break;
		case 112:
			mapInit_Ntdec(ASDER);
			break;
		case 113:
			mapInit_Hes();
			break;
		case 114:
			mapInit_114();
			break;
		case 115:
			mapInit_Kasing();
			break;
		case 116:
			mapInit_116();
			break;
		case 117:
			mapInit_Futuremedia();
			break;
		case 118:
			if (info.mapperType == TKSROM) {
				mapInit_TxROM(TKSROM);
			} else {
				mapInit_TxROM(TLSROM);
			}
			break;
		case 119:
			mapInit_TxROM(TQROM);
			break;
		case 120:
			mapInit_120();
			break;
		case 121:
			mapInit_121();
			break;
		case 132:
			mapInit_Txc(T22211A);
			break;
		case 133:
			mapInit_Sachen(SA72008);
			break;
		case 136:
			mapInit_Sachen(TCU02);
			break;
		case 137:
			mapInit_Sachen(SA8259D);
			break;
		case 138:
			mapInit_Sachen(SA8259B);
			break;
		case 139:
			mapInit_Sachen(SA8259C);
			break;
		case 140:
			mapInit_Jaleco(JF11);
			break;
		case 141:
			mapInit_Sachen(SA8259A);
			break;
		case 142:
			mapInit_Kaiser(KS7032);
			break;
		case 143:
			mapInit_Sachen(TCA01);
			break;
		case 144:
			mapInit_Agci();
			break;
		case 145:
			mapInit_Sachen(SA72007);
			break;
		case 146:
			mapInit_Ave(NINA06);
			break;
		case 147:
			mapInit_Sachen(TCU01);
			break;
		case 148:
			mapInit_Sachen(SA0037);
			break;
		case 149:
			mapInit_Sachen(SA0036);
			break;
		case 150:
			mapInit_Sachen(SA74374B);
			break;
		case 152:
			mapInit_74x161x161x32(IC74X161X161X32B);
			break;
		case 154:
			mapInit_Namco(N3453);
			break;
		case 156:
			mapInit_156();
			break;
		case 158:
			mapInit_Tengen(T800037);
			break;
		case 159:
			mapInit_Bandai(E24C01);
			break;
		case 165:
			mapInit_Waixing(SH2);
			break;
		case 171:
			mapInit_Kaiser(KS7058);
			break;
		case 172:
			mapInit_Txc(T22211B);
			break;
		case 173:
			mapInit_Txc(T22211C);
			break;
		case 175:
			mapInit_Kaiser(KS7022);
			break;
		case 176:
			mapInit_176();
			break;
		case 177:
			if (info.mapperType != DEFAULT) {
				/* questa e' la mappers 179 in nestopia */
				mapInit_Hen(info.mapperType);
			} else {
				mapInit_Hen(HEN177);
			}
			break;
		case 178:
			if (info.mapperType != DEFAULT) {
				mapInit_178(info.mapperType);
			} else {
				mapInit_178(M178);
			}
			break;
		case 180:
			mapInit_UxROM(UNROM180);
			break;
		case 182:
			mapInit_182();
			break;
		case 183:
			mapInit_183();
			break;
		case 184:
			mapInit_Sunsoft(SUN1);
			break;
		case 185:
			mapInit_CNROM(CNROMCNFL);
			break;
		case 186:
			mapInit_186();
			break;
		case 189:
			mapInit_Txc(TXCTW);
			break;
		case 191:
			mapInit_Waixing(WTB);
			break;
		case 192:
			mapInit_Waixing(WTC);
			break;
		case 193:
			mapInit_Ntdec(FHERO);
			break;
		case 194:
			mapInit_Waixing(WTD);
			break;
		case 195:
			mapInit_Waixing(WTE);
			break;
		case 199:
			mapInit_Waixing(WTG);
			break;
		case 200:
			mapInit_200();
			break;
		case 201:
			mapInit_201();
			break;
		case 202:
			mapInit_202();
			break;
		case 203:
			mapInit_203();
			break;
		case 204:
			mapInit_204();
			break;
		case 205:
			mapInit_205();
			break;
		case 206:
			mapInit_Namco(N3416);
			break;
		/*
		 * la mapper 207 la tratto come mapper 80
		 * case 207:
		 */
		case 208:
			mapInit_208();
			break;
		case 212:
			mapInit_212();
			break;
		case 213:
			mapInit_213();
			break;
		case 214:
			mapInit_214();
			break;
		case 215:
			mapInit_215();
			break;
		case 216:
			mapInit_Rcm(GS2015);
			break;
		case 217:
			mapInit_217();
			break;
		case 219:
			mapInit_219();
			break;
		case 221:
			mapInit_221();
			break;
		case 222:
			mapInit_222();
			break;
		case 225:
			mapInit_225();
			break;
		case 226:
			mapInit_226();
			break;
		case 227:
			mapInit_227();
			break;
		case 228:
			mapInit_Active();
			break;
		case 229:
			mapInit_229();
			break;
		case 230:
			mapInit_230();
			break;
		case 231:
			mapInit_231();
			break;
		case 232:
			info.mapperType = BF9096;
			mapInit_Camerica();
			break;
		case 233:
			mapInit_233();
			break;
		case 234:
			mapInit_Ave(D1012);
			break;
		case 235:
			mapInit_235();
			break;
		case 240:
			mapInit_240();
			break;
		case 241:
			mapInit_241();
			break;
		case 242:
			mapInit_242();
			break;
		case 243:
			mapInit_Sachen(SA74374A);
			break;
		case 244:
			mapInit_244();
			break;
		case 245:
			mapInit_Waixing(WTH);
			break;
		case 246:
			mapInit_246();
			break;
		default:
			textAddLineInfo(1, "[yellow]Mapper %d not supported", mapperType);
			fprintf(stderr, "Mapper not supported\n");
			EXTCLCPUWRMEM(0);
			break;
		/* casi speciali */
		case FDS_MAPPER:
			mapInit_FDS();
			break;
		case GAMEGENIE_MAPPER:
			mapInit_GameGenie();
			break;
	}
	mapPrgRom8kUpdate();
	mapPrgRamInit();
	if (mapChrRamInit()) {
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
void mapQuit(void) {
	/* se c'e' della PRG Ram battery packed la salvo in un file */
	if (info.prgRamBatBanks) {
		char prgRamFile[1024], *lastDot;
		FILE *fp;

		/* copio il nome del file nella variabile */
		strcpy(prgRamFile, info.romFile);
		/* rintraccio l'ultimo '.' nel nome */
		lastDot = strrchr(prgRamFile, '.');
		/* elimino l'estensione */
		*lastDot = 0x00;
		/* aggiungo l'estensione prb */
		strcat(prgRamFile, ".prb");
		/* apro il file */
		fp = fopen(prgRamFile, "wb");
		if (fp) {
			if (extclBatteryIO) {
				extclBatteryIO(WRBAT, fp);
			} else {
				mapperWrBatteryDefault();
			}
			/* chiudo */
			fclose(fp);
		}
	}

	info.mapper = 0;
	info.mapperType = 0;
	info.mapperExtendWrite = 0;
	info.mapperExtendRead = 0;
	info.id = 0;
	memset(info.sha1sum, 0, sizeof(info.sha1sum));
	memset(info.sha1sumString, 0, sizeof(info.sha1sumString));
	memset(info.sha1sumChr, 0, sizeof(info.sha1sumChr));
	memset(info.sha1sumStringChr, 0, sizeof(info.sha1sumStringChr));
	info.chrRom8kCount = 0;
	info.chrRom4kCount = 0;
	info.chrRom1kCount = 0;
	info.prgRom16kCount = 0;
	info.prgRom8kCount = 0;
	info.prgRamPlus8kCount = 0;
	info.prgRamBatBanks = 0;
	info.prgRamBatStart = DEFAULT;

	/* PRG */
	if (prg.rom) {
		free(prg.rom);
	}
	if (prg.ram) {
		free(prg.ram);
	}
	if (prg.ramPlus) {
		free(prg.ramPlus);
	}
	prg.rom = NULL;
	memset(prg.rom8k, 0, sizeof(prg.rom8k));
	prg.ram = NULL;
	prg.ramPlus = NULL;
	prg.ramPlus8k = NULL;
	prg.ramBattery = NULL;

	/* CHR */
	if (chr.data) {
		free(chr.data);
	}
	chr.data = NULL;
	memset(chr.bank1k, 0, sizeof(chr.bank1k));

	mirroring_V();

	mapper.writeVRAM = FALSE;
}
void mapPrgRom8k(BYTE banks8k, BYTE at, BYTE value) {
	BYTE a;

	for (a = 0; a < banks8k; ++a) {
		mapper.romMapTo[at + a] = ((value * banks8k) + a);
	}
}
void mapPrgRom8kReset(void) {
	mapper.romMapTo[0] = 0;
	mapper.romMapTo[1] = 1;
	mapper.romMapTo[2] = info.prgRom8kCount - 2;
	mapper.romMapTo[3] = info.prgRom8kCount - 1;
}
void mapPrgRom8kUpdate(void) {
	BYTE i;

	for (i = 0; i < 4; ++i) {
		prg.rom8k[i] = &prg.rom[mapper.romMapTo[i] << 13];
	}
}
void mapPrgRamInit(void) {
	/*
	 * se non ci sono stati settaggi particolari della mapper
	 * e devono esserci banchi di PRG Ram extra allora li assegno.
	 */
	if (((info.reset == CHANGEROM) || (info.reset == POWERUP)) && info.prgRamPlus8kCount
	        && !prg.ramPlus) {
		/* alloco la memoria necessaria */
		prg.ramPlus = malloc(prgRamPlusSize());
		/* inizializzo */
		memset(prg.ramPlus, 0x00, prgRamPlusSize());
		/* gli 8k iniziali */
		prg.ramPlus8k = &prg.ramPlus[0];
		/* controllo se la rom ha una RAM PRG battery packed */
		if (info.prgRamBatBanks && !tas.type) {
			char prgRamFile[1024], *lastDot;
			FILE *fp;

			/* copio il nome del file nella variabile */
			strcpy(prgRamFile, info.romFile);
			/* rintraccio l'ultimo '.' nel nome */
			lastDot = strrchr(prgRamFile, '.');
			/* elimino l'estensione */
			*lastDot = 0x00;
			/* aggiungo l'estensione prb */
			strcat(prgRamFile, ".prb");
			/* provo ad aprire il file */
			fp = fopen(prgRamFile, "rb");

			if (extclBatteryIO) {
				extclBatteryIO(RDBAT, fp);
			} else {
				mapperRdBatteryDefault();
			}
			/* chiudo il file */
			if (fp) {
				fclose(fp);
			}
		}
	}
}
BYTE mapChrRamInit(void) {
	if (((info.reset == CHANGEROM) || (info.reset == POWERUP)) && mapper.writeVRAM) {
		if (chr.data) {
			free(chr.data);
		}
		/* alloco la CHR Rom */
		if (!(chr.data = malloc(chrRamSize()))) {
			fprintf(stderr, "Out of memory\n");
			return (EXIT_ERROR);
		}
		chrBank1kReset()
		memset(chr.data, 0x00, chrRamSize());
	}

	return (EXIT_OK);
}
