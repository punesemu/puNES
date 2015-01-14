/*
 * qt.c
 *
 *  Created on: 17/ott/2014
 *      Author: fhorse
 */

#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include "application.hh"
#include "tas.h"
#include "version.h"
#include "conf.h"
#include "timeline.h"
#include "save_slot.h"
#include "mainWindow.hpp"
#include "screenWidget.hpp"
#include "sbarWidget.hpp"
#include "dlgUncomp.hpp"
#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMessageBox>
#endif
#include "gui.h"
#if defined (SDL) && defined (__WIN32__)
#include "sdl_wid.h"
#endif

struct _qt {
	QApplication *app;
	Ui::mainWindow *ui;
	mainWindow *mwin;
	screenWidget *screen;
	QTranslator *tr;
} qt;

void gui_quit(void) {}
BYTE gui_create(void) {
	qt.ui = new Ui::mainWindow;
	qt.mwin = new mainWindow(qt.ui);
	qt.tr = new QTranslator();

	qt.tr->load("it", ":/tr/translations");
	qt.app->installTranslator(qt.tr);

	qt.ui->setupUi(qt.mwin);
	qt.screen = new screenWidget(qt.ui->centralwidget, qt.mwin);

	qt.mwin->setup();
	qt.mwin->show();

	return (EXIT_OK);
}
void gui_start(void) {
	QTimer *timer = new QTimer(qt.mwin);

	QObject::connect(timer, SIGNAL(timeout()), qt.mwin, SLOT(s_loop()));
	timer->start();

	gui.start = TRUE;

	qApp->exec();
}
void gui_event(void) {
	gui_flush();

	if (info.no_rom | info.pause) {
		return;
	}

	if (tas.type) {
		tas_frame();
		return;
	}

	{
		BYTE i;

		for (i = PORT1; i < PORT_MAX; i++) {
			if (input_add_event[i]) {
				input_add_event[i](i);
			}
		}
	}
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

	qt.mwin->setFixedSize(QSize(gfx.w[VIDEO_MODE],
			gfx.h[VIDEO_MODE] +
			(qt.ui->menubar->isHidden() ? 0 : qt.ui->menubar->height()) +
			(qt.mwin->statusbar->isHidden() ? 0 : qt.mwin->statusbar->height())));

	qt.screen->setFixedSize(QSize(gfx.w[VIDEO_MODE], gfx.h[VIDEO_MODE]));

	qt.mwin->statusbar->update_width(gfx.w[VIDEO_MODE]);
}

void gui_update(void) {
	char title[255];

	gui.in_update = TRUE;

	emu_set_title(title);
	qt.mwin->setWindowTitle(title);
	qt.mwin->update_window();

	gui.in_update = FALSE;
	gui_flush();
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
	qt.mwin->state_save_slot_set(slot);
}

void gui_flush(void) {
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
#if defined (SDL) && defined (__WIN32__)
	sdl_wid();
	gfx_reset_video();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
#endif
}
int gui_uncompress_selection_dialog(void) {
	dlgUncomp *dlg = new dlgUncomp(qt.mwin);

	dlg->show();
	dlg->exec();

	return (gui.dlg_rc);
}

void gui_after_set_video_mode(void) {
#if defined (SDL) && defined (__WIN32__)
	qt.screen->controlEventFilter();
#endif
}
void gui_set_focus(void) {
	qt.screen->setFocus(Qt::ActiveWindowFocusReason);
}
void gui_timeout_redraw_start() {
	qt.mwin->timer_draw->start(50);
}
void gui_timeout_redraw_stop() {
	qt.mwin->timer_draw->stop();
}

#if defined (__WIN32__)
#include "os_windows.h"
#else
#include "os_linux.h"
#endif
