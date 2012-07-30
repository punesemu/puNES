/*
 * fps.h
 *
 *  Created on: 15/apr/2011
 *      Author: fhorse
 */

#ifndef FPS_H_
#define FPS_H_

#include "common.h"

enum {
	FPSDEFAULT,
	FPS60,
	FPS59,
	FPS58,
	FPS57,
	FPS56,
	FPS55,
	FPS54,
	FPS53,
	FPS52,
	FPS51,
	FPS50,
	FPS49,
	FPS48,
	FPS47,
	FPS46,
	FPS45,
	FPS44
};

#define fps_machine_ms(factor)\
	if (fps.fastforward == FALSE) {\
		machine.msFrame = (1000.0 / (double) machine.fps) * factor;\
		fps.ms = machine.msFrame;\
	}

struct _fps {
	uint8_t counter;
	uint8_t frames_before_skip;
	uint8_t max_frames_skipped;
	uint8_t fastforward;
	uint8_t frames_skipped;
	uint32_t total_frames_skipped;
	double ms;
	double next_frame;
	double second_start;
	double second_end;
	double avarage;
	double nominal;
} fps;

void fpsInit(void);
void fpsFastForward(void);
void fpsNormalize(void);
void fpsFrameskip(void);

#endif /* FPS_H_ */
