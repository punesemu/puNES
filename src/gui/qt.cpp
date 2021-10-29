/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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
#include <QtGui/QImage>
#include <QtCore/QDir>
#include <QtGui/QScreen>
#include <QtGui/QFontDatabase>
#if defined (_WIN32)
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
#if !defined (_WIN32)
// mi serve per il std::thread::hardware_concurrency() del gui_hardware_concurrency.
#include <thread>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#if defined (WITH_OPENGL)
#include "opengl.h"
#endif
#include "singleapplication.moc"
#include "singleapplication_p.moc"
#include "mainWindow.hpp"
#include "objCheat.hpp"
#include "dlgSettings.hpp"
#include "dlgUncomp.hpp"
#include "dlgVsSystem.hpp"
#include "wdgScreen.hpp"
#include "wdgStatusBar.hpp"
#include "wdgSettingsVideo.hpp"
#include "wdgOverlayUi.hpp"
#include "video/gfx_thread.h"
#include "emu_thread.h"
#include "version.h"
#include "conf.h"
#include "clock.h"
#include "save_slot.h"
#include "vs_system.h"
#include "gui.h"
#if defined (WITH_D3D9)
#include "d3d9.h"
#endif

static void gui_is_in_desktop(int *x, int *y);

static struct _qt {
	SingleApplication *app;
	mainWindow *mwin;
	wdgScreen *screen;
	objCheat *objch;
	QImage qimage;

	// widget dell'overlay
	wdgOverlayUi *overlay;

	// dialog del settaggio
	dlgSettings *dset;

	// controlli esterni
	dlgVsSystem *vssystem;

	// QObject che non mandano un pause quando in background
	QList<QWidget *>no_bck_pause;
} qt;

class appEventFilter: public QObject {
	public:
		appEventFilter() : QObject() {};
		~appEventFilter() {};

		bool eventFilter(QObject* object, QEvent* event) {
			if (event->type() == QEvent::MouseMove) {
				if (gmouse.hidden == TRUE) {
					if ((input_draw_target() == TRUE) ||
						(cfg->fullscreen != FULLSCR) ||
						qt.mwin->menubar->isVisible()) {
						gui_cursor_hide(FALSE);
					}
				}
				gmouse.timer = gui_get_ms();
			}
			return (QObject::eventFilter(object, event));
		}
};

void gui_init(int *argc, char **argv) {
	QFlags<SingleApplication::Mode> mode = SingleApplication::Mode::ExcludeAppVersion | SingleApplication::Mode::ExcludeAppPath;

	memset(&gui, 0, sizeof(gui));
	qt = {};
	qt.app = new SingleApplication((*argc), argv, true, mode);

	info.gui = TRUE;
	gui.in_update = FALSE;
	gui.main_win_lfp = 0;

	gui_init_os();
}
void gui_quit(void) {}
BYTE gui_control_instance(void) {
	if (qt.app->isSecondary() && (cfg->multiple_instances == FALSE)) {
		if (info.rom.file[0]) {
			unsigned int count = 0;

#if defined (_WIN32)
			// https://github.com/itay-grudev/SingleApplication/blob/master/Windows.md
			AllowSetForegroundWindow(DWORD(qt.app->primaryPid()));
#endif
			do {
				if (qt.app->sendMessage(uQString(info.rom.file).toUtf8()) == true) {
					break;
				}
				gui_sleep(20);
				count++;
			} while (count < 10);
		}
		qt.app->exit(0);
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
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

	QFontDatabase::addApplicationFont(":/fonts/fonts/Blocktopia.ttf");
	QFontDatabase::addApplicationFont(":/fonts/fonts/ChronoType.ttf");
	QFontDatabase::addApplicationFont(":/fonts/fonts/DigitalCounter7-AqDg.ttf");
	QFontDatabase::addApplicationFont(":/fonts/fonts/Rygarde.ttf");

	qt.mwin = new mainWindow();
	qt.screen = qt.mwin->screen;
	qt.objch->setParent(qt.mwin);

	qt.app->installEventFilter(new appEventFilter());

	gfx.device_pixel_ratio = qt.screen->devicePixelRatioF();

	gui_is_in_desktop(&cfg->lg.x, &cfg->lg.y);
	gui_is_in_desktop(&cfg->lg_settings.x, &cfg->lg_settings.y);
	qt.mwin->setGeometry(cfg->lg.x, cfg->lg.y, 0, 0);

	qt.mwin->show();

	qt.dset = new dlgSettings(qt.mwin);
	qt.overlay = new wdgOverlayUi();

	memset(&ext_win, 0x00, sizeof(ext_win));
	qt.vssystem = new dlgVsSystem(qt.mwin);

	qt.no_bck_pause.append(qt.mwin);
	qt.no_bck_pause.append(qt.dset);
	qt.no_bck_pause.append(qt.vssystem);

	gmouse.hidden = FALSE;
	gmouse.timer = gui_get_ms();

	// forzo un evento QEvent::LanguageChange. Se non lo faccio,
	// sotto windows, all'avvio dell'emulatore, se il cfg->language e' impostato
	// sull'inglese, non viene emesso nessun QEvent::LanguageChangen non permettendo
	// di tradurre correttamente i bottoni degli shortcuts nel wdgSettingsInput.
	QEvent event(QEvent::LanguageChange);

	QApplication::sendEvent(qt.mwin, &event);
	QApplication::sendEvent(qt.dset, &event);
	QApplication::sendEvent(qt.overlay, &event);

	return (EXIT_OK);
}
void gui_start(void) {
	gui.start = TRUE;
	fps.frame.expected_end = gui_get_ms() + machine.ms_frame;
	gfx_thread_continue();
	emu_thread_continue();
	if (info.start_with_hidden_gui) {
		qt.mwin->action_Toggle_GUI_in_window->trigger();
	}
	qt.app->exec();
}

void gui_set_video_mode(void) {
	if (cfg->scale == X1) {
		qt.mwin->toolbar->rotate_setVisible(false);
		qt.mwin->toolbar->state_setVisible(false);
		if (overscan.enabled) {
			qt.mwin->menu_Help->menuAction()->setVisible(false);
		} else {
			qt.mwin->menu_Help->menuAction()->setVisible(true);
		}
	} else {
		qt.mwin->toolbar->rotate_setVisible(true);
		qt.mwin->toolbar->state_setVisible(true);
		qt.mwin->menu_Help->menuAction()->setVisible(true);
	}

	{
		SDBWORD w = gfx.w[VIDEO_MODE], h = gfx.h[VIDEO_MODE];

		if (!cfg->fullscreen && ((cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_270))) {
			w = gfx.h[VIDEO_MODE];
			h = gfx.w[VIDEO_MODE];
		}

		qt.screen->setFixedSize(QSize(w, h));

		gui_set_window_size();
	}
}
void gui_set_window_size(void) {
	int w = qt.screen->width(), h = qt.screen->height();
	bool toolbar = qt.mwin->toolbar->isHidden() | qt.mwin->toolbar->isFloating();

	if (qt.mwin->toolbar->orientation() == Qt::Vertical) {
		w += (toolbar ? 0 : qt.mwin->toolbar->sizeHint().width());
	} else {
		h += (toolbar ? 0 : qt.mwin->toolbar->sizeHint().height());
	}

	h += (qt.mwin->menubar->isHidden() ? 0 : qt.mwin->menubar->sizeHint().height());
	h += (qt.mwin->statusbar->isHidden() ? 0 : qt.mwin->statusbar->sizeHint().height());

#if defined (_WIN32) && defined(WITH_OPENGL)
		// when a window is using an OpenGL based surface and is appearing in full screen mode,
		// problems can occur with other top-level windows which are part of the application. Due
		// to limitations of the Windows DWM, compositing is not handled correctly for OpenGL based
		// windows when going into full screen mode. As a result, other top-level windows are not
		// placed on top of the full screen window when they are made visible. For example, menus
		// may not appear correctly, or dialogs fail to show up.
		// https://doc.qt.io/qt-5/windows-issues.html#fullscreen-opengl-based-windows
		// https://bugreports.qt.io/browse/QTBUG-49258
		// https://bugreports.qt.io/browse/QTBUG-47156
		// come workaround incremento di 1 l'altezza del mainWindow e non utilizzo il
		// showFullScreen ma lo simulo.
		if (gfx.type_of_fscreen_in_use == FULLSCR) {
			h += 1;
		}
#endif

	qt.mwin->setFixedSize(QSize(w, h));

	qt.mwin->menubar->setFixedWidth(w);
	qt.mwin->statusbar->setFixedWidth(w);
}

void gui_state_save_slot_set(BYTE slot, BYTE on_video) {
	if (slot >= SAVE_SLOTS) {
		slot = SAVE_SLOTS - 1;
	}
	qt.mwin->state_save_slot_set(slot, on_video);
}
void gui_state_save_slot_set_tooltip(BYTE slot, char *buffer) {
	qt.mwin->state_save_slot_set_tooltip(slot, buffer);
}

void gui_update(void) {
	uTCHAR title[255];

	gui.in_update = TRUE;

	emu_set_title(title, usizeof(title));
	qt.mwin->setWindowTitle(uQString(title));
	qt.mwin->update_window();
	qt.dset->update_dialog();
	qt.overlay->update_widget();

	gui.in_update = FALSE;
}
void gui_update_dset(void) {
	qt.dset->update_dialog();
}
void gui_update_gps_settings(void) {
	qt.dset->change_rom();
}
void gui_update_status_bar(void) {
	qt.mwin->statusbar->update_statusbar();
}

void gui_update_ntsc_widgets(void) {
	qt.dset->update_tab_video();
}
void gui_update_ppu_hacks_widgets(void) {
	qt.dset->widget_Settings_PPU->update_widget();
}
void gui_update_apu_channels_widgets(void) {
	qt.dset->update_tab_audio();
}
void gui_update_recording_widgets(void) {
	qt.mwin->update_recording_widgets();
}

void gui_update_fds_menu(void) {
	qt.mwin->update_fds_menu();
}
void gui_update_recording_tab(void) {
	qt.dset->update_tab_recording();
}

void gui_egds_set_fps(void) {
	qt.mwin->egds->set_fps();
}
void gui_egds_stop_unnecessary(void) {
	if (gui.start == TRUE) {
		qt.mwin->egds->stop_unnecessary();
	}
}
void gui_egds_start_pause(void) {
	qt.mwin->egds->start_pause();
}
void gui_egds_stop_pause(void) {
	qt.mwin->egds->stop_pause();
}
void gui_egds_start_rwnd(void) {
	qt.mwin->egds->start_rwnd();
}
void gui_egds_stop_rwnd(void) {
	qt.mwin->egds->stop_rwnd();
}

void gui_fullscreen(void) {
	// se l'emulatore si avvia in fullscreen modalita' finestra, senza questo ritardo
	// e' possibile che le QT mi passino informazioni non corrette sulle dimensioni del
	// desktop e che le decorazioni della finestra non appaiano correttamente (problema
	// riscontrato sotto Linux e BSD).
	QTimer::singleShot(250, qt.mwin, SLOT(s_set_fullscreen()));
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
	qt.objch->read_game_cheats(NULL);
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
void gui_mainwindow_coords(int *x, int *y, BYTE border) {
	switch (border) {
		// top center
		case 0:
			(*x) = qt.mwin->geometry().x() + (qt.mwin->geometry().width() / 2);
			(*y) = qt.mwin->geometry().y();
			break;
		// top left
		case 1:
			(*x) = qt.mwin->geometry().x();
			(*y) = qt.mwin->geometry().y();
			break;
		// top right
		case 2:
			(*x) = qt.mwin->geometry().x() + qt.mwin->geometry().width();
			(*y) = qt.mwin->geometry().y();
			break;
		// bottom right
		case 3:
			(*x) = qt.mwin->geometry().x() + qt.mwin->geometry().width();
			(*y) = qt.mwin->geometry().y() + qt.mwin->geometry().height();
			break;
		// bottom left
		case 4:
			(*x) = qt.mwin->geometry().x();
			(*y) = qt.mwin->geometry().y() + qt.mwin->geometry().height();
			break;
	}
}
void gui_mainwindow_before_set_res(void) {
	qt.mwin->reset_min_max_size();
	qt.mwin->menubar->setVisible(false);
	qt.mwin->toolbar->setVisible(false);
	qt.mwin->statusbar->setVisible(false);
	qt.mwin->setGeometry(0, 0, 1, 1);
}

void *gui_wdgrewind_get_ptr(void) {
	return ((void *)qt.mwin->toolbar->rewind);
}
void gui_wdgrewind_play(void) {
	wdgrewind->toolButton_Play->click();
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

void gui_max_speed_start(void) {
	if (fps.max_speed == TRUE) {
		return;
	}
	qt.mwin->qaction_extern.max_speed.start->only_one_trigger();
}
void gui_max_speed_stop(void) {
	if (fps.max_speed == FALSE) {
		return;
	}
	qt.mwin->qaction_extern.max_speed.stop->only_one_trigger();
}

void gui_decode_all_input_events(void) {
	if (!qt.screen->events.keyb.count() && !qt.screen->events.mouse.count()) {
		return;
	}

	qt.screen->events.mutex.lock();

	// keyboard
	if (qt.screen->events.keyb.count()) {
		for (QList<_wdgScreen_keyboard_event>::iterator e = qt.screen->events.keyb.begin(); e != qt.screen->events.keyb.end(); ++e) {
			_wdgScreen_keyboard_event &event = *e;

			for (BYTE i = PORT1; i < PORT_MAX; i++) {
				if (port_funct[i].input_decode_event && (port_funct[i].input_decode_event(event.mode,
					event.autorepeat, event.event, event.type, &port[i]) == EXIT_OK)) {
					break;
				}
			}
		}
		qt.screen->events.keyb.clear();
	}

	// mouse
	if (qt.screen->events.mouse.count()) {
		for (QList<_wdgScreen_mouse_event>::iterator e = qt.screen->events.mouse.begin(); e != qt.screen->events.mouse.end(); ++e) {
			_wdgScreen_mouse_event &event = *e;

			if ((event.type == QEvent::MouseButtonPress) ||
				(event.type == QEvent::MouseButtonDblClick)) {
				if (event.button == Qt::LeftButton) {
					gmouse.left = TRUE;
				} else if (event.button == Qt::RightButton) {
					gmouse.right = TRUE;
				}
			} else if (event.type == QEvent::MouseButtonRelease) {
				if (event.button == Qt::LeftButton) {
					gmouse.left = FALSE;
				} else if (event.button == Qt::RightButton) {
					gmouse.right = FALSE;
				}
			} else if (event.type == QEvent::MouseMove) {
				gmouse.x = event.x;
				gmouse.y = event.y;
			}
		}
		qt.screen->events.mouse.clear();
	}

	qt.screen->events.mutex.unlock();
}

void gui_screen_update(void) {
#if defined (WITH_OPENGL)
	qt.screen->wogl->update();
#elif defined (WITH_D3D9)
	qt.screen->wd3d9->update();
#endif
	qt.dset->widget_Settings_Video->widget_Palette_Editor->widget_Palette_PPU->update();
}

void *gui_wdgoverlayui_get_ptr(void) {
	return ((void *)qt.overlay);
}

void *gui_dlgsettings_get_ptr(void) {
	return ((void *)qt.dset);
}
void gui_dlgsettings_input_update_joy_combo(void) {
	qt.dset->widget_Settings_Input->update_joy_list();
}

void gui_external_control_windows_show(void) {
	if (ext_win.vs_system && (cfg->fullscreen != FULLSCR)) {
		qt.vssystem->update_dialog();
		qt.vssystem->show();
	} else {
		qt.vssystem->hide();
	}

	gui_update();
	gui_external_control_windows_update_pos();
	gui_active_window();
	gui_set_focus();
}
void gui_external_control_windows_update_pos(void) {
	unsigned int y = 0;

	y += qt.vssystem->update_pos(y);
}

void gui_vs_system_update_dialog(void) {
	qt.vssystem->update_dialog();
}
void gui_vs_system_insert_coin(void) {
	if (vs_system.enabled == TRUE) {
		qt.vssystem->insert_coin(1);
	}
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
	gfx.bit_per_pixel = qt.app->primaryScreen()->depth();
}

uint32_t gui_color(BYTE a, BYTE r, BYTE g, BYTE b) {
	return (qRgba(r, g, b, a));
}
#endif

BYTE gui_load_lut(void *l, const uTCHAR *path) {
	QImage tmp;
	_lut *lut = (_lut *)l;

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
void gui_save_screenshot(int w, int h, int stride, char *buffer, BYTE flip) {
	QString basename = QString(uQString(info.base_folder)) + QString(SCRSHT_FOLDER) + "/"
		+ QFileInfo(uQString(info.rom.file)).completeBaseName();
	QImage screenshot = QImage((uchar *)buffer, w, h, stride, QImage::Format_RGB32);
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

#if !defined (_WIN32)
unsigned int gui_hardware_concurrency(void) {
	return (std::thread::hardware_concurrency());
}
#endif

#if defined (__linux__)
#include "os_linux.h"
#elif defined (__OpenBSD__) || defined (__FreeBSD__)
#include "os_bsd.h"
#elif defined (_WIN32)
#include "os_windows.h"
#endif

static void gui_is_in_desktop(int *x, int *y) {
	QList<QScreen *> screens = QGuiApplication::screens();
	int i, x_min = 0, x_max = 0, y_min = 0, y_max = 0;

	for (i = 0; i < screens.count(); i++) {
		QRect g = screens[i]->availableGeometry();

		if (g.x() < x_min) {
			x_min = g.x();
		}
		if ((g.x() + g.width()) > x_max) {
			x_max = g.x() + g.width();
		}
		if (g.y() < y_min) {
			y_min = g.y();
		}
		if ((g.y() + g.height()) > y_max) {
			y_max = g.y() + g.height();
		}
	}
	if (((*x) == 0) || ((*x) < x_min) || ((*x) > x_max)) {
		(*x) = 80;
	}
	if (((*y) == 0) || ((*y) < y_min) || ((*y) > y_max)) {
		(*y) = 80;
	}
}
