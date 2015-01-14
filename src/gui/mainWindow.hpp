/*
 * mainWindow.hpp
 *
 *  Created on: 17/ott/2014
 *      Author: fhorse
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
#include "application.hh"
#include "settings.h"

class mainWindow: public QMainWindow {
		Q_OBJECT

	public:
		sbarWidget *statusbar;
		QTimer *timer_draw;

	private:
		Ui::mainWindow *ui;
		QShortcut *shortcut[SET_MAX_NUM_SC];
		QPoint position;

	public:
		mainWindow(Ui::mainWindow *u);
		~mainWindow();
		void setup();
		void update_window();
		void change_rom(const char *rom);
		void state_save_slot_set(int slot);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		void setup_video_rendering();
		void update_recent_roms();
		void update_menu_nes();
		void update_menu_settings();
		void update_menu_state();
		void ctrl_disk_side(QAction *action);
		void shortcuts();
		void connect_shortcut(QAction *action, int index);
		void connect_shortcut(QAction *action, int index, const char *member);
		void connect_menu_signals();
		void connect_action(QAction *action, const char *member);
		void connect_action(QAction *action, int value, const char *member);
		void make_reset(int type);
		void set_filter(int filter);

	public slots:
		static void s_set_effect();
		void s_set_fullscreen();

	private slots:
		void s_loop();
		void s_open();
		void s_open_recent_roms();
		void s_quit();
		void s_make_reset();
		void s_disk_side();
		void s_eject_disk();
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
		void s_set_palette();
		void s_save_palette();
		void s_load_palette();
		void s_set_vsync();
		void s_set_interpolation();
		void s_set_txt_on_screen();
		void s_set_stretch();
		void s_set_samplerate();
		void s_set_channels();
		void s_set_stereo_delay();
		void s_set_audio_quality();
		void s_set_apu_channels();
		void s_set_audio_swap_duty();
		void s_set_audio_enable();
		void s_set_input();
		void s_set_pause();
		void s_gamegenie_select();
		void s_set_save_on_exit();
		void s_save_settings();
		void s_state_save_slot_action();
		void s_state_save_slot_incdec();
		void s_state_save_slot_set();
		void s_help();
		void s_timer_redraw();
};

#endif /* MAINWINDOW_HPP_ */
