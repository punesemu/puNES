/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QShortcut>
#else
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QShortcut>
#endif
#include <QtCore/QObject>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtCore/QPoint>
#include <QtGui/QCloseEvent>
#include "sbarWidget.hpp"
#include "cheatObject.hpp"
#include "application.hh"
#include "settings.h"
#include "jstick.h"

#define parentMain ((mainWindow *)parent())

class mainWindow: public QMainWindow {
		Q_OBJECT

	public:
		cheatObject *chobj;
		sbarWidget *statusbar;
		Ui::mainWindow *ui;
		struct _shcjoy {
			bool enabled;
			QTimer *timer;
			_js joy;
			_js_sch sch;
			DBWORD shortcut[SET_MAX_NUM_SC];
		} shcjoy;
		QString last_import_cheat_path;

	private:
		QShortcut *shortcut[SET_MAX_NUM_SC];
		QPoint position;
		QTranslator *translator;
		QTranslator *qtTranslator;

	public:
		mainWindow(Ui::mainWindow *u, cheatObject *cho);
		~mainWindow();
		void setup();
		void update_window();
		void change_rom(const char *rom);
		void state_save_slot_set(int slot, bool on_video);
		void shortcuts();
		void shcjoy_start();
		void shcjoy_stop();
		void control_visible_cursor();
		void make_reset(int type);

	signals:
		void fullscreen(bool state);

	protected:
#if defined (__WIN32__)
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
		bool winEvent(MSG *msg, long *result);
#else
		bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
#endif
#endif
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		void set_language(int lang);
		void setup_video_rendering();
		void update_recent_roms();
		void update_menu_nes();
		void update_menu_settings();
		void update_menu_tools();
		void update_menu_state();
		void ctrl_disk_side(QAction *action);
		void connect_shortcut(QAction *action, int index);
		void connect_shortcut(QAction *action, int index, const char *member);
		void connect_menu_signals();
		void connect_action(QAction *action, const char *member);
		void connect_action(QAction *action, int value, const char *member);
		void set_filter(int filter);

	public slots:
		void s_set_fullscreen();

	private slots:
		void s_fullscreen(bool state);
		void s_loop();
		void s_open();
		void s_open_recent_roms();
		void s_open_working_folder();
		void s_quit();
		void s_turn_on_off();
		void s_make_reset();
		void s_insert_coin();
		void s_disk_side();
		void s_eject_disk();
		void s_pause();
		void s_fast_forward();
		void s_save_screenshot();
		void s_set_vs_window();
		void s_set_apu_channels();
		void s_set_ppu_hacks();
		void s_set_mode();
		void s_set_rendering();
		void s_set_fps();
		void s_set_fsk();
		void s_set_scale();
		void s_set_par();
		void s_set_par_stretch();
		void s_set_overscan();
		void s_set_overscan_borders();
		void s_set_other_filter();
		void s_set_ntsc_filter();
		void s_load_shader();
		void s_set_palette();
		void s_set_disable_emphasis_pal();
		void s_save_palette();
		void s_load_palette();
		void s_set_vsync();
		void s_set_interpolation();
		void s_set_txt_on_screen();
#if defined (WITH_OPENGL)
		void s_set_disable_srgb_fbo();
#endif
		void s_set_stretch();
		void s_set_audio_buffer_factor();
		void s_set_samplerate();
		void s_set_channels();
		void s_set_stereo_delay();
		void s_set_audio_quality();
		void s_set_audio_swap_duty();
		void s_set_audio_enable();
		void s_set_language();
		void s_set_ff_velocity();
		void s_set_input();
		void s_set_hide_sprites();
		void s_set_hide_background();
		void s_set_unlimited_sprites();
		void s_set_ppu_overclock();
		void s_set_save_battery_ram_file();
		void s_set_pause_in_background();
		void s_cheat_mode_select();
		void s_cheat_dialog();
		void s_set_save_on_exit();
		void s_save_settings();
		void s_state_save_slot_action();
		void s_state_save_slot_incdec();
		void s_state_save_slot_set();
		void s_state_save_file();
		void s_state_load_file();
		void s_help();
		void s_shcjoy_read_timer();
};

#endif /* MAINWINDOW_HPP_ */
