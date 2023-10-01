/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "mappers.h"
#include "info.h"
#include "irqA12.h"
#include "gui.h"
#include "vs_system.h"
#include "nes20db.h"
#include "conf.h"

_mapper mapper;

BYTE map_init(void) {
	// di default la routine di salvataggio
	// di una possibile struttura interna
	// di dati della mapper e' NULL.
	for (unsigned int i = 0; i < LENGTH(mapper.internal_struct); i++) {
		mapper.internal_struct[i] = 0;
	}
	// disabilito gli accessori
	for (int nesidx = 0; nesidx < NES_CHIPS_MAX; nesidx++) {
		nes[nesidx].irqA12.present = FALSE;
		nes[nesidx].irql2f.present = FALSE;
	}
	// disabilito tutte le chiamate relative alle mappers
	extcl_init();

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		dipswitch_search();
		gui_update();
		gui_dipswitch_dialog();
		if (vs_system.enabled && (cfg->dipswitch == -1)) {
			cfg->dipswitch = 0;
		}
	}

	info.mapper.supported = TRUE;

	switch (info.mapper.id) {
		case 0:
			map_init_000();
			break;
		case 1:
			map_init_001();
			break;
		case 2:
			map_init_002();
			break;
		case 3:
			map_init_003();
			break;
		case 4:
			map_init_004();
			break;
		case 5:
			map_init_005();
			break;
		case 6:
			map_init_006();
			break;
		case 7:
			map_init_007();
			break;
		case 8:
			map_init_006();
			break;
		case 9:
			map_init_009();
			break;
		case 10:
			map_init_010();
			break;
		case 11:
			map_init_011();
			break;
		case 12:
			map_init_012();
			break;
		case 13:
			map_init_013();
			break;
		case 14:
			map_init_014();
			break;
		case 15:
			map_init_015();
			break;
		case 16:
			map_init_016();
			break;
		case 17:
			map_init_006();
			break;
		case 18:
			map_init_018();
			break;
		case 19:
			map_init_019();
			break;
		// case 20:
		// 	mapper 20 e' l'FDS
		//	break;
		case 21:
			map_init_021();
			break;
		case 22:
			map_init_022();
			break;
		case 23:
			map_init_023();
			break;
		case 24:
			map_init_024();
			break;
		case 25:
			map_init_025();
			break;
		case 26:
			map_init_026();
			break;
		case 27:
			map_init_027();
			break;
		case 28:
			map_init_028();
			break;
		case 29:
			map_init_029();
			break;
		case 30:
			map_init_030();
			break;
		case 31:
			map_init_031();
			break;
		case 32:
			map_init_032();
			break;
		case 33:
			map_init_033();
			break;
		case 34:
			map_init_034();
			break;
		case 35:
			map_init_209();
			break;
		case 36:
			map_init_036();
			break;
		case 37:
			map_init_037();
			break;
		case 38:
			map_init_038();
			break;
		case 40:
			map_init_040();
			break;
		case 41:
			map_init_041();
			break;
		case 42:
			map_init_042();
			break;
		case 43:
			map_init_043();
			break;
		case 44:
			map_init_044();
			break;
		case 45:
			map_init_045();
			break;
		case 46:
			map_init_046();
			break;
		case 47:
			map_init_047();
			break;
		case 48:
			map_init_048();
			break;
		case 49:
			map_init_049();
			break;
		case 50:
			map_init_050();
			break;
		case 51:
			map_init_051();
			break;
		case 52:
			map_init_052();
			break;
		case 53:
			map_init_053();
			break;
		case 55:
			map_init_055();
			break;
		case 56:
			map_init_056();
			break;
		case 57:
			map_init_057();
			break;
		case 58:
			map_init_058();
			break;
		case 59:
			map_init_059();
			break;
		case 60:
			map_init_060();
			break;
		case 61:
			map_init_061();
			break;
		case 62:
			map_init_062();
			break;
		case 63:
			map_init_063();
			break;
		case 64:
			map_init_064();
			break;
		case 65:
			map_init_065();
			break;
		case 66:
			map_init_066();
			break;
		case 67:
			map_init_067();
			break;
		case 68:
			map_init_068();
			break;
		case 69:
			map_init_069();
			break;
		case 70:
			map_init_070();
			break;
		case 71:
			map_init_071();
			break;
		case 72:
			map_init_072();
			break;
		case 73:
			map_init_073();
			break;
		case 74:
			map_init_074();
			break;
		case 75:
			map_init_075();
			break;
		case 76:
			map_init_076();
			break;
		case 77:
			map_init_077();
			break;
		case 78:
			map_init_078();
			break;
		case 79:
			map_init_079();
			break;
		case 80:
			map_init_080();
			break;
		case 81:
			map_init_081();
			break;
		case 82:
			map_init_082();
			break;
		case 83:
			map_init_083();
			break;
		case 85:
			map_init_085();
			break;
		case 86:
			map_init_086();
			break;
		case 87:
			map_init_087();
			break;
		case 88:
			map_init_088();
			break;
		case 89:
			map_init_089();
			break;
		case 90:
			map_init_209();
			break;
		case 91:
			map_init_091();
			break;
		case 92:
			map_init_072();
			break;
		case 93:
			map_init_093();
			break;
		case 94:
			map_init_094();
			break;
		case 95:
			map_init_095();
			break;
		case 96:
			map_init_096();
			break;
		case 97:
			map_init_097();
			break;
		case 99:
			map_init_099();
			break;
		case 100:
			map_init_100();
			break;
		case 101:
			map_init_101();
			break;
		case 103:
			map_init_103();
			break;
		case 104:
			map_init_104();
			break;
		case 105:
			map_init_105();
			break;
		case 106:
			map_init_106();
			break;
		case 107:
			map_init_107();
			break;
		case 108:
			map_init_108();
			break;
		case 111:
			map_init_111();
			break;
		case 112:
			map_init_112();
			break;
		case 113:
			map_init_113();
			break;
		case 114:
			map_init_114();
			break;
		case 115:
			map_init_115();
			break;
		case 116:
			map_init_116();
			break;
		case 117:
			map_init_117();
			break;
		case 118:
			map_init_118();
			break;
		case 119:
			map_init_119();
			break;
		case 120:
			map_init_120();
			break;
		case 121:
			map_init_121();
			break;
		case 122:
			// 122 e 184 sono la stessa cosa
			map_init_184();
			break;
		case 123:
			map_init_123();
			break;
		case 125:
			map_init_125();
			break;
		case 126:
			map_init_126();
			break;
		case 132:
			map_init_132();
			break;
		case 133:
			map_init_133();
			break;
		case 134:
			map_init_134();
			break;
		case 136:
			map_init_136();
			break;
		case 137:
			map_init_137();
			break;
		case 138:
			map_init_138();
			break;
		case 139:
			map_init_139();
			break;
		case 140:
			map_init_140();
			break;
		case 141:
			map_init_141();
			break;
		case 142:
			map_init_142();
			break;
		case 143:
			map_init_143();
			break;
		case 144:
			map_init_144();
			break;
		case 145:
			map_init_145();
			break;
		case 146:
			map_init_079();
			break;
		case 147:
			map_init_147();
			break;
		case 148:
			map_init_148();
			break;
		case 149:
			map_init_149();
			break;
		case 150:
			map_init_150();
			break;
		case 151:
			map_init_075();
			break;
		case 152:
			map_init_152();
			break;
		case 153:
			// Famicom Jump II - Saikyou no 7 Nin (J) [!].nes
			map_init_153();
			break;
		case 154:
			map_init_154();
			break;
		case 155:
			map_init_001();
			break;
		case 156:
			map_init_156();
			break;
		case 157:
			map_init_157();
			break;
		case 158:
			map_init_064();
			break;
		case 159:
			map_init_159();
			break;
		case 162:
			map_init_162();
			break;
		case 163:
			map_init_163();
			break;
		case 164:
			map_init_164();
			break;
		case 165:
			map_init_165();
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
			map_init_171();
			break;
		case 172:
			map_init_172();
			break;
		case 173:
			map_init_173();
			break;
		case 175:
			map_init_175();
			break;
		case 176:
			map_init_176();
			break;
		case 177:
			map_init_177();
			break;
		case 178:
			map_init_178();
			break;
		case 179:
			map_init_176();
			break;
		case 180:
			map_init_180();
			break;
		case 182:
			map_init_182();
			break;
		case 183:
			map_init_183();
			break;
		case 184:
			map_init_184();
			break;
		case 185:
			map_init_185();
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
			map_init_189();
			break;
		case 190:
			map_init_190();
			break;
		case 191:
			map_init_191();
			break;
		case 192:
			map_init_192();
			break;
		case 193:
			map_init_193();
			break;
		case 194:
			map_init_194();
			break;
		case 195:
			map_init_195();
			break;
		case 196:
			map_init_196();
			break;
		case 197:
			map_init_197();
			break;
		case 198:
			map_init_198();
			break;
		case 199:
			map_init_199();
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
			map_init_206();
			break;
		case 207:
			map_init_080();
			break;
		case 208:
			map_init_208();
			break;
		case 209:
			map_init_209();
			break;
		case 210:
			map_init_210();
			break;
		case 211:
			map_init_209();
			break;
		case 212:
			map_init_212();
			break;
		case 213:
			map_init_058();
			break;
		case 214:
			map_init_214();
			break;
		case 215:
			map_init_215();
			break;
		case 216:
			map_init_216();
			break;
		case 217:
			map_init_217();
			break;
		case 218:
			map_init_218();
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
		case 224:
			info.mapper.submapper = 1;
			map_init_268();
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
			map_init_228();
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
			map_init_232();
			break;
		case 233:
			map_init_233();
			break;
		case 234:
			map_init_234();
			break;
		case 235:
			map_init_235();
			break;
		case 236:
			map_init_236();
			break;
		case 237:
			map_init_237();
			break;
		case 238:
			map_init_238();
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
			map_init_150();
			break;
		case 244:
			map_init_244();
			break;
		case 245:
			map_init_245();
			break;
		case 246:
			map_init_246();
			break;
		case 248:
			map_init_115();
			break;
		case 249:
			map_init_249();
			break;
		case 250:
			map_init_250();
			break;
		case 252:
		case 253:
			map_init_252();
			break;
		case 254:
			map_init_254();
			break;
		case 255:
			map_init_225();
			break;
		case 256:
			map_init_256();
			break;
		case 258:
			map_init_215();
			break;
		case 259:
			map_init_259();
			break;
		case 260:
			map_init_260();
			break;
		case 261:
			map_init_261();
			break;
		case 262:
			map_init_262();
			break;
		case 263:
			map_init_263();
			break;
		case 264:
			map_init_083();
			break;
		case 265:
			map_init_265();
			break;
		case 266:
			map_init_266();
			break;
		case 267:
			map_init_267();
			break;
		case 268:
			map_init_268();
			break;
		case 269:
			map_init_269();
			break;
		case 271:
			map_init_271();
			break;
		case 274:
			map_init_274();
			break;
		case 281:
			map_init_281();
			break;
		case 282:
			map_init_282();
			break;
		case 283:
			map_init_283();
			break;
		case 284:
			map_init_284();
			break;
		case 285:
			map_init_285();
			break;
		case 286:
			map_init_286();
			break;
		case 287:
			map_init_287();
			break;
		case 288:
			map_init_288();
			break;
		case 289:
			map_init_289();
			break;
		case 290:
			map_init_290();
			break;
		case 291:
			map_init_291();
			break;
		case 292:
			map_init_292();
			break;
		case 295:
			map_init_295();
			break;
		case 297:
			map_init_297();
			break;
		case 298:
			map_init_298();
			break;
		case 299:
			map_init_299();
			break;
		case 300:
			map_init_300();
			break;
		case 301:
			map_init_301();
			break;
		case 302:
			map_init_302();
			break;
		case 303:
			map_init_303();
			break;
		case 304:
			map_init_304();
			break;
		case 305:
			map_init_305();
			break;
		case 306:
			map_init_306();
			break;
		case 307:
			map_init_307();
			break;
		case 308:
			map_init_308();
			break;
		case 309:
			map_init_309();
			break;
		case 311:
			map_init_311();
			break;
		case 312:
			map_init_312();
			break;
		case 313:
			map_init_313();
			break;
		case 314:
			map_init_314();
			break;
		case 315:
			map_init_315();
			break;
		case 319:
			map_init_319();
			break;
		case 320:
			map_init_320();
			break;
		case 322:
			map_init_322();
			break;
		case 323:
			map_init_323();
			break;
		case 324:
			map_init_324();
			break;
		case 325:
			map_init_325();
			break;
		case 327:
			map_init_327();
			break;
		case 328:
			map_init_328();
			break;
		case 329:
			map_init_329();
			break;
		case 331:
			map_init_331();
			break;
		case 332:
			map_init_332();
			break;
		case 333:
			map_init_333();
			break;
		case 334:
			map_init_334();
			break;
		case 335:
			map_init_335();
			break;
		case 336:
			map_init_336();
			break;
		case 337:
			map_init_337();
			break;
		case 338:
			map_init_338();
			break;
		case 339:
			map_init_339();
			break;
		case 340:
			map_init_340();
			break;
		case 341:
			map_init_341();
			break;
		case 342:
			map_init_342();
			break;
		case 343:
			map_init_343();
			break;
		case 344:
			map_init_344();
			break;
		case 345:
			map_init_345();
			break;
		case 346:
			map_init_346();
			break;
		case 347:
			map_init_347();
			break;
		case 348:
			map_init_348();
			break;
		case 349:
			map_init_349();
			break;
		case 350:
			map_init_350();
			break;
		case 351:
			map_init_351();
			break;
		case 352:
			map_init_352();
			break;
		case 353:
			map_init_353();
			break;
		case 355:
			map_init_355();
			break;
		case 356:
			map_init_356();
			break;
		case 357:
			map_init_357();
			break;
		case 358:
			map_init_358();
			break;
		case 359:
			map_init_359();
			break;
		case 360:
			map_init_360();
			break;
		case 361:
			map_init_361();
			break;
		case 362:
			map_init_362();
			break;
		case 366:
			map_init_366();
			break;
		case 368:
			map_init_368();
			break;
		case 369:
			map_init_369();
			break;
		case 370:
			map_init_370();
			break;
		case 372:
			map_init_372();
			break;
		case 374:
			map_init_374();
			break;
		case 375:
			map_init_375();
			break;
		case 377:
			map_init_377();
			break;
		case 380:
			map_init_380();
			break;
		case 381:
			map_init_381();
			break;
		case 382:
			map_init_382();
			break;
		case 384:
			map_init_384();
			break;
		case 386:
			map_init_386();
			break;
		case 387:
			map_init_387();
			break;
		case 388:
			map_init_388();
			break;
		case 389:
			map_init_389();
			break;
		case 390:
			map_init_390();
			break;
		case 393:
			map_init_393();
			break;
		case 394:
			map_init_394();
			break;
		case 395:
			map_init_395();
			break;
		case 396:
			map_init_396();
			break;
		case 397:
			map_init_397();
			break;
		case 398:
			map_init_398();
			break;
		case 399:
			map_init_399();
			break;
		case 400:
			map_init_400();
			break;
		case 401:
			map_init_401();
			break;
		case 403:
			map_init_403();
			break;
		case 404:
			map_init_404();
			break;
		case 406:
			map_init_406();
			break;
		case 409:
			map_init_409();
			break;
		case 410:
			map_init_410();
			break;
		case 411:
			map_init_411();
			break;
		case 412:
			map_init_412();
			break;
		case 413:
			map_init_413();
			break;
		case 414:
			map_init_414();
			break;
		case 415:
			map_init_415();
			break;
		case 416:
			map_init_416();
			break;
		case 417:
			map_init_417();
			break;
		case 420:
			map_init_420();
			break;
		case 421:
			map_init_421();
			break;
		case 422:
			map_init_126();
			break;
		case 428:
			map_init_428();
			break;
		case 429:
			map_init_429();
			break;
		case 431:
			map_init_431();
			break;
		case 432:
			map_init_432();
			break;
		case 433:
			map_init_433();
			break;
		case 434:
			map_init_434();
			break;
		case 436:
			map_init_436();
			break;
		case 437:
			map_init_437();
			break;
		case 438:
			map_init_438();
			break;
		case 442:
			map_init_442();
			break;
		case 446:
			map_init_446();
			break;
		case 447:
			map_init_447();
			break;
		case 451:
			map_init_451();
			break;
		case 452:
			map_init_452();
			break;
		case 455:
			map_init_455();
			break;
		case 456:
			map_init_456();
			break;
		case 457:
			map_init_457();
			break;
		case 471:
			map_init_471();
			break;
		case 512:
			map_init_512();
			break;
		case 513:
			map_init_513();
			break;
		case 516:
			map_init_516();
			break;
		case 518:
			map_init_518();
			break;
		case 519:
			map_init_519();
			break;
		case 521:
			map_init_521();
			break;
		case 522:
			map_init_522();
			break;
		case 524:
			map_init_524();
			break;
		case 525:
			map_init_525();
			break;
		case 526:
			map_init_526();
			break;
		case 527:
			map_init_527();
			break;
		case 528:
			map_init_528();
			break;
		case 529:
			map_init_529();
			break;
		case 530:
			map_init_530();
			break;
		case 532:
			map_init_532();
			break;
		case 534:
			map_init_126();
			break;
		case 536:
		case 537:
			// https://forums.nesdev.org/viewtopic.php?p=240335#p240335
			map_init_195();
			break;
		case 538:
			map_init_538();
			break;
		case 539:
			map_init_539();
			break;
		case 540:
			map_init_359();
			break;
		case 541:
			map_init_541();
			break;
		case 543:
			map_init_543();
			break;
		case 547:
			map_init_547();
			break;
		case 550:
			map_init_550();
			break;
		case 551:
			map_init_178();
			break;
		case 552:
			map_init_082();
			break;
		case 554:
			map_init_554();
			break;
		case 556:
			map_init_556();
			break;
		case 557:
			map_init_557();
			break;
		case 558:
			map_init_558();
			break;
		case 559:
			map_init_559();
			break;
		case 560:
			map_init_560();
			break;
		case 561:
			map_init_561();
			break;
		default:
			gui_overlay_info_append_msg_precompiled(11, NULL);
			EXTCL_CPU_WR_MEM(000);
			info.no_rom = TRUE;
			info.mapper.supported = FALSE;
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
			break;
	}

	wram_init();
	vram_init();
	nvram_load_file();

	chrrom_reset_chunks();

	if (info.reset >= HARD) {
		prgrom_reset_chunks();
		wram_reset_chunks();
		ram_reset_chunks();
		nmt_reset_chunks();
	}

	// after mapper init
	if (extcl_after_mapper_init) {
		extcl_after_mapper_init();
	}

	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		emu_info_rom();
	}

	return (EXIT_OK);
}
void map_quit(void) {
	nvram_save_file();

	// devo farlo prima di liberare prgrom.data
	wram_quit();
	vram_quit();
	ram_quit();
	nmt_quit();

	memset(&info.mapper, 0x00, sizeof(info.mapper));
	memset(&info.sha1sum, 0x00, sizeof(info.sha1sum));
	memset(&info.crc32, 0x00, sizeof(info.crc32));
	memset(&vs_system, 0x00, sizeof(vs_system));

	nes20db_reset();

	miscrom_quit();
	prgrom_quit();
	chrrom_quit();

	// faccio un reset
	memmap_init();

	info.number_of_nes = 1;
	info.mapper.ext_console_type = 0;
	info.decimal_mode = FALSE;

	if (extcl_mapper_quit) {
		extcl_mapper_quit();
	}
}
