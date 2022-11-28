/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWidget>
#include <QtGui/QKeyEvent>
#include <QtGui/QKeySequence>
#include "mainApplication.hpp"
#include "mainWindow.hpp"
#include "dlgWizard.hpp"
#include "dlgKeyboard.hpp"
#include "version.h"
#include "gui.h"

//#define FONT_SIZE 9

mainApplication::mainApplication(int &argc, char *argv[], bool allowSecondary, Options options, int timeout, const QString &userData) :
	SingleApplication(argc, argv, allowSecondary, options, timeout, userData) {
	QFont f = font();

	//if (f.pointSize() != FONT_SIZE) {
	//	f.setPointSize(FONT_SIZE);
	//	setFont(f);
	//}
}
mainApplication::~mainApplication() = default;

bool mainApplication::notify(QObject *receiver, QEvent *event) {
	switch (event->type()) {
		case QEvent::ShortcutOverride:
			if (shortcut_override_event(event)) {
				return (true);
			}
			break;
		case QEvent::KeyRelease:
			if (key_release_event(event)) {
				return (true);
			}
			break;
		case QEvent::Shortcut:
			if (dlgkeyb_event(event)) {
				return (true);
			}
			break;
		default:
			break;
	}
	return (QApplication::notify(receiver, event));
}

BYTE mainApplication::base_folder(QDir *new_folder, QDir *old_folder, const QString &base, const QString &message) {
	QString folder = QString(base).remove("/");

	if (old_folder && !info.portable && !new_folder->exists(folder) && old_folder->exists(folder)) {
		old_folder->rename(old_folder->absolutePath() + base, new_folder->absolutePath() + base);
	}
	if (!new_folder->mkpath(folder)) {
		QMessageBox::critical(nullptr,
			//: Do not translate %1
			tr("%1 folders").arg(NAME),
			message, QMessageBox::Ok);
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
BYTE mainApplication::control_base_folders(void) {
	QDir config_folder(uQString(gui_config_folder()));
	QDir data_folder(uQString(gui_data_folder()));
	QDir temp_folder(uQString(gui_temp_folder()));
#if defined (_WIN32)
	QDir old(QString("%0/%1").arg(uQString(gui_home_folder())).arg(NAME));
#else
	QDir old(QString("%0/.%1").arg(uQString(gui_home_folder()), NAME));
#endif

	if (!info.portable && !old.exists() && !config_folder.exists()) {
		dlgWizard *dlg = new dlgWizard(nullptr, config_folder.absolutePath(), uQString(gui_application_folder()));

		dlg->show();
		if (dlg->exec() == QDialog::Rejected) {
			delete (dlg);
			return (EXIT_ERROR);
		}
		config_folder.setPath(uQString(gui_config_folder()));
		data_folder.setPath(uQString(gui_data_folder()));
		temp_folder.setPath(uQString(gui_temp_folder()));
	}

	// controllo l'esistenza della directory principale
	if (base_folder(&config_folder, nullptr, ".", tr("Error on create config folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&data_folder, nullptr, ".", tr("Error on create data folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&temp_folder, nullptr, ".", tr("Error on create temp folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	// creo le sottocartelle
	if (base_folder(&config_folder, &old, CHEAT_FOLDER, tr("Error on create cheat folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&config_folder, &old, PERGAME_FOLDER, tr("Error on create psg folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&config_folder, &old, SHDPAR_FOLDER, tr("Error on create shp folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&config_folder, &old, JSC_FOLDER, tr("Error on create jsc folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&data_folder, &old, BIOS_FOLDER, tr("Error on create bios folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&data_folder, &old, DIFF_FOLDER, tr("Error on create diff folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&data_folder, &old, PRB_FOLDER, tr("Error on create prb folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&data_folder, &old, SAVE_FOLDER, tr("Error on create save folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
	if (base_folder(&data_folder, &old, SCRSHT_FOLDER, tr("Error on create screenshot folder")) == EXIT_ERROR) {
		return (EXIT_ERROR);
	}

	if (!info.portable && old.exists()) {
		QString list[] = { QString(CFGFILENAME), QString(INPFILENAME), QString(RECENTFILENAME) };
		int i;

		for (i = 0; i < 3; i++) {
			QString file = QString(list[i]).remove("/");

			if (!config_folder.exists(file) && old.exists(file)) {
				old.rename(old.absolutePath() + list[i], config_folder.absolutePath() + list[i]);
			}
		}
		old.removeRecursively();
	}
	return (EXIT_OK);
}

QKeySequence mainApplication::key_sequence_from_key_event(QKeyEvent *event) {
	unsigned int modifiers = (unsigned int)event->modifiers();
	int key = event->key();
	QKeySequence ks;

	if (modifiers & Qt::KeypadModifier) {
		modifiers &= ~Qt::KeypadModifier;
	}
	if ((key >= Qt::Key_Shift) && (key <= Qt::Key_Alt)) {
		key = 0;
	}
	return (QKeySequence(modifiers ? (int)modifiers : key, modifiers ? key : 0).toString().remove(", "));
}
bool mainApplication::is_set_inp_shortcut(QEvent *event, int set_inp) {
	return (mainwin && !mainwin->shortcut[set_inp]->key().isEmpty() &&
		(key_sequence_from_key_event((QKeyEvent *)event) == mainwin->shortcut[set_inp]->key()));
}
bool mainApplication::dlgkeyb_event(QEvent *event) {
	// il resto degli eventi
	if (dlgkeyb && dlgkeyb->process_event(event)) {
		return (true);
	}
	return (false);
}
bool mainApplication::shortcut_override_event(QEvent *event) {
	if (!dlgkeyb_event(event)) {
		// shortcut attivi finche' il tasto della tastiera e' premuto
		if (is_set_inp_shortcut(event, SET_INP_SC_SHOUT_INTO_MIC)) {
			if (!((QKeyEvent *)event)->isAutoRepeat()) {
				mainwin->shout_into_mic(PRESSED);
			}
			return (true);
		} else if (is_set_inp_shortcut(event, SET_INP_SC_HOLD_FAST_FORWARD)) {
			if (!((QKeyEvent *)event)->isAutoRepeat()) {
				mainwin->hold_fast_forward(TRUE);
			}
			return (true);
		}
		return (false);
	}
	return (true);
}
bool mainApplication::key_release_event(QEvent *event) {
	if (!dlgkeyb_event(event)) {
		// shortcut attivi finche' il tasto della tastiera e' premuto
		if (is_set_inp_shortcut(event, SET_INP_SC_SHOUT_INTO_MIC)) {
			if (!((QKeyEvent *)event)->isAutoRepeat()) {
				mainwin->shout_into_mic(RELEASED);
			}
			return (true);
		} else if (is_set_inp_shortcut(event, SET_INP_SC_HOLD_FAST_FORWARD)) {
			if (!((QKeyEvent *)event)->isAutoRepeat()) {
				mainwin->hold_fast_forward(FALSE);
			}
			return (true);
		}
		return (false);
	}
	return (true);
}
