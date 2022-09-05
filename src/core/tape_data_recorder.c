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

#include <stdlib.h>
#include <string.h>
#include "tape_data_recorder.h"
#include "clock.h"
#include "extra/filter-c/filter.h"
#include "audio/wave.h"
#include "gui.h"

static INLINE void tape_data_reset(void);
static INLINE double tape_data_cycles(void);

void tape_data_play_punes(void);
void tape_data_record_punes(void);
void tape_data_play_virtuanes(void);
void tape_data_record_virtuanes(void);

_tape_data_recorder tape_data_recorder;

BYTE tape_data_recorder_init(uTCHAR *file, BYTE type, BYTE mode) {
	tape_data_recorder_quit();

	vector_init(&tape_data_recorder.data, sizeof(BYTE));

	tape_data_reset();

	tape_data_recorder.mode = mode;
	tape_data_recorder.type = type;

	switch (tape_data_recorder.mode) {
		case TAPE_DATA_PLAY:
			if ((tape_data_recorder.file = ufopen(file, uL("rb"))) == NULL) {
				tape_data_recorder_quit();
				return (EXIT_ERROR);
			}

			fseek(tape_data_recorder.file, 0, SEEK_END);
			tape_data_recorder.data.total = ftell(tape_data_recorder.file);
			fseek(tape_data_recorder.file, 0, SEEK_SET);
			tape_data_recorder.bits = tape_data_recorder.data.total << 3;
			vector_resize(&tape_data_recorder.data, tape_data_recorder.data.total);
			fread(vector_get(&tape_data_recorder.data, 0), 1, tape_data_recorder.data.total, tape_data_recorder.file);

			if ((tape_data_recorder.type == TAPE_DATA_TYPE_VIRTUANES) ||
				(tape_data_recorder.type == TAPE_DATA_TYPE_NESTOPIA)) {
				tape_data_recorder.tick = tape_data_play_virtuanes;
			} else {
				tape_data_recorder.tick = tape_data_play_punes;
			}

			gui_overlay_info_append_msg_precompiled(33, NULL);
			gui_max_speed_start();
			break;
		case TAPE_DATA_RECORD:
			if ((tape_data_recorder.file = ufopen(file, uL("wb"))) == NULL) {
				tape_data_recorder_quit();
				return (EXIT_ERROR);
			}
			if ((tape_data_recorder.type == TAPE_DATA_TYPE_VIRTUANES) ||
				(tape_data_recorder.type == TAPE_DATA_TYPE_NESTOPIA)) {
				tape_data_recorder.tick = tape_data_record_virtuanes;
			} else {
				tape_data_recorder.tick = tape_data_record_punes;
			}

			gui_overlay_info_append_msg_precompiled(34, NULL);
			gui_max_speed_start();
			break;
		default:
			break;
	}

	gui_update_tape_menu();

	return (EXIT_OK);
}
void tape_data_recorder_quit(void) {
	if (tape_data_recorder.file) {
		tape_data_recorder_stop();
	}

	vector_free(&tape_data_recorder.data);

	tape_data_reset();

	gui_update_tape_menu();
}
void tape_data_recorder_stop(void) {
	if (tape_data_recorder.file) {
		if (tape_data_recorder.mode == TAPE_DATA_RECORD) {
			switch (tape_data_recorder.type) {
				default:
					fwrite(vector_get(&tape_data_recorder.data, 0), 1,
						vector_total(&tape_data_recorder.data), tape_data_recorder.file);
					break;
				case TAPE_DATA_TYPE_WAV: {
					BWHighPass *filter = create_bw_high_pass_filter(20, 1789773, 4000.0f);
					DBWORD i, count = 0;
					_vector samples;

					vector_init(&samples, sizeof(BYTE));

					for (i = 0 ; i < vector_total(&tape_data_recorder.data); i++) {
						BYTE data = (*((BYTE *)vector_get(&tape_data_recorder.data, i)));
						BYTE a = 0;

						for (a = 0; a < 8; a++) {
							FTR_PRECISION sample = bw_high_pass(filter, data & 0x01 ? 0.50f : -0.50f);

							count += 176;
							if (count >= 39375) {
								BYTE out = (sample * 127.0f) - 0x80;

								count -= 39375;
								vector_push_back(&samples, &out);
							}
							data >>= 1;
						}
					}

					free_bw_high_pass(filter);

					{
						_wav wav;

						memset(&wav, 0x00, sizeof(_wav));

						if (wave_open_file(&wav, tape_data_recorder.file, vector_total(&samples), 8, 8000, 1) == EXIT_OK) {
							wave_write(&wav, vector_get(&samples, 0), vector_total(&samples));
							wave_close(&wav);
							tape_data_recorder.file = wav.outfile;
						}

						vector_free(&samples);
					}
					break;
				}
			}
		}
		if (tape_data_recorder.file) {
			fclose(tape_data_recorder.file);
		}
		tape_data_recorder.file = NULL;
	}
	tape_data_reset();

	vector_clear(&tape_data_recorder.data);

	gui_overlay_info_append_msg_precompiled(35, NULL);
	gui_max_speed_stop();

	gui_update_tape_menu();
}
void tape_data_recorder_tick(void) {
	if (tape_data_recorder.tick) {
		if ((tape_data_recorder.cycles -= 1.0f) <= 0) {
			tape_data_recorder.tick();
			tape_data_recorder.cycles += tape_data_cycles();
	    }
	}
}

void tape_data_play_punes(void) {
	if (tape_data_recorder.index < tape_data_recorder.bits) {
		BYTE *item = vector_get(&tape_data_recorder.data, tape_data_recorder.index >> 3);

		tape_data_recorder.out = ((*item) >> (tape_data_recorder.index & 0x07)) & 0x01;
		tape_data_recorder.index++;
	} else {
		tape_data_recorder_stop();
	}
}
void tape_data_record_punes(void) {
	BYTE *item = NULL;

	if ((tape_data_recorder.bits & 0x07) == 0x00) {
		BYTE zero = 0;

		vector_push_back(&tape_data_recorder.data, &zero);
	}

	item = vector_get(&tape_data_recorder.data, tape_data_recorder.index >> 3);

	if (item) {
		(*item) |= (tape_data_recorder.in << (tape_data_recorder.index & 0x07));
	}
	tape_data_recorder.index++;
	tape_data_recorder.bits++;
}
void tape_data_play_virtuanes(void) {
	if (tape_data_recorder.index < tape_data_recorder.data.total) {
		BYTE *item = vector_get(&tape_data_recorder.data, tape_data_recorder.index);

		tape_data_recorder.out = (*item) >= 0x8C ? 0x01 : 0x00;
		tape_data_recorder.index++;
	} else {
		tape_data_recorder_stop();
	}
}
void tape_data_record_virtuanes(void) {
	BYTE data = tape_data_recorder.in ? 0x90 : 0x70;

	vector_push_back(&tape_data_recorder.data, &data);
}

static INLINE void tape_data_reset(void) {
	tape_data_recorder.mode = TAPE_DATA_NONE;
	tape_data_recorder.type = TAPE_DATA_TYPE_TAP;

	tape_data_recorder.in = 0;
	tape_data_recorder.out = 0;
	tape_data_recorder.bits = 0;
	tape_data_recorder.index = 0;
	tape_data_recorder.cycles = 0.0f;

	tape_data_recorder.tick = NULL;
}
static INLINE double tape_data_cycles(void) {
	switch (tape_data_recorder.type) {
		default:
			return (machine.cpu_hz / 44100.0f);
		case TAPE_DATA_TYPE_VIRTUANES:
		case TAPE_DATA_TYPE_NESTOPIA:
			// non e'sufficiente per "Castle Excellent"
			return (machine.cpu_hz / 32000.0f);
	}
}
