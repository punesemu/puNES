/*
 * common.h
 *
 *  Created on: 17/lug/2014
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
	LENGTH_FILE_NAME_LONG = 2048,
	LENGTH_FILE_NAME_MAX = 4096
};
enum forced_mirroring { UNK_HORIZONTAL, UNK_VERTICAL };
enum max_chips_rom { MAX_CHIPS = 8 };
enum languages { LANG_ENGLISH, LANG_ITALIAN, LANG_RUSSIAN };

#define LENGTH(x) (sizeof(x)/sizeof(*(x)))

#if defined (DEBUG)
#define INLINE
#else
#define INLINE inline __attribute__((always_inline))
#endif

#endif /* COMMON_H_ */
