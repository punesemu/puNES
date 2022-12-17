/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef WAVE_H_
#define WAVE_H_

#include "common.h"

typedef struct _wav {
	FILE *outfile;
	char *buffer;
	char *pbuffer;
	int channels;
	int samplerate;
	int bits_per_sample;
	int samples;
	int subchunk2size;
	int buffer_size;
	int count;
} _wav;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE wave_open_file(_wav *wav, FILE *file, int samples, int bits_per_sample, int samplerate, int channels);
EXTERNC BYTE wave_open_filename(_wav *wav, uTCHAR *filename, int samples, int bits_per_sample, int samplerate, int channels);
EXTERNC void wave_close(_wav *wav);
EXTERNC void wave_write(_wav *wav, BYTE *data, int samples);

EXTERNC BYTE wav_from_audio_emulator_open(uTCHAR *filename, int samples);
EXTERNC void wav_from_audio_emulator_close(void);
EXTERNC void wave_from_audio_emulator_write(SWORD *data, int samples);

#undef EXTERNC

#endif /* WAVE_H_ */
