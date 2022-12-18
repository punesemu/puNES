/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QtWidgets/QDesktopWidget>
#else
#include <QtGui/QWindow>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QActionGroup>
#endif
#include <QtWidgets/QSpinBox>
#include <QtGui/QScreen>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QErrorMessage>
#include <QtCore/QBuffer>
#include "mainWindow.hpp"
#include "extra/singleapplication/singleapplication.h"
#include "dlgHeaderEditor.hpp"
#include "dlgJsc.hpp"
#include "dlgKeyboard.hpp"
#include "dlgLog.hpp"
#include "dlgSettings.hpp"
#include "wdgMenuBar.hpp"
#include "wdgOverlayUi.hpp"
#include "common.h"
#include "emu_thread.h"
#include "clock.h"
#include "recent_roms.h"
#include "fds.h"
#include "patcher.h"
#include "save_slot.h"
#include "version.h"
#include "vs_system.h"
#include "c++/l7zip/l7z.h"
#include "gui.h"
#include "tas.h"
#include "ppu.h"
#include "video/effects/tv_noise.h"
#include "debugger.h"
#include "tape_data_recorder.h"
#if defined (WITH_FFMPEG)
#include "recording.h"
#endif
#if defined (FULLSCREEN_RESFREQ)
#include "video/gfx_monitor.h"
#endif

enum state_incdec_enum { INC, DEC };
enum state_save_enum { SAVE, LOAD };

mainWindow::mainWindow() : QMainWindow() {
	setupUi(this);

	org_geom.setX(100);
	org_geom.setY(100);
	fs_geom.setX(0);
	fs_geom.setY(0);

	wscreen = new wdgScreen(this);
	statusbar = new wdgStatusBar(this);
	toolbar = new wdgToolBar(this);
	translator = new QTranslator();
	qtTranslator = new QTranslator();
	shcjoy.timer = new QTimer(this);
	setup_in_out_fullscreen = false;
	fullscreen_resize = false;
	visibility.menubar = true;
	visibility.toolbars = true;

	{
		QHBoxLayout *layout = new QHBoxLayout(central_widget);

		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(wscreen);
	}

	setWindowIcon(QIcon(":icon/icons/application.png"));
	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
#if defined (_WIN32)
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
#endif

	statusbar->setSizeGripEnabled(false);
	setStatusBar(statusbar);

	// non voglio visualizzare il contexmenu del menu per nascondere la toolbar
	setContextMenuPolicy(Qt::NoContextMenu);
	toolbar->setObjectName("toolbar");
	toolbar->setWindowTitle(tr("Widgets"));
	addToolBar(toolbar->area, toolbar);

	// creo gli shortcuts
	for (QShortcut *&i : shortcut) {
		i = new QShortcut(this);
	}
	// shortcuts esterni
	qaction_shcut.mode_auto = new QAction(this);
	qaction_shcut.mode_ntsc = new QAction(this);
	qaction_shcut.mode_pal = new QAction(this);
	qaction_shcut.mode_dendy = new QAction(this);
	qaction_shcut.scale_1x = new QAction(this);
	qaction_shcut.scale_2x = new QAction(this);
	qaction_shcut.scale_3x = new QAction(this);
	qaction_shcut.scale_4x = new QAction(this);
	qaction_shcut.scale_5x = new QAction(this);
	qaction_shcut.scale_6x = new QAction(this);
	qaction_shcut.interpolation = new QAction(this);
	qaction_shcut.integer_in_fullscreen = new QAction(this);
	qaction_shcut.stretch_in_fullscreen = new QAction(this);
	qaction_shcut.toggle_menubar_in_fullscreen = new QAction(this);
	qaction_shcut.toggle_capture_input = new QAction(this);
	qaction_shcut.audio_enable = new QAction(this);
	qaction_shcut.save_settings = new QAction(this);
	qaction_shcut.hold_fast_forward = new QAction(this);
	qaction_shcut.rwnd.active = new QAction(this);
	qaction_shcut.rwnd.step_backward = new QAction(this);
	qaction_shcut.rwnd.step_forward = new QAction(this);
	qaction_shcut.rwnd.fast_backward = new QAction(this);
	qaction_shcut.rwnd.fast_forward = new QAction(this);
	qaction_shcut.rwnd.play = new QAction(this);
	qaction_shcut.rwnd.pause = new QAction(this);

	// qaction esterni
	qaction_extern.max_speed.start = new actionOneTrigger(this);
	qaction_extern.max_speed.start->setObjectName("max_speed.start");
	qaction_extern.max_speed.stop = new actionOneTrigger(this);
	qaction_extern.max_speed.stop->setObjectName("max_speed.stop");

	{
		QActionGroup *grp;

		// NES
		grp = new QActionGroup(this);
		grp->setExclusive(true);
		grp->addAction(action_Disk_1_side_A);
		grp->addAction(action_Disk_1_side_B);
		grp->addAction(action_Disk_2_side_A);
		grp->addAction(action_Disk_2_side_B);
		grp->addAction(action_Disk_3_side_A);
		grp->addAction(action_Disk_3_side_B);
		grp->addAction(action_Disk_4_side_A);
		grp->addAction(action_Disk_4_side_B);

		// State
		grp = new QActionGroup(this);
		grp->setExclusive(true);
		grp->addAction(action_State_Slot_0);
		grp->addAction(action_State_Slot_1);
		grp->addAction(action_State_Slot_2);
		grp->addAction(action_State_Slot_3);
		grp->addAction(action_State_Slot_4);
		grp->addAction(action_State_Slot_5);
		grp->addAction(action_State_Slot_6);
		grp->addAction(action_State_Slot_7);
		grp->addAction(action_State_Slot_8);
		grp->addAction(action_State_Slot_9);
		grp->addAction(action_State_Slot_A);
		grp->addAction(action_State_Slot_B);
	}

	connect(shcjoy.timer, SIGNAL(timeout()), this, SLOT(s_shcjoy_read_timer()));
	connect(qApp, SIGNAL(receivedMessage(quint32,QByteArray)), this, SLOT(s_received_message(quint32,QByteArray)));

	connect(this, SIGNAL(et_reset(BYTE)), this, SLOT(s_et_reset(BYTE)));
	connect(this, SIGNAL(et_gg_reset()), this, SLOT(s_et_gg_reset()));
	connect(this, SIGNAL(et_vs_reset()), this, SLOT(s_et_vs_reset()));
	connect(this, SIGNAL(et_external_control_windows_show()), this, SLOT(s_et_external_control_windows_show()));

	egds = new timerEgds(this);

	shcjoy_start();

	connect_menu_signals();
	shortcuts();

#if !defined (WITH_FFMPEG)
	action_Recording->setVisible(false);
#endif

	adjustSize();

	installEventFilter(this);

	{
		bool visibile = !info.start_with_hidden_gui;

		menubar->setVisible(visibile);
		toolbar->setVisible(visibile && !cfg->toolbar.hidden);
		statusbar->setVisible(visibile && !cfg->toolbar.hidden);
	}

	set_language(cfg->language);

	if (QString(uQString(gui_config_folder())) == QString(uQString(gui_data_folder()))) {
		action_Open_config_folder->setVisible(false);
	}
}
mainWindow::~mainWindow() = default;

#if defined (_WIN32)
bool mainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result) {
	MSG *msg = (MSG *)message;

	switch (msg->message) {
		case WM_ENTERSIZEMOVE:
			break;
		case WM_EXITSIZEMOVE: {
			QTimer::singleShot(10, this, [this]() {
				HMONITOR monitor = MonitorFromWindow((HWND)winId(), MONITOR_DEFAULTTOPRIMARY);

				gfx_control_changed_adapter(&monitor);
			});
			break;
		}
		case WM_SYSCOMMAND: {
			switch (msg->wParam & 0xFFF0) {
				// disabilito screensaver e spegnimento del monitor
				case SC_MONITORPOWER:
				case SC_SCREENSAVE:
					SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
					(*result) = 0;
					return (true);
			}
			break;
		}
		default:
			break;
	}
	return QWidget::nativeEvent(eventType, message, result);
}
#endif
bool mainWindow::eventFilter(QObject *obj, QEvent *event) {
	switch (event->type()) {
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
			if (!setup_in_out_fullscreen) {
				gui_control_pause_bck(event->type());
			}
			break;
		default:
			break;
	}
	return (QObject::eventFilter(obj, event));
}
void mainWindow::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QMainWindow::changeEvent(event);
	}
}
void mainWindow::moveEvent(QMoveEvent *event) {
	if (gui.start) {
		gui_external_control_windows_update_pos();
	}
	QMainWindow::moveEvent(event);
}
void mainWindow::resizeEvent(QResizeEvent *event) {
	if (gui.start) {
		if (fullscreen_resize && (event->size().width() >= SCR_COLUMNS)) {
			fullscreen_resize = false;
			if (gfx.type_of_fscreen_in_use == FULLSCR_IN_WINDOW) {
				fs_geom = QRect(0, 0, event->size().width(), event->size().height());
				update_gfx_monitor_dimension();
#if !defined (_WIN32)
			} else if (gfx.type_of_fscreen_in_use == FULLSCR) {
				gfx.w[FSCR_RESIZE] = event->size().width();
				gfx.h[FSCR_RESIZE] = event->size().height();
#endif
			}
			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE, FALSE);
		}
		gui_external_control_windows_update_pos();
	}

	QMainWindow::resizeEvent(event);
}
void mainWindow::closeEvent(QCloseEvent *event) {
	dlgsettings->close();

	info.stop = TRUE;

	shcjoy_stop();

	if (cfg->fullscreen == NO_FULLSCR) {
		cfg->lg.x = geometry().x();
		cfg->lg.y = geometry().y();
	}

	geom_to_cfg(dlgsettings->geom, &cfg->lg_settings);
	geom_to_cfg(dlgkeyb->geom, &cfg->lg_nes_keyboard);
	geom_to_cfg(dlglog->geom, &cfg->lg_log);
	geom_to_cfg(dlgheader->geom, &cfg->lg_header_editor);

	settings_save_GUI();

	QMainWindow::closeEvent(event);
}

void mainWindow::retranslateUi(mainWindow *mainWindow) {
	Ui::mainWindow::retranslateUi(mainWindow);
	qaction_shcut.hold_fast_forward->setText(tr("Fast Forward (hold button)"));
	shortcuts();
	if (ppu_screen.rd) {
		save_slot_count_load();
	}
	update_window();
}

void mainWindow::update_window(void) {
	// File
	update_menu_file();
	// NES
	update_menu_nes();
	// Tools
	update_menu_tools();
	// State
	update_menu_state();

	toolbar->update_toolbar();
	statusbar->update_statusbar();
}
void mainWindow::update_recording_widgets(void) {
#if defined (WITH_FFMPEG)
	QIcon ia = QIcon(":/icon/icons/nsf_file.svgz"), iv = QIcon(":/icon/icons/film.svgz");
	QString sa = tr("Start &AUDIO recording"), sv = tr("Start &VIDEO recording");
	bool audio = false, video = false;
	QString *sc;

	emit statusbar->rec->et_blink_icon();

	if (!(info.no_rom | info.turn_off | rwnd.active)) {
		if (info.recording_on_air) {
			if (recording_format_type() == REC_FORMAT_AUDIO) {
				audio = true;
				video = false;
				sa = tr("Stop &AUDIO recording");
				ia = QIcon(":/icon/icons/multimedia_stop.svgz");
			} else {
				audio = false;
				video = true;
				sv = tr("Stop &VIDEO recording");
				iv = QIcon(":/icon/icons/multimedia_stop.svgz");
			}
		} else {
			audio = true;
			video = true;
		}
	}

	sc = (QString *)settings_inp_rd_sc(SET_INP_SC_REC_AUDIO, KEYBOARD);
	action_Start_Stop_Audio_recording->setEnabled(audio);
	action_text(action_Start_Stop_Audio_recording, sa, sc);
	action_Start_Stop_Audio_recording->setIcon(ia);

	sc = (QString *)settings_inp_rd_sc(SET_INP_SC_REC_VIDEO, KEYBOARD);
	action_Start_Stop_Video_recording->setEnabled(video);
	action_text(action_Start_Stop_Video_recording, sv, sc);
	action_Start_Stop_Video_recording->setIcon(iv);
#else
	QIcon ia = QIcon(":/icon/icons/multimedia_record.svgz");
	QString sa = tr("Start &WAV recording");
	bool audio = false;
	QString *sc;

	emit statusbar->rec->et_blink_icon();

	if (!(info.no_rom | info.turn_off | rwnd.active)) {
		audio = true;
		if (info.recording_on_air) {
			sa = tr("Stop &WAV recording");
			ia = QIcon(":/icon/icons/multimedia_stop.svgz");
		}
	}

	sc = (QString *)settings_inp_rd_sc(SET_INP_SC_REC_AUDIO, KEYBOARD);
	action_Start_Stop_Audio_recording->setEnabled(audio);
	action_text(action_Start_Stop_Audio_recording, sa, sc);
	action_Start_Stop_Audio_recording->setIcon(ia);

	action_Start_Stop_Video_recording->setVisible(false);
#endif
}
void mainWindow::set_language(int lang) {
	QString lng = "en", file = "en_EN", dir = ":/tr/translations";

	if ((lang == cfg->language) && gui.start) {
		return;
	}

	qApp->removeTranslator(translator);

	// solo per testare le nuove traduzioni
	if (!gui.start) {
		QFile ext(uQString(gui_application_folder()) + "/test.qm");

		if (ext.exists()) {
			if (translator->load("test.qm", uQString(gui_application_folder()))) {
				qApp->installTranslator(translator);
			}
			return;
		}
	}

	switch (lang) {
		case LNG_ITALIAN:
			lng = "it";
			file = "it_IT";
			break;
		case LNG_HUNGARIAN:
			lng = "hu";
			file = "hu_HU";
			break;
		case LNG_PORTUGUESEBR:
			lng = "pt";
			file = "pt_BR";
			break;
		case LNG_RUSSIAN:
			lng = "ru";
			file = "ru_RU";
			break;
		case LNG_SPANISH:
			lng = "es";
			file = "es_ES";
			break;
		case LNG_TURKISH:
			lng = "en";
			file = "tr_TR";
			break;
		case LNG_CHINESE_SIMPLIFIED:
			lng = "zh_CN";
			file = "zh_CN";
			break;
		case LNG_ENGLISH:
		default:
			break;
	}

	QLocale locale = QLocale(lng);
	QLocale::setDefault(locale);

	if (qtTranslator->load("qt_" + lng, dir)) {
		qApp->installTranslator(qtTranslator);
	}
	if (translator->load(file, dir)) {
		qApp->installTranslator(translator);
	}

	cfg->language = lang;
}
void mainWindow::shcjoy_start(void) {
	shcjoy_stop();

	if (js_is_null(&cfg->input.jguid_sch)) {
		return;
	}

	for (int i = 0; i < SET_MAX_NUM_SC; i++) {
		shcjoy.shortcut[i] = js_joyval_from_name(uQStringCD(QString(*(QString *)settings_inp_rd_sc(i + SET_INP_SC_OPEN, JOYSTICK))));
	}

	js_init_shcut();

	shcjoy.enabled = true;
	shcjoy.timer->start(13);
}
void mainWindow::shcjoy_stop(void) {
	shcjoy.enabled = false;
	shcjoy.timer->stop();
}
void mainWindow::control_visible_cursor(void) {
	if (!nsf.enabled && !gmouse.hidden && !input_draw_target()) {
		if ((gui_get_ms() - gmouse.timer) >= 2000) {
			gui_cursor_hide(TRUE);
		}
	}
}
void mainWindow::make_reset(int type) {
	emu_thread_pause();

	if (type == HARD) {
		if ((cfg->cheat_mode == GAMEGENIE_MODE) && gamegenie.rom_present) {
			gamegenie_reset();
			type = CHANGE_ROM;
		} else {
			// se e' stato disabilitato il game genie quando ormai
			// e' gia' in esecuzione e si preme un reset, carico la rom.
			if (info.mapper.id == GAMEGENIE_MAPPER) {
				gamegenie_reset();
				type = CHANGE_ROM;
			}
		}
	}

	if (emu_reset(type)) {
		s_quit();
	}

	// nel caso il timer dell'update dello screen del fast forward (o del max speed) sia attivo, lo fermo.
	egds->stop_ff();
	egds->stop_max_speed();

	// resetto i contatori
	qaction_extern.max_speed.start->reset_count();
	qaction_extern.max_speed.stop->reset_count();

	// il reset manuale, quindi al di fuori del "filmato", lo fa ricominciare
	if (type <= HARD) {
		if (tas.type != NOTAS) {
			tas_restart_from_begin();
		}
	}

	emu_frame_input_and_rewind();

	update_menu_file();
	// dopo un reset la pause e' automaticamente disabilitata quindi faccio
	// un aggiornamento del submenu NES per avere la voce correttamente settata.
	update_menu_nes();

	emu_thread_continue();
}
void mainWindow::change_rom(const uTCHAR *rom) {
	if (info.turn_off) {
		s_turn_on_off();
	}
	emu_thread_pause();
	info.rom.from_load_menu = emu_ustrncpy(info.rom.from_load_menu, (uTCHAR *)rom);
	gamegenie_reset();
	gamegenie_free_paths();
	make_reset(CHANGE_ROM);
	gui_update();
	emu_thread_continue();
}
void mainWindow::shortcuts(void) {
	// se non voglio che gli shortcuts funzionino durante il fullscreen, basta
	// utilizzare lo shortcut associato al QAction. In questo modo quando nascondero'
	// la barra del menu, automaticamente questi saranno disabilitati.

	// File
	connect_shortcut(action_Open, SET_INP_SC_OPEN, SLOT(s_open()));
	connect_shortcut(action_Start_Stop_Audio_recording, SET_INP_SC_REC_AUDIO, SLOT(s_start_stop_audio_recording()));
#if defined (WITH_FFMPEG)
	connect_shortcut(action_Start_Stop_Video_recording, SET_INP_SC_REC_VIDEO, SLOT(s_start_stop_video_recording()));
#endif
	connect_shortcut(action_Quit, SET_INP_SC_QUIT, SLOT(s_quit()));
	// NES
	connect_shortcut(action_Turn_Off, SET_INP_SC_TURN_OFF, SLOT(s_turn_on_off()));
	connect_shortcut(action_Hard_Reset, SET_INP_SC_HARD_RESET, SLOT(s_make_reset()));
	connect_shortcut(action_Soft_Reset, SET_INP_SC_SOFT_RESET, SLOT(s_make_reset()));
	connect_shortcut(action_Insert_Coin, SET_INP_SC_INSERT_COIN, SLOT(s_insert_coin()));
	connect_shortcut(action_Shout_into_Microphone, SET_INP_SC_SHOUT_INTO_MIC, SLOT(s_fake_slot())); // shortcut speciale
	connect_shortcut(action_Switch_sides, SET_INP_SC_SWITCH_SIDES, SLOT(s_disk_side()));
	connect_shortcut(action_Eject_Insert_Disk, SET_INP_SC_EJECT_DISK, SLOT(s_eject_disk()));
	connect_shortcut(action_Fullscreen, SET_INP_SC_FULLSCREEN, SLOT(s_set_fullscreen()));
	connect_shortcut(action_Save_Screenshot, SET_INP_SC_SCREENSHOT, SLOT(s_save_screenshot()));
	connect_shortcut(action_Save_Unaltered_NES_screen, SET_INP_SC_SCREENSHOT_1X, SLOT(s_save_screenshot_1x()));
	connect_shortcut(action_Pause, SET_INP_SC_PAUSE, SLOT(s_pause()));
	connect_shortcut(action_Toogle_Fast_Forward, SET_INP_SC_TOGGLE_FAST_FORWARD, SLOT(s_fast_forward()));
	connect_shortcut(action_Toggle_GUI_in_window, SET_INP_SC_TOGGLE_GUI_IN_WINDOW, SLOT(s_toggle_gui_in_window()));
	// Settings/Mode
	connect_shortcut(qaction_shcut.mode_auto, SET_INP_SC_MODE_AUTO, SLOT(s_shcut_mode()));
	connect_shortcut(qaction_shcut.mode_ntsc, SET_INP_SC_MODE_NTSC, SLOT(s_shcut_mode()));
	connect_shortcut(qaction_shcut.mode_pal, SET_INP_SC_MODE_PAL, SLOT(s_shcut_mode()));
	connect_shortcut(qaction_shcut.mode_dendy, SET_INP_SC_MODE_DENDY, SLOT(s_shcut_mode()));
	// Settings/Video/Scale
	connect_shortcut(qaction_shcut.scale_1x, SET_INP_SC_SCALE_1X, SLOT(s_shcut_scale()));
	connect_shortcut(qaction_shcut.scale_2x, SET_INP_SC_SCALE_2X, SLOT(s_shcut_scale()));
	connect_shortcut(qaction_shcut.scale_3x, SET_INP_SC_SCALE_3X, SLOT(s_shcut_scale()));
	connect_shortcut(qaction_shcut.scale_4x, SET_INP_SC_SCALE_4X, SLOT(s_shcut_scale()));
	connect_shortcut(qaction_shcut.scale_5x, SET_INP_SC_SCALE_5X, SLOT(s_shcut_scale()));
	connect_shortcut(qaction_shcut.scale_6x, SET_INP_SC_SCALE_6X, SLOT(s_shcut_scale()));
	// Settings/Video/[Interpolation, Use integer scaling in fullscreen, Stretch in fullscreen]
	connect_shortcut(qaction_shcut.interpolation, SET_INP_SC_INTERPOLATION, SLOT(s_shcut_interpolation()));
	connect_shortcut(qaction_shcut.integer_in_fullscreen, SET_INP_SC_INTEGER_FULLSCREEN, SLOT(s_shcut_integer_in_fullscreen()));
	connect_shortcut(qaction_shcut.stretch_in_fullscreen, SET_INP_SC_STRETCH_FULLSCREEN, SLOT(s_shcut_stretch_in_fullscreen()));
	// Settings/Audio/Enable
	connect_shortcut(qaction_shcut.audio_enable, SET_INP_SC_AUDIO_ENABLE, SLOT(s_shcut_audio_enable()));
	// Settings/Save settings
	connect_shortcut(qaction_shcut.save_settings, SET_INP_SC_SAVE_SETTINGS, SLOT(s_shcut_save_settings()));
	// State/[Save state, Load state]
	connect_shortcut(action_Save_state, SET_INP_SC_SAVE_STATE, SLOT(s_state_save_slot_action()));
	connect_shortcut(action_Load_state, SET_INP_SC_LOAD_STATE, SLOT(s_state_save_slot_action()));
	// State/[Incremente slot, Decrement slot]
	connect_shortcut(action_Increment_slot, SET_INP_SC_INC_SLOT, SLOT(s_state_save_slot_incdec()));
	connect_shortcut(action_Decrement_slot, SET_INP_SC_DEC_SLOT, SLOT(s_state_save_slot_incdec()));

	// Rewind operations
	connect_shortcut(qaction_shcut.rwnd.active, SET_INP_SC_RWND_ACTIVE_MODE, SLOT(s_shcut_rwnd_active_deactive_mode()));
	connect_shortcut(qaction_shcut.rwnd.step_backward, SET_INP_SC_RWND_STEP_BACKWARD, SLOT(s_shcut_rwnd_step_backward()));
	connect_shortcut(qaction_shcut.rwnd.step_forward, SET_INP_SC_RWND_STEP_FORWARD, SLOT(s_shcut_rwnd_step_forward()));
	connect_shortcut(qaction_shcut.rwnd.fast_backward, SET_INP_SC_RWND_FAST_BACKWARD, SLOT(s_shcut_rwnd_fast_backward()));
	connect_shortcut(qaction_shcut.rwnd.fast_forward, SET_INP_SC_RWND_FAST_FORWARD, SLOT(s_shcut_rwnd_fast_forward()));
	connect_shortcut(qaction_shcut.rwnd.play, SET_INP_SC_RWND_PLAY, SLOT(s_shcut_rwnd_play()));
	connect_shortcut(qaction_shcut.rwnd.pause, SET_INP_SC_RWND_PAUSE, SLOT(s_shcut_rwnd_pause()));

	// Toggle Menubar
	connect_shortcut(qaction_shcut.toggle_menubar_in_fullscreen, SET_INP_SC_TOGGLE_MENUBAR_IN_FULLSCREEN, SLOT(s_shcut_toggle_menubar()));

	// Nes Keyboard
	connect_shortcut(qaction_shcut.toggle_capture_input, SET_INP_SC_TOGGLE_CAPTURE_INPUT, SLOT(s_shcut_toggle_capture_input()));
	connect_shortcut(action_Virtual_Keyboard, SET_INP_SC_TOGGLE_NES_KEYBOARD, SLOT(s_open_dkeyb()));

	// Hold Fast Forward
	connect_shortcut(qaction_shcut.hold_fast_forward, SET_INP_SC_HOLD_FAST_FORWARD, SLOT(s_fake_slot())); // shortcut speciale

	// aggiorno il tooltip del nesKeyboardStatusBar
	if (gui.start) {
		statusbar->update_statusbar();
	}
}
bool mainWindow::is_rwnd_shortcut_or_not_shcut(const QKeyEvent *event) {
	int shcut = is_shortcut(event);

	if ((shcut >= 0) && ((shcut < SET_INP_SC_RWND_STEP_BACKWARD) || (shcut > SET_INP_SC_RWND_PAUSE))) {
		return (false);
	}

	return (true);
}
void mainWindow::update_gfx_monitor_dimension(void) {
	if (gfx.type_of_fscreen_in_use == FULLSCR_IN_WINDOW) {
		bool toolbar_is_hidden = toolbar->isHidden() || toolbar->isFloating();

#if QT_VERSION == QT_VERSION_CHECK(5, 12, 8)
		gfx.w[MONITOR] = fs_geom.width() - (frameGeometry().width() - geometry().width());
		gfx.h[MONITOR] = fs_geom.height() - (frameGeometry().height() - geometry().height());
#else
		gfx.w[MONITOR] = fs_geom.width();
		gfx.h[MONITOR] = fs_geom.height();
#endif

		if (toolbar->orientation() == Qt::Vertical) {
			gfx.w[MONITOR] -= (toolbar_is_hidden ? 0 : toolbar->sizeHint().width());
		} else {
			gfx.h[MONITOR] -= (toolbar_is_hidden ? 0 : toolbar->sizeHint().height());
		}

		gfx.h[MONITOR] -= (menubar->isHidden() ? 0 : menubar->sizeHint().height());
		gfx.h[MONITOR] -= (statusbar->isHidden() ? 0 : statusbar->sizeHint().height());
	} else if (gfx.type_of_fscreen_in_use == FULLSCR) {
		fs_geom = win_handle_screen()->geometry();
#if defined (FULLSCREEN_RESFREQ)
		if (setup_in_out_fullscreen) {
			int w, h, x, y;

			if (gfx_monitor_mode_in_use_info(&x, &y, &w, &h, nullptr) == EXIT_OK) {
				fs_geom = QRect(x, y, w, h);
			}
		}
#endif
		gfx.w[MONITOR] = fs_geom.width();
		gfx.h[MONITOR] = fs_geom.height();

		{
			qreal dpr = win_handle_screen()->devicePixelRatio();

			gfx.w[MONITOR] = (SDBWORD)((qreal)gfx.w[MONITOR] / dpr);
			gfx.h[MONITOR] = (SDBWORD)((qreal)gfx.h[MONITOR] / dpr);
		}
	}
}
QAction *mainWindow::state_save_slot_action(BYTE slot) {
	return (findChild<QAction *>(QString("action_State_Slot_%1").arg(QString::number(slot, 16).toUpper())));
}
void mainWindow::state_save_slot_set(int slot, bool on_video) {
	if (info.no_rom | info.turn_off) {
		return;
	}
	save_slot.slot = slot;
	if (on_video) {
		gui_overlay_enable_save_slot(SAVE_SLOT_INCDEC);
	}
	update_window();
}
void mainWindow::state_save_slot_set_tooltip(BYTE slot, char *buffer) {
	QString tooltip;

	if (buffer) {
		static QPainter painter;
		static QFont f;
		static QRect rect;
		static QPen pen;
		QImage img = QImage((uchar *)buffer, SCR_COLUMNS, SCR_ROWS, SCR_COLUMNS * sizeof(uint32_t), QImage::Format_RGB32);
		QByteArray data;
		QBuffer png(&data);
		int x, y, w, h;
		double mul = 1.5f;

		img = img.scaled((int)(SCR_COLUMNS * mul), (int)(SCR_ROWS * mul), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		// scrivo le info
		w = img.rect().width();
		h = wdgoverlayui->overlaySaveSlot->height_row_slot * mul;
		x = 0;
		y = 0;

		rect.setRect(x, y, w, h);

		f = font();
		f.setPixelSize((int)(h / mul));

		painter.begin(&img);
		painter.setOpacity(0.5);
		painter.fillRect(rect, Qt::darkBlue);

		pen.setWidth(1);

		painter.setFont(f);
		painter.setOpacity(1.0);

		pen.setColor(wdgoverlayui->overlaySaveSlot->color.slot);
		painter.setPen(pen);
		painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, QString(" %0").arg(slot, 1, 16).toUpper());

		pen.setColor(wdgoverlayui->overlaySaveSlot->color.info);
		painter.setPen(pen);
		painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, wdgoverlayui->overlaySaveSlot->date_and_time(slot));

		painter.end();

		img.save(&png, "PNG", 10);
		tooltip = QString("<img src='data:image/png;base64, %1'>").arg(QString(data.toBase64()));
	} else {
		//: Refers to the unused save slot. Important: Do not translate the "%1".
		tooltip = tr("Slot %1 never used").arg(QString::number(slot, 16).toUpper());
	}

	state_save_slot_action(slot)->setToolTip(tooltip);
}
void mainWindow::toggle_toolbars(void) {
	bool visibility = !toolbar->isVisible();

	emu_thread_pause();

	toolbar->setVisible(visibility);
	statusbar->setVisible(visibility);
	update_gfx_monitor_dimension();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);

	emu_thread_continue();
}
void mainWindow::reset_min_max_size(void) {
#if defined (_WIN32)
	return;
#else
	// su alcuni desktop environment, il fullscreen non avviene perche'
	// la dimensione della finestra e' fissa e le qt non riescono
	// a sbloccarla.
	setMinimumSize(QSize(0, 0));
	setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
	toolbar->setMinimumSize(QSize(0, 0));
	toolbar->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
	statusbar->setMinimumSize(QSize(0, 0));
	statusbar->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
#endif
}
QScreen *mainWindow::win_handle_screen(void) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
	QScreen *screen = QGuiApplication::screens().at(qApp->desktop()->screenNumber(this));
#else
	QScreen *screen = windowHandle()->screen();
#endif

	return (screen);
}
void mainWindow::shout_into_mic(BYTE mode) {
	if ((tas.type == NOTAS) && !rwnd.active) {
		if (mode) {
			mic.mode = MIC_RESET;
			mic.enable = TRUE;
		}
	}
}
void mainWindow::hold_fast_forward(BYTE mode) {
	if (fps.fast_forward != mode) {
		s_fast_forward();
	}
}
void mainWindow::open_dkeyb(BYTE mode) {
	set_dialog_geom(dlgkeyb->geom);
	dlgkeyb->setGeometry(dlgkeyb->geom);
	dlgkeyb->switch_mode(mode);
	dlgkeyb->show();
}

void mainWindow::connect_menu_signals(void) {
	// File
	connect_action(action_Open, SLOT(s_open()));
	connect_action(action_Apply_Patch, SLOT(s_apply_patch()));
	connect_action(action_Edit_Current_Header, SLOT(s_open_edit_current_header()));
	connect_action(action_Start_Stop_Audio_recording, SLOT(s_start_stop_audio_recording()));
#if defined (WITH_FFMPEG)
	connect_action(action_Start_Stop_Video_recording, SLOT(s_start_stop_video_recording()));
#endif
	connect_action(action_Open_config_folder, SLOT(s_open_config_folder()));
	connect_action(action_Open_working_folder, SLOT(s_open_working_folder()));
	connect_action(action_Quit, SLOT(s_quit()));
	// NES
	connect_action(action_Turn_Off, SLOT(s_turn_on_off()));
	connect_action(action_Hard_Reset, HARD, SLOT(s_make_reset()));
	connect_action(action_Soft_Reset, RESET, SLOT(s_make_reset()));
	connect_action(action_Insert_Coin, SLOT(s_insert_coin()));
	connect_action(action_Shout_into_Microphone, SLOT(s_shout_into_mic()));
	connect_action(action_Disk_1_side_A, 0, SLOT(s_disk_side()));
	connect_action(action_Disk_1_side_B, 1, SLOT(s_disk_side()));
	connect_action(action_Disk_2_side_A, 2, SLOT(s_disk_side()));
	connect_action(action_Disk_2_side_B, 3, SLOT(s_disk_side()));
	connect_action(action_Disk_3_side_A, 4, SLOT(s_disk_side()));
	connect_action(action_Disk_3_side_B, 5, SLOT(s_disk_side()));
	connect_action(action_Disk_4_side_A, 6, SLOT(s_disk_side()));
	connect_action(action_Disk_4_side_B, 7, SLOT(s_disk_side()));
	connect_action(action_Switch_sides, 0xFFF, SLOT(s_disk_side()));
	connect_action(action_Eject_Insert_Disk, SLOT(s_eject_disk()));
	connect_action(action_Tape_Play, SLOT(s_tape_play()));
	connect_action(action_Tape_Record, SLOT(s_tape_record()));
	connect_action(action_Tape_Stop, SLOT(s_tape_stop()));
	connect_action(action_Fullscreen, SLOT(s_set_fullscreen()));
	connect_action(action_Save_Screenshot, SLOT(s_save_screenshot()));
	connect_action(action_Save_Unaltered_NES_screen, SLOT(s_save_screenshot_1x()));
	connect_action(action_Pause, SLOT(s_pause()));
	connect_action(action_Toogle_Fast_Forward, SLOT(s_fast_forward()));
	connect_action(action_Toggle_GUI_in_window, SLOT(s_toggle_gui_in_window()));

	// Settings
	connect_action(action_General, 0, SLOT(s_open_settings()));
	connect_action(action_Video, 1, SLOT(s_open_settings()));
	connect_action(action_Audio, 2, SLOT(s_open_settings()));
	connect_action(action_Input, 3, SLOT(s_open_settings()));
	connect_action(action_PPU, 4, SLOT(s_open_settings()));
	connect_action(action_Cheats, 5, SLOT(s_open_settings()));
	connect_action(action_Recording, 6, SLOT(s_open_settings()));

	// State/[Save state, Load State]
	connect_action(action_Save_state, SAVE, SLOT(s_state_save_slot_action()));
	connect_action(action_Load_state, LOAD, SLOT(s_state_save_slot_action()));
	// State/[Increment slot, Decrement slot]
	connect_action(action_Increment_slot, INC, SLOT(s_state_save_slot_incdec()));
	connect_action(action_Decrement_slot, DEC, SLOT(s_state_save_slot_incdec()));
	// State/[State slot 0....]
	connect_action(action_State_Slot_0, 0, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_1, 1, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_2, 2, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_3, 3, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_4, 4, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_5, 5, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_6, 6, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_7, 7, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_8, 8, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_9, 9, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_A, 10, SLOT(s_state_save_slot_set()));
	connect_action(action_State_Slot_B, 11, SLOT(s_state_save_slot_set()));
	// State/[Save to file, Load from file]
	connect_action(action_State_Save_to_file, SLOT(s_state_save_file()));
	connect_action(action_State_Load_from_file, SLOT(s_state_load_file()));
	// Tools
	connect_action(action_Joypad_Gamepads_Debug, SLOT(s_open_djsc()));
	connect_action(action_Virtual_Keyboard, SLOT(s_open_dkeyb()));
	connect_action(action_Vs_System, SLOT(s_set_vs_window()));
	// Help/About
	connect_action(action_Show_Log, SLOT(s_show_log()));
	connect_action(action_About, SLOT(s_help()));

	// external shortcuts
	connect_action(qaction_shcut.mode_auto, AUTO, SLOT(s_shcut_mode()));
	connect_action(qaction_shcut.mode_ntsc, NTSC, SLOT(s_shcut_mode()));
	connect_action(qaction_shcut.mode_pal, PAL, SLOT(s_shcut_mode()));
	connect_action(qaction_shcut.mode_dendy, DENDY, SLOT(s_shcut_mode()));
	connect_action(qaction_shcut.scale_1x, 0, SLOT(s_shcut_scale()));
	connect_action(qaction_shcut.scale_2x, 1, SLOT(s_shcut_scale()));
	connect_action(qaction_shcut.scale_3x, 2, SLOT(s_shcut_scale()));
	connect_action(qaction_shcut.scale_4x, 3, SLOT(s_shcut_scale()));
	connect_action(qaction_shcut.scale_5x, 4, SLOT(s_shcut_scale()));
	connect_action(qaction_shcut.scale_6x, 5, SLOT(s_shcut_scale()));
	connect_action(qaction_shcut.interpolation, SLOT(s_shcut_interpolation()));
	connect_action(qaction_shcut.integer_in_fullscreen, SLOT(s_shcut_integer_in_fullscreen()));
	connect_action(qaction_shcut.stretch_in_fullscreen, SLOT(s_shcut_stretch_in_fullscreen()));
	connect_action(qaction_shcut.toggle_menubar_in_fullscreen, SLOT(s_shcut_toggle_menubar()));
	connect_action(qaction_shcut.toggle_capture_input, SLOT(s_shcut_toggle_capture_input()));
	connect_action(qaction_shcut.audio_enable, SLOT(s_shcut_audio_enable()));
	connect_action(qaction_shcut.save_settings, SLOT(s_shcut_save_settings()));
	connect_action(qaction_shcut.rwnd.active, SLOT(s_shcut_rwnd_active_deactive_mode()));
	connect_action(qaction_shcut.rwnd.step_backward, SLOT(s_shcut_rwnd_step_backward()));
	connect_action(qaction_shcut.rwnd.step_forward, SLOT(s_shcut_rwnd_step_forward()));
	connect_action(qaction_shcut.rwnd.fast_backward, SLOT(s_shcut_rwnd_fast_backward()));
	connect_action(qaction_shcut.rwnd.fast_forward, SLOT(s_shcut_rwnd_fast_forward()));
	connect_action(qaction_shcut.rwnd.play, SLOT(s_shcut_rwnd_play()));
	connect_action(qaction_shcut.rwnd.pause, SLOT(s_shcut_rwnd_pause()));

	// external
	connect_action(qaction_extern.max_speed.start, SLOT(s_max_speed_start()));
	connect_action(qaction_extern.max_speed.stop, SLOT(s_max_speed_stop()));
}
void mainWindow::connect_action(QAction *action, const char *member) {
	connect(action, SIGNAL(triggered()), this, member);
}
void mainWindow::connect_action(QAction *action, int value, const char *member) {
	action->setProperty("myValue", QVariant(value));
	connect_action(action, member);
}
void mainWindow::connect_shortcut(QAction *action, int index) {
	QString *sc = (QString *)settings_inp_rd_sc(index, KEYBOARD);

	if (!sc->isEmpty()) {
		QStringList text = action->text().split('\t');

		action->setShortcut(sc->compare("NULL") ? QKeySequence((*sc)) : 0);
		action_text(action, text.at(0), sc);
	}
}
void mainWindow::connect_shortcut(QAction *action, int index, const char *member) {
	QString *sc = (QString *)settings_inp_rd_sc(index, KEYBOARD);

	if (!sc->isEmpty()) {
		QStringList text = action->text().split('\t');
		QVariant value = action->property("myValue");

		shortcut[index]->setKey(sc->compare("NULL") ? QKeySequence((*sc)) : 0);
		if (!value.isNull()) {
			shortcut[index]->setProperty("myValue", value);
		}
		// disconnetto il vecchio (se presente)
		disconnect(shortcut[index], SIGNAL(activated()), this, member);
		// connetto il nuovo
		connect(shortcut[index], SIGNAL(activated()), this, member);

		action_text(action, text.at(0), sc);
	}
}

void mainWindow::update_menu_file(void) {
	action_Apply_Patch->setEnabled(!info.no_rom);

	action_Edit_Current_Header->setEnabled((info.format == iNES_1_0) || (info.format == NES_2_0));
	if (!action_Edit_Current_Header->isEnabled() && dlgheader->isVisible()) {
		dlgheader->hide();
	}

	update_recording_widgets();

	// recent roms
	if (recent_roms_count() > 0) {
		int i;

		foreach (QAction *action, menu_Recent_Roms->actions()) {
			delete (action);
		}

		menu_Recent_Roms->clear();

		for (i = 0; i < RECENT_ROMS_MAX; i++) {
			QString description = QString((const QChar *)recent_roms_item(i), recent_roms_item_size(i));
			QFileInfo rom(description);
			QAction *action;

			if (description.isEmpty()) {
				break;
			}

			action = new QAction(this);
			action->setText(QFileInfo(description).fileName());

			if (rom.suffix().isEmpty() ||
				!rom.suffix().compare("nes", Qt::CaseInsensitive) ||
				!rom.suffix().compare("unf", Qt::CaseInsensitive) ||
				!rom.suffix().compare("unif", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/nes_file.svgz"));
			} else if (!rom.suffix().compare("nsf", Qt::CaseInsensitive) ||
				!rom.suffix().compare("nsfe", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/nsf_file.svgz"));
			} else if (!rom.suffix().compare("fds", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/fds_file.svgz"));
			} else if (!rom.suffix().compare("fm2", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/fm2_file.svgz"));
			} else {
				action->setIcon(QIcon(":/icon/icons/compressed_file.svgz"));
			}

			action->setProperty("myValue", QVariant(i));
			menu_Recent_Roms->addAction(action);
			connect(action, SIGNAL(triggered()), this, SLOT(s_open_recent_roms()));
		}
	}
}
void mainWindow::update_menu_nes(void) {
	QString *sc = (QString *)settings_inp_rd_sc(SET_INP_SC_TURN_OFF, KEYBOARD);

	if (info.turn_off) {
		action_text(action_Turn_Off, tr("&Turn On"), sc);
		action_Turn_Off->setIcon(QIcon(":/icon/icons/turn_on.svgz"));
	} else {
		action_text(action_Turn_Off, tr("&Turn Off"), sc);
		action_Turn_Off->setIcon(QIcon(":/icon/icons/turn_off.svgz"));
	}

	if (info.no_rom | rwnd.active) {
		action_Turn_Off->setEnabled(false);
	} else {
		action_Turn_Off->setEnabled(true);
	}

	if (info.no_rom | info.turn_off | rwnd.active) {
		action_Hard_Reset->setEnabled(false);
		action_Soft_Reset->setEnabled(false);
		action_Shout_into_Microphone->setEnabled(false);
	} else {
		action_Hard_Reset->setEnabled(true);
		action_Soft_Reset->setEnabled(true);
		action_Shout_into_Microphone->setEnabled(cfg->input.controller_mode == CTRL_MODE_FAMICOM);
	}

	if (vs_system.enabled && !rwnd.active) {
		action_Insert_Coin->setEnabled(true);
	} else {
		action_Insert_Coin->setEnabled(false);
	}

	update_fds_menu();
	update_tape_menu();

	if (info.pause_from_gui && !rwnd.active) {
		action_Pause->setChecked(true);
	} else {
		action_Pause->setChecked(false);
	}

	if (!nsf.enabled && !rwnd.active) {
		action_Toogle_Fast_Forward->setEnabled(true);

		if (fps.fast_forward) {
			action_Toogle_Fast_Forward->setChecked(true);
		} else {
			action_Toogle_Fast_Forward->setChecked(false);
		}
	} else {
		action_Toogle_Fast_Forward->setEnabled(false);
	}
}
void mainWindow::update_menu_state(void) {
	bool state = false;

	if (!(info.no_rom | info.turn_off)) {
		state = true;
	}

	action_Save_state->setEnabled(state);
	action_Load_state->setEnabled(state && (tas.type == NOTAS) && save_slot.state[save_slot.slot]);
	action_Increment_slot->setEnabled(state);
	action_Decrement_slot->setEnabled(state);

	for (unsigned int i = 0; i < SAVE_SLOTS; i++) {
		QAction *a = state_save_slot_action(i);
		QString used = " *";
		QString txt = a->text().replace(used, "");

		if (i == save_slot.slot) {
			a->setChecked(true);
		}

		if (save_slot.state[i]) {
			a->setText(txt + used);
		} else {
			a->setText(txt);
		}

		a->setEnabled(state);
	}

	action_State_Save_to_file->setEnabled(state);
	action_State_Load_from_file->setEnabled(state && (tas.type == NOTAS));
}

void mainWindow::update_fds_menu(void) {
	QString *sc = (QString *)settings_inp_rd_sc(SET_INP_SC_EJECT_DISK, KEYBOARD);

	if (fds.info.enabled && !rwnd.active) {
		if (fds.drive.disk_ejected) {
			action_text(action_Eject_Insert_Disk, tr("&Insert disk"), sc);
		} else {
			action_text(action_Eject_Insert_Disk, tr("&Eject disk"), sc);
		}

		menu_Disk_Side->setEnabled(true);
		ctrl_disk_side(action_Disk_1_side_A);
		ctrl_disk_side(action_Disk_1_side_B);
		ctrl_disk_side(action_Disk_2_side_A);
		ctrl_disk_side(action_Disk_2_side_B);
		ctrl_disk_side(action_Disk_3_side_A);
		ctrl_disk_side(action_Disk_3_side_B);
		ctrl_disk_side(action_Disk_4_side_A);
		ctrl_disk_side(action_Disk_4_side_B);
		action_Eject_Insert_Disk->setEnabled(true);
	} else {
		action_text(action_Eject_Insert_Disk, tr("&Eject/Insert disk"), sc);
		menu_Disk_Side->setEnabled(false);
		action_Eject_Insert_Disk->setEnabled(false);
	}
}
void mainWindow::update_tape_menu(void) {
	menu_Tape->setEnabled(!info.no_rom && tape_data_recorder.enabled);

	switch (tape_data_recorder.mode) {
		case TAPE_DATA_NONE:
			action_Tape_Play->setEnabled(!dlgkeyb->paste->enable);
			action_Tape_Record->setEnabled(!dlgkeyb->paste->enable);
			action_Tape_Stop->setEnabled(false);
			break;
		case TAPE_DATA_PLAY:
		case TAPE_DATA_RECORD:
			action_Tape_Play->setEnabled(false);
			action_Tape_Record->setEnabled(false);
			action_Tape_Stop->setEnabled(!dlgkeyb->paste->enable);
			break;
		default:
			action_Tape_Play->setEnabled(false);
			action_Tape_Record->setEnabled(false);
			action_Tape_Stop->setEnabled(false);
			break;
	}
}
void mainWindow::update_menu_tools(void) {
	action_Virtual_Keyboard->setEnabled(nes_keyboard.enabled);
	if (!action_Virtual_Keyboard->isEnabled() && dlgkeyb->isVisible()) {
		dlgkeyb->hide();
	}
}

void mainWindow::action_text(QAction *action, const QString &description, QString *shortcut) {
	if ((*shortcut) == "NULL") {
		action->setText(description);
	} else {
		action->setText(description + '\t' + (*shortcut));
	}
}
void mainWindow::ctrl_disk_side(QAction *action) {
	int side = QVariant(action->property("myValue")).toInt();

	if (side < fds.info.total_sides) {
		action->setEnabled(true);
	} else {
		action->setEnabled(false);
	}

	if (fds.side.change.new_side == 0xFF) {
		if (side == fds.drive.side_inserted) {
			action->setChecked(true);
		}
	} else if (side == fds.side.change.new_side) {
		action->setChecked(true);
	}
}
void mainWindow::geom_to_cfg(const QRect &geom, _last_geometry *lg) {
	lg->x = geom.x();
	lg->y = geom.y();
	lg->w = geom.width();
	lg->h = geom.height();
}
void mainWindow::set_dialog_geom(QRect &geom) {
	int frame_w = frameGeometry().width() - geometry().width();
	int frame_h = frameGeometry().height() - geometry().height();

	if (geom.x() < frame_w) {
		geom.setX(frame_w);
	}
	if (geom.y() < frame_h) {
		geom.setY(frame_h);
	}
}
int mainWindow::is_shortcut(const QKeyEvent *event) {
	int i;

	for (i = 0; i < SET_MAX_NUM_SC; i++) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		if ((unsigned int)shortcut[i]->key()[0] == (event->key() | event->modifiers())) {
#else
			if (shortcut[i]->key()[0] == event->keyCombination()) {
#endif
			return (i);
		}
	}

	return (-1);
}

void mainWindow::s_set_fullscreen(void) {
	BYTE delay = FALSE;

	if (gui.in_update || setup_in_out_fullscreen) {
		return;
	}

	// quando nascondo la finestra al momento dell'attivazione/disattivazione del
	// fullscreen vengono eseguiti nell'ordine un
	// QEvent::WindowActivate
	// seguito da un
	// QEvent::WindowDeactivate
	// che lascia la finestra disattivata. In caso di "Pause when in background" attivo
	// il gui_control_pause_bck non riprende l'emulazione pensando appunto di essere in background.
	setup_in_out_fullscreen = true;

	emu_thread_pause();

	if ((cfg->fullscreen == NO_FULLSCR) || (cfg->fullscreen == NO_CHANGE)) {
		gfx.scale_before_fscreen = cfg->scale;
		org_geom = geometry();
		visibility.menubar = menubar->isVisible();
		visibility.toolbars = toolbar->isVisible();
		if (!gfx.is_wayland) {
			// muovo la finestra nell'angolo superiore del monitor, e' importante
			// perche' in caso di cambio di risoluzione nell fullscreen, se posizionata
			// nella parte destra del monitor potrebbe non essere visualizzata correttamente.
			// E' importante che lo spostamento avvenga prima dell'hide().
			if (!cfg->fullscreen_in_window) {
				QRect mgeom = win_handle_screen()->geometry();

				move(mgeom.x() - (geometry().x() - x()), mgeom.y() - (geometry().y() - y()));
			}
			hide();
#if defined (FULLSCREEN_RESFREQ)
			if (!cfg->fullscreen_in_window) {
				delay = gfx_monitor_set_res(cfg->fullscreen_res_w, cfg->fullscreen_res_h, cfg->adaptive_rrate, FALSE);
			}
#endif
		}
#if defined (FULLSCREEN_RESFREQ)
	} else {
		// su Fedora 35 (Wayland, Gnome 41.5 e QT 5.15.2) il Fullscreen non funziona e
		// quello a finestra funziona solo se non eseguo l'hide().
		if (!gfx.is_wayland) {
			hide();
		}
		if (gfx.type_of_fscreen_in_use == FULLSCR) {
			delay = gfx_monitor_restore_res();
		}
#endif
	}

	if (delay) {
		QTimer::singleShot(1000, this, [this]() { s_fullscreen(); });
	} else {
		s_fullscreen();
	}
}
void mainWindow::s_set_vs_window(void) {
	ext_win.vs_system = !ext_win.vs_system;
	gui_external_control_windows_show();
}
void mainWindow::s_open_dkeyb(void) {
	if (nes_keyboard.enabled) {
		if (dlgkeyb->isHidden()) {
			open_dkeyb(dlgKeyboard::DK_VIRTUAL);
		} else {
			dlgkeyb->hide();
		}
	}
}

void mainWindow::s_fake_slot(void) {}
void mainWindow::s_open(void) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("All supported formats"));
	filters.append(tr("Compressed files"));
	filters.append(tr("Nes rom files"));
	filters.append(tr("UNIF rom files"));
	filters.append(tr("FDS image files"));
	filters.append(tr("NSF rom files"));
	filters.append(tr("NSFE rom files"));
	filters.append(tr("TAS movie files"));
	filters.append(tr("All files"));

	if (l7z_present()) {
		if ((l7z_control_ext(uL(".rar")) == EXIT_OK)) {
			filters[0].append(" (*.zip *.ZIP *.7z *.7Z *.rar *.RAR *.nes *.NES *.unf *.UNF *.unif *.UNIF *.nsf *.NSF *.nsfe *.NSFE *.fds *.FDS *.fm2 *.FM2)");
			filters[1].append(" (*.zip *.ZIP *.7z *.7Z *.rar *.RAR)");
		} else {
			filters[0].append(" (*.zip *.ZIP *.7z *.7Z *.nes *.NES *.fds *.FDS *.nsf *.NSF *.nsfe *.NSFE *.fm2 *.FM2)");
			filters[1].append(" (*.zip *.ZIP *.7z *.7Z)");
		}
	} else {
		filters[0].append(" (*.zip *.ZIP *.nes *.NES *.unf *.UNF *.unif *.UNIF *.fds *.FDS *.nsf *.NSF *.nsfe *.NSFE *.fm2 *.FM2)");
		filters[1].append(" (*.zip *.ZIP)");
	}

	filters[2].append(" (*.nes *.NES)");
	filters[3].append(" (*.unf *.UNF *.unif *.UNIF)");
	filters[4].append(" (*.fds *.FDS)");
	filters[5].append(" (*.nsf *.NSF)");
	filters[6].append(" (*.nsfe *.NSFE)");
	filters[7].append(" (*.fm2 *.FM2)");
	filters[8].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open File"), uQString(gui.last_open_path), filters.join(";;"));

	if (!file.isNull()) {
		QFileInfo fileinfo(file);

		change_rom(uQStringCD(fileinfo.absoluteFilePath()));
		ustrncpy(gui.last_open_path, uQStringCD(fileinfo.absolutePath()), usizeof(gui.last_open_path) - 1);
	}

	emu_pause(FALSE);
}
void mainWindow::s_apply_patch(void) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("All supported formats"));
	filters.append(tr("Compressed files"));
	filters.append(tr("IPS patch files"));
	filters.append(tr("BPS patch files"));
	filters.append(tr("XDELTA patch files"));
	filters.append(tr("All files"));

	if (l7z_present()) {
		if ((l7z_control_ext(uL(".rar")) == EXIT_OK)) {
			filters[0].append(" (*.zip *.ZIP *.7z *.7Z *.rar *.RAR *.ips *.IPS *.bps *.BPS *.xdelta *.XDELTA)");
			filters[1].append(" (*.zip *.ZIP *.7z *.7Z *.rar *.RAR)");
		} else {
			filters[0].append(" (*.zip *.ZIP *.7z *.7Z *.ips *.IPS *.bps *.BPS *.xdelta *.XDELTA)");
			filters[1].append(" (*.zip *.ZIP *.7z *.7Z)");
		}
	} else {
		filters[0].append(" (*.zip *.ZIP *.ips *.IPS *.bps *.BPS *.xdelta *.XDELTA)");
		filters[1].append(" (*.zip *.ZIP)");
	}

	filters[2].append(" (*.ips *.IPS)");
	filters[3].append(" (*.bps *.BPS)");
	filters[4].append(" (*.xdelta *.XDELTA)");
	filters[5].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open IPS/BPS/XDELTA Patch"), uQString(gui.last_open_patch_path),
		filters.join(";;"));

	if (!file.isNull()) {
		QFileInfo fileinfo(file);

		patcher_ctrl_if_exist(uQStringCD(fileinfo.absoluteFilePath()));

		if (patcher.file) {
			if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase == GG_EXECUTE)) {
				change_rom(gamegenie.rom);
			} else {
				change_rom(info.rom.file);
			}
			ustrncpy(gui.last_open_patch_path, uQStringCD(fileinfo.absolutePath()), usizeof(gui.last_open_patch_path) - 1);
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_open_edit_current_header(void) {
	set_dialog_geom(dlgheader->geom);
	dlgheader->setGeometry(dlgheader->geom);
	dlgheader->reset_dialog();
	dlgheader->show();
}
void mainWindow::s_open_recent_roms(void) {
	int index = QVariant(((QObject *)sender())->property("myValue")).toInt();
	QString current = QString((const QChar *)recent_roms_current(), recent_roms_current_size());
	QString item = QString((const QChar *)recent_roms_item(index), recent_roms_item_size(index));

	emu_pause(TRUE);

	if (current != item) {
		change_rom(uQStringCD(item));
	} else {
		// se l'archivio e' compresso e contiene piu' di una rom allora lo carico
		_uncompress_archive *archive;
		BYTE rc;

		archive = uncompress_archive_alloc(uQStringCD(item), &rc);

		if (rc == UNCOMPRESS_EXIT_OK) {
			if (archive->rom.count > 1) {
				change_rom(uQStringCD(item));
			}
			uncompress_archive_free(archive);
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_open_config_folder(void) {
	QDesktopServices::openUrl(QUrl(QDir(uQString(gui_config_folder())).absolutePath()));
}
void mainWindow::s_open_working_folder(void) {
	QDesktopServices::openUrl(QUrl(QDir(uQString(gui_data_folder())).absolutePath()));
}
void mainWindow::s_quit(void) {
	close();
}
void mainWindow::s_turn_on_off(void) {
	emu_thread_pause();

	info.turn_off = !info.turn_off;

	if (info.turn_off) {
		egds->start_turn_off();
		gui_overlay_update();
	} else {
		make_reset(HARD);
		egds->stop_turn_off();
	}

	emu_ctrl_doublebuffer();

	update_menu_nes();
	update_menu_state();

	emu_thread_continue();
}
void mainWindow::s_make_reset(void) {
	int type = QVariant(((QObject *)sender())->property("myValue")).toInt();

	make_reset(type);
}
void mainWindow::s_insert_coin(void) {
	gui_vs_system_insert_coin();
}
void mainWindow::s_shout_into_mic(void) {
	shout_into_mic(PRESSED);
}
void mainWindow::s_disk_side(void) {
	int side = QVariant(((QObject *)sender())->property("myValue")).toInt();

	emu_thread_pause();

	if (side == 0xFFF) {
		side = fds.drive.side_inserted ^ 0x01;
		if (side >= fds.info.total_sides) {
			emu_thread_continue();
			return;
		}
	}

	if (fds.drive.side_inserted == side) {
		emu_thread_continue();
		return;
	}

	if (fds.drive.disk_ejected) {
		fds.side.change.new_side = 0xFF;
		fds.side.change.delay = 0;
		fds_disk_op(FDS_DISK_SELECT, side, FALSE);
	} else {
		fds.side.change.new_side = side;
		fds.side.change.delay = FDS_OP_SIDE_DELAY;
		fds_disk_op(FDS_DISK_EJECT, 0, FALSE);
	}

	emu_thread_continue();

	update_menu_nes();
}
void mainWindow::s_eject_disk(void) {
	emu_thread_pause();
	if (!fds.drive.disk_ejected) {
		fds_disk_op(FDS_DISK_EJECT, 0, FALSE);
	} else {
		fds_disk_op(FDS_DISK_INSERT, 0, FALSE);
	}
	emu_thread_continue();
	update_menu_nes();
}
void mainWindow::s_start_stop_audio_recording(void) {
	if (info.no_rom) {
		return;
	}

#if defined (WITH_FFMPEG)
	if (!info.recording_on_air) {
		wdgRecGetSaveFileName *fd = new wdgRecGetSaveFileName(this);
		QString file;

		emu_pause(TRUE);

		file = fd->audio_get_save_file_name();

		if (!file.isNull()) {
			QFileInfo fileinfo = QFileInfo(file);

			umemset(cfg->last_rec_audio_path, 0x00, usizeof(cfg->last_rec_audio_path));
			ustrncpy(cfg->last_rec_audio_path, uQStringCD(fileinfo.absolutePath()), usizeof(cfg->last_rec_audio_path) - 1);
			recording_start(uQStringCD(file), cfg->recording.audio_format);
		}

		emu_pause(FALSE);
	} else if (recording_format_type() == REC_FORMAT_AUDIO) {
		recording_finish(FALSE);
	}
#else
	if (!info.recording_on_air) {
		QStringList filters;
		QString file, dir;
		QFileInfo rom = QFileInfo(uQString(info.rom.file));

		emu_pause(TRUE);

		filters.append(tr("MS WAVE files"));
		filters.append(tr("All files"));

		filters[0].append(" (*.wav *.WAV)");
		filters[1].append(" (*.*)");

		if (ustrlen(cfg->last_rec_audio_path) == 0) {
			dir = rom.dir().absolutePath();
		} else {
			dir = uQString(cfg->last_rec_audio_path);
		}

		file = QFileDialog::getSaveFileName(this, tr("Record sound"), dir + "/" + rom.completeBaseName(), filters.join(";;"));

		if (!file.isNull()) {
			QFileInfo fileinfo(file);

			if (fileinfo.suffix().isEmpty()) {
				fileinfo.setFile(QString(file) + ".wav");
			}

			umemset(cfg->last_rec_audio_path, 0x00, usizeof(cfg->last_rec_audio_path));
			ustrncpy(cfg->last_rec_audio_path, uQStringCD(fileinfo.absolutePath()), usizeof(cfg->last_rec_audio_path) - 1);
			wav_from_audio_emulator_open(uQStringCD(fileinfo.absoluteFilePath()), snd.samplerate * 5);
		}

		emu_pause(FALSE);
	} else {
		wav_from_audio_emulator_close();
	}
	update_menu_file();
#endif
}
#if defined (WITH_FFMPEG)
void mainWindow::s_start_stop_video_recording(void) {
	if (info.no_rom) {
		return;
	}

	if (!info.recording_on_air) {
		wdgRecGetSaveFileName *fd = new wdgRecGetSaveFileName(this);
		QString file;

		emu_pause(TRUE);

		file = fd->video_get_save_file_name();

		if (!file.isNull()) {
			QFileInfo fileinfo = QFileInfo(file);

			umemset(cfg->last_rec_video_path, 0x00, usizeof(cfg->last_rec_video_path));
			ustrncpy(cfg->last_rec_video_path, uQStringCD(fileinfo.absolutePath()), usizeof(cfg->last_rec_video_path) - 1);
			recording_start(uQStringCD(file), cfg->recording.video_format);
		}

		emu_pause(FALSE);
	} else if (recording_format_type() == REC_FORMAT_VIDEO) {
		recording_finish(FALSE);
	}
}
#endif
void mainWindow::s_save_screenshot(void) {
	info.screenshot = SCRSH_STANDARD;
}
void mainWindow::s_save_screenshot_1x(void) {
	info.screenshot = SCRSH_ORIGINAL_SIZE;
}
void mainWindow::s_pause(void) {
	emu_thread_pause();
	info.pause_from_gui = !info.pause_from_gui;
	emu_pause(info.pause_from_gui);
	emu_thread_continue();
	update_menu_nes();
}
void mainWindow::s_fast_forward(void) {
	if (!nsf.enabled) {
		emu_thread_pause();
		if (!fps.fast_forward) {
			egds->start_ff();
			fps_fast_forward_start();
		} else {
			fps_fast_forward_stop();
			egds->stop_ff();
		}
		emu_thread_continue();
		update_menu_nes();
	}
}
void mainWindow::s_max_speed_start(void) const {
	if (fps.max_speed) {
		return;
	}
	emu_thread_pause();
	egds->start_max_speed();
	fps_max_speed_start();
	emu_thread_continue();
}
void mainWindow::s_max_speed_stop(void) const {
	if (!fps.max_speed) {
		return;
	}
	emu_thread_pause();
	fps_max_speed_stop();
	egds->stop_max_speed();
	emu_thread_continue();
}
void mainWindow::s_toggle_gui_in_window(void) {
	bool visibility;

	if (gfx.type_of_fscreen_in_use == FULLSCR) {
		return;
	}

	emu_thread_pause();

	visibility = !menubar->isVisible();
	menubar->setVisible(visibility);
	if (visibility) {
		visibility = !cfg->toolbar.hidden;
	}
	toolbar->setVisible(visibility);
	statusbar->setVisible(visibility);
	update_gfx_monitor_dimension();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);

	emu_thread_continue();
}
void mainWindow::s_open_settings(void) {
	int index = QVariant(((QObject *)sender())->property("myValue")).toInt();

	set_dialog_geom(dlgsettings->geom);
	dlgsettings->tabWidget_Settings->setCurrentIndex(index);
	dlgsettings->setGeometry(dlgsettings->geom);
	dlgsettings->show();
	dlgsettings->activateWindow();
}
void mainWindow::s_state_save_slot_action(void) {
	int mode = QVariant(((QObject *)sender())->property("myValue")).toInt();

	emu_thread_pause();
	if (mode == SAVE) {
		save_slot_save(save_slot.slot);
		settings_pgs_save();
	} else {
		save_slot_load(save_slot.slot);
	}
	emu_thread_continue();
	update_window();
}
void mainWindow::s_state_save_slot_incdec(void) {
	int mode = QVariant(((QObject *)sender())->property("myValue")).toInt();
	BYTE new_slot;

	if (mode == INC) {
		new_slot = save_slot.slot + 1;
		if (new_slot >= SAVE_SLOTS) {
			new_slot = 0;
		}
	} else {
		new_slot = save_slot.slot - 1;
		if (new_slot >= SAVE_SLOTS) {
			new_slot = SAVE_SLOTS - 1;
		}
	}
	state_save_slot_set(new_slot, true);
}
void mainWindow::s_state_save_slot_set(void) {
	int slot = QVariant(((QObject *)sender())->property("myValue")).toInt();

	state_save_slot_set(slot, true);
}
void mainWindow::s_state_save_file(void) {
	QStringList filters;
	QString file;
	uTCHAR *fl;

	emu_thread_pause();

	filters.append(tr("Save states"));
	filters.append(tr("All files"));

	if (nsf.enabled) {
		filters[0].append(" (*.nns *.NNS)");
	} else {
		filters[0].append(" (*.pns *.PNS)");
	}
	filters[1].append(" (*.*)");

	// game genie
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		fl = gamegenie.rom;
	} else {
		fl = info.rom.file;
	}

	file = QFileDialog::getSaveFileName(this, tr("Save state on file"),
		QFileInfo(uQString(cfg->save_file)).dir().absolutePath() + "/" + QFileInfo(uQString(fl)).baseName(),
		filters.join(";;"));

	if (!file.isNull()) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			if (nsf.enabled) {
				fileinfo.setFile(QString(file) + ".nns");
			} else {
				fileinfo.setFile(QString(file) + ".pns");
			}
		}

		umemset(cfg->save_file, 0x00, usizeof(cfg->save_file));
		ustrncpy(cfg->save_file, uQStringCD(fileinfo.absoluteFilePath()), usizeof(cfg->save_file) - 1);
		save_slot_save(SAVE_SLOT_FILE);
		settings_pgs_save();
	}

	emu_thread_continue();
}
void mainWindow::s_state_load_file(void) {
	QStringList filters;
	QString file;

	emu_thread_pause();

	filters.append(tr("Save states"));
	filters.append(tr("All files"));

	if (nsf.enabled) {
		filters[0].append(" (*.nns *.NNS)");
	} else {
		filters[0].append(" (*.pns *.PNS)");
	}
	filters[1].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open save state"),
			QFileInfo(uQString(cfg->save_file)).dir().absolutePath(), filters.join(";;"));

	if (!file.isNull()) {
		QFileInfo fileinfo(file);

		if (fileinfo.exists()) {
			umemset(cfg->save_file, 0x00, usizeof(cfg->save_file));
			ustrncpy(cfg->save_file, uQStringCD(fileinfo.absoluteFilePath()), usizeof(cfg->save_file) - 1);
			if (save_slot_load(SAVE_SLOT_FILE) == EXIT_OK) {
				settings_pgs_save();
			}
		}
	}

	emu_thread_continue();
}
void mainWindow::s_open_djsc(void) {
	dlgjsc->show();
}
void mainWindow::s_tape_play(void) {
	QFileInfo rom = QFileInfo(uQString(info.rom.file));
	QStringList filters;
	QString file, dir, selected;

	if (info.no_rom || !tape_data_recorder.enabled || (tape_data_recorder.mode != TAPE_DATA_NONE)) {
		return;
	}

	emu_pause(TRUE);

	filters.append(tr("All supported formats"));
	filters.append(tr("puNES tape image"));
	filters.append(tr("Virtuanes tape image"));
	filters.append(tr("Nestopia tape image"));
	filters.append(tr("All files"));

	filters[0].append(" (*.tap *.TAP *.vtp *.VTP *.tp *.TP)");
	filters[1].append(" (*.tap *.TAP)");
	filters[2].append(" (*.vtp *.VTP)");
	filters[3].append(" (*.tp *.TP)");
	filters[4].append(" (*.*)");

	dir = rom.dir().absolutePath();

	file = QFileDialog::getOpenFileName(this, tr("Open tape image"), dir + "/" + rom.completeBaseName() + ".tap",
		filters.join(";;"), &selected, QFileDialog::DontUseNativeDialog);

	if (!file.isNull()) {
		BYTE mode = TAPE_DATA_TYPE_TAP;
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(file + ".tap");
		}

		if (fileinfo.suffix().compare("tap", Qt::CaseInsensitive) == 0) {
			mode = TAPE_DATA_TYPE_TAP;
		} else if (fileinfo.suffix().compare("vtp", Qt::CaseInsensitive) == 0) {
			mode = TAPE_DATA_TYPE_VIRTUANES;
		} else if (fileinfo.suffix().compare("tp", Qt::CaseInsensitive) == 0) {
			mode = TAPE_DATA_TYPE_NESTOPIA;
		} else {
			QMessageBox::critical(nullptr, tr("Tape Image"), tr("Unsupported format"), QMessageBox::Ok);
			emu_pause(FALSE);
			return;
		}

		if (tape_data_recorder_init(uQStringCD(fileinfo.absoluteFilePath()), mode, TAPE_DATA_PLAY) == EXIT_ERROR) {
			QMessageBox::critical(nullptr, tr("Tape Image"), tr("Error opening tape image file"), QMessageBox::Ok);
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_tape_record(void) {
	QFileInfo rom = QFileInfo(uQString(info.rom.file));
	QStringList filters;
	QString file, dir, selected;

	if (info.no_rom || !tape_data_recorder.enabled || (tape_data_recorder.mode != TAPE_DATA_NONE)) {
		return;
	}

	emu_pause(TRUE);

	filters.append(tr("puNES tape image"));
	filters.append(tr("Virtuanes tape image"));
	filters.append(tr("Nestopia tape image"));
	filters.append(tr("WAVE tape image"));
	filters.append(tr("All files"));

	filters[0].append(" (*.tap *.TAP)");
	filters[1].append(" (*.vtp *.VTP)");
	filters[2].append(" (*.tp *.TP)");
	filters[3].append(" (*.wav *.WAV)");
	filters[4].append(" (*.*)");

	dir = rom.dir().absolutePath();

	file = QFileDialog::getSaveFileName(this, tr("Save tape image"), dir + "/" + rom.completeBaseName() + ".tap",
		filters.join(";;"), &selected, QFileDialog::DontUseNativeDialog);

	if (!file.isNull()) {
		BYTE mode = TAPE_DATA_TYPE_TAP;
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(file + ".tap");
		}

		if (fileinfo.suffix().compare("tap", Qt::CaseInsensitive) == 0) {
			mode = TAPE_DATA_TYPE_TAP;
		} else if (fileinfo.suffix().compare("vtp", Qt::CaseInsensitive) == 0) {
			mode = TAPE_DATA_TYPE_VIRTUANES;
		} else if (fileinfo.suffix().compare("tp", Qt::CaseInsensitive) == 0) {
			mode = TAPE_DATA_TYPE_NESTOPIA;
		} else if (fileinfo.suffix().compare("wav", Qt::CaseInsensitive) == 0) {
			mode = TAPE_DATA_TYPE_WAV;
		} else {
			fileinfo.setFile(fileinfo.absoluteFilePath() + ".tap");
			mode = TAPE_DATA_TYPE_TAP;
		}

		if (tape_data_recorder_init(uQStringCD(fileinfo.absoluteFilePath()), mode, TAPE_DATA_RECORD) == EXIT_ERROR) {
			QErrorMessage errorMessage;

			errorMessage.showMessage(tr("Error opening tape image file"));
			errorMessage.exec();
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_tape_stop(void) {
	if (info.no_rom || !tape_data_recorder.enabled || (tape_data_recorder.mode == TAPE_DATA_NONE)) {
		return;
	}

	emu_pause(TRUE);

	tape_data_recorder_stop();

	emu_pause(FALSE);
}
void mainWindow::s_show_log(void) {
	set_dialog_geom(dlglog->geom);
	dlglog->setGeometry(dlglog->geom);
	dlglog->show();
	dlglog->activateWindow();
}
void mainWindow::s_help(void) {
	QDateTime compiled = QDateTime::fromString(COMPILED, "MMddyyyyhhmmss");
	QMessageBox *about = new QMessageBox(this);
	QString text;

	emu_pause(TRUE);

	about->setAttribute(Qt::WA_DeleteOnClose);
	about->setWindowTitle(QString(NAME));

	about->setWindowModality(Qt::WindowModal);

	about->setWindowIcon(QIcon(":/icon/icons/application.png"));
	about->setIconPixmap(QPixmap(":/pics/pics/punes_banner.png"));

	text.append("<center><h2>" + QString(NAME) + " ");
	if (info.portable) {
		text.append(tr("portable version") + " ");
	}
	text.append(QString(VERSION) + "</h2></center>");

#if defined (WITH_GIT_INFO)
	text.append("<center><h4>[<font color='#800000'>Commit " + QString(GIT_COUNT_COMMITS) + "</font> " + "<a href=\"https://github.com/shinyoyo/puNES/commit/" + QString(GIT_LAST_COMMIT_HASH) + "\">" + QString(GIT_LAST_COMMIT) + "</a>]</h4></center>");
#endif
	text.append("<center>" + tr("Nintendo Entertainment System Emulator") + "</center>");
	text.append("<center>" + tr("Compiled") + " " + QLocale().toString(compiled, QLocale::ShortFormat) + " (" + QString(ENVIRONMENT));
	text.append(", " + QString(VERTYPE) + ")</center>");

	about->setText(text);

	text = "<center>" + QString(COPYRUTF8) + "</center>\n";
	text.append("<center><a href=\"" + QString(GITLAB) + "\">" + "GitLab Page</a></center>");
	text.append("<center><a href=\"" + QString(GITHUB) + "\">" + "GitHub Page</a></center>");
	text.append("<center><a href=\"" + QString(WEBSITE) + "\">" + "NesDev Forum</a></center>");

	about->setInformativeText(text);

	about->setStandardButtons(QMessageBox::Ok);
	about->setDefaultButton(QMessageBox::Ok);

	about->show();
	about->activateWindow();
	about->exec();

	emu_pause(FALSE);
}

void mainWindow::s_fullscreen(void) {
#if defined (_WIN32)
	static Qt::WindowFlags window_flags = windowFlags();
#endif

	if (gui.in_update) {
		return;
	}

	if ((cfg->fullscreen == NO_FULLSCR) || (cfg->fullscreen == NO_CHANGE)) {
#if defined (_WIN32)
		window_flags = windowFlags();
#endif
		if (gfx.is_wayland || cfg->fullscreen_in_window) {
			QRect fs_win_geom = win_handle_screen()->availableGeometry();
#if defined (_WIN32)
			// lo showMaximized sotto windows non considera la presenza della barra delle applicazioni
			// cercando di impostare una dimensione falsata percio' ridimensiono la finestra manualmente.
			bool desktop_resolution = false;
#else
			bool desktop_resolution = true;
#endif

			gfx.type_of_fscreen_in_use = FULLSCR_IN_WINDOW;
			gfx.w[FSCR_RESIZE] = 0;
			gfx.h[FSCR_RESIZE] = 0;
#if defined (FULLSCREEN_RESFREQ)
			if ((cfg->fullscreen_res_w >= 0) && (cfg->fullscreen_res_h >= 0) &&
				((cfg->fullscreen_res_w != win_handle_screen()->availableGeometry().width()) ||
				(cfg->fullscreen_res_h != win_handle_screen()->availableGeometry().height()))) {
				fs_win_geom = QRect(org_geom.x(), org_geom.y(), cfg->fullscreen_res_w, cfg->fullscreen_res_h);
				desktop_resolution = false;
			}
#endif
			reset_min_max_size();
			if (desktop_resolution) {
#if QT_VERSION == QT_VERSION_CHECK(5, 12, 8)
				// con le QT 5.12.8 (Xubuntu 2004) lo showMaximized non funziona per un bug
				// nelle Qt quindi utilizzo il vecchio metodo.
				fs_geom = fs_win_geom;
				update_gfx_monitor_dimension();
				move(fs_geom.x() - (geometry().x() - x()), fs_geom.y() - (geometry().y() - y()));
				show();
				fullscreen_resize = false;
				gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE, FALSE);
#else
				fullscreen_resize = true;
				showMaximized();
#endif
			} else {
				show();
				fullscreen_resize = true;
				setGeometry(fs_win_geom);
			}
		} else {
			gfx.type_of_fscreen_in_use = FULLSCR;
			update_gfx_monitor_dimension();
			menubar->setVisible(false);
			toolbar->setVisible(false);
			statusbar->setVisible(false);
			reset_min_max_size();
#if defined (_WIN32)
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

			// sposto la finestra nell'angolo superiore del monitor
			move(fs_geom.x() - (geometry().x() - x()), fs_geom.y() - (geometry().y() - y()));
			setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
			showNormal();
			gfx.w[FSCR_RESIZE] = 0;
			gfx.h[FSCR_RESIZE] = 0;
			fullscreen_resize = false;
			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE, FALSE);
#else
			move(fs_geom.x(), fs_geom.y());
			show();
			fullscreen_resize = true;
			showFullScreen();
#endif
		}
	} else {
		if (gfx.type_of_fscreen_in_use == FULLSCR) {
			menubar->setVisible(visibility.menubar);
			statusbar->setVisible(visibility.toolbars);
			toolbar->setVisible(visibility.toolbars);
		}
		gfx.type_of_fscreen_in_use = NO_FULLSCR;
#if defined (_WIN32)
		setWindowFlags(window_flags);
#endif
		gfx.w[FSCR_RESIZE] = 0;
		gfx.h[FSCR_RESIZE] = 0;
		fullscreen_resize = false;
		showNormal();
		gfx_set_screen(gfx.scale_before_fscreen, NO_CHANGE, NO_CHANGE, NO_FULLSCR, NO_CHANGE, FALSE, FALSE);
		setGeometry(org_geom.x(), org_geom.y(), geometry().width(), geometry().height());
		// al rientro dal fullscreen a finestra devo eseguire un update() ritardato per ridisignare correttamente la GUI.
		if (gfx.is_wayland) {
			QTimer::singleShot(200, this, [this]() { update(); });
		}
	}

	emu_thread_continue();

	gui_external_control_windows_show();
	wscreen->activateWindow();
	setup_in_out_fullscreen = false;
}
void mainWindow::s_shcjoy_read_timer(void) {
	if (!shcjoy.enabled) {
		return;
	}

	if (js_jdev_read_shcut(&shcjoy.sch) == EXIT_OK) {
		int index;

		for (index = 0; index < SET_MAX_NUM_SC; index++) {
			if (shcjoy.sch.value == shcjoy.shortcut[index]) {
				int sch = index + SET_INP_SC_OPEN;

				// shortcut attivi finche' il pulsante/asse e' premuto
				switch (sch) {
					case SET_INP_SC_SHOUT_INTO_MIC:
						shout_into_mic(shcjoy.sch.mode);
						return;
					case SET_INP_SC_HOLD_FAST_FORWARD:
						hold_fast_forward(shcjoy.sch.mode);
						return;
					default:
						break;
				}

				// shortcut che si attivano al rilascio del pulsante/asse
				if (shcjoy.sch.mode == RELEASED) {
					switch (sch) {
						case SET_INP_SC_OPEN:
							action_Open->trigger();
							break;
						case SET_INP_SC_REC_AUDIO:
							action_Start_Stop_Audio_recording->trigger();
							break;
#if defined (WITH_FFMPEG)
						case SET_INP_SC_REC_VIDEO:
							action_Start_Stop_Video_recording->trigger();
							break;
#endif
						case SET_INP_SC_QUIT:
							action_Quit->trigger();
							break;
						case SET_INP_SC_TURN_OFF:
							action_Turn_Off->trigger();
							break;
						case SET_INP_SC_HARD_RESET:
							action_Hard_Reset->trigger();
							break;
						case SET_INP_SC_SOFT_RESET:
							action_Soft_Reset->trigger();
							break;
						case SET_INP_SC_INSERT_COIN:
							action_Insert_Coin->trigger();
							break;
						case SET_INP_SC_SWITCH_SIDES:
							action_Switch_sides->trigger();
							break;
						case SET_INP_SC_EJECT_DISK:
							action_Eject_Insert_Disk->trigger();
							break;
						case SET_INP_SC_FULLSCREEN:
							action_Fullscreen->trigger();
							break;
						case SET_INP_SC_SCREENSHOT:
							action_Save_Screenshot->trigger();
							break;
						case SET_INP_SC_SCREENSHOT_1X:
							action_Save_Unaltered_NES_screen->trigger();
							break;
						case SET_INP_SC_PAUSE:
							action_Pause->trigger();
							break;
						case SET_INP_SC_TOGGLE_FAST_FORWARD:
							action_Toogle_Fast_Forward->trigger();
							break;
						case SET_INP_SC_TOGGLE_GUI_IN_WINDOW:
							action_Toggle_GUI_in_window->trigger();
							break;
						case SET_INP_SC_MODE_PAL:
							qaction_shcut.mode_pal->trigger();
							break;
						case SET_INP_SC_MODE_NTSC:
							qaction_shcut.mode_ntsc->trigger();
							break;
						case SET_INP_SC_MODE_DENDY:
							qaction_shcut.mode_dendy->trigger();
							break;
						case SET_INP_SC_MODE_AUTO:
							qaction_shcut.mode_auto->trigger();
							break;
						case SET_INP_SC_SCALE_1X:
							qaction_shcut.scale_1x->trigger();
							break;
						case SET_INP_SC_SCALE_2X:
							qaction_shcut.scale_2x->trigger();
							break;
						case SET_INP_SC_SCALE_3X:
							qaction_shcut.scale_3x->trigger();
							break;
						case SET_INP_SC_SCALE_4X:
							qaction_shcut.scale_4x->trigger();
							break;
						case SET_INP_SC_SCALE_5X:
							qaction_shcut.scale_5x->trigger();
							break;
						case SET_INP_SC_SCALE_6X:
							qaction_shcut.scale_6x->trigger();
							break;
						case SET_INP_SC_INTERPOLATION:
							qaction_shcut.interpolation->trigger();
							break;
						case SET_INP_SC_INTEGER_FULLSCREEN:
							qaction_shcut.integer_in_fullscreen->trigger();
							break;
						case SET_INP_SC_STRETCH_FULLSCREEN:
							qaction_shcut.stretch_in_fullscreen->trigger();
							break;
						case SET_INP_SC_TOGGLE_MENUBAR_IN_FULLSCREEN:
							qaction_shcut.toggle_menubar_in_fullscreen->trigger();
							break;
						case SET_INP_SC_TOGGLE_CAPTURE_INPUT:
							qaction_shcut.toggle_capture_input->trigger();
							break;
						case SET_INP_SC_TOGGLE_NES_KEYBOARD:
							action_Virtual_Keyboard->trigger();
							break;
						case SET_INP_SC_AUDIO_ENABLE:
							qaction_shcut.audio_enable->trigger();
							break;
						case SET_INP_SC_SAVE_SETTINGS:
							qaction_shcut.save_settings->trigger();
							break;
						case SET_INP_SC_SAVE_STATE:
							action_Save_state->trigger();
							break;
						case SET_INP_SC_LOAD_STATE:
							action_Load_state->trigger();
							break;
						case SET_INP_SC_INC_SLOT:
							action_Increment_slot->trigger();
							break;
						case SET_INP_SC_DEC_SLOT:
							action_Decrement_slot->trigger();
							break;
						case SET_INP_SC_RWND_ACTIVE_MODE:
							qaction_shcut.rwnd.active->trigger();
							break;
						case SET_INP_SC_RWND_STEP_BACKWARD:
							qaction_shcut.rwnd.step_backward->trigger();
							break;
						case SET_INP_SC_RWND_STEP_FORWARD:
							qaction_shcut.rwnd.step_forward->trigger();
							break;
						case SET_INP_SC_RWND_FAST_BACKWARD:
							qaction_shcut.rwnd.fast_backward->trigger();
							break;
						case SET_INP_SC_RWND_FAST_FORWARD:
							qaction_shcut.rwnd.fast_forward->trigger();
							break;
						case SET_INP_SC_RWND_PLAY:
							qaction_shcut.rwnd.play->trigger();
							break;
						case SET_INP_SC_RWND_PAUSE:
							qaction_shcut.rwnd.pause->trigger();
							break;
						default:
							break;
					}
				}
				break;
			}
		}
	}
}
void mainWindow::s_received_message(UNUSED(quint32 instanceId), const QByteArray &message) {
	secondary_instance.mutex.lock();
	secondary_instance.message = QString(message);
	secondary_instance.mutex.unlock();
	QTimer::singleShot(100, this, SLOT(s_exec_message(void)));
}
void mainWindow::s_exec_message(void) {
	secondary_instance.mutex.lock();
	QFileInfo fileinfo(secondary_instance.message);
	secondary_instance.mutex.unlock();

	emu_pause(TRUE);

	if (fileinfo.exists()) {
		change_rom(uQStringCD(fileinfo.absoluteFilePath()));
		ustrncpy(gui.last_open_path, uQStringCD(fileinfo.absolutePath()), usizeof(gui.last_open_path) - 1);
		if (windowState() & Qt::WindowMinimized) {
			setWindowState(windowState() & ~Qt::WindowMinimized);
		}
		raise();
#if defined (_WIN32)
		QApplication::setActiveWindow(this);
#else
		activateWindow();
#endif
	}

	emu_pause(FALSE);
}

void mainWindow::s_shcut_mode(void) {
	int mode = QVariant(((QObject *)sender())->property("myValue")).toInt();

	dlgsettings->shcut_mode(mode);
}
void mainWindow::s_shcut_scale(void) {
	int scale = QVariant(((QObject *)sender())->property("myValue")).toInt();

	dlgsettings->shcut_scale(scale);
}
void mainWindow::s_shcut_interpolation(void) {
	dlgsettings->widget_Settings_Video->checkBox_Interpolation->click();
}
void mainWindow::s_shcut_integer_in_fullscreen(void) {
	dlgsettings->widget_Settings_Video->checkBox_Use_integer_scaling_in_fullscreen->click();
}
void mainWindow::s_shcut_stretch_in_fullscreen(void) {
	dlgsettings->widget_Settings_Video->checkBox_Stretch_in_fullscreen->click();
}
void mainWindow::s_shcut_audio_enable(void) {
	dlgsettings->widget_Settings_Audio->checkBox_Enable_Audio->click();
}
void mainWindow::s_shcut_save_settings(void) {
	dlgsettings->pushButton_Save_Settings->click();
}
void mainWindow::s_shcut_rwnd_active_deactive_mode(void) const {
	if (!rwnd.active) {
		toolbar->rewind->toolButton_Pause->click();
	} else {
		toolbar->rewind->toolButton_Play->click();
	}
}
void mainWindow::s_shcut_rwnd_step_backward(void) const {
	if (!rwnd.active) {
		return;
	}
	if (toolbar->rewind->step_autorepeat_timer_control()) {
		toolbar->rewind->toolButton_Step_Backward->click();
	}
}
void mainWindow::s_shcut_rwnd_step_forward(void) const {
	if (!rwnd.active) {
		return;
	}
	if (toolbar->rewind->step_autorepeat_timer_control()) {
		toolbar->rewind->toolButton_Step_Forward->click();
	}
}
void mainWindow::s_shcut_rwnd_fast_backward(void) const {
	if (!rwnd.active) {
		return;
	}
	toolbar->rewind->toolButton_Fast_Backward->click();
}
void mainWindow::s_shcut_rwnd_fast_forward(void) const {
	if (!rwnd.active) {
		return;
	}
	toolbar->rewind->toolButton_Fast_Forward->click();
}
void mainWindow::s_shcut_rwnd_play(void) const {
	if (!rwnd.active) {
		return;
	}
	toolbar->rewind->toolButton_Play->click();
}
void mainWindow::s_shcut_rwnd_pause(void) const {
	if (!rwnd.active) {
		return;
	}
	toolbar->rewind->toolButton_Pause->click();
}
void mainWindow::s_shcut_toggle_menubar(void) {
	if ((cfg->fullscreen != FULLSCR) || (gfx.type_of_fscreen_in_use != FULLSCR)) {
		return;
	}

	emu_thread_pause();

	menubar->setVisible(!menubar->isVisible());

	emu_thread_continue();
}
void mainWindow::s_shcut_toggle_capture_input(void) const {
	if (nes_keyboard.enabled) {
		gui.capture_input = !gui.capture_input;
		if (gui.capture_input) {
			gui_overlay_info_append_msg_precompiled(36, nullptr);
		} else {
			gui_overlay_info_append_msg_precompiled(37, nullptr);
		}
		statusbar->keyb->icon_pixmap(QIcon::Normal);
		statusbar->keyb->update_tooltip();
		statusbar->keyb->icon->update();
	}
}

void mainWindow::s_et_reset(BYTE type) {
	emu_thread_pause();
	make_reset(type);
	emu_thread_continue();
}
void mainWindow::s_et_gg_reset(void) {
	emu_thread_pause();
	make_reset(CHANGE_ROM);
	gamegenie.phase = GG_FINISH;
	emu_thread_continue();
}
void mainWindow::s_et_vs_reset(void) {
	emu_thread_pause();
	vs_system.watchdog.reset = FALSE;
	make_reset(RESET);
	emu_thread_continue();
}
void mainWindow::s_et_external_control_windows_show(void) {
	gui_external_control_windows_show();
}

// ----------------------------------------------------------------------------------------------

actionOneTrigger::actionOneTrigger(QObject *parent) : QAction(parent) {
	count = 0;
}
actionOneTrigger::~actionOneTrigger() = default;

void actionOneTrigger::only_one_trigger(void) {
	mutex.lock();
	if (count++ == 0) {
		trigger();
		count--;
	}
	mutex.unlock();
}
void actionOneTrigger::reset_count(void) {
	// sono nel trigger in un altro thread
	if (!mutex.tryLock(10)) {
		return;
	}
	count = 0;
	mutex.unlock();
}

// ----------------------------------------------------------------------------------------------

timerEgds::timerEgds(QObject *parent) : QTimer(parent) {
	int i;

	for (i = 0 ; i < EGDS_TOTALS; i++) {
		calls[i].count = 0;
	}

	connect(this, SIGNAL(timeout()), this, SLOT(s_draw_screen()));
}
timerEgds::~timerEgds() = default;

void timerEgds::set_fps(void) {
	if (isActive()) {
		stop();
	}
	setInterval((int)(1000.0f / (double)machine.fps));
}
void timerEgds::stop_unnecessary(void) {
	_etc(EGDS_TURN_OFF);
	stop_turn_off();
}
void timerEgds::start_pause(void) {
	_start_with_emu_thread_pause(EGDS_PAUSE);
}
void timerEgds::stop_pause(void) {
	_stop_with_emu_thread_continue(EGDS_PAUSE, FALSE | rwnd.active | fps_fast_forward_enabled() | info.turn_off);
}
void timerEgds::start_rwnd(void) {
	_start();
}
void timerEgds::stop_rwnd(void) {
	_stop((!!info.pause) | FALSE | fps_fast_forward_enabled() | info.turn_off);
}
void timerEgds::start_ff(void) {
	_start();
}
void timerEgds::stop_ff(void) {
	_stop((!!info.pause) | rwnd.active | FALSE | fps.max_speed | info.turn_off);
}
void timerEgds::start_max_speed(void) {
	_start();
}
void timerEgds::stop_max_speed(void) {
	_stop((!!info.pause) | rwnd.active | fps.fast_forward | FALSE | info.turn_off);
}
void timerEgds::start_turn_off(void) {
	_start_with_emu_thread_pause(EGDS_TURN_OFF);
}
void timerEgds::stop_turn_off(void) {
	_stop_with_emu_thread_continue(EGDS_TURN_OFF, (!!info.pause) | rwnd.active | fps_fast_forward_enabled() | FALSE);
}

void timerEgds::_start(void) {
	if (!isActive()) {
		start();
	}
}
void timerEgds::_start_with_emu_thread_pause(enum with_emu_pause type) {
	calls[type].count++;
	emu_thread_pause();
	_start();
}
void timerEgds::_stop(BYTE is_necessary) {
	if (is_necessary) {
		return;
	}
	stop();
}
void timerEgds::_stop_with_emu_thread_continue(enum with_emu_pause type, BYTE is_necessary) {
	if (calls[type].count && (--calls[type].count == 0)) {
		emu_thread_continue();
	}
	_stop(is_necessary);
}
void timerEgds::_etc(enum with_emu_pause type) {
	int i;

	for (i = 0 ; i < calls[type].count; i++) {
		emu_thread_continue();
	}
}

void timerEgds::s_draw_screen(void) {
	bool ret = false;

	if (info.no_rom) {
		return;
	}

	if (info.turn_off) {
		ret = true;
		tv_noise_effect();
	} else if (info.pause) {
		ret = true;
	} else if (rwnd.active) {
		ret = mainwin->toolbar->rewind->egds_rewind();
	} else if (fps_fast_forward_enabled()) {
		ret = true;

		switch (debugger.mode) {
			case DBG_STEP:
			case DBG_BREAKPOINT:
				return;
			case DBG_GO:
				if (debugger.breakframe) {
					return;
				}
				break;
		}
	}

	if (ret) {
#if defined (WITH_FFMPEG)
		if (info.recording_on_air) {
			recording_audio_silenced_frame();
		}
#endif
		gfx_draw_screen();
	}
}

// ----------------------------------------------------------------------------------------------

QRegularExpression qtHelper::rx_any_numbers("\\s*$");
QRegularExpression qtHelper::rx_comment_0("#.*");
QRegularExpression qtHelper::rx_comment_1("//.*");

void qtHelper::widget_set_visible(void *wdg, bool mode) {
	((QWidget *)wdg)->blockSignals(true);
	((QWidget *)wdg)->setVisible(mode);
	((QWidget *)wdg)->blockSignals(false);
}
void qtHelper::pushbutton_set_checked(void *btn, bool mode) {
	((QPushButton *)btn)->blockSignals(true);
	((QPushButton *)btn)->setChecked(mode);
	((QPushButton *)btn)->blockSignals(false);
}
void qtHelper::checkbox_set_checked(void *cbox, bool mode) {
	((QCheckBox *)cbox)->blockSignals(true);
	((QCheckBox *)cbox)->setChecked(mode);
	((QCheckBox *)cbox)->blockSignals(false);
}
void qtHelper::slider_set_value(void *slider, int value) {
	((QSlider *)slider)->blockSignals(true);
	((QSlider *)slider)->setValue(value);
	((QSlider *)slider)->blockSignals(false);
}
void qtHelper::spinbox_set_value(void *sbox, int value) {
	((QSpinBox *)sbox)->blockSignals(true);
	((QSpinBox *)sbox)->setValue(value);
	((QSpinBox *)sbox)->blockSignals(false);
}
void qtHelper::combox_set_index(void *cbox, int value) {
	((QComboBox *)cbox)->blockSignals(true);
	((QComboBox *)cbox)->setCurrentIndex(value);
	((QComboBox *)cbox)->blockSignals(false);
}
void qtHelper::lineedit_set_text(void *ledit, const QString &txt) {
	((QLineEdit *)ledit)->blockSignals(true);
	((QLineEdit *)ledit)->setText(txt);
	((QLineEdit *)ledit)->blockSignals(false);
}
