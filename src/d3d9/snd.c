/*
 * snd.c
 *
 *  Created on: 01/mar/2013
 *      Author: fhorse
 */

#include "snd.h"
#include "emu.h"

BYTE snd_init(void) {
	snd_apu_tick = NULL;
	snd_end_frame = NULL;

	return (EXIT_OK);
}
BYTE snd_start(void) {
	return (EXIT_OK);
}
void snd_output(void *udata, BYTE *stream, int len) {
	return;
}
void snd_stop(void) {
	return;
}
void snd_quit(void) {
	return;
}
