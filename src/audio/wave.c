/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#include <stdio.h>
#include <stdlib.h>
#include "wave.h"
#include "audio/snd.h"
#include "info.h"
#include "gui.h"

struct _wav {
	FILE *outfile;
	char *buffer;
	char *pbuffer;
	int samples;
	int subchunk2size;
	int buffer_size;
	int count;
} wav;

BYTE wave_open(uTCHAR *filename, int samples) {
	wave_close();

	wav.buffer_size = samples * snd.channels * (16 / 8);

	if ((wav.buffer = malloc(wav.buffer_size)) == NULL) {
		return (EXIT_ERROR);
	}

	snd_thread_pause();

	if ((wav.outfile = ufopen(filename, uL("wb"))) == NULL) {
		free(wav.buffer);
		wav.buffer = NULL;
		return (EXIT_ERROR);
	}

	wav.pbuffer = wav.buffer;
	wav.samples = samples;
	wav.subchunk2size = 0;
	wav.count = 0;

	// ChunkID
	fputs("RIFF", wav.outfile);
	// ChunkSize
	fseek(wav.outfile, 4, SEEK_CUR);
	// Format
	fputs("WAVE", wav.outfile);
	// Subchunk1ID
	fputs("fmt ", wav.outfile);
	// Subchunk1Size
	fputc(16, wav.outfile);
	fputc(0, wav.outfile);
	fputc(0, wav.outfile);
	fputc(0, wav.outfile);
	// AudioFormat
	fputc(1, wav.outfile);
	fputc(0, wav.outfile);
	// NumChannels
	fputc((char) snd.channels, wav.outfile);
	fputc(0, wav.outfile);
	// SampleRate
	fputc((snd.samplerate >> 0) & 0xFF, wav.outfile);
	fputc((snd.samplerate >> 8) & 0xFF, wav.outfile);
	fputc((snd.samplerate >> 16) & 0xFF, wav.outfile);
	fputc((snd.samplerate >> 24) & 0xFF, wav.outfile);
	// ByteRate
	{
		int byterate = snd.samplerate * snd.channels * 16 / 8;

		fputc((byterate >> 0) & 0xFF, wav.outfile);
		fputc((byterate >> 8) & 0xFF, wav.outfile);
		fputc((byterate >> 16) & 0xFF, wav.outfile);
		fputc((byterate >> 24) & 0xFF, wav.outfile);
	}
	// BlockAlign
	{
		short int blockalign = snd.channels * 16 / 8;

		fputc((blockalign >> 0) & 0xFF, wav.outfile);
		fputc((blockalign >> 8) & 0xFF, wav.outfile);
	}
	// BitsPerSample
	fputc(16, wav.outfile);
	fputc(0, wav.outfile);
	// Subchunk2ID
	fputs("data", wav.outfile);
	// Subchunk2Size
	fseek(wav.outfile, 4, SEEK_CUR);

	info.wave_in_record = TRUE;

	gui_overlay_info_append_msg_precompiled(0, NULL);

	snd_thread_continue();

	return (EXIT_OK);
}
void wave_close(void) {
	if (snd.cache) {
		snd_thread_pause();
	}

	if (wav.outfile) {
		long int actual_size;

		// scrivo l'ultimo segmento
		if (wav.count) {
			wav.subchunk2size += fwrite(wav.buffer, 1, wav.count * snd.channels * (16 / 8),
					wav.outfile);
		}

		actual_size = ftell(wav.outfile) - 8;

		// ChunkSize
		fseek(wav.outfile, 4, SEEK_SET);
		fputc((actual_size >> 0) & 0xFF, wav.outfile);
		fputc((actual_size >> 8) & 0xFF, wav.outfile);
		fputc((actual_size >> 16) & 0xFF, wav.outfile);
		fputc((actual_size >> 24) & 0xFF, wav.outfile);
		// Subchunk2Size
		fseek(wav.outfile, 0x28, SEEK_SET);
		fputc((wav.subchunk2size >> 0) & 0xFF, wav.outfile);
		fputc((wav.subchunk2size >> 8) & 0xFF, wav.outfile);
		fputc((wav.subchunk2size >> 16) & 0xFF, wav.outfile);
		fputc((wav.subchunk2size >> 24) & 0xFF, wav.outfile);

		fclose(wav.outfile);
		wav.outfile = NULL;

		gui_overlay_info_append_msg_precompiled(1, NULL);

	}

	if (wav.buffer) {
		free(wav.buffer);
		wav.buffer = NULL;
	}

	info.wave_in_record = FALSE;

	if (snd.cache) {
		snd_thread_continue();
	}
}
void wave_write(SWORD *data, int samples) {
	SWORD *src = data;
	int a, b;

	if (wav.outfile == NULL) {
		return;
	}

	for (a = 0; a < samples; a++) {
		for (b = 0; b < snd.channels; b++) {
			(*wav.pbuffer++) = (*src) & 0xFF;
			(*wav.pbuffer++) = ((*src) >> 8) & 0xFF;
			src++;
		}
		if (++wav.count >= wav.samples) {
			wav.subchunk2size += fwrite(wav.buffer, 1, wav.buffer_size, wav.outfile);
			wav.pbuffer = wav.buffer;
			wav.count = 0;
		}
	}
}
