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

BYTE emu_loop(void);
BYTE emu_make_dir(char *path);
BYTE emu_load_rom(void);
BYTE emu_search_in_database(FILE *fp);
void emu_set_title(char *title);
BYTE emu_turn_on(void);
void emu_pause(BYTE mode);
BYTE emu_reset(BYTE type);
void emu_quit(BYTE exit_code);

#endif /* EMU_H_ */

