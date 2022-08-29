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
		case QEvent::KeyRelease:
		case QEvent::Shortcut:
			if (dlgkeyb && dlgkeyb->process_event(event)) {
				return (true);
			}
			break;
		default:
			break;
	}
	return (QApplication::notify(receiver, event));
}
