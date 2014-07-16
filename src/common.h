/*
 * common.h
 *
 *  Created on: 29/mar/2010
 *      Author: fhorse
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DBWORD;
typedef signed char SBYTE;
typedef signed short SWORD;
typedef signed int SDBWORD;

#define BIOS_FOLDER    "/bios"
#define DIFF_FOLDER    "/diff"
#define PERGAME_FOLDER "/pgs"
#define SAVE_FOLDER    "/save"
#define TMP_FOLDER     "/tmp"
#define PRB_FOLDER     "/prb"

#if !defined (FALSE)
enum false_value { FALSE, TRUE };
#endif
enum exit_type { EXIT_OK, EXIT_ERROR };
enum lower_value { LOWER, UPPER };
enum machine_mode { AUTO, NTSC, PAL, DENDY, DEFAULT = 255 };
enum reset_type {
	RESET       = 0x10,
	HARD        = 0x20,
	CHANGE_ROM  = 0x30,
	CHANGE_MODE = 0x40,
	POWER_UP    = 0x50
};
/* le dimesioni dello screen da renderizzare */
enum screen_dimension { SCR_LINES = 240, SCR_ROWS = 256 };
enum type_of_system_info { HEADER, DATABASE };
enum header_type { iNES_1_0, NES_2_0, UNIF_FORMAT, FDS_FORMAT };
enum lenght_file_name_type {
	LENGTH_FILE_NAME      = 512,
	LENGTH_FILE_NAME_MID  = 1024,
	LENGTH_FILE_NAME_LONG = 2048
};
enum forced_mirroring { UNK_HORIZONTAL, UNK_VERTICAL };
enum max_chips_rom { MAX_CHIPS = 8 };

#define LENGTH(x) (sizeof(x)/sizeof(*(x)))

#if defined (DEBUG)
#define INLINE
#else
#define INLINE inline __attribute__((always_inline))
#endif

struct _info {
	char base_folder[LENGTH_FILE_NAME_MID];
	char rom_file[LENGTH_FILE_NAME_MID];
	char load_rom_file[LENGTH_FILE_NAME_MID];
	BYTE format;
	BYTE machine[2];
	struct _info_mapper {
		WORD id;
		BYTE submapper;
		BYTE extend_wr;
		BYTE extend_rd;
	} mapper;
	BYTE mirroring_db;
	BYTE portable;
	BYTE id;
	BYTE trainer;
	BYTE stop;
	BYTE reset;
	BYTE execute_cpu;
	BYTE gui;
	BYTE no_rom;
	BYTE uncompress_rom;
	BYTE pause;
	BYTE on_cfg;
	BYTE pause_frames_drawscreen;
	BYTE first_illegal_opcode;
	struct _info_sh1sum {
		struct _info_sha1sum_prg {
			BYTE value[20];
			char string[41];
		} prg;
		struct _info_sha1sum_chr {
			BYTE value[20];
			char string[41];
		} chr;
	} sha1sum;
	struct _info_chr {
		struct _info_chr_rom {
			WORD banks_8k;
			WORD banks_4k;
			WORD banks_1k;
			struct _info_chr_rom_max {
				WORD banks_8k;
				WORD banks_4k;
				WORD banks_2k;
				WORD banks_1k;
			} max;
		} rom;
	} chr;
	struct _info_prg {
		struct _info_prg_rom {
			WORD banks_16k;
			WORD banks_8k;
			struct _info_prg_rom_max {
				WORD banks_32k;
				WORD banks_16k;
				WORD banks_8k;
				WORD banks_8k_before_last;
				WORD banks_4k;
			} max;
		} rom;
		struct _info_prg_ram {
			BYTE banks_8k_plus;
			struct _info_prg_ram_bat {
				BYTE banks;
				BYTE start;
			} bat;
		} ram;
	} prg;
	BYTE r4016_dmc_double_read_disabled;
	BYTE r2002_race_condition_disabled;
	BYTE r4014_precise_timing_disabled;
} info;

#endif /* COMMON_H_ */
