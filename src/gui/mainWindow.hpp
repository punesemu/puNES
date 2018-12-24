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

#ifndef MAINWINDOW_HPP_
#define MAINWINDOW_HPP_

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QShortcut>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtCore/QPoint>
#include "settings.h"
#include "jstick.h"
#include "application.hh"
#include "wdgScreen.hpp"
#include "wdgStatusBar.hpp"

class mainWindow : public QMainWindow, public Ui::mainWindow {
		Q_OBJECT

	public:
		struct _qaction_shcut_extern {
			QAction *mode_auto;
			QAction *mode_ntsc;
			QAction *mode_pal;
			QAction *mode_dendy;
			QAction *scale_1x;
			QAction *scale_2x;
			QAction *scale_3x;
			QAction *scale_4x;
			QAction *scale_5x;
			QAction *scale_6x;
			QAction *interpolation;
			QAction *stretch_in_fullscreen;
			QAction *audio_enable;
			QAction *save_settings;
		} qaction_shcut;
		struct _shcjoy {
			bool enabled;
			QTimer *timer;
			_js_sch sch;
			DBWORD shortcut[SET_MAX_NUM_SC];
		} shcjoy;
		QTimer *ff;

	public:
		wdgScreen *screen;
		wdgStatusBar *statusbar;

	private:
		QShortcut *shortcut[SET_MAX_NUM_SC];
		QPoint position;
		QTranslator *translator;
		QTranslator *qtTranslator;

	public:
		mainWindow();
		~mainWindow();

	signals:
		void et_gg_reset(void);
		void et_vs_reset(void);
		void et_external_control_windows_show(void);
		void fullscreen(bool state);

	protected:
#if defined (__WIN32__)
		bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#endif
		bool eventFilter(QObject *obj, QEvent *event);
		void changeEvent(QEvent *event);
		void closeEvent(QCloseEvent *event);
		void moveEvent(QMoveEvent *event);
		void resizeEvent(QResizeEvent *event);

	private:
		void retranslateUi(mainWindow *mainWindow);

	public:
		void update_window(void);
		void set_language(int lang);
		void shcjoy_start(void);
		void shcjoy_stop(void);
		void control_visible_cursor(void);
		void make_reset(int type);
		void change_rom(const uTCHAR *rom);
		void state_save_slot_set(int slot, bool on_video);

	private:
		void connect_menu_signals(void);
		void connect_action(QAction *action, const char *member);
		void connect_action(QAction *action, int value, const char *member);
		void shortcuts(void);
		void connect_shortcut(QAction *action, int index);
		void connect_shortcut(QAction *action, int index, const char *member);

	private:
		void update_menu_file(void);
		void update_menu_nes(void);
		void update_menu_tools(void);
		void update_menu_state(void);

	private:
		void ctrl_disk_side(QAction *action);

	private slots:
		void s_open(void);
		void s_apply_patch(void);
		void s_open_recent_roms(void);
		void s_open_working_folder(void);
		void s_quit(void);
		void s_turn_on_off(void);
		void s_make_reset(void);
		void s_insert_coin(void);
		void s_disk_side(void);
		void s_eject_disk(void);
		void s_start_stop_wave(void);
		void s_fast_forward(void);
	public slots:
		void s_set_fullscreen(void);
	private slots:
		void s_save_screenshot(void);
		void s_pause(void);
		void s_open_settings(void);
		void s_state_save_slot_action(void);
		void s_state_save_slot_incdec(void);
		void s_state_save_slot_set(void);
		void s_state_save_file(void);
		void s_state_load_file(void);
	public slots:
		void s_set_vs_window(void);
	private slots:
		void s_help(void);

	private slots:
		void s_ff_draw_screen(void);
		void s_fullscreen(bool state);
		void s_shcjoy_read_timer(void);

	private slots:
		void s_shcut_mode(void);
		void s_shcut_scale(void);
		void s_shcut_interpolation(void);
		void s_shcut_stretch_in_fullscreen(void);
		void s_shcut_audio_enable(void);
		void s_shcut_save_settings(void);

	private slots:
		void s_et_gg_reset(void);
		void s_et_vs_reset(void);
		void s_et_external_control_windows_show(void);
};

#endif /* MAINWINDOW_HPP_ */
