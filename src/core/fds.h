/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#include "common.h"

enum fds_formats { FDS_TYPE_RAW, FDS_TYPE_FDS };
enum fds_write_mode { FDS_WR_DIFF_FILE, FDS_WR_ORIGINAL_FILE };
enum fds_operations { FDS_OP_NONE, FDS_OP_READ, FDS_OP_WRITE };
enum fds_disk_operations {
	FDS_DISK_INSERT,
	FDS_DISK_EJECT,
	// e' importante che tutte le modalita' SELECT siano dopo la FDS_DISK_SELECT.
	FDS_DISK_SELECT,
	FDS_DISK_SELECT_AND_INSERT,
	FDS_DISK_SELECT_FROM_REWIND
};
enum fds_gaps {
	FDS_GAP_START = 28300 / 8,
	FDS_GAP_BLOCK = 976 / 8,
	FDS_GAP_FILE_BLOCK = 32 / 8
};
enum fds_block_type {
	BL_DISK_INFO = 1,
	BL_FILE_AMOUNT,
	BL_FILE_HEADER,
	BL_FILE_DATA,
};
enum fds_misc {
	FDS_DISK_GAP = 0x00,
	FDS_DISK_BLOCK_MARK = 0x80,
	FDS_AUTOINSERT_R4032_MAX_CHECKS = 7,
	FDS_MIN_LAG_FRAMES = 20,
	FDS_IMAGE_SIDE_SIZE = 75500,
	DISK_FDS_SIDE_SIZE = 65500,
	DISK_QD_SIDE_SIZE = 65536,
};

// https://www.chrismcovell.com/fds-lister.html
// The theoretical ideal data rate for FDS disks is said to be 96400 bits per second, or 12050 bytes/sec.
// Inverted, that's 1 byte every 82.988 microseconds.  With the Famicom CPU having a clock period of 559 ns/cycle,
// there should be a new byte coming in to the FDS RAM adaptor every 148.46 (~hex $94) CPU clock cycles.
// At an average of 1 byte every 147-150 CPU cycles that I get on my newly-"calibrated" disk drive, that comes pretty
// close to the ideal data rate of 148 listed above.
#define FDS_8BIT_MS_DELAY 0.082988f
// Aspic (1988)(Bothtec)(J) necessita di almeno 1500 ms
// Pulsar no Hikari - Space Wars Simulation (Japan) di almeno 1600 ms
#define FDS_OP_SIDE_MS_DELAY 1600.0f
// Kosodate Gokko (Japan) (Unl) ha un delay di 78 ms
#define FDS_INSERT_MS_DELAY 74.0f

#define fds_auto_insert_enabled() (cfg->fds_switch_side_automatically & !fds.auto_insert.disabled & !fds.bios.first_run)
#define fds_reset_envelope_counter(env) (fds.snd.envelope.speed << 3) * (fds.snd.env.speed + 1)
#define fds_sweep_bias(val) (SBYTE)((val & 0x7F) << 1) / 2;

typedef struct _fds_info_side {
	BYTE side;
	BYTE *data;
	uint32_t size;
} _fds_info_side;
typedef struct _fds_info_bios {
	uTCHAR file[LENGTH_FILE_NAME_LONG];
	uint32_t crc32;
	BYTE first_run;
} _fds_info_bios;
typedef struct _fds_info_protection {
	BYTE autodetect;
	BYTE magic_card_trainer;
	BYTE quick_hunter;
	BYTE ouji;
	BYTE kgk;
} _fds_info_protection;
typedef struct _fds_info {
	BYTE enabled;
	BYTE *data;
	BYTE *image;
	BYTE write_protected;
	BYTE writings_occurred;
	BYTE total_sides;
	BYTE expcted_sides;
	BYTE format;
	BYTE type;
	uint32_t total_size;
	BYTE last_operation;
	BYTE frame_insert;
	uint32_t cycles_8bit_delay;
	uint32_t cycles_insert_delay;
	uint32_t cycles_dummy_delay;
	_fds_info_protection protection;
	_fds_info_side sides[20];
} _fds_info;
typedef struct _fds {
	_fds_info info;
	_fds_info_bios bios;
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
		uint32_t delay_insert;
		uint32_t delay_8bit;
		BYTE disk_ejected;
		BYTE side_inserted;
		BYTE mark_finded;
		BYTE end_of_head;
		BYTE scan;
		BYTE crc_control;
		WORD crc;
		BYTE enabled_dsk_reg;
		BYTE enabled_snd_reg;
		BYTE data_io;
		BYTE data_available;
		BYTE transfer_flag;
		BYTE motor_stop;
		BYTE transfer_reset;
		BYTE motor_started;
		BYTE io_mode;
		BYTE mirroring;
		BYTE unknow;
		BYTE crc_enabled;
		BYTE irq_disk_enabled;
		BYTE irq_timer_enabled;
		BYTE irq_timer_reload_enabled;
		BYTE irq_timer_high;
		WORD irq_timer_reload;
		WORD irq_timer_counter;
		BYTE irq_timer_delay;
		BYTE data_external_connector;
		BYTE scan_disabled;
		/* per usi futuri */
		BYTE filler[29];
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
			BYTE disabled;
			uint32_t frames;
			uint32_t checks;
		} r4032;
		struct _fds_auto_insert_end_of_head {
			BYTE disabled;
		} end_of_head;
		struct _fds_auto_insert_delay {
			int32_t dummy;
		} delay;
		struct _fds_auto_insert_rE445 {
			BYTE in_run;
		} rE445;
		BYTE disabled;
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
EXTERNC BYTE fds_load_rom(BYTE format);
EXTERNC BYTE fds_load_bios(void);
EXTERNC BYTE fds_create_empty_disk(uTCHAR *file, BYTE format, BYTE type, BYTE double_side);
EXTERNC BYTE fds_change_disk(uTCHAR *file);
EXTERNC void fds_info(void);
EXTERNC void fds_info_side(BYTE side);
EXTERNC void fds_disk_op(WORD type, BYTE side_to_insert, BYTE quiet);
EXTERNC void fds_diff_to_file(void);
EXTERNC BYTE fds_from_image_to_file(uTCHAR *file, BYTE format, BYTE type);
EXTERNC BYTE fds_image_to_file(uTCHAR *file);
EXTERNC WORD fds_crc_byte(WORD base, BYTE data);
EXTERNC uint32_t fds_disk_side_size(BYTE format);
EXTERNC uint32_t fds_image_side_size(void);

#undef EXTERNC

#endif /* FDS_H_ */
