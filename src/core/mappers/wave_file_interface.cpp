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

#include "wave_file_interface.h"
#include "waveFile.h"
#define EXTERNC extern "C"

EXTERNC void wavefiles_clear(void) {
	waveFiles.clear();
}
EXTERNC void wavefiles_restart(int index) {
	if (index < (int)waveFiles.size()) {
		waveFiles[index].restart();
	}
}
EXTERNC int wavefiles_get_next_sample(int index) {
	return (index < (int)waveFiles.size() ? waveFiles[index].getNextSample() : 0);
}
EXTERNC BYTE wavefiles_is_finished(int index) {
	return (index < (int)waveFiles.size() ? waveFiles[index].isFinished() : (BYTE)TRUE);
}

#undef EXTERNC
