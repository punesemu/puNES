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

#ifndef DATABASE_H_
#define DATABASE_H_

#include "common.h"
#include "mappers.h"
#include "vs_system.h"

typedef struct _dbelement {
	char sha1sum[41];
	WORD mapper;
	BYTE submapper;
	BYTE machine;
	BYTE mirroring;
	BYTE vs_ppu;
	BYTE vs_sm;
	WORD extra;
} _dbelement;

static const _dbelement dblist[] = {
/* --------------------------------------- PD --------------------------------------------- */
	/* full-palette-pal.nes */
	{"98a08dfe9ade517c91efdb661754575603c14fb9", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 01.len_ctr.nes */
	{"9245932519e89431ac1d5c7ce6e4383759e7b299", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 02.len_table.nes */
	{"7ae3df944bd7e37602ebc1df2cbeecf5aca6ede5", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 03.irq_flag.nes */
	{"fd824c69b76e50fc27b39658770bfd085358bf60", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 04.clock_jitter.nes */
	{"d9d848be2dd0d3397d46961d18b1efd93e471f8d", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 05.len_timing_mode0.nes */
	{"1ee769960521a02c5abcb74abda15826d4ccd756", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 06.len_timing_mode1.nes */
	{"832494e2360cbea3f334283bdb4e0c66ee07e662", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 07.irq_flag_timing.nes */
	{"06d6edcdf4bb7696b27b54de3de0ab9d7d184ca5", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 08.irq_timing.nes */
	{"c6b2b53e68c314a4622a0f4e02d682908123494d", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 10.len_halt_timing.nes */
	{"40d4cc577e80ed6c0ff4f86b1b11c72d1a9b13e9", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 11.len_reload_timing.nes */
	{"0b71bea825866b5f6205c2d646988fe2af0834e3", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* logo (E).nes */
	{"34dd5c48c82a6cbb7cba1456ca81ae376427c9c3", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chip Addiction by Mico1972 (older) (PD) (PAL).nes */
	{"c888919709863bab45a6984d65ae7b1b8f7c6ff7", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chip Addiction by Micol972 (PD) (PAL).nes */
	{"0c062ce269e59d71ecde1f92452bb378af6d2218", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NeSnake 2 (V1.2) (PAL) by Matrixz (PD).nes */
	{"59f5d571074e1178cfb5455e85a55ccade08553d", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Stuck in Castle Nessenstein by Micol972 (PD) (PAL).nes */
	{"16849e3a37dc8a7971e4982f2bce762901301d24", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Colours Demo by Chris Covell (PD) [a1].nes */
	{"c735246cbd108c321efd83d2329d75e8410eb38e", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Colours Demo by Chris Covell (PD).nes */
	{"fbb25fdf4f91912d2ade9bc24281c8ddcf01c611", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tank Demo by Ian Bell (Mapper 0 PAL) (PD).nes */
	{"1db0c2a5b03f27cdb15731e7a389e2cb4a33864a", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tetramino v0.26PAL by Damian Yerrick (PD).nes */
	{"477f0a916986d166ff2e434e96624d50dd2b7a0c", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tetramino (V0.27-PAL) by Damian Yerrick (PD).nes */
	{"0b3effe6edc1699dc9d4c3294839591331ae6d1b", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tetramino v0.30PAL by Damian Yerrick (PD).nes */
	{"37e541a1bde00fe604a93750505d3a4a43d118f2", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TAROT by membler (PD).nes */
	{"0ee348c22ca320afb543cded7739ea77d7bbed9a", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* scrolling_bot_pal.nes */
	{"94c68842f2f3f6f02a73957affb6af989b906b2e", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* demo_pal.nes */
	{"74323018092bc1d0aea59cea157f1162d96e634f", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* colorwin_pal.nes */
	{"956c8835586188d138136dcf494023c8f7a739b1", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* window2_pal.nes */
	{"e933fc861055f815c824a8efab9fb11825cb7512", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* window_old_pal.nes */
	{"e392ce12e622295f2e0b9f0c2ea3e8a31eb69d31", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* nes15-PAL.nes */
	{"37aaba8411f8306b6377dcfb209ccd285f32d70f", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PalRenderingDisabled_Rows1-256v2.nes */
	{"6454236f1ed53de6b123c58dded1c2c649cee46c", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PalRenderingDisabled_Rows257-512v2.nes */
	{"40408ac93b0cf478e6ada60dd9fdda9524b18811", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PalRenderingEnabled_Rows1-256v2.nes */
	{"465d21fb353c62a188511f9527cc4fe8f3c92696", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PalRenderingEnabled_Rows257-512v2.nes */
	{"c5ebd392edd1c9791020c6dc774a2df3017a0fcd", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PalRendOff1_v3.nes */
	{"e15029fe7b038657465aa2e9a641addb0689304b", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PalRendOff2_v3.nes */
	{"4b172cf92c7fadcf321833bfe3128c5328dec200", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PalRendOn1_v3.nes */
	{"bfd9e7ce1c926b5a1435e628aaed4de07a0f1679", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PalRendOn2_v3.nes */
	{"e63db7b0b826bd73ccf0705341eab57079945c7c", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Retrocoders - Years behind.nes */
	{"cb69e8acda28b487cd4e9254b603272f428416eb", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Driar.nes */
	{"240998798bdb32da7428d9b8447d335f05dd5885", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Retrocoders - Years behind_debugwrites.nes ret.nes */
	{"398b1d268a3c466ca55ed382b15b8ca70ce094eb", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TANESPOT.nes */
	{"74d49e41970d79fb26ba6737fdb5d027703ee36f", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 20090805_shaping_reality_(lcp_2009).nes */
	{"83acbb3ea52925907c0740cfac78f0554c7b0cc8", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* rastesam4.nes */
	{"f1164ed2453c51d9ae2e441ae103d2de02abccc7", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* --------------------------------------- 0 ---------------------------------------------- */
	/* ElevatorAction_J.nes */
	{"2070d3241f5f29de74b004c9a6237cec17951f9a", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Hogan's Alley (W) [b1].nes */
	{"e0f22ffc03c315145686ca98f8dcf3f91cca06f5", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Hogan's Alley (W) [o1].nes */
	{"1bc2f7e09a5c965df3beba6e49c8d4ba5ed059f7", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Hogan's Alley (W) [o2].nes */
	{"bca489ed0fb58e1e99f36c427bc0d7d805b6c61a", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Hogan's Alley (W) [p1].nes */
	{"43c7fe2bfa2720448289bcbc20657e3938f0676f", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Hogan's Alley (W) [p1][o3].nes */
	{"56d7e3bf11eb1574c3d8b30d07f8126952ff928a", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Hogan's Alley (W) [p2].nes */
	{"13743a35b1617c6a191fc3939e81be0388d36cdb", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Hogan's Alley (W) [p2][o1].nes */
	{"91bae50bcd418e372352bb391fb9e78b2c179a33", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Hogan's Alley (W) [p3].nes */
	{"94b530f0df450ae21bfae75b64118ea164796bb9", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Balloon Fight (E) [!].nes */
	{"53583ef3c8a9eadd5cab927492ca12de8d4acdd1", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Balloon Fight (E) [T+Fre1.1_ks151].nes */
	{"2b3571b68505dfb9993490aeaea4ca9a1b95e36f", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Devil World (E) [!].nes */
	{"00a7345604a551750a3bcbdc6b25e4b461ffc136", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Devil World (E) [T+Fre1.0_ks151].nes */
	{"578bc20add83b24ebcc90e0e54f1291b2a01cb29", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Excitebike (E) [!].nes */
	{"8c37839c645d54184df6273e481527d4cfa312d2", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Galaga (E) [!].nes */
	{"33ebc0cb147519d88e76017d9eac55dbc9ffd010", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Golf (E) [!].nes */
	{"b2011c444fd6190665d720209822b1f2b8cf5303", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ice Hockey (E) [!].nes */
	{"141020a00eeafe58c881c0e0103f678098da98b8", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kung Fu (E) [!].nes */
	{"4e5472b0f486e818d6fb41b8ebbcf8299cdc4d31", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lunar Pool (E) [b1].nes */
	{"7ca1888232170558b120aeb98fe1b1a129e5ef22", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lunar Pool (E).nes */
	{"56ed0c47d49b7a41d0ae58fcde39c574000f04c7", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* M82 Game Selectable Working Product Display (E).nes */
	{"cbd2d14fa073273ba58367758f40d67fd8a9106d", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mach Rider (E) [!].nes */
	{"270182fb07a20fedfd079a6bc49974497545a9e7", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario Bros. Classic (E) [!].nes */
	{"497d0eee07a2cff4ee29b119d5ecc649a4ec7955", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario Bros. Classic (E) [T+Fre_ks151].nes */
	{"fdba4087683caaaa6d36a2a8d390c8661bc0b6ee", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario Bros. (E) [!].nes */
	{"f5d609720bc60bcb02f434d41149ae68e9f6b899", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NGE Third Impact 40Kb.nes */
	{"5e8757d31927fee033a83967fb558aff9d94745f", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pac-Man (E) [!].nes */
	{"8fef2bdce0c0be2ece67b26587aa22097ba3c9cf", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pac-Man (E) [T+FreFinal_ks151].nes */
	{"87f51c74f062b7a8527e8704a3fd1f608a2fec51", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pinball (E) [!].nes */
	{"9fb84f4b9e3eaa1df88be37c679ce20204acef77", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pinball (E) [T+FreFinal_ks151].nes */
	{"d13fdaf673cd6a08c42292c374f0c2d08ea3aa26", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Action Replay (E) (No Cart Present) (V1.2 RevB) [!].nes */
	{"fb475458a69c7de661e9053dcbe0117edc0d5a62", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Action Replay (E) (REVA) [!].nes */
	{"be20dc43e94760d4f37d1c478cba4824ff96ce78", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Action Replay (E) (V1.2 RevB) [!].nes */
	{"33bdd5be41d23e28e5896f98c3716cb92a754fdf", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* R.C. Pro-Am (E) (PRG0) [b1].nes */
	{"3ce309dacd61e496d63f36469b3530e069df8a6f", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Road Fighter (E) [!].nes */
	{"09970d1cc4a2bd82715cc4243e4f9fa4667c68c5", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Slalom (E) [!].nes */
	{"d97dfd798e1cc18b5e060738d71a768c13ef953e", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Soccer (E) [!].nes */
	{"8cf4c23ab27733ba7def35a04dc0e35a96f2a18a", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Spy Vs Spy (E) [!].nes */
	{"2f83c8fd8171a9206648ac27981ce4078d8fd1c2", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. (E) (REVA) [o1].nes */
	{"31b332f6bc338e058a7b958dca285066c405b697", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. (E) (REVA) [T+FreBeta_Generation IX].nes */
	{"b428822e6d46bbdf93757573eb67351242203966", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. (E) (REVA) [T+Rus100_Bishop].nes */
	{"2759209764ed4c0d7f9469adb2ae54be3623a636", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. (E) (REVA) [T+Spa100].nes */
	{"d7cc7b1c6aca6fbef8e52f85dc1ed04f4c6acbee", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. (E) (REVB) [!p].nes */
	{"fbe27f9cca860d227bf28f6769571e9efcbdf178", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tennis (E) [!].nes */
	{"ec779c863b96124e9d07a96636bc287052b733e8", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Xevious (E) [!].nes */
	{"d8bd8c5a11df5603285990aef02a26d765389eec", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Xevious (E) [T+Fre].nes */
	{"1c94f97d684d71109cc06042acac7f523c6643a2", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Balda (R) [!].nes */
	{"77f997c379b9e417a8d6bec542ef9a36a8b7b951", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Morskoy Boy (R) [!].nes */
	{"d868b0f142a9021013e40093cb2ae4c042a31f9c", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pole Chudes 2 (R) [!].nes */
	{"335d2abcd575c012cf8ff9553acf8cb5b09acab5", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pole Chudes (R) [!].nes */
	{"048b016b32b30cc626485cc6deb49986092949a4", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pravila Dorozhnogo Dvizheniya (R) [!].nes */
	{"e6ec5a626cac4dedfefdb5406dcacbf960d7b7b5", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Puteshestvie po Evropa (R).nes */
	{"894bd6be70ced0181dcfebcdbc1230486b002a68", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Text Editor (R) [!].nes */
	{"aa177408e11b4da57129df21a58af20c650f16f6", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ugadayka (R) [!].nes */
	{"0b02c1421daf7b41d4faae91390582f23e1b0d64", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Volleyball (UE) [b1].nes */
	{"2a0c490b978060ad6db3c205f5e087c1851cbb49", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Volleyball (UE) [o5].nes */
	{"3cc766a7c760ec394ec43a4414718022cf9d99ce", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Volleyball (UE) [T+FreBeta_ks151].nes */
	{"159df86c0113d5e1d37c06fe23ff3629ad5afce6", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Volleyball (UE) [T+Ita1.0_Zombikiller].nes */
	{"9b45bacbd47492b72c6aea176e9dbe36100c376d", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Volleyball (UE) [T+Por100%_K-Traducoes].nes */
	{"6fa89c60bc8b9fe108d9226c61522b9f0115cfe6", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Warehouse No. 18 (R) [!].nes */
	{"42f723363370dc32b30b10ca59d0614cb41a8a05", 0, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* cpu_interrupts.nes */
	{"c6ba32f673254ba52e0b6d142a46310b4ba8652a", 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Autobot Bros. 2 by Ratch (SMB1 Hack).nes */
	{"d345a9332e44a33c1f8e523b1ba2a3902f5577c7", 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario + Toad (Nuts & Milk Hack).nes */
	{"6d244c8d198f11605899d7a4614cb069b01a5270", 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Johnny the Homicidal Mario v1.0 (SMB1 Hack).nes */
	{"3c85b71f3e4a9edbd2cd651107cf6a0be7364a84", 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mortal Kombat Bros Sub Zero Mythologies Quest (SMB1 Hack).nes */
	{"a90272033ac700c5569aaf4049eb9572ccc3a9ad", 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Othello (HES) [!].nes */
	{"ff341d3b1e87a8685c05594365858ab747795f0d", 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Othello (HES) [o1].nes */
	{"0b698ad2b7cfa855e01726226f221294a8a2d9ed", 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ball Hack v.2_2001-03-21 by Knoxburry-Aneurysm (Balloon Fight Hack) [o1].nes */
	{"c0a936c933185a73edde30f34160c61219a8bae0", 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hidden Chinese Chess - An Qi (Asia) (NTSC) (Unl).nes */
	//{"261d13eaeeba7b9893b1f9427ddf431618bf41c1", 0, DEFAULT, BAD_AN_QI, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ms. Pac-Man (USA) (Unl).nes */
	{"83057a9b6f6a3ea8fe59af149be4c71d9c319617", 0, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Mr. Happyface (Ms. Pac-Man Hack).nes */
	{"2c77d3a31556035681ab369bee319b15663190a1", 0, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Tank (Caltron, CN-16)(Unl)[!].nes */
	{"1c013575aea14e70371cb76aaacaf948c1a3e696", 0, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------- MMC1 -------------------------------------------- */
	/* Darkman (E) [!].nes */
	{"0edef77c64e3b64b2ea26fb18d5b0e43e86e8c12", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Action in New York (E).nes */
	{"c0983f1bddbc6aefe32f4e616589932c2e609427", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Action in New York (E) [T+Chi].nes */
	{"c66e9225d7f00ff4a1dce6948256fd5f7a8f6bff", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Addams Family, The (E) [!].nes */
	{"a524c78ccf2dbc189bf3dd389c6768f1b7e9ff82", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Addams Family, The - Pugsley's Scavenger Hunt (E) [!].nes */
	{"44aba89ac69b8fb50c7eb1d85bcb393708267a05", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Addams Family, The - Pugsley's Scavenger Hunt (E) (Prototype) [!].nes */
	{"c02861963e1b1b73c06fe236c89cc3f18c09b1bd", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Adventures in the Magic Kingdom (E) [!].nes */
	{"28ff21a39a3176349052c846d7dd59f0ebef09fb", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Adventures of Bayou Billy, The (E) [!].nes */
	{"a59d7dd528180f1f7af6e4f08c16578eeeb44e25", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Adventures of Lolo 3 (E) [!].nes */
	{"9f7207eab00630204116c564f53f4e62466f655a", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Adventures of Lolo (E) [!].nes */
	{"fc08bb50dea966294a78895bb26715d671d53009", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Adventures of Rad Gravity, The (E) [!].nes */
	{"6d98ce7344720ea2889a2bc3c18bf335f415ebe1", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Air Fortress (E).nes */
	{"90c08d541f02c1dc84ea8b2662c8e13fdc01fabd", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Airwolf (E) [!].nes */
	{"3edbadcded4597d7216e57e129fecd9a754c48d1", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Anticipation (E) [!].nes */
	{"d755f6c0fb42534254d55dd3da67b3fe2923e6b4", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Attack of the Killer Tomatoes (E) [!].nes */
	{"755fd5195ae7cde0dafdab54fdc29dae073d9682", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Barbie (E).nes */
	{"e0eec92796235d23b09cae00a6d343e87598e59e", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Barker Bill's Trick Shooting (E) [!].nes */
	{"e0bb71d086fce8836555aafb8f2ef4a26b5c4bf0", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Battle of Olympus, The (E) [!].nes */
	{"d70bca094b244fac0f126f80af31139fa3f0c49d", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Battle of Olympus, The (E) [T+Fre_Lyssal].nes */
	{"5cd399bf4e59c9fc84211b92dfbac005d3af4a46", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bigfoot (E) [!].nes */
	{"c373f24633614b4f28f9ecd0f3064333d23d44e4", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bionic Commando (E) [!].nes */
	{"6590d556e68453bb368472871dfb8cd3a5fe1689", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blaster Master (E) [b1][o1].nes */
	{"be887034ca5c0dd469612c0583627827c9c9a3f2", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blaster Master (E) [!].nes */
	{"474e1b29d0e03ec937755728d6d1d29d030d3e13", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blaster Master (E) [T+Bra1.0_BRGames].nes */
	{"3b17a3e61a02d3ebef2f8f7b55f62ffa7d7f2451", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Boulder Dash (E) [!].nes */
	{"172020656d4f66e6d673b3d6812e31d47586dbfc", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Boy and His Blob, A - Trouble on Blobolonia (E) (CHR1) [!].nes */
	{"33637a033cdf33c62742bac19d5e1d65ba0ab3f6", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bubble Bobble (E) [b1].nes */
	{"c3442e3c6f727cb2961966b5f4ca3cfa47b6985b", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bubble Bobble (E) [!].nes */
	{"c1b1d738fab58b662d7d53fb78b18f0a3e8af685", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bubble Bobble (E) [T+Ger][b1].nes */
	{"194d0bd49356a2e16c4c53bc39b08860c4910c29", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania II - Simon's Quest (E) [!].nes */
	{"fbee405460fb313e1921715c0d171c43b9e962dc", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania II - Simon's Quest (E) [T+Swe1.0_ATLAS].nes */
	{"70e3d73285cdea38e7bd421619b76dd51c443174", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania II - Simon's Quest (E) [T+Swe1.0_MetalHead].nes */
	{"54f1392f2a6c8458e6c4c8e5cc5b09c1cf2d7595", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Championship Rally (E) [!].nes */
	{"93b3f2a7a37df47a01738d0fd39cbf4edde75b26", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chessmaster, The (E) [!].nes */
	{"b61db944c3597982e3d5f65945c88369fd8b6371", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chip 'n Dale Rescue Rangers 2 (E).nes */
	{"ad0d9add5985884eb5826235c5d000a83d3806ca", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chip 'n Dale Rescue Rangers (E) [!].nes */
	{"2190b1f557cbce57b23910e3fcf929c102094c25", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chip 'n Dale Rescue Rangers (E) [T+Fre100%_Terminus].nes */
	{"4e51d743e5a8d14f7c273de4bb10d2949a9e8723", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Corvette ZR-1 Challenge (E) [!].nes */
	{"56d7d5a44dd9a05073117d641c9709dabcca4930", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Darkwing Duck (E) [!].nes */
	{"b627120f2e9f2a41a3452250829c5c0ca3bf9543", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Defender of the Crown (E) [!].nes */
	{"3aac8e5472a9d4c26ca50080d29d9ef937bddc0f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Die Hard (E) [!].nes */
	{"dfb73d9d445e5a5fe402cd83d7a4cd288e6c1b2d", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Double Dragon (E) [!].nes */
	{"c5560ed90f4e2ab5e4ca05adfee40e1c0439c2c0", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Double Dragon (E) [T+Fre].nes */
	{"f7496b31705ef8031559651d2b6fcfbbef0f3b22", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dr. Mario (E) [b1].nes */
	{"0544c1a3fc5bf87ff07651973ec3d5c4c17b5f8e", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dr. Mario (E) [!].nes */
	{"0b9d9c54d9f4d55fcb6c412ec005638d0eccbc45", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Eliminator Boat Duel (E) [!].nes */
	{"2ea0b8e37ea421d0b0dc2be06c3af28b2f091b3b", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Faxanadu (E) [!].nes */
	{"0711bc8d0bf42a0829391c2320393a0d3df2dd1f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Faxanadu (E) [T+Fre100_BessaB].nes */
	{"be96b59955d2ad1b9ba6d14536d0eeff20282338", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Faxanadu (E) [T+Ger1.2_GTrans].nes */
	{"ec9dabebe4dff3f30a163120f3035b9e8428f8ca", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Faxanadu (E) [T-Ger].nes */
	{"0842e76d572bcd3c82866b4cefb77461a6b06b1b", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ferrari - Grand Prix Challenge (E) [!].nes */
	{"55be4dc9077933c5a8b32356790b87b7a8245332", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fester's Quest (E) [!].nes */
	{"139de7e7f515ffd73964dbf7627bfbd296acf764", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Four Players Tennis (E) [!].nes */
	{"18354250bbf03aa0869cb40b44f5611193312d8d", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ghostbusters II (E) [!].nes */
	{"e2674a4ee72c0334c62062399ff813202faaca99", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goal! (E) [!].nes */
	{"50dc8e09add5f7d0151011ee38a29a6b8a55d2e7", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Godzilla - Monster of Monsters! (E).nes */
	{"b3e772dd74b4431c56b92968ad253fe1623c730e", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Guerrilla War (E).nes */
	{"16326f34efb3cbf5b8fd7076b06a7d9965a1fbc0", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hook (E) [!p].nes */
	{"a3b6115cf1e344cd64940d7c6b9c88407f05b472", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hoops (E).nes */
	{"fd3648b4ba162b665d6aa28a0bc12af8860ce488", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hudson Hawk (E) [!].nes */
	{"b4ed49ff344c6ee77ec983cc4b753cbbcf9e1144", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* International Cricket (E) [!].nes */
	{"4034d9e4e89b72ab1545e3aef325b59f61fa961c", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* International Cricket (E) (Prototype).nes */
	{"1f8ffef26eaaa4080ef35e8e3e2013067c1f5940", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Iron Tank - The Invasion of Normandy (E).nes */
	{"689ed0fc97902645bd06fecb0df6a80bae36a797", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Journey to Silius (E) [!].nes */
	{"42d7ebe8fe1ded3644cc4488fed04a1274944c03", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Knight Rider (E) [!].nes */
	{"96325685170a9a27cae766fd47287100e0b85caf", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lee Trevino's Fighting Golf (E).nes */
	{"6ba288d29bacf3c3ad8cbdecfc3f0c839de4a9a6", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Prince Valiant, The (E) [!].nes */
	{"545bb6a9d8ac48942a4ccb2f7b2176a8e90ae12b", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lemmings (E) [!].nes */
	{"5857f181cebaee5400229efb20929b3cf5f79676", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lethal Weapon (E) [!].nes */
	{"568efc696afab3c360406768c6b3058f9a39c2c3", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario & Yoshi (E) [!].nes */
	{"51f5bcfd9f43f4eb013044ceca523562008f86d9", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman II (E) [!].nes */
	{"e28ba80be814bb032bbe4647c1b2104f868dfa25", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Miracle Piano Teaching System, The (E) [!].nes */
	{"ed961fe21ecce26b15c2722a91080da39f4e394d", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Miracle Piano Teaching System, The (France).nes */
	{"b0eb752ae1c5d14280ec8c67bc88bbacdedf31a7", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* New Ghostbusters II (E) [!].nes */
	{"3bb85626b3caa71c996a00bbe1f05e342f870c2f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* New Ghostbusters II (E) [T+Rus_magic team].nes */
	{"ce62ce1b8621553db7ee4b5729f9e20dac876776", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* New Ghostbusters II (E) [T+Rus].nes */
	{"8c391f977af518f9f961bc69c6d46f63ca1f0cc9", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nigel Mansell's World Championship Challenge (E) (M5) [!].nes */
	{"28ce5e2115b1950c372b052068dcd4111e218e3d", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Operation Wolf (E) [!].nes */
	{"3cfb33ae21278b4f03cb6e531e7a4d84cd1a5e25", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parasol Stars - The Story of Bubble Bobble 3 (E) [!].nes */
	{"50991602680588a9e7b03e06d45ee607fc00db13", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parasol Stars - The Story of Bubble Bobble 3 (E) (Prototype) [!].nes */
	{"8ebb63909b9c81b9d260534a7b09ff0167c4bf05", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pinball Quest (E) [!].nes */
	{"ba2f1f2bdba107800922ccb43ffa07a3b6618453", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pirates! (E) [!].nes */
	{"63df8b410ce3af69e3f6c6c78671533863736954", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* P.O.W. - Prisoners of War (E) [!].nes */
	{"573481a0ede58335aaab49d1b2164bba5687dc5f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Predator (E).nes */
	{"0194142f3d50062694c9873fc807456d7e957c2e", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Racket Attack (E) [!].nes */
	{"81f0f4b2dd106efb134fcb1325ff0afd227fe63b", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rad Racer (E) [!].nes */
	{"1393fe50051dcd71500d396ffb995020222053cb", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rainbow Islands - The Story of Bubble Bobble 2 (E) [!].nes */
	{"dc0ea1f07b8b5c70a7567be545618b3d4a3ba4d8", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* R.C. Pro-Am (E) (PRG1).nes */
	{"b619ecf43277af93b5a6c0d62fe6d956cb5a7548", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rescue - The Embassy Mission (E) [!].nes */
	{"5b4e9e05309eff032f6296e7da67cd20dd3f99fe", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* RoadBlasters (E).nes */
	{"512d9e65e7a5708c7dd6442841f93660a44f495f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Robin Hood - Prince of Thieves (E).nes */
	{"b2ac0cb323679b79a2dd07d296ae77512bf198b8", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* RoboCop 2 (E) [!].nes */
	{"9129b9f39c210fbbbf0c2c51d8e7c8ebea115d9a", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* RoboCop 3 (E) [!].nes */
	{"da9f0c3689e76cfed96c0c603101d91c029cc965", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shadow Warriors (E) [!].nes */
	{"819558e2d1946ffa86bed9c4f5a95c8d786b9227", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shadow Warriors (E) [T+Swe1.1_TheTranslator].nes */
	{"877eb560903503f27d63440bcaa906fac3d446c9", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Simpsons, The - Bart Vs. the Space Mutants (E) [!].nes */
	{"236cd92a36eeed1949956fcb7b9b4b1c27af1006", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ski or Die (E) [!].nes */
	{"7e2f22b2d58430105cd73fdc2192b66910dc49e4", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Snake Rattle N Roll (E) [!].nes */
	{"58ad9b56b03b99bda8c4992998d9185ae4d7ce8a", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Snake's Revenge (E) [!].nes */
	{"ada9f8b80889e5bb3be2028e305436c214f200fa", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Snowboard Challenge (E).nes */
	{"75c8fbaf206dbecd73f5449ede8fe97ae0fbd80f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Snow Brothers (E) [!].nes */
	{"f76679c6416546b5aeb593e3878e44e689a4f33c", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Stealth ATF (E) [!].nes */
	{"9aa7887dd62d31de1fccb188505136d5f02aa788", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Swamp Thing (E) [!].nes */
	{"cce85825e0b504306dfb6481f1160d2f03318028", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TaleSpin (E) [!].nes */
	{"4b087b94a58f0d2a7188b1cc130015f5f7892ec5", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TaleSpin (E) [T+Rus_RTS].nes */
	{"dc4ecce73c1e8aa2f43d6d13496e2bd8f4aa462e", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TaleSpin (E) [T+Swe1.2].nes */
	{"78550727165249dd6ca1cf74df1f6cc953eebce3", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tecmo Cup - Football Game (E) [!].nes */
	{"8a223c9eafba85ba8041a5d40e65eac24bb9c9f2", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tecmo World Wrestling (E) [!].nes */
	{"1b4999f81f4f1fae89df5e51ad52886844baeb0e", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Hero Turtles (E) [!].nes */
	{"eb9e58302128a01d3c5db97cab967bb58bc560d2", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tetris (E) [!].nes */
	{"677fecd986cdc9bce65533d04b1bd90728045393", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Track & Field II (E) [!].nes */
	{"4c5acae0e8048a720dddbcdc8269ad7e9f1923e8", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Turbo Racing (E) [!].nes */
	{"4775dc7a1b5ab448389b06cf1d99ea15b3247d88", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Willow (E) [!].nes */
	{"aab665bb7c6e6c0b4d8cecec7f70cb9554b37017", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wrath of the Black Manta (E) [!].nes */
	{"f093555823e296712f7a7d4857d3e5c796b0b7b5", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zelda II - The Adventure of Link (E) (PRG0) [!].nes */
	{"a1ae2b795283c5728ac3f515eeef0b7f97ba2842", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zelda II - The Adventure of Link (E) (PRG1) [!].nes */
	{"c00e1129266a5b7fba1de5d0a0ba22f45398583f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zelda II - The Adventure of Link (E) (PRG2) [!].nes */
	{"bf53929834b04670dad2733fe7b58c381ca5baee", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Darkwing Duck (G) [!].nes */
	{"1a1fcd763ca8c8327c43e79b38125747fca90f64", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Monopoly (G) [!].nes */
	{"a2e294edac9e66d52725a3f60023b9f825391e91", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pirates! (G).nes */
	{"befc9fb1e862e95b3c6face3c4d3925b242b30ef", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Robin Hood - Prince of Thieves (G) [!].nes */
	{"df7315c617a1107880273925280cd2472609e508", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Defender of the Crown (F) [!].nes */
	{"5700de705c481b919d7549a0fd1706e1bb1b5603", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Les Chevaliers du Zodiaque - La Legende d'Or (F) [!].nes */
	{"68e282ad99185fff11d35e0fcee351b9e78fbc05", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Les Chevaliers du Zodiaque - La Legende d'Or (F) [T+Spa0.50b_Tanero].nes */
	{"61401f2c2dc1c255387dd9c32fda62f6b922ccae", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Monopoly (F) [!].nes */
	{"f750d1ec58dfa5875c8f7e7a2fc3e97d93df391f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles (I).nes */
	{"91bfef1695ac06783332f2530082ec12f718b184", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Robin Hood - Prince of Thieves (S) [!].nes */
	{"7031d4124e914c66a831a770a0fbec873149b250", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tecmo Cup - Football Game (S) [!].nes */
	{"6d43a2739bb56e57857f89221b27be3567a14bcb", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dr Web (R) [!].nes */
	{"2a1c5d0a55faa95dfa5446756962cb3ddecbddb1", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Yoshi (U) [!].nes */
	{"b5a7664905020635c4a41270105c201100e8fec3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Iron Tank - The Invasion of Normandy (U) [!].nes */
	{"1b8e9bab097a4345c67fc46fa13951847aabef12", 1, DEFAULT, NTSC, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Airwolf (U) [o3].nes */
	{"7bc0296cfd36fb817a1e589fb81e0de0032f5958", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Airwolf (U) [!].nes */
	{"a9a8083a26f70f9ad4cb9095045ab1be75f6567b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Bard's Tale, The - Tales of the Unknown (J) [b1].nes */
	{"2dcd37cc860793899a4ab305057af2e66e88aba1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bard's Tale, The - Tales of the Unknown (J) [b2].nes */
	{"695de632eef6e8f72ed49a5228052aef5430eb7b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bard's Tale, The - Tales of the Unknown (J) [!].nes */
	{"e22ae723541184adf7a81d824ad003cfd59ab485", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bard's Tale, The - Tales of the Unknown (U) [b1].nes */
	{"c3da1804b1e666e0856058b9c6686b5f7a427c79", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bard's Tale, The - Tales of the Unknown (U) [!].nes */
	{"3cb2c31f2f7e3a0f254353c4b88190103254703e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (J) [!].nes */
	{"cd665acea15a4542a9e4cf16a7ca2ce53c88726d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (J) [t1].nes */
	{"8784a1aec73f13b1caa9bb5126065bff6314d260", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (J) [t2].nes */
	{"c9e44530a5d846ea97adb6e449263abc8468433c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (U) [b1].nes */
	{"e4293a52259baa0fe9b99e72da9a8bcf9d43027f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (U) [!].nes */
	{"2e401097b7b6f5de5b0f88e6a97c5675bd916801", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (U) [T+Ita1.0_ZombiKiller].nes */
	{"2eef202c2395c5946e1e556edbd884aa391537d7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Destiny of an Emperor (U) [!].nes */
	{"aef894686836ea03fb047fe87979afeb079c350c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior II (U) [!].nes */
	{"191a131e7e27e121d094cc412fed87c708147b2c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dynablaster (E) [!].nes */
	{"31c720f9a75eb2d069244bd257aa23de6b5bce8b", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Elite by Ian Bell & David Braben (NTSC Hack).nes */
	{"fcf9ff27f097892148eaec439ab79bc53164d751", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Elite (E) [!].nes */
	{"8d7ca48394996ef45ab1094b582d00ad7b028f21", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Easy Final Fantasy (FF1 Hack).nes */
	{"f0cdefe0fe59049ce35160a8310043a77d0d22ea", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy 1C (FF1 Hack).nes */
	{"2a2aeab9f481d0c54b230056acdbe6f8880b2217", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy 1X V2.0 (FF1 Hack).nes */
	{"7c4d3b0c4f06afd6868f0a462c567e2d4dd38016", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy 2nd Journey (FF1 Hack).nes */
	{"b3ce37eb75ee447a4b5d0bc213c7424910031482", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy 2nd Story (FF1 Hack).nes */
	{"e65a42724fe5d35cafe5b9b37ab32121d77e7566", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy 7 (FF1 Hack).nes */
	{"b2c568adfd3c8df22a7960c5496ff60994354955", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Amazon (FF1 Hack).nes */
	{"513b08d2ba2f8188e886f513a4a5d202ca297623", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - A New World (FF1 Hack).nes */
	{"11a250424cc605d52a3ad60f08ec97799a5774de", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Another Story (FF1 Hack).nes */
	{"bced2e588f21b9debfb53a850ca674c5ad072e76", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - A Return to Darkness V0.6 (FF1 Hack).nes */
	{"10a3cff4394894266136eae3a0d2010972fbac35", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Bronze (FF1 Hack).nes */
	{"e774bc72ad603cb945c91df6303a81e62ca19b21", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy by Atomis (FF1 Hack).nes */
	{"f03dc3d3350ae97f1781d673073f86468dfa3c69", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy by Phin (FF1 Hack).nes */
	{"9fe934db5d36dceb846ecab349da0dda2da65c65", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Cecil (FF1 Hack) [b1].nes */
	{"426f9a1cd4a705188faa456d46e7446af516a350", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Challenge (FF1 Hack).nes */
	{"6379ad4c2408d4d723cc4c5bf649b4fed40ccb3c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Christmas (FF1 Hack).nes */
	{"04741fa412ad291dd5e829ee8f119ee8add00996", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Dark Cecil (FF1 Hack).nes */
	{"8e03225fb1eb6fb816132770eb393c4c5fc36506", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - Dark Warriors (FF1 Hack).nes */
	{"d128ce09fd9946c56beb5151bf3a17392cff6af5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Deluxe Edition V2.3 (FF1 Hack).nes */
	{"e07f2049fb74f5696029cd115cadd3dbb2821906", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Deluxe Edition Vx.x (FF1 Hack) [a1].nes */
	{"c731ad8e1faa467e62a4ad8579d551c11b41fa2a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Deluxe Edition Vx.x (FF1 Hack).nes */
	{"e930992b478cf53234f55a39ba79640c6d40e47a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Double-Plus (FF1 Hack).nes */
	{"c42e20c2988c148982d6236ffc07a58c7bff2659", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Double-Plus v1.45 (FF1 Hack).nes */
	{"313d8fbe9cc151713ee364c2d8dd23dfd18754d9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Double-Plus - World of Chaos v1.1 (FF1 Hack).nes */
	{"97ee6b0bce0c054b4a8501dea9fa01579aacf574", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Easy Type Plus (FF1 Hack).nes */
	{"45a10a3cf541749e70555d6bb0956fdbc52080fc", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Easy Type Pristine (FF1 Hack).nes */
	{"05221fa8bfa0c618d48bfa9aa100061643593619", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Epica V1.7 by Clomax Dominion (FF1 Hack).nes */
	{"1b5e3e3477de4bc02eb4190931a780cffceea936", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Epica V1.9a by Clomax Dominion (FF1 Hack).nes */
	{"33a72b15ae8fa74965be0d4dbcbb8aa1a45a6667", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Epica V2.0 by Clomax Dominion (FF1 Hack).nes */
	{"c1694e8f2a0efa2f3e29757bca8af84f54d055c8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Funny (FF1 Hack).nes */
	{"c866c8f4a7532579da73c16c26ac98d45e5e5448", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Gaiden (FF1 Hack).nes */
	{"9c66bad1efef9f39bce0e83e560dccd136b3f8e0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Hard Type by Red Wizard (FF1 Hack).nes */
	{"2c371c6b2ed712fa91a29a606109ee09b7fb78e5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Hardtype V1.1 (FF1 Hack).nes */
	{"5fe401fab43580dda96079bd878da1d3e20fe9bf", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - Item Shop Enhancer (FF1 Hack).nes */
	{"a9d686c9596326e06203af4d8121e0bb8e7911c2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (J) (PRG0) [b1].nes */
	{"98a16de0d099214255d2afba1e6cd0d542f5898a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (J) (PRG0) [T+Kor].nes */
	{"c8a025472b23d6121b7c22efe64f6b3b2925851b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy J-Type (FF1 Hack).nes */
	{"cabbe1cbf4be869dec6a004768fc565533d2678e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - Knight of Argonia (FF1 Hack).nes */
	{"4e144373d0cc66eb2f027d177dbf2b116318e031", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy One by Jordan Bieber (FF1 Hack).nes */
	{"bece2facefcbf77e7e71d4b0db40d32baa09f63a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy One x2 (FF1 Hack).nes */
	{"1b9aa317a0fe9c991fd353b07f8eb59a88ed5f4c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - Or is It 2 (FF1 Hack).nes */
	{"192374f71e34df07ab84495a4ca6a0917b8dd2ab", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - Or is It (FF1 Hack).nes */
	{"5e1c9e642fff5724e5aecc7892a793b54c3c4da8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Plus (FF1 Hack).nes */
	{"016b2fe4717020d41ddea5e2a92411934f584916", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Plus Plus Alpha by Grond (FF1 Hack).nes */
	{"3f07a2e35f8820d474e4ca28ad9ecc7af3f67007", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Plus Plus V1.3 by Grond (FF1 Hack).nes */
	{"76765fc97cc60b97914497bca1fc1d55a5fd8230", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Plus Plus V1.41 by Grond (FF1 Hack).nes */
	{"29006f6eb9235cf3450ee710465c929225b495cd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Plus Plus V1.4 by Grond (FF1 Hack).nes */
	{"b4ab2930d5c46edb1b166d232bff53a1af0ad4d9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Plus Plus World of Chaos V1.0 by Grond (FF1 Hack).nes */
	{"8a9fbfc0d753f21e6610ddece8da8538ed73e986", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Remix (FF1 Hack) [a1].nes */
	{"f579c22d3ad3af4759208deb6d994b0a8224df38", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Remix (FF1 Hack).nes */
	{"f776a3f9e86d322d0a393426aff8d048db7c403f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Revamp (FF1 Hack).nes */
	{"ce3e101301a010dc80b643047a95250da819cc5a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Revolution (FF1 Hack).nes */
	{"0b9ee2916026c455a31574c6c863f4ce71581499", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - Sliders (FF1 Hack).nes */
	{"fb443e2bb923569e459356882b3d5e482ece31b5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Special Edition (FF1 Hack).nes */
	{"0d4fa005a457f54096a7d78b7ec1328502c9316d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - Spiritua Rising V2.0 (FF1 Hack).nes */
	{"cc9b79c3a4df10f95f02280a33ae2e8d4beee9e2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy SR (FF1 Hack).nes */
	{"4646f43e56739eeb4d2dae013eb994ae398a94ab", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - The New Class (FF1 Hack).nes */
	{"24bcfd997ace354636305a3edf8058673080953c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [b1].nes */
	{"57b8855ab034cbe05e33a1aee81a0af72c92935d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [b1][T+Esperanto].nes */
	{"4533398dbe9aff9c9e70507faf88968f159ffb9f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [h1].nes */
	{"ea97b4e2a30c4f064cd7efef5d7013b3c49a9b20", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [h2].nes */
	{"9cefdb11d68f5c08ad335696478c6470ec9b285d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [h3].nes */
	{"11f764ea96428d05f8163d612e2eb40af6a9ec36", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [h4].nes */
	{"34d9a2c54517262fecf4d813970da07929261cfb", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [!].nes */
	{"c9cfbf5455085e198dce039298b083cd6fc88bce", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [o1][T-Por_CBT].nes */
	{"333009741e3855293ff1d72092fcb09d5a0093b9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Fin1.00_Tuoppi].nes */
	{"aa454e44d9405214f0d48f1e38736f6e3b5bce98", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Ger1.00_StarTrans].nes */
	{"2807314edf4ce180384301a3a6a230cdf4bfd1e4", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Ger].nes */
	{"7eadacb31b1454f20be56f399a5b273526d14a01", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Gre1.0_Rouchliogiama,Zero_Soul,Giama].nes */
	{"57e0db09dc12bfd63ff5b4963d8841d2dc67c2e1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Ita0.14_Brisma].nes */
	{"6575d838458679d291b3b282a39a93137973ab27", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Spa100_Chilensis].nes */
	{"b87c57314b1262a1dc7e5ba2aea9cd728b29a4e0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Spa100%Corrected_Sayans Traduction].nes */
	{"d89debf98eecc8b57b955fed18d0c495b9da3f8a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Spa100%_PaladinKnights][b1].nes */
	{"4bf131b854a6cfb54cb7c6aa4111fce61a842e31", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T-Spa100%_Sayans Traductions][b1].nes */
	{"54f8e20e886a6120743cf7d1dc697b71bd295769", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Spa2.0].nes */
	{"de9710610d4427dd3f54f3bd70bc698dea6a2183", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (V1.3 JChar Hack).nes */
	{"e17f3ccd800ff78425bb927cb245f16fd58f468a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy X2 (FF1 Hack).nes */
	{"aa716fd92211290c711625950061389597b5eaa3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy XR (FF1 Hack).nes */
	{"c79fa720e7643e326d0d5f7c5fd7efe5b36d0010", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy Z (FF1 Hack).nes */
	{"780605d59988fe5e7979ea8161861c2ce6d50010", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Grond's Final Fantasy Bugfix & Balancing (FF1 Hack).nes */
	{"76b3c8a867a11cdaf9b8c1be4bb6db09010c2d9e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lenophis' Other Final Fantasy (FF1 Hack).nes */
	{"cd0c873b0911c8e7268ad5d3e4a52604b845593d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Marak's Final Fantasy Mod (FF1 Hack).nes */
	{"e5cde82a5c412f64916a005f0bfb079eee5cfa37", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Greg Norman's Golf Power (U) [!].nes */
	{"ec5498889cf0543de2c71836788766bc0f5fb98b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hatris (J) [!].nes */
	{"487904836e6a4eb0d2f46bca5ec1e634d1a93348", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hatris (U) [b1].nes */
	{"3bad23fb5e2fe0db3c51e7996ebfc8feb2aa9885", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hatris (U) [b2].nes */
	{"201e51f6fd28b200e8fc9c8676d7e487fd857ca7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hatris (U) [!].nes */
	{"13d2e9997c02462214fa789bc939f4ddab6d65a1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Advanced Dungeons & Dragons - Hillsfar (J) [b1].nes */
	{"67d8e0e135b8e38164d791f4846346d3aa5787c4", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Advanced Dungeons & Dragons - Hillsfar (J) [b2].nes */
	{"b3c062258eb6694d4888b0aa58a54d1d53c23711", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Advanced Dungeons & Dragons - Hillsfar (J).nes */
	{"85dc8bf106cbe7e3359b30337dfb07abb43a31b2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Advanced Dungeons & Dragons - Hillsfar (U) [!].nes */
	{"b2662816d0367143d41a697b7b714f312e9ac125", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Icarus in The Hood (Kid Icarus Hack).nes */
	{"85668f4cb733525db5a7d6bf7c9863bf6aec9f2b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (E) (PRG1) [!].nes */
	{"85de67a28e01ef680f2ff6aae80e4315491ceee9", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus Remix V1.0 by Clomax Dominion (Hack).nes */
	{"7539fb1d95fac583ef4af898e6b254911d7ea204", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [b1].nes */
	{"2c27ec6ecc67645a3a3dec3496dc4ab5d35049af", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [b1][T+Chi_MS emumax].nes */
	{"038cab15d869da759b6b7f9e4688da70a4582ef3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [b1][T+Por95%_Mini Translations].nes */
	{"ad707827d554063cd1785b4850031582503985b7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [!].nes */
	{"920b7e56f917690b89258a627edacf2a4650685d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Fre1.0_Generation IX].nes */
	{"776b9007173e9e7fe837dec72394c71c50073ad3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Fre_Cyril125].nes */
	{"7be1295d66c12a9534054ebee257da4f5cf69873", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T-Fre].nes */
	{"a5bbc19ef719b48c60bee1045cf78aab14161fd1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Fre].nes */
	{"1c09436be09fa655d96c8273347e1d6106f156e1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Ger1.0].nes */
	{"cff052d52a615fec88c606d108fde4f4a7fac386", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Ita1.0_Clomax_Dominion].nes */
	{"e3bfcefee719a18d777bf6aa124d294300029538", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Jap][a1].nes */
	{"0ad04f59ffe88434b11c351689a378d06c053c99", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Jap].nes */
	{"f236ab8978bc9436ba65f69e7e9f6adeb042e9e5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Por95%_Mini Translations].nes */
	{"b86ecc6d9f9dffca4a2ea62ccf5966d3a3439965", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kid Icarus (UE) [T+Swe1.1b_MetalHead].nes */
	{"a19ae6503af2a2adc2e842640dd8c7fc0af2948d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NES Land Journey (Kid Icarus Hack).nes */
	{"18459a0f9b4b7cbd3049a6a0c56ffc8cb7952f93", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Pikachu (Zelda Hack).nes */
	{"2869dae2ecb3d59f8f0ce88b498d03787812790d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Pokemon, The (Zelda Hack).nes */
	{"5c04aa6e3136e4b2629fe8cd0c9e3ea0852cac6c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda GST (Zelda Hack).nes */
	{"ce415ebeb34f18dbab7a3f8e9c194b49e535ba9d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda Outlands by Clomax Dominion (Zelda Hack).nes */
	{"904302290634038446359ec2bcaef42d05c3c4ec", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda Outlands Final by GameMakr24 (Zelda Hack).nes */
	{"dd6eb20634ef75f908697dffd209cb83449f16cd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda Pocket Edition V1.0 by PocketNES (Zelda PRG0 Hack).nes */
	{"670113d59d2583e4d56a58c0e075416f686cda44", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda Pocket Edition V1.0 by PocketNES (Zelda PRG1 Hack).nes */
	{"f42579ca8281e707650cccdd3187375ae4a54735", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (E) (PRG0) [!].nes */
	{"6322960a969e008cc63bf14879d98412096bc795", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (E) (PRG1) [!].nes */
	{"4af52c8aaeab49a2e74131f076bea6d2cedba687", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (Esperanto Hack).nes */
	{"f4aab5e5e6b40c8ffe19213c760fbb932dff6de8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The - Ganon's Deception V0.2PRG0 (Hack).nes */
	{"95e022b904ab75c34af12dfb98023a8643e35b93", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The - Ganon's Deception V0.2PRG1 (Hack).nes */
	{"6eba10e48257dfc8afaa2a23bc8cdfe5a53c24d6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (GC) [a1].nes */
	{"bb1be04d91135fe68e39c6d2e05ea6e888db0d85", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (GC).nes */
	{"2ff3122a9d499920bdeeac9f3033fbd74ff5d94a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The - Special Edition (Zelda Hack).nes */
	{"fab0041ec97f9eed55384394660c46e997b1d9fe", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [b1].nes */
	{"f84b24b75c4a7d550e7cdd541250afb3a42ee662", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [b2].nes */
	{"04fa6e84cbc97b133880ec0110ce468c61ad4742", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [b3].nes */
	{"d3d50daa1697955c2f28cc40a9ab5a0395bf36cc", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [b4].nes */
	{"abebfc82386c32df059918f6ec04fa6b41f487fe", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [!].nes */
	{"a12d74c73a0481599a5d832361d168f4737bbcf6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Dan0.12].nes */
	{"5cbeaffd2d41d0675ac63735f382fe4e9a11a35b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Dut1.0_Ok Impala!].nes */
	{"f04eb992161fcf3148df25b451cfe91ed7b583f0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Fre.95].nes */
	{"e4d23c3145a2271a4c7763aabb986c90fce083b5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Ger].nes */
	{"54d2da1e02f0a0ecdb7f224fdf811819dd10a482", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Gre1.1_Lugia_13gr].nes */
	{"92e45017232c6288aa977010964a7cba9d143ca0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T-Gre].nes */
	{"5ddc6dd8b41717215c90361de295eb128d565456", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Ita1.0_Zombikiller].nes */
	{"0e404af7530669ec828404edb51d77dab7c25438", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Lit0.99b_Joshas].nes */
	{"90cfee443da6a742c426bb979cd4c20bc861b206", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Pol1.0_Nintendencja].nes */
	{"ac46aee692f06de5f5307979135ff7b24e37fda1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Pol_Dreamforce].nes */
	{"13d07bb25246f90670ab7a4d67ba292c8a8edb28", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Por100%_Emuroms].nes */
	{"7680e8790b715462fcf50474b5a0d10abedce628", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Por100%_Thirteen].nes */
	{"09c2e999c41f96c6eb39e9260b90756251c63590", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Por1.0_Hexagon].nes */
	{"9e4afa8a5ede7ab1d4d1acd0b8e9e77f6b9f19e9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Ser(Cyrillic)_AmishYugoslavTrans].nes */
	{"de5e48214839f911917c73f7ba7ed452f2741d1e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Ser(Latin)_AmishYugoslavTrans].nes */
	{"b140ef87b97a7127d494bd7ea31852a19d24e9b2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Spa1.1_Hyrule Knight].nes */
	{"ebd9467ba82e67a995f2c1987c606b739baa39ee", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T-Spa_Hyrule Knight].nes */
	{"287086d60a3b76a3836afcad5f48eacf45243537", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T-Spa].nes */
	{"15da794f8d6cddb3691c09cfcab363358db47b20", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T-Swe1.02b_TheTranslator][a1].nes */
	{"1e74792e53fcc177cc0065742bccffe51d812152", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T-Swe1.02b_TheTranslator].nes */
	{"6ba9b30059b7bc86916cb4031182e22969fc0079", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Swe2.0_TheTranslator].nes */
	{"41eea73c132da7f4ed24649d296a6c91cdd5fd97", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T-Swe.9_MetalHead].nes */
	{"ea2e8646a40658f8a4e841a23f0a5d950b07f7c6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG0) [T+Tur_knighTeen87].nes */
	{"511b31688a4c6d2b9ff3fcad81ffbf66fe5659ea", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG1) [b1].nes */
	{"d0b8bd154af2e62ce3d45ce566771ccdc5cbd8f4", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG1) [b2].nes */
	{"3890bcfc9bb98804a5a53d170621a0492f751fd2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG1) [!].nes */
	{"be2f5dc8c5ba8ec1a344a71f9fb204750af24fe7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG1) [T+Fre_Generation IX].nes */
	{"a10707e3c5ace3315095107696b27f5f15c549ad", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG1) [T+Nor1.00_Just4Fun].nes */
	{"f9a95e080e13439624a4fcac64281ecb62ba60e9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG1) [T+Ser(Cyrillic)_AmishYugoslavTrans].nes */
	{"22a2831cee17ce1feb8b4dc7222dc4232a490d40", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG1) [T+Ser(Latin)_AmishYugoslavTrans].nes */
	{"1a1c706b249f128dccf5eabe583b3bf902d2af41", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legend of Zelda, The (U) (PRG1) [T+Spa100%_Chilensis].nes */
	{"cb0bc3133b6baf4632d93c174b8e04448747cead", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Legendo of Zelda, The (Hack).nes */
	{"5cb0fc06d85f984beea2554fde14fe6128bf2a96", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [b1].nes */
	{"92d924c24a0e6ba963cfc206b4f4c134e982ea48", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [b1][T+Nor0.05].nes */
	{"082c6997749099e6e0ffa2e8b70b456ee18e9d32", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Croatian1.95_CROcOp].nes */
	{"7c97e357be1521c2f30b0697b81d6d74653f7954", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T-Croatian_CROcOp][a1].nes */
	{"6ecc1e05bdea4fd073520dab3dfc3583a85ebbd7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T-Croatian_CROcOp].nes */
	{"4bf4176fa1cbaf2acb9fceb018e1ac8d43165d18", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Dut1.0_OK Impala!].nes */
	{"af8c2e80ae987b0a8915119547fcf78fc8da99bf", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Esperanto].nes */
	{"d6d31638d28462389eb0267ebc28d67311ae4c5a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Fre0.95_Final Translation].nes */
	{"1e6ee0abde68dab373b52d9b35e8e1d470158219", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Fre2.1].nes */
	{"53dfe0655b91278ab3d60d0a1aaec20729c53f4e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T-Fre].nes */
	{"8b3f111bf34425a2e7cb0374048c14dd8d1fa2dc", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T-Ita1.2a_Weapon Hunter].nes */
	{"b9c4debfc358fd0095f24c61a7d0b3bdb1e9c6bc", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Ita1.2b_Weapon Hunter].nes */
	{"cd8dd8d8f7d52ef3c509dfe74f0977e60a4bfd86", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Nor0.05].nes */
	{"d6a67ffa1a7a47f53b8fb3dc3043c183d4a15b0a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Por1.3_CBT].nes */
	{"a5ccf7c88d47bb427f22d56b8a640578b314931d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Spa100_ereza].nes */
	{"08d9ed9347a55d4ee9c927a4beea0a760efff6be", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Spa100%_PaladinKnights].nes */
	{"ea597e6c257d56078ec6b80d00d736bab5a0cb81", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T-Spa100%_Sayans Traductions].nes */
	{"32c97a33e3c481238d1b6b587997559f2ed43db7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (U) [T+Swe1.0_General CoolNES].nes */
	{"63c34c0705b783f78e4c8cd66b3abeb87e19b00f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* M.U.L.E. (U) [!].nes */
	{"a67d3fb6e78305fda82b2907c5c8d83fc38b0b63", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (E).nes */
	{"e22854daf598b94c3f8620d42e959d4bf2c81bdd", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (F) [!].nes */
	{"a261161ba810e1e5fc6857e3c574702f57036892", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (G) [!].nes */
	{"84cde9618cb026b98c2ffff36ddc2326099c3155", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (I) [!].nes */
	{"f78c4c9a33b5cedce9cf9deefe15d2c9e5b9e3e1", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (S) [!].nes */
	{"8b6311003d26e3c90dc8a46f1a8f4b90e5078696", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (Sw) [!].nes */
	{"9fe2047809314f14fc1ccda622782bc02d35110f", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (U) [!].nes */
	{"8a8bbecc77fdf59826257754f357d38a7f825971", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (U) (Prototype) [!].nes */
	{"2e18ebb1e0dae649818a1d42b38f99fb3a0824e8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maniac Mansion (U) [T+Fre].nes */
	{"22560db1ee11a6c2d95e479014a9645b07fff09f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Adventures of Catman, The (Metroid Hack).nes */
	{"f705a5ef623fbb90c06d1a70c124bb1654ba7959", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bionic Metroid (Metroid Hack).nes */
	{"8f2613b6890af307b294d8120d0f4c18a15dec78", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Evaroid by Mogemogesama (Metroid Hack).nes */
	{"3917672268539e980571630178faf337beecde6d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy - Chaos Realm or Something (Metroid Hack).nes */
	{"8d618c043ef53f38575c4b55956befe68a83b802", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid 2000 (Metroid Hack).nes */
	{"b2acd5d4cfb6b7d291f1edbe9b08ec1b2258ef95", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Adventure (Metroid Hack).nes */
	{"a70d5ab9cf45461f4c08d035910faa7ca5338baf", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Alpha (Metroid Hack).nes */
	{"55f2a2df98831765cdc618f7675f555e27e107f2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Captive by SPJ (Metroid Hack).nes */
	{"dcc5d136f7f910e5b5de8539f3c17805f8852b96", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Challenge (Metroid Hack).nes */
	{"a60c609ed0e52f3f2ece7b1a9ce88980ff40558d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Challenger V0.40 (Metroid Hack).nes */
	{"8f6170f566c008fcb2b6f597e7b294961138bf0d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Challenger V0.55 (Metroid Hack).nes */
	{"2154a5c6bece4e8c0752ebd34112e01145ca67dc", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Challenger V0.67b (Metroid Hack).nes */
	{"022d91233634392a5a2198751264002887620de1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Challenger V0.75 (Metroid Hack).nes */
	{"3249bd5cc7ea1eebea3aa100ed15cdd0e92bf8d5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Challenger V0.87 (Metroid Hack).nes */
	{"8bbdf3a4569e59671584a67ef766b7f1f97e46e1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Deluxe v1.1 (Metroid Hack).nes */
	{"692f9cb18464f3bcae4f4041c70c916a46f333d4", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Demo NTSC (Metroid Hack).nes */
	{"7ea53415b65c4a522ac39b2eab11103d0c51c963", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Demo PAL (Metroid Hack).nes */
	{"0ba1d3f4d88f0eeb9519c3a59dad9f4a3d3438ac", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Dirty Pair (Metroid Hack).nes */
	{"174d1dbc4098cd28a8204ed1e325b9a007a8e877", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (E) (All Blocks Hack).nes */
	{"5c3e271a438670527288c01206e6936f7fd080a0", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (E) (Automap Hack).nes */
	{"d7209c51130bf3969761c87886038cafa4a16345", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (E) [!].nes */
	{"68a55eafcefa3014a4771cb7983d7db42f80456a", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Eternal Darkness (Metroid Hack).nes */
	{"f4b9c19e2ef6dbb3403cdd826506a4acba64501c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (E) [T+Spa050_PaladinKnights].nes */
	{"eb988362c1815ea5562ac694952ff0518e97665b", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (E) (Wall Jumping Hack).nes */
	{"b6784c84c5b89c70ec652d7bb1f7555b3aeb95c8", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Genocide (Metroid Hack).nes */
	{"f631532a0dd075273f37194f79715d8dfcb8a307", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Gone Wrong (Metroid Hack).nes */
	{"8c5dc63eaf4893bd005837895a2bcabedd3381f6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Hard Core Elite (Metroid Hack).nes */
	{"93484bba8d7440a98bd82ec2661a278421c3038b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Mario (Metroid Hack).nes */
	{"bb2c6140eddad28d50ec9be8146bfdebc1dae146", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Master (Metroid Hack).nes */
	{"74997a1c47561ebbff982ebccad524dae07f5e6b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Master v1.0 by Chris Allen (Metroid Hack).nes */
	{"e9d68cd1045e00db7afe99b4f8033eb30a4bfb02", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid M (Metroid Hack).nes */
	{"9243b694cdd329a88c598b93c696a9b700e36f74", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Moyado (Metroid Hack).nes */
	{"991014180383b0b3994e9fb66d73c4d7e268d185", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Mutation (Metroid Hack).nes */
	{"df657e419eeed9d5c4ccb9db7ed9b7803bffba17", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Omega (Metroid Hack).nes */
	{"fcae3323891e888b4d673240efb807e08addcdab", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Origin V0.2 (Metroid Hack).nes */
	{"55312092b536c76624739329cf5a3d98d93045ec", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (PC10).nes */
	{"fdbfc7871962f72a1ef57e5a7e456164fb93430b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Planet (Metroid Hack).nes */
	{"b2e26e9ee2b4999df102eae3d4cd9b90253a2f62", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Pocket Edition V1.0 by PocketNES (Metroid Hack).nes */
	{"15df15789efb2b7986a85e33d7c637ac77f513f3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Quest V3.0 (Metroid Hack).nes */
	{"69ac3f667edc248fad4ee0699af11b355cd3f07f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Remix V1.2 (Metroid Hack).nes */
	{"22e8154470e33cafd4129a99eb921da604c38fc8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid TSI (Metroid Hack).nes */
	{"4ddb4bc5f5f3b6885a909523a2a18e4326f6aa8a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid - Unused GFX Tiles & 99 Energy Units by Grond (Metroid Hack).nes */
	{"d5e906d340a7a2c4ee481f1390f9cd86ef2d4f63", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) (All Blocks Hack).nes */
	{"a220af02613951733e971338801702883ec302f0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) (Automap Hack).nes */
	{"3889acbdccfc351d8cf62a6df18fd49c4fd08caa", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [b1].nes */
	{"ba4374fdbc1f4787916ba27d4e8c1b5d6c2603ef", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [b2].nes */
	{"99e703eff099b19746ff48977cef27d9ba9458e6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [!].nes */
	{"d42b8d874b7da087afb711b03ac611fa9de441cc", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Fre].nes */
	{"285bec2e5779afa612b3ba53fd2ec14ae15d4720", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Fre_terminus].nes */
	{"8f194d63579d13c128f3e85fb2b3c9b3598bedfd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Ger1.00_StarTrans].nes */
	{"c3cf950d839367ff08248242c8bb7965528dd232", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Ita1.00_Clomax_Dominion].nes */
	{"a55717f0d74390f78e68046e4d923e799a13a304", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Por100%_E2P].nes */
	{"916320229983a846e4f95a1c82951d08c4f205a8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Por100%_Falcon].nes */
	{"3a70ee47648c38446e3336ee7cd42229375449d4", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Ser0.999_SeeGot].nes */
	{"9f8951b6716e309dad28e2b8beae6c1b5e8eac9a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Spa050_PaladinKnights].nes */
	{"a141f0926037068a799b5f6d337713f90a8737bd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Spa100b_ereza].nes */
	{"f184544067696eeed34a676066830339eb94a73f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) [T+Swe1.0_TheTranslator].nes */
	{"a9708111f7609b2897906357a26edca69da20a5d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid (U) (PRG0) (Wall Jumping Hack).nes */
	{"89c459af6829363971f2a8f684a77d99bbaca498", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid - Wart's Invasion (Metroid Hack).nes */
	{"66df949ad5ebe9430200ec0f4038460a74e6bc64", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid X V1.0 (Metroid Hack).nes */
	{"492793e60e123a37b78d938ef3200f0d44fe2e55", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid - Zebian Illusion (Metroid Hack).nes */
	{"2d790fc7e3ed83509479cc31f7016f1c80404476", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metroid Zelda (Metroid Hack).nes */
	{"5189318cf3b6c22f86bf30654649385c25391247", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* My Metroid (Metroid Hack).nes */
	{"7871bbb48a6c8bdb1bb442602a2e4ca2f3d7c793", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Samus's Dream (Metroid Hack).nes */
	{"6d257beb1c82c744d7be1a3f9e79675cdcdd9c4e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Samus Super Metroid Look by Grond (Metroid Hack).nes */
	{"078a4be73464ef248594be767a447f2cee9fbae2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Second World of Metroid (Metroid Hack).nes */
	{"3b4ead7d828f5e3cd724437b16936df7fc1a2dee", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NES Open Tournament Golf (E) [!].nes */
	{"4fd939373eed4add41a1c68c9c2412bd3b428f32", 1, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NES Open Tournament Golf (U) [b1].nes */
	{"3c7e808a05fc02ddfcbc9cccb693e60b0ee51e16", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NES Open Tournament Golf (U) [!].nes */
	{"c881749bded199785699f51c3925f940332efac1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Qix (U) [b1].nes */
	{"b16847980931fa9e8d9a74e9ad60f26fac03015f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Qix (U) [b2].nes */
	{"db385ecc16a7e54784fa46d0ab97eb138a1d0d87", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Qix (U) [!].nes */
	{"b0f6caf1292865c998b6a4a93dae3e66b7d00c39", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shingen The Ruler (U) [b1].nes */
	{"d887bae937d1202f6f400348c5da4e0081866526", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shingen The Ruler (U) [b2].nes */
	{"0809f2b790b36b283079aadd87e5c3cf6ad889e4", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shingen The Ruler (U) [b3].nes */
	{"375c26b527b4e9d0b2edc69cf233b0c41043a97e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shingen The Ruler (U) [!].nes */
	{"28d346fcd44cd94abbc02a93fa65b4df0d4216c8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario Open Golf (J) (PRG0) [!].nes */
	{"cfa21cc3b97794ac5083e1239751852b0b5f0194", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario Open Golf (J) (PRG1) [!].nes */
	{"9a5628dfb31b4e8f07079a30b58100884b5590a9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario Open Golf (PC10) [!].nes */
	{"2d5ac7c65ce63d409b6e0b2e2185d81bc7c57c69", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Spot (J).nes */
	{"8b62b3fc95957f52d146e7bc3c90ab33c4005aec", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Spot (U) [!].nes */
	{"29fb71f936a6a4ff8bff8f8ebbafe986b4a18cd3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Exodus - Graphics Improve V3 by Zero Soul (Hack).nes */
	{"aca664d776adaaf4370681b2cc2fa1ae088164d8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Exodus (U) [b1].nes */
	{"8eec36ee708938285ce0f3f61b264c06969f10dd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Exodus (U) [b2].nes */
	{"6587e29b7f9c73399dc95a5912f133c748d282fe", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Exodus (U) [b3].nes */
	{"3f74b31e5b41f3e147ecb0d91abca70065fc4bca", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Exodus (U) [b4].nes */
	{"5a4c504fe598904fd1f63c7f469edfcc5b971b0f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Exodus (U) [!].nes */
	{"053dae5a60dab8af9bdd8c70be72a805276658eb", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [b1].nes */
	{"7dfa2fa0d3ced5e9f420cffdf504cbaa6f47d327", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [b2].nes */
	{"28700efbcb1199ec8aa8535fb37ec04c0eb66176", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [b3].nes */
	{"48f95e537d7973513d7220ad13e88233407eb0ce", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [b4].nes */
	{"8952a6b22cacd008edd995331889ceaa19e4dd6a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [!].nes */
	{"957ecc025de4177ab582e7309dff283f16dc7130", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Quest of the Avatar (U) [!].nes */
	{"168eb95578671562d0b1532aa78162ad13a363f8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Warriors of Destiny (U) [b1].nes */
	{"b3dd2c680ba3073f94434be12c02a146bf8601f1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Warriors of Destiny (U) [!].nes */
	{"017e9f14580b76aae9ed2c0f1bf36864d1f6e165", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akagawa Jirou no Yuurei Ressha (J).nes */
	{"05779cc072cafcecb91be5c7eeb8fd86519a4a6b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bard's Tale II, The - The Destiny Knight (J).nes */
	{"6c2458af83ecd6c19416e9477548d1881932e142", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Best Play Pro Yakyuu '90 (J) [b1].nes */
	{"0f62a3448b22134e670502d8460bdeec7f1b77fe", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Best Play Pro Yakyuu '90 (J).nes */
	{"8f228f38437423388a2e0860c05b1798e20cfe95", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Best Play Pro Yakyuu II (J) [b1].nes */
	{"0aef3badd6a839be9e957fd638534c963b710af0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Best Play Pro Yakyuu II (J) [b2].nes */
	{"39872ace880b7407504f3f6ec57f3386c8b15baa", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Best Play Pro Yakyuu II (J).nes */
	{"ce103fe6971e2f97b82f13b8011c592039f46265", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bikkuriman World - Gekitou Sei Senshi (J) [b1].nes */
	{"b5d870add8c0b9a21a50109e39d93ec81ed6b3b1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bikkuriman World - Gekitou Sei Senshi (J) [b2].nes */
	{"e8e63807eda49a0a6029fe0da44129074aa81ba8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bikkuriman World - Gekitou Sei Senshi (J) [!].nes */
	{"2d47cb78c94f4a69e73ec32b5ad52fb9e62217a3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bikkuriman World - Gekitou Sei Senshi (J) [T+Eng].nes */
	{"fabc18b929261f9e193f266d1ef3ef832192e843", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bloody Warriors - Shan-Go no Gyakushuu (J) [b1].nes */
	{"1c522e7956637f7937f05523ea7a25db44431fd1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bloody Warriors - Shan-Go no Gyakushuu (J) [b2].nes */
	{"809bbd04c366b4960d4b4d44c26e1a31f4e41c61", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bloody Warriors - Shan-Go no Gyakushuu (J).nes */
	{"ae64b6fbcc992ef8a2b3a6b7d80ad598b168e39d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bloody Warriors - Shan-Go no Gyakushuu (J) [T+Eng0.13_Aushack].nes */
	{"bf3e600ac92b01d16b50b768802568593a2a2894", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon 4 - Kuro no Youjutsushi (J) [b1].nes */
	{"e73a6fc5c3f9646542eb6416b5af6def354f53e9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon 4 - Kuro no Youjutsushi (J) [b2].nes */
	{"dbccc979ec08fa28923aa1b54a7001c4b6d8ef1d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon 4 - Kuro no Youjutsushi (J) [!].nes */
	{"46c7a91d77da4362efdee1ce61fa1eb641988dd5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon 4 - Kuro no Youjutsushi (J) [o1][T+Eng1.0_Dragoon-X].nes */
	{"17131743140ec780fe5cc209d58f20be4b7b4888", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon 4 - Kuro no Youjutsushi (J) [T+Eng0.1_OffCourse].nes */
	{"9cce98293cd7c307948eed903394987530161a56", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon 4 - Kuro no Youjutsushi (J) [T-Eng][a1].nes */
	{"e9538ea6211b4d49391ed478c6b449af3f4eaff8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon 4 - Kuro no Youjutsushi (J) [T-Eng].nes */
	{"efb10c57770bf26be9170f816d95107a9003ab21", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon III - Yuushi heno Tabi (J) [!].nes */
	{"9820b7bfba13097b4de2bfa4aacea4a4609f89d1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon III - Yuushi heno Tabi (J) [T+Eng_SSX].nes */
	{"15c3720f47c933d0a17b1890b3c8811d3d96a8cd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Doraemon - Giga Zombie no Gyakushuu (J) [!].nes */
	{"5c2376e02696e4170f9bd48855db483147e7c3d8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Doraemon - Giga Zombie no Gyakushuu (J) [o1][T+Eng1.0_WakdHacks].nes */
	{"4f5efb6f37352301d5f2507a04817dd8acaa4301", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest III - Soshite Densetsu e... (J) (PRG0) [!].nes */
	{"5759a9d658d253c8a6aaa38969c443d66e0f3349", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest III - Soshite Densetsu e... (J) (PRG0) [T+Eng0.0111_Spinner_8].nes */
	{"b5844d5432db7bdfccba5cffc083ddd60e097f79", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest III - Soshite Densetsu e... (J) (PRG1) [!].nes */
	{"9df34897d002e8d0e4d53b76154f8cd46927cdf9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy 99 (DQ3 Hack).nes */
	{"db2803a7126a6a4c2a00ebe3454123c8201163c8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dungeon Kid (J) [b1].nes */
	{"1b6ceb49eb1aa8063d7007f281cc23b200e1eff5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dungeon Kid (J).nes */
	{"34ee4d5913cd3d868a121aeb195dbf7c628ab359", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (J) (PRG0) [b2].nes */
	{"18af0afdb81595b4d181f63717015863cff4f83e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (J) (PRG0) [!].nes */
	{"e6296ad3e929e9b0219ac1d4364990e2ba912f4c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (J) (PRG1) [!].nes */
	{"c4c46c1be3ebf607e31010cc63258f22a5d98f84", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [!].nes */
	{"070d4ce87eb4abcc9f9d2a97b8010881cb57defa", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Bra10%_BRGames].nes */
	{"16462c7178a4eea53dd360efdb37ca5d42d83efa", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Eng0.36_Toma&AlanMidas].nes */
	{"4bedae341f228a01398e2995f4669e51874905a9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T-Eng1.02_Demiforce].nes */
	{"18874d6b40b12a842a9b6a0c68315b020b3eee96", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Eng1.03_Demiforce].nes */
	{"f5346a349e4aa9a5351235f1f527a3041298461c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Eng1.03PasoFami_Demiforce].nes */
	{"e6c84c6ffc2606fd223b457968c2f24a37ad77dd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T-Eng1.0].nes */
	{"b3d2c02a629cee8f43b43ad6e4ae5f2fe3e64be5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Ita0.99a_Weapon Hunter].nes */
	{"4e86168e460f1f331be2b39955487b71ee5e77e9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Por100%_Thirteen Traducoes].nes */
	{"b85fc3badfebe0155f9697b8c4caad7a08ecd230", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Spa010_PaladinKnights][b1].nes */
	{"c5052c1bc4f1fd2cb312287206d134e32a654a70", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Spa030_butz][b1].nes */
	{"cb1e285ee7fc08ca15a8d07497ac954f292402d7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [T+Swe_cyndeline].nes */
	{"790229e65a02a7a048dc20506a3dd75b1200f22b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gambler Jiko Chuushin Ha 2 (J) [b1].nes */
	{"d9c196eb6d09a5570678462b709f08cedb25d972", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gambler Jiko Chuushin Ha 2 (J).nes */
	{"8ef9616555199946c3224125337016fc5cf13e95", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gambler Jiko Chuushin Ha - Mahjong Game (J) [!].nes */
	{"31e7a573cb6409774901ebbe8f09b056d34d0225", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hanjuku Eiyuu (J) [b1].nes */
	{"5d52397b5293a7ea151620eaf880c9e8cb7ac14c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hanjuku Eiyuu (J).nes */
	{"cc781a17f800614b9d5e5c1661fdcd32cea7a7b8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Herakles no Eikou 2 - Titan no Metsubou (J).nes */
	{"7c6bed2ec057ba526abd00290a1f935c20bc4770", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Herakles no Eikou 2 - Titan no Metsubou (J) [o1][T-Eng1.0].nes */
	{"547e9aff068236f848349d273cf02751ea5efea2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Herakles no Eikou 2 - Titan no Metsubou (J) [T-Eng0.13_Toma].nes */
	{"8d8557e1863be45cebf1fd99624f4bc16f0fbd9d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Herakles no Eikou 2 - Titan no Metsubou (J) [T-Eng1.1].nes */
	{"dfff8533e1bb7cafc56de20b55204f73403ef431", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Herakles no Eikou 2 - Titan no Metsubou (J) [T+Eng1.31_Toma].nes */
	{"4fd46d48bd03ed0a8cb8e68be416c0ff9bf03dc2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Herakles no Eikou 2 - Titan no Metsubou (J) [T-Eng1.3_Toma].nes */
	{"817b4a9b580fe18ce44053f83316979357ba0fed", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Herakles no Eikou 2 - Titan no Metsubou (J) [T-Eng_SpoonyBard].nes */
	{"f6b5cae5e9eba3e37ea6bb229eb6dce1581414fb", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hissatsu Doujou Yaburi (J).nes */
	{"76218a9a6ccac652acd182153fcfea45e9ed4369", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hitler no Fukkatsu - Top Secret (J) [!].nes */
	{"95b2782b26fd1cef86997e59885358c410f832b0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hitler no Fukkatsu - Top Secret (J) [T+Chi][a1].nes */
	{"94cf0d2f95552df5c7aba9d9b29bf94fd96bd5a8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hitler no Fukkatsu - Top Secret (J) [T+Chi].nes */
	{"00bf2e28042e4ecf6243b56826f062d4ea80dcd0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hokuto no Ken 3 - Shin Seiki Souzou Seiken Restuden (J) [!].nes */
	{"3b7c9a0988f6246d253379d5de725602a202f68e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hokuto no Ken 4 - Shichisei Haken Den - Hokuto Shinken no Kanata he (J).nes */
	{"c6fef52264372fab620d1e5ee6a3e60e46262775", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hototogisu (J) [!].nes */
	{"16d8f322ed337255a15b607fd1fb77950d59ad56", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Isaki Shuugorou no Keiba Hisshou Gaku (J).nes */
	{"028c17789bcb480537a41fd576ad4454724b8339", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mahjong Taikai (J).nes */
	{"c03a3e92a65c031a1a03036e965ef3048974ca73", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maison Ikkoku (J).nes */
	{"263409a1374f520bca8656a17cb85344402aaab6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashin Eiyuu Den Wataru Gaiden (J) [b1].nes */
	{"3828d87e7932edede39655929b7dd9f286927952", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashin Eiyuu Den Wataru Gaiden (J) [b2].nes */
	{"9a1b586d1f62bf33ea9fad1c80d12db3450c3d41", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashin Eiyuu Den Wataru Gaiden (J) [b3].nes */
	{"c9ab0211ca9972c38c846065d2cbef2454689021", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashin Eiyuu Den Wataru Gaiden (J) [!].nes */
	{"adbeaa030ef8713d45cc7cb596fcd1976cb1b780", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Matsumoto Tooru no Kabushiki Hisshou Gaku 2 (J) [!].nes */
	{"c3463fc08a3553c77ebd6114c5f1910d63773a85", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Matsumoto Tooru no Kabushiki Hisshou Gaku (J) [!].nes */
	{"644b0b587b1da22989efc5e4fc1b62222a9cce05", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mirai Senshi - Lios (J) [b1].nes */
	{"ad3044682a92c9328e44e4094f2b80bc80ace34a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mirai Senshi - Lios (J).nes */
	{"3473fd11551a399041ddeef6adb45ae9d8d08b7d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mirai Senshi - Lios (J) [T+Eng1.00_AGTP].nes */
	{"f1d09fdbc80d71f02ad24b9a2f979e2fcad71a72", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Monster Maker - 7 Tsu no Hihou (J).nes */
	{"68f558cff6d8aa33bf5584597bc7e79bba0f09bd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Morita Kazuo no Shougi (J).nes */
	{"8a8d9dae0a48b2bdb4d35e4b7eb2370327bed5b8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Navy Blue (J).nes */
	{"0dffb53fb89240fef1967d51969508b15f8eebbe", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu Satsujin Jiken! (J) [!].nes */
	{"474b0728b0eb3d48ed3ef0c650f35230d526352d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Satsui no Kaisou - Power Soft Satsujin Jiken (J) [!].nes */
	{"86ade7be14f83eb0ec39fa52aa4a5dcd109aefa1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shin Satomi Hakken-Den - Hikari to Yami no Tatakai (J).nes */
	{"835a0f9a3ddd516cecbccf34fbe8f7632f297558", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shinsenden (J) [b1].nes */
	{"61d77aebbe33778c43a1b00e4793609ee42eed8f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shinsenden (J) [!].nes */
	{"1d7f39341cb0cf7db0b6b1b18365d583735d7316", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shinsenden (J) [T-Eng][b1].nes */
	{"8a5e43dab630d0dafa4695fc3a6f2030379a39d9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shinsenden (J) [T-Eng][b2].nes */
	{"537939ed6c8e78db0a22dcf76032659ce40b793b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shinsenden (J) [T+Eng].nes */
	{"b475b8da33bd252bb572b3c690137f1ca668ba0f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shougun (J) [b1].nes */
	{"e8ad066248a8054a9d4694994d6564d01bd77ab0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shougun (J) [!].nes */
	{"dc5a63f35e4869daa30f4a21cc327db59e39bc27", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shoukoushi Ceddie (J) [b1].nes */
	{"7a74bb838c70d16885c066677107c5aa1c4e190a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shoukoushi Ceddie (J) [b2].nes */
	{"819bc4191b889cecd2101377e511a67ef6dd4e7b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shoukoushi Ceddie (J) [b3].nes */
	{"9f536cabea7d4355c1c5f7ff3ed2e0b9086a39ba", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shoukoushi Ceddie (J) [b4].nes */
	{"043adc467d3c2e328612e1409900080dfb9cbc99", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shoukoushi Ceddie (J).nes */
	{"ff7faff87beef1451c6c495b94aee280eee5d21f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Square no Tom Sawyer (J) [b1].nes */
	{"4d02c93809a93b4d6a7aedb57f1e1882d5801582", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Square no Tom Sawyer (J).nes */
	{"ff166fcad80c61a17641a7c029dd2eb3d99495fd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Square no Tom Sawyer (J) [T+Eng0.07_Kikuyama].nes */
	{"52e2e178a119fd1a6bffc434bc281ead6fb6458e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Black Onyx (J) [b1].nes */
	{"67d1300b763989ad80140b40d2cc60ca790b1e3a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Black Onyx (J) [h1].nes */
	{"1d97987494a01b783ae6e271a2fbb14b05a06b20", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Black Onyx (J).nes */
	{"824bb64c887d2f06886a3e491d2fd899ba13ccbc", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Momotarou Dentetsu (J) [!].nes */
	{"d144e4dc539a1b7af88f6830f815bd10dc8e74cf", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) [b1].nes */
	{"07e4f7761816c1ea787ba152a48804b6034f57b0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) [b1][T-Eng][a1].nes */
	{"c06648bd3024504fbc4cc93ebd74dd9ea2cf0ccf", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) [b1][T-Eng].nes */
	{"3deca0e4ad847cef7afdf7bd716c90c71923abab", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) [b2].nes */
	{"8986010c0af2ed0b3649c461d85262aea47c335a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) (Beta).nes */
	{"20970dddce40a8d006cd6c08b36dc909bae0ac68", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) [!].nes */
	{"a07bd0fdf7ba3a30fc4e40d50373cb77a9627eee", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) [T+Eng1.00_Gaijin+Suicidal].nes */
	{"41a1869af6aa6d1bf3758332a68a2ac07df8baa6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) [T+Por70%_Made in Brasil].nes */
	{"746e1631bf987cfcc78def36ac4e13df344ec861", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Takeda Shingen 2 (J) [!].nes */
	{"e892b4f6166d16b3b8a1720216980253fd9a9319", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tamura Koushou Mahjong Seminar (J) [b1].nes */
	{"98447d5ea02c2a89ace0e980dfeab1634ee3dbf4", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tamura Koushou Mahjong Seminar (J) [!].nes */
	{"084ac1e599f1be679eeebd5d51d604dc9e5708f3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tanigawa Kouji no Shougi Shinan 3 (J).nes */
	{"9d22023f55e608687776b53e27e016bb5f81caa6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tanigawa Kouji no Shougi Shinan II (J) [!].nes */
	{"47b019360f011b0fdde9203e5ea1eb023d5c3dc8", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sougi wo Kurau V1.52 (Tenchi wo Kurau Hack).nes */
	{"9a6b57610890090206e879d6b18c1767cc20945f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tenchi wo Kurau by Echigoya (Hack).nes */
	{"192a1ac845dc9e56d98d05197bd574183ba7d974", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tenchi wo Kurau by fenglianxilin (070810) (Hack).nes */
	{"1a106a6da485f75dadd01ac6344bc3874231381b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tenchi wo Kurau (J) [!].nes */
	{"70158a66caa9c27033f4e11c4272c33e73893496", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tenchi wo Kurau (J) [T+Chi1.13b_Partial].nes */
	{"6b76207e620e1a9396224f9f6e2438cb5f2a35b0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tetris 2 + BomBliss (J) [a1].nes */
	{"7a06ef6867f116cb9217d66813cc6e94e3da4a33", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tetris 2 + BomBliss (J) [!].nes */
	{"6d707aa6e1bc88dd905269936ff16a50f65bfbca", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Titan (J).nes */
	{"2bd0f4b345d8eccded95c1899099e8cee0adf967", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Seisha heno Michi (J).nes */
	{"2be4add0035ac8e5669e3e3b4b6c47f94291f555", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Vegas Connection - Casino Kara Ai wo Komete (J) [b1].nes */
	{"597c2ee30e0731b62f0906040a0eeb8ae080329a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Vegas Connection - Casino Kara Ai wo Komete (J) [b2].nes */
	{"fa0f71cd93c34dedbbc4fb631f348dd96ac2f866", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Vegas Connection - Casino Kara Ai wo Komete (J) [b3].nes */
	{"eb8370d567330c66aea69db0df8a05e1664e8ae1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Vegas Connection - Casino Kara Ai wo Komete (J) [!].nes */
	{"1cdc08894f9cce3bf0550cc271390379ec0cf5e6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [b1].nes */
	{"27dde808cbeabbbf6f9995428093c295ad0ff6b1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [b2].nes */
	{"84ba068f42aa43782fc730aeb94d8c6ea4ca40d1", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [b3].nes */
	{"6994ad9c03fbf166876ffe60fad14c4593d0519e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [b4].nes */
	{"74f81c2b36d733041918263ada0b07616823b41b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [b5].nes */
	{"ebbf1d04b222675763bbdcf9529527d7d4569566", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [b6].nes */
	{"7b0cda6fb3361c90eae1fbf7882dac3bbfc244c0", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [!].nes */
	{"5064c11751ea56acf28fdc63a866657e0035c42e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [o1][T+EngR1_DMullen].nes */
	{"97fca0ae3cd1dbc02f38941ef9ac892c8291cfa5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys (J) [T+Rus1.00_Shedevr].nes */
	{"4df3fcc5bd04793b42020207afe1a13d8f52f270", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zelda no Densetsu 1 - The Hyrule Fantasy (J).nes */
	{"8cd5a43785089a8bf3b121d975e6d248b796a9d3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zoids 2 - Zenebas no Gyakushuu (J) [b1].nes */
	{"b0283acf0593a6da5e7c026eaf32bb4cbeb05194", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zoids 2 - Zenebas no Gyakushuu (J) [b2].nes */
	{"27a84a4eb3ae7413cf47d4ab19d7e06fbcec7bf7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zoids 2 - Zenebas no Gyakushuu (J) [b3].nes */
	{"6b9f311055c90cca5b509753646fb8311a27426c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zoids 2 - Zenebas no Gyakushuu (J) [b4].nes */
	{"7f9bede62044615c23beb31e0af925666fad702d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zoids 2 - Zenebas no Gyakushuu (J).nes */
	{"3b6e0ef1458c883799923f6b86d106b488b5c6a3", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Genghis Khan (U) [b1].nes */
	{"5b061fd54d4356bc3ae1ced447ab3dacbbe2f94e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Genghis Khan (U) [!].nes */
	{"209911d7bd15abb7bef2e35a473df725b6738cd7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms (U) [b1].nes */
	{"dc031f6f0df12851c9d6d46ffab960f471e26f6f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms (U) [b2].nes */
	{"16a09d08379e1b9f35303bb72cc92d066b0e3cd9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms (U) [!].nes */
	{"8c98f1008eba860f6e1ce396f4c8f93899762fcb", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga's Ambition (U) [b1].nes */
	{"176e97c496b6dc72c808c2a1730532b5fc25379a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga's Ambition (U) [b2].nes */
	{"8f3c79109fcb0c27e7b9fc17dbb9590ba37051a5", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga's Ambition (U) [b3].nes */
	{"e15a5f46f317a6f40878b271dd5633a4c2f40ce6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga's Ambition (U) [!].nes */
	{"d9fe4f00109b7d75456e1673c2b30de68a125a5b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sangokushi (J) [f1].nes */
	{"b5fac490e012ba6a7fe0fd5ecb053826108a723c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sangokushi (J).nes */
	{"f987c9c26efa260da38d38e2130df7547bb7c557", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga no Yabou - Zenkoku Ban (J) [!].nes */
	{"3ea83db139c7e92ed1f7ff72ddbba038a9a22ea6", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga no Yabou - Zenkoku Han (J) [a1].nes */
	{"09d5fe3dbc077fcc3ac9438713f4e60b34749b43", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga no Yabou - Zenkoku Han (J) [b1].nes */
	{"1b39ff3d81eaa79f4f4fa0de2e4501b39fc67a6f", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Aoki Ookami to Shiroki Mejika - Genghis Khan (J) [b1].nes */
	{"ffe1b9cd23b070c3103d17d558b4cfa381a0ff24", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Aoki Ookami to Shiroki Mejika - Genghis Khan (J) [!].nes */
	{"35e34c7f7370a1ccb34014a62ad526bcc5f2b48b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Dragon Warrior IV (U) [b1].nes */
	{"da46fbf0d3bd89d3b3d3c9ff782b3357e2f6b638", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior IV (U) [!].nes */
	{"1b3dc265ba0d7e4b2cef9b3f8b0be0847df67c3c", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior IV (U) [o1].nes */
	{"3fb61976847bca4187db61f484bf21346b42c95a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior IV (U) [o2].nes */
	{"07560e7dad20594afe681fbaaaba65b5120e2dc9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior IV (U) [o3].nes */
	{"697ecccdd178809207726a05a5593d874ca39ca9", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior IV (U) [o4].nes */
	{"f97a6ed6b3f9d3656718be3cc03f558d5ec47079", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior IV (U) [o5].nes */
	{"e33d3a487678f35e05c0e5da7e4c30f8fefa600d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III Special Ed. V0.5 (Hack).nes */
	{"0334014a7226fd77265b2e8cac934261337dd345", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [b1].nes */
	{"d4ff3e71587c641138dcbd34315d8632babdfb37", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [b2].nes */
	{"67b099ab83b23f5b18cf4056c12529d881952c06", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [b3].nes */
	{"9ec4c10325312e278161860c4caf3144828c863e", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [b4].nes */
	{"37f5e426c6a4077c06e6d91fcf6d7c06c68cd85b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [b5].nes */
	{"6e545f84090934ee0bdbdf22bd95a096ef2c599d", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [b6].nes */
	{"63b59bd4514cbb90f98b32c73e5b0efe5e8981da", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [!].nes */
	{"e58b61b031a9f0dc168525654bcb9883ef61a976", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [T+Fre1.0_Generation IX].nes */
	{"09252a9be4bfb8f86d6de8eb27eb2fd02ba880d7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [T-FreBeta_Generation IX].nes */
	{"5b7ebae5b7fca9b946ab7238fa474e0c884ded4b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [T+Por1.1_CBT].nes */
	{"f2fa9b0a263aecb1cdf5af2b6c9d6f70dc70d1ec", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Warrior III (U) [T-Por_CBT].nes */
	{"7ebfce972e919db819168a0b28b271b5d5f35a90", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest IV Mayuge V1.0 (DQ4 Hack).nes */
	{"00284c3c9efa80f9d03cfe76528ada749ab657c2", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest IV - Michibikareshi Monotachi (J) (PRG1) [b1].nes */
	{"9b212cc8092c66ef336dca3bbbeb17518e8e357b", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest IV - Michibikareshi Monotachi (J) (PRG1) [b2].nes */
	{"8422c88f1a9674a91cb37d1f8f904560d33a73a7", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest IV - Michibikareshi Monotachi (J) (PRG1) [b4].nes */
	{"699ec5fdd03bb2687b60624684dce06b25851e74", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest IV - Michibikareshi Monotachi (J) (PRG1) [!].nes */
	{"4f5c643e6b2c960c3a9e010cb76499aed8408a7a", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest IV - Michibikareshi Monotachi (J) (PRG1) [o1].nes */
	{"739698bd6050572f24e739a20978e4760cf33e59", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ninjara Hoi! (J) [b1].nes */
	{"72dd667ad7f404fd66922783b07450591d11f3fc", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ninjara Hoi! (J) [b2].nes */
	{"6aadd786c16a5462d2da861b52c2f9c314a56369", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ninjara Hoi! (J) [!].nes */
	{"55df1d724f051b38630e2bd14bc02edf2650f2bd", 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Final Fantasy I & II (J) [b1].nes */
	{"faf8f5eb223ef867ae2bc91ea51fa4dcaede5f1a", 1, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy I & II (J) [b2].nes */
	{"b1147a379cefcb74b6bbf807abe77b349c6b6460", 1, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy I & II (J) [!].nes */
	{"215d399ee058d3be160916ed7596bcaa12f9adea", 1, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy I & II (J) [T+Eng1.00_Demiforce].nes */
	{"1591cc3ba500758aef0276d9dc7b2c0f0b371d11", 1, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Best Play Pro Yakyuu Special (J) [b1].nes */
	{"2fb686cef60a5bc7ca5637966065253a32ea8849", 1, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Best Play Pro Yakyuu Special (J) [b2].nes */
	{"5ed43edcbda73753751861382d61c7c7ed45c491", 1, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Best Play Pro Yakyuu Special (J).nes */
	{"340f507cfc3f3827ee0b7269814e08d634b807f4", 1, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- UxROM -------------------------------------------- */
	/* MazeOfGalious2.nes */
	{"9ff5514c5b7d5b2d98696faa068aae5edccbc5d6", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maze of Galious 2,The (English).nes */
	{"981ae03706c4cdd6547915c47aec0af299bdc442", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 100 Man $ Kid - Maboroshi no Teiou Hen (J) [hM02][b1].nes */
	{"5c77b1ef0d261591cdee00f380a001ebbb6c82ab", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 100 Man $ Kid - Maboroshi no Teiou Hen (J) [hM02].nes */
	{"2b3149b2226913261aa2e22ac0f851199cf88f4a", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bikkuriman World - Gekitou Sei Senshi (J) [p1][hM02].nes */
	{"8722c63831049acbd02a437937cf1817e5282374", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bloody Warriors - Shan-Go no Gyakushuu (J) [p1][hM02].nes */
	{"9c7844b8c8273dab366a4341086b8a5387a44c5c", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (J) [p1][hM02][b1].nes */
	{"f32dc02026cf98579870da73efa48474b2270144", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (J) [p1][hM02][o3].nes */
	{"3e5212e24f16fbf7d59c8e9888749f88665a90fd", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bomberman II (J) [p2][hM02].nes */
	{"4a34eb22d9a9720a2985f2fc9535881532badacc", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cadillac (J) [hM02][b1].nes */
	{"02ee8f61a261fb2e3464763c1850fb765ef504ab", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cadillac (J) [hM02][b2].nes */
	{"abe7e6c7c7f7f1dac1b9f8fd1fe4650e6cb920e1", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deep Dungeon III - Yuushi heno Tabi (J) [hM02].nes */
	{"8df7c5d690bdc5ea867805a80e3f8150cab7eb28", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Quest III - Soshite Densetsu e... (J) (PRG0) [hM02][o4].nes */
	{"f4b72cf188b90a3206cf1fa24ac3269002fa2060", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fantasy Zone (J) [hM02][b1].nes */
	{"1168b3518799031a091e398f61a7e996b9ef5210", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fantasy Zone (J) [hM02][b2].nes */
	{"118675c27e82742ff761ce02b8a4b03061cc9278", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fantasy Zone (J) [hM02][b3].nes */
	{"ebb448830a4e8dc195ebb9ca6bec8d360d429da6", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fantasy Zone (J) [hM02][o3].nes */
	{"22e69eb78d4f4808e99ba2dae02e168e215fe4d9", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Faxanadu (J) [hM02][b1].nes */
	{"9aa3844bb4d31841dca88e824752cf2dca2960ed", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Faxanadu (J) [hM02][o1].nes */
	{"6e839a9ededae48fe039f9f96d8ba9a29da8411f", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][b1].nes */
	{"36e6058c1885e5e9d1757894cbbff0f191c07e93", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][b2].nes */
	{"837e9a580747d465c9fe50f51dc6f9a52f70e8ea", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][b3].nes */
	{"4d949ee3aeaaf005fb442098e821da2a5bba3c07", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][b5].nes */
	{"b988d45b99f345375b3e64924835b222e0669f2e", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][o1].nes */
	{"3f4d41ad195b9328b83c06ffef1fb9413bde2b2e", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Bra100%_BRGames].nes */
	{"3652dde6977537a9b8b22947aba0bd5224f4a01d", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng1.02_Demiforce][a1].nes */
	{"23c8b8cd2964717b455763c40340d337f107ea53", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng1.02_Demiforce][a2].nes */
	{"ce0ca7ef4e508e79e847e4cc53174158cc745511", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng1.02_Demiforce][a3].nes */
	{"6a03506eda6c4b4bc6904f64b98357e6c6209851", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng1.02_Demiforce][a4].nes */
	{"fa1cbd9ae4ce5b4c35dba3b2a1adc3ec1ce3df39", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng1.02_Demiforce].nes */
	{"567b236a58ac653b6ef60c12e45412b6d6cdb409", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Eng1.03AltFont_Demiforce].nes */
	{"4b756702aacd6ad801759ab099175292a766e960", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Eng1.03_Demiforce][a1].nes */
	{"d2fee46b6a41df3d2c3e412e811da118f77b63e4", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Eng1.03_Demiforce].nes */
	{"d10cd464ac4aae6c2f6105fe0c7f29b6a05863a5", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng1.0][a1].nes */
	{"9dd9065b41cfa14ba12bd132643a6c03220fcf4d", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng1.0][a2].nes */
	{"c8928639fc695941d01d501389fb7f0ac92f133d", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng1.0].nes */
	{"0e90448e510d6cfb5e5f04d541609514c4da05b5", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng3-7-97][a1].nes */
	{"ed6e642892f659070cee9f87a5a0a2f6aebc9858", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng3-7-97][a2].nes */
	{"b6beddfcbdbdc52eb7725889692aea74fb5452d4", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng3-7-97].nes */
	{"a0e9ec0e1198a27045b6f3316cc7c9172e3cff9d", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng][b1].nes */
	{"1c961db982c80db9702f6701999a9b6df7478ca3", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Eng][b2].nes */
	{"247df53fd2906fc14f34809feb220eb421d34c31", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Eng_Toma&AlanMidas].nes */
	{"12be6b1495922796853bb5a2ae15100b4266a99a", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Fre0.98_Minh].nes */
	{"0a0bc87f5d782b9304621761deb510c38032fb96", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Fre1.02_Demiforce+Unk].nes */
	{"4efd92bced44092c5e8f0f303627f0cc59aa20d1", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T-Fre.98_Minh].nes */
	{"11445d96f1b6b1e13fc94eba0333e53937dc6c10", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Ger1.00_Demiforce&StarTrans].nes */
	{"95a0d39751f89d4b432045724cf34189552507b1", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Ger1.00][o1].nes */
	{"c05e97ed91ea91f2c00b2cce7118615cee880327", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy II (J) [hM02][T+Spa_Demiforce][a1].nes */
	{"b539fd087f287da40eb76d55bee2947b86f6ff7b", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (J) (PRG0) [hM02][a1].nes */
	{"9f8aea8fef93f5a4f8c692cf74d36c33d59bbcdf", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Final Fantasy (J) (PRG0) [hM02].nes */
	{"1a1c20bcba0ea4ec4f1a9c35edc2ceb065608330", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gambler Jiko Chuushin Ha - Mahjong Game (J) [p1][hM02].nes */
	{"a21e2d47646cc9217cfbf63e3095d620ada9dc2f", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hanjuku Eiyuu (J) [hM02].nes */
	{"7b04525e9ac0ad3531adcb1599c95b1b79721268", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hatris (J) [p1][hM02][o1].nes */
	{"2216db94be900976907d3d8ef95ef033b91272cb", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Highway Star (J) [hM02].nes */
	{"fb6f1525813699cb72f1a4b999ee3d2d713498a2", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Highway Star (J) [hM02][p1].nes */
	{"11fbe907d0d2ee7a85405a3ce72ab195b2ca663e", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hokuto no Ken 4 - Shichisei Haken Den - Hokuto Shinken no Kanata he (J) [hM02].nes */
	{"2732e21581ea1448fc97ad1b9aaa4608e2144e35", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hototogisu (J) [hM02].nes */
	{"34ba1410090955562cdf6172509c0118e628abf8", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ikari II - Dogosoken (J) [hM02][b1].nes */
	{"f98a590801e38e255cae9f923c92e0ff6a8de0f1", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ikari II - Dogosoken (J) [hM02].nes */
	{"722e60dac21e4a013b82df4c9816df629c222abf", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Indora no Hikari (J) [hM02][o1].nes */
	{"572f090fe2d50a829565faa5261c016555442dd0", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jesus - Kyoufu no Bio Monster (J) [hM02][b1].nes */
	{"fa46b1606e35c5f92b1e03807728093f306ca063", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kaguya Hime Densetsu (J) [hM02].nes */
	{"0a8ec4e41dfa5453355ab378ff66e3d132adef5d", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kujaku Ou (J) [hM02].nes */
	{"4ca93aebba09eface2370b62dbada51968c7df12", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lipple Island (J) [hM02][b1].nes */
	{"c6b8d4e5742ae0205630a2a3297afcb79ed2383e", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lipple Island (J) [hM02].nes */
	{"56aed69e440b0531a56bb5416e151c22a4a438cb", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maison Ikkoku (J) [hM02][b1].nes */
	{"7ae42bc3f381175d9ae0a4addfdd407514935627", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Maison Ikkoku (J) [hM02].nes */
	{"623a6f0e5def6683999727eece7dd015c031ebc8", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashou (J) [hM02][b1].nes */
	{"f1f3712345704ebb0040e3b82f9132804d477ad2", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mei Tantei Holmes - Kiri no London Satsujin Jiken (J) [hM02].nes */
	{"b4a7104955ce87ec9de4cd9a6a13d3486f88abfb", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mezase Pachi Pro - Pachio-kun (J) [hM02].nes */
	{"9d76be895e1ff6327ef254b9fd89b313d9e06ad9", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mizushima Shinji no Dai Koushien (J) [p1][hM02][b1].nes */
	{"9c513255f63fa5fd6478ac15f0b50fd322f35182", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mizushima Shinji no Dai Koushien (J) [p1][hM02].nes */
	{"4635e05a7978d9e8902a6b148aba9d728043b692", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Momotarou Dentetsu (J) [hM02].nes */
	{"bd9680e5664ec1f284198ad0017f7dc10a069a00", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga no Yabou - Zenkoku Han (J) [hM02].nes */
	{"9da6b5ea723f397da6b5845f03fa244d92526748", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pachio-kun 2 (J) [p1][hM02].nes */
	{"4b3b8fd6bfd912e6fcab627b4307aa5af2694e1d", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu Satsujin Jiken! (J) [hM02][b1].nes */
	{"378c201e3c40ccda3de468e4d0e19f90fe65d014", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu Satsujin Jiken! (J) [hM02].nes */
	{"a4e9dff3690d10b73f33bf1f78ce1864410f24f6", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu Satsujin Jiken! (J) [p1][hM02][o1].nes */
	{"599de3d8caee1682c431458e22002eadfdb4de70", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Reigen Doushi (J) [hM02].nes */
	{"46966e6f68e4c36a06609693e4a2b6fcb4dc12d5", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Salad no Kuni no Tomato Hime (J) [hM02].nes */
	{"dfaf8665e46c1cbee4b97be24c4c34e075611b7e", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Salamander (J) [hM02][b1].nes */
	{"5198332516cae2bb50defb5c416b2a1cca32636c", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Salamander (J) [hM02].nes */
	{"d7d3a476ee0f13e1fa8cf25371220acbbeed821c", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Salamander (J) [hM02][o1].nes */
	{"23a720e8ffe7021eac94757512a8b05b996e4750", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Satsui no Kaisou - Power Soft Satsujin Jiken (J) [hM02].nes */
	{"ccb056e832713aac60d0ee72cc143ef6dacc3713", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Senjou no Ookami (J) [hM02][b1].nes */
	{"388ebe65c9912f8f31bb0725b6513a0ae6d3c8b4", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Senjou no Ookami (J) [hM02][b2].nes */
	{"5439fdfe5ba8fe55c124dfefc2b9e3e4dc374104", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Senjou no Ookami (J) [hM02][b3].nes */
	{"1794f28fc4ed4a1b02b0a29113f2c9532f0722b4", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Senjou no Ookami (J) [hM02][o2].nes */
	{"81693871e71ec10d5acdd513df5a9074b4feceb5", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shanghai (J) [hM02].nes */
	{"d49806b9f020311e0b189e3e9a5de77c8a528f62", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shanghai (J) (Sample) [hM02].nes */
	{"9d09474ca9d01f93a4dc817fd4fbbc6969c53ec0", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shougun (J) [hM02].nes */
	{"1564d43eca3ba28ef12e53e36f60f02ce9b4cfa4", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shoukoushi Ceddie (J) [p1][hM02].nes */
	{"c8163167f684ec225189dcf459874148baf27c9f", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Space Harrier (J) [hM02].nes */
	{"50d1777ccca12bf6a7ad0ca66d07454fcfefddc3", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Space Harrier (J) [p1][hM02][b1].nes */
	{"17844272283a2dc089a9aaeee1fabd2e02a2c04d", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Space Harrier (J) [p1][hM02].nes */
	{"5ea8158b04324504c4e448fbe32b5d20accce2e9", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Black Onyx (J) [hM02].nes */
	{"016b66ef5b8f696835b7f19cdd60fa738262e4d3", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Momotarou Dentetsu (J) [p1][hM02].nes */
	{"6d5b740e6d65da594b2416bae1aa8f38566a118a", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Pinball (J) [hM02][o1].nes */
	{"592ba61cfc2f9db609faa74eca87d2fce69f728f", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sweet Home (J) [p1][hM02].nes */
	{"4753ddaf7b0a6db689aafb4e198405177c5d17ee", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Taiyou no Shinden - Asteka 2 (J) [hM02][b1].nes */
	{"4761c7f0426796eabd377d2f5dc83954c77e703a", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Taiyou no Shinden - Asteka 2 (J) [hM02].nes */
	{"b5396c92113031047c811d1d7f42ace756618f93", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [hM02][b1].nes */
	{"775876c70b29abbdc8924828901c5f9bc9bf05bb", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [hM02][b2].nes */
	{"f4fb683c3a8eced52f8d5157dc5cef4e4a007f21", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [hM02][b3].nes */
	{"9b69bac31d62abeb1e26f17cadc1ae782df92f72", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultima - Kyoufu no Exodus (J) [hM02].nes */
	{"1b143750dc8597372892ff4fa64c7e982b5e7cdb", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zoids 2 - Zenebas no Gyakushuu (J) [p1][hM02].nes */
	{"e81966607afceca236779c63134ff5a9333c4638", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Biocats (World) (Aftermarket) (Unl).nes */
	{"d53ea86cb0c5a1d485d49217471a23a27d7a074e", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Trog (U) [b1].nes */
	{"13c3394f08f4bc6d3df83db92693bdac9d091766", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Trog (U) [o1].nes */
	{"29ca7cc398b855472c9d94e7581be8288468382b", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mega Man (U).nes */
	{"6047e52929dfe8ed4708d325766ccb8d3d583c7d", 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Alfred Chicken (E) [!].nes */
	{"3f32f7fd57790c6dd72bb3fc2727e25e125ffd6e", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Asterix (E) [!].nes */
	{"7b0b8d19bd56aa255501852136828300ee2d2457", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blades of Steel (E) [b1].nes */
	{"40dc07e43d9e810bcf42a98a1cde243b0846a980", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blades of Steel (E) [!].nes */
	{"72aef6054bf5e8b3301fea396187c71454c2465d", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blues Brothers, The (E).nes */
	{"e0c970fc9c2cf42afba958b056e2dd5f99097ed8", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* California Games (E) [!].nes */
	{"d48db138130628dd91bf4e02fffe68dd6a06d380", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castelian (E) [!].nes */
	{"85abcffdc088e1a7b9af2c7eed0a75d72bb26cb4", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania (E) [!].nes */
	{"0ea29d38b0d1740234ded685546d5ed9e1b3699e", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania (E) [t1].nes */
	{"53c7418569d0d1dc4ca35ee7045797afd6553d4a", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania (E) [T+Fre2_Generation IX].nes */
	{"2895b6885e019ed34dd59cc88d8c1e4aff1ea590", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crackout (E) [!p].nes */
	{"aba26002dff0d265bc2ac27a2689924b4fe0ed81", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Double Dribble (E) [!].nes */
	{"c6617721f3d3e26fd7122b30e8fe070b230afd38", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Duck Tales 2 (E) [!].nes */
	{"eb07b89ac4d5557a5921661f77d63c629ceed63b", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Duck Tales 2 (E) (Prototype).nes */
	{"68d966faf6650f47c00f8678f2e39e998b38ef9b", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Duck Tales (E) [!].nes */
	{"1e4796ee09739c45f7fed2a9bbd1ceb132c4ff7f", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ghosts 'N Goblins (E) [!].nes */
	{"a369b2a4a468c9e69533709e0dc3495457c9b9b2", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies II, The (E) [!].nes */
	{"58a45fa6b22f0507d1575ea54e70122f469a600f", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies II, The (E) [t1].nes */
	{"9709fa04fe48182a86a1b26345ecfaf0549ac036", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies II, The (E) [t2].nes */
	{"f95ed5e680497cca7920e3051fc8e98cee614be9", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies II, The (E) [t3].nes */
	{"9f0eb212cf07f19e2288cbd638ee71954b0fbc64", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies II, The (E) [t4].nes */
	{"12b69f86a3162bee6c4affffc67488e3b9eb96cf", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Guardian Legend, The (E) [!].nes */
	{"68b2d42cb1848517106b64d169307a167c58a802", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gun.Smoke (E) [!].nes */
	{"df8e74a0f5af5f30710840a7d983eb3ca34f4389", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gun.Smoke (E) [T+Fre1.0_ks151].nes */
	{"a685e34509348c9bb73fa959bcbbb81363506043", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ikari Warriors (E) [!].nes */
	{"460bbe5f79bbb9a7526c5047974d17bb7cfe16c0", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Indiana Jones and the Last Crusade (E) [!].nes */
	{"e17a4458a5c94c5a019a2b60fc27ede742de51b3", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jack Nicklaus' Greatest 18 Holes of Major Championship Golf (E) [!].nes */
	{"fbec10a6cfaa15f54b0df380af8c3df8610f830b", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jimmy Connor's Tennis (E) [!].nes */
	{"a18e5d125ac4b057b3ec3ef3f0267577ef05f910", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kick Off (E) [b1].nes */
	{"154e33415af9ac4c031a1d7b170d481d41d45bc6", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kick Off (E) [!].nes */
	{"d03873980d19d0620cb0717c3661a3f61ce97295", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Konami Hyper Soccer (E) [!].nes */
	{"d9f4e6df6be501e6ed03ee1d8132accec4e883c6", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Life Force - Salamander (E) [!].nes */
	{"5b0936d61dbf262f536b8eeb491075ed4311f865", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Life Force - Salamander (E) [t1].nes */
	{"712cfc95d4f308bf1b5ae2f9ddb5917241b0d1b0", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Life Force - Salamander (E) [t2].nes */
	{"e8da7979bbdd62c502dea74c498b4804a15c84f8", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Life Force - Salamander (E) [t3].nes */
	{"d44262816f053043d94c9ca11e78227a1faac979", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Life Force - Salamander (E) [t4].nes */
	{"81279a8310809c2e841a3ece2996c959d1ab053e", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman (E) [!].nes */
	{"fdb6818e0efcbbd33cb9d17f1cca251734e268a5", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman (E) [T-Dut1.0_Ok Impala!].nes */
	{"a90c47d5dd386098219991f30d45422b07c84bd2", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman (E) [T+Dut1.1_Ok Impala!].nes */
	{"821a13869feaa69263c0c154dc939dfa727db9e7", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman (E) [T+Ita][b1].nes */
	{"5ff0068cf197bb6a7b5d55727c49f47f603e2a8a", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metal Gear (E) [!].nes */
	{"c00723784ce98fd9c1e60c02d1ecf592f0409d69", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metal Gear (E) [T+Spa080_chilensis].nes */
	{"4e57dec4b667855d9db1a65425d8b6b0751efab5", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Paperboy 2 (E) [!].nes */
	{"d5a685d4e79b84fb19542860f5371ab449905e78", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Phantom Air Mission (E).nes */
	{"c81c29a3ecba7ffc735e65273ab8bba886a33029", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Prince of Persia (E) [!].nes */
	{"ec6ba3a7d118783df5ae9f1d0323b38a3348f333", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Probotector (E) [!].nes */
	{"999aca2dfdbdcc164ddab12282290247f4ce21e1", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Wrestling (E) [!].nes */
	{"8ff2b9decac9513d08a29e7bd45d3c31f36f2966", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Robo Warrior (E).nes */
	{"97a51781dd24ac527a690c656502bb7c97ac9d71", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rod Land (E) [!].nes */
	{"ad5c446e1c3368295ce6431c79569a3b321aaf21", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rod Land (E) (Prototype) [!].nes */
	{"ea4e90bed794ac2c1db8bcfb74362f5223b816d9", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rush'n Attack (E) [!].nes */
	{"52fee5216352e714b8e10ea136a2c3aca640127d", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rush'n Attack (E) [t1].nes */
	{"2dc713473c6c497f736f7216ff160f3207cace63", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rygar (E) [!].nes */
	{"81cb1057c361acac0bb19bcc7b3b5e7c30d923fd", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rygar (E) [T+FreBeta_Generation IX].nes */
	{"fe2619501fb5af51c101b1891363b3efe6de9653", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rygar (E) [T-FreBeta].nes */
	{"a97727e79314227ba593d31ded064297f7eb4a41", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rygar (E) [T+Fre_Terminus].nes */
	{"13135f908f35b18700808b52a6b8427b4edc09e9", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rygar (E) [T+Spa099_spctrmxd].nes */
	{"b4835736d0a1571e67a2fcd7776886a870f058df", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Section-Z (E) [!].nes */
	{"fe81f6b0eb309c5484e18fe1d0e859add29a8dbd", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Side Pocket (E).nes */
	{"b1dc42eba45471436abe54f22d5727407b8eb4bb", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Silent Service (E) [!].nes */
	{"250fd2ffe1c7565e1469055147b67c4d4c799e56", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Skate or Die! (E) [!].nes */
	{"1c8dda6e0f1797aa5b9cbb56b11c33bc760bc3eb", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Smurfs, The (E) (M4) [!].nes */
	{"022facf54615ad37d269ac8f561da6ff51334cc1", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Swords and Serpents (E) [!].nes */
	{"0708f250dc4093edd4b715f660e2f67b5777cbcd", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Top Gun (E) [!].nes */
	{"a9a6b03ae4e260d01067352b3e64915daf2b4786", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Top Gun (E) [T+Fre_KS151].nes */
	{"7bbefd30688447fe891808b4e2a838fbe1de8a1d", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Total Recall (E) [!].nes */
	{"30670c049052cd9bb286c9f8c0c149ff7a241130", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Trog (E) [!].nes */
	{"60065354f49a9880407e871ad8d9eb6784004cfe", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Trojan (E) [!].nes */
	{"a4a60000249de76d2d4100cdb5c3dae0c7137f15", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* WWF WrestleMania Challenge (E) [!].nes */
	{"3b5614a315f41dc19c2ed41f9924433314719799", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Duck Tales 2 (G) [!].nes */
	{"52e60afdfaf9a80f39124d49fe1924e2e2256b7a", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Prince of Persia (G).nes */
	{"1b33af3acb40acf6fb3d8709246278249579d019", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Duck Tales 2 (F) [!].nes */
	{"ab207029d164baaf87c802e4fdbba06b575aec06", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Prince of Persia (F) [!].nes */
	{"6b6f19237e16b21374d4072d64a29d9b8683085a", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Swords and Serpents (F) [!].nes */
	{"c8293d5632795058bfe3e987b471710b2f8d69f1", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Prince of Persia (S) [!].nes */
	{"3f097632e2387fbc26d2f808438ea544cebe3d95", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Prince of Persia (S) [o1].nes */
	{"0b5672e139a98ce2aa49fd8f1be933d278e0161d", 2, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- CNROM -------------------------------------------- */
	/* Cybernoid - The Fighting Machine (U) [o2].nes */
	{"2b2dacd2dd74a6be31dafad273fa4a34c5785fe7", 3, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Alpha Mission (E) [!].nes */
	{"1293d08a7a5690aeb08f15b5e4ecec7703b9668b", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Athletic World (E) [!].nes */
	{"dec198c52fea50cf0a69e8f9f6dacd3f8ab87377", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Battleship (E) (M4) [!].nes */
	{"62f5f4dc9c8c1e0dfadcb2469e1d20c272406656", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* City Connection (E).nes */
	{"aa247c87bdfa04b95921679608043b9dbd37feec", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Drop Zone (E).nes */
	{"6eef8b3df065617359dc60a302e0bc9c0cc167cc", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gradius (E) [!].nes */
	{"ba9b8971965195eff20d77bff58d5128267941aa", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gradius (E) [t1].nes */
	{"b3ed77cb52470a12df5e32a4a14a7c726002a02d", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gradius (E) [t2].nes */
	{"0bdc283022ac6b32b914e88082a100ee766575df", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gradius (E) [t3].nes */
	{"e853b55f5f481755eac448ac0470fdd7988f0af2", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gradius (E) [t4].nes */
	{"142a7dc56fbd01722668d66605f9851ec4d3e29d", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gradius (E) [t5].nes */
	{"2726a875e030a334258c0c7150d1b59c33440762", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hudson's Adventure Island (E) [!].nes */
	{"3c5b9356f4695d2560bd6ed53a7629c8e8f7bf0a", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mighty Bomb Jack (E) [!].nes */
	{"4dda3c6b133c10213ef43be89927b7c84d5a4a52", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Paperboy (E) [!].nes */
	{"74706fe55534f97efc02764cbd8e4abde16f1b40", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Puzznic (E) [!].nes */
	{"0a195957ece20c9db955cc8641dda5d1bc09cd97", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Solomon's Key (E) [!].nes */
	{"80b6514e06f480bfc8ba18a70645516a96aed5c1", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Solomon's Key (E) [T+Fre1.0_ks151].nes */
	{"a0598fc58d273c9bcefa3892fceb9e8d5f451496", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Stadium Events (E) [!].nes */
	{"3d3b8269573666eef42c0d959d4d54a0c719ba04", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Star Force (E) [!].nes */
	{"43558458b5fd320899576d0cf73dd7c3899786a5", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiger-Heli (E) [!].nes */
	{"7b8cf32cdd641e33c0adfbf8b93130e74be80205", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiger-Heli (E) [T+FreBeta_ks151].nes */
	{"25896915492cfbde9965673c0c8e1e0f8ce8adc6", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Track & Field in Barcelona (E) [!].nes */
	{"e1cc56758f02204b6ba324f88699c2522ec4e499", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Monster Truck Rally (R) [!].nes */
	{"6270865daa1b4a7dff2ac303ac6a4132a7f7a414", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* quantum_disco_brothers_by_wAMMA.nes */
	{"2ab9a13a76f17546582ebe2c7ce7ed55713622bd", 3, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- MMC3 --------------------------------------------- */
	/* Summer Carnival '92 - Recca (U).nes */
	{"4042bbe6e1ca346d5b56889dfc1cfe6124761333", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Adventures of Tomley Kleene by Bob Rost (PD).nes */
	{"d7bbb045df0e5baa6c35aeda82e99b447dd3d05e", 4, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros 13 (SMB1 Hack).nes */
	{"36c7fce6519c9b24ca8c3ba294599e9ff98e2f91", 4, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros SAB (SMB1 Hack).nes */
	{"d6dd4da16966a41ee56ad380269cc1aa62567e8c", 4, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros Vertical (SMB1 Hack) [b1].nes */
	{"8a49c665547ff4f9c0446a3fa5123179a6e0dfcc", 4, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Tower RE Mario Bros (SMB1 Hack).nes */
	{"f2cdb4546a858914570f36e9ff86510eb0f65ca7", 4, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Rad Racer 2 (U) [b2].nes */
	{"f35d9cbcf160efef4d6d4b358da08125a728e8bf", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rad Racer 2 (U) [o1].nes */
	{"0956f7d9a216549dbd80b1dbf2653b36a320d0ab", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Pro Yakyuu - Family Stadium (J) [!].nes */
	{"08fd83bcecff6ab2117dbcde14d7e14c99028bac", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu - Family Stadium '85 PAL ('87 Hack).nes */
	{"eb3c23be0906317e52e39670b62f4e5b08c58211", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu - Family Stadium '87 (J) [!].nes */
	{"a2b44fc7346569b328389746499d18206d2e28c8", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu - Family Stadium '87 (J) [b1].nes */
	{"69e3d375282ec192a4a1acc19729072f57a1a882", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Family Boxing (J) [b1].nes */
	{"a495bd78a056aae191ac5ba056592f617aa5f470", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Family Boxing (J) [!].nes */
	{"cc6890c1e3d365e2ca633a350842dff53e515315", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Family Tennis (J) [b2].nes */
	{"dfa20d137fc9daebbd625a053973d34ba87a6c74", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Family Tennis (J) [!].nes */
	{"51b07f77fcbea59b1ea602a6d8b39409b3db98b5", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Family Circuit (J) [!].nes */
	{"625649dc40f11c7f514a6f60665b38d4c1242ff5", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mappy-Land (J) [b1].nes */
	{"27c9b0c95e9353e9eb71f5c238e4a894c5d11a52", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mappy-Land (J) [!].nes */
	{"6001a25f88c85d58f23ba6dc70d945722be5fed5", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Famista '89 - Kaimaku Han!! (J) [!].nes */
	{"78344ddcf7af1d8620a5a9eef2f77dc512379eb2", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '89 - Kaimaku Ban!! (J) [b3].nes */
	{"fe5661ca4e589536c3a2b548dadc317b5b9d3053", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '99 (Famista '89 Hack).nes */
	{"b485d11b4fd22266a74da8a8e85d67c52c0f4793", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Slayer 4 - Drasle Family (J) [b2].nes */
	{"165e82715474abd3afd8b0f24369718c54a69189", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Slayer 4 - Drasle Family (J) [!].nes */
	{"b040ebeb612b2e962dfd3f836897bcc6f53129c5", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Slayer 4 - Drasle Family (J) [T+Chi].nes */
	{"fdf0d5c5507c14b4c47c44b869a724d3e207f7d3", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jikuu Yuuden - Debias (J).nes */
	{"4a6889c6e6950deec49ad2aea96c2ab5599b77ea", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Quest of Ki, The (J).nes */
	{"02411824e0e8d31342eeaa2663cfedb13a6c394b", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Quest of Ki, The (J) [T+Chi].nes */
	{"a5eb93b6a6544f24de7744e2b7e22db3e97af922", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lupin Sansei - Pandora no Isan (J).nes */
	{"508847e90c47f76477392e402bc8e2d83d2c66ef", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lupin Sansei - Pandora no Isan (J) [T+Eng_Vice].nes */
	{"a71a4f2615dcc94addd6dc64e918741fb505e8a5", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu - Family Stadium '88 (J) [b1].nes */
	{"5aaa9a9a034272f83f73727e3145e1a72e92ad05", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Yakyuu - Family Stadium '88 (J) [!].nes */
	{"b0260107732a3eb61ec79f9654a0154b1b415aaa", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rasaaru Ishii no Childs Quest (J) [!].nes */
	{"d480fcabff77924cbd241e6e2b92d7f23fcb3214", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tenkaichi Bushi - Keru Naguuru (J) [!].nes */
	{"d272f60626efb8436d260c4760ade1c58c4cd074", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tenkaichi Bushi - Keru Naguuru (J) [T+Spa30%].nes */
	{"942ef8a4fae720066febfe22a0e8315093c06200", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wagyan Land (J) [!].nes */
	{"6f12b2f878a9af877134afa8bd937fc68705869b", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wagyan Land (J) [T+Eng100%_AlanMidas].nes */
	{"b517a5065bdb30acbba696ef7c6007dd1853786a", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Sky Kid (J) [!].nes */
	{"55254d37cd88b26aebbb484b91192f56c4a00b0b", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Metro-Cross (J) [!].nes */
	{"a3ab5d18cee63b89306522bc550e813f6cd174a2", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* PepsiMan by Mogemogesama (Metro-Cross Hack).nes */
	{"694533baca3d527501ff9352f7ec6a169d420459", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Crystalis (U).nes */
	{"f06b5725ac276b37bb578dd9d1b3eee67e5d38bd", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Super Mario Brothers 2 100% by ereza (SMB2 Hack).nes */
	{"73fefdeb318cbd4228042721128e862a91109133", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Super Mario Bros 2J + SMB1 Chars (Hack).nes */
	{"3af18925766baa141dce6ab67ce011fa94d643f7", 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Startropics (E) [!].nes */
	{"eee6be294652cba02e7f29e3b22967030623abff", 4, 1, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Startropics (U) [b3].nes */
	{"f53b55f6a9a2c6b8f4cf24c283a461d412354e4c", 4, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Startropics (U) [o3].nes */
	{"224d96bf55391c7e9b651eb4d6a8f06faf6e29da", 4, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Startropics II - Zoda's Revenge (U) [o1].nes */
	{"a185c8a6d01956d6d6c59910d27ce9507e4d2d5b", 4, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* 5-MMC3.nes */
	{"f8266c01494c0da28db89c405105274e71adaac0", 4, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 5-MMC3.nes */
	{"7ab1e18c8fbce1ac5f7699ea50ea05b5490fe3e0", 4, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 6.MMC3_rev_B.nes */
	{"0e971e2ccad1dee51a0c305ed38fafd2e6ca3b41", 4, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Adventures of Lolo 2 (E) [!].nes */
	{"1d99e097573e55457cc0858a06c63e8e6b76fb8f", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Alien 3 (E) [!].nes */
	{"e08ee23bf86bf54369647b83dacbcdfbceaacc34", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Astyanax (E) [!].nes */
	{"d3c6faa646f86754ae2ef3125eb6bda4f3420a1d", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bad Dudes Vs Dragon Ninja (E) [!].nes */
	{"356085c37570ceefb93752af560acd8f028e6752", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Batman (E) [!].nes */
	{"590584b4730ac4cb00d39aa80081b2d0794081b8", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Batman (E) [T+Fre].nes */
	{"a4288c55ffe2e049554c0a0fdeeb73418259f89e", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Batman Returns (E) [!].nes */
	{"ebf3405287b45aba0ba88bf2c53c02cc8030150a", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Beauty and the Beast (E) [!].nes */
	{"14937367f2941bb992ee0e3ec66158fbef48fd03", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blue Shadow (E) [o1].nes */
	{"f773b4062700944328b506187c8f04ca542c32b6", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blue Shadow (E) [T+Chi].nes */
	{"4f2b991acb3a51274eaa902a46c5dc4132651345", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blue Shadow (E) [T+Fre].nes */
	{"7edd9029171751a4c46962eee25d39061c737de3", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blue Shadow (E) [T+Ita1.1_Sephirot][a1].nes */
	{"b13c2e8de905f1294dd4a3ab871199f88840a6fb", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blue Shadow (E) [T+Ita1.1_Sephirot].nes */
	{"f7a3e22ac96fe7ef888213eb78d20e1786014781", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blue Shadow (E) [T+Rus].nes */
	{"a20afb3d381dc99b8ca2852c647ed5cfbb1cf1b1", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Blue Shadow (E) [T+Swe1.0_TheTranslator].nes */
	{"aedf7f64c654fd368456ead62177fc8f807d76f3", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bram Stoker's Dracula (E).nes */
	{"8d1887fff08bd1132aaeec90b04bf942011a59db", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bucky O'Hare (E) [!].nes */
	{"85e70f11357419d1be23eeb76282449b190f9bae", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bucky O'Hare (E) [t1].nes */
	{"fdfa9d30fb3bfdfee0269070fe25b907a26df346", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bucky O'Hare (E) [T+Rus].nes */
	{"67fb2a465f98431ae0cb864a4d4b36a3d2f9ca07", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bugs Bunny Blowout, The (E) [!].nes */
	{"35c0340e5605a315d4c7c294cc420840cb7cf481", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bugs Bunny Blowout, The (E) [T+Fre_YF06].nes */
	{"2b7ee482ea37a546d4f8e8239e5d75f35cd613f8", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Burai Fighter (E) [!].nes */
	{"8b0ea668331287cbb1fe9e9b5dab912e95d45009", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain America and The Avengers (E).nes */
	{"8c1f65597bfccafa78ef853aa96d12d34e04d908", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain Planet and The Planeteers (E) [!].nes */
	{"189cafa68431c1de88bdfbcf7099ce223d6a3197", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Days of Thunder (E) [!].nes */
	{"2343358138d3d436634b7fd8e0f25bb3c9702300", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Double Dragon III - The Sacred Stones (E) [!].nes */
	{"e8fcd54dde8468c39f52b57926cf5224da787fad", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Double Dragon II - The Revenge (E) [!].nes */
	{"90c5d89bebc7fb7d783ddfdf0bd393b8cd9c47b3", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon's Lair (E) [!].nes */
	{"5ab4267ef02251a84875d6a21b306b0eb720d74c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 Strike Eagle (E) [!].nes */
	{"4d8e95cd00455bd664eee975a96bae9646f37e4c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Felix the Cat (E) [!].nes */
	{"b77d5956303de39773017c99ed5cee441b8494f2", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Flintstones, The - The Rescue of Dino & Hoppy (E) [!].nes */
	{"8d0b0a576b956c2e659b4afb246488cec7dce397", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Flintstones, The - The Surprise at Dinosaur Peak! (E).nes */
	{"209b5311eb27fa5fd7409c39aefaea0d2fa9063c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Formula 1 Sensation (E) [!].nes */
	{"454b27dcba3c8f1478a04c46cafacac8e10e8350", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Galaxy 5000 (E) [!].nes */
	{"f90cb60e2870c721d99d521f2d5264517c6ec7fe", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gargoyle's Quest II - The Demon Darkness (E) [!].nes */
	{"a2dc4d379834dd6382932ad4100e9051e7065221", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gauntlet II (E) [!].nes */
	{"155ce05c2927510486d6f163034218806ac3e212", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* George Foreman's KO Boxing (E) [!].nes */
	{"347a316477ec29dd14db9073f8a0c8ef4e78189d", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goal! 2 (E) [b1].nes */
	{"ba346b08cbed4071e3ae51744cbf2799ee012caa", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gold Medal Challenge '92 (E) [!].nes */
	{"84f8bef43d95446f0e57f9cade0744d5ab28b440", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gremlins 2 - The New Batch (E) [!].nes */
	{"0fbc4f055df6fce1013aa075694c14857ffdb56b", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hammerin' Harry (E) [!].nes */
	{"a0c4e60bcbafd4e04caf727bf5ffbee0f09402bf", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Home Alone 2 - Lost in New York (E) [!].nes */
	{"88226bed22293ffefd486b3fe236b1a828d8f683", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hudson's Adventure Island II (E) [!].nes */
	{"ec982a07c3f342b85ddb924000766c366e67217a", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hunt for Red October, The (E) [!].nes */
	{"0765b8029241f6542ef3fff530dd5cf8d30d4aa8", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Incredible Crash Dummies, The (E) [!].nes */
	{"a13f8913dcab428d217563d357895b47c84f8d6b", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Isolated Warrior (E) [!].nes */
	{"9f5ad2bd13d6b3ba696662728f172f0b8b683f0e", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jackie Chan's Action Kung Fu (E) [!].nes */
	{"bb05a66fa3c6972327026f356868ac8c98bad9fc", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* James Bond Jr (E) [!p].nes */
	{"dde643f669851a25f1207a71a0f11b3eb5b6a934", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jetsons, The - Cogswell's Caper! (E) [!].nes */
	{"30f10636e35196f0a738be517316c3c21aff1fa9", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Joe & Mac - Caveman Ninja (E) [!].nes */
	{"5cda93dcd35d2fa7794591df1325d50526a45e93", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jungle Book, The (E) [!].nes */
	{"4271317152b017a5e8e1cad87ef25041f4d3fcce", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jurassic Park (E) [!].nes */
	{"0caf586abe0fe54ec75da085afb14bba40fdcd15", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kabuki - Quantum Fighter (E) [!].nes */
	{"a313fe28e699eb025ad015f29242b0550a267186", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kickle Cubicle (E) [!].nes */
	{"a0aae437f9b3de8734e62287591a65d094246734", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kickle Cubicle (E) [T+Fre99.5%_YF06].nes */
	{"7e6ff4be31b17cba10d699cec271548e16a327e9", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kirby's Adventure (E) [!].nes */
	{"80c419d49cc13b60d1d52e4dd043edc0e4d93a0c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Krusty's Fun House (E).nes */
	{"906a3b1b99d05274660a77a88713128cd6a60732", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Little Nemo - The Dream Master (E) [!].nes */
	{"d5ae46caecc55bf9cc8fb6bdc3078a4b63818052", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Little Ninja Brothers (E) [!].nes */
	{"883924d4be49d11c0d6d4cf00956c0aa13409aa5", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Little Samson (E) [!].nes */
	{"66efcf18a04fc1eebbb6d231add852ddb4b3e5f9", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Low G Man - The Low Gravity Man (E) [f1].nes */
	{"d433cbf86dd0467dbc2baa466e8d8579336d9553", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Low G Man - The Low Gravity Man (E) [!].nes */
	{"1e84c71f6c10f98f2c9d0dcafee0b84cce7a0355", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario is Missing! (E) [!].nes */
	{"64bfe58f37ad4e5e4ad311e057337b3e38778032", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* McDonaldLand (E) [!].nes */
	{"0a71078fbbda868c8d081cc4f26132bbbdcef3ff", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman III (E) (PRG0) [!].nes */
	{"4ee17b72ea85664cedc8b7b52b02db355b0dfb5e", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman III (E) (PRG1) [!].nes */
	{"136ed00687a64019a6e0e3fc1c4a7ae8e63bf74c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman IV (E) [!].nes */
	{"8f6e3d7145e7856eaec1bc1f195df30631425c0e", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Megaman V (E) [!].nes */
	{"cce4b2af8f19c520dd4e55c26497324886535787", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mighty Final Fight (E) [!].nes */
	{"5c61d535f7df0f668dc552e7164eb5075dab1794", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mission - Impossible (E) [!].nes */
	{"c16b872eedd834021ca3f41cb3be7d0e4779ecaf", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Monster In My Pocket (E).nes */
	{"5c80b6c731efb783e34df63c29fcf902f483206b", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* New Zealand Story, The (E) [!].nes */
	{"db216c83a327c132904e818a12f5c8de299f8764", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nintendo World Cup (E) [a1].nes */
	{"68b7d5fae076d713785fa307fa9d3e1a30eb7c6d", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nintendo World Cup (E) [!].nes */
	{"14968edafa080a967e7415384a4b83aca7465c1f", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Noah's Ark (E) [b2].nes */
	{"63ac90ac06240842eb0f659089d65976e4b7e426", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Noah's Ark (E) [!].nes */
	{"f0a801ec059e217d8bd1b9b97ce8434c3136829d", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* North & South (E) [!].nes */
	{"1fbd350a89d3501d01721f574bc21b2f845d68fc", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* North & South (E) [T+FreFinal_Generation IX].nes */
	{"d250fc66789273f536bd6687f0d707e794f8b1ac", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Over Horizon (E) [!].nes */
	{"92525ae69bc2c9725ddc547765877ec4eeeb2ff3", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Panic Restaurant (E) [!].nes */
	{"a0086d32d102df82c52431e262d5458037fac1ce", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius (E) [!].nes */
	{"959bef0d1bcc71a8746b65ff19ea3a0976b13d97", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius (E) [t1].nes */
	{"8d3492d45b11efb5a5db41c2558a1d1ba4a7c3a2", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius (E) [T+Chi].nes */
	{"909286e144fd8c7597d79d4e662f248d35b0bb8d", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Power Blade (E) [!].nes */
	{"40d7dc8191e7b9af497e57399741c22107d932b0", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Probotector II - Return of the Evil Forces (E) [b1].nes */
	{"31ce6953bd2d5731f8a41a60b61e624fd1aa02bb", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Probotector II - Return of the Evil Forces (E) [!].nes */
	{"1ae9e233057adcfcaee1417237069554c2aa0faf", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rackets & Rivals (E) [!].nes */
	{"d109eeb2a6847737103a628afabc5d0f75a7d43a", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rampart (E).nes */
	{"8015776a5e316721913d2db996325ffbd5a67370", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* RoboCop (E) [!].nes */
	{"db1420383f7d0797aa2a46a20113fef06a055bdc", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rockin' Kats (E) [!].nes */
	{"9682a984040775db8f96428a74ae7dc124f993a7", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rollergames (E) [!].nes */
	{"e9a8572de750f1276ad240539c139cafbb2abfee", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Roundball - 2-on-2 Challenge (E) [!].nes */
	{"96abf4b13c752df756c5905f6ae1e1f63d6c90ad", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shadowgate (E) [!].nes */
	{"bc70c6b2a6ff59bd1cec81b8df3160b7109d8924", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shadow Warriors Episode II - The Dark Sword of Chaos (E) [!].nes */
	{"4dea3395f734522b2c4f2ae48359274134653125", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shatterhand (E) [!].nes */
	{"898d2350595edae3a4f3c94034a9780579f4f439", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Simpsons, The - Bartman Meets Radioactive Man (E) [!].nes */
	{"632d717e745589ae943939cc5d36e257ac08683e", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Simpsons, The - Bart Vs. the World (E) [!].nes */
	{"06e5df0d342ab0bd39b899a10af054aa3981fce0", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Smash T.V. (E) [!].nes */
	{"e28b68e7a8219ee1a4eb3c72c0d258c5e71a6914", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Solomon's Key 2 (E) [!].nes */
	{"05a4b50d7e9cfcbf55094046c6667abc5a7b35f8", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Spider-Man - Return of the Sinister Six (E) [!].nes */
	{"291f5dd81332075355637a00c47947f0c5359e3d", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Star Wars (E) [!].nes */
	{"ef79978c46b87d86e9798a9eb6bb76a3cad545af", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Star Wars (E) [T+Por100%_Central de Traducoes].nes */
	{"099b2e7922a6213196b7692771e01f013db1f149", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Star Wars - The Empire Strikes Back (E) [!].nes */
	{"738524397555dd8187b5e1f6e6f2b475c28a8291", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Street Gangs (E) [b1].nes */
	{"b17bddec2522548d969753d685c86358c6ef9871", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Street Gangs (E) [!].nes */
	{"11be5a728b6d9857c26ce62d08c3fc92c687b7d4", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Street Gangs (E) [T+Fre1.00GoldEdition_Maty].nes */
	{"5db89f1ba92213d44a7dfa317d8723a90f2dcbc7", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sunman (E) (Prototype).nes */
	{"0a48b10ee8ec3a6acbf8a94170259f9298c75296", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 2 (E) [!].nes */
	{"f38f8823112e1a33794fe638cdded7efbb98c67a", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 2 (E) [p1].nes */
	{"a26dce752ccdbfc397ea2338be839e97e317ce37", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 3 (E) [h1].nes */
	{"d4971c116270fbeeab8aef1efe7c00cb3372ae34", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 3 (E) [h2].nes */
	{"1678f47252306ceb7a4d3a5fc5578ac8f7b5f625", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 3 (E) [!].nes */
	{"8d389a0893beedf273e21ef865c9a8288a929184", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 3 (E) [T+Dut1.0_Ok_Impala!].nes */
	{"0f3816e1f8f97c71a64f0db796b32a616e6d4d41", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 3 (E) [T+Rus].nes */
	{"643bac7f0efff54cebaae2fe7329adf741f6e46c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 3 (E) [T+Ser1.0_MadFox].nes */
	{"606176d4b00f6f345a6f88f69ab1bfbfa87a0f6c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 3 (E) [T+Spa100].nes */
	{"ab2cb088923a52cd9660bc99b5b1fd85d7ffceca", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Spike V'Ball (E) [!].nes */
	{"bdb34f9f3f9eed78aabccb6659e5e36b2f84ddeb", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Spy Hunter (E) [!].nes */
	{"ef34d59db855fe77f8197346d784bfc5e10e018c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Turrican (E) [!].nes */
	{"20dd8bcce6ae04d35f96421adcce310e2bf14c29", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sword Master (E) [!].nes */
	{"7e9f47c52dc23e957d89501036b99c155247843d", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tecmo World Cup Soccer (E) [!].nes */
	{"0028d6a715cb45b1babb414c14e587dee9decea2", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Hero Turtles II - The Arcade Game (E) [!].nes */
	{"3dfc04ca0d6c9ecb3806a3cf532f6fae18a2fd75", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Hero Turtles - Tournament Fighters (E) [!].nes */
	{"7f819bd3a4b9dc2d97b4929ed62e69a56b31c81c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Terminator 2 - Judgment Day (E) [!].nes */
	{"f0cc018ad97ed84f28aba003bc6124e32be2165c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tetris 2 (E).nes */
	{"30375817dd08d97517aa88035973f72d4e8a27d9", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures 2 - Trouble in Wackyland (E) [!].nes */
	{"49e257b0e1c567f366ebf6d26c9c2072d407a0e0", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures Cartoon Workshop (E).nes */
	{"95cee6809817aaf490f27d435e074c22773cc7e6", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures (E) [!].nes */
	{"42d3c102d7ce9b90f03237f0a56f028d55831c4b", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tom & Jerry (E) [!].nes */
	{"16e0a1cdd7142c619e46853ffb95d5d2f4bf8f1b", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Top Gun - The Second Mission (E) [!].nes */
	{"92383596577d011da687e3619cb6b7d4785aa731", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Totally Rad (E) [!].nes */
	{"442d9421e7c658422b7f1261c18bdd2fff9e4152", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* To The Earth (E) [!].nes */
	{"ed558caaa857b5ea81f8d6c38a9c4c0d7dfba43a", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Trolls in Crazyland, The (E).nes */
	{"5edab1bee1f1c05058ad8408427d2ac8aeeb982e", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* U-four-ia - The Saga (E) [!].nes */
	{"4a36136ddbfedcac485b4ac964e3b5127fefc882", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* U-four-ia - The Saga (E) (Prototype) [!].nes */
	{"ba8671963441daa706c14b1cd7fe71f5d7565f97", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* U-four-ia - The Saga (E) [T+Fin0.99a_Tuoppi].nes */
	{"6e35fe9e2365892d28a400051ca94315e1da2d14", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* U-four-ia - The Saga (E) [T+Ita1.0_Clomax_Dominion].nes */
	{"74641f35b085bec341d920ffe0d396a6f36d9464", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* U-four-ia - The Saga (E) [T+Rus_CaH4e3_&_Chronix].nes */
	{"992d6a9c02b2f0e084b737318e37ea29b0103cf3", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* U-four-ia - The Saga (E) [T+Spa].nes */
	{"e86ed1302bf899145f229fe26897f5a9b793f067", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* U-four-ia - The Saga (E) [T+Swe1.0_TheTranslator].nes */
	{"80b71aa726d18a3ed339740c34405896f3697e6c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultimate Air Combat (E) (Beta) (M3).nes */
	{"985fa2bf3e694c84af44dd769874df69cbb8de1d", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultimate Air Combat (E) (M3)[!].nes */
	{"260f7542993a70c4886d507528b10ca1559822ff", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wario's Woods (E) [!].nes */
	{"b868e2ae26643ecafd689da2500b6562b3bbb37c", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Werewolf - The Last Warrior (E) [!].nes */
	{"73ba99ef6b8d815765a2805a4be061def42a04b4", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wily & Light no Rockboard - That's Paradise (J) [T+Eng(E)_Interordi].nes */
	{"426f72d8c905210037ae03e7289de38ed456a2c0", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* World Champ (E) [!].nes */
	{"d0b67ffc0849c18013253965ba497b6b76d8ccfd", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* WWF King of the Ring (E).nes */
	{"eaf5a20b6f6d2c84a889bbf2ff857dd5ece35bb0", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* WWF WrestleMania - Steel Cage Challenge (E).nes */
	{"7c8ede2e39d39f03e27d1888cb015b8e186d172e", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Yoshi's Cookie (E) [!].nes */
	{"ee5021a650d2d95111a606f9aae6cb5bd9232799", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zen - Intergalactic Ninja (E).nes */
	{"9ee8cf897139d8e43a5572e53215821cbd3660b9", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Banana Prince (G) [!].nes */
	{"0b852be84c8e5517fa82a403404db3671a58d998", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Banana Prince (G) [T-Eng1.0_KingMike].nes */
	{"c75f349069f086d883d98bd0dcec77fa417f1bc4", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Banana Prince (G) [T+Eng1.1a_KingMike].nes */
	{"5dcea42e904f7247504be4e55540e6cd8a8ed355", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Banana Prince (G) [T-Eng1.1_KingMike].nes */
	{"9a4ce3735d06c88f5d1c65e2c8aab4d8ba9f5234", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kirby's Adventure (G) [!].nes */
	{"38e2f2c7d2f935766243bc0b97c944c806b77e72", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shadowgate (G) [!].nes */
	{"552f57c507da43915f1e11ff019972b1d88030dd", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Star Trek - 25th Anniversary (G).nes */
	{"313cf399b61b9c8e64398978ab57cb394a80dd70", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 Strike Eagle (F) (M3) [!].nes */
	{"8df1d70041b6fe46d6cd5293e8385303568fa3e4", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kirby's Adventure (F) [!].nes */
	{"a0375ccd66a90441d89804687a86e350c1be9ce2", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* McDonaldLand (F) [!].nes */
	{"1ad8b7dcba91331f1f1352a54258fb5af9cf9cd7", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mission - Impossible (F) [!].nes */
	{"3e035931a543c0ed96a6a42c435b012bde9b15c3", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shadowgate (F) [!].nes */
	{"a41c30914b28822033157b29742c86ec080a6831", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 Strike Eagle (I) [!].nes */
	{"ee7e9ce18f7b5742424b21bcbe819d4f918a40e1", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deja Vu (Sw).nes */
	{"9ab2ddaa2181217d1fb6b05c5407db008c78891e", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 Strike Eagle (Sw) (M3).nes */
	{"bc04da4fdc19de7983b87b42cd54daefa17feb08", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shadowgate (Sw) [o1].nes */
	{"0e429a553a32aebbd9d88377014f2956dda7b1a1", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* high-hopes.nes */
	{"c3c7ef2dbc3f4e0fa56fe461a6088541ef1b74d8", 4, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* --------------------------------------- MMC5 ------------------------------------------- */
	/* Gemfire (U) [!].nes */
	{"03f0a3a41071fc445b3aff63782a1c36c8a7dcba", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Just Breed (J) [b1].nes */
	{"5672e2aed40ca87bb1551f2aa3f19617a853a2db", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Just Breed (J) [b2].nes */
	{"040e177ca6fea1eef5a41e719abc119579bf5521", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Just Breed (J) [!].nes */
	{"62e05f499de22848707e8a93104f3719e748c0ac", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Just Breed (J) [T+Eng0.01Beta_Obsolete].nes */
	{"c93786e405fd6c39a4d173ea74b54538c63eafe4", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Just Breed (J) [T+Eng1.00_Stealth].nes */
	{"a7c24fb1190830adc72c928aed8c6c8b497434a4", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Just Breed (J) [T+Spa100_butz].nes */
	{"94568688ab994c339abe334fc1655f1fcc789432", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Just Breed (J) [T+Spa2.0].nes */
	{"224ebc860b6e1065b383aeed4a0dfbb5b6189375", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Royal Blood (J) [o1].nes */
	{"b67183da893639745d72a4d99f4d0c0f60fdc136", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shin 4 Nin Uchi Mahjong - Yakuman Tengoku (J) [!].nes */
	{"7441a13386f8bfc34b54b07b37f3f312dc903040", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (KC).nes */
	{"97ead1a57f822196d809ffc72fd1a1c571fcfd5b", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [b1][o1].nes */
	{"00e9e0ac6a3b5ba4956ee25f83756dd3ee10ae5a", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [b2].nes */
	{"c4f45c05fd732618cbcded996df9073d97745184", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [b2][o1].nes */
	{"d70a31031cd461bb3a83ec26c7b9594042acf2a5", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [b3][o1].nes */
	{"95579a9cc79809bf382a2ddb3754ce37adc3f060", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [b4].nes */
	{"dcf6609febb5fe7b1c28e6214e4696413dbf4770", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [b6].nes */
	{"664717657979fed2d91fbd27c2911f57561a3ff4", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [o7].nes */
	{"91a93f59a92955af2448a2f519ee254ce49c679d", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [T+Bra100%_BRGames].nes */
	{"faebd533a67ee0e9cce67f66993a6461905706ae", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [T+Por100%_Odin Games].nes */
	{"60b9d9e878674c368676aaca814d0948d285db81", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [T+Rus].nes */
	{"3f7b2b21fe7f3b6701ed9d361a1d2cb4e5f23f70", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [T-Spa0.82_Toruzz].nes */
	{"7554960e954a9abdaef5754891362a7b25e58897", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [T+Spa1.0_eduardo_a2j][o1].nes */
	{"b6c922ac17316c30db7c1b414bed8cca434df9b9", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [T+Spa1.0_Toruzz].nes */
	{"4c487310a200f43e6d85923f847599e54dd8158d", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (U) [T+Spa_PaladinKnights].nes */
	{"9c7d65a9f72a6cb588acd41416c6db2107ea2700", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (E) [!].nes */
	{"b573cf562201b83b85515a64f668604a48514792", 5, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (E) [T+Spa080_toruzz].nes */
	{"755e1e8ad693ed3ce5e48b227af0b9c448f17de7", 5, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Castlevania III - Dracula's Curse (E) [T-Spa].nes */
	{"dc39f024d72747675afc09e651772e69d42c4584", 5, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bandit Kings of Ancient China (U) [b1].nes */
	{"177989840cd22ab85afbde9667d72baf1cba08ac", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bandit Kings of Ancient China (U) [b3].nes */
	{"6b89389b78deb4e4ece7e307a06ced95815cd29b", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bandit Kings of Ancient China (U) [o1].nes */
	{"c0d05838234c50f18b94836f5ac89faba11a07c4", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* L'Empereur (J).nes */
	{"8d21b6d59c15c61b00038e73e281b42b566ddcf5", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* L'Empereur (U) [b1].nes */
	{"bc297ede993f46c470380ad600cb25829e2fe705", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* L'Empereur (U) [o2].nes */
	{"65a73047a3606433329dd50a52b82e0a75d2cff6", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga's Ambition 2 (U) [b1].nes */
	{"4ffffb986905922f3974349d8b5a27feb929711f", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga's Ambition 2 (U) [o1].nes */
	{"0be2df0b062e908cc0492ebecedb481e94940abe", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Uncharted Waters (U) [b1].nes */
	{"8e5ce000807572d9e6b40b38f4ac6d2a380ede0f", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Uncharted Waters (U) [o1].nes */
	{"a4b22fe17dc873e09ef54959d7438c8548859fd0", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Daikoukai Jidai (J) [b1].nes */
	{"7cd9272d290765a5cedb4ef2951f8fe8f98cc442", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Daikoukai Jidai (J).nes */
	{"058493b592fcaa7eb6e1615d2e55f32946ba0c61", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ishin no Arashi (J).nes */
	{"a8b46771c1d848da3ca7cc203901ad001598489b", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga no Yabou - Sengoku Gunyuu Den (J) [o1].nes */
	{"e95a668ee2d39720c7a7b216c2fda2068353a4d8", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Suikoden - Tenmei no Chikai (J) [!].nes */
	{"a233e79e6c3919e0f6efb61325df8212013732e3", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Aoki Ookami to Shiroki Mejika - Genchou Hishi (J).nes */
	{"254f44d573b8cf59767cf4cb67cb4c8e7df08a53", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms II (U) [b2].nes */
	{"08994dffedf2722f4eea1e172408cc946e960fce", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms II (U) [b4].nes */
	{"289351c258d1d990db1595f2c817caec3cf98190", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms II (U) [b5].nes */
	{"fe70a4e4b36fcc91ed6004a909185cb5aefd6b7b", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms II (U) [b6].nes */
	{"a6960ebe8a3ac41443624811b3a8a5b4f11c4184", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms II (U) [b7].nes */
	{"e23bd8f9d1eebc39d801a85cdf258066bd95332e", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms II (U) [b8].nes */
	{"64e7cd0c408519ffc743868e5417d111e0eb0797", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Romance of The Three Kingdoms II (U) [o1].nes */
	{"3b584c70948473f993a17d95479c23cb8ba12df1", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nobunaga no Yabou - Bushou Fuuun Roku (J).nes */
	{"52436255fce4e7d0e2314e61343fae776b6e423f", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sangokushi II (J) [!].nes */
	{"124339f341fe40324444da1d57bd094fd0b03db8", 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ AxROM --------------------------------------------- */
	/* Arch Rivals - A BasketBrawl! (E) [!].nes */
	{"e5a1074a938001d01941b2bdd56f442090a267d1", 7, 2, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Arch Rivals - A Basketbrawl! (U) [o1].nes */
	{"989c15904eb8f9f6e77ae59300a76ffa935abf43", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Beetlejuice (U) [b1].nes */
	{"d9d0a6631b07849cd80413daf0d991da5708d043", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Beetlejuice (U) [o1].nes */
	{"e62a1eeb5b88c4b812858c741907f6fe51765e56", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Beetlejuice (U) [T+Rus_magic team].nes */
	{"789a79713d755d8c89048534adfd119fe097df81", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cabal (U) [b1].nes */
	{"b7a0f2dff8500e00286ae28be4fe87cf76655ead", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cabal (U) [b2].nes */
	{"af893a46201e3bf5c161b826b8688b62d6a57a98", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cabal (U) [b3].nes */
	{"ba33f2595f28bb96c592c3fa455a82a44e136b56", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cabal (U) [o1].nes */
	{"6fc96949c5283911969f6f873b8ed52752aadaba", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cabal (U) [t1].nes */
	{"3c215a7cacc06db2493f06b8ae093a6fbdeb0c69", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cabal (U) [t2].nes */
	{"fc94f3448d87b3220e037587d9f61bb73e0a5d29", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain SkyHawk (E) [!].nes */
	{"5d940b2224946def3f9f9fad13d879170f3bf494", 7, 2, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain Skyhawk (PC10) [!].nes */
	{"e12086a3f1a3b5e9ec035cb778505e43f501416a", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain Skyhawk (U) (PRG0) [o1].nes */
	{"92f0c3153d0dec1f78d61425f88fc6573b0f9be8", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain Skyhawk (U) (PRG1) [!].nes */
	{"355649d34aeffae0dd15c03a51b16ef49b668852", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Digger - The Legend of the Lost City (U) [b1].nes */
	{"629b671d17b4dfc12d1fb640ede67b8e29c86792", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Digger - The Legend of the Lost City (U) [b2].nes */
	{"306ea1f2a18abfb2e994e9884491b5e1998c0940", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Digger - The Legend of the Lost City (U) [o1].nes */
	{"8e699f08869d84cb8f310335e6d6bec08c1ece60", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Digger T. Rock - The Legend of the Lost City (E) [!].nes */
	{"b97dbe5e62f7a30d7be639f506d48553853f1c7b", 7, 2, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Danny Sullivan's Indy Heat (U) [o1].nes */
	{"238a5a46db79ac6e5252dc902f5bbda669fa4dd7", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Marble Madness (E) [!].nes */
	{"81721d79f99016672651ce58c2481d58b97c4522", 7, 2, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NARC (U) [b1].nes */
	{"2f48e5c9e32734e7ee0962b2ca4b092961422542", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NARC (U) [b2].nes */
	{"84776ca89a821bb9ced24e554b998a9fe1e528e2", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NARC (U) [o1].nes */
	{"40f7287b34a8b9a5729685a58c65df3078664db8", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nightmare on Elm Street, A (U) [o1].nes */
	{"e3704078fe0460a65fdb57a9406a5dad4155c215", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nightmare on Elm Street, A (U) [T+Bra_BRGames].nes */
	{"98c292362e471437d6679b737037c21a2869a08a", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nightmare on Elm Street, A (U) [T+Spa1.00_ShinjiE].nes */
	{"b79f81239f5c4756b41dc523917735f7f7b12e36", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ivan Ironman Stewart's Super Off-Road (E) [!].nes */
	{"e83443c990d36dd3c851a317fafc87f2abf4a944", 7, 2, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ivan Ironman Stewart's Super Off-Road (U) [b1].nes */
	{"97523d87b577c05d01caa3a27a79e432dda2078d", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ivan Ironman Stewart's Super Off-Road (U) [b2].nes */
	{"291af0095d29a3af8a59e609934f72a4a42f857d", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ivan Ironman Stewart's Super Off-Road (U) [o1].nes */
	{"61df677041a4899a8cd6eeeb1f789a2c70adb482", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Time Lord (E).nes */
	{"81de1de17faf55735d1e13351bab5e54bf715422", 7, 2, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Time Lord (U) [o1].nes */
	{"a2d029e51a8e3d92ba17cb2e0a7d96fbc91d96cf", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Solstice (J).nes */
	{"758c9ec51d442db2c02c4fb37a9aaa37cc8f6b05", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Densetsu no Kishi - Elrond (J) [!].nes */
	{"0837f9fa65478ce511c96392b5249d21abaa632d", 7, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* WWF Wrestlemania (E) [!].nes */
	{"105ed9260ce18f3fd46a36dab99f05b24885e717", 7, 1, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Aladdin (E) [!].nes */
	{"f1ffbecbc99558eef25e8c6db1eeb9c408e2f33e", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Battletoads & Double Dragon - The Ultimate Team (E) [!p].nes */
	{"6a6579963319cc14da457c963fa43741bfbf645c", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Battletoads (E) [!].nes */
	{"e13626a09ad745bc5d9fb0972383dbf358deaddc", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cobra Triangle (E) [!].nes */
	{"d3c4b6ba78d0405ad95d5d5c17c10f0d4db27c6a", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ironsword - Wizards & Warriors II (E) [!].nes */
	{"a12c57bca1dbf24ecbb08fc7c6f510c062fca75a", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lion King, The (E) [!].nes */
	{"c550af552779c712f94bfc776899fd76a64b3ca5", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* R.C. Pro-Am (E) (PRG0) [!].nes */
	{"5328f9961dd96f117928eb45c797c062a7d37d07", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* R.C. Pro-Am II (E) [!].nes */
	{"3b26dcd54cf922a6aaa6265087b56def4c109a7e", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Solar Jetman - Hunt for the Golden Warpship (E) [!].nes */
	{"9fe6fed4e8dec3df34c69c6d4b2e528a288246d2", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Solstice (E) [!].nes */
	{"761ba83f7e90c11f8fbe35aec510d33d8bb6f024", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Solstice (E) [T+Fre100%_Terminus].nes */
	{"35281f565457bbc94325aca4ed16d556ae531eed", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wizards & Warriors (E) [!].nes */
	{"d3f63259229b951dd55e59ce65d3415e3d993577", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wizards & Warriors III - Kuros - Visions of Power (E) [!].nes */
	{"7c856725cb324e2cc3994773fb9a2462dc91dc48", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Balda (BBG) (R) [!].nes */
	{"dd5e8d4913aa52595015ecde297ea8ee4f5752a5", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cvetnye Linii (R) [!].nes */
	{"deb888164e724d87e6386391cdd97892c6148e73", 7, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* BB Car (Unl) [!].nes */
	{"04302d2a6d2b76395de595a6793a599111f78930", 7, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------- MMC2 e MMC4 ----------------------------------------- */
	/* Mike Tyson's Punch-Out!! (E) (PRG0) [!].nes */
	{"389c55695caf74a2fc817838288eaf4ed036ba1c", 9, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mike Tyson's Punch-Out!! (E) (PRG1) [!].nes */
	{"6f5ba7e9814853bfc4c28eb8be560180010aecfd", 9, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Punch-Out!! (E) [!].nes */
	{"a7bb7d58766aa67004c1170f872b1b944c8bf959", 9, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Famicom Wars (J) [!].nes */
	{"17a1fe80304159d529f2b96f035192d6063a8702", 10, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Color Dreams -------------------------------------- */
	/* King Neptune's Adventure (Color Dreams) [!].nes */
	{"012a23b6fe872bcbcc42dfeb5835a7bfaf721fc6", 11, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Kojiki_by_wAMMA.nes */
	{"9becee07199b622408c0806d1dd1f510989b2488", 11, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Free Fall (U) (Prototype) [!].nes */
	{"41f6f581c7da9f90c18e0eb2442df7a6adbe3e76", 11, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Secret Scout in the Temple of Demise (USA) (Beta) (Unl).nes */
	{"e667fe1a0eb044c8615042d31dea3b2913d813bb", 11, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* --------------------------------------- CPROM ------------------------------------------ */
	/* Volshebnaya Palitra (R) [!].nes */
	{"e4414c160c7e91136c62d99154336035e5636eeb", 13, DEFAULT, NTSC, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* --------------------------------------- SL1632 ----------------------------------------- */
	/* Samurai Spirits (Rex Soft).nes */
	{"648dc9e5f3b4228aeb9d424f0ae4047cd946a736", 14, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 15 ---------------------------------------- */
	/* 168-in-1 [p1][!].nes */
	{"123105a996b3f325e55b7080ddd83d4cf043e543", 15, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Bandai FCGX --------------------------------------- */
	/* Akuma-kun - Makai no Wana (J) [!].nes */
	{"6f3bbce3d234a68747bcccfd3909c5edcf12c875", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crayon Shin-Chan - Ora to Poi Poi (J).nes */
	{"7067e67d3f67ca28a197d8d30655fc6ba81967b8", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crayon Shin-Chan - Ora to Poi Poi (J) [T+Eng0.1_Alcy].nes */
	{"5fe8b11a7a9f7e24bbdb8cbc114e7defe1717003", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball 3 - Gokuu Den (J) [a1].nes */
	{"c53b57f311f2d0b76773afb45085c7e7d9cb156d", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball 3 - Gokuu Den (J) [a1][T+Eng0.15_Dodgy].nes */
	{"5b9ecacab11044e223ac7c5ab51fa1378d114576", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball 3 - Gokuu Den (J) [!].nes */
	{"c89072f4037e8d87b3e48abbf42ee7c15612e68f", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball 3 - Gokuu Den (J) [T+Kor_88ktt].nes */
	{"9f41b40c502c4ce072fe9e2a7e15e61738081118", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Dai Maou Fukkatsu (J) [b3].nes */
	{"0091decdd5b804824b4321d20be260343b6748a3", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Dai Maou Fukkatsu (J) [o1].nes */
	{"83dd37327f867db2cdecd5dd4bb894280967a523", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Dai Maou Fukkatsu (J) [T+Kor_88ktt].nes */
	{"d3a8c329b295954ae3e16696ce15fb13f3f355a1", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z Gaiden - Saiya Jin Zetsumetsu Keikaku (J) [o1].nes */
	{"b7582841bcbf50de7a93b525430f1317e07ab7e4", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z Gaiden - Saiya Jin Zetsumetsu Keikaku (J) [T+Eng0.05_Saiya].nes */
	{"356507b94a1419bc5674a98dde8bc5c69eb934e7", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z Gaiden - Saiya Jin Zetsumetsu Keikaku (J) [T+Kor_88ktt].nes */
	{"687a7a0ef7382e839671cf03339c08d7e56b809e", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z II - Gekishin Freeza!! (J) [o1].nes */
	{"fd2b56ec610aa29e25c95dcc93e0ec4fe6f0f833", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z II - Gekishin Freeza!! (J) [t1].nes */
	{"63cf572a6af6ed67d14e0cf99205d52d858985ca", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z II - Gekishin Freeza!! (J) [T+Eng0.35_Omniverse].nes */
	{"47d32bf7d3252a12b32d1f5c4e6018c373331eb0", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z II - Gekishin Freeza!! (J) [T+Eng0.70_Saiya].nes */
	{"f6c8c0b327ee42240ee600d8c4511c0bde00f5c0", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z II - Gekishin Freeza!! (J) [T+Kor_88ktt].nes */
	{"41e76bf48e05f448b43c5e51087b4fd4f886f556", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z II - Gekishin Freeza!! (J) [T+Spa70%_PaladinKnights].nes */
	{"76638198e16c4883c1f3bc48d9ef50214f95199a", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z III - Ressen Jinzou Ningen (J) [o1].nes */
	{"fd3040d78fab07059a7930c90a9f8483d7228f53", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z III - Ressen Jinzou Ningen (J) [T+Eng_Twilight].nes */
	{"ead1ea48f9390b1411dc5c150e352ca8a165b533", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z III - Ressen Jinzou Ningen (J) [T+Ger10%_Sven Rech].nes */
	{"3e41e7244176577e37b18949fd790623ace9d579", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z III - Ressen Jinzou Ningen (J) [T+Spa70%_PaladinKnights].nes */
	{"cf0ae54dacc2a70bf627e32d1f48994685ca68b5", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famicom Jump - Eiyuu Retsuden (J) [f1].nes */
	{"fa9e8062a38a115bd9b2e9fe8fda2badf4177762", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famicom Jump - Eiyuu Retsuden (J) [!].nes */
	{"e0bf3933c4f0ba1c0b418b170919f6c906eb53e1", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Meimon! Dai San Yakyuu Bu (J).nes */
	{"228d3283c86413fd0c01175025cdf4e48404402c", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Nishimura Kyoutarou Mystery - Blue Train Satsujin Jiken (J) [o1].nes */
	{"2fbbf4e75b2416bb3dfc8487a33384aa5ecf4a1a", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rokudenashi Blues (J) [o1].nes */
	{"93227e22de2c07d413425e7d1e6f108cb9b2118a", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sakigake!! Otoko Juku - Shippuu Ichi Gou Sei (J).nes */
	{"0e686bde9038e6626f31f27e0a75879aa99ad9d6", 16, 4, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* SD Gundam Gaiden - Knight Gundam Monogatari 2 - Hikari no Kishi (J) [o1].nes */
	{"506229a48e17b29b51ed6ffae18cefbc28d0cc53", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* SD Gundam Gaiden - Knight Gundam Monogatari 3 - Densetsu no Kishi Dan (J) [o1].nes */
	{"f8d7481aa8599757d78f252d7dce9dc65e920693", 16, 5, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------- Jaleco (SS8606) --------------------------------------- */
	/* Jajamaru Gekimaden - Maboroshi no Kinmajou (J) [o1].nes */
	{"f631a3cf417baa42c91b268f6f74db00299439ae", 18, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jajamaru Gekimaden - Maboroshi no Kinmajou (J) [T+Eng0.1_Ethereal_Productions].nes */
	{"fa72875f499d35ba01e2c7b9cf92851a53dde29f", 18, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mezase Top Pro - Green ni Kakeru Yume (J).nes */
	{"42e473b6e6244d336524e08a396eb2d44697f3f5", 18, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Namco 163 ----------------------------------------- */
	/* Family Stadium 2006 by Clambutt (Famista '90 Hack).nes */
	{"4f255055a190b2e11c9271e5caf73e6339ce159b", 19, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '99 (Famista '91 Hack).nes */
	{"fc5dc237e9665a3eab003e4b793c71500d78e6e9", 19, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '92 (J) (PAL V1.1 Hack).nes */
	{"784e481a9bad75bb8bcf9af1d666ee39d9e954b2", 19, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Splatter House - Wanpaku Graffiti (J) [t1].nes */
	{"e5085e0ae64c3315b06a23ffcf429a46a7588d46", 19, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Splatter House - Wanpaku Graffiti (J) [t2].nes */
	{"ef8c19b7f681f285d015c0b5df92bc78400cee82", 19, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Splatter House - Wanpaku Graffiti (J) [t3].nes */
	{"56059de71f1695a63b516356d4d4c672e81f4c67", 19, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Splatter House - Wanpaku Graffiti (J) [T-Eng1.0_Spinner8].nes */
	{"0f0c696b2013466e6414f8aed8bad0b0d4dcb62d", 19, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Splatter House - Wanpaku Graffiti (J) [T+Por100%_Emuroms].nes */
	{"e5c01250469149c64ae6324a631b2611a08e844c", 19, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mindseeker (J).nes */
	{"0c792cff77e2bbfca9248cddaa073b69b191c821", 19, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------- VRC4 -------------------------------------------- */
	/* Ganbare Goemon Gaiden 2 - Tenka no Zaihou (J) [b1][o1].nes */
	{"a9ff91f04a778fd5c9c0a6c1bd41e3d3939324e1", 21, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ganbare Goemon Gaiden 2 - Tenka no Zaihou (J) [b2].nes */
	{"b911fde519b8f62ebda1a4528f586ef8215ebfdd", 21, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ganbare Goemon Gaiden 2 - Tenka no Zaihou (J) [o1].nes */
	{"711929b7621a9a80743f0542c89877c8b862c32e", 21, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ganbare Goemon Gaiden 2 - Tenka no Zaihou (J) [o1].nes Traduction v101 */
	{"16c6c4fb02e9ee1eee286b21ef35d09f4eadb408", 21, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ganbare Goemon Gaiden 2 - Tenka no Zaihou (J) [b2].nes Traduction v101 */
	{"e8a7c35c793288bf9c0161c3edd49c825c8f7981", 21, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ganbare Goemon Gaiden 2 - Tenka no Zaihou (Spanish).nes */
	{"9eb76760cfffad74b8ff1157ef0059208c07946b", 21, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* 2-in-1 - Yuu Yuu + Dragonball Z [p1][!].nes */
	{"4097d5a814308e39d592466b60b64862699cc22a", 23, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Akumajou Special - Boku Dracula-kun (J) [o1].nes */
	{"f7baf2b7f4e2bff59d57f9c249c532fc52decbc2", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [p1][t1].nes */
	{"4dcff251754747ede445b6cd7cc894f11bff3d95", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [t1].nes */
	{"5582871715c4d7a5ed432a78b5c5c10cfa7c157a", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [t2].nes */
	{"c2996762453076a6f013aa2181ff58c52673f1c3", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T-Bra1.00_Made in Brasil].nes */
	{"eac17d25fc7475a3717d86166bc0ee035145d89c", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T-Bra1.01_Made in Brasil].nes */
	{"55fdaa8a896fd6fa85b1811c31a22cb481bb042a", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T+Bra1.02_Made in Brasil].nes */
	{"7ba99f29f068631ad75aa2b8f6902ecbdbe596e7", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T-Eng1.00a_Vice].nes */
	{"2eca35cbed8df9e09dadde6645146b8609f73a98", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T-Eng1.01_Vice].nes */
	{"e1e7be97bf65d9f6836d8ef02f3b93c8494cda98", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T-Eng1.02_Vice].nes */
	{"0869930354f8f736a820cc1f62f17204e62948e7", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T+Eng1.03_Vice].nes */
	{"a372bfa21c48d6d289d1a0216df5382f5efb6e29", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T-Eng1.03_Vice Translations][t1].nes */
	{"4ea21ee1eb903539cc5b1b5b847eac5b0008bb3f", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T+Eng1.04_Vice Translations].nes */
	{"c384462009cb645d87455ca0b18bfb10dbf34027", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akumajou Special - Boku Dracula-kun (J) [T+EngR1_Kalas].nes */
	{"c7209cfa6b7f13f949599f5e5b12fd92be99a926", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* mlla1i.nes and iki73k.nes*/
	{"99ab4d923ca6433c6063a700c2b8aea77f6a8314", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [b1].nes */
	{"baf62ae548f93e69af44e4458a32a0d28faa1439", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [b1][t1].nes */
	{"24239da3a2611a97d4ef9f5623a83739986cb2c9", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [!].nes */
	{"66e802ddec4ccf58f7fe214a14b71bd41da4463a", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [t1].nes */
	{"b2367999f886acd656078ed87f30965426013ec2", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [t2].nes */
	{"db46483702cfb30e8d7b55c2291c1648f3afd5af", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [t3].nes */
	{"cdfae740505c7ade85577cc82747c2de2f34115f", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [T+Chi_huanxiang].nes */
	{"bf91e6d8e307537742028292d4f0795f6b057ae4", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [T+Chi].nes */
	{"d0d130d8c8f5393eba38c4a5015cd664e92bd9d9", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [T+Fre1.0_ks151].nes */
	{"55019ae11412a53c632a81b253e5f5345e68823b", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [T+Kor_dkny94].nes */
	{"5af8a0e92e057f308a5e60505089ec893ea9aff8", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius da! (J) [!].nes */
	{"446ae9a995262a08fcee1d4039d2d6a4328fdc28", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius da! (J) [t1].nes */
	{"b6863adb507cc12bdf1cf3df6ac161884192ba14", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius da! (J) [t2].nes */
	{"cb8d5ea7121ee30ec4ae4d17f83153f483139229", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius da! (J) [t3].nes */
	{"a4fa77ed601b5ef97cd941f8a8103db087879a94", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius da! (J) [t4].nes */
	{"ea4f552696aa12c3f5e76ab36c066b989b24645e", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures (J) [f1][b1].nes */
	{"deb93b41ea05f6edf9918d0e8250fe1a0c768d88", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures (J) [f1].nes */
	{"fa470c3c748ae3a2e7db817a08e0f68205092f9d", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures (J) [f1][t1].nes */
	{"9e282f597970e70e1d6776af6a35a235e73adb3c", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures (J) [f1][T+Chi_MS emumax].nes */
	{"61544e934c6f6cc5fde69135a9dabb641e526912", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures (J) [!].nes */
	{"de0048cf06032ce82e8f187958cefe53c8fa6f6d", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures (J) [t1].nes */
	{"aedce98281c36b7d0479c55fd65c873bdb544451", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiny Toon Adventures (J) [t2].nes */
	{"bc01e450d01d57f423ddb5b662b4e4d05477b259", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Moe Pro! - Saikyou Hen (J) [hM23][p1].nes */
	{"f230f8d36653532393ece507a24cb4d746f5d9d1", 23, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* World Hero (Unl) [b1].nes */
	{"a4febc49dec126cc66c311d8b0c0d3055bd339d2", 23, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* World Hero (Unl) [!].nes */
	{"b338be96b19445016a5ea5ccd8248f62cd5e1622", 23, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

		/* Batman 4 (Unl) [!].nes */
	{"95b08688c277e5d697f34f44a893918fd091b552", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* FIFA International 2' 96 (Unl) [!].nes */
	{"c929a66402eb154fa6838a6a8c0fd3cf4c3c4f65", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goal!! (J) [p1][b1].nes */
	{"05b83f6f18079219595b78d43d7eb3c8c3631d38", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goal!! (J) [p1].nes */
	{"df1f2c990d245f1c5e605cf5daf972ccadd22011", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pizza Pop Mario (Unl) [!].nes */
	{"4267d5ba3f971f73a2309ae90dea8c28a149de10", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles (J) [o1].nes */
	{"e080bfbff6b5bb63bbc27c5885fedc660f344833", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles (J) [t1].nes */
	{"e68223843295a9239feeca1f677ad75c1b891e99", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles (J) [t2].nes */
	{"cff03427be81389d2ffe3e3bd6ae8dca30e5f79d", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles (J) [t3].nes */
	{"616ec9312cc9181d659b9614c3818bf2196ff11e", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles 2 - The Manhattan Project (J) [o1].nes */
	{"b004ad1c691de84e41aa092d4e65d218b20aafca", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles 2 - The Manhattan Project (J) [T-Chi_MS emumax][a1][o1].nes */
	{"b932257162d729ed0e305bc30f11ead1c18775c3", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles 2 - The Manhattan Project (J) [T-Chi_MS emumax].nes */
	{"318716fba77a4b0310a391ca45bae5955114a910", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles 2 - The Manhattan Project (J) [T+Chi_MS emumax][o1].nes */
	{"adb2ded147cd3295c84d34951286e0a15e94ee79", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Teenage Mutant Ninja Turtles 2 - The Manhattan Project (J) [T+Kor_eastwind].nes */
	{"4568f379dbe6e73df96466e332c90e88962674da", 25, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* Gradius II (J) (VC).nes */
	{"cf97e13a30b688eafdd0509b63e0228ea2185a25", 25, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

	/* AxelayNES.nes */
	{"ab210c819a69026ec4d0ebcae630ecfc56669b9b", 27, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* AxelayNES.nes */
	{"dd8a615289e5508640a8c21922e3eddb272917d6", 27, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* World Hero (Unl)(TV System Select)[!].nes */
	{"0b351c586130ad0ae6203144136b6e199fa5ae20", 27, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 30 ---------------------------------------- */
	/* Battle Kid 2 - Mountain of Torment (U).nes */
	{"dcd6870e0d5a79899dce11b975c8a16652fc7143", 30, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------- BxROM ------------------------------------------- */
	/* Bio Senshi Dan - Increaser Tono Tatakai (J) [hM34].nes */
	{"26b9209aed037a3df48c16ef0e490e4686f3df8c", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Shen Long no Nazo (J) [hM34][b7].nes */
	{"6e2ee85d8f14ad8fcadf36eccbbab4bc17b97a49", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Shen Long no Nazo (J) [hM34][o1].nes */
	{"cac343f85b13c62b8e5972b863fc922da2bd9c08", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Shen Long no Nazo (J) [hM34][T-Eng1.0_TransBRC].nes */
	{"db0b2b60aedceb37fa3423e5cfc603737d555045", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Shen Long no Nazo (J) [hM34][T+Eng1.11_TransBRC].nes */
	{"34264167d47730d392b029ae87ce30d7a568f44d", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Shen Long no Nazo (J) [hM34][T+Por1.00_TransBRC+CBT].nes */
	{"09c0180b4f3e4301198484be08285465159283b4", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball - Shen Long no Nazo (J) [hM34][T+Por95%_CBT].nes */
	{"999828ad3a37296fbcb6afaf1cff25978f9494c2", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Impossible Mission II (U) [b1].nes */
	{"b1c919a40fa6c1fc368d40c8afb7c6de9e9c00b8", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Impossible Mission II (U) [o1].nes */
	{"5a199f03c366930e06eb2247c593254bb99df208", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mississippi Satsujin Jiken (J) [hM34].nes */
	{"082a83f58720191979bb903260939d7a1bc87635", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* SD Gundam - Gachapon Senshi 2 - Capsule Senki (J) [hM34].nes */
	{"25eda90ce02bfb1feb15c9a0ade09a7a20db46c5", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Takahashi Meijin no Bugutte Honey (J) [hM34].nes */
	{"66f44675d7690a687cb8f7405edc2c6681f5c618", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dance Xtreme - Prima (Unl).nes */
	{"e634f17660ee95a5cb968c9aa7a93034ae55ac89", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Darkseed (Unl) [p1][b1].nes */
	{"40bc04f58bd288c66150018a9445abe9a604462b", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Darkseed (Unl) [p1].nes */
	{"480fc6261897f1d44f17fbd0fe8fc3f538861589", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* LIKO Study Cartridge 3-in-1 (Unl) [!].nes */
	{"1b8675b55ddd3d0430358ea3a2f888f210c8f680", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Titanic 1912 (Unl).nes */
	{"b96cb87b7cb300bc9225e8778d928672c2659ee6", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 3-D Battles of World Runner, The (U) [b5].nes */
	{"a74e7a04611536f7523a27d506f01d73d0d956b3", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deadly Towers (U) [b1].nes */
	{"fc15dba2abed3e3d9c0409edbd213158b8da27b9", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Deadly Towers (U) [o1].nes */
	{"764c9a41db58d9a0f8dd56499cf1ea84c10addf8", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashou (J) [b1][o1].nes */
	{"962e318b883d7dc3848a3e52eb62720850298426", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashou (J) [b5].nes */
	{"f1f3712345704ebb0040e3b82f9132804d477ad2", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashou (J) [b6].nes */
	{"e3ce526d4af1045f19a5445d014602be14585458", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashou (J) [b7].nes */
	{"0b099fed3dafb39191aedb8bded5313dac3ff1ba", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mashou (J) [!].nes */
	{"a6b1a2acae1e4034c5e048d1318541d67a86f17c", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Variable Width Font Demo v0.02 by Damian Yerrick+Blargg (PD).nes */
	{"8ef30f9143876207c9862fbc03af353df416fce7", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Caesars Palace (U) [b1][o1].nes */
	{"204988c8d9547839c83e8b4d856a696d18518978", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Caesars Palace (U) [b2].nes */
	{"2e6c5157bf7b62a1a6cd9c80dd20c8d689bbd879", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* WWF WrestleMania Challenge (U) [b1][o1].nes */
	{"a93a6e0039d119ae04c5fb803723b67468f0103b", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* WWF WrestleMania Challenge (U) [b2].nes */
	{"f1745a076053b5cf55a71390b1e5d12a99c08221", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* WWF WrestleMania Challenge (U) [b3].nes */
	{"5cad070d0926f25941556c5274b5ac9ce9d1c6ef", 34, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 36 ---------------------------------------- */
	/* 3-in-1 Supergun (Multi)[Micro Genius][1993][Zapper].nes */
	{"eb05d4cb13c3a334fa180c16f78ad4f8886afc88", 36, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 37 ---------------------------------------- */
	/* Super Mario Bros. + Tetris + Nintendo World Cup (E) [b1].nes */
	{"3fba4a79a0898ee9782243401fcdb88c61e999bd", 37, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. + Tetris + Nintendo World Cup (E) [!].nes */
	{"1781381c95eadb7756186a5a52d3fb73e9cf51a6", 37, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 45 ---------------------------------------- */
	/* Kunio 8-in-1 [p1][!].nes */
	{"b80dca69c0943fea309d3cc0bf34d6bfc38e29c9", 45, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------- Taito ------------------------------------------- */
	/* Bakushou!! Jinsei Gekijou 3 (J) [o1].nes */
	{"4961e8c599137438c14c27ebb0392c840be308a1", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bubble Bobble 2 (J).nes */
	{"d64e1006c7db5e2e605451f2db659e217619b9a5", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Don Doko Don 2 (J) [o1].nes */
	{"ea1d369fd6ce689db1456752b1dc90fe2a4138da", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Don Doko Don 2 (J) [t1].nes */
	{"e4ba09274a63d9ad39b2280455b2661356af4c6a", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Don Doko Don 2 (J) [t2].nes */
	{"25bc3c723f108280b8f64f3f3356143beadb67cc", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Don Doko Don 2 (J) [t3].nes */
	{"b01cc8c170812896b9242181a71632cd5bdc4305", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain Saver (J).nes */
	{"17a2f568f1d9f380b17b1aa3516e901b22668f35", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Flintstones, The - The Rescue of Dino & Hoppy (J).nes */
	{"0609d3fa46bde957636afe28645422a52231f676", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jetsons, The - Cogswell's Caper! (J).nes */
	{"77e12ce848192a51309393136c4a240c729ddb30", 48, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 59 ---------------------------------------- */
	/* Menu_42_Game_[p]_[!]_by_Guyver.nes */
	{"dd545b471abe0a158c1bdcc656138274adee7eae", 59, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TN 95-in-1 (6-in-1) [p1].nes */
	{"9a37bc3003f6629499fe86c7ca2487a74291bcff", 59, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 5-in-1 (Multi)[Unknown][YH501].nes */
	{"f0bf757efa141060298295e1ec38af43fa768356", 59, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 60 ---------------------------------------- */
	/* Reset Based 4-in-1 [p1].nes */
	{"8765e6ec43d4c767e2568ac55e35ed670dfa8730", 60, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 63 ---------------------------------------- */
	/* 82-in-1 (82AB Ghost of Charming Girl) (Unl) [o1].nes */
	{"50cd94badca3cd6cddfa708f3c732183bce7b2f9", 63, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------- GxROM ------------------------------------------- */
	/* Dragon Ball (E) (PRG0).nes */
	{"b493bdd843627d63353f73b846d120723149d59d", 66, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball (E) (PRG1) [!].nes */
	{"2338059ab6dc540e59283edee65714bcd94ac188", 66, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* AV Super Real Pachinko (Japan) (Unl).nes */
	{"c11d158465475bfe8fdd950fb39180df2ef597f0", 66, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. + Duck Hunt (E) [!].nes */
	{"63db029e1e1ff3374c0b08b9df198018d4472716", 66, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mississippi Satsujin Jiken (J) [h2][o1].nes */
	{"e0459af1bcd2a8510bc058c9c00bb5b4bf64c83b", 66, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Sunsoft S4 ---------------------------------------- */
	/* Maharaja (J).nes */
	{"4ec7cd65520d37467088a9cc726156c030a307ca", 68, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Sunsoft FME7 -------------------------------------- */
	/* Barcode World (J) [!].nes */
	{"92f055653d33b8e31ec91b458357e75ad2336c33", 69, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Honoo no Doukyuuji - Dodge Danpei 2 (J) [o1].nes */
	{"31b0c5be4394269433d5100502bbf9b92a0c22b7", 69, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Batman - Return of the Joker (E) [!].nes */
	{"6131bc47d788bc9a501cc452799379f7c0fb1ba0", 69, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mr. Gimmick (E) [!].nes */
	{"bbf9c079c11e8ec9682bf4ecddaa2bc7efe803e3", 69, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------- 74x161x161x32 model A --------------------------------- */
	/* Family Trainer - Manhattan Police (J).nes */
	{"686ad5223d0d0f9150fd3262f1808a4209324dd6", 70, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Family Trainer - Meiro Daisakusen (J).nes */
	{"a8a5d1e7cc7d8a726a8edc66d947ef0de9f94ba4", 70, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Kamen Rider Club - Gekitotsu Shocker Land (J) [!].nes */
	{"3e5e51a732523f38369a3fd04aedc68e9023e586", 70, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Space Shadow (J).nes */
	{"5ab195bbbd35cdecc9c00dd7de7a315234fe7300", 70, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Camerica ----------------------------------------- */
	/* Fire Hawk (PAL)[!].nes */
	{"7b23cacd3a6864450561c546c052b6f7f348cf04", 71, 1, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Hawk (Camerica) [b1].nes */
	{"1b22f43bf366ebb4fda452ddf2fc9e980f9dd2f1", 71, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Hawk (Camerica) [b2].nes */
	{"b4ae7c551f1b81b89a3e7b1e344a39180a1f8ee3", 71, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Hawk (Camerica) [o1].nes */
	{"334781c830f135cf30a33e392d8aaa4afdc223f9", 71, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 74 ---------------------------------------- */
	/* Di 4 Ci - Ji Qi Ren Dai Zhan (Ch).nes */
	{"171fe225bbd3460f09531a55e2354df2a61129e4", 74, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ji Jia Zhan Shi (Ch) [a4].nes */
	{"bbe9ade0f5ec240115e40a459a5b9db252bbd630", 74, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ji Jia Zhan Shi (Ch) [a5].nes */
	{"d6beed11fd4863dc4e95eb1ff683e52bc5d3a012", 74, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ji Jia Zhan Shi (Ch) [a6].nes */
	{"449fe6ba3be25a623f55901f0c5e75f40cc071c7", 74, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ji Jia Zhan Shi (Ch) [b1].nes */
	{"2a5b809c45ada5bf80c1f0571269c9d3751e96e6", 74, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ji Jia Zhan Shi (Ch).nes */
	{"83eab901663029e13c0351d8459752a8a5fa85fb", 74, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ji Jia Zhan Shi (Ch) [T+Chi20061221_MS emumax].nes */
	{"4030c1fd138df6db53404d0ea25d81d4e46208f0", 74, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wei Luo Chuan Qi (Ch).nes */
	{"10bdfec00b39d77d1ed89545df44426ba3ae2e29", 74, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Jaleco ------------------------------------------- */
	/* Holy Diver (J) [o1].nes */
	{"57e96d24d2aa7627a21fd6b5657c0f603e8711d7", 78, 3, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Holy Diver [Easy].nes */
	{"bd90cf262c24d9b0335d6eeb43aed4dc3aaa917b", 78, 3, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Holy Diver (J) [t1].nes */
	{"5a683ef35bf8afeb977b59e314f176d8f61b76b9", 78, 3, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Holy Diver (J) [t2].nes */
	{"7159fdc13c67afc6517e297e553b51a1ebad9a8c", 78, 3, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Holy Diver (J) [t3].nes */
	{"41f445180c06a5491dd772c16ff3f362bf982152", 78, 3, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Jaleco ------------------------------------------- */
	/* Blackjack (AVE) [o2].nes */
	{"2e501d454c6229dd29cb8eaeba2c99a52e3dafbe", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Blackjack (AVE) [o4].nes */
	{"045f4a47d00533ece07081d3f12b50bb830e5207", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Religious Blackjack by RyanVG (Blackjack Hack).nes */
	{"d89404c95c5765eecb30373bb95ae46ffeb617ea", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Deathbots (AVE) (V1.0) [!].nes */
	{"b1e4b67b0a8b61fda6cbe142e53e90e6a4bcbf80", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Deathbots (AVE) (V1.1) [o1].nes */
	{"a9ccff26a57f92d3fa4ea3e5bfc31f25ffdcd43e", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Double Strike (AVE) (V1.0) [!].nes */
	{"86e482c16f3d08fc86c42a8a21b5e14eff7f69c8", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Double Strike (AVE) (V1.1) [o1].nes */
	{"d9461d1a8f7003dc7e6a754116ad1c90228a35fb", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Dudes With Attitude (AVE) [o1].nes */
	{"33aa683eb46e5e92af6e8cdfa8177851bb4d18d8", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 City War (AVE) (REV1.1) [!].nes */
	{"de66004bd0d88aee42c9efe6c65fb1937ab6b9bb", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 City War (AVE) (REV1.x) [a1].nes */
	{"9c909a3790acb9f057fdfb62c54fb471a57ea345", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 City War (AVE) (REV1.x) [b4].nes */
	{"3220b2018c395b99a19b53eb499f84c73d02f4b3", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 City War (AVE) (REV1.x) [!].nes */
	{"f9aecd119f631ac68b5d7b92de87c5b5870f7ed4", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* F-15 City War (AVE) (REV1.x) [o1].nes */
	{"ad0c35de53f235a3f3933aabccc5be921decfabc", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Futebol (Unl) [o1].nes */
	{"bb68961595633ef9dafadbb69af0d0f46cc52b1f", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Krazy Kreatures (AVE) [!].nes */
	{"0929c3e2866e221cc63f16805a59cce23f565ab2", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Krazy Kreatures (AVE) [o1].nes */
	{"e787a9e00a56a6156c467e410641fab10611fb67", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Mermaids of Atlantis - The Riddle of the Magic Bubble (AVE) [!].nes */
	{"03b5d95bd2ae1c0a005b22624fa973e32670b36c", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Puzzle (NINA03-USA) (AVE) [!].nes */
	{"7ce6d42024313ed82a9649b468f642f3101538eb", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Pyramid (AVE) (PRG0) [!].nes */
	{"d23b489dc965585888fbcf610931181430dd5d33", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Rad Racket - Deluxe Tennis II (AVE) [o1].nes */
	{"8c676cc9bef5b93c5a7630d12b8a5b2febce3438", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Solitaire (AVE) (REV1.1) [o2].nes */
	{"16920ddf01a0d994fd9de530a2aa43ccac1487da", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiles of Fate (AVE) [o1].nes */
	{"803e0c1ee67b24ce9937f158aa965569e92fa979", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Tiles of Fate (AVE) [T+Fre_KS151].nes */
	{"242ab629f3a721c030deb630cb94eaeee8fae99a", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Trolls on Treasure Island (AVE) [o1].nes */
	{"65a5dd04b8b6822a82dcd50f82c3c0f942e8610a", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultimate League Soccer (AVE) [o1].nes */
	{"dc19e9d883ff3c084ffd3b24554ba166a6f10f27", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Ultimate League Soccer (AVE) (PAL) [!].nes */
	{"7966e616abfb294feb22a255f6ad352b514101f8", 79, DEFAULT, PAL, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Venice Beach Volleyball (AVE) [b1].nes */
	{"3b4286290967c59de8a19298256e8238ee671da7", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Venice Beach Volleyball (AVE) [o1].nes */
	{"f16caf18b4c888cb4d1e495ac5481417f95466e6", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Mahjang Companion (Sachen-Hacker) [!].nes */
	{"d9149219f6b8121641d9fad4a11f0e054b5ed2f5", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Metal Fighter (Joy Van) [!].nes */
	{"eaa991b7d38f2493ae5f8cf52564b732bedf2a6e", 79, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Papillon (Joy Van) [!].nes */
	{"c52828b3159306a6e1d91678c7fe1b8cdfb422cb", 79, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},

	/* Pipe 5 (Sachen) [!].nes */
	{"1eddbad1f6f53dd403f8fc2e0512d540d6867200", 79, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Taito X1005 --------------------------------------- */
	/* Yamamura Misa Suspense - Kyouto Ryuu no Tera Satsujin Jiken (J).nes */
	{"74683a411fcfa269d644a6caf5ab498dab356406", 80, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 83 ---------------------------------------- */
	/* Dragon Ball Party [p1][!].nes */
	{"af8df8dd9449d512c34deddee2a758ca46677bd5", 83, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fatal Fury 2 (Unl) [a1].nes */
	{"7eb0902c5da5cb0635af2d2559ba4f2467d2baa4", 83, 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fatal Fury 2 (Unl).nes */
	{"5118e873f62ca3421f344edb8c67635165e8a009", 83, 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fatal Fury 2' (Unl) [!].nes */
	{"3012891262def01890ff0b24dcc777ff5ed74c9b", 83, 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* World Heroes 2 (Unl).nes */
	{"4f9b4b225eb17982b306b676ed5a8b9cd4667701", 83, 0, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------------- VRC7 ------------------------------------------ */

	/* Lagrange Point (J) [b1].nes */
	{"2dc2c795421a5db2427c460f35828a23beba9274", 85, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lagrange Point (J).nes */
	{"c87e7e6a68dd9c7e24652cd2c7d390a14e8adf04", 85, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lagrange Point (Japan) [En by Aeon Genesis v1.00].nes */
	{"aefe0d1a7c618d38b3962f654fea6b7c36dd74d3", 85, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lagrange Point (J).nes (English patch 1.00 by Aeon Genesis) */
	{"e808ebc015a94a38dcb0eaa9383267beb4cf08ea", 85, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lagrange Point (J).nes (English patch 1.01 by Aeon Genesis) */
	{"4bbdd562584f2c5736024ac19995397b4edff1f9", 85, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lagrange Point (J).nes (English patch 1.01 by Aeon Genesis) */
	{"dbc7ccafd0a2a43682c94697964642ac8d2d5d7d", 85, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lagrange Point (J) [eng 1.01 by Aeon Genesis].nes */
	{"33c6c29404e1d3f01fa0059acb6949eb2bcd80f0", 85, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Jaleco (JF05) ------------------------------------- */
	/* Argus (J) [!].nes */
	{"5f58aaca377794f9ce9971aea8cc8223fa2a64ab", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Choplifter (J) [b1].nes */
	{"dbe379e780154bfbdc3f036071a3fd23745241f2", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Choplifter (J) [!].nes */
	{"5172b48008d60db7fccc7e502d93a4dcada591ee", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* City Connection (J).nes */
	{"c8592217287850fc7a5cae79ac7079a777b34ea9", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [t1].nes */
	{"44f61ab1185241c3352a2e6856f8ff65fa8a07f9", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [t2].nes */
	{"ac601d32ec5674df2c1974ab4cd7c06748278513", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [t3].nes */
	{"72b2b985e59110d7baab8aec56dff60aee6146e4", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [t4].nes */
	{"8ef867e904335816529df4be8d67a153ce2195b3", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [t5].nes */
	{"b70de599788bad2f207413f333c573b29956c063", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [t6].nes */
	{"9ebf38976318ff767b864330b929555e866cc51c", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [T+Por1.00_Iyasu].nes */
	{"1099c418288500c9e99e5a3138f1778ba8030d67", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [T+Por85%_Made In Brasil].nes */
	{"e90c400175451c81256e7e27bb31bd1e11fa453b", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (J) [T+Rus].nes */
	{"51d48fdc198709206416e8bde9717cc08290fda4", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (PC10).nes */
	{"8e0374858dce0a10ffcfc5109f8287ebdea388e8", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goonies, The (PC10) [T+Fre1.0_ks151].nes */
	{"6dd1b8912412074e3dd8bdf836704f92e48507a3", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Hyper Olympic (Genteiban!) (J).nes */
	{"91023417168e05ea9b63696ccca34355fc53011c", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jajamaru no Daibouken (J) [!].nes */
	{"474ebc84f7c19b357e0f9ef087da1b6a103900b8", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kage no Densetsu (J) [!].nes */
	{"7b71f891dbf8beb9ab4a13e18313c6321eec1b29", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kage no Densetsu (J) (VC).nes */
	{"a4c8b7d762cc0be1cdbba479498d51c5d472988d", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ninja Jajamaru-kun (J) [a1].nes */
	{"bcb59af271894e6f20cc64ec5792c306ae13d556", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ninja Jajamaru-kun (J) [!].nes */
	{"1f8b2c945438612968fb524f927decd85b535d79", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [b1].nes */
	{"36f095eac927027f484f341c8ac2762f88ddec1b", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [b3].nes */
	{"01aa4342f89181a8f862791535f8d1662ad803a9", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [h1].nes */
	{"046f4e5def071c5389dc163df165dba7c8a46562", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [!].nes */
	{"74e20e84d5cbc021e977fdcbed37fa0de0fd60b5", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [p1].nes */
	{"e1056164a97f2947fe2781432d6b56fa9d1cbb04", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [t3].nes */
	{"dfd5f10a62ea7c4247a049ae044d43f99f902d3a", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [T-Chi_MS emumax].nes */
	{"95e2b2c031397f93039baf7a1074eab4f0686a21", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [T+Chi_MS emumax].nes */
	{"70951e9b6d543e4c561ff16b641166e9e5d57fec", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [t1].nes */
	{"77bf17d0bb239fe56529dfad49b3255289d05757", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* TwinBee (J) [t2].nes */
	{"bb4142a8ef1c70a7d7d1af32e4e83ab2fb0025a0", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Urusei Yatsura - Lum no Wedding Bell (J) [!].nes */
	{"1a83157cf727a5e707a54bef0d6bb213206f819d", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Urusei Yatsura - Lum no Wedding Bell (J) [a1][T+Fre].nes */
	{"30493270d26fff020fa68fb764334300ab38d868", 87, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Namco 3433 --------------------------------------- */
	/* Dragon Spirit - Aratanaru Densetsu (J) [o1].nes */
	{"9a7cc535c03bb3df22e642fdf25396034f948a8c", 88, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Spirit - Aratanaru Densetsu (J) [t1].nes */
	{"3f38915e5f55cd4b972f2b00d23c244217a692d3", 88, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Namcot Mahjong 3 - Mahjong Tengoku (J) [o1].nes */
	{"0084bc60a7012071e6e6f7e2fda01657dcd7e700", 88, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Quinty (J) [b1].nes */
	{"7a02de4404443339c6245d088ee9e5be4b338752", 88, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Quinty (J) [!].nes */
	{"7eff416a03ae5e33f5abf074a1c3553b6f3a53c0", 88, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------- Bandai 161x02x74 ----------------------------------- */
	/* Oeka Kids - Anpanman no Hiragana Daisuki (J).nes */
	{"32d71dd6c5a8d78a918fe1b9d6d6c4a570d9652d", 96, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Oeka Kids - Anpanman to Oekaki Shiyou!! (J).nes */
	{"2d95c59ccd0b3faed4b4e3e61ac2859352d59594", 96, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 100 ------------------------------------------ */
	/* Ai Sensei no Oshiete - Watashi no Hoshi (J) [hM04][b2].nes */
	{"3e26f7a507c4ce2fccbf13a1b45ebb0d3992be56", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akira (J) [hM04][t1][b1].nes */
	{"e9f9f9ff5479f815dd3023259ed41a16d949ae67", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Akira (J) [hM04][t1].nes */
	{"ed3e18f66ace9ec2e90a5c261343001b528527d7", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Aladdin (Unl) [p1][hM04][a1].nes */
	{"bf9a3fc1bedac091f78401bbb459affd7944fe8f", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Aladdin (Unl) [p1][hM04].nes */
	{"defd73f5a538653623b020aec42d63e0ca9d522f", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Bakushou!! Jinsei Gekijou 3 (J) [p1][hM04].nes */
	{"33d7dabc1cc10c35604cc53aa16d0f8c5dd56789", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chibi Maruko-Chan - Uki Uki Shopping (J) [hM04].nes */
	{"6183bc974cc8048cb0548b0543f7581e791d7ffd", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Contra (J) [hM04].nes */
	{"57e70c04b63338dbc5ff138e9c188ca4197c8319", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Contra (J) [hM04] (PAL).nes */
	{"bb42daa689c67a4b84c3a5d9e2a68cb9d2ce8802", 100, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Crisis Force (J) [hM04].nes */
	{"6956a6602fbc27153c580963e8992f48406fdf13", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Digital Devil Monogatari - Megami Tensei II (J) [hM04].nes */
	{"95db1e65c66a1f012d55fef7ab6d9775d64f32ab", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Digital Devil Monogatari - Megami Tensei (J) [hM04][b2].nes */
	{"d9eece23cf14724e4b131da5b0fbd491b8aa2cfe", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Digital Devil Monogatari - Megami Tensei (J) [hM04][b3].nes */
	{"5ddeb88308ab21e782dbbaeafbb9f2f81931a005", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Digital Devil Monogatari - Megami Tensei (J) [hM04][o1].nes */
	{"b2eab7f21d55633ffcc2bfa5204900e6d0bd98e1", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Super Butouden 2 (Unl) [hM04].nes */
	{"b2b155af7f9daaed53be121f7c87b94212fab0e5", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Spirit - Aratanaru Densetsu (J) [hM04].nes */
	{"7379db9e8da7a817c8c16d2cd25bedf7c1c5cad8", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Erika to Satoru no Yume Bouken (J) [hM04][b1].nes */
	{"1eb7c8bf171607741d15e1f83c4a1a49e909b892", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '91 (J) [hM04][b1].nes */
	{"6cf7a8f6cfc5fec9400f226f6f7db584ce220c1f", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '92 (J) [hM04][b1].nes */
	{"b85385ef5a6d754f9c339a81576c37d0911dfc9e", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '93 (J) [hM04][b2].nes */
	{"31aa672929054b31a2e5c8e7ffc1de10f379e15d", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '94 (J) [hM04][b2].nes */
	{"a79d0fdf45149e84fc9a781edae48af5a8e23ccd", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '94 (J) [hM04][b3].nes */
	{"5e9e2ef10bf379b46689203366ce6d257ab7c934", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Emblem Gaiden (J) [hM04].nes */
	{"3f23b30a5803b3aac26319a7ad0439577bf183ae", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Emblem Gaiden (J) [hM04][T-Eng].nes */
	{"68bf7ac0492d65c80340bd1f4e0d62e5f0fcdf31", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Emblem (J) [hM04].nes */
	{"197ba38c3eeaf43b213f5672697df502ea825eb3", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ganbare Goemon 2 (J) [hM04][b3].nes */
	{"0f892f278f0931185ffc6ebbf662733331491a03", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ganbare Goemon 2 (J) [hM04][b5].nes */
	{"d127a1e2a9a90fd829994f25229255ea63a16045", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ganbare Goemon 2 (J) [hM04].nes */
	{"56fa0d62680661f6fe65b58f245ccf3ee38b73dd", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Gradius II (J) [hM04].nes */
	{"cdbb9a203abbf496060de83787df5ff165c5fa1e", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jajamaru Ninpou Chou (J) [hM04].nes */
	{"af8f45822591ef652fc0caa037032c1d38004225", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jarinko Chie - Bakudan Musume no Shiawase Sagashi (J) [hM04].nes */
	{"6785b5e406a1b191bac8128a0245897f212c1da1", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kaijuu Monogatari (J) [hM04].nes */
	{"a9f7077382335791255490331ca6bf40d8edda03", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* King Kong 2 - Ikari no Megaton Punch (J) [hM04].nes */
	{"01f42fab8f0d7c62cd3937c2ba5adc73272802f7", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* King of Kings (J) [hM04].nes */
	{"4e54b30f7f09b48f429ac68e31a19b99f212451d", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Kyuukyoku Harikiri Stadium (J) [hM04].nes */
	{"e859bd7d4a0e17d1d9185262455c173c520527b4", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mario 7-in-1 [p1][hM04][b1].nes */
	{"38e51ad10f4317c2c71653db8446c66ec9526b1a", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Minelvaton Saga (J) [hM04][b2].nes */
	{"43f6fd8ddf3335b27ee8e3b74838b9982bdbb6c0", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Minelvaton Saga (J) [hM04][b3].nes */
	{"30573c4daa2fc0927ce27708dbfcda1d18559376", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mirai Shinwa Jarvas (J) [hM04].nes */
	{"b5dacd097fd2568a8d29d7efb17170044e66c9e2", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Namcot Mahjong 3 - Mahjong Tengoku (J) [hM04].nes */
	{"bbe7918075cc8360cdb7cd5704402b173357a86a", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Parodius da! (J) [p1][hM04].nes */
	{"8f5d78b9f9c82c49d29a43a19abc56a935051a04", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Quinty (J) [hM04].nes */
	{"676a4cf4a8765fae084ebade0d92a5f7b4810450", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Star Wars (J) (Namco) [hM04].nes */
	{"6028cbd507eee4cecf4a7a5a7ee09f6cf0911ce8", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 11 (Unl) [hM04].nes */
	{"6ddcb429995518c31824018f6c0e465b75a631b5", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Mario Bros. 11 (Unl) [hM04][t1].nes */
	{"86e9c0ca48196caf64d56314c00f304fb3904b6d", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Takeshi no Sengoku Fuuunji (J) [hM04].nes */
	{"14b4c39e5216954457281c2e60d30a2072165b60", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tetsuwan Atom (J) [hM04].nes */
	{"c5391a9e1cae423bcbccaf540031a5ce18ab02d3", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Top Striker (J) [hM04].nes */
	{"c7b5993c490a9eb6e7add35acc3331402278dd07", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wagyan Land 2 (J) [hM04].nes */
	{"3dff2552998b30beb260a12ce25d10792323bbc4", 100, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Camerica GoldenFIve ------------------------------ */
	/* Pegasus 5-in-1 (Golden Five) [o1].nes */
	{"60fc5fa5b5accaf3aefeba73fc8bffd3c4dae558", 104, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------- Ntdec (ASDER) -------------------------------------- */
	/* Chik Bik Ji Jin - Saam Gwok Ji (CN-20) (Tw) [!].nes */
	{"ae0b625f035fa0ec9e3b6a44752a5b64559dbbd5", 112, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Cobra Mission (CN-27) (Ch) [!].nes */
	{"495f5b3ff54e99de3479bd3e7ec2a534a8e2e3fc", 112, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fighting Hero III (Unl) [!].nes */
	{"40ef386e14c39cad0fa19454bc0892f097b7866d", 112, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Huang Di (Ch).nes */
	{"e26ef0fcb1c7b8f51b880a0fcb16004a5c052fce", 112, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Master Shooter (CN-26) (Unl) [!].nes */
	{"89d8a6170b065ed17fa3fe2e2db505bebc34764c", 112, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* San Guo Zhi - Qun Xiong Zheng Ba (Ch).nes */
	{"36280662bda3cf9f737d2fd8144a4683f2b49bdc", 112, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zhen Ben Xi You Ji (Ch) [!].nes */
	{"98476ed2b00ea977824174748e2fd4ec986b1884", 112, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* --------------------------------------- Hes -------------------------------------------- */
	/* AV Hanafuda Club (Hacker) [!].nes */
	{"98983c8e2e511a43c9b76801f3b788238a9fca20", 113, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fun Blaster Pak (HES) [!].nes */
	{"a4d3d7fe07ec13d812abcff05857c2270a1fe1b5", 113, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* HES 6-in-1 (HES).nes */
	{"5a0475d7cf81cac591ccecb802c579cc433160cc", 113, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mind Blower Pak (HES).nes */
	{"9e336710eccba90d83aac5b7990851648992bfeb", 113, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sidewinder (HES) [o1].nes */
	{"294a82b061ce9c0908305777656f026b8474066a", 113, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Total Funpak (HES).nes */
	{"17c721cab79fe83eb47a6fb1e53af1f65b851558", 113, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sidewinder (Australia) (Unl).nes */
	{"b1cc3ed04c610be18742a3fd78a8f27a597f8549", 113, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* --------------------------------------- 114 -------------------------------------------- */
	/* Boogerman (Sugar Softec) (Unl) [!].nes */
	{"3a0a145ac8dadf51f8310c952f352950e6e25acc", 114, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Lion King, The (Unl) (Mapper 114).nes */
	{"d093664a551cd05f09cb28968429c4b42379d254", 114, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Kasing ------------------------------------------- */
	/* AV Jiu Ji Ma Jiang 2 (Unl) [!].nes */
	{"d8267c9f23d03e08e588cbfdb19baacac2977639", 115, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shisen Mahjong 2 (Ch) [!].nes */
	{"c86bced383d22538657490228680ab6e51819085", 115, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Thunderbolt 2 (Ch) [!].nes */
	{"dbc507cd0b59b0a1b232ba9cda6fd3b35f40a61d", 115, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Yuu Yuu Hakusho Final - Makai Saikyou Retsuden (Chinese) (Unl) [h1].nes */
	{"451fb3db30b1479643284652fbe6d971891f71e3", 115, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Yuu Yuu Hakusho Final - Makai Saikyou Retsuden (English) (Unl).nes */
	{"463d17f20ba0a9320e14cbe0bf58e74929ac4f58", 115, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------- Futuremedia ---------------------------------------- */
	/* Crayon Shin-Chan (Ch) [!].nes */
	{"1210ba78511cf727db940f558567999356f7248e", 117, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* San Guo Zhi 4 - Chi Bi Feng Yun (Ch) [!].nes */
	{"55cafec17a29346258f128b17f0cc74607bb1a32", 117, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* --------------------------------- TxROM (TKSROM/TLSROM) -------------------------------- */
	/* Armadillo (J) [o1].nes */
	{"6ad31c2000c804cb2b80b284eef0696f954fc664", 118, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Armadillo (J) [T+Eng_vice].nes */
	{"6de1355bd0688fbf07dbb7943973d9b70aca3d7a", 118, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goal! 2 (E) [!].nes */
	{"6c2989151e613ad39f645973508651fdc947640c", 118, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goal! Two (U) [o1].nes */
	{"ee936f8914d47cdd8e3f3fad2651537974981b3a", 118, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Goal! Two (U) [o2].nes */
	{"17a8335dda231e6a82996e54c49143e1bfa11c21", 118, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* NES Play Action Football (U) [o1].nes */
	{"204c1bc2d59b410d87a1728e128e059496673243", 118, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pro Sport Hockey (U) [o1].nes */
	{"b8f2c09ecce7bc3e731f8b68a87b348fed5cf478", 118, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ys III - Wanderers From Ys (J) [o1].nes */
//	{"f3b3b876c0de347b178e277b057346d62142cfcc", 118, TKSROM, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
//	/* Ys III - Wanderers From Ys (J) [T+Eng.005_Vice].nes */
//	{"c27a30421859695b2e9ee9fffe5b7e08457c63c7", 118, TKSROM, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
//	/* Ys III - Wanderers From Ys (J) [T-Eng0.80].nes */
//	{"d3f11ec08c38231460294ba53ce21f1e3675ff25", 118, TKSROM, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
//	/* Ys III - Wanderers From Ys (J) [T+Eng1.00].nes */
//	{"00e1b6f8e7e630cfe86dbf0b13ce8af9a85805e8", 118, TKSROM, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------- TxROM (TQROM) -------------------------------------- */
	/* High Speed (E) [b1].nes */
	{"37c4155d0d1df690455e4ba10fb3688f61b4ff6d", 119, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* High Speed (E) [!].nes */
	{"115fb5e1736c768e94c254fb8e3e95e6a99c4135", 119, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* High Speed (U) [o3].nes */
	{"ed54e3cff79a727a0f0d7e29e712a2800aec57a1", 119, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pin Bot (E) [!].nes */
	{"f7aeaa0c72bc1e659e1318a20192c4d69636e39a", 119, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pin Bot (U) [o3].nes */
	{"703e41d4c1a4716b324dece6df2ce12a847f082c", 119, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------------- 121 ------------------------------------------- */
	/* Super Sonic 5 (1997) (Unl) [!].nes */
	{"5a19f9e7565f7074dd2b041652c551e06937a1d3", 121, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sonic 3D Blast 6 (Unl) [f1].nes */
	{"f132093a6874859b30c9d4daefe1f58b2860e68b", 121, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Sonic 3D Blast 6 (Unl).nes */
	{"c576df6ed9f825740760350155e85743d06784b1", 121, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Street Fighter Zero 2 '97 (Unl) [!].nes */
	{"806588fc93c772f3b28977cfa3e57a287e721ef9", 121, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------- Txc (T22211A) --------------------------------------- */
	/* Qi Wang - Chinese Chess (MGC-001) (Ch) [!].nes */
	{"d293232ddbdeecd0d999652f02cb3ae2576cf251", 132, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mahjong Block (MGC-008) [!].nes */
	{"61112008249db3bbcdd624424560b8ca9ec5ea93", 132, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------- Sachen (SA72008) ------------------------------------ */
	/* Jovial Race (Sachen) [a1][!].nes */
	{"84ddf82ff8d9f48d2f8d0d969cb83717ab487354", 133, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Jovial Race (Sachen) [!].nes */
	{"181b05d6ba12871059b16239039154837185690a", 133, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------- Sachen (SA8259B) ------------------------------------ */
	/* Silver Eagle (Sachen) [!].nes */
	{"2a4dae7634943c9c86760a00e24d2bc70efb03d0", 138, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------- Sachen (SA8259C) ------------------------------------ */
	/* Hell Fighter (Asia) (PAL) (Unl).nes */
	{"db8e5ff5ffd45517226611c3a577e89a1236e394", 139, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Jaleco ------------------------------------------- */
	/* Youkai Kurabu (J) [o1].nes */
	{"7227a9aa0baf11d324f55280c1c1ddc2d46bebd3", 140, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mississippi Satsujin Jiken (Japan) (Rev A).nes */
	{"5505e239b1317816b3608f973bf46117084216cc", 140, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Sachen (SA8259A) ---------------------------------- */
	/* Q Boy (Sachen) [!].nes */
	{"568bf236fb3085b8bd522fbd2c18c7831e3a2dd1", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Po Po Team (Sachen) [!].nes */
	{"0514b2d5a70601cb895b40d577e60c350fc42438", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Rockball (Sachen) [!].nes */
	{"2f38e7cc617142cbdca1eafbc596840177e5d77d", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 1 - 4 in 1 (Asia) (Unl).nes */
	{"aa801e57925d74b83fe5abc3d294fe5aeb314817", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 2 - 10 in 1 (Asia) (Unl).nes */
	{"7227e6664d7a8e2f386b7bec0b098c1977ec8b54", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 3 - 8 in 1 (Asia) (Unl).nes */
	{"26cf676f1af40c0ff63c07e225a3f40a4c88a377", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 4 - 6 in 1 (Asia) (Unl).nes */
	{"138aa2df6a9369d14ec4fc7e7d491d3f2df1ea06", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 5 - 7 in 1 (Asia) (Unl).nes */
	{"d5303cbd082858070494775f29fea2877da64d56", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 6 - 6 in 1 (Asia) (Unl).nes */
	{"556085d9ef4977cdaa8da86afa359cb838f615fc", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 7 - 4 in 1 (Asia) (Unl).nes */
	{"d6f2111ebb31e11dfb94bdddfe6b8048653d55c0", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 8 - 4 in 1 (Asia) (Unl).nes */
	{"6b112db7b32511654dbcedf86baa54b7560b6dcf", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Cartridge Ver 9 - 3 in 1 (Asia) (Unl).nes */
	{"f0784b3ded01f16b252ad99594894a0f5e7989ad", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Pang (Sachen) [!].nes */
	{"769ba79eb7e3133aa692487f300333c618badb40", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Pang 2 (Sachen) [!].nes */
	{"41dc9fb7107f8130621da4def43d5b9e4aefe5bf", 141, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Agci --------------------------------------------- */
	/* Death Race (AGCI) [!].nes */
	{"d33248186697e488021358369e6407b5d85526fc", 144, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Sachen (SA72007) ---------------------------------- */
	/* Sidewinder (Sachen) [!].nes */
	{"248e11d7dba271a9fa047a357f9d532494969568", 145, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Ave (NINA06) ------------------------------------- */
	/* Metal Fighter (Asia) (Sachen) (Unl).nes */
	{"5c92ee55b274b658725c358301d802c91d9aac12", 146, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pyramid II (Sachen) [!].nes */
	{"5ade95b4b28acf730258a23d37737bdfb001baba", 146, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Silent Assault (Sachen-Joy Van) [!].nes */
	{"40bc04ea95570005fe6b4ac316a5bbe94c927d70", 146, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Twin Eagle (Asia) (PAL) (Unl).nes */
	{"ff3cffbd148201fc104165eeea294d1421ac2d56", 146, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Sachen (SA0037) ----------------------------------- */
	/* AV Soccer (Hacker) [!].nes */
	{"db709becf4c74da229dc50f69e87fa84f631a795", 148, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mahjong World (Sachen-FC) [!].nes */
	{"4ec05c4d1627e1aac370987299b32b48c0ef98e3", 148, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mahjong World (Sachen) [!].nes */
	{"b48627517b149261e32d275d46e41f72f48d623a", 148, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shisen Mahjong - Seifuku Hen (Sachen) [!].nes */
	{"a9c6581ce2a59fb5409f2723567c5010714d069f", 148, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Sachen (SA0036) ----------------------------------- */
	/* Taiwan Mahjong 16 (Sachen) [!].nes */
	{"cf16da11aa7a6b4fabc80a589e4fbd0738626a7a", 149, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------- Sachen (SA74374B) ---------------------------------- */
	/* 2-in-1 Cosmo Cop + Cyber Monster (Sachen) [!].nes */
	{"6f2f0620adbea24a67008ef6d6c10014763bbefa", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 2-in-1 Tough Cop + Super Tough Cop (Sachen) [!].nes */
	{"1a55031fe7c59b43882a720c0775486afa892e62", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Auto-Upturn (Sachen) [!].nes */
	{"db78d214ce0386efbc26709245828cbdae50b16b", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chinese Checkers (Sachen-USA) [!].nes */
	{"2c17b7844957d602fc6a6fdd96d68e510ca049d6", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Happy Pairs (Sachen) [!].nes */
	{"aeba0c4b42da0a5a7c6a2ff55982ef9691e7ded7", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Magic Cubes (Sachen) [!].nes */
	{"b904fbed198a7b1e922ccf9f8477da78e847f9a6", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mei Nu Quan (Honey Peach) (Sachen) [!].nes */
	{"729f092550e9d1069014f82629e997fb5f596fd5", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Olympic IQ (Sachen) [!].nes */
	{"e92e0dc1f5d187177601733fcdc297e400575e56", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Poker II (Sachen) [!].nes */
	{"9412579d559bef57b91222c2c6c1934665a9edf3", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Strategist (Sachen) [!].nes */
	{"18bdeee4b7e5b41eca3a74b68415b6a313ed9658", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Strategist (With Intro) (Sachen) [!].nes */
	{"f8ef5025aeec43e277eebcc4614f971c309c6559", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tasac (Sachen) [!].nes */
	{"1a94d8571c0611781488c1ed74d8fbc92de9fc33", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chess Academys (Sachen-JAP) [!].nes */
	{"a687dd3555fe3c79babddfa405a417a393b27de3", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chinese Checkers (Sachen-JAP) [!].nes */
	{"b6db4a62d8442f72994aedf1352fffce745992b3", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Mahjong Academy (Sachen) [!].nes */
	{"da4feef0a5ab050badb4548a1ec888ad7d026ac2", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Taiwan Mahjong 2 (Sachen) [!].nes */
	{"01c3940601960da3e9fc1063e44f878de486053a", 150, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------- 74x161x161x32 model B --------------------------------- */
	/* Arkanoid II (J) [!].nes */
	{"d33c061d6d3b46c6ab312da6d628a5c2a7ef78e7", 152, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Arkanoid II (J) [b1][t1].nes */
	{"c0458ad11e3dff7b73e61ef61b81d4bc23d00858", 152, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Arkanoid II (J) [b2].nes */
	{"4094905bbb6aabad2db67d6641504a43682a4757", 152, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Arkanoid II (J) [T+Fre_terminus].nes */
	{"9a082cb14df3180b142d7e6a959fa98edd5a4619", 152, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ge Ge Ge no Kitarou 2 - Youkai Gundan no Chousen (J) [!].nes */
	{"fa8b6aaf71534ab023bcd94feff9008a201542c0", 152, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pocket Zaurus - Juu Ouken no Nazo (J) [o1].nes */
	{"721f484d52221e96ff89c1eaf171fdc7d938a97e", 152, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Saint Seiya - Ougon Densetsu (J) [!].nes */
	{"1ca2ee893fe00c942822e00dbdc3d0093380a5ba", 152, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 153 ------------------------------------------ */
	/* Famicom Jump II - Saikyou no 7 Nin (J) [b1].nes */
	{"7348a4530054205925f3182aa5f12dd4d8a19410", 153, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famicom Jump II - Saikyou no 7 Nin (J) [o1].nes */
	{"1d7417d31e19b590afceb6a8a6e7b9cab9f9b475", 153, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 155 ------------------------------------------ */
	/* Money Game, The (J).nes */
	{"b4b93631b86bf3863058375718be43c59a37dd65", 155, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tatakae!! Rahmen Man - Sakuretsu Choujin 102 Gei (J) [b2].nes */
	{"d6f525b61b75e0f4662adfd957625a7e5a180662", 155, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Tatakae!! Rahmen Man - Sakuretsu Choujin 102 Gei (J) [!].nes */
	{"4ea4bc91ff74142a51efe3dd79d0c9ed48e784f3", 155, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 156 ------------------------------------------ */
	/* Buzz and Waldog (Prototype) [!].nes */
	{"ef47319e0a953de35e2b96e8113056c3caaf3d01", 156, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Metal Force (K) [!].nes */
	{"7e09097e89b629b2c13640fbde7875970ff238b2", 156, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 157 ------------------------------------------ */
	/* Datach - Battle Rush - Build Up Robot Tournament (J).nes */
	{"29cc759d5ca214eefc0cc1c6ce0f8b9111e58124", 157, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Datach - Crayon Shin Chan - Ora to Poi Poi (J).nes */
	{"6f3c65bd945fe13305a7a39d8cd884a5bf314a8f", 157, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Datach - Dragon Ball Z - Gekitou Tenkaichi Budou Kai (J).nes */
	{"87478b635fefb25fa13c4876e20f505a97426c1b", 157, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Datach - J League Super Top Players (J).nes */
	{"b2089533e05fe95814d0d77f2729944cbeb95988", 157, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Datach - SD Gundam - Gundam Wars (J).nes */
	{"edd7a45a7f27e396b6d686f1861642d509863132", 157, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Datach - Ultraman Club - Supokon Fight! (J) [o1].nes */
	{"533170d5eb685231ccd27b9c136ffa4fad68a7b8", 157, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Datach - Yuu Yuu Hakusho - Bakutou Ankoku Bujutsu Kai (J) [hM16].nes */
	{"1218c891dee878c18d31d38d07cad5fb06b3b2ce", 157, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Datach - Yuu Yuu Hakusho - Bakutou Ankoku Bujutsu Kai (J).nes */
	{"74218aae93e4febfb2284bcf15811453418a2029", 157, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Tengen (800037) ---------------------------------- */
	/* Alien Syndrome (Tengen) [b1].nes */
	{"0ec876f7c35f4bea2eeda9105ba0e243932c8273", 158, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Alien Syndrome (Tengen) [o1].nes */
	{"2209efd63949ff8c47da300985ff255755c5f621", 158, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------- Bandai FCGX (24C01) -------------------------------- */
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [b1].nes */
	{"adbbd539235dd2cc19f588d2af566a42c9972393", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [o1].nes */
	{"176b114f6ba4494a55085ec1ac21b216c6394f97", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [p1].nes */
	{"f7a2813c4f72995480fb2a1411dc87020f20b051", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [p1][T+Por100].nes */
	{"1beb65ec38e16333a6504a1f29c1513a9594bbb0", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T+Eng0.47_Eigo ni Yakusu Productions].nes */
	{"9e40a211d17dd1775e7a1450e6d3ebe3b695a5d2", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T-Eng0.50_TransBRC].nes */
	{"692b97836a4e81ba68ca0cd7a924c45941d88f7a", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T-Eng0.99_RedComet].nes */
	{"d2502fbb9cf349f070073793ca710c7e4797b454", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T-Eng1.00_RedComet].nes */
	{"969123ce9f117ee5b4872d2628904d0b23703871", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T+Eng1.01_RedComet].nes */
	{"62868e772f9a1254715075f050da7a3524afd658", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T+Eng1.1].nes */
	{"9083a9a231e0dd01a6e685a85f57aa02066f591e", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T-Eng].nes */
	{"1ea9d61eada5d4a7602e3e2fc4f92b2398fbeafd", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T+Kor20060719_88ktt].nes */
	{"1d0eaf00ef92ee4de58f7ab54d600178ff9b380f", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T-Kor_88ktt].nes */
	{"28a7221e1d4c90bcebd18c4ae9a17f17e42e7a77", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T+Por100%_RaphFS].nes */
	{"c80f94b552435ac658bc05d641d7b5b6409dfb3c", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T+Por45%_Tradu-Roms].nes */
	{"8d36ef6d27a4cf896b531259a83e773e51451fb8", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T-Spa].nes */
	{"3d467a0a17b44e5402f675d53371b06a724651ac", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z - Kyoushuu! Saiya Jin (J) [T+Spa_PaladinKnights].nes */
	{"cdf549177a055d0958117117677d0605836c660d", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Magical Taruruuto-kun 2 - Mahou Daibouken (J) [o1].nes */
	{"b3f1919e24879d918718f61d583b40926641b57f", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Magical Taruruuto-kun - Fantastic World!! (J) (V1.1) [o1].nes */
	{"94863ef5c44039d436f5e8282c8a0c5714ee78a6", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* SD Gundam Gaiden - Knight Gundam Monogatari (J) (V1.0) [!].nes */
	{"bca657405b367eb52fc9e74e5534428a32027c8a", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* SD Gundam Gaiden - Knight Gundam Monogatari (J) (V1.0) [T+Eng0.10_Assassin_Productions].nes */
	{"35032f052159f2ae1afe3cd658066d90eacf890d", 159, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Namco (3453) -------------------------------------- */
	/* Devil Man (J) [o1].nes */
	{"13da54a3ab62a3eb2707517288d002d5a42691d1", 154, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Waixing (SH2) ------------------------------------- */
	/* Fire Emblem (Ch) [b1].nes */
	{"8feaa4aa4571478cd158b6a5a4e395744f4d1ba6", 165, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Emblem (Ch).nes */
	{"0469c327f52a9c352d0ecc7fe6bdd9f2fcddfb9f", 165, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Emblem Gaiden (Ch).nes */
	{"64d57b052811379e9043804ff18afbec9fb22fae", 165, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fire Emblem Two Dragon Edition (Hack).nes */
	{"8b96846806dc7ffb771eb338fdd1a072fe457ba6", 165, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 176 ------------------------------------------ */
	/* Shu Qi Yu - Zhi Li Xiao Zhuan Yuan (Ch).nes */
	{"f61e641a941850d7f3e5948dc468643a34f69e32", 176, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 3in1 (ES-Q800C)(FSS).nes */
	{"951c25acf6a8951f1e728b554e3fd8ee6bc4dbcb", 176, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 3in1 (ES-Q800C)(TRS).nes */
	{"341143c90eb8d90c7966d38bc0890065fc4bd808", 176, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 4in1 (BS-0210A)(MMMM).nes */
	{"aee107385d53e0d3138a9446f18ca9b62606e035", 176, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 4in1 (BS-0306M)(TPST).nes */
	{"f8364e2c8621cfeb7a880a6a04244e8790e2b43f", 176, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 4in1 (ES-Q800C)(TCCC).nes */
	{"24dce8f88fedf2c450d4efb0246661e7358a9c6a", 176, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 4in1 (ES-Q800C)(TPFS).nes */
	{"00e4643e9108df18c3de70142da2b72737ed3f3a", 176, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* [ES-1066] Xi Chu Ba Wang (C).nes */
	{"22f015e08159f7e5d5b3778ed2adea896ca7afb0", 176, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- Hen ------------------------------------------ */
	/* Mei Guo Fu Hao (Ch).nes */
	{"08b24645ed9b14e47e9c5899d89fe5ccda142fdf", 177, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Shang Gu Shen Jian (Ch).nes */
	{"c981e1f48f1f220f5430f63b574420530d82a5d4", 177, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wang Zi Fu Chou Ji (Ch).nes */
	{"0d6dda4af970459d532957d50b937ca42395cf8f", 177, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Xing He Zhan Shi (Ch).nes */
	{"17473c223453d2d80fcb9dcfa317947287dc5c52", 177, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Xing Zhan Qing Yuan (Ch).nes */
	{"6fa15dd888ab43b5c464ad1b5c1da5fbd426f035", 177, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 178 ------------------------------------------ */
	/* Education Computer 32-in-1 (Game Star)(Unl)[!].nes */
	{"156a81a1c1e0d2238cbdc261f8c71d71afe17d73", 178, DEFAULT, DENDY, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Education Computer 2000 48-in-1 (Unl)[f1].nes */
	{"c8770f4285e7600b15b2e728533575b41db7b612", 178, DEFAULT, DENDY, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Education Computer 48-in-1 (Game Star)(Unl)[!].nes */
	{"1b6da86ea95ca2b8e0793b09b60b4df24c3cc74c", 178, DEFAULT, DENDY, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* GLK Book 48-in-1 (Unl)[!].nes */
	{"c9853f1530c0e557e3098b9ad228c3df111a4fb0", 178, DEFAULT, DENDY, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* San Guo Zhong Lie Zhuan (Ch).nes */
	{"5437f45450954ef46b97e92517a9b48304772596", 178, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------- UxROM (UNROM_180) ---------------------------------- */
	/* Crazy Climber (J).nes */
	{"003b7f1315f5ee0dc60c5f9d989ec8bab0811ed4", 180, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 182 ------------------------------------------ */
	/* Pocahontas (Unl).nes */
	{"3f860b3c1515d600fd214ce7047d2fcd93d2e8f4", 182, DEFAULT, DEFAULT, UNK_HORIZONTAL, DEFAULT, DEFAULT, NOEXTRA},
	/* Super Donkey Kong (Unl) [o1].nes */
	{"5e48932d61ac1c4e538afc6439cea2e5f2563600", 182, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------- TXC (TW) ---------------------------------------- */
	/* Master Fighter II (Unl) [a1][b1].nes */
	{"5136df55e8f69e81eb192d303814e49a4b532066", 189, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Master Fighter II (Unl) [a2].nes */
	{"89f58e5a5041326db44bf223f150f6427c924b2e", 189, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Master Fighter II (Unl) [h1].nes */
	{"b596ada2076a8a86fc41b2f1cad06ff807488fcc", 189, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Master Fighter II (Unl) [!].nes */
	{"ec1d596ae83ad93ffd82f07b470633632cf9bad5", 189, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Street Fighter II - The World Warrior (Unl) [a2].nes */
	{"9462a5a066c936135d66d20a9d6ba1537aeb1ed4", 189, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Street Fighter II - The World Warrior (Unl) [!].nes */
	{"771a31ecbf6c743783fda3f8dfe91f70e7e6c033", 189, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Thunder Warrior (Unl).nes */
	{"94003e3c42ae7f582f91c72a58689171872391c9", 189, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 191 ------------------------------------------ */
	/* Sugoro Quest - Dice no Senshitachi (Ch) [o1].nes */
	{"03b517beb21a3dd4556a8ab520994385bbf681d6", 191, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 192 ------------------------------------------ */
	/* You Ling Xing Dong (Ch).nes */
	{"b0a171730179818628fa1ed8cebbaf8c3e690a69", 192, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Young Chivalry (Ch) [b3].nes */
	{"ca3b352c1d2e27ca0d43415b3dfff8d1def0a436", 192, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Young Chivalry (Ch) [!].nes */
	{"9c088fdbc7ba336197b208b88ff1c61fb962e117", 192, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ying Lie Qun Xia Zhuan (Ch).nes */
	{"428f5e052d242605834fcac14af66c22c5093297", 192, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Netdec (FHERO)------------------------------------- */
	/* Fighting Hero (Unl).nes */
	{"aaa96009f28d397e79e3a918918340eaadb85c66", 193, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 195 ------------------------------------------ */
	/* Captain Tsubasa Vol. II - Super Striker (Ch) [a1].nes */
	{"5d68a7c5f3053f052fa9acab1ce3fe181da4c6ba", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain Tsubasa Vol. II - Super Striker (Ch) [a2].nes */
	{"2b053e07fc9030355e73994bb3d8d4b6b05535fc", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain Tsubasa Vol. II - Super Striker (Ch) [a3].nes */
	{"b7256ad224f696eccf78423cd67ef9c1c1cfa72f", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Captain Tsubasa Vol. II - Super Striker (Ch).nes */
	{"baeff77afe81a3b1179091e83ae430d830cc0696", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chaos World (Ch) [a1].nes */
	{"488317d54392c486a72bdb9d5a900e4b5fef15e9", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chaos World (Ch).nes */
	{"e6e51225857dd5c5f15ac9bcfda8d65fa87546ac", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* God Slayer - Haruka Tenkuu no Sonata (Ch).nes */
	{"316235d44b4fdd09acad6da5f7be3fe140d199c9", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zu Qiu Xiao Jiang (Ch) [f1].nes */
	{"ca363262cfaf098a33a17ea2a64f9a1fc709af59", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Zu Qiu Xiao Jiang (Ch).nes */
	{"bf8185a064c9d28765f56919551b6f5449d4417a", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Luo Ke Ren X (Sugoro Quest - Dice no Senshitachi) (Ch).nes */
	{"f007a15c250297e1bd7d6287219fa94928afa759", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 天神之剑: Waixing's Chinese translation of SNK's God Slayer/Crystalis */
	{"5ab3419d0885f50207efa179f6c0c2f289d814b3", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Columbus - Ougon no Yoake (Ch).nes */
	{"4e90457b70e827d8277256ab55938e873ad2da3e", 195, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------------- 196 ------------------------------------------ */
	/* Street Fighter VI 16 Peoples (Unl)[!].nes */
	{"462780b4cbb84364a107b1b37a6a5278eefc6cbf", 196, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Waixing (WTG) ------------------------------------- */
	/* Dragon Ball Z Gaiden - Saiya Jin Zetsumetsu Keikaku (Ch).nes */
	{"bb8676d355ee581b065f5fd9823b96c3b926fc87", 199, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dragon Ball Z II - Gekishin Freeza!! (Ch).nes */
	{"ed203ba48d30f270e9177fc51473c85f5c435608", 199, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* San Guo Zhi 2 (Ch).nes */
	{"ef674f6f7b4ac1a91cab73075f75e2366fa783a8", 199, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Ying Xiong Chuan Qi (Ch).nes */
	{"dbfcc70cc2dec7a889429ed542267f0f1bd967bf", 199, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 200 --------------------------------------- */
	/* 150-in-1 (Multi)[Unknown][Mapper 204].nes */
	{"9f57094ecb761e799e9e339ecb82c5fc2b50649a", 200, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- Namco 3416 --------------------------------------- */
	/* Dragon Buster II - Yami no Fuuin (J) [T+Eng1.0_PentarouZero].nes */
	{"1575a2477c0b9e3884f3141e8c01f56fe2cc9ef0", 206, DEFAULT, DEFAULT, UNK_VERTICAL, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------ Taito X1005 Modified ------------------------------ */
	/* Fudou Myouou Den (J) [o1].nes */
	{"aa4c51a72a5564447ef227dfa56511024cba625e", 207, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fudou Myouou Den (J) [t1].nes */
	{"98d4590497829e8ee75379d5f1851af024209ae6", 207, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 208 --------------------------------------- */
	/* Street Fighter IV (Unl).nes */
	{"d7ec65c41ed88cb449f9afb898d2f2044992af45", 208, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 209 --------------------------------------- */
	/* Power Rangers III (Pirate Original).nes */
	{"3c309516cb3a4b8ced49e1c7a2a7785fd573c6e5", 209, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Power Rangers IV (Pirate Original).nes */
	{"3a5df16b1a08d72d75281e0b950e0fb4084b8a9b", 209, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 210 --------------------------------------- */
	/* Family Circuit '91 (J) [o3].nes */
	{"ae790ab9f5f8a2ba8e3ae24c404cfaf50b03e1ab", 210, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Chibi Maruko-Chan - Uki Uki Shopping (J) [o1].nes */
	{"5f16614de02e6e085f744a5479287156840acefd", 210, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '91 (J) [!].nes */
	{"1328839b0da3cdfa378174b32d6dfe915f59da0f", 210, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '92 (J) [!].nes */
	{"71e5d3d8d8b13f6676e4e810d90bd1333e402bf4", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '93 (J) [!].nes */
	{"4c0a756bc26d3d9b37bc821e138dc3f75951bf8c", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Famista '94 (J).nes */
	{"dad035531eea666db0bf3991fdeaa7795e9c3b56", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Heisei Tensai Bakabon (J) [o1].nes */
	{"e65567d44f1362d73a31401fbec138ac6f10211d", 210, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Heisei Tensai Bakabon (J) [T+Eng_kingmike].nes */
	{"af3a0034dd3fa240f24568fb81ca601122805338", 210, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Splatter House - Wanpaku Graffiti (J) [o2].nes */
	{"33fe79e76f248f894e80c6a0dab5ddd5064c2392", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Splatter House - Wanpaku Graffiti (J) [T+Eng2.0_Spinner8].nes */
	{"16b5f683f63719ce8a4ff2341c0d7746fbfaa78f", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Top Striker (J).nes */
	{"55d7ba8bdb50af005323eb5b10b680dcd76aab1a", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wagyan Land 2 (J) [!].nes */
	{"b9b5b84d5aeaaef306a0d090e718514cf993e35d", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wagyan Land 2 (J) [T+Eng99%_ACP].nes */
	{"258b7e3d22d232f0a69083723fc46f551bab967b", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wagyan Land 3 (J) [!].nes */
	{"1757ee3f24d68cb90b0812b14379d980d3bd8063", 210, 2, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Dream Master (J) [!].nes */
	{"fcc7eb6efd17c9b9d7db24744a1908a587eacc79", 210, 1, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 215 --------------------------------------- */
	/* Mortal Kombat 3 Extra 60 (Unl) [!].nes */
	{"dc1e114d59afcdb2cf2927b830d94f42dd0ea547", 215, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Golden Card 6-in-1 (Unl) [!].nes */
	{"207fa25ae26ff797cae0fd40e7ec4d2c00e52497", 215, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 216 --------------------------------------- */
	/* Videopoker Bonza (Rev A)(R)[!].nes */
	{"e934d390fc7027bbaa30a8a659f98b5a66fac07d", 216, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Videopoker Bonza (Rev B)(R)[!].nes */
	{"3dcad958c7c05a39629780a37e0edf00d8005b9f", 216, DEFAULT, PAL, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 219 --------------------------------------- */
	/* Toy Story (Unl)[!].nes */
	{"c9dc5e47cc22fe1c792ac194de033f8aa8e60a3e", 219, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 224 --------------------------------------- */
	/* Pokemon HeartGold (KT-008 PCB)(Ch)[!].nes */
	{"6545611c3704b8624343f4bdab7c1096523a1ac9", 224, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pokemon Platinum Alt title 1 (KT-008 PCB)(Ch)[!].nes */
	{"d88dfadb681a61dfa36177afbec07364ed6c0011", 224, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pokemon Platinum Alt title 2 (KT-008 PCB)(Ch)[!].nes */
	{"6d57879ec66a523882813bcc0bc69d5bc8ff0ed0", 224, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Pokemon Platinum (KT-008 PCB)(Ch)[!].nes */
	{"7bdc865f0123a643cbfb7645957cf6cc0e30677e", 224, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 225 --------------------------------------- */
	/* 52 Games [p1].nes */
	{"512296fff80e4c6fc12a4c55c1af1854d12ab70d", 225, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 235 --------------------------------------- */
	/* 150-in-1 [a1][p1][!].nes (2MB) */ /* 260-in-1 [p1][b1].nes (4Mb) */
	{"4692a622af6a3d79514bf3e5fb4e9fbe6c6080fc", 235, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 260-in-1 [p1][!].nes (4MB) */
	{"012fd90b213454b76608e018e70f89f1e166dc82", 235, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 238 --------------------------------------- */
	/* Contra Fighter (Unl).nes */
	{"8c525181c1a563beb82e648fa0ed02d8b986b952", 238, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 241 --------------------------------------- */
	/* Xing Ji Zheng Ba (Ch).nes */
	{"3c72706af5998133ec6be703994c10466a094eab", 241, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Education Computer 26-in-1 (R)[!].nes */
	{"eea726f679d364e6805d48f923ddd21fe14d9f43", 241, DEFAULT, DENDY, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Study and Game 32-in-1 [!].nes */
	{"c073cd6389e373cbb3433f81196ece4d09c33bef", 241, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fan Kong Jing Ying (Ch).nes */
	{"bfa31777e077e64af0e274b5a22b57c6765d36e1", 241, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 242 --------------------------------------- */
	/* Dragon Quest VIII (ES-1077) (Ch) [!].nes */
	{"5d9d519b84e245891928f3482a42a071e4402b6c", 242, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wai Xing Zhan Shi (Ch) [a1].nes */
	{"04a483b692f834757a60e17a3f3cdeb44ed244ca", 242, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Wai Xing Zhan Shi (Ch).nes */
	{"9bdff9a19265d84979f43f0f6e925a735ddeb38b", 242, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ----------------------------------- Waixing (WTH) -------------------------------------- */
	/* Ying Xiong Yuan Yi Jing Chuan Qi (Ch).nes */
	{"c4df1dff6ad1f6194bff1e5333073edf0c178d7d", 245, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Yong Zhe Dou E Long - Dragon Quest V (Ch) [b1].nes */
	{"23ddca575b3f1127b8a27991374bb42b7a65269a", 245, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Yong Zhe Dou E Long - Dragon Quest V (Ch) [b2].nes */
	{"20a4fdc7939b7585b0d83eb956ba07a99c6257c3", 245, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Yong Zhe Dou E Long - Dragon Quest V (Ch) [o1].nes */
	{"ff645f86382012c203b21b42ecb5c34df6fe191e", 245, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Yong Zhe Dou E Long - Dragon Quest VI (Ch) [a1].nes */
	{"0fffcf24e8708775a764eba4c38354a457ffba2f", 245, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Yong Zhe Dou E Long - Dragon Quest VII (Ch) [b1].nes */
	{"2d03f3105af3149cc070f7a72b868660cd905304", 245, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 246 --------------------------------------- */
	/* Fong Shen Bang - Zhu Lu Zhi Zhan (Ch) [a1].nes */
	{"cfd1a846d35925066c6e2e7e8dbaafd3d17a61b9", 246, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fong Shen Bang - Zhu Lu Zhi Zhan (Ch) [a2].nes */
	{"cf7d9c2cbe6953f4fd15fc5b59ee19944914dc4e", 246, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Fong Shen Bang - Zhu Lu Zhi Zhan (Ch) [T+Chi(Simp)20060423].nes */
	{"991968732e6f4f2289f8e751539b02b8a9d4853f", 246, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 254 --------------------------------------- */
	/* 魂斗罗7加难无敌版.nes */
	{"f2fbaeae5f99d5128760946491f9ea7670287c04", 254, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 256 --------------------------------------- */
	/* 2-in-1- Street Dance + Hit Mouse (Multi)[Feng Li][!].nes */
	{"df00e70af3c77b28fea955b5518b27f54a120ed7", 256, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 260 --------------------------------------- */
	/* 2-in-1- Aladdin + Lion King, The (Multi)[Unknown][!].nes */
	{"f7c23ea5cdabdb89af08424cc36962ed5fbd3f37", 260, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 2-in-1- Boogerman + Flintstones, The (Multi)[Unknown][!].nes */
	{"f1a419af5d89c9c44b36efc8e5b4d9a81d3a56e8", 260, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 2-in-1- Mortal Kombat V Turbo 30 + Super Aladdin (Multi)[Unknown][!].nes */
	{"1f721cd383219f39519245581052d3bfda912541", 260, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 4-in-1 (Multi)[Unknown][Harry Potter].nes */
	{"0a1876bb76ffbcff65b350379474c3af9ac91964", 260, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 4-in-1 (Multi)[Unknown][YH4222].nes */
	{"ef70467cee09adbbd7ed37ac7501f017a84f19bf", 260, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------------- SHERO ----------------------------------------- */
	/* Street Heroes (Asia) (Unl).nes */
	{"9500c52045909a239c21f15cdeda1782c7303f1e", 262, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ---------------------------------------- KOF97 ----------------------------------------- */
	/* Boogerman II - The Final Adventure (Rex-Soft) [!].nes */
	{"28805e433ca0ad1505058085667b889a8a7b967f", 263, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- BMC-22026 ---------------------------------------- */
	/* 4-in-1 (MGC-026, TXC, Idea-Tek).nes */
	{"a612291ad4cd3a9a4dc8a3ff511a7c3f4979d0af", 271, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 291 --------------------------------------- */
	/* 2-in-1- Mortal Kombat VI + Mortal Kombat VII (Multi)[Unknown][NT-639].nes */
	{"4478cd97d34a1e5b7e0498851b36d6afa8172a76", 291, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* ------------------------------------- 3D-BLOCK ----------------------------------------- */
	/* 3D Block (Hwang Shinwei) [!].nes */
	{"56cba51568b0f1458aa7e0de6575d1225e40f7a1", 355, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Block Force.nes [nes2:0] */
	{"d326485501ac08b5c3815bfd8dff3979c97556c6", 355, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Block Force (Hwang Shinwei).nes */
	{"5de850b4e28c49ff9ed53def2a335d29b09387c2", 355, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 366 --------------------------------------- */
	/* 4-in-1 (Multi)[Unknown][K-3131GS, GN-45][!].nes */
	{"ef17c86e521110637b443437fca269f2e3f2385f", 366, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 4-in-1 (Multi)[Unknown][K-3131SS, GN-45][!].nes */
	{"354116730efa2131172ccca3793a065abee6026e", 366, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 512 --------------------------------------- */
	/* Chuugoku Taitei (Sachen).nes */
	{"32436a43d3c371ab7aa6bb5165f60ca5c006a60e", 512, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 518 --------------------------------------- */
	/* 5-in-1 (Multi)[Unknown][Subor 999].nes */
	{"95c1482141e5b0c2e304a872b333f28fca32c360", 518, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* 8-in-1 (Multi)[Unknown][Subor v5.0].nes */
	{"e92aeb559db317cfe4902a535b9bf6d73866c1d0", 518, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------------- 534 --------------------------------------- */
	/* 2-in-1 (Multi)[Unknown][Shu Du + Wu Zi Qi].nes */
	{"7c3be2e58853f9cb3201ec7a3719a43b8d47e71c", 534, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},

/* -------------------------------------- VS System --------------------------------------- */
	/* Battle City (VS) [p1][o1].nes */
	{"3e9a70cb6e7118b1a3b5fb5354679a9fdfe567a0", 0, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, EXP_VS_1P_R4017},
	/* VS. Slalom (VS).nes */
	{"88450e1e392529ff53c6c730c6c622d89a6d12f9", 0, DEFAULT, DEFAULT, DEFAULT, RP2C04_0002, DEFAULT, NOEXTRA},

	/* Dr. Mario (VS).nes */
	{"cd74c3a7a2fc7c25420037ae5f4a25307aff6587", 1, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, DEFAULT, EXP_VS_1P_R4017},

	/* Top Gun (VS).nes */
	{"d9b1b87204e025a637821a0168475e1209ce0c8a", 2, DEFAULT, DEFAULT, DEFAULT, RC2C05_04, DEFAULT, NOEXTRA},
	/* VS. Castlevania (VS) [b1].nes */
	{"ef5fc47ce468457846b64309312cfe7a68ed6280", 2, DEFAULT, DEFAULT, UNK_VERTICAL, RP2C04_0002, DEFAULT, NOEXTRA},
	/* VS. Castlevania (VS) [o1].nes */
	{"9eb3b75e7b45df51b8bcd29df84689a7e8557f4f", 2, DEFAULT, DEFAULT, UNK_VERTICAL, RP2C04_0002, DEFAULT, NOEXTRA},

	/* VS. Super Mario Bros. (VS) [b1][o1].nes */
	{"97212c3a49ff09e74d11392dfe1f71fadc410638", 3, DEFAULT, DEFAULT, DEFAULT, RC2C03B, DEFAULT, NOEXTRA},

	/* Atari RBI Baseball (VS).nes */
	{"e090b0aec98463c565e300a910561499d8bd9676", 4, DEFAULT, DEFAULT, DEFAULT, RP2C04, VS_SM_RBI_Baseball, EXP_VS_1P_R4017},
	/* Freedom Force (VS).nes */
	{"59602d7883ebbfbf17b10af5f30eefbad2d7b2c4", 4, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, NOEXTRA},
	/* Super Sky Kid (VS).nes */
	{"e4a3d2a223f066c231631d92504f08e60b303dfd", 4, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, EXP_VS_1P_R4017},
	/* VS. TKO Boxing (VS) [a1].nes */
	{"8d6e756b1cf39487dfd00538ce7b25769233998d", 4, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, VS_SM_TKO_Boxing, NOEXTRA},
	/* VS. TKO Boxing (VS) [!].nes */
	{"257c9f3565ff1d136094e99636ca57e300352b7e", 4, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, VS_SM_TKO_Boxing, NOEXTRA},
	/* VS. TKO Boxing (VS) [hM04].nes */
	{"3d0d0822fc93e577996b82de584e48d85b9e6509", 100, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, DEFAULT, NOEXTRA},

	/* Platoon (VS).nes */
	{"1f8e33d6da8402be6a376668a424bfde38471021", 68, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, NOEXTRA},

	/* Battle City (VS) [p2].nes */
	{"ed8a401dfb6646fcae97a54b961390bbe2227db8", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, EXP_VS_1P_R4017},
	/* Clu Clu Land (VS).nes */
	{"1b80b90904dc4f87f9168dea6b636a27e8006e22", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, EXP_VS_1P_R4017},
	/* Duck Hunt (VS).nes */
	{"1166d65866b39a889a9f9c58400119eba83fa214", 99, DEFAULT, DEFAULT, DEFAULT, RC2C03B, DEFAULT, EXP_VS_ZAPPER},
	/* Excitebike (VS) [a1].nes */
	{"d4c8eb6d216585c742d0f7acd029c5218f4b3cb2", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, NOEXTRA},
	/* Excitebike (VS).nes */
	{"611322f482ca236c42c7325d540df7c906c1c330", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, DEFAULT, NOEXTRA},
	/* Gumshoe (VS) [b1].nes */
	{"946d3714b46253386bee068065b7161fdfaa8bc3", 99, DEFAULT, DEFAULT, DEFAULT, RC2C05_03, DEFAULT, EXP_VS_ZAPPER},
	/* Gumshoe (VS) [b2].nes */
	{"4882cdf87a1e170f7a0bfda3444cd8cf51af3837", 99, DEFAULT, DEFAULT, DEFAULT, RC2C05_03, DEFAULT, EXP_VS_ZAPPER},
	/* Gumshoe (VS).nes */
	{"9f0a314f05955c8d388a5e698cc824a20930258e", 99, DEFAULT, DEFAULT, DEFAULT, RC2C05_03, DEFAULT, EXP_VS_ZAPPER},
	/* Hogan's Alley (VS).nes */
	{"754762b9322267013f584603c0df8fc533705789", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, EXP_VS_ZAPPER},
	/* Ice Climber (VS) [o1].nes */
	{"56fab1e86b8fb7c1547618f596d6bedcba4fd622", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, EXP_VS_1P_R4017},
	/* Ladies Golf (VS).nes */
	{"ceb206b0af6c28053a38181efa00f6ccf178d2c9", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0002, DEFAULT, NOEXTRA},
	/* Mach Rider (VS) [b1].nes */
	{"f3cdb2f9f3cbc5a4107528b3a152807b7d1e1fcd", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0002, DEFAULT, NOEXTRA},
	/* Mach Rider (VS).nes */
	{"7aac543c763a6c1a865341c356fde772691f0ded", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0002, DEFAULT, NOEXTRA},
	/* Mighty Bomb Jack (J) (VS).nes */
	{"1c643b9603c36ce914bf4214e9f38fa52446d722", 99, DEFAULT, DEFAULT, DEFAULT, RC2C05_02, DEFAULT, NOEXTRA},
	/* Ninja Jajamaru-kun (VS).nes */
	{"d323fd1888b5e0aab9444384fe66af19fd1b119f", 99, DEFAULT, DEFAULT, DEFAULT, RC2C05_01, DEFAULT, EXP_VS_1P_R4017},
	/* Pinball (VS) [a1].nes */
	{"c5d86eb0a0ca8cb1ba96a8035dde554c4fa462bb", 99, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Pinball (VS).nes */
	{"ca8a80135a1cc6e609fea8f7ead20fbb56138478", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, NOEXTRA},
	/* Soccer (VS).nes */
	{"ec1bea24b1f7e5a86006760edf12b00a9922b3f5", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, DEFAULT, EXP_VS_1P_R4017},
	/* Star Luster (VS).nes */
	{"35602296fdaf4dcdf6356f38c6897cf7f1f03f2a", 99, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Stroke & Match Golf (VS) [a1].nes */
	{"62cdadf72c39e565181e1e7388a57aa7710f4f83", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0002, DEFAULT, NOEXTRA},
	/* Stroke & Match Golf (VS) [!].nes */
	{"8b0ea39aa9b3cfde4dcd376959dade1d79618c4d", 99, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Tetris (VS) [a1].nes */
	{"83f26eb28474806561a3e5b7c1266c6f138f801b", 99, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Tetris (VS).nes */
	{"810442b860bcef2fd54f9c76f3b8058c9a769b15", 99, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Vs. Battle Shitty by Jackass (Hack).nes */
	{"6be8870249d8b1a80388352bbf8c7a46dccc40ff", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, NOEXTRA},
	/* VS. Blink (SMB1 VS Hack) [o1].nes */
	{"f3e834ed58e78f7ccfa902ac05fc3b81b68da757", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, NOEXTRA},
	/* VS. Super Mario Bros. (VS) [a1].nes */
	{"a9f381da57af4548f456fa47b1d6e31c337b76c3", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, NOEXTRA},
	/* VS. Super Mario Bros. (VS) [b3].nes */
	{"38a0751a5a41637f4f674aaf1c637894334f15c6", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, NOEXTRA},
	/* VS. Super Mario Bros. (VS) [o1].nes */
	{"f25ab9354e24e3cd99b6cfd8331f9be40b603c5b", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, NOEXTRA},

	/* Goonies, The (VS) [b2].nes */
	{"1e78a8ddc878aa27aab466e969e76687cd82be51", 151, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, DEFAULT, NOEXTRA},
	/* Goonies, The (VS) [o1].nes */
	{"ac187904c125e56a71acff979e53f3398a05c075", 151, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, DEFAULT, NOEXTRA},
	/* Gradius (VS) [b1].nes */
	{"36bd4cb34d6d2c3ec8f387ee0d6f1ce9e9ac4aa1", 151, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, EXP_VS_1P_R4017},
	/* Gradius (VS).nes */
	{"b26efe78798453a903921723f3c9ac69f579b7d2", 151, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, EXP_VS_1P_R4017},

	/* Super Xevious - Gump no Nazo (VS) [b1].nes */
	//{"e0e9b84585b2b217d8ca3af23e6e935e26d1427f", 1, DEFAULT, DEFAULT, DEFAULT, RP2C04, VS_SM_Super_Xevious, NOEXTRA},
	/* Super Xevious - Gump no Nazo (VS) [b2].nes */
	//{"c88eb9d327f1ec3a6e24e28fd6332da2e8f2e6a4", 1, DEFAULT, DEFAULT, DEFAULT, RP2C04, VS_SM_Super_Xevious, NOEXTRA},
	/* Super Xevious - Gump no Nazo (VS).nes */
	{"f169152ca55b93d4bc59e46b977f468e9461af18", 206, DEFAULT, DEFAULT, DEFAULT, RP2C04, VS_SM_Super_Xevious, NOEXTRA},

	/* arbib.nes */
	//{"bd7a98e6f4f5110aed84c7729d6530d9e7243486", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04, DEFAULT, NOEXTRA},

	/* Baseball (VS) (Player 1 Mode) [a1].nes */
	//{"01c426fa005426956ef45026a2259d5e736a9dc4", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Baseball (VS) (Player 1 Mode) [a2].nes */
	//{"e4c47bca95427b719eac3aa6b92e357417eb98ae", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Baseball (VS) (Player 1 Mode).nes */
	//{"21f5a8e77f4c1556d71d85d3e0e2d71a8586b720", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Baseball (VS) (Player 2 Mode) [a1].nes */
	//{"224a2750c4dfb21a86896d26064db5c0d9071319", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Baseball (VS) (Player 2 Mode) [a2].nes */
	//{"5ec203a2fca2038978b5736fa60513f1427121c2", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Baseball (VS) (Player 2 Mode).nes */
	//{"c2e9fae32a826bf1fa85d2dd22dea4da8edfad7c", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Ice Climber (VS) (Player 1 Mode).nes */
	//{"8b46c777384eee064a8b12d498e1976fb3370a40", 0, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, NOEXTRA},
	/* Ice Climber (VS) (Player 2 Mode).nes */
	//{"dc7cc7ee2305595601dc42f11e9babd5babddf7c", 0, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, NOEXTRA},
	/* Mahjong (VS) (Player 1 Mode).nes */
	//{"7dc8e55c2e19b09b0f1348534f0b5e826886e3d3", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Mahjong (VS) (Player 2 Mode).nes */
	//{"8fe3e85f13572aa6d307147cabc0f618f2fedb88", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Tennis (VS) (Player 2 Mode).nes */
	//{"0ccc27da40737d0e1b8ab73f2e85d6283149147b", 0, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Wrecking Crew (VS) (Player 1 Mode).nes */
	//{"2bb5fc42b8af61ba7aba73b664b5d5b3dc03ad1c", 0, DEFAULT, DEFAULT, DEFAULT, RP2C04_0002, DEFAULT, NOEXTRA},
	/* Wrecking Crew (VS) (Player 2 Mode).nes */
	//{"bd504eb19bca051650b4c5f59f9f757ea46a5c7f", 0, DEFAULT, DEFAULT, DEFAULT, RP2C04_0002, DEFAULT, NOEXTRA},
	/* Balloon Fight (VS) [!].nes */
	//{"e26ee82718e48f35985fb2771eac18bc244a65f1", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0003, DEFAULT, NOEXTRA},
	/* Baseball (VS) [b1].nes */
	//{"c2e9fae32a826bf1fa85d2dd22dea4da8edfad7c", 99, DEFAULT, DEFAULT, DEFAULT, RP2C04_0004, DEFAULT, NOEXTRA},
	/* Mahjong (VS) [a1].nes */
	//{"db213a983ed32111ae6bdb4642cd036446841d08", 99, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Mahjong (VS) [!].nes */
	//{"4573541dfd369d8ebe27b60727096c64355872d2", 99, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},
	/* Tennis (VS) [o1].nes */
	//{"aaa62708b65196f3d0222cc5896ab47ed4b155cf", 99, DEFAULT, DEFAULT, DEFAULT, RP2C03B, DEFAULT, NOEXTRA},

/* -------------------------------------- Game Genie -------------------------------------- */
	/* Game Genie Decoder by Chris Covell (PD).nes */
	{"8939d894f7569f9fadf2a04098010292662b14fd", GAMEGENIE_MAPPER, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Game Genie (Unl) (V1.5A) [o2].nes */
	{"8bb0d98c4515cba77a805bc8b06788dd6ed6263c", GAMEGENIE_MAPPER, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
	/* Game Genie (Unl) (V1.5A) [o3].nes */
	{"39ee2d598b084c88de7ac467ad65e4049c3db5e4", GAMEGENIE_MAPPER, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA},
};

#endif /* DATABASE_H_ */
