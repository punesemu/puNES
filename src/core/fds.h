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

#ifndef FDS_H_
#define FDS_H_

#include <stdio.h>
#include "common.h"

enum fds_formats { FDS_FORMAT_RAW, FDS_FORMAT_FDS };
enum fds_write_mode { FDS_WR_DIFF_FILE, FDS_WR_ORIGINAL_FILE };
enum fds_operations { FDS_OP_NONE, FDS_OP_READ, FDS_OP_WRITE };
enum fds_disk_operations {
	FDS_DISK_INSERT,
	FDS_DISK_EJECT,
	// e' importante che tutte le modalita'
	// SELECT siano dopo la FDS_DISK_SELECT.
	FDS_DISK_SELECT,
	FDS_DISK_SELECT_AND_INSERT,
	FDS_DISK_SELECT_FROM_REWIND
};
enum fds_gaps {
	FDS_GAP_START = 28300 / 8,
	// 1016 bit di gap alla fine di ogni blocco.
	// Note : con 976 funziona correttamente la read del disco ma non e'
	// sufficiente per la write.
	//FDS_GAP_BLOCK = 1016 / 8, //976 / 8,
	FDS_GAP_BLOCK = 976 / 8,
	FDS_GAP_END = 0
};
enum fds_block_type {
	BL_DISK_INFO = 1,
	BL_FILE_AMOUNT,
	BL_FILE_HEADER,
	BL_FILE_DATA,
	DISK_FDS_SIDE_SIZE = 65500,
	DISK_QD_SIDE_SIZE = 65536
};
enum fds_misc {
	//FDS_8BIT_DELAY = 149, //20 * 8,
	FDS_8BIT_DELAY = 22 * 8,
	FDS_DISK_GAP = 0x0100,
	FDS_DISK_BLOCK_MARK = 0x0180,
	FDS_DISK_CRC_CHAR1 = 0x0155,
	FDS_DISK_CRC_CHAR2 = 0x01AA,
	FDS_OP_SIDE_DELAY = 2800000,
	FDS_AUTOINSERT_OP_SIDE_DELAY = 100,
	FDS_AUTOINSERT_R4032_MAX_CHECKS = 150
};

#define fds_auto_insert_enabled() (cfg->fds_switch_side_automatically & !fds.auto_insert.disabled & !fds.info.bios_first_run)
#define fds_reset_envelope_counter(env) (fds.snd.envelope.speed << 3) * (fds.snd.env.speed + 1)
#define fds_sweep_bias(val) (SBYTE)((val & 0x7F) << 1) / 2;

typedef struct _fds_info_side {
	BYTE side;
	WORD *data;
	uint32_t size;
	uint32_t last_position;
} _fds_info_side;
typedef struct _fds {
	struct _fds_info {
		BYTE enabled;
		BYTE *data;
		WORD *image;
		FILE *diff;
		BYTE writings_occurred;
		BYTE total_sides;
		BYTE expcted_side;
		BYTE type;
		uint32_t total_size;
		BYTE last_operation;
		BYTE bios_first_run;
		BYTE frame_insert;
		_fds_info_side sides[20];
	} info;
	struct _fds_side {
		struct _fds_side_change {
			BYTE new_side;
			uint32_t delay;
		} change;
		_fds_info_side *info;
	} side;
	// le variabili da salvare nei savestate
	struct _fds_drive {
		uint32_t disk_position;
		uint32_t delay;
		BYTE disk_ejected;
		BYTE side_inserted;
		BYTE gap_ended;
		BYTE end_of_head;
		BYTE scan;
		BYTE crc_char;
		BYTE enabled_dsk_reg;
		BYTE enabled_snd_reg;
		BYTE data_readed;
		BYTE data_to_write;
		// anche se continuo a salvarlo nel save_slot.c, questa
		// variabile non e' piu' utilizzata. quindi se servisse
		// potrebbe essere riciclata per qualche altra cosa.
		BYTE transfer_flag;
		BYTE motor_on;
		BYTE transfer_reset;
		BYTE read_mode;
		BYTE mirroring;
		BYTE crc_control;
		BYTE unknow;
		BYTE drive_ready;
		BYTE irq_disk_enabled;
		BYTE at_least_one_scan;
		BYTE irq_timer_enabled;
		BYTE irq_timer_reload_enabled;
		BYTE irq_timer_high;
		WORD irq_timer_reload;
		WORD irq_timer_counter;
		BYTE irq_timer_delay;
		BYTE data_external_connector;
		/* per usi futuri */
		BYTE filler[30];
	} drive;
	// snd
	struct _fds_snd {
		struct _fds_snd_wave {
			BYTE data[64];
			BYTE writable;
			BYTE volume;

			BYTE index;
			int32_t counter;

			// -------------------------------------------------------
			// questi valori non e' necessario salvarli nei savestates
			// -------------------------------------------------------
			BYTE clocked;
			// -------------------------------------------------------
		} wave;
		struct _fds_snd_envelope {
			BYTE speed;
			BYTE disabled;
		} envelope;
		struct _fds_snd_main {
			BYTE silence;
			WORD frequency;

			SWORD output;
		} main;
		struct _fds_snd_volume {
			BYTE speed;
			BYTE mode;
			BYTE increase;

			BYTE gain;
			uint32_t counter;
		} volume;
		struct _fds_snd_sweep {
			SBYTE bias;
			BYTE mode;
			BYTE increase;
			BYTE speed;

			BYTE gain;
			uint32_t counter;
		} sweep;
		struct _fds_snd_modulation {
			SBYTE data[64];
			WORD frequency;
			BYTE disabled;

			BYTE index;
			int32_t counter;
			SWORD mod;
		} modulation;
	} snd;
	// auto insert
	struct _fds_auto_insert {
		struct _fds_auto_insert_r4032 {
			uint32_t frames;
			uint32_t checks;
		} r4032;
		struct _fds_auto_insert_delay {
			int32_t eject;
			int32_t dummy;
			int32_t side;
		} delay;
		struct _fds_auto_insert_rE445 {
			BYTE in_run;
			BYTE count;
		} rE445;
		BYTE disabled;
		BYTE new_side;
		BYTE in_game;
	} auto_insert;
} _fds;

extern _fds fds;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void fds_init(void);
EXTERNC void fds_quit(void);
EXTERNC BYTE fds_load_rom(BYTE type);
EXTERNC BYTE fds_load_bios(void);
EXTERNC void fds_info(void);
EXTERNC void fds_info_side(BYTE side);
EXTERNC void fds_disk_op(WORD type, BYTE side_to_insert, BYTE quiet);
EXTERNC void fds_diff_op(BYTE side, BYTE mode, uint32_t position, WORD value);
EXTERNC BYTE fds_from_image(uTCHAR *file, BYTE format, BYTE type);
EXTERNC BYTE fds_image_to_file(uTCHAR *file);
EXTERNC uint32_t fds_disk_side_size_format(BYTE format);
EXTERNC uint32_t fds_disk_side_size(void);
EXTERNC uint32_t fds_image_side_size(void);
EXTERNC uint32_t fds_image_side_bytes(void);

#undef EXTERNC

#endif /* FDS_H_ */
