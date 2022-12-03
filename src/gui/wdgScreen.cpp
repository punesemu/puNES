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

#include <QtCore/QtGlobal>
#include <QtCore/QMimeData>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#if defined (WITH_OPENGL)
#include "opengl.h"
#endif
#include "wdgScreen.hpp"
#include "mainWindow.hpp"
#include "objSettings.hpp"
#include "dlgKeyboard.hpp"
#include "conf.h"
#include "tas.h"
#include "gui.h"
#include "patcher.h"
#include "input.h"
#include "tape_data_recorder.h"

wdgScreen::wdgScreen(QWidget *parent) : QWidget(parent) {
	target = nullptr;
	paste = new QAction(this);
	tape.play = new QAction(this);
	tape.record = new QAction(this);
	tape.stop = new QAction(this);
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

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	paste->setIcon(QIcon(":/icon/icons/paste.svg"));
	tape.play->setIcon(QIcon(":/icon/icons/cassette_tape_play.svg"));
	tape.record->setIcon(QIcon(":/icon/icons/cassette_tape_record.svg"));
	tape.stop->setIcon(QIcon(":/icon/icons/cassette_tape_stop.svg"));

	connect (this, SIGNAL(et_cursor_set()), this, SLOT(s_cursor_set()));
	connect (this, SIGNAL(et_cursor_hide(int)), this, SLOT(s_cursor_hide(int)));
	connect(paste, SIGNAL(triggered()), this, SLOT(s_paste_event()));
	connect(tape.play, SIGNAL(triggered()), this, SLOT(s_tape_play_event()));
	connect(tape.record, SIGNAL(triggered()), this, SLOT(s_tape_record_event()));
	connect(tape.stop, SIGNAL(triggered()), this, SLOT(s_tape_stop_event()));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(s_context_menu(QPoint)));

	installEventFilter(this);
}
wdgScreen::~wdgScreen() = default;

QPaintEngine *wdgScreen::paintEngine() const {
	return (nullptr);
}
bool wdgScreen::eventFilter(QObject *obj, QEvent *event) {
	static QMouseEvent *mouseEvent;
	static QKeyEvent *keyEvent;
	static DBWORD keyval;

	if (event->type() == QEvent::ShortcutOverride) {
		keyEvent = ((QKeyEvent *)event);

		if (rwnd.active || !mainwin->is_rwnd_shortcut_or_not_shcut(keyEvent)) {
			return (true);
		}

		keyval = objInp::kbd_keyval_decode(keyEvent);

#if !defined (RELEASE)
		if (keyval == Qt::Key_Insert) {
			info.snd_info = !info.snd_info;
		}
#endif
		if (tas.type == NOTAS) {
			events.mutex.lock();
			events.keyb << _wdgScreen_keyboard_event(PRESSED, keyEvent->isAutoRepeat(), keyval, KEYBOARD);
			events.mutex.unlock();
		}
	} else if (event->type() == QEvent::KeyRelease) {
		keyEvent = ((QKeyEvent *)event);

		if (rwnd.active || !mainwin->is_rwnd_shortcut_or_not_shcut(keyEvent)) {
			return (true);
		}

		keyval = objInp::kbd_keyval_decode(keyEvent);

		if (tas.type == NOTAS) {
			events.mutex.lock();
			events.keyb << _wdgScreen_keyboard_event(RELEASED, keyEvent->isAutoRepeat(), keyval, KEYBOARD);
			events.mutex.unlock();
		}
	} else if ((tas.type == NOTAS) && !rwnd.active) {
		if ((event->type() == QEvent::MouseButtonPress) ||
			(event->type() == QEvent::MouseButtonRelease) ||
			(event->type() == QEvent::MouseButtonDblClick)) {
			mouseEvent = ((QMouseEvent *)event);
			events.mutex.lock();
			events.mouse << _wdgScreen_mouse_event(event->type(), mouseEvent->button(), 0, 0);
			events.mutex.unlock();
		} else if (event->type() == QEvent::MouseMove) {
			mouseEvent = ((QMouseEvent *)event);
			events.mutex.lock();
			events.mouse << _wdgScreen_mouse_event(event->type(), Qt::NoButton, mouseEvent->pos().x(), mouseEvent->pos().y());
			events.mutex.unlock();
		}
	}

	return (QObject::eventFilter(obj, event));
}
void wdgScreen::dragEnterEvent(QDragEnterEvent *event) {
	if (event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
	if (nes_keyboard.enabled && event->mimeData()->hasText()) {
		event->acceptProposedAction();
	}
}
void wdgScreen::dropEvent(QDropEvent *event) {
	foreach (const QUrl &url, event->mimeData()->urls()) {
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
			if (is_patch && !is_rom && !info.rom.file[0]) {
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
		return;
	}
	if (!event->mimeData()->text().isEmpty() && nes_keyboard.enabled) {
		if (!dlgkeyb->paste->enable && (tape_data_recorder.mode == TAPE_DATA_NONE)) {
			dlgkeyb->paste->set_text(event->mimeData()->text());
		}
		return;
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
	if (input_draw_target()) {
		setCursor((*target));
	} else {
		gmouse.hidden = FALSE;
		unsetCursor();
	}
}
void wdgScreen::s_cursor_hide(int hide) {
	if (hide) {
		setCursor(Qt::BlankCursor);
	} else {
		cursor_set();
	}
}
void wdgScreen::s_paste_event(void) {
	const QClipboard *clipboard = QApplication::clipboard();
	const QMimeData *mimeData = clipboard->mimeData();

	if (mimeData->hasUrls() || mimeData->hasText()) {
		QDropEvent de(QPointF(0, 0), Qt::CopyAction, clipboard->mimeData(), Qt::NoButton, Qt::NoModifier);

		dropEvent(&de);
	}
}
void wdgScreen::s_tape_play_event(void) {
	mainwin->action_Tape_Play->trigger();
}
void wdgScreen::s_tape_record_event(void) {
	mainwin->action_Tape_Record->trigger();
}
void wdgScreen::s_tape_stop_event(void) {
	mainwin->action_Tape_Stop->trigger();
}
void wdgScreen::s_context_menu(const QPoint &pos) {
	QPoint global_pos = mapToGlobal(pos);
	int counter = 0;
	QMenu menu;

	menu.setMinimumWidth(200);

	if (!info.no_rom) {
		if (nes_keyboard.enabled) {
			const QClipboard *clipboard = QApplication::clipboard();
			const QMimeData *mimeData = clipboard->mimeData();

			menu.addSection(dlgkeyb->keyboard->keyboard_name());

			paste->setText(tr("Paste"));
			paste->setEnabled((mimeData->hasUrls() || mimeData->hasText()) &&
				!dlgkeyb->paste->enable && (tape_data_recorder.mode == TAPE_DATA_NONE));

			menu.addAction(paste);

			counter++;
		}
		if (tape_data_recorder.enabled) {
			menu.addSection(tr("Tape"));

			tape.play->setText(tr("Play"));
			tape.record->setText(tr("Record"));
			tape.stop->setText(tr("Stop"));

			tape.play->setEnabled(mainwin->action_Tape_Play->isEnabled());
			tape.record->setEnabled(mainwin->action_Tape_Record->isEnabled());
			tape.stop->setEnabled(mainwin->action_Tape_Stop->isEnabled());

			menu.addAction(tape.play);
			menu.addAction(tape.record);
			menu.addAction(tape.stop);

			counter++;
		}
	}

	if (counter) {
		menu.exec(global_pos);
	}
}
