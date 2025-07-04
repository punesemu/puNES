/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include "unicode_def.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DBWORD;
typedef signed char SBYTE;
typedef signed short SWORD;
typedef signed int SDBWORD;

#define FHMAX(a, b) (((a) > (b)) ? (a) : (b))
#define FHMIN(a, b) (((a) < (b)) ? (a) : (b))

#define CFGFILENAME "/puNES.cfg"
#define INPFILENAME "/input.cfg"
#define RECENTROMSFILENAME "/recent.cfg"
#define RECENTDISKSFILENAME "/disks.cfg"

#define CHEAT_FOLDER   "/cheat"
#define PERGAME_FOLDER "/pgs"
#define SHDPAR_FOLDER  "/shp"
#define JSC_FOLDER     "/jsc"

#define BIOS_FOLDER    "/bios"
#define DIFF_FOLDER    "/diff"
#define PRB_FOLDER     "/prb"
#define SAVE_FOLDER    "/save"
#define SCRSHT_FOLDER  "/screenshot"

#if !defined (FALSE)
enum false_value { FALSE, TRUE };
#endif
enum exit_type { EXIT_OK, EXIT_ERROR };
enum lower_value { LOWER, UPPER };
enum machine_mode { AUTO, NTSC, PAL, DENDY, DEFAULT = 255 };
enum initial_ram_value_type { IRV_0X00, IRV_0XFF, IRV_RANDOM };
enum overcan_type { PERGAME_OFF, PERGAME_ON, PERGAME_DEFAULT };
enum console_type {
	REGULAR_NES,
	VS_SYSTEM,
	PLAYCHOICE10,
	FAMICLONE_DECIMAL_MODE,
	EPSM,
	VT01,
	VT02,
	VT03,
	VT09,
	VT32,
	VT369,
	UMC_UM6578,
	FAMICOM_NETWORK_SYSTEM
};
enum reset_type {
	RESET       = 0x10,
	HARD        = 0x20,
	CHANGE_ROM  = 0x30,
	CHANGE_MODE = 0x40,
	POWER_UP    = 0x50
};
enum _nes_chips_info {
	NES_CHIPS_MAX = 2
};
/* le dimesioni dello screen da renderizzare */
enum screen_dimension { SCR_ROWS = 240, SCR_COLUMNS = 256 };
enum type_of_system_info { HEADER, DATABASE };
enum header_type { iNES_1_0, NES_2_0, UNIF_FORMAT, FDS_FORMAT, QD_FORMAT, NSF_FORMAT, NSF2_FORMAT, NSFE_FORMAT, HEADER_UNKOWN };
enum length_file_name_type {
	LENGTH_FILE_NAME      = 512,
	LENGTH_FILE_NAME_MID  = 1024,
	LENGTH_FILE_NAME_LONG = 2048,
	LENGTH_FILE_NAME_MAX  = 4096
};
enum forced_mirroring { UNK_HORIZONTAL, UNK_VERTICAL };
enum languages {
	LNG_ENGLISH,
	LNG_ITALIAN,
	LNG_RUSSIAN,
	LNG_SPANISH,
	LNG_HUNGARIAN,
	LNG_TURKISH,
	LNG_PORTUGUESEBR,
	LNG_CHINESE_SIMPLIFIED,
	LNG_FRENCH,
	LNG_GERMAN,
	LNG_POLISH,
	LNG_ARABIC
};
enum epansions_device_mode {
	NOEXTRA = 0x0000,
	EXP_UNKNOW = NOEXTRA,
	EXP_STANDARD,
	EXP_FOUR_SCORE,
	EXP_FAMICOM_FOUR,
	EXP_VS_1P_R4016,
	EXP_VS_1P_R4017,
	EXP_RESERVED,
	EXP_VS_ZAPPER,
	EXP_ZAPPER_R4017,
	EXP_TWO_ZAPPER,
	EXP_BANDAI_LIGHTGUN,
	EXP_POWER_PAD_A,
	EXP_POWER_PAD_B,
	EXP_FAMILY_TRAINER_A,
	EXP_FAMILY_TRAINER_B,
	EXP_ARKANOID_VAUS_NES,
	EXP_ARKANOID_VAUS_FAMICOM,
	EXP_TWO_VAUS_PLUS_DATA_RECORDER,
	EXP_KONAMY_HYPER_SHOT,
	EXP_COCONUTS_PACHINKO,
	EXP_EXCITING_BOXING,
	EXP_JISSEN_MAHJONG,
	EXP_PARTY_TAP,
	EXP_OEKA_KIDS,
	EXP_SUNSOFT_BARCODE,
	EXP_MIRACLE_PIANO,
	EXP_POKKUN_MOHURAA,
	EXP_TOP_RIDER,
	EXP_DOUBLE_FISTED,
	EXP_FAMICOM_3D,
	EXP_DOREMIKKO_KEYBOARD,
	EXP_ROB_GYRO,
	EXP_FAMICOM_DATA_RECORDER,
	EXP_ASCII_TURBO_FILE,
	EXP_IGS_STORAGE,
	EXP_FAMILY_BASIC_KEYOBARD_PLUS_DATA_RECORDER,
	EXP_DONGDA_PEC586_KEYOBARD,
	EXP_BITCORP_BIT79_KEYOBARD,
	EXP_SUBOR_KEYOBARD,
	EXP_SUBOR_KEYOBARD_PLUS_MOUSE_3X8BIT,
	EXP_SUBOR_KEYOBARD_PLUS_MOUSE_24BIT_R4016,
	EXP_SNES_MOUSE_R4017,
	EXP_MULTICART,
	EXP_TWO_SNES_CONTROLLERS,
	EXP_RACERMATE_BICYCLE,
	EXP_UFORCE,
	EXP_ROB_STACKUP,
	EXP_CITY_PATROLMAN_LIGHTGUN,
	EXP_SHARP_C1_CASSETTE,
	EXP_STANDARD_SWAPPED,
	EXP_EXCALIBUR_SUDOKU,
	EXP_ABL_PINBALL,
	EXP_GOLDEN_NUGGET_CASINO,
	EXP_UNKNOW_FAMICLONE_KEYBOARD,
	EXP_SUBOR_KEYBOARD_PLUS_MOUSE_24BIT_R4017,
	EXP_PORT_TEST,
	EXP_BANDAI_MULTI_GAME_GAMEPAD,
	EXP_VENOM_TV_DANCE_MAT,
	EXP_LG_TV_REMOTE
};
enum toolbar { TLB_TOP, TLB_RIGHT, TLB_BOTTOM, TLB_LEFT };

#define LENGTH(x) (sizeof(x)/sizeof(*(x)))
#define UNUSED(var) var __attribute__((unused))

#if defined (DEBUG)
#define INLINE
#else
#define INLINE inline __attribute__((always_inline))
#endif

#endif /* COMMON_H_ */
