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

#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMessageBox>
#include <QtGui/QImage>
#include <QtCore/QDir>
#include <QtGui/QScreen>
#if defined (__WIN32__)
#include <QtCore/QtPlugin>
#if defined (QT5_PLUGIN_QWINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif
#if defined (QT_PLUGIN_QWINDOWSVISTASTYLE)
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
#endif
#if defined (QT_PLUGIN_QGIF)
Q_IMPORT_PLUGIN(QGifPlugin)
#endif
#if defined (QT_PLUGIN_QICO)
Q_IMPORT_PLUGIN(QICOPlugin)
#endif
#if defined (QT_PLUGIN_QJPEG)
Q_IMPORT_PLUGIN(QJpegPlugin)
#endif
#if defined (QT_PLUGIN_QSVG)
Q_IMPORT_PLUGIN(QSvgPlugin)
#endif
#endif
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#if defined (WITH_OPENGL)
#include "opengl.h"
#endif
#include "mainWindow.hpp"
#include "objCheat.hpp"
#include "dlgSettings.hpp"
#include "dlgUncomp.hpp"
#include "dlgVsSystem.hpp"
#include "dlgAPUChannels.hpp"
#include "dlgPPUHacks.hpp"
#include "wdgScreen.hpp"
#include "wdgStatusBar.hpp"
#include "video/gfx_thread.h"
#include "emu_thread.h"
#include "version.h"
#include "conf.h"
#include "clock.h"
#include "timeline.h"
#include "save_slot.h"
#include "vs_system.h"
#include "gui.h"
#if defined (WITH_D3D9)
#include "d3d9.h"
#endif

static struct _qt {
	QApplication *app;
	mainWindow *mwin;
	wdgScreen *screen;
	objCheat *objch;
	QImage qimage;

	// dialog del settaggio
	dlgSettings *dset;

	// controlli esterni
	dlgVsSystem *vssystem;
	dlgAPUChannels *apuch;
	dlgPPUHacks *ppuhacks;

	// QObject che non mandano un pause quando in background
	QList<QWidget *> no_bck_pause;
} qt;

class appEventFilter: public QObject {
	public:
		appEventFilter() : QObject() {};
		~appEventFilter() {};

		bool eventFilter(QObject* object, QEvent* event) {
			if (event->type() == QEvent::MouseMove) {
				if (gmouse.hidden == TRUE) {
					if ((input_draw_target() == TRUE) || (cfg->fullscreen != FULLSCR)) {
						gui_cursor_hide(FALSE);
					}
				}
				gmouse.timer = gui_get_ms();
			}
			return (QObject::eventFilter(object, event));
		}
};

void gui_quit(void) {}
BYTE gui_create(void) {
#if defined (WITH_OPENGL)
	QSurfaceFormat fmt;

	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	fmt.setRedBufferSize(8);
	fmt.setGreenBufferSize(8);
	fmt.setBlueBufferSize(8);
	fmt.setAlphaBufferSize(8);
	fmt.setSwapInterval(cfg->vsync);
	QSurfaceFormat::setDefaultFormat(fmt);
#endif
	qt.mwin = new mainWindow();
	qt.screen = qt.mwin->screen;
	qt.objch->setParent(qt.mwin);

	qt.app->installEventFilter(new appEventFilter());

	{
		int screenNumber = qApp->desktop()->screenNumber(qt.mwin);

		if (cfg->last_pos.x > qApp->desktop()->screenGeometry(screenNumber).width()) {
			cfg->last_pos.x = 0;
			cfg->last_pos_settings.x = 0;
		}
		if (cfg->last_pos.y > qApp->desktop()->screenGeometry(screenNumber).height()) {
			cfg->last_pos.x = 0;
			cfg->last_pos_settings.y = 0;
		}
		qt.mwin->move(QPoint(cfg->last_pos.x, cfg->last_pos.y));
	}

	qt.mwin->show();

	qt.dset = new dlgSettings();

	memset(&ext_win, 0x00, sizeof(ext_win));
	qt.vssystem = new dlgVsSystem(qt.mwin);
	qt.apuch = new dlgAPUChannels(qt.mwin);
	qt.ppuhacks = new dlgPPUHacks(qt.mwin);

	qt.no_bck_pause.append(qt.mwin);
	qt.no_bck_pause.append(qt.dset);
	qt.no_bck_pause.append(qt.vssystem);
	qt.no_bck_pause.append(qt.apuch);
	qt.no_bck_pause.append(qt.ppuhacks);

	gmouse.hidden = FALSE;
	gmouse.timer = gui_get_ms();

	// forzo un evento QEvent::LanguageChange. Se non lo faccio,
	// sotto windows, all'avvio dell'emulatore, se il cfg->language e' impostato
	// sull'inglese, non viene emesso nessun QEvent::LanguageChangen non permettendo
	// di tradurre correttamente i bottoni degli shortcuts nel wdgSettingsInput.
	QEvent event(QEvent::LanguageChange);
	QApplication::sendEvent(qt.mwin, &event);
	QApplication::sendEvent(qt.dset, &event);

	return (EXIT_OK);
}
void gui_start(void) {
	gui.start = TRUE;
	fps.frame.expected_end = gui_get_ms() + machine.ms_frame;
	gfx_thread_continue();
	emu_thread_continue();
	qApp->exec();
}

void gui_set_video_mode(void) {
	if (cfg->scale == X1) {
		qt.mwin->statusbar->state->setVisible(false);
		if (overscan.enabled) {
			qt.mwin->menu_Help->menuAction()->setVisible(false);
		} else {
			qt.mwin->menu_Help->menuAction()->setVisible(true);
		}
	} else {
		qt.mwin->statusbar->state->setVisible(true);
		qt.mwin->menu_Help->menuAction()->setVisible(true);
	}

	qt.screen->setFixedSize(QSize(gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE]));

	qt.mwin->setFixedSize(QSize(qt.screen->width(),
		(qt.mwin->menubar->isHidden() ? 0 : qt.mwin->menubar->sizeHint().height()) +
		(qt.screen->height() + 2) +
		(qt.mwin->statusbar->isHidden() ? 0 : qt.mwin->statusbar->sizeHint().height())));

	qt.mwin->menubar->setFixedWidth(gfx.w[VIDEO_MODE]);
	qt.mwin->statusbar->update_width(gfx.w[VIDEO_MODE]);
}

void gui_update(void) {
	uTCHAR title[255];

	gui.in_update = TRUE;

	emu_set_title(title, usizeof(title));
	qt.mwin->setWindowTitle(uQString(title));
	qt.mwin->update_window();
	qt.dset->update_dialog();

	gui.in_update = FALSE;
}
void gui_update_gps_settings(void) {
	qt.dset->change_rom();
}

void gui_fullscreen(void) {
	qt.mwin->s_set_fullscreen();
}
void gui_timeline(void) {
	qt.mwin->statusbar->timeline->setValue(tl.snaps_fill - 1, false);
}
void gui_save_slot(BYTE slot) {
	if (slot >= SAVE_SLOTS) {
		slot = SAVE_SLOTS - 1;
	}
	qt.mwin->state_save_slot_set(slot, FALSE);
}

void gui_print_usage(char *usage) {
	QMessageBox *box = new QMessageBox();

	if (box->font().pointSize() > 9) {
		QFont font;

		font.setPointSize(9);
		box->setFont(font);
	}

	box->setAttribute(Qt::WA_DeleteOnClose);
	box->setWindowTitle(QString(NAME));

	box->setWindowModality(Qt::WindowModal);

	// monospace
	box->setText("<pre>" + QString(usage) + "</pre>");

	box->setStandardButtons(QMessageBox::Ok);
	box->setDefaultButton(QMessageBox::Ok);

	box->show();
	box->exec();
}

int gui_uncompress_selection_dialog(_uncompress_archive *archive, BYTE type) {
	dlgUncomp *dlg = new dlgUncomp(qt.mwin, (void *)archive, type);

	dlg->show();
	dlg->exec();

	return (gui.dlg_rc);
}

void gui_control_pause_bck(WORD event) {
	BYTE found = FALSE;
	int i;

	if (cfg->bck_pause == FALSE) {
		return;
	}

	for (i = 0; i < qt.no_bck_pause.count(); i++) {
		if (qt.no_bck_pause.at(i)->isActiveWindow()) {
			found = TRUE;
			break;
		}
	}

	if (event == QEvent::WindowActivate) {
		if (gui.main_win_lfp == TRUE) {
			emu_pause(FALSE);
		}
		gui.main_win_lfp = FALSE;
	} else {
		if (found == FALSE) {
			emu_pause(TRUE);
			gui.main_win_lfp = TRUE;
		}
	}
}

void gui_active_window(void) {
	qt.screen->activateWindow();
}
void gui_set_focus(void) {
	qt.screen->setFocus(Qt::ActiveWindowFocusReason);
}

void *gui_objcheat_get_ptr(void) {
	return ((void *)qt.objch);
}
void gui_objcheat_init(void) {
	if (qt.objch == NULL) {
		qt.objch = new objCheat(0);
	}
	qt.objch->clear_list();
}
void gui_objcheat_read_game_cheats(void) {
	qt.objch->read_game_cheats();
}
void gui_objcheat_save_game_cheats(void) {
	qt.objch->save_game_cheats();
}

void gui_cursor_init(void) {
	qt.screen->cursor_init();
}
void gui_cursor_set(void) {
	qt.screen->cursor_set();
}
void gui_cursor_hide(BYTE hide) {
	gmouse.hidden = hide;
	qt.screen->cursor_hide(hide);
}
void gui_control_visible_cursor(void) {
	qt.mwin->control_visible_cursor();
}

void *gui_mainwindow_get_ptr(void) {
	return ((void *)qt.mwin);
}

void gui_emit_et_gg_reset(void) {
	emit qt.mwin->et_gg_reset();
}
void gui_emit_et_vs_reset(void) {
	emit qt.mwin->et_vs_reset();
}
void gui_emit_et_external_control_windows_show(void) {
	emit qt.mwin->et_external_control_windows_show();
}

void gui_screen_update(void) {
#if defined (WITH_OPENGL)
	qt.screen->wogl->update();
#elif defined (WITH_D3D9)
	qt.screen->wd3d9->repaint();
#endif
}

void *gui_dlgsettings_get_ptr(void) {
	return ((void *)qt.dset);
}

void gui_external_control_windows_show(void) {
	if (ext_win.vs_system && (cfg->fullscreen != FULLSCR)) {
		qt.vssystem->update_dialog();
		qt.vssystem->show();
	} else {
		qt.vssystem->hide();
	}
	if (ext_win.apu_channels && (cfg->fullscreen != FULLSCR)) {
		qt.apuch->update_dialog();
		qt.apuch->show();
	} else {
		qt.apuch->hide();
	}
	if (ext_win.ppu_hacks && (cfg->fullscreen != FULLSCR)) {
		qt.ppuhacks->update_dialog();
		qt.ppuhacks->show();
	} else {
		qt.ppuhacks->hide();
	}

	gui_update();
	gui_external_control_windows_update_pos();
	gui_active_window();
	gui_set_focus();
}
void gui_external_control_windows_update_pos(void) {
	unsigned int y = 0;

	y += qt.vssystem->update_pos(y);
	y += qt.apuch->update_pos(y);
	y += qt.ppuhacks->update_pos(y);
}

void gui_vs_system_update_dialog(void) {
	qt.vssystem->update_dialog();
}
void gui_vs_system_insert_coin(void) {
	if (vs_system.enabled == TRUE) {
		qt.vssystem->insert_coin(1);
	}
}

void gui_apu_channels_widgets_update(void) {
	qt.dset->update_tab_audio();
	qt.apuch->update_dialog();
}

void gui_ppu_hacks_widgets_update(void) {
	qt.dset->widget_wdgSettingsPPU->update_widget();
	qt.ppuhacks->widget_wdgSettingsPPU->update_widget();
}

#if defined (WITH_OPENGL)
void gui_wdgopengl_make_current(void) {
	if (gui.start == TRUE) {
		qt.screen->wogl->makeCurrent();
	}
}
unsigned int gui_wdgopengl_framebuffer_id(void) {
	return (qt.screen->wogl->framebuffer_id());
}

void gui_screen_info(void) {
	gfx.bit_per_pixel = qApp->primaryScreen()->depth();
}

uint32_t gui_color(BYTE a, BYTE r, BYTE g, BYTE b) {
	return (qRgba(r, g, b, a));
}
#endif

BYTE gui_load_lut(void *l, const uTCHAR *path) {
	QImage tmp;
	_lut *lut = (_lut*) l;

	if (path && (ustrlen(path) > 0)) {
		tmp = QImage(uQString(path));
	}

	if (tmp.isNull()) {
		lut->w = 0;
		lut->h = 0;
		lut->bits = nullptr;
		return (EXIT_ERROR);
	}

	qt.qimage = tmp.convertToFormat(QImage::Format_ARGB32);

	lut->w = qt.qimage.width();
	lut->h = qt.qimage.height();
	lut->bits = qt.qimage.bits();

	return (EXIT_OK);
}
void gui_save_screenshot(int w, int h, char *buffer, BYTE flip) {
	QString basename = QString(uQString(info.base_folder)) + QString(SCRSHT_FOLDER) + "/"
		+ QFileInfo(uQString(info.rom.file)).completeBaseName();
	QImage screenshot = QImage((uchar *)buffer, w, h, QImage::Format_RGB32);
	QFile file;
	uint count;

	if (!info.rom.file[0]) {
		return;
	}

	for (count = 1; count < 999999; count++) {
		QString final = basename + QString("_%1.png").arg(count, 6, 'd', 0, '0');

		if (QFileInfo(final).exists() == false) {
			file.setFileName(final);
			break;
		}
	}

	if (flip) {
		screenshot = screenshot.mirrored(false, true);
	}

	file.open(QIODevice::WriteOnly);
	screenshot.save(&file, "PNG");
}

void gui_utf_printf(const uTCHAR *fmt, ...) {
	static uTCHAR buffer[1024];
	va_list ap;

	va_start(ap, fmt);
	uvsnprintf(buffer, usizeof(buffer), fmt, ap);
	va_end(ap);

	QString utf = uQString(buffer);
	QMessageBox::warning(0, QString("%1").arg(utf.length()), utf);
}
void gui_utf_dirname(uTCHAR *path, uTCHAR *dst, size_t len) {
	QString utf = uQString(path);

	umemset(dst, 0x00, len);
	ustrncpy(dst, uQStringCD(QFileInfo(utf).absolutePath()), len - 1);
}
void gui_utf_basename(uTCHAR *path, uTCHAR *dst, size_t len) {
	QString utf = uQString(path);

	umemset(dst, 0x00, len);
	ustrncpy(dst, uQStringCD(QFileInfo(utf).fileName()), len - 1);
}
int gui_utf_strcasecmp(uTCHAR *s0, uTCHAR *s1) {
	return (QString::compare(uQString(s0), uQString(s1), Qt::CaseInsensitive));
}

#if defined (__linux__)
#include "os_linux.h"
#elif defined (__OpenBSD__)
#include "os_openbsd.h"
#elif defined (__WIN32__)
#include "os_windows.h"
#endif
