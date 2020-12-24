/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QFileInfo>
#include "tas.h"
#include "emu.h"
#include "info.h"
#include "gui.h"
#include "ppu.h"
#include "rewind.h"

#define tas_set_data_port_ctrlstd(prt, dt)\
	prt.data[dt] = tas.il[tas.index].prt[dt]
#define tas_increment_index()\
	if (++tas.index == tas.count) {\
		tas_read();\
	}

static _port tas_port_bck[PORT_MAX];
struct _tas_internal {
	unsigned int count;
	QList<size_t> file_byte_il;
	uint32_t index;
} tsint;

BYTE tas_file(uTCHAR *ext, uTCHAR *file) {
	QString extension = uQString(ext);

	if (extension.compare(".fm2", Qt::CaseInsensitive) == 0) {
		tas.type = FM2;
		tas_header = tas_header_FM2;
		tas_read = tas_read_FM2;
		tas_frame = tas_frame_FM2;
		tas_rewind = tas_rewind_FM2;
		tas_restart_from_begin = tas_restart_from_begin_FM2;
	}

	if (tas.type != NOTAS) {
		BYTE found = FALSE;

		{
			BYTE i;

			for (i = PORT1; i < PORT_MAX; i++) {
				memcpy(&tas_port_bck[i], &port[i], sizeof(_port));
			}
		}

		umemset(tas.file, 0x00, usizeof(tas.file));
		ustrncpy(tas.file, file, usizeof(tas.file) - 1);

		tas.fp = ufopen(file, uL("rb"));

		tsint.count = 0;
		tsint.file_byte_il.clear();
		tsint.index = 0;

		tas_header(file);

		{
			BYTE i;
			const QString rom_ext[4] = { ".nes", ".NES", ".fds", ".FDS" };

			for (i = 0; i < LENGTH(rom_ext); i++) {
				QString rom = uQString(info.rom.file) + rom_ext[i];

				if (QFileInfo(rom).exists()) {
					umemset(info.rom.file, 0x00, usizeof(info.rom.file));
					ustrncpy(info.rom.file, uQStringCD(rom), usizeof(info.rom.file) - 1);
					found = TRUE;
					break;
				}
			}
		}

		if (found) {
			tas_read();
			tsint.index = 0;
			tas.index = -1;
			tas.frame = -1;
		} else {
			tas.file[0] = 0;
			info.rom.file[0] = 0;
			tas_quit();
		}
	}
	return (EXIT_OK);
}
void tas_quit(void) {
	BYTE i;

	tas.type = NOTAS;

	if (tas.fp) {
		fclose(tas.fp);
		tas.fp = NULL;
	}

	tas_header = NULL;
	tas_read = NULL;
	tas_frame = NULL;

	for (i = PORT1; i < PORT_MAX; i++) {
		memcpy(&port[i], &tas_port_bck[i], sizeof(_port));
	}

	input_init(NO_SET_CURSOR);

	tsint.file_byte_il.clear();
}

void tas_header_FM2(uTCHAR *file) {
	QString line;
	char buffer[1024];
	int32_t counter;
	size_t pos;

	tas.emulator = FCEUX;

	tas.frame = 0;
	tas.total = 0;

	counter = 0;
	pos = ftell(tas.fp);

	while (fgets(buffer, sizeof(buffer), tas.fp)) {
		QString key, value;

		// elimino spazi iniziali, tabulazioni e ritorno a capo
		line = QString::fromUtf8(buffer).simplified();

		if (line.isEmpty() || line.startsWith('#')) {
			pos = ftell(tas.fp);
			continue;
		}

		key = line.section(" ", 0, 0);
		value = line.section(" ", 1);

		if (!key.startsWith('|') && value.isEmpty()) {
			pos = ftell(tas.fp);
			continue;
		}

		if (key.startsWith('|')) {
			tas.total++;
			if (counter == 0) {
				tsint.file_byte_il.append(pos);
				tsint.count++;
			}
			if (++counter == LENGTH(tas.il)) {
				counter = 0;
			}
		} else if (key.compare("emulator", Qt::CaseInsensitive) == 0) {
			if (value.compare("punes", Qt::CaseInsensitive) == 0) {
				tas.emulator = PUNES;
			}
		} else if (key.compare("emuVersion", Qt::CaseInsensitive) == 0) {
			tas.emu_version = value.toInt();
		} else if (key.compare("punesStartFrame", Qt::CaseInsensitive) == 0) {
			tas.start_frame = value.toInt() + 1;
		} else if (key.compare("romFilename", Qt::CaseInsensitive) == 0) {
			QString rom = QFileInfo(uQString(file)).absolutePath() + "/" + value;

			umemset(info.rom.file, 0x00, usizeof(info.rom.file));
			ustrncpy(info.rom.file, uQStringCD(rom), usizeof(info.rom.file) - 1);
		} else if (key.compare("port0", Qt::CaseInsensitive) == 0) {
			port[PORT1].type = value.toInt();
			if (port[PORT1].type == CTRL_ZAPPER) {
				info.zapper_is_present = TRUE;
			}
		} else if (key.compare("port1", Qt::CaseInsensitive) == 0) {
			port[PORT2].type = value.toInt();
			if (port[PORT2].type == CTRL_ZAPPER) {
				info.zapper_is_present = TRUE;
			}
		}

		pos = ftell(tas.fp);
	}

	if (tas.emulator == FCEUX) {
		info.r4014_precise_timing_disabled = TRUE;
		// nell'FCEUX viene saltato il primo vblank (flag ppudead)
		info.r2002_jump_first_vblank = TRUE;
		//if (tas.emu_version <= 9828) {
			info.r2002_race_condition_disabled = TRUE;
			info.r4016_dmc_double_read_disabled = TRUE;
		//}
	}

	fseek(tas.fp, 0, SEEK_SET);
}
void tas_read_FM2(void) {
	unsigned int start;
	char line[256], *sep, *saveptr;

	tas.count = tas.index = 0;

	while (fgets(line, sizeof(line), tas.fp)) {
		for (start = 0; start < strlen(line); start++) {
			if ((line[start] == ' ') || (line[start] == '\t')) {
				continue;
			}
			break;
		}
		if (line[start] != '|') {
			continue;
		}

		start++;

		sep = strtok_r(line + start, "|", &saveptr);

		tas.il[tas.count].state = atoi(sep);

		{
			BYTE a, b;

			for (a = PORT1; a <= PORT2; a++) {
				sep = strtok_r(NULL, "|", &saveptr);

				if (port[a].type == CTRL_STANDARD) {
					for (b = 0; b < 8; b++) {
						tas.il[tas.count].port[a][RIGHT - b] = PRESSED;

						if ((sep[b] == ' ') || (sep[b] == '.')) {
							tas.il[tas.count].port[a][RIGHT - b] = RELEASED;
						}
					}
				} else if (port[a].type == CTRL_ZAPPER) {
					char *space, *last;

					space = strtok_r(sep, " ", &last);
					tas.il[tas.count].port[a][0] = QString::fromUtf8(space).simplified().toUInt();
					space = strtok_r(NULL, " ", &last);
					tas.il[tas.count].port[a][1] = QString::fromUtf8(space).simplified().toUInt();
					space = strtok_r(NULL, " ", &last);
					tas.il[tas.count].port[a][2] = QString::fromUtf8(space).simplified().toUInt();
					space = strtok_r(NULL, " ", &last);
					tas.il[tas.count].port[a][3] = QString::fromUtf8(space).simplified().toUInt();
				}
			}
		}

		if (++tas.count == LENGTH(tas.il)) {
			break;
		}
	}

	tsint.index++;
}
void tas_frame_FM2(void) {
	// il primo frame
	if (!tas.frame) {
		gui_overlay_info_append_msg_precompiled(20, NULL);
		//tas_increment_index()
	}

	if (++tas.frame >= tas.total) {
		if (tas.frame == tas.total) {
			gui_overlay_info_append_msg_precompiled(21, NULL);
		} else if (tas.frame == tas.total + 10) {
			// nel tas_quit() eseguo il ripristino delle porte e l'input_init() solo che questo
			// cambia lo stato delle pulsanti e in alcuni film (aglar-marblemadness.fm2)
			// questo influisce su i frames immediatamente successivi la fine del film (mentre l'input
			// non dovrebbe subire modifiche rispetto all'ultimo frame) "sporcandoli" e non permettendo
			// il completamento del finale. Per questo motivo ritardo di qualche frame il tas_quit().
			tas_quit();
		}
		return;
	}

	tas_increment_index()

	if (tas.il[tas.index].state > 0) {
		if (tas.il[tas.index].state == 1) {
			emu_reset(RESET);
		} else if (tas.il[tas.index].state == 2) {
			emu_reset(HARD);
		}
	}

	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			if (port[i].type == CTRL_STANDARD) {
				tas_set_data_port_ctrlstd(port[i], BUT_A);
				tas_set_data_port_ctrlstd(port[i], BUT_B);
				tas_set_data_port_ctrlstd(port[i], SELECT);
				tas_set_data_port_ctrlstd(port[i], START);
				tas_set_data_port_ctrlstd(port[i], UP);
				tas_set_data_port_ctrlstd(port[i], DOWN);
				tas_set_data_port_ctrlstd(port[i], LEFT);
				tas_set_data_port_ctrlstd(port[i], RIGHT);
			} else if (port[i].type == CTRL_ZAPPER) {
				gmouse.x = tas.il[tas.index].port[i][0];
				gmouse.y = tas.il[tas.index].port[i][1];
				gmouse.left = tas.il[tas.index].port[i][2];
				if ((gmouse.left == 0) && tas.il[tas.index].port[i][3]) {
					gmouse.left = tas.il[tas.index].port[i][3];
				}
				gmouse.right = 0;
			}
		}
	}
}
void tas_rewind_FM2(int32_t frames_to_rewind) {
	uint32_t frames, chunk, snaps;

	frames = tas.frame + frames_to_rewind;
	chunk = frames / LENGTH(tas.il);
	snaps = frames % LENGTH(tas.il);

	if (chunk != tsint.index) {
		fseek(tas.fp, tsint.file_byte_il.at(chunk), SEEK_SET);
		tas_read();
		tsint.index = chunk;
	}

	tas.frame = frames;
	tas.index = snaps;
}
void tas_restart_from_begin_FM2(void) {
	if (tas.type != NOTAS) {
		tsint.count = 0;
		tsint.file_byte_il.clear();
		tsint.index = 0;

		tas.frame = 0;

		fseek(tas.fp, 0, SEEK_SET);

		tas_read();

		tsint.index = 0;
		tas.index = -1;
		tas.frame = -1;
	}
}
