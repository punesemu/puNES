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

#include <QtWidgets/QWidget>
#include <QtGui/QKeyEvent>
#include <QtGui/QKeySequence>
#include "mainApplication.moc"
#include "singleapplication.moc"
#include "singleapplication_p.moc"
#include "mainWindow.hpp"
#include "dlgKeyboard.hpp"
#include "wdgScreen.hpp"
#include "gui.h"

mainApplication::mainApplication(int &argc, char *argv[], bool allowSecondary, Options options, int timeout, const QString &userData) :
	SingleApplication(argc, argv, allowSecondary, options, timeout, userData) {
}
mainApplication::~mainApplication() {}

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

QKeySequence mainApplication::key_sequence_from_key_event(QKeyEvent *event) {
	int modifiers = event->modifiers();
	int key = event->key();
	QKeySequence ks;

	if ((key >= Qt::Key_Shift) && (key <= Qt::Key_Alt)) {
	    key = 0;
	}
	return (QKeySequence(modifiers ? modifiers : key, modifiers ? key : 0).toString().remove(", "));
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
		if (key_sequence_from_key_event((QKeyEvent *)event) == mainwin->shortcut[SET_INP_SC_SHOUT_INTO_MIC]->key()) {
			if (!((QKeyEvent *)event)->isAutoRepeat()) {
				mainwin->shout_into_mic(PRESSED);
			}
			return (true);
		} else if (key_sequence_from_key_event((QKeyEvent *)event) == mainwin->shortcut[SET_INP_SC_HOLD_FAST_FORWARD]->key()) {
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
		if (key_sequence_from_key_event((QKeyEvent *)event) == mainwin->shortcut[SET_INP_SC_SHOUT_INTO_MIC]->key()) {
			if (!((QKeyEvent *)event)->isAutoRepeat()) {
				mainwin->shout_into_mic(RELEASED);
			}
			return (true);
		} else if (key_sequence_from_key_event((QKeyEvent *)event) == mainwin->shortcut[SET_INP_SC_HOLD_FAST_FORWARD]->key()) {
			if (!((QKeyEvent *)event)->isAutoRepeat()) {
				mainwin->hold_fast_forward(FALSE);
			}
			return (true);
		}
		return (false);
	}
	return (true);
}
