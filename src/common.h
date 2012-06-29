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

#define BIOSFOLDER    "/bios"
#define DIFFFOLDER    "/diff"
#define PERGAMEFOLDER "/pgs"
#define SAVEFOLDER    "/save"

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
enum { SCRLINES = 240, SCRROWS = 256 };
/* le modalita' del colore possibili con la PPU */
enum { GRAYSCALE = 0x30, NORMAL = 0x3F };

#define LENGTH(x) (sizeof(x)/sizeof(*(x)))

#ifdef DEBUG
#define INLINE
#else
#define INLINE __attribute__((always_inline))
#endif

struct _info {
	char baseFolder[1024];
	char romFile[1024];
	char loadRomFile[1024];
	BYTE machine;
	BYTE machineDb;
	WORD mapper;
	BYTE mapperType;
	BYTE mapperExtendWrite;
	BYTE mapperExtendRead;
	BYTE portables;
	BYTE id;
	BYTE trainer;
	BYTE stop;
	BYTE reset;
	BYTE executeCPU;
	BYTE gui;
	BYTE no_rom;
	BYTE pause;
	BYTE pause_frames_drawscreen;
	BYTE first_illegal_opcode;
	BYTE sha1sum[20];
	char sha1sumString[41];
	BYTE sha1sumChr[20];
	char sha1sumStringChr[41];
	WORD chrRom8kCount;
	WORD chrRom4kCount;
	WORD chrRom1kCount;
	WORD prgRom16kCount;
	WORD prgRom8kCount;
	BYTE prgRamPlus8kCount;
	BYTE prgRamBatBanks;
	BYTE prgRamBatStart;

	BYTE r4016_dmc_double_read_disabled;
	BYTE r2002_race_condition_disabled;
	BYTE r4014_precise_timing_disabled;
} info;

#endif /* COMMON_H_ */

