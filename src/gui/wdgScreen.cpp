/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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
#include <QtCore/QMimeData>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#if defined (WITH_OPENGL)
#include "opengl.h"
#endif
#include "mainWindow.hpp"
#include "objSettings.hpp"
#include "wdgScreen.moc"
#include "conf.h"
#include "tas.h"
#include "gui.h"
#include "patcher.h"

wdgScreen::wdgScreen(QWidget *parent) : QWidget(parent) {
	target = NULL;
#if defined (WITH_OPENGL)
	wogl = new wdgOpenGL(this);
#elif defined (WITH_D3D9)
	wd3d9 = new wdgD3D9(this);

	setAttribute(Qt::WA_PaintOnScreen);
#endif
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);

	setAcceptDrops(true);

	setFocusPolicy(Qt::StrongFocus);
	setFocus(Qt::ActiveWindowFocusReason);

	setMouseTracking(true);

	connect (this, SIGNAL(et_cursor_set()), this, SLOT(s_cursor_set()));
	connect (this, SIGNAL(et_cursor_hide(int)), this, SLOT(s_cursor_hide(int)));

	installEventFilter(this);
}
wdgScreen::~wdgScreen() {}

bool wdgScreen::eventFilter(QObject *obj, QEvent *event) {
	static QMouseEvent *mouseEvent;
	static QKeyEvent *keyEvent;
	static DBWORD keyval;

	if (event->type() == QEvent::ShortcutOverride) {
		keyEvent = ((QKeyEvent *)event);

		if ((rwnd.active == TRUE) || (mainwin->is_rwnd_shortcut_or_not_shcut(keyEvent) == false)) {
			return (true);
		}

		keyval = objInp::kbd_keyval_decode(keyEvent);

#if !defined (RELEASE)
		if (keyval == Qt::Key_Insert) {
			info.snd_info = !info.snd_info;
		}
#endif
		if (tas.type == NOTAS) {
			input_event << _wdgScreen_input_event(PRESSED, keyEvent->isAutoRepeat(), keyval, KEYBOARD);
		}
	} else if (event->type() == QEvent::KeyPress) {
		return (true);
	} else if (event->type() == QEvent::KeyRelease) {
		keyEvent = ((QKeyEvent *)event);

		if ((rwnd.active == TRUE) || (mainwin->is_rwnd_shortcut_or_not_shcut(keyEvent) == false)) {
			return (true);
		}

		keyval = objInp::kbd_keyval_decode(keyEvent);

		if (tas.type == NOTAS) {
			input_event << _wdgScreen_input_event(RELEASED, keyEvent->isAutoRepeat(), keyval, KEYBOARD);
		}
	} else if ((tas.type == NOTAS) && (rwnd.active == FALSE)) {
		if ((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::MouseButtonRelease) ||
				(event->type() == QEvent::MouseButtonDblClick)) {
			mouseEvent = ((QMouseEvent *)event);
			mouse_event << _wdgScreen_mouse_event(event->type(), mouseEvent->button(), 0, 0);
		} else if (event->type() == QEvent::MouseMove) {
			mouseEvent = ((QMouseEvent *)event);
			mouse_event << _wdgScreen_mouse_event(event->type(), Qt::NoButton, mouseEvent->x(), mouseEvent->y());
		}
	}

	return (QObject::eventFilter(obj, event));
}
void wdgScreen::dragEnterEvent(QDragEnterEvent *event) {
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}
void wdgScreen::dropEvent(QDropEvent *event) {
	foreach (const QUrl &url, event->mimeData()->urls()){
		QFileInfo fileinfo(url.toLocalFile());
		_uncompress_archive *archive;
		BYTE is_rom = FALSE, is_patch = FALSE, rc;
		uTCHAR *rom, *patch = nullptr;

		if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase == GG_EXECUTE)) {
			rom = gamegenie.rom;
		} else {
			rom = info.rom.file;
		}

		archive = uncompress_archive_alloc(uQStringCD(fileinfo.absoluteFilePath()), &rc);

		if (rc == UNCOMPRESS_EXIT_OK) {
			if (archive->rom.count > 0) {
				is_rom = TRUE;
			}
			if (archive->patch.count > 0) {
				is_patch = TRUE;
			}
			if ((is_patch == TRUE) && (is_rom == FALSE) && !info.rom.file[0]) {
				is_patch = FALSE;
			}
			if (is_rom) {
				switch ((rc = uncompress_archive_extract_file(archive, UNCOMPRESS_TYPE_ROM))) {
					case UNCOMPRESS_EXIT_OK:
						rom = uncompress_archive_extracted_file_name(archive, UNCOMPRESS_TYPE_ROM);
						break;
					case UNCOMPRESS_EXIT_ERROR_ON_UNCOMP:
						return;
					default:
						break;
				}
			}
			if (is_patch) {
				switch ((rc = uncompress_archive_extract_file(archive, UNCOMPRESS_TYPE_PATCH))) {
					case UNCOMPRESS_EXIT_OK:
						patch = uncompress_archive_extracted_file_name(archive, UNCOMPRESS_TYPE_PATCH);
						break;
					case UNCOMPRESS_EXIT_ERROR_ON_UNCOMP:
						return;
					default:
						is_patch = FALSE;
						break;
				}
			}
			uncompress_archive_free(archive);
		} else if (rc == UNCOMPRESS_EXIT_IS_NOT_COMP) {
			 if (((fileinfo.suffix().toLower() == "ips") ||
				 (fileinfo.suffix().toLower() == "bps") ||
				 (fileinfo.suffix().toLower() == "xdelta")) &&
				 info.rom.file[0]) {
				is_patch = TRUE;
				patch = uQStringCD(fileinfo.absoluteFilePath());
			} else {
				is_rom = TRUE;
				rom = uQStringCD(fileinfo.absoluteFilePath());
			}
		}

		if (is_patch) {
			patcher.file = emu_ustrncpy(patcher.file, patch);
		}

		mainwin->change_rom(rom);
		activateWindow();
		gui_set_focus();
		break;
	}
}
void wdgScreen::resizeEvent(QResizeEvent *event) {
#if defined (WITH_OPENGL)
	wogl->setUpdatesEnabled(false);
	wogl->resize(event->size());
	wogl->setUpdatesEnabled(true);
#elif defined (WITH_D3D9)
	wd3d9->setUpdatesEnabled(false);
	wd3d9->resize(event->size());
	wd3d9->setUpdatesEnabled(true);
#endif
}

void wdgScreen::cursor_init(void) {
	//target = new QCursor(QPixmap(":/pointers/pointers/target_48x48.xpm"), -1, -1);
	target = new QCursor(QPixmap(":/pointers/pointers/target_32x32.xpm"), -1, -1);
}
void wdgScreen::cursor_set(void) {
	emit et_cursor_set();
}
void wdgScreen::cursor_hide(BYTE hide) {
	emit et_cursor_hide(hide);
}

void wdgScreen::s_cursor_set(void) {
	if (input_draw_target() == TRUE) {
		setCursor((*target));
	} else {
		gmouse.hidden = FALSE;
		unsetCursor();
	}
}
void wdgScreen::s_cursor_hide(int hide) {
	if (hide == TRUE) {
		setCursor(Qt::BlankCursor);
	} else {
		cursor_set();
	}
}
