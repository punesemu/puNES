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
#include <string.h>
#include <stdlib.h>
#include "rewind.h"
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "mappers.h"
#include "irqA12.h"
#include "irql2f.h"
#include "fds.h"
#include "gui.h"
#include "input.h"
#include "clock.h"
#include "tas.h"
#include "emu_thread.h"
#include "video/gfx_thread.h"
#include "clock.h"
#include "conf.h"
#include "audio/snd.h"

enum rewind_misc {
	REWIND_CHUNK_TYPE_SEGMENT,
	REWIND_CHUNK_TYPE_CHUNK_BUFFER,
	REWIND_SNAPS_FOR_CHUNK = 15,
	REWIND_SNAPS_FOR_FACTOR = 4
};

#define rewind_on_struct(_mode, _strct, _size_buff)\
	switch (_mode) {\
	case REWIND_OP_SAVE:\
		memcpy(snap->data + index, &_strct, sizeof(_strct));\
		index += sizeof(_strct);\
		break;\
	case REWIND_OP_READ:\
		memcpy(&_strct, snap->data + index, sizeof(_strct));\
		index += sizeof(_strct);\
		break;\
	case REWIND_OP_COUNT:\
		rwint.size._size_buff += sizeof(_strct);\
		break;\
	}
#define rewind_on_mem(_mode, _mem, _size, _size_buff)\
	switch (_mode) {\
	case REWIND_OP_SAVE:\
		memcpy(snap->data + index, _mem, _size);\
		index += _size;\
		break;\
	case REWIND_OP_READ:\
		memcpy(_mem, snap->data + index, _size);\
		index += _size;\
		break;\
	case REWIND_OP_COUNT:\
		rwint.size._size_buff += _size;\
		break;\
	}

typedef struct _rewind_index {
	int32_t segment;
	int32_t chunk;
	int32_t snap;
} _rewind_index;
typedef struct _rewind_snapshoot {
	_rewind_index index;
	BYTE *data;
} _rewind_snapshoot;
typedef struct _rewind_chunk {
	BYTE type;
	BYTE *data;
	_rewind_snapshoot *snaps;
} _rewind_chunk;

INLINE static BYTE rewind_is_disabled(void);
INLINE static void rewind_increment_count_chunks(void);
INLINE static void rewind_update_chunk_snaps( _rewind_chunk *chunk, int32_t chunk_index, BYTE *src);
INLINE static void rewind_operation(BYTE mode, BYTE save_input, _rewind_snapshoot *snap);
INLINE static void rewind_free_chunk(_rewind_chunk *chunk);
INLINE static void rewind_execute_frame(void);
INLINE static char *rewind_convert_time_in_text(char *txt, size_t size, char *color, int32_t counter);

static BYTE _rewind_frames(int32_t frames_to_rewind, BYTE exec_last_frame);

struct _rewind_internal {
	uTCHAR file_name[LENGTH_FILE_NAME_LONG];
	FILE *file;

	int32_t chunks_for_segment;
	int32_t first_valid_snap;
	int32_t snap_cursor;

	_rewind_chunk segment;
	_rewind_chunk cbuffer;

	_rewind_index index;

	struct _rewind_info_max_buffered {
		int32_t segments;
		int32_t snaps;
	} max_buffered;
	struct _rewind_info_range {
		struct _rewind_info_cunk_range {
			int32_t first;
			int32_t last;
		} chunk;
	} range;
	struct _rewind_info_count {
		int32_t segments;
		int32_t chunks;
		int32_t snaps;
	} count;
	struct _rewind_info_size {
		size_t keyframe;
		size_t screen;
		size_t input;
		size_t chunk;
		size_t first_chunk;
		size_t total;
	} size;
} rwint;

BYTE rewind_init(void) {
	// in caso di riavvio del rewind
	rewind_quit();

	memset(&rwnd, 0, sizeof(rwnd));
	memset(&rwint, 0, sizeof(rwint));

	rwint.count.snaps = -1;
	rwint.chunks_for_segment = (machine.fps * 60) / REWIND_SNAPS_FOR_CHUNK;

	{
		size_t size = sizeof(_rewind_snapshoot) * REWIND_SNAPS_FOR_CHUNK;

		if (rewind_is_disabled() == FALSE) {
			if ((rwint.segment.snaps = (_rewind_snapshoot *)malloc(size)) == NULL) {
				fprintf(stderr, "rewind : Out of memory\n");
				return (EXIT_ERROR);
			}
			memset(rwint.segment.snaps, 0, size);
		}

		if ((rwint.cbuffer.snaps = (_rewind_snapshoot *)malloc(size)) == NULL) {
			fprintf(stderr, "rewind : Out of memory\n");
			return (EXIT_ERROR);
		}
		memset(rwint.cbuffer.snaps, 0, size);
	}

	{
		_rewind_snapshoot snap;

		snap.index.chunk = 0;
		snap.index.snap = 0;
		rewind_operation(REWIND_OP_COUNT, TRUE, &snap);
	}

	rwint.size.chunk = rwint.size.keyframe + (rwint.size.input * REWIND_SNAPS_FOR_CHUNK);
	rwint.size.first_chunk = rwint.size.screen + rwint.size.chunk;
	rwint.size.total = rwint.size.screen + (rwint.size.chunk * rwint.chunks_for_segment);

	if (rewind_is_disabled() == FALSE) {
		if ((rwint.segment.data = (BYTE *)malloc(rwint.size.total)) == NULL) {
			fprintf(stderr, "rewind : Out of memory\n");
			return (EXIT_ERROR);
		}
		memset(rwint.segment.data, 0, rwint.size.chunk);
		rwint.segment.type = REWIND_CHUNK_TYPE_SEGMENT;
		rewind_update_chunk_snaps(&rwint.segment, 0, NULL);
	}
	if ((rwint.cbuffer.data = (BYTE *)malloc(rwint.size.first_chunk)) == NULL) {
		fprintf(stderr, "rewind : Out of memory\n");
		return (EXIT_ERROR);
	}
	memset(rwint.cbuffer.data, 0, rwint.size.first_chunk);
	rwint.cbuffer.type = REWIND_CHUNK_TYPE_CHUNK_BUFFER;
	rewind_update_chunk_snaps(&rwint.cbuffer, 0, NULL);

	// (se non ho limiti allora devo impostare rwint.max_buffered.segments a 0)
	// lo incremento di 1 perche' se, per esempio, sono 3 i segmenti che devono
	// sempre essere bufferizzati, quando sto trattando il segmento 3 (cioe' il quarto),
	// chunk 0, snap 15, per poter fare il rewindw completo di 3 segmenti devo caricare
	// il segmento 0, chunk 0, snap 15 percio' devo sempre bufferizzarne 3 + 1.
	switch (cfg->rewind_minutes) {
		case RWND_0_MINUTES:
			rwint.max_buffered.segments = 1;
			break;
		case RWND_2_MINUTES:
			rwint.max_buffered.segments = 2 + 1;
			break;
		case RWND_5_MINUTES:
			rwint.max_buffered.segments = 5 + 1;
			break;
		default:
		case RWND_15_MINUTES:
			rwint.max_buffered.segments = 15 + 1;
			break;
		case RWND_30_MINUTES:
			rwint.max_buffered.segments = 30 + 1;
			break;
		case RWND_60_MINUTES:
			rwint.max_buffered.segments = 60 + 1;
			break;
		case RWND_UNLIMITED_MINUTES:
			rwint.max_buffered.segments = 0;
			break;
	}

	if (rwint.max_buffered.segments > 0) {
		rwint.max_buffered.snaps = (rwint.max_buffered.segments - 1) * rwint.chunks_for_segment * REWIND_SNAPS_FOR_CHUNK;
	}

	rwint.range.chunk.first = 0;
	rwint.range.chunk.last = rwint.range.chunk.first + rwint.chunks_for_segment;

	rwint.index.segment = 0;
	rwint.index.chunk = 0;
	rwint.index.snap = -1;

	// creo il file temporaneo
	if (rewind_is_disabled() == FALSE) {
		uTCHAR basename[255], *last_dot;;

		gui_utf_basename(info.rom.file, basename, usizeof(basename));

		// rintraccio l'ultimo '.' nel nome
		if ((last_dot = ustrrchr(basename, uL('.')))) {
			// elimino l'estensione
			(*last_dot) = 0x00;
		};

#if defined (__WIN32__)
		usnprintf(rwint.file_name, usizeof(rwint.file_name), uL("" uPERCENTs uPERCENTs "_" uPERCENTs ".rwd"), gui.ostmp,
			basename, emu_rand_str());
#else
		usnprintf(rwint.file_name, usizeof(rwint.file_name), uL("" uPERCENTs "/" uPERCENTs "_" uPERCENTs ".rwd"), gui.ostmp,
			basename, emu_rand_str());
#endif

		if ((rwint.file = ufopen(rwint.file_name, uL("w+b"))) == NULL) {
			return (EXIT_ERROR);
		}
	}

	return (EXIT_OK);
}
void rewind_quit(void) {
	rewind_free_chunk(&rwint.segment);
	rewind_free_chunk(&rwint.cbuffer);
	if (rwint.file) {
		fclose(rwint.file);
		rwint.file = NULL;
		if (emu_file_exist(rwint.file_name) == EXIT_OK) {
			uremove(rwint.file_name);
		}
	}
}
void rewind_snapshoot(void) {
	_rewind_snapshoot *snap;

	// se non ci sono rom caricate, non faccio niente
	if (rewind_is_disabled()) {
		return;
	}

	if (++rwint.index.snap == REWIND_SNAPS_FOR_CHUNK) {
		rwint.index.snap = 0;
		rewind_increment_count_chunks();
	}

	snap = rwint.segment.snaps + rwint.index.snap;

	snap->index.chunk = rwint.index.chunk;
	snap->index.snap = rwint.index.snap;

	rewind_operation(REWIND_OP_SAVE, TRUE, snap);

	rwint.count.snaps++;

	if (rwint.max_buffered.segments > 0) {
		int32_t first_valid_snap = rwint.count.snaps - rwint.max_buffered.snaps;

		if (first_valid_snap > rwint.first_valid_snap) {
			rwint.first_valid_snap = first_valid_snap;
		}
	}
}
void rewind_frames(int32_t frames_to_rewind) {
	if (_rewind_frames(frames_to_rewind, FALSE) == EXIT_OK) {
		if (tas.type != NOTAS) {
			tas_rewind(frames_to_rewind);
		}
	}
}

void rewind_save_state_snap(BYTE mode) {
	_rewind_snapshoot *snap = rwint.cbuffer.snaps;

	snap->index.segment = 0;
	snap->index.chunk = 0;
	snap->index.snap = 0;

	rewind_update_chunk_snaps(&rwint.cbuffer, 0, NULL);

	rewind_operation(mode, FALSE, snap);
}

void rewind_init_operation(void) {
	emu_thread_pause();
	gfx_thread_pause();
	ppu_draw_screen_pause();

	snd_playback_stop();

	rwint.snap_cursor = rwint.count.snaps;

	rwnd.active = TRUE;
	emu_ctrl_doublebuffer();
	gui_update();

	gfx_thread_continue();
}
void rewind_close_operation(void) {
	gfx_thread_pause();

	rwint.count.snaps = rwint.snap_cursor;
	rwint.count.segments = rwint.index.segment;

	rwnd.active = FALSE;
	emu_ctrl_doublebuffer();
	gui_update();

	snd_playback_start();

	ppu_draw_screen_continue();
	gfx_thread_continue();
	emu_thread_continue();
}

BYTE rewind_is_first_snap(void) {
	return (rwint.snap_cursor == 0);
}
BYTE rewind_is_last_snap(void) {
	return ((rwint.snap_cursor + 1) > rwint.count.snaps);
}

int32_t rewind_count_snaps(void) {
	return (rwint.count.snaps);
}
int32_t rewind_snap_cursor(void) {
	return (rwint.snap_cursor);
}
int32_t rewind_calculate_snap_cursor(int factor, BYTE direction) {
	int32_t snaps = factor * REWIND_SNAPS_FOR_FACTOR;

	if (direction == RWND_BACKWARD) {
		snaps = -snaps;

		if ((rwint.snap_cursor + snaps) < rwint.first_valid_snap) {
			snaps = -rwint.snap_cursor;
		}
	} else {
		if ((rwint.snap_cursor + snaps) > rwint.count.snaps) {
			snaps = (rwint.count.snaps - rwint.snap_cursor);
		}
	}

	return (snaps);
}

char *rewind_text_time_count_snaps(void) {
	static char txt[100];

	return (rewind_convert_time_in_text((char *)&txt, sizeof(txt), "[normal]", rwint.count.snaps));
}
char *rewind_text_time_snap_cursor(void) {
	static char txt[100];

	return (rewind_convert_time_in_text((char *)&txt, sizeof(txt), "[yellow]", rwint.snap_cursor));
}
char *rewind_text_time_backward(void) {
	static char txt[100];

	return (rewind_convert_time_in_text((char *)&txt, sizeof(txt), "[cyan]", rwint.count.snaps - rwint.snap_cursor));
}

INLINE static BYTE rewind_is_disabled(void) {
	return((info.no_rom | info.turn_off) || (cfg->rewind_minutes == RWND_0_MINUTES));
}
INLINE static void rewind_increment_count_chunks(void) {
	if (++rwint.index.chunk == rwint.chunks_for_segment) {
		int32_t segment_to_save = rwint.index.segment;

		if (rwint.max_buffered.segments > 0) {
			segment_to_save = rwint.index.segment % rwint.max_buffered.segments;
		}

		fseek(rwint.file, segment_to_save * rwint.size.total, SEEK_SET);
		fwrite(rwint.segment.data, rwint.size.total, 1, rwint.file);
		rwint.index.chunk = 0;
		rwint.range.chunk.first = rwint.range.chunk.last;
		rwint.range.chunk.last = rwint.range.chunk.first + rwint.chunks_for_segment;
		rwint.index.segment = ++rwint.count.segments;
	}
	rewind_update_chunk_snaps(&rwint.segment, rwint.index.chunk, NULL);
	rwint.count.chunks++;
}
INLINE static void rewind_update_chunk_snaps( _rewind_chunk *chunk, int32_t chunk_index, BYTE *src) {
	BYTE *start;
	uint32_t i;

	if (chunk_index == 0) {
		start = chunk->data + rwint.size.screen + (rwint.size.chunk * chunk_index);
		chunk->snaps->data = chunk->data;
	} else {
		if (chunk->type == REWIND_CHUNK_TYPE_SEGMENT) {
			start = chunk->data + rwint.size.screen + (rwint.size.chunk * chunk_index);
		} else {
			start = chunk->data;
		}
		chunk->snaps->data = start;
	}

	for (i = 1; i < REWIND_SNAPS_FOR_CHUNK; i++) {
		(chunk->snaps + i)->data = (start + rwint.size.keyframe) + (rwint.size.input * i);
	}

	if (src != NULL) {
		if (chunk_index == 0) {
			memcpy(chunk->data, src, rwint.size.first_chunk);
		} else {
			memcpy(start, src, rwint.size.chunk);
		}
	}
}
INLINE static void rewind_operation(BYTE mode, BYTE save_input, _rewind_snapshoot *snap) {
	size_t index = 0;
	BYTE i;

	if (snap->index.snap == 0) {
		if (snap->index.chunk == 0) {
			rewind_on_mem(mode, screen.rd->data, screen_size(), screen);
		}

		// CPU
		rewind_on_struct(mode, cpu, keyframe);
		rewind_on_struct(mode, irq, keyframe);
		rewind_on_struct(mode, nmi, keyframe);

		// PPU
		rewind_on_struct(mode, ppu, keyframe);
		rewind_on_struct(mode, ppu_openbus, keyframe);
		rewind_on_struct(mode, r2000, keyframe);
		rewind_on_struct(mode, r2001, keyframe);
		rewind_on_struct(mode, r2002, keyframe);
		rewind_on_struct(mode, r2003, keyframe);
		rewind_on_struct(mode, r2004, keyframe);
		rewind_on_struct(mode, r2006, keyframe);
		rewind_on_struct(mode, r2007, keyframe);
		rewind_on_struct(mode, spr_ev, keyframe);
		rewind_on_struct(mode, sprite, keyframe);
		rewind_on_struct(mode, sprite_plus, keyframe);
		rewind_on_struct(mode, tile_render, keyframe);
		rewind_on_struct(mode, tile_fetch, keyframe);

		// APU
		rewind_on_struct(mode, apu, keyframe);
		rewind_on_struct(mode, r4011, keyframe);
		rewind_on_struct(mode, r4015, keyframe);
		rewind_on_struct(mode, r4017, keyframe);
		rewind_on_struct(mode, S1, keyframe);
		rewind_on_struct(mode, S2, keyframe);
		rewind_on_struct(mode, TR, keyframe);
		rewind_on_struct(mode, NS, keyframe);
		rewind_on_struct(mode, DMC, keyframe);

		// mem map
		rewind_on_struct(mode, mmcpu, keyframe);
		rewind_on_struct(mode, prg, keyframe);
		rewind_on_mem(mode, prg.ram.data, prg.ram.size, keyframe);
		if (prg.ram_plus) {
			rewind_on_mem(mode, prg.ram_plus, prg_ram_plus_size(), keyframe);
		}
		rewind_on_struct(mode, chr, keyframe);
		if (mapper.write_vram) {
			rewind_on_mem(mode, chr_chip(0), chr_ram_size(), keyframe);
		}
		if (chr.extra.size) {
			rewind_on_mem(mode, chr.extra.data, chr.extra.size, keyframe);
		}
		rewind_on_struct(mode, ntbl, keyframe);
		rewind_on_struct(mode, mmap_palette, keyframe);
		rewind_on_struct(mode, oam, keyframe);

		// mapper
		rewind_on_struct(mode, mapper, keyframe);
		for (i = 0; i < LENGTH(mapper.internal_struct); i++) {
			if (mapper.internal_struct[i]) {
				rewind_on_mem(mode, mapper.internal_struct[i], mapper.internal_struct_size[i], keyframe);
			}
		}

		// irqA12
		if (irqA12.present) {
			rewind_on_struct(mode, irqA12, keyframe);
		}

		// irql2f
		if (irql2f.present) {
			rewind_on_struct(mode, irql2f, keyframe);
		}

		// FDS
		if (fds.info.enabled) {
			BYTE old_side_inserted = fds.drive.side_inserted;

			rewind_on_struct(mode, fds.drive, keyframe);
			rewind_on_struct(mode, fds.snd, keyframe);
			rewind_on_struct(mode, fds.info.last_operation, keyframe);

			// in caso di ripristino di una snapshot, se era caricato
			// un'altro side del disco, devo ricaricarlo.
			if ((mode == REWIND_OP_READ) && (old_side_inserted != fds.drive.side_inserted)) {
				fds_disk_op(FDS_DISK_SELECT_FROM_REWIND, fds.drive.side_inserted);
				gui_update();
			}
		}
	}

	// input
	if (save_input == TRUE) {
		// standard controller
		for (i = PORT1; i < PORT_MAX; i++) {
			rewind_on_struct(mode, port[i].type_pad, input);
			rewind_on_struct(mode, port[i].index, input);
			rewind_on_struct(mode, port[i].data, input);
			rewind_on_struct(mode, port[i].input, input);
			rewind_on_struct(mode, port[i].turbo, input);
		}

		// zapper, mouse, arkanoid, oeka_kids_tablet
		rewind_on_struct(mode, gmouse.x, input);
		rewind_on_struct(mode, gmouse.y, input);
		rewind_on_struct(mode, gmouse.left, input);
		rewind_on_struct(mode, gmouse.right, input);

		rewind_on_struct(mode, tas.total_lag_frames, input);
	}
}
INLINE static void rewind_free_chunk(_rewind_chunk *chunk) {
	if (chunk->snaps) {
		free(chunk->snaps);
		chunk->snaps = NULL;
	}
	if (chunk->data) {
		free(chunk->data);
		chunk->data = NULL;
	}
}
INLINE static void rewind_execute_frame(void) {
	if (info.frame_status == FRAME_FINISHED) {
		info.frame_status = FRAME_STARTED;
	}

	while (info.frame_status == FRAME_STARTED) {
		cpu_exe_op();
	}
}
INLINE static char *rewind_convert_time_in_text(char *txt, size_t size, char *color, int32_t counter) {
	int32_t seconds, ms, s, m, h;

	seconds = counter / machine.fps;
	h = (seconds / 3600);
	m = (seconds -(3600 * h)) / 60;
	s = (seconds -(3600 * h) - (m * 60));
	ms = (counter % machine.fps) * (1000 / machine.fps);

	memset(txt, 0, size);
	snprintf(txt, size, "%s%d[normal]:%s%02d[normal]:%s%02d[normal].%s%03d[normal]", color, h, color, m, color, s, color, ms);

	return (txt);
}

static BYTE _rewind_frames(int32_t frames_to_rewind, BYTE exec_last_frame) {
	int32_t cursor, segment, chunk, snaps;
	_rewind_snapshoot *snap = NULL;
	_rewind_index index;
	BYTE *src;

	cursor = rwint.snap_cursor + frames_to_rewind;

	if (frames_to_rewind < 0) {
		// backward
		if (cursor < rwint.first_valid_snap) {
			return (EXIT_ERROR);
		}
	} else if (frames_to_rewind > 0) {
		// forward
		if (cursor > rwint.count.snaps) {
			return (EXIT_ERROR);
		}
	} else {
		return (EXIT_ERROR);
	}

	chunk = cursor / REWIND_SNAPS_FOR_CHUNK;
	snaps = cursor % REWIND_SNAPS_FOR_CHUNK;
	segment = chunk / rwint.chunks_for_segment;

	index.chunk = chunk % rwint.chunks_for_segment;
	index.snap = 0;

	rwint.snap_cursor = cursor;

	// se non sono nel segmento corrente carico da disco
	if ((chunk < rwint.range.chunk.first) || (chunk >= rwint.range.chunk.last)) {
		int32_t segment_to_load = segment, segment_to_save = rwint.index.segment;

		if (rwint.max_buffered.segments > 0) {
			segment_to_load %= rwint.max_buffered.segments;
			segment_to_save %= rwint.max_buffered.segments;
		}

		if (rwint.index.segment == rwint.count.segments) {
			fseek(rwint.file, segment_to_save * rwint.size.total, SEEK_SET);
			fwrite(rwint.segment.data, rwint.size.total, 1, rwint.file);
		}

		fseek(rwint.file, segment_to_load * rwint.size.total, SEEK_SET);
		if (fread(rwint.segment.data, rwint.size.total, 1, rwint.file) < 1) {
			return (EXIT_ERROR);
		}

		rwint.index.segment = segment;
		rwint.range.chunk.first = segment * rwint.chunks_for_segment;
		rwint.range.chunk.last = rwint.range.chunk.first + rwint.chunks_for_segment;
	}

	if (index.chunk == 0) {
		src = rwint.segment.data;
	} else {
		src = rwint.segment.data + rwint.size.screen + (rwint.size.chunk * index.chunk);
	}

	if (snaps == 0) {
		if (index.chunk > 0) {
			// visto che lo screen lo salvo solo per il primo chunk della serie
			// se facessi solo il rewind_operation ripristinerei il keyframe ma
			// non avrei lo screen aggiornato quindi sono costretto ad eseguire
			// prima tutti gli snaps del chunk precedente compreso l'ultimo frame.
			_rewind_frames(-1, TRUE);
		}

		rewind_update_chunk_snaps(&rwint.cbuffer, index.chunk, src);

		snap = rwint.cbuffer.snaps;
		snap->index.chunk = index.chunk;
		snap->index.snap = index.snap;

		rewind_operation(REWIND_OP_READ, TRUE, rwint.cbuffer.snaps);
	} else {
		rewind_update_chunk_snaps(&rwint.cbuffer, index.chunk, src);

		while (index.snap <= snaps) {
			snap = rwint.cbuffer.snaps + index.snap;
			snap->index.chunk = index.chunk;
			snap->index.snap = index.snap;

			rewind_operation(REWIND_OP_READ, TRUE, snap);

			if ((index.snap == snaps) && (exec_last_frame == FALSE)) {
				break;
			}

			rewind_execute_frame();
			index.snap++;
		}
	}

	rewind_update_chunk_snaps(&rwint.segment, index.chunk, rwint.cbuffer.data);

	rwint.index.chunk = index.chunk;
	rwint.index.snap = index.snap;

	rwint.snap_cursor = cursor;

	return (EXIT_OK);
}
