/*
 * emu.h
 *
 *  Created on: 27/mar/2010
 *      Author: fhorse
 */

#ifndef EMU_H_
#define EMU_H_

#include <stdio.h>
#include "common.h"

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE emu_loop(void);
EXTERNC BYTE emu_make_dir(const char *fmt, ...);
EXTERNC BYTE emu_file_exist(const char *file);
EXTERNC BYTE emu_load_rom(void);
EXTERNC BYTE emu_search_in_database(FILE *fp);
EXTERNC void emu_set_title(char *title);
EXTERNC BYTE emu_turn_on(void);
EXTERNC void emu_pause(BYTE mode);
EXTERNC BYTE emu_reset(BYTE type);
EXTERNC WORD emu_round_WORD(WORD number, WORD round);
EXTERNC void emu_quit(BYTE exit_code);

#undef EXTERNC

#endif /* EMU_H_ */

