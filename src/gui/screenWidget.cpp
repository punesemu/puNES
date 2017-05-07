/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtCore/QMimeData>
#endif
#include <QtCore/QUrl>
#include "screenWidget.moc"
#include "settingsObject.hpp"
#include "conf.h"
#include "tas.h"
#include "timeline.h"
#include "gui.h"
#if defined (WITH_OPENGL)
#include "opengl.h"
#endif

screenWidget::screenWidget(QWidget *parent, mainWindow *mw) : QWidget(parent) {
#if defined (__WIN32__)
#if defined (WITH_OPENGL)
	memset(&data, 0x00, sizeof(data));
	data.qt = (WNDPROC)GetWindowLongPtr((HWND) winId(), GWLP_WNDPROC);

	// applico un sfondo nero
	parent->setStyleSheet("background-color: black");
#endif
	target = nullptr;
#endif

	mwin = mw;

	// se non faccio questa chiamata, la versione SDL crasha all'avvio
	winId();

	setUpdatesEnabled(false);

	setAcceptDrops(true);

	setFocusPolicy(Qt::StrongFocus);
	setFocus(Qt::ActiveWindowFocusReason);

	setMouseTracking(true);

	installEventFilter(this);
}
screenWidget::~screenWidget() {}
#if defined (__WIN32__)
#if defined (WITH_OPENGL)
void screenWidget::controlEventFilter() {
	data.tmp = (WNDPROC)GetWindowLongPtr((HWND) winId(), GWLP_WNDPROC);

	if ((data.tmp != data.sdl) && (data.tmp != data.qt)) {
		data.sdl = data.tmp;
	}

	if (data.tmp != data.qt) {
		SetWindowLongPtr((HWND) winId(), GWLP_WNDPROC, (LONG_PTR) data.qt);
	}
}
#endif
void screenWidget::cursor_init() {
	target = new QCursor(QPixmap(":/pointers/pointers/target_32x32.xpm"), -1, -1);
}
void screenWidget::cursor_set() {
	if (input_draw_target() == TRUE) {
		setCursor((*target));
	} else {
		unsetCursor();
	}
}
void screenWidget::cursor_hide(BYTE hide) {
	if (hide == TRUE) {
		setCursor(Qt::BlankCursor);
	} else {
		cursor_set();
	}
}
#endif

void screenWidget::dragEnterEvent(QDragEnterEvent *e) {
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}
void screenWidget::dropEvent(QDropEvent *e) {
	foreach (const QUrl &url, e->mimeData()->urls()){
		mwin->change_rom(uQStringCD(url.toLocalFile()));
		activateWindow();
		gui_set_focus();
		break;
	}
}
bool screenWidget::eventFilter(QObject *obj, QEvent *event) {
	static QMouseEvent *mouseEvent;
	static QKeyEvent *keyEvent;
	static DBWORD keyval;

	if (event->type() == QEvent::KeyPress) {
		keyEvent = ((QKeyEvent *)event);
		keyval = inpObject::kbd_keyval_decode(keyEvent);

		if (keyval == gui.key.tl) {
			if (!tl.key) {
				mwin->statusbar->timeline->timeline_pressed(&tl.key);
			}
			return (true);
#if !defined (RELEASE)
		} else if (keyval == Qt::Key_Insert) {
			info.snd_info = !info.snd_info;
#endif
		} else if (keyval == Qt::Key_Left) {
			if (tl.key) {
				int snap = mwin->statusbar->timeline->value();

				mwin->statusbar->timeline->setValue(snap - 1, true);
				return (true);
			}
		} else if (keyval == Qt::Key_Right) {
			if (tl.key) {
				int snap = mwin->statusbar->timeline->value();

				mwin->statusbar->timeline->setValue(snap + 1, true);
				return (true);
			}
		}

		if (!tas.type) {
			for (BYTE i = PORT1; i < PORT_MAX; i++) {
				if (port_funct[i].input_decode_event && (port_funct[i].input_decode_event(PRESSED,
						keyval, KEYBOARD, &port[i]) == EXIT_OK)) {
					return (true);
				}
			}
		}
	} else if (event->type() == QEvent::KeyRelease) {
		keyEvent = ((QKeyEvent *)event);
		keyval = inpObject::kbd_keyval_decode(keyEvent);

		if (keyval == gui.key.tl) {
			if (tl.key) {
				mwin->statusbar->timeline->timeline_released(&tl.key);
			}
			return (true);
		}

		if (!tas.type) {
			for (BYTE i = PORT1; i < PORT_MAX; i++) {
				if (port_funct[i].input_decode_event && (port_funct[i].input_decode_event(RELEASED,
						keyval, KEYBOARD, &port[i]) == EXIT_OK)) {
					return (true);
				}
			}
		}
	} else if (event->type() == QEvent::MouseButtonPress) {
		mouseEvent = ((QMouseEvent *)event);

		if (mouseEvent->button() == Qt::LeftButton) {
			gmouse.left = TRUE;
		} else if (mouseEvent->button() == Qt::RightButton) {
			gmouse.right = TRUE;
		}
	} else if (event->type() == QEvent::MouseButtonRelease) {
		mouseEvent = ((QMouseEvent *)event);

		if (mouseEvent->button() == Qt::LeftButton) {
			gmouse.left = FALSE;
		} else if (mouseEvent->button() == Qt::RightButton) {
			gmouse.right = FALSE;
		}
	} else if (event->type() == QEvent::MouseMove) {
		mouseEvent = ((QMouseEvent *)event);

		gmouse.x = mouseEvent->x();
		gmouse.y = mouseEvent->y();
	}

	return (QObject::eventFilter(obj, event));
}
