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

#include "mainWindow.hpp"
#include "recentFiles.hpp"
#include "recent_files.h"
#include "gui.h"

// ----------------------------------------------------------------------------------------------

recentFiles recent_roms;

void recent_roms_init(void) {
	recent_roms.init(QString("%0%1").arg(uQString(gui_config_folder())).arg(QString(RECENTROMSFILENAME)));
}
void recent_roms_add(uTCHAR *file) {
	recent_roms.add(file);
	mainwin->wd->update_menu_recent_roms();
}
void recent_roms_parse(void) {
	recent_roms.parse();
}
void recent_roms_save(void) {
	recent_roms.save();
}
int recent_roms_count(void) {
	return (recent_roms.count());
}
const char *recent_roms_item(const int index) {
	return (recent_roms.item(index));
}
int recent_roms_item_size(const int index) {
	return (recent_roms.item_size(index));
}
const char *recent_roms_current(void) {
	return (recent_roms.current());
}
int recent_roms_current_size(void) {
	return (recent_roms.current_size());
}

// ----------------------------------------------------------------------------------------------

recentFiles recent_disks;

void recent_disks_init(void) {
	recent_disks.init(QString("%0%1").arg(uQString(gui_config_folder())).arg(QString(RECENTDISKSFILENAME)));
}
void recent_disks_add(uTCHAR *file) {
	recent_disks.add(file);
	mainwin->wd->update_menu_recent_disks();
}
void recent_disks_parse(void) {
	recent_disks.parse();
}
void recent_disks_save(void) {
	recent_disks.save();
}
int recent_disks_count(void) {
	return (recent_disks.count());
}
const char *recent_disks_item(const int index) {
	return (recent_disks.item(index));
}
int recent_disks_item_size(const int index) {
	return (recent_disks.item_size(index));
}
const char *recent_disks_current(void) {
	return (recent_disks.current());
}
int recent_disks_current_size(void) {
	return (recent_disks.current_size());
}
