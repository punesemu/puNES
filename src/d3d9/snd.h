/*
 * snd.h
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#ifndef SND_H_
#define SND_H_

#include "common.h"

BYTE snd_init(void);
BYTE snd_start(void);
void snd_output(void *udata, BYTE *stream, int len);
void snd_stop(void);
void snd_quit(void);

void (*snd_apu_tick)(void);
void (*snd_end_frame)(void);

#endif /* SND_H_ */
