/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtWidgets/QShortcut>
#else
#include <QtGui/QShortcut>
#endif
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtCore/QPoint>
#include <QtCore/QRegularExpression>
#include <QtGui/QValidator>
#include "conf.h"
#include "settings.h"
#include "os_jstick.h"
#include "ui_mainWindow.h"
#include "wdgScreen.hpp"
#include "wdgStatusBar.hpp"
#include "wdgToolBar.hpp"
#include "dlgCmdLineHelp.hpp"

// ----------------------------------------------------------------------------------------------

class toUpValidator final : public QValidator {
	public:
		explicit toUpValidator(QObject *parent = nullptr);

	public:
		State validate(QString &input, int &pos) const override;
};

// ----------------------------------------------------------------------------------------------

class qtHelper {
	public:
		static QRegularExpression rx_any_numbers;
		static QRegularExpression rx_comment_0;
		static QRegularExpression rx_comment_1;

	public:
		static void widget_set_visible(void *wdg, bool mode);
		static void pushbutton_set_checked(void *btn, bool mode);
		static void checkbox_set_checked(void *cbox, bool mode);
		static void slider_set_value(void *slider, int value);
		static void spinbox_set_value(void *sbox, int value);
		static void combox_set_index(void *cbox, int value);
		static void lineedit_set_text(void *ledit, const QString &txt);
};

// ----------------------------------------------------------------------------------------------

class timerEgds final : public QTimer {
	Q_OBJECT

	private:
		enum with_emu_pause {
			EGDS_PAUSE,
			EGDS_TURN_OFF,
			EGDS_TOTALS
		};
		struct _calls {
			int count;
		} calls[EGDS_TOTALS];

	public:
		explicit timerEgds(QObject *parent = nullptr);
		~timerEgds() override;

	public:
		void set_fps(void);
		void stop_unnecessary(void);
		void start_pause(void);
		void stop_pause(void);
		void start_rwnd(void);
		void stop_rwnd(void);
		void start_ff(void);
		void stop_ff(void);
		void start_max_speed(void);
		void stop_max_speed(void);
		void start_turn_off(void);
		void stop_turn_off(void);

	private:
		void _start(void);
		void _start_with_emu_thread_pause(enum with_emu_pause type);
		void _stop(BYTE is_necessary);
		void _stop_with_emu_thread_continue(enum with_emu_pause type, BYTE is_necessary);
		void _etc(enum with_emu_pause type) const;

	private slots:
		static void s_draw_screen(void);
};

// ----------------------------------------------------------------------------------------------

class actionOneTrigger : public QAction {
	public:
		unsigned int count;
		QMutex mutex;
		double last;

	public:
		explicit actionOneTrigger(QObject *parent = nullptr);
		~actionOneTrigger() override;

	public:
		void only_one_trigger(void);
		void reset_count(void);
};

// ----------------------------------------------------------------------------------------------

class mainWindow : public QMainWindow, public Ui::mainWindow {
	Q_OBJECT

	public:
		enum _toggle_menubar_mode {
			TOGGLE_MENUBAR_NONE,
			TOGGLE_MENUBAR_FROM_SHORTCUT,
			TOGGLE_MENUBAR_FROM_MOUSEMOVE
		};

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
			QAction *integer_in_fullscreen;
			QAction *stretch_in_fullscreen;
			QAction *toggle_menubar_in_fullscreen;
			QAction *toggle_capture_input;
			QAction *audio_enable;
			QAction *save_settings;
			QAction *hold_fast_forward;
			struct _qaction_shcut_extern_rwnd {
				QAction *active;
				QAction *step_backward;
				QAction *fast_backward;
				QAction *play;
				QAction *pause;
				QAction *fast_forward;
				QAction *step_forward;
			} rwnd;
		} qaction_shcut;
		struct _qaction_extern {
			struct _qaction_extern_max_speed {
				actionOneTrigger *start;
				actionOneTrigger *stop;
			} max_speed;
		} qaction_extern;
		// ext_gfx_draw_screen
		timerEgds *egds;
		wdgScreen *wscreen;
		wdgStatusBar *statusbar;
		wdgToolBar *toolbar;
		QShortcut *shortcut[SET_MAX_NUM_SC];
		BYTE tmm;

	public:
		struct _shcjoy {
			bool enabled;
			QTimer *timer;
			_js_sch sch;
			DBWORD shortcut[SET_MAX_NUM_SC];
		} shcjoy;
		struct _visibility {
			bool menubar;
			bool toolbars;
		} visibility;
		struct _secondary_instance {
			QMutex mutex;
			QString message;
		} secondary_instance;
		struct _nsf_author_note {
			wdgDlgCmdLineHelp *dlg;
			QRect geom;
		} nsf_author_note;
		QTranslator *translator;
		QTranslator *qtTranslator;

	signals:
		void et_reset(BYTE type);
		void et_gg_reset(void);
		void et_vs_reset(void);
		void et_external_control_windows_show(void);
		void et_toggle_menubar_from_mouse(void);
		void et_nsf_author_note_open(const uTCHAR *string);
		void et_nsf_author_note_close(void);
		void et_set_fullscreen(void);
		void et_toggle_gui_in_window(void);
		void et_quit(void);

	public:
		mainWindow();
		~mainWindow() override;

	protected:
		void changeEvent(QEvent *event) override;

	private:
		void retranslateUi(QMainWindow *mainWindow);

	public:
		void update_window(void);
		void update_recording_widgets(void) const;
		void set_language(int lang) const;
		void shcjoy_start(void);
		void shcjoy_stop(void);
		static void control_visible_cursor(void);
		void make_reset(int type);
		void change_rom(const uTCHAR *rom);
		void change_disk(const QString &disk);
		void shortcuts(void) const;
		bool is_rwnd_shortcut_or_not_shcut(const QKeyEvent *event) const;
		QAction *state_save_slot_action(BYTE slot) const;
		void state_save_slot_set(int slot, bool on_video);
		void state_save_slot_set_tooltip(BYTE slot) const;
		void toggle_toolbars(void) const;
		void reset_min_max_size(void);
		static void shout_into_mic(BYTE mode);
		void hold_fast_forward(BYTE mode) const;
		static void open_dkeyb(void);
		void unsupported_hardware(void);

	private:
		void connect_menu_signals(void) const;
		void connect_action(const QAction *action, const char *member) const;
		void connect_action(QAction *action, int value, const char *member) const;
		static void connect_shortcut(QAction *action, int index);
		void connect_shortcut(QAction *action, int index, const char *member) const;

	private:
		void update_menu_file(void);
		void update_menu_nes(void) const;
		void update_menu_state(void) const;

	public:
		void update_fds_menu(void) const;
		void update_tape_menu(void) const;
		void update_menu_tools(void) const;
		void update_menu_recent_roms(void);
		void update_menu_recent_disks(void);

	private:
		static void action_text(QAction *action, const QString &description, const QString *scut);
		static void ctrl_disk_side(QAction *action);
		int is_shortcut(const QKeyEvent *event) const;
		void toggle_menubar(BYTE mode);

	public slots:
		void s_set_fullscreen(void);
		static void s_set_vs_window(void);
		static void s_set_detach_barcode_window(void);
		static void s_open_dkeyb(void);

	private slots:
		static void s_fake_slot(void);
		void s_open(void);
		void s_apply_patch(void);
		static void s_open_edit_current_header(void);
		void s_open_recent_roms(void);
		static void s_open_config_folder(void);
		static void s_open_working_folder(void);
		void s_quit(void);
		void s_turn_on_off(void);
		void s_make_reset(void);
		static void s_insert_coin(void);
		static void s_shout_into_mic(void);
		void s_disk_side(void) const;
		void s_eject_disk(void) const;
		void s_change_disk(void);
		void s_open_recent_disks(void);
		void s_create_empty_disk(void);
		void s_export_fds_image(void);
		void s_start_stop_audio_recording(void);
#if defined (WITH_FFMPEG)
		void s_start_stop_video_recording(void);
#endif
		static void s_save_screenshot(void);
		static void s_save_screenshot_1x(void);
		void s_pause(void) const;
		void s_fast_forward(void) const;
		void s_max_speed_start(void) const;
		void s_max_speed_stop(void) const;
		void s_toggle_gui_in_window(void);
		void s_open_settings(void) const;
		void s_state_save_slot_action(void);
		void s_state_save_slot_incdec(void);
		void s_state_save_slot_set(void);
		void s_state_save_file(void);
		void s_state_load_file(void);
		static void s_open_ddip(void);
		static void s_open_djsc(void);
		void s_tape_play(void);
		void s_tape_record(void);
		static void s_tape_stop(void);
		static void s_show_log(void);
		void s_help(void);

	private slots:
		void s_shcjoy_read_timer(void);
		void s_received_message(quint32 instanceId, const QByteArray &message);
		void s_exec_message(void);
		void s_nsf_author_note_close(void);

	private slots:
		void s_shcut_mode(void) const;
		void s_shcut_scale(void) const;
		static void s_shcut_interpolation(void);
		static void s_shcut_integer_in_fullscreen(void);
		static void s_shcut_stretch_in_fullscreen(void);
		static void s_shcut_audio_enable(void);
		static void s_shcut_save_settings(void);
		void s_shcut_rwnd_active_deactive_mode(void) const;
		void s_shcut_rwnd_step_backward(void) const;
		void s_shcut_rwnd_fast_backward(void) const;
		void s_shcut_rwnd_play(void) const;
		void s_shcut_rwnd_pause(void) const;
		void s_shcut_rwnd_fast_forward(void) const;
		void s_shcut_rwnd_step_forward(void) const;
		void s_shcut_toggle_menubar(void);
		void s_shcut_toggle_capture_input(void) const;

	private slots:
		void s_et_reset(BYTE type);
		void s_et_gg_reset(void);
		void s_et_vs_reset(void);
		static void s_et_external_control_windows_show(void);
		void s_et_toggle_menubar_from_mouse(void);
		void s_et_nsf_author_note_open(const uTCHAR *string);
		void s_et_nsf_author_note_close(void) const;
};

// ----------------------------------------------------------------------------------------------

class wdgDlgMainWindow final : public wdgTitleBarDialog {
	Q_OBJECT

	public:
		mainWindow *wd;

	private:
		bool fullscreen_resize;
		bool setup_in_out_fullscreen;
		QRect org_geom, fs_geom;

	public:
		explicit wdgDlgMainWindow(QWidget *parent = nullptr);
		~wdgDlgMainWindow() override;

	protected:
#if defined (_WIN32)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);
#else
		bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#endif
#endif
		bool eventFilter(QObject *obj, QEvent *event) override;
		void closeEvent(QCloseEvent *event) override;
		void resizeEvent(QResizeEvent *event) override;

	public:
		QScreen *win_handle_screen(void) const;
		void update_gfx_monitor_dimension(bool adjust_fs_geom);
		void set_dialog_geom(QRect &new_geom) const;

	private:
		void set_fullscreen(void);
		static void geom_to_cfg(const QRect &save_geom, _last_geometry *lg);

	private slots:
		void s_set_fullscreen(void);
		void s_toggle_gui_in_window(void);
};

#endif /* MAINWINDOW_HPP_ */
