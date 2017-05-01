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

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QDesktopWidget>
#else
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QDesktopWidget>
#endif
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#if defined (__linux__)
#include <unistd.h>
#include <fcntl.h>
#endif
#include <libgen.h>
#include "mainWindow.moc"
#include "dlgOverscanBorders.hpp"
#include "dlgApuChannels.hpp"
#include "dlgInput.hpp"
#include "dlgCheats.hpp"
#include "common.h"
#include "settings.h"
#include "cheat.h"
#include "conf.h"
#include "recent_roms.h"
#include "fds.h"
#include "clock.h"
#include "ppu.h"
#include "text.h"
#include "save_slot.h"
#include "version.h"
#include "audio/delay.h"
#include "audio/wave.h"
#include "vs_system.h"
#if defined (WITH_OPENGL)
#if defined (__linux__)
#include "sdl_wid.h"
#endif
#include "opengl.h"
#endif
#include "timeline.h"
#include "c++/l7zip/l7z.h"
#include "gui.h"
#if defined (__linux__) || defined (WITH_D3D9)
#define __GFX_OTHERS_FUNC__
#include "gfx_functions_inline.h"
#undef __GFX_OTHERS_FUNC__
#endif

enum state_incdec_enum { INC, DEC };
enum state_save_enum { SAVE, LOAD };

mainWindow::mainWindow(Ui::mainWindow *u, cheatObject *cho) : QMainWindow() {
	ui = u;
	statusbar = new sbarWidget(u, this);
	chobj = cho;

	position.setX(100);
	position.setY(100);

	installEventFilter(this);
	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
	setStatusBar(statusbar);

	setWindowIcon(QIcon(":icon/icons/application.png"));

	connect(this, SIGNAL(fullscreen(bool)), this, SLOT(s_fullscreen(bool)));

	// creo gli shortcuts
	for (int i = 0; i < SET_MAX_NUM_SC; i++) {
		shortcut[i] = new QShortcut(this);
	}

	{
		QFont actual;

		if (font().pointSize() > 10) {
			actual.setPointSize(10);
		} else {
			actual.setPointSize(font().pointSize());
		}
		actual.setWeight(QFont::Light);
		setFont(actual);
	}

	shcjoy.timer = new QTimer(this);
	connect(shcjoy.timer, SIGNAL(timeout()), this, SLOT(s_shcjoy_read_timer()));

	shcjoy_start();

	translator = new QTranslator();
	qtTranslator = new QTranslator();
}
mainWindow::~mainWindow() {}
void mainWindow::setup() {
	QActionGroup *grp;

	chobj->setParent(this);

	setup_video_rendering();

	connect_menu_signals();

	shortcuts();

	// NES
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Disk_1_side_A);
	grp->addAction(ui->action_Disk_1_side_B);
	grp->addAction(ui->action_Disk_2_side_A);
	grp->addAction(ui->action_Disk_2_side_B);
	grp->addAction(ui->action_Disk_3_side_A);
	grp->addAction(ui->action_Disk_3_side_B);
	grp->addAction(ui->action_Disk_4_side_A);
	grp->addAction(ui->action_Disk_4_side_B);
	// Settings/Mode
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_PAL);
	grp->addAction(ui->action_NTSC);
	grp->addAction(ui->action_Dendy);
	grp->addAction(ui->action_Mode_Auto);
	// Settings/Video/FPS
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_FPS_Default);
	grp->addAction(ui->action_FPS_60);
	grp->addAction(ui->action_FPS_59);
	grp->addAction(ui->action_FPS_58);
	grp->addAction(ui->action_FPS_57);
	grp->addAction(ui->action_FPS_56);
	grp->addAction(ui->action_FPS_55);
	grp->addAction(ui->action_FPS_54);
	grp->addAction(ui->action_FPS_53);
	grp->addAction(ui->action_FPS_52);
	grp->addAction(ui->action_FPS_51);
	grp->addAction(ui->action_FPS_50);
	grp->addAction(ui->action_FPS_49);
	grp->addAction(ui->action_FPS_48);
	grp->addAction(ui->action_FPS_47);
	grp->addAction(ui->action_FPS_46);
	grp->addAction(ui->action_FPS_45);
	grp->addAction(ui->action_FPS_44);
	// Settings/Video/Frame skip
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Fsk_Default);
	grp->addAction(ui->action_Fsk_1);
	grp->addAction(ui->action_Fsk_2);
	grp->addAction(ui->action_Fsk_3);
	grp->addAction(ui->action_Fsk_4);
	grp->addAction(ui->action_Fsk_5);
	grp->addAction(ui->action_Fsk_6);
	grp->addAction(ui->action_Fsk_7);
	grp->addAction(ui->action_Fsk_8);
	grp->addAction(ui->action_Fsk_9);
	// Settings/Video/Scale
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_1x);
	grp->addAction(ui->action_2x);
	grp->addAction(ui->action_3x);
	grp->addAction(ui->action_4x);
	grp->addAction(ui->action_5x);
	grp->addAction(ui->action_6x);
	// Settings/Video/Pixel Aspect Ratio
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_PAR_11);
	grp->addAction(ui->action_PAR_54);
	grp->addAction(ui->action_PAR_87);
	grp->addAction(ui->action_PAR_118);
	// Settings/Video/Overscan
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Oscan_Default);
	grp->addAction(ui->action_Oscan_On);
	grp->addAction(ui->action_Oscan_Off);
	// Settings/Video/Overscan/Default
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Oscan_Def_On);
	grp->addAction(ui->action_Oscan_Def_Off);
	// Settings/Video/Software Filters
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_No_Filter);
	grp->addAction(ui->action_Scale2X);
	grp->addAction(ui->action_Scale3X);
	grp->addAction(ui->action_Scale4X);
	grp->addAction(ui->action_Hq2X);
	grp->addAction(ui->action_Hq3X);
	grp->addAction(ui->action_Hq4X);
	grp->addAction(ui->action_xBRZ_2X);
	grp->addAction(ui->action_xBRZ_3X);
	grp->addAction(ui->action_xBRZ_4X);
	grp->addAction(ui->action_xBRZ_5X);
	grp->addAction(ui->action_xBRZ_6X);
	grp->addAction(ui->action_xBRZ_2X_MT);
	grp->addAction(ui->action_xBRZ_3X_MT);
	grp->addAction(ui->action_xBRZ_4X_MT);
	grp->addAction(ui->action_xBRZ_5X_MT);
	grp->addAction(ui->action_xBRZ_6X_MT);
	grp->addAction(ui->action_NTSC_Composite);
	grp->addAction(ui->action_NTSC_SVideo);
	grp->addAction(ui->action_NTSC_RGB);
	// Settings/Video/Shaders
	grp = new QActionGroup(this);
	grp->addAction(ui->action_No_Shader);
	grp->addAction(ui->action_Shader_CRT_Dotmask);
	grp->addAction(ui->action_Shader_CRT_Scanlines);
	grp->addAction(ui->action_Shader_CRT_With_Curve);
	grp->addAction(ui->action_Shader_Emboss);
	grp->addAction(ui->action_Shader_Noise);
	grp->addAction(ui->action_Shader_NTSC_2Phase_Composite);
	grp->addAction(ui->action_Shader_Old_TV);
	grp->addAction(ui->action_Shader_File);
	// Settings/Video/Palette
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Palette_PAL);
	grp->addAction(ui->action_Palette_NTSC);
	grp->addAction(ui->action_Sony_CXA2025AS_US);
	grp->addAction(ui->action_Firebrandx_Nostalgia);
	grp->addAction(ui->action_Firebrandx_YUV);
	grp->addAction(ui->action_Monochrome);
	grp->addAction(ui->action_Green);
	grp->addAction(ui->action_Palette_File);
	// Settings/Audio/Buffer Size factor
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Absf_0);
	grp->addAction(ui->action_Absf_1);
	grp->addAction(ui->action_Absf_2);
	grp->addAction(ui->action_Absf_3);
	grp->addAction(ui->action_Absf_4);
	grp->addAction(ui->action_Absf_5);
	grp->addAction(ui->action_Absf_6);
	grp->addAction(ui->action_Absf_7);
	grp->addAction(ui->action_Absf_8);
	grp->addAction(ui->action_Absf_9);
	// Settings/Audio/Samplerate
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Sample_rate_48000);
	grp->addAction(ui->action_Sample_rate_44100);
	grp->addAction(ui->action_Sample_rate_22050);
	grp->addAction(ui->action_Sample_rate_11025);
	// Settings/Audio/Channels
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Mono);
	grp->addAction(ui->action_Stereo_Panning_Mode);
	grp->addAction(ui->action_Stereo_Delay_Mode);
	// Settings/Audio/Channels/Stereo delay
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Stereo_delay_5);
	grp->addAction(ui->action_Stereo_delay_10);
	grp->addAction(ui->action_Stereo_delay_15);
	grp->addAction(ui->action_Stereo_delay_20);
	grp->addAction(ui->action_Stereo_delay_25);
	grp->addAction(ui->action_Stereo_delay_30);
	grp->addAction(ui->action_Stereo_delay_35);
	grp->addAction(ui->action_Stereo_delay_40);
	grp->addAction(ui->action_Stereo_delay_45);
	grp->addAction(ui->action_Stereo_delay_50);
	grp->addAction(ui->action_Stereo_delay_55);
	grp->addAction(ui->action_Stereo_delay_60);
	grp->addAction(ui->action_Stereo_delay_65);
	grp->addAction(ui->action_Stereo_delay_70);
	grp->addAction(ui->action_Stereo_delay_75);
	grp->addAction(ui->action_Stereo_delay_80);
	grp->addAction(ui->action_Stereo_delay_85);
	grp->addAction(ui->action_Stereo_delay_90);
	grp->addAction(ui->action_Stereo_delay_95);
	grp->addAction(ui->action_Stereo_delay_100);
	// Settings/Audio/Quality
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Audio_Quality_Low);
	grp->addAction(ui->action_Audio_Quality_High);
	// Settings/Language
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_English);
	grp->addAction(ui->action_Italian);
	grp->addAction(ui->action_Russian);
	// Settings/Cheat
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_Cheats_Disabled);
	grp->addAction(ui->action_Game_Genie);
	grp->addAction(ui->action_Cheats_List);
	// Settings/Fast Forward velocity
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_FF_2x);
	grp->addAction(ui->action_FF_3x);
	grp->addAction(ui->action_FF_4x);
	grp->addAction(ui->action_FF_5x);
	// State
	grp = new QActionGroup(this);
	grp->setExclusive(true);
	grp->addAction(ui->action_State_Slot_0);
	grp->addAction(ui->action_State_Slot_1);
	grp->addAction(ui->action_State_Slot_2);
	grp->addAction(ui->action_State_Slot_3);
	grp->addAction(ui->action_State_Slot_4);
	grp->addAction(ui->action_State_Slot_5);
	grp->addAction(ui->action_State_Slot_6);
	grp->addAction(ui->action_State_Slot_7);
	grp->addAction(ui->action_State_Slot_8);
	grp->addAction(ui->action_State_Slot_9);

	set_language(cfg->language);
}
void mainWindow::update_window() {
	// File
	update_recent_roms();
	// NES
	update_menu_nes();
	// Settings
	update_menu_settings();
	// Tools
	update_menu_tools();
	// State
	update_menu_state();

	statusbar->update_statusbar();
}
void mainWindow::change_rom(const uTCHAR *rom) {
	ustrncpy(info.load_rom_file, rom, usizeof(info.load_rom_file));
	gamegenie_reset();
#if defined (WITH_OPENGL) && defined (__WIN32__)
	gfx_sdlwe_set(SDLWIN_CHANGE_ROM, SDLWIN_NONE);
#else
	gfx_CHANGE_ROM();
#endif
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
#if defined (__WIN32__)
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
bool mainWindow::winEvent(MSG *msg, long *result) {
#else
bool mainWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
#endif

#if (QT_VERSION > QT_VERSION_CHECK(5, 0, 0))
	MSG *msg = (MSG *)message;
#endif

	switch (msg->message) {
#if defined (WITH_D3D9)
		case WM_ENTERSIZEMOVE:
			break;
		case WM_EXITSIZEMOVE: {
			HMONITOR monitor = MonitorFromWindow(winId(), MONITOR_DEFAULTTOPRIMARY);

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
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
	return QWidget::winEvent(msg, result);
#else
	return QWidget::nativeEventFilter(eventType, message, result);
#endif
}
#endif
bool mainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::Close) {
		shcjoy_stop();

		// in linux non posso spostare tramite le qt una finestra da un monitor
		// ad un'altro, quindi salvo la posizione solo se sono sul monitor 0;
		if ((cfg->fullscreen == NO_FULLSCR) && (qApp->desktop()->screenNumber(this) == 0)) {
			cfg->last_pos.x = pos().x();
			cfg->last_pos.y = pos().y();
		}

		settings_save_GUI();

		info.stop = TRUE;
	} else if (event->type() == QEvent::LanguageChange) {
		ui->retranslateUi(this);
		shortcuts();
		update_window();
#if defined (WITH_OPENGL)
		ui->action_PAR_Soft_Stretch->setText(tr("GLSL &soft stretch"));
#elif defined (WITH_D3D9)
		ui->action_PAR_Soft_Stretch->setText(tr("HLSL &soft stretch"));
#endif
	} else if (event->type() == QEvent::Move) {
		if (gui.start == TRUE) {
			gui_external_control_windows_update_pos();
		}
	} else if (event->type() == QEvent::Resize) {
		if (gui.start == TRUE) {
			gui_external_control_windows_update_pos();
		}
	}

	gui_control_pause_bck(event->type());

	return (QObject::eventFilter(obj, event));
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
void mainWindow::setup_video_rendering() {
#if defined (WITH_OPENGL)
	ui->action_PAR_Soft_Stretch->setText(tr("GLSL &soft stretch"));
#elif defined (WITH_D3D9)
	ui->action_PAR_Soft_Stretch->setText(tr("HLSL &soft stretch"));
#endif
}
void mainWindow::update_recent_roms() {
	if (recent_roms_count() > 0) {
		int i;

		ui->menu_Recent_Roms->clear();

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
				action->setIcon(QIcon(":/icon/icons/nes_file.png"));
			} else if (!QString::compare(rom.suffix(), "fds", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/fds_file.png"));
			} else if (!QString::compare(rom.suffix(), "fm2", Qt::CaseInsensitive)) {
				action->setIcon(QIcon(":/icon/icons/fm2_file.png"));
			} else {
				action->setIcon(QIcon(":/icon/icons/nes_file.png"));
			}

			action->setProperty("myValue", QVariant(i));
			ui->menu_Recent_Roms->addAction(action);
			connect(action, SIGNAL(triggered()), this, SLOT(s_open_recent_roms()));
		}
	}
}
void mainWindow::update_menu_nes() {
	QString *sc = (QString *)settings_inp_rd_sc(SET_INP_SC_TURN_OFF, KEYBOARD);

	if (info.turn_off) {
		ui->action_Turn_Off->setText(tr("&Turn On") + '\t' + ((QString)*sc));
		ui->action_Turn_Off->setIcon(QIcon(":/icon/icons/turn_on.png"));
	} else {
		ui->action_Turn_Off->setText(tr("&Turn Off") + '\t' + ((QString)*sc));
		ui->action_Turn_Off->setIcon(QIcon(":/icon/icons/turn_off.png"));
	}

	if (info.no_rom) {
		ui->action_Turn_Off->setEnabled(false);
	} else {
		ui->action_Turn_Off->setEnabled(true);
	}

	if (info.no_rom | info.turn_off) {
		ui->action_Hard_Reset->setEnabled(false);
		ui->action_Soft_Reset->setEnabled(false);
	} else {
		ui->action_Hard_Reset->setEnabled(true);
		ui->action_Soft_Reset->setEnabled(true);
	}

	if (vs_system.enabled == TRUE) {
		ui->action_Insert_Coin->setEnabled(true);
	} else {
		ui->action_Insert_Coin->setEnabled(false);
	}

	sc = (QString *)settings_inp_rd_sc(SET_INP_SC_EJECT_DISK, KEYBOARD);

	if (fds.info.enabled) {
		if (fds.drive.disk_ejected) {
			ui->action_Eject_Insert_Disk->setText(tr("&Insert disk") + '\t' + ((QString)*sc));
		} else {
			ui->action_Eject_Insert_Disk->setText(tr("&Eject disk") + '\t' + ((QString)*sc));
		}

		ui->menu_Disk_Side->setEnabled(true);
		ctrl_disk_side(ui->action_Disk_1_side_A);
		ctrl_disk_side(ui->action_Disk_1_side_B);
		ctrl_disk_side(ui->action_Disk_2_side_A);
		ctrl_disk_side(ui->action_Disk_2_side_B);
		ctrl_disk_side(ui->action_Disk_3_side_A);
		ctrl_disk_side(ui->action_Disk_3_side_B);
		ctrl_disk_side(ui->action_Disk_4_side_A);
		ctrl_disk_side(ui->action_Disk_4_side_B);
		ui->action_Eject_Insert_Disk->setEnabled(true);
	} else {
		ui->action_Eject_Insert_Disk->setText(tr("&Eject/Insert disk") + '\t' + ((QString)*sc));
		ui->menu_Disk_Side->setEnabled(false);
		ui->action_Eject_Insert_Disk->setEnabled(false);
	}

	sc = (QString *)settings_inp_rd_sc(SET_INP_SC_WAV, KEYBOARD);

	if (info.no_rom | info.turn_off) {
		ui->action_Start_Stop_WAV_recording->setEnabled(false);
		ui->action_Start_Stop_WAV_recording->setText(tr("Start/Stop &WAV recording") + '\t' + ((QString)*sc));
		ui->action_Start_Stop_WAV_recording->setIcon(QIcon(":/icon/icons/wav_start.png"));
	} else {
		ui->action_Start_Stop_WAV_recording->setEnabled(true);
		if (info.wave_in_record) {
			ui->action_Start_Stop_WAV_recording->setText(tr("Stop &WAV recording") + '\t' + ((QString)*sc));
			ui->action_Start_Stop_WAV_recording->setIcon(QIcon(":/icon/icons/wav_stop.png"));
		} else {
			ui->action_Start_Stop_WAV_recording->setText(tr("Start &WAV recording") + '\t' + ((QString)*sc));
			ui->action_Start_Stop_WAV_recording->setIcon(QIcon(":/icon/icons/wav_start.png"));
		}
	}

	if (info.pause_from_gui == TRUE) {
		ui->action_Pause->setChecked(true);
	} else {
		ui->action_Pause->setChecked(false);
	}

	if (fps.fast_forward == TRUE) {
		ui->action_Fast_Forward->setChecked(true);
	} else {
		ui->action_Fast_Forward->setChecked(false);
	}
}
void mainWindow::update_menu_settings() {
	// Mode
	if (cfg->mode == AUTO) {
		ui->action_Mode_Auto->setChecked(true);
	} else if (machine.type == PAL) {
		ui->action_PAL->setChecked(true);
	} else if (machine.type == NTSC) {
		ui->action_NTSC->setChecked(true);
	} else if (machine.type == DENDY) {
		ui->action_Dendy->setChecked(true);
	}
	// FPS
	switch (cfg->fps) {
		case 0:
			ui->action_FPS_Default->setChecked(true);
			break;
		case 1:
			ui->action_FPS_60->setChecked(true);
			break;
		case 2:
			ui->action_FPS_59->setChecked(true);
			break;
		case 3:
			ui->action_FPS_58->setChecked(true);
			break;
		case 4:
			ui->action_FPS_57->setChecked(true);
			break;
		case 5:
			ui->action_FPS_56->setChecked(true);
			break;
		case 6:
			ui->action_FPS_55->setChecked(true);
			break;
		case 7:
			ui->action_FPS_54->setChecked(true);
			break;
		case 8:
			ui->action_FPS_53->setChecked(true);
			break;
		case 9:
			ui->action_FPS_52->setChecked(true);
			break;
		case 10:
			ui->action_FPS_51->setChecked(true);
			break;
		case 11:
			ui->action_FPS_50->setChecked(true);
			break;
		case 12:
			ui->action_FPS_49->setChecked(true);
			break;
		case 13:
			ui->action_FPS_48->setChecked(true);
			break;
		case 14:
			ui->action_FPS_47->setChecked(true);
			break;
		case 15:
			ui->action_FPS_46->setChecked(true);
			break;
		case 16:
			ui->action_FPS_45->setChecked(true);
			break;
		case 17:
			ui->action_FPS_44->setChecked(true);
			break;
	}
	// Frame skip
	switch (cfg->frameskip) {
		case 0:
			ui->action_Fsk_Default->setChecked(true);
			break;
		case 1:
			ui->action_Fsk_1->setChecked(true);
			break;
		case 2:
			ui->action_Fsk_2->setChecked(true);
			break;
		case 3:
			ui->action_Fsk_3->setChecked(true);
			break;
		case 4:
			ui->action_Fsk_4->setChecked(true);
			break;
		case 5:
			ui->action_Fsk_5->setChecked(true);
			break;
		case 6:
			ui->action_Fsk_6->setChecked(true);
			break;
		case 7:
			ui->action_Fsk_7->setChecked(true);
			break;
		case 8:
			ui->action_Fsk_8->setChecked(true);
			break;
		case 9:
			ui->action_Fsk_9->setChecked(true);
			break;
	}
	// Scale
	if (cfg->fullscreen == NO_FULLSCR) {
		switch (cfg->scale) {
			case X1:
				ui->action_1x->setChecked(true);
				break;
			case X2:
				ui->action_2x->setChecked(true);
				break;
			case X3:
				ui->action_3x->setChecked(true);
				break;
			case X4:
				ui->action_4x->setChecked(true);
				break;
			case X5:
				ui->action_5x->setChecked(true);
				break;
			case X6:
				ui->action_6x->setChecked(true);
				break;
		}
	}
	// Settings/Video/Pixel Aspect Ratio
	switch (cfg->pixel_aspect_ratio) {
		case PAR11:
			ui->action_PAR_11->setChecked(true);
			break;
		case PAR54:
			ui->action_PAR_54->setChecked(true);
			break;
		case PAR87:
			ui->action_PAR_87->setChecked(true);
			break;
		case PAR118:
			ui->action_PAR_118->setChecked(true);
			break;
	}

	if (cfg->filter != NTSC_FILTER) {
		ui->menu_Pixel_Aspect_Ratio->setEnabled(true);
	} else {
		ui->menu_Pixel_Aspect_Ratio->setEnabled(false);
	}

	if (cfg->pixel_aspect_ratio != PAR11) {
		ui->action_PAR_Soft_Stretch->setEnabled(true);
		if (cfg->PAR_soft_stretch == TRUE) {
			ui->action_PAR_Soft_Stretch->setChecked(true);
		} else {
			ui->action_PAR_Soft_Stretch->setChecked(false);
		}
	} else {
		ui->action_PAR_Soft_Stretch->setEnabled(false);
	}
	// Settings/Video/Overscan
	ui->action_Oscan_Black_Borders_in_Window->setChecked(cfg->oscan_black_borders);
	switch (cfg->oscan) {
		case OSCAN_ON:
			ui->action_Oscan_On->setChecked(true);
			break;
		case OSCAN_OFF:
			ui->action_Oscan_Off->setChecked(true);
			break;
		case OSCAN_DEFAULT:
			ui->action_Oscan_Default->setChecked(true);
			break;
	}
	switch (cfg->oscan_default) {
		case OSCAN_ON:
			ui->action_Oscan_Def_On->setChecked(true);
			break;
		case OSCAN_OFF:
			ui->action_Oscan_Def_Off->setChecked(true);
			break;
	}
	// Settings/Video/Software Filters
	switch (cfg->filter) {
		case NO_FILTER:
			ui->action_No_Filter->setChecked(true);
			break;
		case SCALE2X:
			ui->action_Scale2X->setChecked(true);
			break;
		case SCALE3X:
			ui->action_Scale3X->setChecked(true);
			break;
		case SCALE4X:
			ui->action_Scale4X->setChecked(true);
			break;
		case HQ2X:
			ui->action_Hq2X->setChecked(true);
			break;
		case HQ3X:
			ui->action_Hq3X->setChecked(true);
			break;
		case HQ4X:
			ui->action_Hq4X->setChecked(true);
			break;
		case XBRZ2X:
			ui->action_xBRZ_2X->setChecked(true);
			break;
		case XBRZ3X:
			ui->action_xBRZ_3X->setChecked(true);
			break;
		case XBRZ4X:
			ui->action_xBRZ_4X->setChecked(true);
			break;
		case XBRZ5X:
			ui->action_xBRZ_5X->setChecked(true);
			break;
		case XBRZ6X:
			ui->action_xBRZ_6X->setChecked(true);
			break;
		case XBRZ2XMT:
			ui->action_xBRZ_2X_MT->setChecked(true);
			break;
		case XBRZ3XMT:
			ui->action_xBRZ_3X_MT->setChecked(true);
			break;
		case XBRZ4XMT:
			ui->action_xBRZ_4X_MT->setChecked(true);
			break;
		case XBRZ5XMT:
			ui->action_xBRZ_5X_MT->setChecked(true);
			break;
		case XBRZ6XMT:
			ui->action_xBRZ_6X_MT->setChecked(true);
			break;
		case NTSC_FILTER: {
			switch (cfg->ntsc_format) {
				case COMPOSITE:
					ui->action_NTSC_Composite->setChecked(true);
					break;
				case SVIDEO:
					ui->action_NTSC_SVideo->setChecked(true);
					break;
				case RGBMODE:
					ui->action_NTSC_RGB->setChecked(true);
					break;
			}
			break;
		}
	}
	// Settings/Video/Shaders
	if (ustrlen(cfg->shader_file) != 0) {
		ui->action_Shader_File->setText(QFileInfo(uQString(cfg->shader_file)).baseName());
		ui->action_Shader_File->setEnabled(true);
	} else {
		ui->action_Shader_File->setText(tr("[Select a file]"));
		ui->action_Shader_File->setEnabled(false);
	}
	switch (cfg->shader) {
		case NO_SHADER:
			ui->action_No_Shader->setChecked(true);
			break;
		case SHADER_CRTDOTMASK:
			ui->action_Shader_CRT_Dotmask->setChecked(true);
			break;
		case SHADER_CRTSCANLINES:
			ui->action_Shader_CRT_Scanlines->setChecked(true);
			break;
		case SHADER_CRTWITHCURVE:
			ui->action_Shader_CRT_With_Curve->setChecked(true);
			break;
		case SHADER_EMBOSS:
			ui->action_Shader_Emboss->setChecked(true);
			break;
		case SHADER_NOISE:
			ui->action_Shader_Noise->setChecked(true);
			break;
		case SHADER_NTSC2PHASECOMPOSITE:
			ui->action_Shader_NTSC_2Phase_Composite->setChecked(true);
			break;
		case SHADER_OLDTV:
			ui->action_Shader_Old_TV->setChecked(true);
			break;
		case SHADER_FILE:
			ui->action_Shader_File->setChecked(true);
			break;
	}
	// Settings/Video/Palette
	if (ustrlen(cfg->palette_file) != 0) {
		ui->action_Palette_File->setText(QFileInfo(uQString(cfg->palette_file)).baseName());
		ui->action_Palette_File->setEnabled(true);
	} else {
		ui->action_Palette_File->setText(tr("[Select a file]"));
		ui->action_Palette_File->setEnabled(false);
	}

	switch (cfg->palette) {
		case PALETTE_PAL:
			ui->action_Palette_PAL->setChecked(true);
			break;
		case PALETTE_NTSC:
			ui->action_Palette_NTSC->setChecked(true);
			break;
		case PALETTE_SONY:
			ui->action_Sony_CXA2025AS_US->setChecked(true);
			break;
		case PALETTE_FRBX_NOSTALGIA:
			ui->action_Firebrandx_Nostalgia->setChecked(true);
			break;
		case PALETTE_FRBX_YUV:
			ui->action_Firebrandx_YUV->setChecked(true);
			break;
		case PALETTE_MONO:
			ui->action_Monochrome->setChecked(true);
			break;
		case PALETTE_GREEN:
			ui->action_Green->setChecked(true);
			break;
		case PALETTE_FILE:
			ui->action_Palette_File->setChecked(true);
			break;
	}
	// Settings/Video/[VSync, Interpolation, Text on screen, Disable sRGB FBO]
	ui->action_Disable_emphasis_swap_PAL->setChecked(cfg->disable_swap_emphasis_pal);
	ui->action_VSync->setChecked(cfg->vsync);
	ui->action_Interpolation->setChecked(cfg->interpolation);
	ui->action_Text_on_screen->setChecked(cfg->txt_on_screen);
	ui->action_Input_display->setChecked(cfg->input_display);
	ui->action_Disable_TV_noise_emulation->setChecked(cfg->disable_tv_noise);
	ui->action_Disable_sepia_color_on_pause->setChecked(cfg->disable_sepia_color);
#if defined (WITH_OPENGL)
	ui->action_Disable_sRGB_FBO->setChecked(cfg->disable_srgb_fbo);
#else
	ui->action_Disable_sRGB_FBO->setVisible(false);
#endif
	ui->action_Fullscreen_in_window->setChecked(cfg->fullscreen_in_window);
	ui->action_Stretch_in_fullscreen->setChecked(cfg->stretch);
	// Settings/Audio/Buffer Size factor
	switch (cfg->audio_buffer_factor) {
		case 0:
			ui->action_Absf_0->setChecked(true);
			break;
		case 1:
			ui->action_Absf_1->setChecked(true);
			break;
		case 2:
			ui->action_Absf_2->setChecked(true);
			break;
		case 3:
			ui->action_Absf_3->setChecked(true);
			break;
		case 4:
			ui->action_Absf_4->setChecked(true);
			break;
		case 5:
			ui->action_Absf_5->setChecked(true);
			break;
		case 6:
			ui->action_Absf_6->setChecked(true);
			break;
		case 7:
			ui->action_Absf_7->setChecked(true);
			break;
		case 8:
			ui->action_Absf_8->setChecked(true);
			break;
		case 9:
			ui->action_Absf_9->setChecked(true);
			break;
	}
	// Settings/Audio/Samplerate
	switch (cfg->samplerate) {
		case S48000:
			ui->action_Sample_rate_48000->setChecked(true);
			break;
		case S44100:
			ui->action_Sample_rate_44100->setChecked(true);
			break;
		case S22050:
			ui->action_Sample_rate_22050->setChecked(true);
			break;
		case S11025:
			ui->action_Sample_rate_11025->setChecked(true);
			break;
	}
	// Settings/Audio/Channels
	switch (cfg->channels_mode) {
		case CH_MONO:
			ui->action_Mono->setChecked(true);
			ui->menu_Stereo_delay->setEnabled(false);
			break;
		case CH_STEREO_DELAY:
			ui->action_Stereo_Delay_Mode->setChecked(true);
			ui->menu_Stereo_delay->setEnabled(true);
			break;
		case CH_STEREO_PANNING:
			ui->action_Stereo_Panning_Mode->setChecked(true);
			ui->menu_Stereo_delay->setEnabled(false);
			break;
	}
	// Settings/Audio/Channels/Stereo delay
	{
		int delay = cfg->stereo_delay * 100;

		switch (delay) {
			case 5:
				ui->action_Stereo_delay_5->setChecked(true);
				break;
			case 10:
				ui->action_Stereo_delay_10->setChecked(true);
				break;
			case 15:
				ui->action_Stereo_delay_15->setChecked(true);
				break;
			case 20:
				ui->action_Stereo_delay_20->setChecked(true);
				break;
			case 25:
				ui->action_Stereo_delay_25->setChecked(true);
				break;
			case 30:
				ui->action_Stereo_delay_30->setChecked(true);
				break;
			case 35:
				ui->action_Stereo_delay_35->setChecked(true);
				break;
			case 40:
				ui->action_Stereo_delay_40->setChecked(true);
				break;
			case 45:
				ui->action_Stereo_delay_45->setChecked(true);
				break;
			case 50:
				ui->action_Stereo_delay_50->setChecked(true);
				break;
			case 55:
				ui->action_Stereo_delay_55->setChecked(true);
				break;
			case 60:
				ui->action_Stereo_delay_60->setChecked(true);
				break;
			case 65:
				ui->action_Stereo_delay_65->setChecked(true);
				break;
			case 70:
				ui->action_Stereo_delay_70->setChecked(true);
				break;
			case 75:
				ui->action_Stereo_delay_75->setChecked(true);
				break;
			case 80:
				ui->action_Stereo_delay_80->setChecked(true);
				break;
			case 85:
				ui->action_Stereo_delay_85->setChecked(true);
				break;
			case 90:
				ui->action_Stereo_delay_90->setChecked(true);
				break;
			case 95:
				ui->action_Stereo_delay_95->setChecked(true);
				break;
			case 100:
				ui->action_Stereo_delay_100->setChecked(true);
				break;
		}
	}
	// Settings/Audio/Quality
	switch (cfg->audio_quality) {
		case AQ_LOW:
			ui->action_Audio_Quality_Low->setChecked(true);
			break;
		case AQ_HIGH:
			ui->action_Audio_Quality_High->setChecked(true);
			break;
	}
	// Settings/Audio/[Swap Duty Cycle, Enable]
	ui->action_Swap_Duty_Cycles->setChecked(cfg->swap_duty);
	ui->action_Audio_Enable->setChecked(cfg->apu.channel[APU_MASTER]);
	// Settings/Cheats
	ui->action_Cheats_Editor->setEnabled(false);
	switch (cfg->cheat_mode) {
		case NOCHEAT_MODE:
			ui->action_Cheats_Disabled->setChecked(true);
			break;
		case GAMEGENIE_MODE:
			ui->action_Game_Genie->setChecked(true);
			break;
		case CHEATSLIST_MODE:
			ui->action_Cheats_List->setChecked(true);
			if (!info.no_rom) {
				ui->action_Cheats_Editor->setEnabled(true);
			}
			break;
	}
	// Settings/Fast Forward velocity
	switch (cfg->ff_velocity) {
		case FF_2X:
			ui->action_FF_2x->setChecked(true);
			break;
		case FF_3X:
			ui->action_FF_3x->setChecked(true);
			break;
		case FF_4X:
			ui->action_FF_4x->setChecked(true);
			break;
		case FF_5X:
			ui->action_FF_5x->setChecked(true);
			break;
	}

	// Settings/PPU
	ui->action_Hide_sprites->setChecked(cfg->hide_sprites);
	ui->action_Hide_background->setChecked(cfg->hide_background);
	ui->action_Unlimited_sprites->setChecked(cfg->unlimited_sprites);
	ui->action_PPU_Overclock->setChecked(cfg->ppu_overclock);

	// Settings/Language
	switch (cfg->language) {
		case LNG_ENGLISH:
			ui->action_English->setChecked(true);
			break;
		case LNG_ITALIAN:
			ui->action_Italian->setChecked(true);
			break;
		case LNG_RUSSIAN:
			ui->action_Russian->setChecked(true);
			break;
	}
	//Settings/[Disable new menu management, Save battery RAM file every 3 min.,
	//          Pause when in backgrounds, Save settings on exit]
	ui->action_Disable_new_menu_management->setChecked(cfg->disable_new_menu);
	ui->action_Save_battery_RAM_file_every_tot->setChecked(cfg->save_battery_ram_file);
	ui->action_Pause_when_in_background->setChecked(cfg->bck_pause);
	ui->action_Save_settings_on_exit->setChecked(cfg->save_on_exit);
}
void mainWindow::update_menu_tools() {
	ui->action_Vs_System->setChecked(ext_win.vs_system);
	ui->action_APU_channels->setChecked(ext_win.apu_channels);
}
void mainWindow::update_menu_state() {
	bool state = false;

	if (!(info.no_rom | info.turn_off)) {
		state = true;
	}

	ui->action_Save_state->setEnabled(state);
	ui->action_Load_state->setEnabled(state);
	ui->action_State_Save_to_file->setEnabled(state);
	ui->action_State_Load_from_file->setEnabled(state);

	switch (save_slot.slot) {
		case 0:
			ui->action_State_Slot_0->setChecked(true);
			break;
		case 1:
			ui->action_State_Slot_1->setChecked(true);
			break;
		case 2:
			ui->action_State_Slot_2->setChecked(true);
			break;
		case 3:
			ui->action_State_Slot_3->setChecked(true);
			break;
		case 4:
			ui->action_State_Slot_4->setChecked(true);
			break;
		case 5:
			ui->action_State_Slot_5->setChecked(true);
			break;
		case 6:
			ui->action_State_Slot_6->setChecked(true);
			break;
		case 7:
			ui->action_State_Slot_7->setChecked(true);
			break;
		case 8:
			ui->action_State_Slot_8->setChecked(true);
			break;
		case 9:
			ui->action_State_Slot_9->setChecked(true);
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
	if (side == fds.drive.side_inserted) {
		action->setChecked(true);
	}
}
void mainWindow::shortcuts() {
	// se non voglio che gli shortcut funzionino durante il fullscreen, basta
	// utilizzare lo shortcut associato al QAction. In questo modo quando nascondero'
	// la barra del menu, automaticamente questi saranno disabilitati.

	// File
	connect_shortcut(ui->action_Open, SET_INP_SC_OPEN, SLOT(s_open()));
	connect_shortcut(ui->action_Quit, SET_INP_SC_QUIT, SLOT(s_quit()));
	// NES
	connect_shortcut(ui->action_Turn_Off, SET_INP_SC_TURN_OFF, SLOT(s_turn_on_off()));
	connect_shortcut(ui->action_Hard_Reset, SET_INP_SC_HARD_RESET, SLOT(s_make_reset()));
	connect_shortcut(ui->action_Soft_Reset, SET_INP_SC_SOFT_RESET, SLOT(s_make_reset()));
	connect_shortcut(ui->action_Insert_Coin, SET_INP_SC_INSERT_COIN, SLOT(s_insert_coin()));
	connect_shortcut(ui->action_Switch_sides, SET_INP_SC_SWITCH_SIDES, SLOT(s_disk_side()));
	connect_shortcut(ui->action_Eject_Insert_Disk, SET_INP_SC_EJECT_DISK, SLOT(s_eject_disk()));
	connect_shortcut(ui->action_Start_Stop_WAV_recording, SET_INP_SC_WAV, SLOT(s_start_stop_wave()));
	connect_shortcut(ui->action_Fullscreen, SET_INP_SC_FULLSCREEN, SLOT(s_set_fullscreen()));
	connect_shortcut(ui->action_Pause, SET_INP_SC_PAUSE, SLOT(s_pause()));
	connect_shortcut(ui->action_Fast_Forward, SET_INP_SC_FAST_FORWARD, SLOT(s_fast_forward()));
	connect_shortcut(ui->action_Save_Screenshot, SET_INP_SC_SCREENSHOT, SLOT(s_save_screenshot()));
	// Settings/Mode
	connect_shortcut(ui->action_PAL, SET_INP_SC_MODE_PAL, SLOT(s_set_mode()));
	connect_shortcut(ui->action_NTSC, SET_INP_SC_MODE_NTSC, SLOT(s_set_mode()));
	connect_shortcut(ui->action_Dendy, SET_INP_SC_MODE_DENDY, SLOT(s_set_mode()));
	connect_shortcut(ui->action_Mode_Auto, SET_INP_SC_MODE_AUTO, SLOT(s_set_mode()));
	// Settings/Video/Scale
	connect_shortcut(ui->action_1x, SET_INP_SC_SCALE_1X);
	connect_shortcut(ui->action_2x, SET_INP_SC_SCALE_2X);
	connect_shortcut(ui->action_3x, SET_INP_SC_SCALE_3X);
	connect_shortcut(ui->action_4x, SET_INP_SC_SCALE_4X);
	connect_shortcut(ui->action_5x, SET_INP_SC_SCALE_5X);
	connect_shortcut(ui->action_6x, SET_INP_SC_SCALE_6X);
	// Settings/Video/[Interpolation, Stretch in fullscreen]
	connect_shortcut(ui->action_Interpolation, SET_INP_SC_INTERPOLATION,
			SLOT(s_set_interpolation()));
	connect_shortcut(ui->action_Stretch_in_fullscreen, SET_INP_SC_STRETCH_FULLSCREEN,
			SLOT(s_set_stretch()));
	// Settings/Audio/Enable
	connect_shortcut(ui->action_Audio_Enable, SET_INP_SC_AUDIO_ENABLE, SLOT(s_set_audio_enable()));
	// Settings/Save settings
	connect_shortcut(ui->action_Save_settings, SET_INP_SC_SAVE_SETTINGS, SLOT(s_save_settings()));
	// State/[Save state, Load state]
	connect_shortcut(ui->action_Save_state, SET_INP_SC_SAVE_STATE,
			SLOT(s_state_save_slot_action()));
	connect_shortcut(ui->action_Load_state, SET_INP_SC_LOAD_STATE,
			SLOT(s_state_save_slot_action()));
	// State/[Incremente slot, Decrement slot]
	connect_shortcut(ui->action_Increment_slot, SET_INP_SC_INC_SLOT,
			SLOT(s_state_save_slot_incdec()));
	connect_shortcut(ui->action_Decrement_slot, SET_INP_SC_DEC_SLOT,
			SLOT(s_state_save_slot_incdec()));
}
void mainWindow::shcjoy_start() {
	shcjoy_stop();

	if (js_is_null(&cfg->input.shcjoy_id)) {
		return;
	}

	for (int i = 0; i < SET_MAX_NUM_SC; i++) {
		shcjoy.shortcut[i] = name_to_jsv(uQStringCD(QString(*(QString *) settings_inp_rd_sc(i + SET_INP_SC_OPEN, JOYSTICK))));
	}

	js_shcut_init();

	shcjoy.enabled = true;
	shcjoy.timer->start(13);
}
void mainWindow::shcjoy_stop() {
	shcjoy.enabled = false;
	shcjoy.timer->stop();
	js_shcut_stop();
}
void mainWindow::control_visible_cursor() {
	if ((gmouse.hidden == FALSE) && (input_draw_target() == FALSE)) {
		if (cfg->fullscreen == FULLSCR) {
			gui_cursor_hide(TRUE);
		} else if ((gui_get_ms() - gmouse.timer) >= 2000) {
			gui_cursor_hide(TRUE);
		}
	}
}
void mainWindow::make_reset(int type) {
	if (type == HARD) {
		if ((cfg->cheat_mode == GAMEGENIE_MODE) && gamegenie.rom_present) {
			if (info.mapper.id != GAMEGENIE_MAPPER) {
				ustrncpy(info.load_rom_file, info.rom_file, usizeof(info.load_rom_file));
			}
			gamegenie_reset();
			type = CHANGE_ROM;
		} else {
			/*
			 * se e' stato disabilitato il game genie quando ormai
			 * e' gia' in esecuzione e si preme un reset, carico la rom.
			 */
			if (info.mapper.id == GAMEGENIE_MAPPER) {
				gamegenie_reset();
				type = CHANGE_ROM;
			}
		}
	}

	if (emu_reset(type)) {
		s_quit();
	}

	/*
	 * dopo un reset la pause e' automaticamente disabilitata quindi faccio
	 * un aggiornamento del submenu NES per avere la voce correttamente settata.
	 */
	update_menu_nes();
}
void mainWindow::connect_shortcut(QAction *action, int index) {
	QString *sc = (QString *)settings_inp_rd_sc(index, KEYBOARD);

	if (sc->isEmpty() == false) {
		action->setShortcut(QKeySequence((QString)(*sc)));
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
void mainWindow::connect_menu_signals() {
	// File
	connect_action(ui->action_Open, SLOT(s_open()));
	connect_action(ui->action_Open_working_folder, SLOT(s_open_working_folder()));
	connect_action(ui->action_Quit, SLOT(s_quit()));
	// NES
	connect_action(ui->action_Turn_Off, SLOT(s_turn_on_off()));
	connect_action(ui->action_Hard_Reset, HARD, SLOT(s_make_reset()));
	connect_action(ui->action_Soft_Reset, RESET, SLOT(s_make_reset()));
	connect_action(ui->action_Insert_Coin, SLOT(s_insert_coin()));
	connect_action(ui->action_Disk_1_side_A, 0, SLOT(s_disk_side()));
	connect_action(ui->action_Disk_1_side_B, 1, SLOT(s_disk_side()));
	connect_action(ui->action_Disk_2_side_A, 2, SLOT(s_disk_side()));
	connect_action(ui->action_Disk_2_side_B, 3, SLOT(s_disk_side()));
	connect_action(ui->action_Disk_3_side_A, 4, SLOT(s_disk_side()));
	connect_action(ui->action_Disk_3_side_B, 5, SLOT(s_disk_side()));
	connect_action(ui->action_Disk_4_side_A, 6, SLOT(s_disk_side()));
	connect_action(ui->action_Disk_4_side_B, 7, SLOT(s_disk_side()));
	connect_action(ui->action_Switch_sides, 0xFFF, SLOT(s_disk_side()));
	connect_action(ui->action_Eject_Insert_Disk, SLOT(s_eject_disk()));
	connect_action(ui->action_Start_Stop_WAV_recording, SLOT(s_start_stop_wave()));
	connect_action(ui->action_Fullscreen, SLOT(s_set_fullscreen()));
	connect_action(ui->action_Pause, SLOT(s_pause()));
	connect_action(ui->action_Fast_Forward, SLOT(s_fast_forward()));
	connect_action(ui->action_Save_Screenshot, SLOT(s_save_screenshot()));

	// Tools
	connect_action(ui->action_Vs_System, SLOT(s_set_vs_window()));
	connect_action(ui->action_APU_channels, SLOT(s_set_apu_channels()));
	connect_action(ui->action_PPU_Hacks, SLOT(s_set_ppu_hacks()));

	// Settings/Mode
	connect_action(ui->action_PAL, PAL, SLOT(s_set_mode()));
	connect_action(ui->action_NTSC, NTSC, SLOT(s_set_mode()));
	connect_action(ui->action_Dendy, DENDY, SLOT(s_set_mode()));
	connect_action(ui->action_Mode_Auto, AUTO, SLOT(s_set_mode()));
	// Settings/Video/FPS
	connect_action(ui->action_FPS_Default, FPS_DEFAULT, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_60, FPS_60, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_59, FPS_59, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_58, FPS_58, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_57, FPS_57, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_56, FPS_56, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_55, FPS_55, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_54, FPS_54, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_53, FPS_53, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_52, FPS_52, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_51, FPS_51, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_50, FPS_50, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_49, FPS_49, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_48, FPS_48, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_47, FPS_47, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_46, FPS_46, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_45, FPS_45, SLOT(s_set_fps()));
	connect_action(ui->action_FPS_44, FPS_44, SLOT(s_set_fps()));
	// Settings/Video/Frame skip
	connect_action(ui->action_Fsk_Default, 0, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_1, 1, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_2, 2, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_3, 3, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_4, 4, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_5, 5, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_6, 6, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_7, 7, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_8, 8, SLOT(s_set_fsk()));
	connect_action(ui->action_Fsk_9, 9, SLOT(s_set_fsk()));
	// Settings/Video/Scale
	connect_action(ui->action_1x, X1, SLOT(s_set_scale()));
	connect_action(ui->action_2x, X2, SLOT(s_set_scale()));
	connect_action(ui->action_3x, X3, SLOT(s_set_scale()));
	connect_action(ui->action_4x, X4, SLOT(s_set_scale()));
	connect_action(ui->action_5x, X5, SLOT(s_set_scale()));
	connect_action(ui->action_6x, X6, SLOT(s_set_scale()));
	// Settings/Video/Pixel Aspect Ratio
	connect_action(ui->action_PAR_11, PAR11, SLOT(s_set_par()));
	connect_action(ui->action_PAR_54, PAR54, SLOT(s_set_par()));
	connect_action(ui->action_PAR_87, PAR87, SLOT(s_set_par()));
	connect_action(ui->action_PAR_118, PAR118, SLOT(s_set_par()));
	connect_action(ui->action_PAR_Soft_Stretch, SLOT(s_set_par_stretch()));
	// Settings/Video/Overscan
	connect_action(ui->action_Oscan_Black_Borders_in_Window, SLOT(s_set_overscan_black_borders()));
	connect_action(ui->action_Oscan_Default, OSCAN_DEFAULT, SLOT(s_set_overscan()));
	connect_action(ui->action_Oscan_On, OSCAN_ON, SLOT(s_set_overscan()));
	connect_action(ui->action_Oscan_Off, OSCAN_OFF, SLOT(s_set_overscan()));
	// Settings/Video/Overscan/Default
	connect_action(ui->action_Oscan_Def_On, OSCAN_DEFAULT_ON, SLOT(s_set_overscan()));
	connect_action(ui->action_Oscan_Def_Off, OSCAN_DEFAULT_OFF, SLOT(s_set_overscan()));
	connect_action(ui->action_Oscan_Set_Borders, SLOT(s_set_overscan_borders()));
	// Settings/Video/Software Filters
	connect_action(ui->action_No_Filter, NO_FILTER, SLOT(s_set_filter()));
	connect_action(ui->action_Scale2X, SCALE2X, SLOT(s_set_filter()));
	connect_action(ui->action_Scale3X, SCALE3X, SLOT(s_set_filter()));
	connect_action(ui->action_Scale4X, SCALE4X, SLOT(s_set_filter()));
	connect_action(ui->action_Hq2X, HQ2X, SLOT(s_set_filter()));
	connect_action(ui->action_Hq3X, HQ3X, SLOT(s_set_filter()));
	connect_action(ui->action_Hq4X, HQ4X, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_2X, XBRZ2X, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_3X, XBRZ3X, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_4X, XBRZ4X, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_5X, XBRZ5X, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_6X, XBRZ6X, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_2X_MT, XBRZ2XMT, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_3X_MT, XBRZ3XMT, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_4X_MT, XBRZ4XMT, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_5X_MT, XBRZ5XMT, SLOT(s_set_filter()));
	connect_action(ui->action_xBRZ_6X_MT, XBRZ6XMT, SLOT(s_set_filter()));
	connect_action(ui->action_NTSC_Composite, COMPOSITE, SLOT(s_set_ntsc_filter()));
	connect_action(ui->action_NTSC_SVideo, SVIDEO, SLOT(s_set_ntsc_filter()));
	connect_action(ui->action_NTSC_RGB, RGBMODE, SLOT(s_set_ntsc_filter()));
	// Settings/Video/Shaders
	connect_action(ui->action_No_Shader, NO_SHADER, SLOT(s_set_shader()));
	connect_action(ui->action_Shader_CRT_Dotmask, SHADER_CRTDOTMASK, SLOT(s_set_shader()));
	connect_action(ui->action_Shader_CRT_Scanlines, SHADER_CRTSCANLINES, SLOT(s_set_shader()));
	connect_action(ui->action_Shader_CRT_With_Curve, SHADER_CRTWITHCURVE, SLOT(s_set_shader()));
	connect_action(ui->action_Shader_Emboss, SHADER_EMBOSS, SLOT(s_set_shader()));
	connect_action(ui->action_Shader_Noise, SHADER_NOISE, SLOT(s_set_shader()));
	connect_action(ui->action_Shader_NTSC_2Phase_Composite, SHADER_NTSC2PHASECOMPOSITE,
			SLOT(s_set_shader()));
	connect_action(ui->action_Shader_Old_TV, SHADER_OLDTV, SLOT(s_set_shader()));
	connect_action(ui->action_Shader_File, SHADER_FILE, SLOT(s_set_shader()));
	connect_action(ui->action_Shader_Load_File, SLOT(s_load_shader()));
	// Settings/Video/Palette
	connect_action(ui->action_Palette_PAL, PALETTE_PAL, SLOT(s_set_palette()));
	connect_action(ui->action_Palette_NTSC, PALETTE_NTSC, SLOT(s_set_palette()));
	connect_action(ui->action_Sony_CXA2025AS_US, PALETTE_SONY, SLOT(s_set_palette()));
	connect_action(ui->action_Firebrandx_Nostalgia, PALETTE_FRBX_NOSTALGIA, SLOT(s_set_palette()));
	connect_action(ui->action_Firebrandx_YUV, PALETTE_FRBX_YUV, SLOT(s_set_palette()));
	connect_action(ui->action_Monochrome, PALETTE_MONO, SLOT(s_set_palette()));
	connect_action(ui->action_Green, PALETTE_GREEN, SLOT(s_set_palette()));
	connect_action(ui->action_Palette_File, PALETTE_FILE, SLOT(s_set_palette()));
	connect_action(ui->action_Palette_Save_File, SLOT(s_save_palette()));
	connect_action(ui->action_Palette_Load_File, SLOT(s_load_palette()));
	// Settings/Video/[VSync, Interpolation, Text on screen, Fullscreen_in_window,
	//                 Stretch in fullscreen]
	connect_action(ui->action_Disable_emphasis_swap_PAL, SLOT(s_set_disable_emphasis_pal()));
	connect_action(ui->action_VSync, SLOT(s_set_vsync()));
	connect_action(ui->action_Interpolation, SLOT(s_set_interpolation()));
	connect_action(ui->action_Text_on_screen, SLOT(s_set_txt_on_screen()));
	connect_action(ui->action_Input_display, SLOT(s_set_input_display()));
	connect_action(ui->action_Disable_TV_noise_emulation, SLOT(s_set_disable_tv_noise()));
	connect_action(ui->action_Disable_sepia_color_on_pause, SLOT(s_set_disable_sepia_pause()));
#if defined (WITH_OPENGL)
	connect_action(ui->action_Disable_sRGB_FBO, SLOT(s_set_disable_srgb_fbo()));
#endif
	connect_action(ui->action_Fullscreen_in_window, SLOT(s_set_fullscreen_in_window()));
	connect_action(ui->action_Stretch_in_fullscreen, SLOT(s_set_stretch()));
	// Settings/Audio/Output Device
	connect(ui->menu_Output_Device, SIGNAL(aboutToShow()), this, SLOT(s_update_output_devices()));
	// Settings/Audio/Buffer Size factor
	connect_action(ui->action_Absf_0, 0, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_1, 1, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_2, 2, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_3, 3, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_4, 4, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_5, 5, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_6, 6, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_7, 7, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_8, 8, SLOT(s_set_audio_buffer_factor()));
	connect_action(ui->action_Absf_9, 9, SLOT(s_set_audio_buffer_factor()));
	// Settings/Audio/Samplerate
	connect_action(ui->action_Sample_rate_48000, S48000, SLOT(s_set_samplerate()));
	connect_action(ui->action_Sample_rate_44100, S44100, SLOT(s_set_samplerate()));
	connect_action(ui->action_Sample_rate_22050, S22050, SLOT(s_set_samplerate()));
	connect_action(ui->action_Sample_rate_11025, S11025, SLOT(s_set_samplerate()));
	// Settings/Audio/Channels
	connect_action(ui->action_Mono, CH_MONO, SLOT(s_set_channels()));
	connect_action(ui->action_Stereo_Panning_Mode, CH_STEREO_PANNING, SLOT(s_set_channels()));
	connect_action(ui->action_Stereo_Delay_Mode, CH_STEREO_DELAY, SLOT(s_set_channels()));
	// Settings/Audio/Channels/Stereo delay
	connect_action(ui->action_Stereo_delay_5, 5, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_10, 10, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_15, 15, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_20, 20, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_25, 25, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_30, 30, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_35, 35, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_40, 40, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_45, 45, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_50, 50, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_55, 55, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_60, 60, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_65, 65, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_70, 70, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_75, 75, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_80, 80, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_85, 85, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_90, 90, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_95, 95, SLOT(s_set_stereo_delay()));
	connect_action(ui->action_Stereo_delay_100, 100, SLOT(s_set_stereo_delay()));
	// Settings/Audio/Quality
	connect_action(ui->action_Audio_Quality_Low, AQ_LOW, SLOT(s_set_audio_quality()));
	connect_action(ui->action_Audio_Quality_High, AQ_HIGH, SLOT(s_set_audio_quality()));
	// Settings/Audio/[APU_channels, Swap Duty Cycles, enable]
	connect_action(ui->action_Swap_Duty_Cycles, SLOT(s_set_audio_swap_duty()));
	connect_action(ui->action_Audio_Enable, SLOT(s_set_audio_enable()));
	// Settings/Input/Config
	connect_action(ui->action_Input_Config, SLOT(s_set_input()));
	// Settings/Cheats
	connect_action(ui->action_Cheats_Disabled, NOCHEAT_MODE, SLOT(s_cheat_mode_select()));
	connect_action(ui->action_Game_Genie, GAMEGENIE_MODE, SLOT(s_cheat_mode_select()));
	connect_action(ui->action_Cheats_List, CHEATSLIST_MODE, SLOT(s_cheat_mode_select()));
	connect_action(ui->action_Cheats_Editor, SLOT(s_cheat_dialog()));
	// Settings/Fast Forward velocity
	connect_action(ui->action_FF_2x, FF_2X, SLOT(s_set_ff_velocity()));
	connect_action(ui->action_FF_3x, FF_3X, SLOT(s_set_ff_velocity()));
	connect_action(ui->action_FF_4x, FF_4X, SLOT(s_set_ff_velocity()));
	connect_action(ui->action_FF_5x, FF_5X, SLOT(s_set_ff_velocity()));
	// Settings/PPU
	connect_action(ui->action_Hide_sprites, SLOT(s_set_hide_sprites()));
	connect_action(ui->action_Hide_background, SLOT(s_set_hide_background()));
	connect_action(ui->action_Unlimited_sprites, SLOT(s_set_unlimited_sprites()));
	connect_action(ui->action_PPU_Overclock, SLOT(s_set_ppu_overclock()));
	// Settings/Langauge
	connect_action(ui->action_English, LNG_ENGLISH, SLOT(s_set_language()));
	connect_action(ui->action_Italian, LNG_ITALIAN, SLOT(s_set_language()));
	connect_action(ui->action_Russian, LNG_RUSSIAN, SLOT(s_set_language()));
	// Settings/[Disable new menu management, Save battery RAM file every 3 min.,
	//           Pause when in backgrounds, Save settings on exit, Save settings]
	connect_action(ui->action_Disable_new_menu_management, SLOT(s_set_disable_new_menu()));
	connect_action(ui->action_Save_battery_RAM_file_every_tot,
			SLOT(s_set_save_battery_ram_file()));
	connect_action(ui->action_Pause_when_in_background, SLOT(s_set_pause_in_background()));
	connect_action(ui->action_Save_settings_on_exit, SLOT(s_set_save_on_exit()));
	connect_action(ui->action_Save_settings, SLOT(s_save_settings()));
	// State/[Save state, Load State]
	connect_action(ui->action_Save_state, SAVE, SLOT(s_state_save_slot_action()));
	connect_action(ui->action_Load_state, LOAD, SLOT(s_state_save_slot_action()));
	// State/[Increment slot, Decrement slot]
	connect_action(ui->action_Increment_slot, INC, SLOT(s_state_save_slot_incdec()));
	connect_action(ui->action_Decrement_slot, DEC, SLOT(s_state_save_slot_incdec()));
	// State/[State slot 0....]
	connect_action(ui->action_State_Slot_0, 0, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_1, 1, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_2, 2, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_3, 3, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_4, 4, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_5, 5, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_6, 6, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_7, 7, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_8, 8, SLOT(s_state_save_slot_set()));
	connect_action(ui->action_State_Slot_9, 9, SLOT(s_state_save_slot_set()));
	// State/[Save to file, Load from file]
	connect_action(ui->action_State_Save_to_file, SLOT(s_state_save_file()));
	connect_action(ui->action_State_Load_from_file, SLOT(s_state_load_file()));
	// Help/About
	connect_action(ui->action_About, SLOT(s_help()));
}
void mainWindow::connect_action(QAction *action, const char *member) {
	connect(action, SIGNAL(triggered()), this, member);
}
void mainWindow::connect_action(QAction *action, int value, const char *member) {
	action->setProperty("myValue", QVariant(value));
	connect_action(action, member);
}
void mainWindow::switch_filter(int filter) {
#if defined (WITH_OPENGL) && defined (__WIN32__)
	gfx_sdlwe_set(SDLWIN_FILTER, filter);
#else
	gfx_FILTER(filter);
#endif
}
void mainWindow::switch_shader(int shader) {
#if defined (WITH_OPENGL) && defined (__WIN32__)
	gfx_sdlwe_set(SDLWIN_SHADER, shader);
#else
	gfx_SHADER(shader);
#endif
}
void mainWindow::s_set_fullscreen() {
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
			        - ui->menubar->sizeHint().height() - 2 - statusbar->sizeHint().height();

			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, FULLSCR, NO_CHANGE, FALSE, FALSE);
			move(QPoint(0, 0));
		} else {
			gfx.type_of_fscreen_in_use = FULLSCR;

			gfx.w[MONITOR] = qApp->desktop()->screenGeometry(screenNumber).width();
			gfx.h[MONITOR] = qApp->desktop()->screenGeometry(screenNumber).height();
	#if defined (WITH_OPENGL) && defined (__WIN32__)
			// su alcuni windows, se setto il gfx.w[MONITOR] alla dimensione
			// del desktop, l'immagine a video ha dei glitch grafici marcati
			gfx.w[MONITOR]--;
	#endif

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
void mainWindow::s_fullscreen(bool state) {
	if (state == true) {
		showFullScreen();
	} else {
		showNormal();
	}
}
void mainWindow::s_loop() {
	emu_frame();
}
void mainWindow::s_open() {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("All supported formats"));
	filters.append(tr("Compressed files"));
	filters.append(tr("Nes rom files"));
	filters.append(tr("UNIF rom files"));
	filters.append(tr("FDS image files"));
	filters.append(tr("TAS movie files"));
	filters.append(tr("All files"));

	/* potrei essere entrato con il CTRL+O */
	tl.key = FALSE;

	if (l7z_present() == TRUE) {
		if ((l7z_control_ext(uL("rar")) == EXIT_OK)) {
			filters[0].append(
				" (*.zip *.ZIP *.7z *.7Z *.rar *.RAR *.nes *.NES *.unf *.UNF *.unif *.UNIF *.fds *.FDS *.fm2 *.FM2)");
			filters[1].append(" (*.zip *.ZIP *.7z *.7Z *.rar *.RAR)");
		} else {
			filters[0].append(" (*.zip *.ZIP *.7z *.7Z *.nes *.NES *.fds *.FDS *.fm2 *.FM2)");
			filters[1].append(" (*.zip *.ZIP *.7z *.7Z)");
		}
	} else {
		filters[0].append(" (*.zip *.ZIP *.nes *.NES *.unf *.UNF *.unif *.UNIF *.fds *.FDS *.fm2 *.FM2)");
		filters[1].append(" (*.zip *.ZIP)");
	}

	filters[2].append(" (*.nes *.NES)");
	filters[3].append(" (*.unf *.UNF *.unif *.UNIF)");
	filters[4].append(" (*.fds *.FDS)");
	filters[5].append(" (*.fm2 *.FM2)");
	filters[6].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open File"), uQString(gui.last_open_path),
		filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		change_rom(uQStringCD(fileinfo.absoluteFilePath()));

		ustrncpy(gui.last_open_path, uQStringCD(fileinfo.absolutePath()),
				usizeof(gui.last_open_path));
	}

	emu_pause(FALSE);
}
void mainWindow::s_open_recent_roms() {
	int index = QVariant(((QObject *)sender())->property("myValue")).toInt();
	QString current = QString((const QChar *) recent_roms_current(), recent_roms_current_size());
	QString item = QString((const QChar *) recent_roms_item(index), recent_roms_item_size(index));

	emu_pause(TRUE);

	if (current != item) {
		change_rom(uQStringCD(item));
	} else {
		/* se l'archivio e' compresso e contiene piu' di una rom allora lo carico */
		if ((info.uncompress_rom == TRUE) && (uncomp.files_founded > 1)) {
			change_rom(uQStringCD(item));
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_open_working_folder() {
	QDesktopServices::openUrl(QUrl(uQString(info.base_folder)));
}
void mainWindow::s_quit() {
	close();
}
void mainWindow::s_turn_on_off() {
	info.turn_off = !info.turn_off;
	info.pause_frames_drawscreen = 0;

	if (!info.turn_off) {
#if defined (WITH_OPENGL) && defined (__WIN32__)
		gfx_sdlwe_set(SDLWIN_MAKE_RESET, HARD);
#else
		gfx_MAKE_RESET(HARD);
#endif
	}

	update_menu_nes();
	update_menu_state();
}
void mainWindow::s_make_reset() {
	int type = QVariant(((QObject *)sender())->property("myValue")).toInt();

#if defined (WITH_OPENGL) && defined (__WIN32__)
	gfx_sdlwe_set(SDLWIN_MAKE_RESET, type);
#else
	gfx_MAKE_RESET(type);
#endif
}
void mainWindow::s_insert_coin() {
	gui_vs_system_insert_coin();
}
void mainWindow::s_disk_side() {
	int side = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (side == 0xFFF) {
		side = fds.drive.side_inserted;
		if (++side >= fds.info.total_sides) {
			side = 0;
		}
	}

	if (fds.drive.side_inserted == side) {
		return;
	}

	fds_disk_op(FDS_DISK_SELECT, side);

	update_menu_nes();
}
void mainWindow::s_eject_disk() {
	if (!fds.drive.disk_ejected) {
		fds_disk_op(FDS_DISK_EJECT, 0);
	} else {
		fds_disk_op(FDS_DISK_INSERT, 0);
	}

	update_menu_nes();
}
void mainWindow::s_start_stop_wave() {
	if (info.wave_in_record == FALSE) {
		QStringList filters;
		QString file;

		emu_pause(TRUE);

		filters.append(tr("MS WAVE files"));
		filters.append(tr("All files"));

		filters[0].append(" (*.wav *.WAV)");
		filters[1].append(" (*.*)");

		file = QFileDialog::getSaveFileName(this, tr("Record sound"),
				QFileInfo(uQString(info.rom_file)).completeBaseName(),
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
void mainWindow::s_pause() {
	info.pause_from_gui = !info.pause_from_gui;
	info.pause_frames_drawscreen = 0;

	emu_pause(info.pause_from_gui);
	update_menu_nes();
}
void mainWindow::s_fast_forward() {
	if (fps.fast_forward == FALSE) {
		fps_fast_forward();
	} else {
		fps_normalize();
	}
	update_menu_nes();
}
void mainWindow::s_save_screenshot() {
	gfx.save_screenshot = true;
}
void mainWindow::s_set_vs_window() {
	ext_win.vs_system = !ext_win.vs_system;
	gui_external_control_windows_show();
}
void mainWindow::s_update_output_devices() {
	QActionGroup *grp = new QActionGroup(this);
	QString id = uQString(cfg->audio_output);
	int i;

	grp->setExclusive(true);

	ui->menu_Output_Device->clear();

	snd_list_devices();

	for (i = 0; i < snd_list.playback.count; i++) {
		QString description = uQString(snd_playback_device_desc(i));
		QString id_new = uQString(snd_playback_device_id(i));
		QAction *action = new QAction(this);

		if (i == 0) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
			description = QApplication::translate("mainWindow", "System Default", 0,
					QApplication::UnicodeUTF8);
#else
			description = QApplication::translate("mainWindow", "System Default", Q_NULLPTR);
#endif
		} else if (i == 1) {
			ui->menu_Output_Device->addSeparator();
		}

		if (description.isEmpty()) {
			break;
		}

		action->setCheckable(true);
		action->setText(description);
		action->setProperty("myValue", QVariant(i));
		action->setProperty("myId", QVariant(id_new));

		grp->addAction(action);

		ui->menu_Output_Device->addAction(action);
		connect(action, SIGNAL(triggered()), this, SLOT(s_set_output_device()));

		if (id == id_new) {
			action->setChecked(true);
		}
	}
}
void mainWindow::s_set_output_device() {
	int index = QVariant(((QObject *)sender())->property("myValue")).toInt();
	QString id_new = QVariant(((QObject *)sender())->property("myId")).toString();
	QString id = uQString(cfg->audio_output);

	if (id != id_new) {
		ustrncpy(cfg->audio_output, (uTCHAR *) snd_list.playback.devices[index].id,
				usizeof(cfg->audio_output));
		emu_pause(TRUE);
		snd_playback_start();
		emu_pause(FALSE);
	}
}
void mainWindow::s_set_apu_channels() {
	ext_win.apu_channels = !ext_win.apu_channels;
	gui_external_control_windows_show();
}
void mainWindow::s_set_ppu_hacks() {
	ext_win.ppu_hacks = !ext_win.ppu_hacks;
	gui_external_control_windows_show();
}
void mainWindow::s_set_mode() {
	int mode = QVariant(((QObject *)sender())->property("myValue")).toInt();
	bool reset = true;

	if (mode == cfg->mode) {
		return;
	}

	cfg->mode = mode;

	if (cfg->mode == AUTO) {
		if (info.no_rom) {
			mode = NTSC;
		} else {
			switch (info.machine[DATABASE]) {
				case NTSC:
				case PAL:
				case DENDY:
					mode = info.machine[DATABASE];
					break;
				case DEFAULT:
					mode = info.machine[HEADER];
					break;
				default:
					mode = NTSC;
					break;
			}
		}
	}

	/*
	 * se la nuova modalita' e' identica a quella attuale
	 * non e' necessario fare un reset.
	 */
	if (mode == machine.type) {
		reset = FALSE;
	}

	machine = machinedb[mode - 1];

	if (reset) {
		QString ascii = uQString(opt_mode[machine.type].lname);

		text_add_line_info(1, "switched to [green]%s", ascii.toLatin1().constData());
#if defined (WITH_OPENGL) && defined (__WIN32__)
		gfx_sdlwe_set(SDLWIN_SWITCH_MODE, SDLWIN_NONE);
#else
		gfx_SWITCH_MODE();
#endif
	}
}
void mainWindow::s_set_fps() {
	int fps = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->fps == fps) {
		return;
	}
	cfg->fps = fps;
	emu_pause(TRUE);
	fps_init();
	snd_playback_start();
	emu_pause(FALSE);
}
void mainWindow::s_set_fsk() {
	int fsk = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->frameskip == fsk) {
		return;
	}

	cfg->frameskip = fsk;

	if (!fps.fast_forward) {
		fps_normalize();
	}
}
void mainWindow::s_set_scale() {
	int scale = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->fullscreen) {
		return;
	}

#if defined (WITH_OPENGL) && defined (__WIN32__)
	gfx_sdlwe_set(SDLWIN_SCALE, scale);
#else
	gfx_SCALE(scale);
#endif
}
void mainWindow::s_set_par() {
	int par = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->pixel_aspect_ratio == par) {
		return;
	}

	cfg->pixel_aspect_ratio = par;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
void mainWindow::s_set_par_stretch() {
	cfg->PAR_soft_stretch = !cfg->PAR_soft_stretch;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
void mainWindow::s_set_overscan_black_borders() {
	cfg->oscan_black_borders = !cfg->oscan_black_borders;
	if (overscan.enabled) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
}
void mainWindow::s_set_overscan() {
	int oscan = QVariant(((QObject *)sender())->property("myValue")).toInt();

	switch (oscan) {
		case OSCAN_ON:
		case OSCAN_OFF:
		case OSCAN_DEFAULT:
			cfg->oscan = oscan;
			settings_pgs_save();
			break;
		case OSCAN_DEFAULT_OFF:
			cfg->oscan_default = OSCAN_OFF;
			break;
		case OSCAN_DEFAULT_ON:
			cfg->oscan_default = OSCAN_ON;
			break;
	}

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
void mainWindow::s_set_overscan_borders() {
	dlgOverscanBorders *dlg = new dlgOverscanBorders(this);

	dlg->show();
}
void mainWindow::s_set_filter() {
	int filter = QVariant(((QObject *)sender())->property("myValue")).toInt();

	switch_filter(filter);
}

void mainWindow::s_set_ntsc_filter() {
	int filter = QVariant(((QObject *)sender())->property("myValue")).toInt();

	cfg->ntsc_format = filter;
	switch_filter(NTSC_FILTER);
}
void mainWindow::s_set_shader() {
	int shader = QVariant(((QObject *)sender())->property("myValue")).toInt();

	switch_shader(shader);
}
void mainWindow::s_load_shader() {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("Shaders files"));
	filters.append(tr("All files"));

#if defined (WITH_OPENGL_CG)
	filters[0].append(" (*.cgp *.glslp)");
#elif defined (WITH_OPENGL)
	filters[0].append(" (*.glslp)");
#elif defined (WITH_D3D9)
	filters[0].append(" (*.cgp)");
#endif
	filters[1].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open Shader file"),
			QFileInfo(uQString(cfg->shader_file)).dir().absolutePath(), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.exists()) {
			umemset(cfg->shader_file, 0x00, usizeof(cfg->shader_file));
			ustrncpy(cfg->shader_file, uQStringCD(fileinfo.absoluteFilePath()),
					usizeof(cfg->shader_file) - 1);
			switch_shader(SHADER_FILE);
		} else {
			text_add_line_info(1, "[red]error on shader file");
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_set_palette() {
	int palette = QVariant(((QObject *)sender())->property("myValue")).toInt();

	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, palette, FALSE, FALSE);
}
void mainWindow::s_set_disable_emphasis_pal() {
	cfg->disable_swap_emphasis_pal = !cfg->disable_swap_emphasis_pal;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, TRUE);
}
void mainWindow::s_save_palette() {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("Palette files"));
	filters.append(tr("All files"));

	filters[0].append(" (*.pal *.PAL)");
	filters[1].append(" (*.*)");

	file = QFileDialog::getSaveFileName(this, tr("Save palette on file"),
			uQString(opt_palette[cfg->palette].lname).replace(" ", "_"),
			filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(QString(file) + ".pal");
		}

		palette_save_on_file(uQStringCD(fileinfo.absoluteFilePath()));
	}

	emu_pause(FALSE);
}
void mainWindow::s_load_palette() {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("Palette files"));
	filters.append(tr("All files"));

	filters[0].append(" (*.pal *.PAL)");
	filters[1].append(" (*.*)");

	file = QFileDialog::getOpenFileName(this, tr("Open palette file"),
			QFileInfo(uQString(cfg->palette_file)).dir().absolutePath(), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.exists()) {
			umemset(cfg->palette_file, 0x00, usizeof(cfg->palette_file));
			ustrncpy(cfg->palette_file, uQStringCD(fileinfo.absoluteFilePath()),
					usizeof(cfg->palette_file) - 1);
			gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, PALETTE_FILE, FALSE, TRUE);
		} else {
			text_add_line_info(1, "[red]error on palette file");
		}
	}

	emu_pause(FALSE);
}
void mainWindow::s_set_vsync() {
	cfg->vsync = !cfg->vsync;

#if defined (WITH_OPENGL) && defined (__WIN32__)
	gfx_sdlwe_set(SDLWIN_VSYNC, SDLWIN_NONE);
#else
	gfx_VSYNC();
#endif
}
void mainWindow::s_set_interpolation() {
	cfg->interpolation = !cfg->interpolation;
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
}
void mainWindow::s_set_txt_on_screen() {
	cfg->txt_on_screen = !cfg->txt_on_screen;
}
void mainWindow::s_set_input_display() {
	cfg->input_display = !cfg->input_display;

	if (!cfg->input_display) {
		BYTE i;

		for (i = 0; i < PORT_MAX; i++) {
			_txt_element *ele = &text.tas.controllers[i];

			if (ele->surface) {
				text_clear(ele);
				gfx_text_release_surface (ele);
			}
		}
	}
}
void mainWindow::s_set_disable_tv_noise() {
	cfg->disable_tv_noise = !cfg->disable_tv_noise;
}
void mainWindow::s_set_disable_sepia_pause() {
	cfg->disable_sepia_color = !cfg->disable_sepia_color;
}
#if defined (WITH_OPENGL)
void mainWindow::s_set_disable_srgb_fbo() {
	cfg->disable_srgb_fbo = !cfg->disable_srgb_fbo;

	if (info.sRGB_FBO_in_use == TRUE) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	}
}
#endif
void mainWindow::s_set_fullscreen_in_window() {
	cfg->fullscreen_in_window = !cfg->fullscreen_in_window;
}
void mainWindow::s_set_stretch() {
	cfg->stretch = !cfg->stretch;

	if (cfg->fullscreen == FULLSCR) {
		gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, FALSE, FALSE);
	}
}
void mainWindow::s_set_audio_buffer_factor() {
	int factor = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->audio_buffer_factor == factor) {
		return;
	}

	emu_pause(TRUE);
	cfg->audio_buffer_factor = factor;
	snd_playback_start();
	gui_update();
	emu_pause(FALSE);
}
void mainWindow::s_set_samplerate() {
	int samplerate = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->samplerate == samplerate) {
		return;
	}

	emu_pause(TRUE);
	wave_close();
	cfg->samplerate = samplerate;
	snd_playback_start();
	gui_update();
	emu_pause(FALSE);
}
void mainWindow::s_set_channels() {
	int channels = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->channels_mode == channels) {
		return;
	}

	emu_pause(TRUE);
	wave_close();
	cfg->channels_mode = channels;
	snd_playback_start();
	gui_update();
	emu_pause(FALSE);
}
void mainWindow::s_set_stereo_delay() {
	double delay = QVariant(((QObject *)sender())->property("myValue")).toDouble() / 100.0f;

	if (cfg->stereo_delay == delay) {
		return;
	}

	cfg->stereo_delay = delay;
	ch_stereo_delay_set();
	gui_update();
}
void mainWindow::s_set_audio_quality() {
	int quality = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->audio_quality == quality) {
		return;
	}

	emu_pause(TRUE);
	cfg->audio_quality = quality;
	audio_quality(cfg->audio_quality);
	gui_update();
	emu_pause(FALSE);
}
void mainWindow::s_set_audio_swap_duty() {
	emu_pause(TRUE);
	cfg->swap_duty = !cfg->swap_duty;
	gui_update();
	emu_pause(FALSE);
}
void mainWindow::s_set_audio_enable() {
	emu_pause(TRUE);
	if ((cfg->apu.channel[APU_MASTER] = !cfg->apu.channel[APU_MASTER])) {
		snd_playback_start();
	} else {
		snd_playback_stop();
	}
	gui_apu_channels_update_dialog();
	gui_update();
	emu_pause(FALSE);
}
void mainWindow::s_set_language() {
	int lang = QVariant(((QObject *)sender())->property("myValue")).toInt();

	set_language(lang);
}
void mainWindow::s_set_disable_new_menu() {
	cfg->disable_new_menu = !cfg->disable_new_menu;
}
void mainWindow::s_set_ff_velocity() {
	int velocity = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->ff_velocity == velocity) {
		return;
	}

	cfg->ff_velocity = velocity;
	gui_update();

	if (fps.fast_forward == TRUE) {
		fps_fast_forward();
	}
}
void mainWindow::s_set_input() {
	dlgInput *dlg = new dlgInput(this);

	dlg->show();
}
void mainWindow::s_set_hide_sprites() {
	cfg->hide_sprites = !cfg->hide_sprites;
	gui_ppu_hacks_update_dialog();
	update_menu_settings();
}
void mainWindow::s_set_hide_background() {
	cfg->hide_background = !cfg->hide_background;
	gui_ppu_hacks_update_dialog();
	update_menu_settings();
}
void mainWindow::s_set_unlimited_sprites() {
	cfg->unlimited_sprites = !cfg->unlimited_sprites;
	gui_ppu_hacks_update_dialog();
	update_menu_settings();
}
void mainWindow::s_set_ppu_overclock() {
	emu_pause(TRUE);
	cfg->ppu_overclock = !cfg->ppu_overclock;
	gui_ppu_hacks_update_dialog();
	update_menu_settings();
	ppu_overclock(TRUE);
	emu_pause(FALSE);
	settings_pgs_save();
}
void mainWindow::s_set_save_battery_ram_file() {
	if (!cfg->save_battery_ram_file) {
		info.bat_ram_frames = 0;
	}

	cfg->save_battery_ram_file = !cfg->save_battery_ram_file;
}
void mainWindow::s_set_pause_in_background() {
	cfg->bck_pause = !cfg->bck_pause;
}
void mainWindow::s_cheat_mode_select() {
	int mode = QVariant(((QObject *)sender())->property("myValue")).toInt();

	if (cfg->cheat_mode == mode) {
		return;
	}

	emu_pause(TRUE);
	cfg->cheat_mode = mode;

	switch (cfg->cheat_mode) {
		case NOCHEAT_MODE:
			cheatslist_blank();
			break;
		case GAMEGENIE_MODE:
			cheatslist_blank();
			gamegenie_check_rom_present(TRUE);
			break;
		case CHEATSLIST_MODE:
			chobj->apply_cheats();
			break;
	}

	gui_update();
	emu_pause(FALSE);
}
void mainWindow::s_cheat_dialog() {
	dlgCheats *dlg = new dlgCheats(this, chobj);

	dlg->exec();
}
void mainWindow::s_set_save_on_exit() {
	cfg->save_on_exit = !cfg->save_on_exit;
}
void mainWindow::s_save_settings() {
	settings_save();
}
void mainWindow::s_state_save_slot_action() {
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
void mainWindow::s_state_save_slot_incdec() {
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
void mainWindow::s_state_save_slot_set() {
	int slot = QVariant(((QObject *)sender())->property("myValue")).toInt();

	state_save_slot_set(slot, true);
	update_window();
}
void mainWindow::s_state_save_file() {
	QStringList filters;
	QString file;
	uTCHAR *fl;

	emu_pause(TRUE);

	filters.append(tr("Save states"));
	filters.append(tr("All files"));

	filters[0].append(" (*.pns *.PNS)");
	filters[1].append(" (*.*)");

	/* game genie */
	if (info.mapper.id == GAMEGENIE_MAPPER) {
		fl = info.load_rom_file;
	} else {
		fl = info.rom_file;
	}

	file = QFileDialog::getSaveFileName(this, tr("Save state on file"),
			QFileInfo(uQString(fl)).baseName(), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(QString(file) + ".pns");
		}

		umemset(cfg->save_file, 0x00, usizeof(cfg->save_file));
		ustrncpy(cfg->save_file, uQStringCD(fileinfo.absoluteFilePath()),
				usizeof(cfg->save_file) - 1);
		save_slot_save(SAVE_SLOT_FILE);
		settings_pgs_save();
	}

	emu_pause(FALSE);
}
void mainWindow::s_state_load_file() {
	QStringList filters;
	QString file;

	emu_pause(TRUE);

	filters.append(tr("Save states"));
	filters.append(tr("All files"));

	filters[0].append(" (*.pns *.PNS)");
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
void mainWindow::s_help() {
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
	text.append("<center>" + tr("If you like the emulator and you want to support it's development or would you pay for a beer at the programmer :") + "</center>\n");
	text.append("<center><a href=\"" + QString(DONATE) + "\">" + "<img src=\":/pics/pics/btn_donate_SM.gif\">" + "</a></center>\n");
	text.append("<center>" + tr("Anyway, thank you all for the love and the help.") + "</center>");

	about->setInformativeText(text);

	about->setStandardButtons(QMessageBox::Ok);
	about->setDefaultButton(QMessageBox::Ok);

	about->show();
	about->exec();

	emu_pause(FALSE);
}
void mainWindow::s_shcjoy_read_timer() {
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
					ui->action_Open->trigger();
					break;
				case SET_INP_SC_QUIT:
					ui->action_Quit->trigger();
					break;
				case SET_INP_SC_TURN_OFF:
					ui->action_Turn_Off->trigger();
					break;
				case SET_INP_SC_HARD_RESET:
					ui->action_Hard_Reset->trigger();
					break;
				case SET_INP_SC_SOFT_RESET:
					ui->action_Soft_Reset->trigger();
					break;
				case SET_INP_SC_INSERT_COIN:
					ui->action_Insert_Coin->trigger();
					break;
				case SET_INP_SC_SWITCH_SIDES:
					ui->action_Switch_sides->trigger();
					break;
				case SET_INP_SC_EJECT_DISK:
					ui->action_Eject_Insert_Disk->trigger();
					break;
				case SET_INP_SC_WAV:
					ui->action_Start_Stop_WAV_recording->trigger();
					break;
				case SET_INP_SC_FULLSCREEN:
					ui->action_Fullscreen->trigger();
					break;
				case SET_INP_SC_PAUSE:
					ui->action_Pause->trigger();
					break;
				case SET_INP_SC_FAST_FORWARD:
					ui->action_Fast_Forward->trigger();
					break;
				case SET_INP_SC_SCREENSHOT:
					ui->action_Save_Screenshot->trigger();
					break;
				case SET_INP_SC_MODE_PAL:
					ui->action_PAL->trigger();
					break;
				case SET_INP_SC_MODE_NTSC:
					ui->action_NTSC->trigger();
					break;
				case SET_INP_SC_MODE_DENDY:
					ui->action_Dendy->trigger();
					break;
				case SET_INP_SC_MODE_AUTO:
					ui->action_Mode_Auto->trigger();
					break;
				case SET_INP_SC_SCALE_1X:
					ui->action_1x->trigger();
					break;
				case SET_INP_SC_SCALE_2X:
					ui->action_2x->trigger();
					break;
				case SET_INP_SC_SCALE_3X:
					ui->action_3x->trigger();
					break;
				case SET_INP_SC_SCALE_4X:
					ui->action_4x->trigger();
					break;
				case SET_INP_SC_SCALE_5X:
					ui->action_5x->trigger();
					break;
				case SET_INP_SC_SCALE_6X:
					ui->action_6x->trigger();
					break;
				case SET_INP_SC_INTERPOLATION:
					ui->action_Interpolation->trigger();
					break;
				case SET_INP_SC_STRETCH_FULLSCREEN:
					ui->action_Stretch_in_fullscreen->trigger();
					break;
				case SET_INP_SC_AUDIO_ENABLE:
					ui->action_Audio_Enable->trigger();
					break;
				case SET_INP_SC_SAVE_SETTINGS:
					ui->action_Save_settings->trigger();
					break;
				case SET_INP_SC_SAVE_STATE:
					ui->action_Save_state->trigger();
					break;
				case SET_INP_SC_LOAD_STATE:
					ui->action_Load_state->trigger();
					break;
				case SET_INP_SC_INC_SLOT:
					ui->action_Increment_slot->trigger();
					break;
				case SET_INP_SC_DEC_SLOT:
					ui->action_Decrement_slot->trigger();
					break;
			}
		} else if (shcjoy.sch.mode == PRESSED) {}
	}
}
