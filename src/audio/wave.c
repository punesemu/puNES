/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

BYTE wave_init(_wav *wav, int samples, int bits_per_sample, int samplerate, int channels);

_wav audio_recording;

BYTE wave_open_file(_wav *wav, FILE *file, int samples, int bits_per_sample, int samplerate, int channels) {
	wave_close(wav);

	if (file == NULL) {
		return (EXIT_ERROR);
	}
	wav->outfile = file;

	return (wave_init(wav, samples, bits_per_sample, samplerate, channels));
}
BYTE wave_open_filename(_wav *wav, uTCHAR *filename, int samples, int bits_per_sample, int samplerate, int channels) {
	wave_close(wav);

	if ((wav->outfile = ufopen(filename, uL("wb"))) == NULL) {
		return (EXIT_ERROR);
	}

	return (wave_init(wav, samples, bits_per_sample, samplerate, channels));
}
void wave_close(_wav *wav) {
	if (wav->outfile) {
		long int actual_size;

		// scrivo l'ultimo segmento
		if (wav->count) {
			wav->subchunk2size += fwrite(wav->buffer, 1, wav->count * wav->channels * (wav->bits_per_sample / 8), wav->outfile);
		}

		actual_size = ftell(wav->outfile) - 8;

		// ChunkSize
		fseek(wav->outfile, 4, SEEK_SET);
		fputc((actual_size >> 0) & 0xFF, wav->outfile);
		fputc((actual_size >> 8) & 0xFF, wav->outfile);
		fputc((actual_size >> 16) & 0xFF, wav->outfile);
		fputc((actual_size >> 24) & 0xFF, wav->outfile);
		// Subchunk2Size
		fseek(wav->outfile, 0x28, SEEK_SET);
		fputc((wav->subchunk2size >> 0) & 0xFF, wav->outfile);
		fputc((wav->subchunk2size >> 8) & 0xFF, wav->outfile);
		fputc((wav->subchunk2size >> 16) & 0xFF, wav->outfile);
		fputc((wav->subchunk2size >> 24) & 0xFF, wav->outfile);

		fclose(wav->outfile);
		wav->outfile = NULL;
	}

	if (wav->buffer) {
		free(wav->buffer);
		wav->buffer = NULL;
	}
}
void wave_write(_wav *wav, BYTE *data, int samples) {
	BYTE *src = data;
	int a, b, c;

	if (wav->outfile == NULL) {
		return;
	}

	for (a = 0; a < samples; a++) {
		for (b = 0; b < wav->channels; b++) {
			for (c = 0; c < (wav->bits_per_sample / 8); c++) {
				(*wav->pbuffer++) = (*src) & 0xFF;
				src++;
			}
			//(*wav->pbuffer++) = ((*src) >> 8) & 0xFF;
		}
		if (++wav->count >= wav->samples) {
			wav->subchunk2size += fwrite(wav->buffer, 1, wav->buffer_size, wav->outfile);
			wav->pbuffer = wav->buffer;
			wav->count = 0;
		}
	}
}

BYTE wave_init(_wav *wav, int samples, int bits_per_sample, int samplerate, int channels) {
	wav->buffer_size = samples * channels * (bits_per_sample / 8);

	if ((wav->buffer = malloc(wav->buffer_size)) == NULL) {
		return (EXIT_ERROR);
	}

	wav->pbuffer = wav->buffer;
	wav->samples = samples;
	wav->subchunk2size = 0;
	wav->count = 0;
	wav->channels = channels;
	wav->samplerate = samplerate;
	wav->bits_per_sample = bits_per_sample;

	// ChunkID
	fputs("RIFF", wav->outfile);
	// ChunkSize
	fseek(wav->outfile, 4, SEEK_CUR);
	// Format
	fputs("WAVE", wav->outfile);
	// Subchunk1ID
	fputs("fmt ", wav->outfile);
	// Subchunk1Size
	fputc(16, wav->outfile);
	fputc(0, wav->outfile);
	fputc(0, wav->outfile);
	fputc(0, wav->outfile);
	// AudioFormat
	fputc(1, wav->outfile);
	fputc(0, wav->outfile);
	// NumChannels
	fputc((char)channels, wav->outfile);
	fputc(0, wav->outfile);
	// SampleRate
	fputc((samplerate >> 0) & 0xFF, wav->outfile);
	fputc((samplerate >> 8) & 0xFF, wav->outfile);
	fputc((samplerate >> 16) & 0xFF, wav->outfile);
	fputc((samplerate >> 24) & 0xFF, wav->outfile);
	// ByteRate
	{
		int byterate = samplerate * channels * bits_per_sample / 8;

		fputc((byterate >> 0) & 0xFF, wav->outfile);
		fputc((byterate >> 8) & 0xFF, wav->outfile);
		fputc((byterate >> 16) & 0xFF, wav->outfile);
		fputc((byterate >> 24) & 0xFF, wav->outfile);
	}
	// BlockAlign
	{
		short int blockalign = channels * bits_per_sample / 8;

		fputc((blockalign >> 0) & 0xFF, wav->outfile);
		fputc((blockalign >> 8) & 0xFF, wav->outfile);
	}
	// BitsPerSample
	fputc(bits_per_sample, wav->outfile);
	fputc(0, wav->outfile);
	// Subchunk2ID
	fputs("data", wav->outfile);
	// Subchunk2Size
	fseek(wav->outfile, 4, SEEK_CUR);

	return (EXIT_OK);
}

BYTE wav_from_audio_emulator_open(uTCHAR *filename, int samples) {
	BYTE rc;

	snd_thread_pause();

	rc = wave_open_filename(&audio_recording, filename, samples, 16, snd.samplerate, snd.channels);
	if (rc == EXIT_OK) {
		info.recording_on_air = TRUE;
		info.recording_is_a_video = FALSE;

		gui_overlay_info_append_msg_precompiled(0, NULL);
	}

	snd_thread_continue();

	return (rc);
}
void wav_from_audio_emulator_close(void) {
	if (snd.cache) {
		snd_thread_pause();
	}

	if (audio_recording.outfile) {
		gui_overlay_info_append_msg_precompiled(1, NULL);
	}

	wave_close(&audio_recording);

	info.recording_on_air = FALSE;
	info.recording_is_a_video = FALSE;

	if (snd.cache) {
		snd_thread_continue();
	}
}
void wave_from_audio_emulator_write(SWORD *data, int samples) {
	wave_write(&audio_recording, (BYTE *)data, samples);
}
