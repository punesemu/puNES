/*
 * settings.cpp
 *
 *  Created on: 24/ott/2014
 *      Author: fhorse
 */

#include "settingsObject.hpp"
#include <QtCore/QFileInfo>
#include "conf.h"

#define CFGFILENAME "/puNES.cfg"
#define PGSFILENAME QString(PERGAME_FOLDER) + "/" +\
	QFileInfo(info.rom_file).completeBaseName() + ".pgs"
#define INPFILENAME "/input.cfg"

_emu_settings s;

void settings_init(void) {
	cfg = &cfg_from_file;

	memset(&s, 0x00, sizeof(_emu_settings));

	s.cfg = QSettings::registerFormat("cfg", rd_cfg_file, wr_cfg_file);
	s.list = LSET_SET;
	s.set = new setObject(s.cfg, QString(CFGFILENAME), LSET_SET);
	s.list = LSET_INP;
	s.inp = new inpObject(s.cfg, QString(INPFILENAME), LSET_INP);
}
void settings_save(void) {
	s.set->wr();
	s.inp->wr();
}
void settings_save_GUI(void) {
	s.set->wr("GUI");
}
void settings_set_overscan_default(_overscan_borders *ob, BYTE mode) {
	s.set->oscan_default(ob, mode);
}
int settings_val_to_int(int index, const char *buffer) {
	return (s.set->val_to_int(index, buffer));
}
double settings_val_to_double(WORD round, const char *buffer) {
	return (s.set->val_to_double(round, buffer));
}
void settings_val_to_oscan(int index, _overscan_borders *ob, const char *buffer) {
	s.set->oscan_val_to_int(index, ob, buffer);
}

void settings_pgs_parse(void) {
	/* game genie */
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		return;
	}

	if (!info.rom_file[0]) {
		return;
	}

	if (s.pgs) {
		delete(s.pgs);
		s.pgs = NULL;
	}

	s.list = LSET_PGS;
	s.pgs = new pgsObject(s.cfg, PGSFILENAME, LSET_PGS);
}
void settings_pgs_save(void) {
	if (s.pgs) {
		s.pgs->wr();
	}
}

void *settings_inp_sc_ks(int index) {
	return (s.inp->val_to_qstring_pntr(index));
}
void settings_inp_all_default(_config_input *config_input, _array_pointers_port *array) {
	s.inp->set_all_input_default(config_input, array);
}
void settings_inp_port_default(_port *port, int index, int mode) {
	s.inp->set_kbd_joy_default(port, index, mode);
}
void settings_inp_save(void) {
	s.inp->wr();
}
