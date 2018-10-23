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

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QDesktopWidget>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <libgen.h>
#include "mainWindow.moc"
#include "dlgSettings.hpp"
#include "common.h"
#include "recent_roms.h"
#include "fds.h"
#include "patcher.h"
#include "save_slot.h"
#include "version.h"
#include "audio/wave.h"
#include "vs_system.h"
#include "timeline.h"
#include "c++/l7zip/l7z.h"
#include "gui.h"

enum state_incdec_enum { INC, DEC };
enum state_save_enum { SAVE, LOAD };

mainWindow::mainWindow() : QMainWindow() {
	setupUi(this);

	position.setX(100);
	position.setY(100);

	screen = new wdgScreen(centralwidget);
	statusbar = new wdgStatusBar(this);
	translator = new QTranslator();
	qtTranslator = new QTranslator();
	shcjoy.timer = new QTimer(this);

	setWindowIcon(QIcon(":icon/icons/application.png"));
	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
	setStatusBar(statusbar);

	// creo gli shortcuts
	for (int i = 0; i < SET_MAX_NUM_SC; i++) {
		shortcut[i] = new QShortcut(this);
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
	qaction_shcut.stretch_in_fullscreen = new QAction(this);
	qaction_shcut.audio_enable = new QAction(this);
	qaction_shcut.save_settings = new QAction(this);

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
	}

	connect(this, SIGNAL(fullscreen(bool)), this, SLOT(s_fullscreen(bool)));
	connect(shcjoy.timer, SIGNAL(timeout()), this, SLOT(s_shcjoy_read_timer()));

	// creo il thread per la gestione dell'emu_frame().
	{
		thref.thr = new QThread(this);

		thref.obj = new objEmuFrame();
		thref.obj->moveToThread(thref.thr);

		connect(thref.thr, SIGNAL(started()), thref.obj, SLOT(loop()));
		connect(thref.obj, SIGNAL(finished()), thref.thr, SLOT(quit()));
		connect(thref.obj, SIGNAL(finished()), thref.obj, SLOT(deleteLater()));
		connect(thref.thr, SIGNAL(finished()), thref.thr, SLOT(deleteLater()));
		connect(thref.obj, SIGNAL(gg_reset()), this, SLOT(s_ef_gg_reset()));
		connect(thref.obj, SIGNAL(vs_reset()), this, SLOT(s_ef_vs_reset()));
	}

	shcjoy_start();

	connect_menu_signals();
	shortcuts();

	adjustSize();
	setFixedSize(size());

	installEventFilter(this);

	set_language(cfg->language);
}
mainWindow::~mainWindow() {}

#if defined (__WIN32__)
bool mainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result) {
	MSG *msg = (MSG *)message;

	switch (msg->message) {
#if defined (WITH_D3D9)
		case WM_ENTERSIZEMOVE:
			break;
		case WM_EXITSIZEMOVE: {
			HMONITOR monitor = MonitorFromWindow((HWND)winId(), MONITOR_DEFAULTTOPRIMARY);

			gfx_control_changed_adapter(&monitor);
			break;
		}
#endif
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
			gui_control_pause_bck(event->type());
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
	if (gui.start == TRUE) {
		gui_external_control_windows_update_pos();
	}
	QMainWindow::moveEvent(event);
}
void mainWindow::resizeEvent(QResizeEvent *event) {
	if (gui.start == TRUE) {
		gui_external_control_windows_update_pos();
	}
	QMainWindow::resizeEvent(event);
}
void mainWindow::closeEvent(QCloseEvent *event) {
	dlgsettings->close();

	info.stop = TRUE;

	shcjoy_stop();

	// in linux non posso spostare tramite le qt una finestra da un monitor
	// ad un'altro, quindi salvo la posizione solo se sono sul monitor 0;
	if (qApp->desktop()->screenNumber(this) == 0) {
		if (cfg->fullscreen == NO_FULLSCR) {
			cfg->last_pos.x = pos().x();
			cfg->last_pos.y = pos().y();
		}
		cfg->last_pos_settings.x = dlgsettings->geom.x();
		cfg->last_pos_settings.y = dlgsettings->geom.y();
	}

	settings_save_GUI();

	QMainWindow::closeEvent(event);
}

void mainWindow::retranslateUi(mainWindow *mainWindow) {
	Ui::mainWindow::retranslateUi(mainWindow);
	shortcuts();
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

	statusbar->update_statusbar();
}
void mainWindow::set_language(int lang) {
	QString lng = "en", file = "en_EN", dir = ":/tr/translations";

	if ((lang == cfg->language) && (gui.start == TRUE)) {
		return;
	}

	qApp->removeTranslator(translator);

	// solo per testare le nuove traduzioni
	if (gui.start == FALSE) {
		QFile ext(uQString(info.base_folder) + "/test.qm");

		if (ext.exists()) {
			if (translator->load("test.qm", uQString(info.base_folder))) {
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
		case LNG_RUSSIAN:
			lng = "ru";
			file = "ru_RU";
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

	if (js_is_null(&cfg->input.shcjoy_id)) {
		return;
	}

	for (int i = 0; i < SET_MAX_NUM_SC; i++) {
		shcjoy.shortcut[i] = name_to_jsv(uQStringCD(QString(*(QString *)settings_inp_rd_sc(i + SET_INP_SC_OPEN, JOYSTICK))));
	}

	js_shcut_init();

	shcjoy.enabled = true;
	shcjoy.timer->start(13);
}
void mainWindow::shcjoy_stop(void) {
	shcjoy.enabled = false;
	shcjoy.timer->stop();
	js_shcut_stop();
}
void mainWindow::control_visible_cursor(void) {
	if ((nsf.enabled == FALSE) && (gmouse.hidden == FALSE) && (input_draw_target() == FALSE)) {
		if (cfg->fullscreen == FULLSCR) {
			gui_cursor_hide(TRUE);
		} else if ((gui_get_ms() - gmouse.timer) >= 2000) {
			gui_cursor_hide(TRUE);
		}
	}
}
void mainWindow::make_reset(int type) {
	thref.mutex.lock();

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
		thref.mutex.unlock();
		s_quit();
	}

	thref.mutex.unlock();

	// dopo un reset la pause e' automaticamente disabilitata quindi faccio
	// un aggiornamento del submenu NES per avere la voce correttamente settata.
	update_menu_nes();
}
void mainWindow::change_rom(const uTCHAR *rom) {
	info.rom.from_load_menu = emu_ustrncpy(info.rom.from_load_menu, (uTCHAR *)rom);
	gamegenie_reset();
	gamegenie_free_paths();
	make_reset(CHANGE_ROM);
	gui_update();
}
void mainWindow::state_save_slot_set(int slot, bool on_video) {
	if (info.no_rom | info.turn_off) {
		return;
	}
	save_slot.slot = slot;
	if (on_video == true) {
		text_save_slot(SAVE_SLOT_INCDEC);
	}
}

void mainWindow::connect_menu_signals(void) {
	// File
	connect_action(action_Open, SLOT(s_open()));
	connect_action(action_Apply_IPS_Patch, SLOT(s_apply_ips_patch()));
	connect_action(action_Open_working_folder, SLOT(s_open_working_folder()));
	connect_action(action_Quit, SLOT(s_quit()));
	// NES
	connect_action(action_Turn_Off, SLOT(s_turn_on_off()));
	connect_action(action_Hard_Reset, HARD, SLOT(s_make_reset()));
	connect_action(action_Soft_Reset, RESET, SLOT(s_make_reset()));
	connect_action(action_Insert_Coin, SLOT(s_insert_coin()));
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
	connect_action(action_Start_Stop_WAV_recording, SLOT(s_start_stop_wave()));
	connect_action(action_Fullscreen, SLOT(s_set_fullscreen()));
	connect_action(action_Pause, SLOT(s_pause()));
	connect_action(action_Fast_Forward, SLOT(s_fast_forward()));
	connect_action(action_Save_Screenshot, SLOT(s_save_screenshot()));

	// Settings
	connect_action(action_General, 0, SLOT(s_open_settings()));
	connect_action(action_Video, 1, SLOT(s_open_settings()));
	connect_action(action_Audio, 2, SLOT(s_open_settings()));
	connect_action(action_Input, 3, SLOT(s_open_settings()));
	connect_action(action_PPU, 4, SLOT(s_open_settings()));
	connect_action(action_Cheats, 5, SLOT(s_open_settings()));

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
	// State/[Save to file, Load from file]
	connect_action(action_State_Save_to_file, SLOT(s_state_save_file()));
	connect_action(action_State_Load_from_file, SLOT(s_state_load_file()));
	// Help/About
	connect_action(action_About, SLOT(s_help()));

	// tools
	connect_action(action_Vs_System, SLOT(s_set_vs_window()));
	connect_action(action_APU_channels, SLOT(s_set_apu_channels()));
	connect_action(action_PPU_Hacks, SLOT(s_set_ppu_hacks()));

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
	connect_action(qaction_shcut.stretch_in_fullscreen, SLOT(s_shcut_stretch_in_fullscreen()));
	connect_action(qaction_shcut.audio_enable, SLOT(s_shcut_audio_enable()));
	connect_action(qaction_shcut.save_settings, SLOT(s_shcut_save_settings()));
}
void mainWindow::connect_action(QAction *action, const char *member) {
	connect(action, SIGNAL(triggered()), this, member);
}
void mainWindow::connect_action(QAction *action, int value, const char *member) {
	action->setProperty("myValue", QVariant(value));
	connect_action(action, member);
}
void mainWindow::shortcuts(void) {
	// se non voglio che gli shortcut funzionino durante il fullscreen, basta
	// utilizzare lo shortcut associato al QAction. In questo modo quando nascondero'
	// la barra del menu, automaticamente questi saranno disabilitati.

	// File
	connect_shortcut(action_Open, SET_INP_SC_OPEN, SLOT(s_open()));
	connect_shortcut(action_Quit, SET_INP_SC_QUIT, SLOT(s_quit()));
	// NES
	connect_shortcut(action_Turn_Off, SET_INP_SC_TURN_OFF, SLOT(s_turn_on_off()));
	connect_shortcut(action_Hard_Reset, SET_INP_SC_HARD_RESET, SLOT(s_make_reset()));
	connect_shortcut(action_Soft_Reset, SET_INP_SC_SOFT_RESET, SLOT(s_make_reset()));
	connect_shortcut(action_Insert_Coin, SET_INP_SC_INSERT_COIN, SLOT(s_insert_coin()));
	connect_shortcut(action_Switch_sides, SET_INP_SC_SWITCH_SIDES, SLOT(s_disk_side()));
	connect_shortcut(action_Eject_Insert_Disk, SET_INP_SC_EJECT_DISK, SLOT(s_eject_disk()));
	connect_shortcut(action_Start_Stop_WAV_recording, SET_INP_SC_WAV, SLOT(s_start_stop_wave()));
	connect_shortcut(action_Fullscreen, SET_INP_SC_FULLSCREEN, SLOT(s_set_fullscreen()));
	connect_shortcut(action_Pause, SET_INP_SC_PAUSE, SLOT(s_pause()));
	connect_shortcut(action_Fast_Forward, SET_INP_SC_FAST_FORWARD, SLOT(s_fast_forward()));
	connect_shortcut(action_Save_Screenshot, SET_INP_SC_SCREENSHOT, SLOT(s_save_screenshot()));
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
	// Settings/Video/[Interpolation, Stretch in fullscreen]
	connect_shortcut(qaction_shcut.interpolation, SET_INP_SC_INTERPOLATION,
		SLOT(s_shcut_interpolation()));
	connect_shortcut(qaction_shcut.stretch_in_fullscreen, SET_INP_SC_STRETCH_FULLSCREEN,
		SLOT(s_shcut_stretch_in_fullscreen()));
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
}
void mainWindow::connect_shortcut(QAction *action, int index) {
	QString *sc = (QString *)settings_inp_rd_sc(index, KEYBOARD);

	if (sc->isEmpty() == false) {
		QStringList text = action->text().split('\t');

		action->setShortcut(QKeySequence((QString)(*sc)));
		action->setText(text.at(0) + '\t' + (QString)(*sc));
	}
}
void mainWindow::connect_shortcut(QAction *action, int index, const char *member) {
	QString *sc = (QString *)settings_inp_rd_sc(index, KEYBOARD);

	if (sc->isEmpty() == false) {
		QStringList text = action->text().split('\t');
		QVariant value = action->property("myValue");

		shortcut[index]->setKey(QKeySequence((QString)(*sc)));
		if (!value.isNull()) {
			shortcut[index]->setProperty("myValue", value);
		}
		// disconnetto il vecchio (se presente)
		disconnect(shortcut[index], SIGNAL(activated()), this, member);
		// connetto il nuovo
		connect(shortcut[index], SIGNAL(activated()), this, member);

		action->setText(text.at(0) + '\t' + (QString)(*sc));
	}
}

void mainWindow::update_menu_file(void) {
	if (info.no_rom) {
		action_Apply_IPS_Patch->setEnabled(false);
	} else {
		action_Apply_IPS_Patch->setEnabled(true);
	}

	// recent roms
	if (recent_roms_count() > 0) {
		int i;

		menu_Recent_Roms->clear();

		for (i = 0; i < RECENT_ROMS_MAX; i++) {
			QString description = QString((const QChar *) recent_roms_item(i),
					recent_roms_item_size(i));
			QFileInfo rom(description);
			QAction *action = new QAction(this);

			if (description.isEmpty()) {
				break;
			}

			action->setText(QFileInfo(description).completeBaseName());

			if (rom.suffix().isEmpty()) {
				action->setIcon(QIcon(":/icon/icons/nes_file.svg"));
			} else if (!QString::compare(rom.suffix(), "fds", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/fds_file.svg"));
			} else if (!QString::compare(rom.suffix(), "fm2", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/fm2_file.svg"));
			} else if (!QString::compare(rom.suffix(), "nsf", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/nsf_file.svg"));
			} else if (!QString::compare(rom.suffix(), "nsfe", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/nsf_file.svg"));
			} else {
				action->setIcon(QIcon(":/icon/icons/nes_file.svg"));
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
		action_Turn_Off->setText(tr("&Turn On") + '\t' + ((QString)*sc));
		action_Turn_Off->setIcon(QIcon(":/icon/icons/turn_on.svg"));
	} else {
		action_Turn_Off->setText(tr("&Turn Off") + '\t' + ((QString)*sc));
		action_Turn_Off->setIcon(QIcon(":/icon/icons/turn_off.svg"));
	}

	if (info.no_rom) {
		action_Turn_Off->setEnabled(false);
	} else {
		action_Turn_Off->setEnabled(true);
	}

	if (info.no_rom | info.turn_off) {
		action_Hard_Reset->setEnabled(false);
		action_Soft_Reset->setEnabled(false);
	} else {
		action_Hard_Reset->setEnabled(true);
		action_Soft_Reset->setEnabled(true);
	}

	if (vs_system.enabled == TRUE) {
		action_Insert_Coin->setEnabled(true);
	} else {
		action_Insert_Coin->setEnabled(false);
	}

	sc = (QString *)settings_inp_rd_sc(SET_INP_SC_EJECT_DISK, KEYBOARD);

	if (fds.info.enabled) {
		if (fds.drive.disk_ejected) {
			action_Eject_Insert_Disk->setText(tr("&Insert disk") + '\t' + ((QString)*sc));
		} else {
			action_Eject_Insert_Disk->setText(tr("&Eject disk") + '\t' + ((QString)*sc));
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
		action_Eject_Insert_Disk->setText(tr("&Eject/Insert disk") + '\t' + ((QString)*sc));
		menu_Disk_Side->setEnabled(false);
		action_Eject_Insert_Disk->setEnabled(false);
	}

	sc = (QString *)settings_inp_rd_sc(SET_INP_SC_WAV, KEYBOARD);

	if (info.no_rom | info.turn_off) {
		action_Start_Stop_WAV_recording->setEnabled(false);
		action_Start_Stop_WAV_recording->setText(tr("Start/Stop &WAV recording") + '\t' + ((QString)*sc));
		action_Start_Stop_WAV_recording->setIcon(QIcon(":/icon/icons/wav_start.svg"));
	} else {
		action_Start_Stop_WAV_recording->setEnabled(true);
		if (info.wave_in_record) {
			action_Start_Stop_WAV_recording->setText(tr("Stop &WAV recording") + '\t' + ((QString)*sc));
			action_Start_Stop_WAV_recording->setIcon(QIcon(":/icon/icons/wav_stop.svg"));
		} else {
			action_Start_Stop_WAV_recording->setText(tr("Start &WAV recording") + '\t' + ((QString)*sc));
			action_Start_Stop_WAV_recording->setIcon(QIcon(":/icon/icons/wav_start.svg"));
		}
	}

	if (info.pause_from_gui == TRUE) {
		action_Pause->setChecked(true);
	} else {
		action_Pause->setChecked(false);
	}

	if (nsf.enabled == FALSE) {
		action_Fast_Forward->setEnabled(true);

		if (fps.fast_forward == TRUE) {
			action_Fast_Forward->setChecked(true);
		} else {
			action_Fast_Forward->setChecked(false);
		}
	} else {
		action_Fast_Forward->setEnabled(false);
	}
}
void mainWindow::update_menu_tools(void) {
	action_Vs_System->setChecked(ext_win.vs_system);
	action_APU_channels->setChecked(ext_win.apu_channels);
	action_PPU_Hacks->setChecked(ext_win.ppu_hacks);
}
void mainWindow::update_menu_state(void) {
	bool state = false;

	if (!(info.no_rom | info.turn_off)) {
		state = true;
	}

	action_Save_state->setEnabled(state);
	action_Load_state->setEnabled(state);
	action_State_Save_to_file->setEnabled(state);
	action_State_Load_from_file->setEnabled(state);

	switch (save_slot.slot) {
		case 0:
			action_State_Slot_0->setChecked(true);
			break;
		case 1:
			action_State_Slot_1->setChecked(true);
			break;
		case 2:
			action_State_Slot_2->setChecked(true);
			break;
		case 3:
			action_State_Slot_3->setChecked(true);
			break;
		case 4:
			action_State_Slot_4->setChecked(true);
			break;
		case 5:
			action_State_Slot_5->setChecked(true);
			break;
		case 6:
			action_State_Slot_6->setChecked(true);
			break;
		case 7:
			action_State_Slot_7->setChecked(true);
			break;
		case 8:
			action_State_Slot_8->setChecked(true);
			break;
		case 9:
			action_State_Slot_9->setChecked(true);
			break;
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

	// potrei essere entrato con il CTRL+O
	tl.key = FALSE;

	if (l7z_present() == TRUE) {
		if ((l7z_control_ext(uL("rar")) == EXIT_OK)) {
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

	file = QFileDialog::getOpenFileName(this, tr("Open File"), uQString(gui.last_open_path),
		filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		change_rom(uQStringCD(fileinfo.absoluteFilePath()));
		ustrncpy(gui.last_open_path, uQStringCD(fileinfo.absolutePath()), usizeof(gui.last_open_path) - 1);
	}

	emu_pause(FALSE);
}
void mainWindow::s_apply_ips_patch(void) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("All supported formats"));
	filters.append(tr("Compressed files"));
	filters.append(tr("IPS patch files"));
	filters.append(tr("All files"));

	if (l7z_present() == TRUE) {
		if ((l7z_control_ext(uL("rar")) == EXIT_OK)) {
			filters[0].append(" (*.zip *.ZIP *.7z *.7Z *.rar *.RAR *.ips *.IPS)");
			filters[1].append(" (*.zip *.ZIP *.7z *.7Z *.rar *.RAR)");
		} else {
			filters[0].append(" (*.zip *.ZIP *.7z *.7Z *.ips *.IPS)");
			filters[1].append(" (*.zip *.ZIP *.7z *.7Z)");
		}
	} else {
		filters[0].append(" (*.zip *.ZIP *.ips *.IPS)");
		filters[1].append(" (*.zip *.ZIP)");
	}

	filters[2].append(" (*.ips *.IPS)");
	filters[3].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open IPS Patch"), uQString(gui.last_open_ips_path),
		filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		patcher_ctrl_if_exist(uQStringCD(fileinfo.absoluteFilePath()));

		if (patcher.file) {
			if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase == GG_EXECUTE)) {
				change_rom(gamegenie.rom);
			} else {
				change_rom(info.rom.file);
			}
			ustrncpy(gui.last_open_ips_path, uQStringCD(fileinfo.absolutePath()),
				usizeof(gui.last_open_ips_path) - 1);
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_open_recent_roms(void) {
	int index = QVariant(((QObject *)sender())->property("myValue")).toInt();
	QString current = QString((const QChar *) recent_roms_current(), recent_roms_current_size());
	QString item = QString((const QChar *) recent_roms_item(index), recent_roms_item_size(index));

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
void mainWindow::s_open_working_folder(void) {
	QDesktopServices::openUrl(QUrl(uQString(info.base_folder)));
}
void mainWindow::s_quit(void) {
	close();
}
void mainWindow::s_turn_on_off(void) {
	info.turn_off = !info.turn_off;
	info.pause_frames_drawscreen = 0;

	if (!info.turn_off) {
		make_reset(HARD);
	}

	update_menu_nes();
	update_menu_state();
}
void mainWindow::s_make_reset(void) {
	int type = QVariant(((QObject *)sender())->property("myValue")).toInt();

	make_reset(type);
}
void mainWindow::s_insert_coin(void) {
	gui_vs_system_insert_coin();
}
void mainWindow::s_disk_side(void) {
	int side = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (side == 0xFFF) {
		side = fds.drive.side_inserted ^ 0x01;
		if (side >= fds.info.total_sides) {
			return;
		}
	}

	if (fds.drive.side_inserted == side) {
		return;
	}

	if (fds.drive.disk_ejected) {
		fds.side.change.new_side = 0xFF;
		fds.side.change.delay = 0;
		fds_disk_op(FDS_DISK_SELECT, side);
	} else {
		fds.side.change.new_side = side;
		fds.side.change.delay = 3000000;
		fds_disk_op(FDS_DISK_EJECT, 0);
	}

	update_menu_nes();
}
void mainWindow::s_eject_disk(void) {
	if (!fds.drive.disk_ejected) {
		fds_disk_op(FDS_DISK_EJECT, 0);
	} else {
		fds_disk_op(FDS_DISK_INSERT, 0);
	}

	update_menu_nes();
}
void mainWindow::s_start_stop_wave(void) {
	if (info.wave_in_record == FALSE) {
		QStringList filters;
		QString file;

		emu_pause(TRUE);

		filters.append(tr("MS WAVE files"));
		filters.append(tr("All files"));

		filters[0].append(" (*.wav *.WAV)");
		filters[1].append(" (*.*)");

		file = QFileDialog::getSaveFileName(this, tr("Record sound"),
				QFileInfo(uQString(info.rom.file)).completeBaseName(),
				filters.join(";;"));

		if (file.isNull() == false) {
			QFileInfo fileinfo(file);

			if (fileinfo.suffix().isEmpty()) {
				fileinfo.setFile(QString(file) + ".wav");
			}

			wave_open(uQStringCD(fileinfo.absoluteFilePath()), snd.samplerate * 5);
		}

		emu_pause(FALSE);
	} else {
		wave_close();
	}
	update_menu_nes();
}
void mainWindow::s_fast_forward(void) {
	if (nsf.enabled == FALSE) {
		if (fps.fast_forward == FALSE) {
			fps_fast_forward();
		} else {
			fps_normalize();
		}
		update_menu_nes();
	}
}
void mainWindow::s_set_fullscreen(void) {
	if (gui.in_update) {
		return;
	}

	if ((cfg->fullscreen == NO_FULLSCR) || (cfg->fullscreen == NO_CHANGE)) {
		int screenNumber = qApp->desktop()->screenNumber(this);

		gfx.scale_before_fscreen = cfg->scale;
		position = pos();

		if (cfg->fullscreen_in_window == TRUE) {
			gfx.type_of_fscreen_in_use = FULLSCR_IN_WINDOW;

			gfx.w[MONITOR] = qApp->desktop()->availableGeometry(screenNumber).width()
				- (frameGeometry().width() - geometry().width());
			gfx.h[MONITOR] = qApp->desktop()->availableGeometry(screenNumber).height()
				- (frameGeometry().height() - geometry().height())
				- menubar->sizeHint().height() - 2 - statusbar->sizeHint().height();

			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE, FALSE);
			move(QPoint(0, 0));
		} else {
			gfx.type_of_fscreen_in_use = FULLSCR;

			gfx.w[MONITOR] = qApp->desktop()->screenGeometry(screenNumber).width();
			gfx.h[MONITOR] = qApp->desktop()->screenGeometry(screenNumber).height();

			menuWidget()->setVisible(false);
			statusbar->setVisible(false);
			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE, FALSE);

			// su alcune macchine, il fullscreen non avviene perche'
			// la dimensione della finestra e' fissa e le qt non riescono
			// a sbloccarla.
			setMinimumSize(QSize(0, 0));
			setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

			emit fullscreen(true);
		}
	} else {
		if (gfx.type_of_fscreen_in_use == FULLSCR) {
			emit fullscreen(false);

			menuWidget()->setVisible(true);
			statusbar->setVisible(true);
		}

		gfx_set_screen(gfx.scale_before_fscreen, NO_CHANGE, NO_CHANGE, NO_FULLSCR, NO_CHANGE, FALSE,
				FALSE);
		move(position);

		gfx.type_of_fscreen_in_use = NO_FULLSCR;
	}

	gui_external_control_windows_show();
	gui_set_focus();
}
void mainWindow::s_save_screenshot(void) {
	gfx.save_screenshot = true;
}
void mainWindow::s_pause(void) {
	info.pause_from_gui = !info.pause_from_gui;
	info.pause_frames_drawscreen = 0;

	emu_pause(info.pause_from_gui);
	update_menu_nes();
}
void mainWindow::s_open_settings(void) {
	int index = QVariant(((QObject *)sender())->property("myValue")).toInt();
	int frame_w = frameGeometry().width() - geometry().width();
	int frame_h = frameGeometry().height() - geometry().height();

	if (dlgsettings->geom.x() < frame_w) {
		dlgsettings->geom.setX(frame_w);
	}
	if (dlgsettings->geom.y() < frame_h) {
		dlgsettings->geom.setY(frame_h);
	}

	dlgsettings->tabWidget_Settings->setCurrentIndex(index);
	dlgsettings->setGeometry(dlgsettings->geom);
	dlgsettings->show();
}
void mainWindow::s_state_save_slot_action(void) {
	int mode = QVariant(((QObject *)sender())->property("myValue")).toInt();

	emu_pause(TRUE);

	if (mode == SAVE) {
		save_slot_save(save_slot.slot);
		settings_pgs_save();
	} else {
		save_slot_load(save_slot.slot);
	}

	emu_pause(FALSE);
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
	update_window();
}
void mainWindow::s_state_save_slot_set(void) {
	int slot = QVariant(((QObject *)sender())->property("myValue")).toInt();

	state_save_slot_set(slot, true);
	update_window();
}
void mainWindow::s_state_save_file(void) {
	QStringList filters;
	QString file;
	uTCHAR *fl;

	emu_pause(TRUE);

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
			QFileInfo(uQString(fl)).baseName(), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			if (nsf.enabled) {
				fileinfo.setFile(QString(file) + ".nns");
			} else {
				fileinfo.setFile(QString(file) + ".pns");
			}
		}

		umemset(cfg->save_file, 0x00, usizeof(cfg->save_file));
		ustrncpy(cfg->save_file, uQStringCD(fileinfo.absoluteFilePath()),
				usizeof(cfg->save_file) - 1);
		save_slot_save(SAVE_SLOT_FILE);
		settings_pgs_save();
	}

	emu_pause(FALSE);
}
void mainWindow::s_state_load_file(void) {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

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

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.exists()) {
			umemset(cfg->save_file, 0x00, usizeof(cfg->save_file));
			ustrncpy(cfg->save_file, uQStringCD(fileinfo.absoluteFilePath()),
				usizeof(cfg->save_file) - 1);
			if (save_slot_load(SAVE_SLOT_FILE) == EXIT_OK) {
				settings_pgs_save();
			}
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_set_vs_window(void) {
	ext_win.vs_system = !ext_win.vs_system;
	gui_external_control_windows_show();
}
void mainWindow::s_set_apu_channels(void) {
	ext_win.apu_channels = !ext_win.apu_channels;
	gui_external_control_windows_show();
}
void mainWindow::s_set_ppu_hacks(void) {
	ext_win.ppu_hacks = !ext_win.ppu_hacks;
	gui_external_control_windows_show();
}
void mainWindow::s_help(void) {
	QDateTime compiled = QDateTime::fromString(COMPILED, "MMddyyyyhhmmss");
	QMessageBox *about = new QMessageBox(this);
	QString text;

	emu_pause(TRUE);

	about->setAttribute(Qt::WA_DeleteOnClose);
	about->setWindowTitle(QString(NAME));

	about->setWindowModality(Qt::WindowModal);

	about->setWindowIcon(QIcon(QString::fromUtf8(":/icon/icons/application.png")));
	about->setIconPixmap(QPixmap(QString::fromUtf8(":/pics/pics/pushpin.png")));

	text.append("<center><h2>" + QString(NAME) + " ");
	if (info.portable) {
		text.append(tr("portable version") + " ");
	}
	text.append(QString(VERSION) + "</h2></center>\n");
#if !defined (RELEASE)
	text.append("<center><h4>[<font color='#800000'>Commit " + QString(GIT_COUNT_COMMITS) + "</font> " + "<a href=\"https://github.com/punesemu/puNES/commit/" + QString(GIT_LAST_COMMIT_HASH) + "\">" + QString(GIT_LAST_COMMIT) + "</a>]</h4></center>");
#endif
	text.append("<center>" + tr("Nintendo Entertainment System Emulator") + "</center>");
	text.append("<center>" + tr("Compiled") + " " +
			compiled.toString(Qt::DefaultLocaleShortDate) + "</center>");

	about->setText(text);

	text = "<center>" + QString(COPYRUTF8) + "</center>\n";
	text.append("<center><a href=\"" + QString(GITHUB) + "\">" + "GitHub Page</a></center>");
	text.append("<center><a href=\"" + QString(WEBSITE) + "\">" + "NesDev Forum</a></center>");
	text.append("<center>" + QString("-") + "</center>\n");
	text.append("<center>" + tr("If you like the emulator and you want to support it's development or would you pay for a beer at the programmer") + "</center>\n");
	text.append("<center><a href=\"" + QString(DONATE) + "\">" + "<img src=\":/pics/pics/btn_donate_SM.gif\">" + "</a></center>\n");
	text.append("<center>" + tr("Anyway, thank you all for the love and the help.") + "</center>");

	about->setInformativeText(text);

	about->setStandardButtons(QMessageBox::Ok);
	about->setDefaultButton(QMessageBox::Ok);

	about->show();
	about->exec();

	emu_pause(FALSE);
}

void mainWindow::s_fullscreen(bool state) {
	if (state == true) {
		showFullScreen();
	} else {
		showNormal();
	}
}
void mainWindow::s_shcjoy_read_timer(void) {
	if (shcjoy.enabled == false) {
		return;
	}

	if (js_shcut_read(&shcjoy.sch) == EXIT_OK) {
		int index;

		for (index = 0; index < SET_MAX_NUM_SC; index++) {
			if (shcjoy.sch.value == shcjoy.shortcut[index]) {
				break;
			}
		}

		if (shcjoy.sch.mode == RELEASED) {
			switch (index + SET_INP_SC_OPEN) {
				case SET_INP_SC_OPEN:
					action_Open->trigger();
					break;
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
				case SET_INP_SC_WAV:
					action_Start_Stop_WAV_recording->trigger();
					break;
				case SET_INP_SC_FULLSCREEN:
					action_Fullscreen->trigger();
					break;
				case SET_INP_SC_PAUSE:
					action_Pause->trigger();
					break;
				case SET_INP_SC_FAST_FORWARD:
					action_Fast_Forward->trigger();
					break;
				case SET_INP_SC_SCREENSHOT:
					action_Save_Screenshot->trigger();
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
				case SET_INP_SC_STRETCH_FULLSCREEN:
					qaction_shcut.stretch_in_fullscreen->trigger();
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
			}
		} else if (shcjoy.sch.mode == PRESSED) {}
	}
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
	dlgsettings->widget_wdgSettingsVideo->checkBox_Interpolation->click();
}
void mainWindow::s_shcut_stretch_in_fullscreen(void) {
	dlgsettings->widget_wdgSettingsVideo->checkBox_Stretch_in_fullscreen->click();
}
void mainWindow::s_shcut_audio_enable(void) {
	dlgsettings->widget_wdgSettingsAudio->checkBox_Enable_Audio->click();
}
void mainWindow::s_shcut_save_settings(void) {
	dlgsettings->pushButton_Save_Settings->click();
}

void mainWindow::s_ef_gg_reset(void) {
	make_reset(CHANGE_ROM);
	gamegenie.phase = GG_FINISH;
}
void mainWindow::s_ef_vs_reset(void) {
	vs_system.watchdog.reset = FALSE;
	make_reset(RESET);
}
void mainWindow::s_ef_external_control_windows_show(void) {
	gui_external_control_windows_show();
}
