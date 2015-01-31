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
#include "clock.h"
#include "timeline.h"
#include "save_slot.h"
#include "mainWindow.hpp"
#include "screenWidget.hpp"
#include "sbarWidget.hpp"
#include "dlgUncomp.hpp"
#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDesktopWidget>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QDesktopWidget>
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
			qt.screen->height() +
			(qt.ui->menubar->isHidden() ? 0 : qt.ui->menubar->sizeHint().height()) +
			(qt.mwin->statusbar->isHidden() ? 0 : qt.mwin->statusbar->sizeHint().height())));

	qt.ui->menubar->setFixedWidth(gfx.w[VIDEO_MODE]);
	qt.mwin->statusbar->update_width(gfx.w[VIDEO_MODE]);
}

void gui_update(void) {
	char title[255];

	gui.in_update = TRUE;

	emu_set_title(title);
	qt.mwin->setWindowTitle(title);
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
	qt.mwin->state_save_slot_set(slot);
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
