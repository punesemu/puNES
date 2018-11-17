/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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
#include "objSettings.hpp"
#include "conf.h"

#define CFGFILENAME "/puNES.cfg"
#define INPFILENAME "/input.cfg"
#define PGSFILENAME QString(PERGAME_FOLDER) + "/" +\
	QFileInfo(uQString(info.rom.file)).completeBaseName() + ".pgs"
#define SHPFILENAME QString(SHDPAR_FOLDER) + "/" +\
	QFileInfo(uQString(cfg->shader_file)).fileName() + ".shp"

_emu_settings s;

void settings_init(void) {
	cfg = &cfg_from_file;

	memset(&s, 0x00, sizeof(_emu_settings));

	s.cfg = QSettings::registerFormat("cfg", rd_cfg_file, wr_cfg_file);
	s.list = LSET_SET;
	s.set = new objSet(s.cfg, QString(CFGFILENAME), LSET_SET);
	s.list = LSET_INP;
	s.inp = new objInp(s.cfg, QString(INPFILENAME), LSET_INP);
}
void settings_save(void) {
	s.list = LSET_SET;
	s.set->wr();
	settings_inp_save();
}
void settings_save_GUI(void) {
	s.list = LSET_SET;
	s.set->wr("GUI");
}
void settings_set_overscan_default(_overscan_borders *ob, BYTE mode) {
	s.set->oscan_default(ob, mode);
}
int settings_val_to_int(int index, const uTCHAR *buffer) {
	return (s.set->val_to_int(index, buffer));
}
double settings_val_to_double(WORD round, const uTCHAR *buffer) {
	return (s.set->val_to_double(round, buffer));
}
void settings_cpy_utchar_to_val(int index, uTCHAR *buffer) {
	s.set->cpy_utchar_to_val(index, buffer);
}
void settings_val_to_oscan(int index, _overscan_borders *ob, const uTCHAR *buffer) {
	s.set->oscan_val_to_int(index, ob, buffer);
}

void *settings_inp_rd_sc(int index, int type) {
	return (s.inp->sc_val_to_qstring_pntr(index, type));
}
void settings_inp_wr_sc(void *str, int index, int type) {
	s.inp->sc_qstring_pntr_to_val(str, index, type);
}
void settings_inp_all_default(_config_input *config_input, _array_pointers_port *array) {
	s.inp->set_all_input_default(config_input, array);
}
void settings_inp_port_default(_port *port, int index, int mode) {
	s.inp->set_kbd_joy_default(port, index, mode);
}
void settings_inp_save(void) {
	s.list = LSET_INP;
	s.inp->wr();
}

void settings_pgs_parse(void) {
	// game genie
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		return;
	}

	if (info.no_rom) {
		return;
	}

	if (s.pgs) {
		delete(s.pgs);
		s.pgs = NULL;
	}

	s.list = LSET_PGS;
	s.pgs = new objPgs(s.cfg, PGSFILENAME, LSET_PGS);

	if (cfg->ppu_overclock) {
		text_add_line_info(1,
				"overclock enabled [font8]([green]VB[normal] [cyan]%d,[green]PR[normal] [cyan]%d[normal])",
				cfg->extra_vb_scanlines, cfg->extra_pr_scanlines);
	}
}
void settings_pgs_save(void) {
	if (s.pgs) {
		s.list = LSET_PGS;
		s.pgs->wr();
	}
}

void settings_shp_parse(void) {
	QString file;

	if (s.shp) {
		delete(s.shp);
		s.shp = NULL;
	}

	switch (cfg->shader) {
		case NO_SHADER:
			return;
		case SHADER_CRTDOTMASK:
			file = QString(SHDPAR_FOLDER) + "/" + "crt_dotmask.internal.shp";
			break;
		case SHADER_CRTSCANLINES:
			file = QString(SHDPAR_FOLDER) + "/" + "crt_scanlines.internal.shp";
			break;
		case SHADER_CRTWITHCURVE:
			file = QString(SHDPAR_FOLDER) + "/" + "crt_with_curve.internal.shp";
			break;
		case SHADER_EMBOSS:
			file = QString(SHDPAR_FOLDER) + "/" + "emboss.internal.shp";
			break;
		case SHADER_NOISE:
			file = QString(SHDPAR_FOLDER) + "/" + "noise.internal.shp";
			break;
		case SHADER_NTSC2PHASECOMPOSITE:
			file = QString(SHDPAR_FOLDER) + "/" + "ntsc_2_phase_composite.internal.shp";
			break;
		case SHADER_OLDTV:
			file = QString(SHDPAR_FOLDER) + "/" + "old_tv.internal.shp";
			break;
		case SHADER_FILE:
			file = SHPFILENAME;
			break;
	}

	s.list = LSET_NONE;
	s.shp = new objShp(s.cfg, file, LSET_NONE);
}
void settings_shp_save(void) {
	if (s.shp) {
		s.list = LSET_NONE;
		s.shp->wr();
	}
}
