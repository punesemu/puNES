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

#ifndef COMPILATION_UNIT_ORPHAN_H_
#define COMPILATION_UNIT_ORPHAN_H_

#include "conf.h"
_config *cfg;
_config cfg_from_file;

#include "tas.h"
_tas tas;

void (*tas_header)(uTCHAR *file);
void (*tas_read)(void);
void (*tas_frame)(void);
void (*tas_rewind)(int32_t frames_to_rewind);
void (*tas_restart_from_begin)(void);

#include "gui.h"
_gui gui;
_gui_mouse gmouse;
_external_windows ext_win;

double (*gui_get_ms)(void);

#include "info.h"
_info info;

#include "clock.h"
_machine machine;

#include "vs_system.h"
_vs_system vs_system;

#include "debugger.h"
_debugger debugger;

#endif /* COMPILATION_UNIT_ORPHAN_H_ */
