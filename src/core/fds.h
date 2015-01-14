/*
 * fds.h
 *
 *  Created on: 25/mar/2012
 *      Author: fhorse
 */

#ifndef FDS_H_
#define FDS_H_

#include "common.h"

enum fds_formats { FDS_FORMAT_RAW, FDS_FORMAT_FDS };
enum fds_operations { FDS_OP_NONE, FDS_OP_READ, FDS_OP_WRITE };
enum fds_disk_memory_operations { FDS_DISK_MEMSET, FDS_DISK_MEMCPY };
enum fds_disk_operations {
	FDS_DISK_COUNT,
	FDS_DISK_INSERT,
	FDS_DISK_EJECT,
	/*
	 * e' importante che tutte le modalita'
	 * SELECT siano dopo la FDS_DISK_SELECT.
	 */
	FDS_DISK_SELECT,
	FDS_DISK_SELECT_AND_INSERT,
	FDS_DISK_TIMELINE_SELECT
};
enum fds_block_type {
	BL_DISK_INFO = 1,
	BL_FILE_AMOUNT,
	BL_FILE_HEADER,
	BL_FILE_DATA,
	DISK_SIDE_SIZE = 65500
};
enum fds_misc {
	FDS_DISK_GAP = 0x0100,
	FDS_DISK_BLOCK_MARK = 0x0180,
	FDS_DISK_CRC_CHAR1 = 0x0155,
	FDS_DISK_CRC_CHAR2 = 0x01AA
};

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _fds {
	/* generali */
	struct {
		BYTE enabled;
		FILE *fp;
		FILE *diff;
		BYTE total_sides;
		BYTE type;
		uint32_t total_size;
		uint32_t sides_size[20];
		BYTE last_operation;
	} info;
	/* side */
	struct {
		struct {
			uint32_t position;
		} block_1;
		struct {
			uint32_t position;
			BYTE tot_files;
		} block_2;
		struct {
			struct {
				uint32_t position;
				uint32_t length;
			} block_3;
			struct {
				uint32_t position;
			} block_4;
		} file[0xFF];
		WORD *data;
		uint32_t counted_files;
	} side;
	/* le variabili da salvare nei savestate */
	struct {
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
		/*
		 * anche se continuo a salvarlo nel save_slot.c, questa
		 * variabile non e' piu' utilizzata. quindi se servisse
		 * potrebbe essere riciclata per qualche altra cosa.
		 */
		BYTE transfer_flag;
		BYTE motor_on;
		BYTE transfer_reset;
		BYTE read_mode;
		BYTE mirroring;
		BYTE crc_control;
		BYTE unknow;
		BYTE drive_ready;
		BYTE irq_disk_enabled;
		BYTE irq_disk_high;
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
	/* snd */
	struct {
		struct {
			BYTE data[64];
			BYTE writable;
			BYTE volume;

			BYTE index;
			int32_t counter;

		/* ------------------------------------------------------- */
		/* questi valori non e' necessario salvarli nei savestates */
		/* ------------------------------------------------------- */
		/* */ BYTE clocked;                                     /* */
		/* ------------------------------------------------------- */
		} wave;
		struct {
			BYTE speed;
			BYTE disabled;
		} envelope;
		struct {
			BYTE silence;
			WORD frequency;

			SWORD output;
		} main;
		struct {
			BYTE speed;
			BYTE mode;
			BYTE increase;

			BYTE gain;
			uint32_t counter;
		} volume;
		struct {
			SBYTE bias;
			BYTE mode;
			BYTE increase;
			BYTE speed;

			BYTE gain;
			uint32_t counter;
		} sweep;
		struct {
			SBYTE data[64];
			WORD frequency;
			BYTE disabled;

			BYTE index;
			int32_t counter;
			SWORD mod;
		} modulation;
	} snd;
} fds;

EXTERNC void fds_init(void);
EXTERNC void fds_quit(void);
EXTERNC BYTE fds_load_rom(void);
EXTERNC BYTE fds_load_bios(void);
EXTERNC void fds_disk_op(WORD type, BYTE side_to_insert);
EXTERNC void fds_diff_op(BYTE mode, uint32_t position, WORD value);

#undef EXTERNC

#endif /* FDS_H_ */
