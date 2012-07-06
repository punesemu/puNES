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

BYTE emuLoop(void);
BYTE emuMakeDir(char *path);
BYTE emuLoadRom(void);
BYTE emuSearchInDatabase(FILE *fp);
void emuSetTitle(char *title);
BYTE emuTurnON(void);
void emuPause(BYTE mode);
BYTE emuReset(BYTE type);
void emuQuit(BYTE exitCode);

#endif /* EMU_H_ */

