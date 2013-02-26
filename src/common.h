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

#ifndef FALSE
enum { FALSE, TRUE };
#endif
enum { EXIT_OK, EXIT_ERROR };
enum { LOWER, UPPER };
enum { READ, WRITE };
enum { AUTO, NTSC, PAL, DENDY, DEFAULT = 255 };
enum { APU60HZ, APU48HZ };
/* i vari mirroring */
enum {
	HORIZONTAL,
	VERTICAL,
	SINGLESCR0,
	SINGLESCR1,
	FOURSCR,
	SCR0x1_SCR1x3,
	SCR0x3_SCR1x1
};
/* i tipi di reset */
enum {
	RESET      = 0x10,
	HARD       = 0x20,
	CHANGEROM  = 0x30,
	CHANGEMODE = 0x40,
	POWERUP    = 0x50
};
/* le dimesioni dello screen da renderizzare */
enum { SCR_LINES = 240, SCR_ROWS = 256 };
/* le modalita' del colore possibili con la PPU */
enum { GRAYSCALE = 0x30, NORMAL = 0x3F };

#define LENGTH(x) (sizeof(x)/sizeof(*(x)))

#ifdef DEBUG
#define INLINE
#else
#define INLINE __attribute__((always_inline))
#endif

struct _info {
	char base_folder[1024];
	char rom_file[1024];
	char load_rom_file[1024];
	BYTE machine;
	BYTE machine_db;
	WORD mapper;
	BYTE mapper_type;
	BYTE mapper_extend_wr;
	BYTE mapper_extend_rd;
	BYTE portable;
	BYTE id;
	BYTE trainer;
	BYTE stop;
	BYTE reset;
	BYTE execute_cpu;
	BYTE gui;
	BYTE no_rom;
	BYTE pause;
	BYTE on_cfg;
	BYTE pause_frames_drawscreen;
	BYTE first_illegal_opcode;
	BYTE sha1sum[20];
	char sha1sum_string[41];
	BYTE sha1sum_chr[20];
	char sha1sum_string_chr[41];
	WORD chr_rom_8k_count;
	WORD chr_rom_4k_count;
	WORD chr_rom_1k_count;
	WORD prg_rom_16k_count;
	WORD prg_rom_8k_count;
	BYTE prg_ram_plus_8k_count;
	BYTE prg_ram_bat_banks;
	BYTE prg_ram_bat_start;

	BYTE r4016_dmc_double_read_disabled;
	BYTE r2002_race_condition_disabled;
	BYTE r4014_precise_timing_disabled;
} info;

#endif /* COMMON_H_ */
