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

#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include "application.hh"
#include "tas.h"
#include "version.h"
#include "conf.h"
#include "clock.h"
#include "timeline.h"
#include "save_slot.h"
#include "vs_system.h"
#include "mainWindow.hpp"
#include "screenWidget.hpp"
#include "sbarWidget.hpp"
#include "dlgUncomp.hpp"
#include "dlgVsSystem.hpp"
#include "dlgApuChannels.hpp"
#include "dlgPPUHacks.hpp"
#include "pStyle.hpp"
#include "cheatObject.hpp"
#include "pMenu.hpp"
#if defined (WITH_OPENGL)
#include "opengl.h"
#if defined (__WIN32__)
#include "sdl_wid.h"
#endif
#elif defined (WITH_D3D9)
#include "d3d9.h"
#endif
#include "gui.h"
#include <QtCore/QtGlobal>
#include <QtGui/QImage>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDesktopWidget>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMessageBox>
#endif

static void gui_pmenu_management(void);
static void gui_enable_pmenu(QWidget *parent, QAction *action, QList<pMenu*> *pmenus);

static struct _qt {
	QApplication *app;
	Ui::mainWindow *ui;
	mainWindow *mwin;
	screenWidget *screen;
	cheatObject *chobj;
	QImage qimage;

	// controlli esterni
	dlgVsSystem *vssystem;
	dlgApuChannels *apuch;
	dlgPPUHacks *ppuhacks;
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
	qt.ui = new Ui::mainWindow;
	qt.mwin = new mainWindow(qt.ui, qt.chobj);

	qt.app->setStyle(new pStyle());
	qt.app->installEventFilter(new appEventFilter());

	qt.ui->setupUi(qt.mwin);
	qt.screen = new screenWidget(qt.ui->centralwidget, qt.mwin);

	qt.mwin->setup();

	{
		int screenNumber = qApp->desktop()->screenNumber(qt.mwin);

		if (cfg->last_pos.x > qApp->desktop()->screenGeometry(screenNumber).width()) {
			cfg->last_pos.x = 0;
		}
		if (cfg->last_pos.y > qApp->desktop()->screenGeometry(screenNumber).height()) {
			cfg->last_pos.x = 0;
		}
		qt.mwin->move(QPoint(cfg->last_pos.x, cfg->last_pos.y));
	}

	qt.mwin->show();

	memset(&ext_win, 0x00, sizeof(ext_win));
	qt.vssystem = new dlgVsSystem(qt.mwin);
	qt.apuch = new dlgApuChannels(qt.mwin);
	qt.ppuhacks = new dlgPPUHacks(qt.mwin);

	gmouse.hidden = FALSE;
	gmouse.timer = gui_get_ms();

	gui_pmenu_management();

	return (EXIT_OK);
}
void gui_start(void) {
	QTimer *timer = new QTimer(qt.mwin);

	QObject::connect(timer, SIGNAL(timeout()), qt.mwin, SLOT(s_loop()));
	timer->start();

	gui.start = TRUE;

	/*
	 * questi settaggi prima li facevo nell'emu_loop prima di avviare
	 * il loop.
	 */
	{
		/*
		 * ho notato che (sotto windows, per linux ho visto
		 * un lieve peggioramento) settandol'affinity di questo
		 * thread su un singolo core,le prestazioni migliorano
		 * notevolmente. In questo caso setto l'uso del core 0.
		 */
		//#if defined (__WIN32__)
		//	guiSetThreadAffinity(0);
		//#endif

		fps.second_start = gui_get_ms();
		fps.next_frame = gui_get_ms() + machine.ms_frame;
	}

	qApp->exec();
}

void gui_set_video_mode(void) {
	if (cfg->scale == X1) {
		qt.mwin->statusbar->state->setVisible(false);
		if (overscan.enabled) {
			qt.ui->menu_Help->menuAction()->setVisible(false);
		} else {
			qt.ui->menu_Help->menuAction()->setVisible(true);
		}
	} else {
		qt.mwin->statusbar->state->setVisible(true);
		qt.ui->menu_Help->menuAction()->setVisible(true);
	}

	qt.screen->setFixedSize(QSize(gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE]));

	qt.mwin->setFixedSize(QSize(qt.screen->width(),
			(qt.ui->menubar->isHidden() ? 0 : qt.ui->menubar->sizeHint().height()) +
			(qt.screen->height() + 2) +
			(qt.mwin->statusbar->isHidden() ? 0 : qt.mwin->statusbar->sizeHint().height())));

	qt.ui->menubar->setFixedWidth(gfx.w[VIDEO_MODE]);
	qt.mwin->statusbar->update_width(gfx.w[VIDEO_MODE]);
}

void gui_update(void) {
	uTCHAR title[255];

	gui.in_update = TRUE;

	emu_set_title(title, usizeof(title));
	qt.mwin->setWindowTitle(uQString(title));
	qt.mwin->update_window();

	gui.in_update = FALSE;
}

void gui_fullscreen(void) {
	qt.mwin->s_set_fullscreen();
}
void gui_timeline(void) {
	tl.update = TRUE;
	qt.mwin->statusbar->timeline->setValue(tl.snaps_fill - 1, false);
	tl.update = FALSE;
}
void gui_save_slot(BYTE slot) {
	if (slot >= SAVE_SLOTS) {
		slot = SAVE_SLOTS - 1;
	}
	qt.mwin->state_save_slot_set(slot, FALSE);
}

void gui_flush(void) {
	qApp->flush();
	qApp->sendPostedEvents();
	qApp->processEvents();
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
void gui_reset_video(void) {
#if defined (WITH_OPENGL) && defined (__WIN32__)
	sdl_wid();
	gfx_reset_video();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
#endif
}
int gui_uncompress_selection_dialog(void) {
	dlgUncomp *dlg = new dlgUncomp(qt.mwin);

	dlg->show();
	dlg->exec();

	return (gui.dlg_rc);
}

void gui_control_pause_bck(WORD type) {
	if (type == QEvent::WindowActivate) {
		if ((cfg->bck_pause == TRUE) && (++gui.main_win_lfp == 1)) {
			emu_pause(FALSE);
		}
	} else if (type == QEvent::WindowDeactivate) {
		if ((cfg->bck_pause == TRUE) && (--gui.main_win_lfp == 0)) {
			emu_pause(TRUE);
		}
	}
}

void gui_after_set_video_mode(void) {
#if defined (WITH_OPENGL) && defined (__WIN32__)
	qt.screen->controlEventFilter();
#endif
}
void gui_active_window(void) {
	qt.screen->activateWindow();
}
void gui_set_focus(void) {
	qt.screen->setFocus(Qt::ActiveWindowFocusReason);
}

void gui_cheat_init(void) {
	qt.chobj = new cheatObject(0);
}
void gui_cheat_read_game_cheats(void) {
	qt.chobj->read_game_cheats();
}
void gui_cheat_save_game_cheats(void) {
	qt.chobj->save_game_cheats();
}

void gui_cursor_init(void) {
#if defined (__WIN32__)
	qt.screen->cursor_init();
#endif
}
void gui_cursor_set(void) {
#if defined (__WIN32__)
	qt.screen->cursor_set();
#endif
}
void gui_cursor_hide(BYTE hide) {
	gmouse.hidden = hide;
#if defined (__WIN32__)
	qt.screen->cursor_hide(hide);
#else
	gfx_cursor_hide(hide);
#endif
}
void gui_control_visible_cursor(void) {
	qt.mwin->control_visible_cursor();
}

void gui_mainWindow_make_reset(BYTE type) {
	qt.mwin->make_reset(type);
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
	gui_flush();
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

void gui_apu_channels_update_dialog(void) {
	qt.apuch->update_dialog();
}

void gui_ppu_hacks_update_dialog(void) {
	qt.ppuhacks->update_dialog();
}
void gui_ppu_hacks_lag_counter_update(void) {
	if (ext_win.ppu_hacks && (cfg->fullscreen != FULLSCR)) {
		qt.ppuhacks->lag_counter_update();
	}
}

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
			+ QFileInfo(uQString(info.rom_file)).completeBaseName();
	QImage screenshot = QImage((uchar *)buffer, w, h, QImage::Format_RGB32);
	QFile file;
	uint count;

	if (!info.rom_file[0]) {
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

static void gui_pmenu_management(void) {
	static QList<pMenu*> pmenus;
	pmenus
		<< qt.ui->menu_Settings
		<< qt.ui->menu_Mode
		<< qt.ui->menu_Video
		<< qt.ui->menu_FPS
		<< qt.ui->menu_Frame_skip
		<< qt.ui->menu_Scale
		<< qt.ui->menu_Pixel_Aspect_Ratio
		<< qt.ui->menu_Overscan
		<< qt.ui->menu_Oscan_Default_Value
		<< qt.ui->menu_Software_Filters
		<< qt.ui->menu_Shader
		<< qt.ui->menu_Palette
		<< qt.ui->menu_Audio
		<< qt.ui->menu_Output_Device
		<< qt.ui->menu_Sample_rate
		<< qt.ui->menu_Channels
		<< qt.ui->menu_Stereo_delay
		<< qt.ui->menu_Buffer_Size_factor
		<< qt.ui->menu_Input
		<< qt.ui->menu_Language
		<< qt.ui->menu_Cheats
		<< qt.ui->menu_Fast_Forward_velocity
		<< qt.ui->menu_PPU
		<< qt.ui->menu_State;

	static QList<QAction*> triggerAndHide;
	triggerAndHide
		<< qt.ui->action_English
		<< qt.ui->action_Italian
		<< qt.ui->action_Russian
		<< qt.ui->action_Save_settings
		<< qt.ui->action_Save_state
		<< qt.ui->action_Load_state;

	for (int i = 0; i < qt.ui->menubar->actions().count(); i++) {
		gui_enable_pmenu(qt.ui->menubar, qt.ui->menubar->actions().at(i), &pmenus);
	}
	for (int i = 0; i < triggerAndHide.count(); i++) {
		triggerAndHide.at(i)->setProperty("triggerAndHide", QVariant(true));
	}

	pmenus.clear();
	triggerAndHide.clear();
}
static void gui_enable_pmenu(QWidget *parent, QAction *action, QList<pMenu*> *pmenus) {
	pMenu *menu = qobject_cast<pMenu*>(action->menu());

	if (menu) {
		for (int i = 0; i < pmenus->count(); i++) {
			if (pmenus->at(i) == menu) {
				menu->setNewMenagement(true);
			}
		}
		for (int i = 0; i < menu->actions().count(); i++) {
			QAction *act = menu->actions().at(i);

			gui_enable_pmenu(menu, act, pmenus);
		}
	}
}

#if defined (__WIN32__)
#include "os_windows.h"
#else
#include "os_linux.h"
#endif
