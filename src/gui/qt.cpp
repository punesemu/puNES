/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QMessageBox>
#include <QtGui/QImage>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtGui/QScreen>
#include <QtGui/QFontDatabase>
#include <QtCore/QBuffer>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QTextCodec>
#else
#include <QtCore/QStringDecoder>
#endif
#if defined (_WIN32)
#include <QtCore/QtPlugin>
#if defined (QT5_PLUGIN_QWINDOWS)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif
#if defined (QT_PLUGIN_QWINDOWSVISTASTYLE)
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
#endif
#if defined (QT_PLUGIN_QGIF)
Q_IMPORT_PLUGIN(QGifPlugin)
#endif
#if defined (QT_PLUGIN_QICO)
Q_IMPORT_PLUGIN(QICOPlugin)
#endif
#if defined (QT_PLUGIN_QJPEG)
Q_IMPORT_PLUGIN(QJpegPlugin)
#endif
#if defined (QT_PLUGIN_QSVG)
Q_IMPORT_PLUGIN(QSvgPlugin)
#endif
#endif
#if !defined (_WIN32)
// mi serve per il std::thread::hardware_concurrency() del gui_hardware_concurrency.
#include <thread>
#endif
#include <unistd.h>
#include <libgen.h>
#if defined (WITH_OPENGL)
#include "opengl.h"
#endif
#include "mainApplication.hpp"
#include "mainWindow.hpp"
#include "objCheat.hpp"
#include "dlgHeaderEditor.hpp"
#include "dlgJsc.hpp"
#include "dlgKeyboard.hpp"
#include "dlgLog.hpp"
#include "dlgSettings.hpp"
#include "dlgUncomp.hpp"
#include "dlgVsSystem.hpp"
#include "dlgDetachBarcode.hpp"
#include "dlgDipswitch.hpp"
#include "wdgScreen.hpp"
#include "wdgOverlayUi.hpp"
#include "video/gfx_thread.h"
#include "emu_thread.h"
#include "version.h"
#include "conf.h"
#include "clock.h"
#include "save_slot.h"
#include "vs_system.h"
#include "dipswitch.h"
#include "gui.h"
#if defined (WITH_D3D9)
#include "d3d9.h"
#endif
#include "cmd_line.h"
#include "input/standard_controller.h"

INLINE void gui_init_os(void);
INLINE uTCHAR *gui_home(void);

static void gui_is_in_desktop(int *x, int *y);

static struct _qt {
	mainApplication *app{};
	mainWindow *mwin{};
	wdgScreen *screen{};
	objCheat *objch{};
	QImage qimage;
	QByteArray sba;

	// widget dell'overlay
	dlgLog *log{};

	// widget dell'overlay
	wdgOverlayUi *overlay{};

	// dialog del settaggio
	dlgSettings *dset{};

	// controlli esterni
	dlgVsSystem *vssystem{};
	dlgDetachBarcode *dbarcode{};
	dlgJsc *djsc{};

	// dialog della tastiera virtuale
	dlgKeyboard *dkeyb{};

	// dialog dell'editor di header
	dlgHeaderEditor *header{};

	// QObject che non mandano un pause quando in background
	QList<QWidget *>no_bck_pause;
} qt;

class appEventFilter: public QObject {
	public:
		appEventFilter() : QObject() {};
		~appEventFilter() override = default;;

		bool eventFilter(QObject* object, QEvent* event) override {
			if (event->type() == QEvent::MouseMove) {
				gmouse.timer = gui_get_ms();
				if (gmouse.hidden) {
					gui_cursor_hide(FALSE);
				}
			}
			return (QObject::eventFilter(object, event));
		}
};

BYTE gui_init(int *argc, char **argv) {
	QFlags<mainApplication::Mode> mode = mainApplication::Mode::ExcludeAppVersion | mainApplication::Mode::ExcludeAppPath;
	int i = 0;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
	//QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
	//QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

	memset(&gui, 0, sizeof(gui));
	qt = {};
	qt.app = new mainApplication((*argc), argv, true, mode);

#if defined (WITHOUT_PORTABLE_MODE)
	info.portable = FALSE;
#else
	info.portable = cmd_line_check_portable((*argc), (uTCHAR **)argv);
#endif

	info.gui = TRUE;
	gui.in_update = FALSE;
	gui.main_win_lfp = 0;

	for (i = 0; i < PORT_MAX; i++) {
		gui.dlg_tabWidget_kbd_joy_index[i] = -1;
	}


	fprintf(stderr, "0\n");

	gui_init_os();

	fprintf(stderr, "1\n");


#if defined(WITH_D3D9)
	if (d3d9_is_installed() == EXIT_ERROR) {
		return (EXIT_ERROR);
	}
#endif

	return (qt.app->control_base_folders());
}
void gui_quit(void) {}
BYTE gui_control_instance(void) {
	if (qt.app->isSecondary() && !cfg->multiple_instances) {
		if (info.rom.file[0]) {
			QFileInfo finfo(uQString(info.rom.file));
			unsigned int count = 0;

#if defined (_WIN32)
			// https://github.com/itay-grudev/SingleApplication/blob/master/Windows.md
			AllowSetForegroundWindow(DWORD(qt.app->primaryPid()));
#endif
			if (finfo.exists()) {
				do {
					if (qt.app->sendMessage(finfo.absoluteFilePath().toUtf8(), 200)) {
						break;
					}
					gui_sleep(20);
					count++;
				} while (count < 10);
			}
		}
		mainApplication::exit(0);
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}
BYTE gui_create(void) {
#if defined (WITH_OPENGL)
	QSurfaceFormat fmt;

	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setProfile(QSurfaceFormat::CoreProfile);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	fmt.setRedBufferSize(8);
	fmt.setGreenBufferSize(8);
	fmt.setBlueBufferSize(8);
	fmt.setAlphaBufferSize(8);
	fmt.setSwapInterval(cfg->vsync);
	QSurfaceFormat::setDefaultFormat(fmt);
#endif

	// Thx to https://people.mpi-inf.mpg.de/~uwe/misc/uw-ttyp0/
	// "Ttyp0_11" (unicode 11px)
	QFontDatabase::addApplicationFont(":/fonts/fonts/t0-11-uni_fixed.ttf");
	// "Ttyp0_12" (unicode 12px)
	QFontDatabase::addApplicationFont(":/fonts/fonts/t0-12-uni_fixed.ttf");
	// "Ttyp0_13" (unicode 13px)
	QFontDatabase::addApplicationFont(":/fonts/fonts/t0-13-uni_fixed.ttf");
	// "Ttyp0_14" (unicode 14px)
	QFontDatabase::addApplicationFont(":/fonts/fonts/t0-14-uni_fixed.ttf");

	// Thx to Skylar Park
	// "Pixelated" (8px very small)
	QFontDatabase::addApplicationFont(":/fonts/fonts/pixelated.ttf");

	// Thx to https://andrewtyler.gumroad.com/
	// "pixelmix" (8px)
	QFontDatabase::addApplicationFont(":/fonts/fonts/pixelmix.ttf");

	// Thx to https://github.com/cmvnd/fonts
	// "lemon_10" (10px)
	QFontDatabase::addApplicationFont(":/fonts/fonts/lemon_fixed.ttf");

	// Thx to http://www.devincook.com/
	// "Commodore 64 Pixelized" (10px)
	QFontDatabase::addApplicationFont(":/fonts/fonts/Commodore_Pixelized_v1.2.ttf");

	qt.mwin = new mainWindow();
	qt.screen = qt.mwin->wscreen;
	qt.objch->setParent(qt.mwin);

	qt.app->installEventFilter(new appEventFilter());

	gui_is_in_desktop(&cfg->lg.x, &cfg->lg.y);
	gui_is_in_desktop(&cfg->lg_settings.x, &cfg->lg_settings.y);
	gui_is_in_desktop(&cfg->lg_nes_keyboard.x, &cfg->lg_nes_keyboard.y);
	qt.mwin->setGeometry(cfg->lg.x, cfg->lg.y, 0, 0);

	qt.mwin->show();

	qt.log = new dlgLog(qt.mwin);
	qt.log->start_thread();

	log_info(uL("" uPs("") " (by FHorse) " uPs("") ", " uPs("") ", " uPs("")
#if defined (WITH_GIT_INFO)
		", commit " uPs("")),
#else
		uPs("")),
#endif
		uL("" NAME), uL("" VERSION), uL("" ENVIRONMENT), uL("" VERTYPE),
#if defined (WITH_GIT_INFO)
		uL("" GIT_COUNT_COMMITS));
#else
		uL(""));
#endif

	log_info(uL("folders"));
	log_info_box(uL("config;" uPs("") ""), gui_config_folder());
	log_info_box(uL("data;" uPs("") ""), gui_data_folder());
	log_info_box(uL("temp;" uPs("") ""), gui_temp_folder());

	qt.dset = new dlgSettings(qt.mwin);
	qt.overlay = new wdgOverlayUi();

	memset(&ext_win, 0x00, sizeof(ext_win));
	qt.vssystem = new dlgVsSystem(qt.mwin);
	qt.dbarcode = new dlgDetachBarcode(qt.mwin);
	qt.djsc = new dlgJsc(qt.mwin);
	qt.dkeyb = new dlgKeyboard(qt.mwin);
	qt.header = new dlgHeaderEditor(qt.mwin);

	qt.no_bck_pause.append(qt.mwin);
	qt.no_bck_pause.append(qt.dset);
	qt.no_bck_pause.append(qt.vssystem);
	qt.no_bck_pause.append(qt.dbarcode);
	qt.no_bck_pause.append(qt.djsc);
	qt.no_bck_pause.append(qt.dkeyb);

	gmouse.hidden = FALSE;
	gmouse.timer = gui_get_ms();

	// forzo un evento QEvent::LanguageChange. Se non lo faccio,
	// sotto windows, all'avvio dell'emulatore, se il cfg->language e' impostato
	// sull'inglese, non viene emesso nessun QEvent::LanguageChangen non permettendo
	// di tradurre correttamente i bottoni degli shortcuts nel wdgSettingsInput.
	QEvent event(QEvent::LanguageChange);

	QApplication::sendEvent(qt.mwin, &event);
	QApplication::sendEvent(qt.dset, &event);
	QApplication::sendEvent(qt.overlay, &event);

	if (theme::is_dark_theme()) {
		gui_set_dark_theme();
	} else {
		gui_set_light_theme();
	}
	return (EXIT_OK);
}
void gui_start(void) {
	gui.start = TRUE;
	fps.frame.expected_end = gui_get_ms() + machine.ms_frame;
	gfx_thread_continue();
	emu_thread_continue();
	mainApplication::exec();
}

void gui_set_dark_theme(void) {
	QPalette palette = qt.mwin->palette();

	palette.setColor(QPalette::Active, QPalette::WindowText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::Button, QColor("#323232"));
	palette.setColor(QPalette::Active, QPalette::Light, QColor("#3E3E3E"));
	palette.setColor(QPalette::Active, QPalette::Midlight, QColor("#383838"));
	palette.setColor(QPalette::Active, QPalette::Dark, QColor("#2C2C2C"));
	palette.setColor(QPalette::Active, QPalette::Mid, QColor("#383838"));
	palette.setColor(QPalette::Active, QPalette::Text, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::BrightText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::ButtonText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::Base, QColor("#323232"));
	palette.setColor(QPalette::Active, QPalette::Window, QColor("#323232"));
	palette.setColor(QPalette::Active, QPalette::Shadow, QColor("#020202"));
	palette.setColor(QPalette::Active, QPalette::Highlight, QColor("#15539E"));
	palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::Link, QColor("#308CC6"));
	palette.setColor(QPalette::Active, QPalette::LinkVisited, QColor("#FF00FF"));
	palette.setColor(QPalette::Active, QPalette::AlternateBase, QColor("#2F2F2F"));
	palette.setColor(QPalette::Active, QPalette::NoRole, QColor("#000000"));
	palette.setColor(QPalette::Active, QPalette::ToolTipBase, QColor("#FFFFDC"));
	palette.setColor(QPalette::Active, QPalette::ToolTipText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
	palette.setColor(QPalette::Active, QPalette::PlaceholderText, QColor("#9B9B9B"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	palette.setColor(QPalette::Active, QPalette::Accent, QColor("#308CC6"));
#endif
#endif

	palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#989898"));
	palette.setColor(QPalette::Disabled, QPalette::Button, QColor("#323232"));
	palette.setColor(QPalette::Disabled, QPalette::Light, QColor("#3E3E3E"));
	palette.setColor(QPalette::Disabled, QPalette::Midlight, QColor("#383838"));
	palette.setColor(QPalette::Disabled, QPalette::Dark, QColor("#2C2C2C"));
	palette.setColor(QPalette::Disabled, QPalette::Mid, QColor("#383838"));
	palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#989898"));
	palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#989898"));
	palette.setColor(QPalette::Disabled, QPalette::Base, QColor("#323232"));
	palette.setColor(QPalette::Disabled, QPalette::Window, QColor("#323232"));
	palette.setColor(QPalette::Disabled, QPalette::Shadow, QColor("#020202"));
	palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor("#15539E"));
	palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Disabled, QPalette::Link, QColor("#308CC6"));
	palette.setColor(QPalette::Disabled, QPalette::LinkVisited, QColor("#FF00FF"));
	palette.setColor(QPalette::Disabled, QPalette::AlternateBase, QColor("#2F2F2F"));
	palette.setColor(QPalette::Disabled, QPalette::NoRole, QColor("#000000"));
	palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, QColor("#FFFFDC"));
	palette.setColor(QPalette::Disabled, QPalette::ToolTipText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
	palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, QColor("#9B9B9B"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	palette.setColor(QPalette::Disabled, QPalette::Accent, QColor("#919191"));
#endif
#endif

	palette.setColor(QPalette::Inactive, QPalette::WindowText, QColor("#919190"));
	palette.setColor(QPalette::Inactive, QPalette::Button, QColor("#323232"));
	palette.setColor(QPalette::Inactive, QPalette::Light, QColor("#3E3E3E"));
	palette.setColor(QPalette::Inactive, QPalette::Midlight, QColor("#383838"));
	palette.setColor(QPalette::Inactive, QPalette::Dark, QColor("#2C2C2C"));
	palette.setColor(QPalette::Inactive, QPalette::Mid, QColor("#383838"));
	palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#FFFFFF"));
	palette.setColor(QPalette::Inactive, QPalette::BrightText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Inactive, QPalette::ButtonText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Inactive, QPalette::Base, QColor("#323232"));
	palette.setColor(QPalette::Inactive, QPalette::Window, QColor("#323232"));
	palette.setColor(QPalette::Inactive, QPalette::Shadow, QColor("#020202"));
	palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor("#15539E"));
	palette.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Inactive, QPalette::Link, QColor("#308CC6"));
	palette.setColor(QPalette::Inactive, QPalette::LinkVisited, QColor("#FF00FF"));
	palette.setColor(QPalette::Inactive, QPalette::AlternateBase, QColor("#2F2F2F"));
	palette.setColor(QPalette::Inactive, QPalette::NoRole, QColor("#000000"));
	palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, QColor("#FFFFDC"));
	palette.setColor(QPalette::Inactive, QPalette::ToolTipText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
	palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, QColor("#9B9B9B"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	palette.setColor(QPalette::Inactive, QPalette::Accent, QColor("#308CC6"));
#endif
#endif

	QApplication::setPalette(palette);
}

void gui_set_light_theme(void) {
	QPalette palette = qt.mwin->palette();

	palette.setColor(QPalette::Active, QPalette::WindowText, QColor("#000000"));
	palette.setColor(QPalette::Active, QPalette::Button, QColor("#EFEFEF"));
	palette.setColor(QPalette::Active, QPalette::Light, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::Midlight, QColor("#CACACA"));
	palette.setColor(QPalette::Active, QPalette::Dark, QColor("#9F9F9F"));
	palette.setColor(QPalette::Active, QPalette::Mid, QColor("#B8B8B8"));
	palette.setColor(QPalette::Active, QPalette::Text, QColor("#000000"));
	palette.setColor(QPalette::Active, QPalette::BrightText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::ButtonText, QColor("#000000"));
	palette.setColor(QPalette::Active, QPalette::Base, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::Window, QColor("#EFEFEF"));
	palette.setColor(QPalette::Active, QPalette::Shadow, QColor("#767676"));
	palette.setColor(QPalette::Active, QPalette::Highlight, QColor("#308CC6"));
	palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Active, QPalette::Link, QColor("#0000FF"));
	palette.setColor(QPalette::Active, QPalette::LinkVisited, QColor("#FF00FF"));
	palette.setColor(QPalette::Active, QPalette::AlternateBase, QColor("#F7F7F7"));
	palette.setColor(QPalette::Active, QPalette::NoRole, QColor("#000000"));
	palette.setColor(QPalette::Active, QPalette::ToolTipBase, QColor("#FFFFDC"));
	palette.setColor(QPalette::Active, QPalette::ToolTipText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
	palette.setColor(QPalette::Active, QPalette::PlaceholderText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	palette.setColor(QPalette::Active, QPalette::Accent, QColor("#308CC6"));
#endif
#endif

	palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#BEBEBE"));
	palette.setColor(QPalette::Disabled, QPalette::Button, QColor("#EFEFEF"));
	palette.setColor(QPalette::Disabled, QPalette::Light, QColor("#FFFFFF"));
	palette.setColor(QPalette::Disabled, QPalette::Midlight, QColor("#CACACA"));
	palette.setColor(QPalette::Disabled, QPalette::Dark, QColor("#BEBEBE"));
	palette.setColor(QPalette::Disabled, QPalette::Mid, QColor("#B8B8B8"));
	palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#BEBEBE"));
	palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#BEBEBE"));
	palette.setColor(QPalette::Disabled, QPalette::Base, QColor("#EFEFEF"));
	palette.setColor(QPalette::Disabled, QPalette::Window, QColor("#EFEFEF"));
	palette.setColor(QPalette::Disabled, QPalette::Shadow, QColor("#B1B1B1"));
	palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor("#919191"));
	palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Disabled, QPalette::Link, QColor("#0000FF"));
	palette.setColor(QPalette::Disabled, QPalette::LinkVisited, QColor("#FF00FF"));
	palette.setColor(QPalette::Disabled, QPalette::AlternateBase, QColor("#F7F7F7"));
	palette.setColor(QPalette::Disabled, QPalette::NoRole, QColor("#000000"));
	palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, QColor("#FFFFDC"));
	palette.setColor(QPalette::Disabled, QPalette::ToolTipText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
	palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	palette.setColor(QPalette::Disabled, QPalette::Accent, QColor("#919191"));
#endif
#endif

	palette.setColor(QPalette::Inactive, QPalette::WindowText, QColor("#000000"));
	palette.setColor(QPalette::Inactive, QPalette::Button, QColor("#EFEFEF"));
	palette.setColor(QPalette::Inactive, QPalette::Light, QColor("#FFFFFF"));
	palette.setColor(QPalette::Inactive, QPalette::Midlight, QColor("#CACACA"));
	palette.setColor(QPalette::Inactive, QPalette::Dark, QColor("#9F9F9F"));
	palette.setColor(QPalette::Inactive, QPalette::Mid, QColor("#B8B8B8"));
	palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#000000"));
	palette.setColor(QPalette::Inactive, QPalette::BrightText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Inactive, QPalette::ButtonText, QColor("#000000"));
	palette.setColor(QPalette::Inactive, QPalette::Base, QColor("#FFFFFF"));
	palette.setColor(QPalette::Inactive, QPalette::Window, QColor("#EFEFEF"));
	palette.setColor(QPalette::Inactive, QPalette::Shadow, QColor("#767676"));
	palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor("#308CC6"));
	palette.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor("#FFFFFF"));
	palette.setColor(QPalette::Inactive, QPalette::Link, QColor("#0000FF"));
	palette.setColor(QPalette::Inactive, QPalette::LinkVisited, QColor("#FF00FF"));
	palette.setColor(QPalette::Inactive, QPalette::AlternateBase, QColor("#F7F7F7"));
	palette.setColor(QPalette::Inactive, QPalette::NoRole, QColor("#000000"));
	palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, QColor("#FFFFDC"));
	palette.setColor(QPalette::Inactive, QPalette::ToolTipText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
	palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, QColor("#000000"));
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	palette.setColor(QPalette::Inactive, QPalette::Accent, QColor("#308CC6"));
#endif
#endif

	QApplication::setPalette(palette);
}

size_t gui_utf8_to_utchar(char *input, uTCHAR **output, size_t max_size) {
	bool is_iso8859_1 = false;
	uTCHAR *buff = NULL;
	size_t size = 0;
	QString s;

	{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QTextCodec *clatin1 = QTextCodec::codecForName("ISO 8859-1");
		QTextDecoder latin1(clatin1);
		QTextCodec *cutf8 = QTextCodec::codecForName("UTF-8");
		QTextDecoder utf8(cutf8);
		QString slatin1 = latin1.toUnicode(input);
		QString sutf8 = utf8.toUnicode(input);

		is_iso8859_1 = !latin1.hasFailure() && utf8.hasFailure();
#else
		QStringDecoder latin1 = QStringDecoder(QStringDecoder::Latin1);
		QStringDecoder utf8 = QStringDecoder(QStringDecoder::Utf8);
		QString slatin1 = latin1(input);
		QString sutf8 = utf8(input);

		is_iso8859_1 = !latin1.hasError() && utf8.hasError();
#endif
	}

	s = is_iso8859_1 ? QString::fromLatin1(input) : QString::fromUtf8(input);
	size = s.size();
	if (size && (size <= max_size)) {
		QByteArray array = uQByteArrayFromString(s);
		size_t asize = array.length();

		buff = asize ? (uTCHAR *)malloc(asize + sizeof(uTCHAR)) : NULL;
		if (buff) {
			memset(buff, 0x00, asize + sizeof(uTCHAR));
			memcpy(buff, uQByteArrayCD(array), asize);
		}
	}
	(*output) = buff;
	return (buff ? size + 1: 0);
}

const uTCHAR *gui_home_folder(void) {
	return (gui_home());
}
const uTCHAR *gui_application_folder(void) {
	QString path = QCoreApplication::applicationDirPath();

	qt.sba.clear();
	qt.sba = uQByteArrayFromString(path);
	return (uQByteArrayCD(qt.sba));
}
const uTCHAR *gui_config_folder(void) {
	QString path;

	if (info.portable) {
		return (gui_application_folder());
	}
	path = QString("%0/%1").arg(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation), NAME);
	qt.sba.clear();
	qt.sba = uQByteArrayFromString(path);
	return (uQByteArrayCD(qt.sba));
}
const uTCHAR *gui_data_folder(void) {
	QString path;

	if (info.portable) {
		return (gui_application_folder());
	}
#if defined (_WIN32)
	path = QString("%0/%1").arg(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation), NAME);
#else
	path = QString("%0/%1").arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation), NAME);
#endif
	qt.sba.clear();
	qt.sba = uQByteArrayFromString(path);
	return (uQByteArrayCD(qt.sba));
}
const uTCHAR *gui_temp_folder(void) {
	QString path = QString("%0/%1").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation), NAME);

	qt.sba.clear();
	qt.sba = uQByteArrayFromString(path);
	return (uQByteArrayCD(qt.sba));
}
const uTCHAR *gui_extract_base(const uTCHAR *path) {
	QString spath = (uQString(path)).remove(QRegularExpression(QString("/%0$").arg(NAME)));

	qt.sba.clear();
	qt.sba = uQByteArrayFromString(spath);
	return (uQByteArrayCD(qt.sba));
}

double gui_device_pixel_ratio(void) {
	return (qt.mwin->win_handle_screen()->devicePixelRatio());
}
void gui_set_window_size(void) {
	int w = gfx.w[VIDEO_MODE], h = gfx.h[VIDEO_MODE];
	bool toolbar = false;

#if defined (_WIN64)
	if (gfx.type_of_fscreen_in_use == FULLSCR_IN_WINDOW) {
		return;
	}
#else
#if QT_VERSION == QT_VERSION_CHECK(5, 12, 8)
	if (gfx.type_of_fscreen_in_use == FULLSCR) {
		return;
	}
#else
	if (cfg->fullscreen) {
		return;
	}
#endif
#endif

	if ((cfg->screen_rotation == ROTATE_90) || (cfg->screen_rotation == ROTATE_270)) {
		w = gfx.h[VIDEO_MODE];
		h = gfx.w[VIDEO_MODE];
	}

	if (cfg->scale == X1) {
		qt.mwin->toolbar->rotate_setVisible(false);
		qt.mwin->toolbar->state_setVisible(false);
		if (overscan.enabled) {
			qt.mwin->menu_Help->menuAction()->setVisible(false);
		} else {
			qt.mwin->menu_Help->menuAction()->setVisible(true);
		}
	} else {
		qt.mwin->toolbar->rotate_setVisible(true);
		qt.mwin->toolbar->state_setVisible(true);
		qt.mwin->menu_Help->menuAction()->setVisible(true);
	}

	toolbar = qt.mwin->toolbar->isHidden() || qt.mwin->toolbar->isFloating();

	if (qt.mwin->toolbar->orientation() == Qt::Vertical) {
		w += (toolbar ? 0 : qt.mwin->toolbar->sizeHint().width());
	} else {
		h += (toolbar ? 0 : qt.mwin->toolbar->sizeHint().height());
	}

	// nella versione D3D9, con le shaders CRT, quando e' visibile il menu (le toolbars non influiscono) la shader
	// non funziona correttamente, appare una riga al centro dell'immagine. Sembra quasi che l'altezza dello screen
	// non venga applicata correttamente dalle QT ma che in presenza del menu venga ridotta di 1.
	// Aumentare di 1 l'altrezza quando e' visibile mitiga il problema e non sembra influenzi in alcun modo
	// anche la versione OpenGL.
	h += (qt.mwin->menubar->isHidden() ? 0 : qt.mwin->menubar->sizeHint().height() + 1);
	h += (qt.mwin->statusbar->isHidden() ? 0 : qt.mwin->statusbar->sizeHint().height());

#if defined (_WIN32) && defined(WITH_OPENGL)
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
	if (gfx.type_of_fscreen_in_use == FULLSCR) {
		h += 1;
	}
#endif

#if defined (_WIN32)
	qt.mwin->resize(QSize(w, h));
#else
	qt.mwin->setFixedSize(QSize(w, h));
	qt.mwin->menubar->setFixedWidth(w);
	qt.mwin->statusbar->setFixedWidth(w);
#endif
}

void gui_state_save_slot_set(BYTE slot, BYTE on_video) {
	if (slot >= SAVE_SLOTS) {
		slot = SAVE_SLOTS - 1;
	}
	qt.mwin->state_save_slot_set(slot, on_video);
}
void gui_state_save_slot_set_tooltip(BYTE slot) {
	qt.mwin->state_save_slot_set_tooltip(slot);
}

void gui_update(void) {
	uTCHAR title[255];

	gui.in_update = TRUE;

	emu_set_title(title, usizeof(title));
	qt.mwin->setWindowTitle(uQString(title));
	qt.mwin->update_window();
	qt.dset->update_dialog();
	qt.overlay->update_widget();

	gui.in_update = FALSE;
}
void gui_update_gps_settings(void) {
	qt.dset->change_rom();
}
void gui_update_status_bar(void) {
	qt.mwin->statusbar->update_statusbar();
}

void gui_update_ntsc_widgets(void) {
	qt.dset->update_tab_video();
}
void gui_update_apu_channels_widgets(void) {
	qt.dset->update_tab_audio();
}
void gui_update_recording_widgets(void) {
	qt.mwin->update_recording_widgets();
}

void gui_update_ppu_hacks_lag_frames(void) {
	qt.dset->widget_Settings_PPU->lag_counter_update();
}

void gui_update_fds_menu(void) {
	qt.mwin->update_fds_menu();
}
void gui_update_tape_menu(void) {
	qt.mwin->update_tape_menu();
}
void gui_update_recording_tab(void) {
	qt.dset->update_tab_recording();
}

void gui_egds_set_fps(void) {
	qt.mwin->egds->set_fps();
}
void gui_egds_stop_unnecessary(void) {
	if (gui.start) {
		qt.mwin->egds->stop_unnecessary();
	}
}
void gui_egds_start_pause(void) {
	qt.mwin->egds->start_pause();
}
void gui_egds_stop_pause(void) {
	qt.mwin->egds->stop_pause();
}
void gui_egds_start_rwnd(void) {
	qt.mwin->egds->start_rwnd();
}
void gui_egds_stop_rwnd(void) {
	qt.mwin->egds->stop_rwnd();
}

void gui_fullscreen(void) {
	// se l'emulatore si avvia in fullscreen modalita' finestra, senza questo ritardo
	// e' possibile che le QT mi passino informazioni non corrette sulle dimensioni del
	// desktop e che le decorazioni della finestra non appaiano correttamente (problema
	// riscontrato sotto Linux e BSD).
	QTimer::singleShot(250, qt.mwin, SLOT(s_set_fullscreen()));
}

void gui_dipswitch_dialog(void) {
	if (dipswitch.used && dipswitch.show_dlg) {
		dlgDipswitch *dlg = new dlgDipswitch(qt.mwin);

		dlg->show();
		dlg->exec();
	}
}

int gui_uncompress_selection_dialog(_uncompress_archive *archive, BYTE type) {
	dlgUncomp *dlg = new dlgUncomp(qt.mwin, (void *)archive, type);

	dlg->show();
	dlg->exec();

	return (gui.dlg_rc);
}

void gui_control_pause_bck(WORD event) {
	BYTE found = FALSE;
	int i;

	if (!cfg->bck_pause) {
		return;
	}

	for (i = 0; i < qt.no_bck_pause.count(); i++) {
		if (qt.no_bck_pause.at(i)->isActiveWindow()) {
			found = TRUE;
			break;
		}
	}

	if (event == QEvent::WindowActivate) {
		if (gui.main_win_lfp) {
			emu_pause(FALSE);
		}
		gui.main_win_lfp = FALSE;
	} else {
		if (!found) {
			emu_pause(TRUE);
			gui.main_win_lfp = TRUE;
		}
	}
}

void gui_active_window(void) {
	qt.screen->activateWindow();
}
void gui_set_focus(void) {
	qt.screen->setFocus(Qt::ActiveWindowFocusReason);
}

void *gui_objcheat_get_ptr(void) {
	return ((void *)qt.objch);
}
void gui_objcheat_init(void) {
	if (qt.objch == nullptr) {
		qt.objch = new objCheat(nullptr);
	}
	qt.objch->clear_list();
}
void gui_objcheat_read_game_cheats(void) {
	qt.objch->read_game_cheats(nullptr);
}

void gui_cursor_init(void) {
	qt.screen->cursor_init();
}
void gui_cursor_set(void) {
	qt.screen->cursor_set();
}
void gui_cursor_hide(BYTE hide) {
	gmouse.hidden = hide;
	qt.screen->cursor_hide(hide);
}
void gui_control_visible_cursor(void) {
	qt.mwin->control_visible_cursor();
}

void *gui_mainwindow_get_ptr(void) {
	return ((void *)qt.mwin);
}
void gui_mainwindow_coords(int *x, int *y, BYTE border) {
	switch (border) {
		default:
		// top center
		case 0:
			(*x) = qt.mwin->geometry().x() + (qt.mwin->geometry().width() / 2);
			(*y) = qt.mwin->geometry().y();
			break;
		// top left
		case 1:
			(*x) = qt.mwin->geometry().x();
			(*y) = qt.mwin->geometry().y();
			break;
		// top right
		case 2:
			(*x) = qt.mwin->geometry().x() + qt.mwin->geometry().width();
			(*y) = qt.mwin->geometry().y();
			break;
		// bottom right
		case 3:
			(*x) = qt.mwin->geometry().x() + qt.mwin->geometry().width();
			(*y) = qt.mwin->geometry().y() + qt.mwin->geometry().height();
			break;
		// bottom left
		case 4:
			(*x) = qt.mwin->geometry().x();
			(*y) = qt.mwin->geometry().y() + qt.mwin->geometry().height();
			break;
	}
}
void gui_mainwindow_before_set_res(void) {
	qt.mwin->reset_min_max_size();
	qt.mwin->menubar->setVisible(false);
	qt.mwin->toolbar->setVisible(false);
	qt.mwin->statusbar->setVisible(false);
	qt.mwin->setGeometry(0, 0, 1, 1);
}

void *gui_wdgrewind_get_ptr(void) {
	return ((void *)qt.mwin->toolbar->rewind);
}
void gui_wdgrewind_play(void) {
	wdgrewind->toolButton_Play->click();
}

void gui_emit_et_reset(BYTE type) {
	emit qt.mwin->et_reset(type);
}
void gui_emit_et_gg_reset(void) {
	emit qt.mwin->et_gg_reset();
}
void gui_emit_et_vs_reset(void) {
	emit qt.mwin->et_vs_reset();
}
void gui_emit_et_external_control_windows_show(void) {
	emit qt.mwin->et_external_control_windows_show();
}

void gui_max_speed_start(void) {
	if (fps.max_speed) {
		return;
	}
	qt.mwin->qaction_extern.max_speed.start->only_one_trigger();
}
void gui_max_speed_stop(void) {
	if (!fps.max_speed) {
		return;
	}
	qt.mwin->qaction_extern.max_speed.stop->only_one_trigger();
}

void gui_nsf_author_note_open(const uTCHAR *string) {
	emit qt.mwin->et_nsf_author_note_open(string);
}
void gui_nsf_author_note_close(void) {
	if (qt.mwin) {
		emit qt.mwin->et_nsf_author_note_close();
	}
}

void gui_toggle_audio(void) {
	qt.mwin->qaction_shcut.audio_enable->trigger();
}

void gui_decode_all_input_events(void) {
	if (info.clean_input_data) {
		for (unsigned int a = PORT1; a < PORT_MAX; a++) {
			_port *prt = &port[a];

			for (unsigned int b = 0; b < LENGTH(prt->data.raw); b++) {
				if (b < 8) {
					input_data_set_standard_controller(b, RELEASED, prt);
				}
			}
			info.clean_input_data = FALSE;
		}
	}

	if (!qt.screen->events.keyb.count() && !qt.screen->events.mouse.count()) {
		return;
	}

	qt.screen->events.mutex.lock();

	// keyboard
	if (qt.screen->events.keyb.count()) {
		for (_wdgScreen_keyboard_event &event : qt.screen->events.keyb) {
			for (BYTE i = PORT1; i < PORT_MAX; i++) {
				if (port_funct[i].input_decode_event) {
					port_funct[i].input_decode_event(event.mode, event.autorepeat, event.event, event.type, &port[i]);
				}
			}
		}
		qt.screen->events.keyb.clear();
	}

	// mouse
	if (qt.screen->events.mouse.count()) {
		for (_wdgScreen_mouse_event &event : qt.screen->events.mouse) {
			if ((event.type == QEvent::MouseButtonPress) || (event.type == QEvent::MouseButtonDblClick)) {
				if (event.button == Qt::LeftButton) {
					gmouse.left = TRUE;
				} else if (event.button == Qt::RightButton) {
					gmouse.right = TRUE;
				}
			} else if (event.type == QEvent::MouseButtonRelease) {
				if (event.button == Qt::LeftButton) {
					gmouse.left = FALSE;
				} else if (event.button == Qt::RightButton) {
					gmouse.right = FALSE;
				}
			} else if (event.type == QEvent::MouseMove) {
				gmouse.x = event.x;
				gmouse.y = event.y;
			}
		}
		qt.screen->events.mouse.clear();
	}

	qt.screen->events.mutex.unlock();
}

void gui_screen_update(void) {
#if defined (WITH_OPENGL)
	qt.screen->wogl->update();
#elif defined (WITH_D3D9)
	qt.screen->wd3d9->update();
#endif
	qt.dset->widget_Settings_Video->widget_Palette_Editor->widget_Palette_PPU->update();
}

void *gui_wdgoverlayui_get_ptr(void) {
	return ((void *)qt.overlay);
}

void *gui_dlgheadereditor_get_ptr(void) {
	return ((void *)qt.header);
}
void gui_dlgheadereditor_read_header(void) {
	qt.header->read_header(info.rom.file);
}

void *gui_dlgsettings_get_ptr(void) {
	return ((void *)qt.dset);
}
void gui_dlgsettings_input_update_joy_combo(void) {
	qt.dset->widget_Settings_Input->update_joy_list();
}

void *gui_dlgjsc_get_ptr(void) {
	return ((void *)qt.djsc);
}
void gui_dlgjsc_emit_update_joy_combo(void) {
	if (qt.djsc->isVisible()) {
		emit qt.djsc->et_update_joy_combo();
	}
}

void *gui_dlgkeyboard_get_ptr(void) {
	return ((void *)qt.dkeyb);
}

void *gui_dlglog_get_ptr(void) {
	return ((void *)qt.log);
}

void gui_js_joyval_icon_desc(int index, DBWORD input, void *icon, void *desc) {
	uTCHAR *uicon = nullptr, *udesc = nullptr;
	QString *si = (QString *)icon, *sd = (QString *)desc;

	js_joyval_icon_and_desc(index, input, &uicon, &udesc);
	(*si) = uicon ? uQString(uicon) : "";
	(*sd) = udesc ? uQString(udesc) : "";
}

void gui_external_control_windows_show(void) {
	if (ext_win.vs_system && (cfg->fullscreen != FULLSCR)) {
		qt.vssystem->update_dialog();
		qt.vssystem->show();
	} else {
		qt.vssystem->hide();
	}
	if (ext_win.detach_barcode && (cfg->fullscreen != FULLSCR)) {
		qt.dbarcode->update_dialog();
		qt.dbarcode->show();
	} else {
		qt.dbarcode->hide();
	}
	gui_update();
	gui_external_control_windows_update_pos();
	gui_active_window();
	gui_set_focus();
}
void gui_external_control_windows_update_pos(void) {
	unsigned int y = 0;

	y += qt.vssystem->update_pos((int)y);
	y += qt.dbarcode->update_pos((int)y);
}

void gui_vs_system_update_dialog(void) {
	qt.vssystem->update_dialog();
}
void gui_vs_system_insert_coin(void) {
	if (vs_system.enabled) {
		qt.vssystem->insert_coin(1);
	}
}

void gui_detach_barcode_change_rom(void) {
	qt.dbarcode->change_rom();
}

void gui_unsupported_hardware(void) {
	qt.mwin->unsupported_hardware();
}

#if defined (WITH_OPENGL)
void gui_wdgopengl_make_current(void) {
	if (gui.start) {
		qt.screen->wogl->makeCurrent();
	}
}
unsigned int gui_wdgopengl_framebuffer_id(void) {
	return (qt.screen->wogl->framebuffer_id());
}

void gui_screen_info(void) {
#if !defined (_WIN32)
	const static char *cwdisplay = "WAYLAND_DISPLAY";
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
	QByteArray qbwdisplay = qgetenv(cwdisplay);

	gfx.is_wayland = qbwdisplay.length() > 0 ? TRUE : FALSE;
#else
	gfx.is_wayland = qEnvironmentVariableIsSet(cwdisplay) ? TRUE : FALSE;
#endif
#else
	gfx.is_wayland = FALSE;
#endif
	gfx.bit_per_pixel = mainApplication::primaryScreen()->depth();
}

uint32_t gui_color(BYTE a, BYTE r, BYTE g, BYTE b) {
	return (qRgba(r, g, b, a));
}
#endif

BYTE gui_load_lut(void *l, const uTCHAR *path) {
	QImage tmp;
	_lut *lut = (_lut *)l;

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
void gui_save_screenshot(int w, int h, int stride, char *buffer, BYTE flip) {
	QString basename = QString(uQString(gui_data_folder())) + QString(SCRSHT_FOLDER) + "/"
		+ QFileInfo(uQString(info.rom.file)).completeBaseName();
	QImage screenshot = QImage((uchar *)buffer, w, h, stride, QImage::Format_RGB32);
	QFile file;
	uint count = 0;

	if (!info.rom.file[0]) {
		return;
	}

	for (count = 1; count < 999999; count++) {
		QString final = basename + QString("_%1.png").arg(count, 6, 'd', 0, '0');

		if (!QFileInfo::exists(final)) {
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
void gui_save_slot_preview_to_png(int slot, void **dst, size_t *size) {
	QImage *preview = (QImage *)gui_overlay_slot_preview_get(slot);
	QBuffer buffer;

	(*dst) = nullptr;
	(*size) = 0;

	if (!preview) {
		return;
	}

	qt.sba.clear();
	buffer.setBuffer(&qt.sba);
	buffer.open(QIODevice::WriteOnly);
	if (preview->save(&buffer, "PNG")) {
		(*dst) = uQByteArrayCD(qt.sba);
		(*size) = qt.sba.size();
	}
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

#if !defined (_WIN32)
unsigned int gui_hardware_concurrency(void) {
	return (std::thread::hardware_concurrency());
}
#endif

#if defined (__linux__)
#include "os_linux.h"
#elif defined (__OpenBSD__) || defined (__FreeBSD__)
#include "os_bsd.h"
#elif defined (_WIN32)
#include "os_windows.h"
#endif

void gui_warning(const uTCHAR *txt) {
	QMessageBox msgBox;

	msgBox.setIcon(QMessageBox::Warning);
	msgBox.setWindowTitle("Warning!");
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setText(uQString(txt));
	msgBox.exec();
	if (qt.log) {
		log_warning(txt);
	}
}
void gui_critical(const uTCHAR *txt) {
	QMessageBox msgBox;

	msgBox.setIcon(QMessageBox::Critical);
	msgBox.setWindowTitle("Error!");
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setText(uQString(txt));
	msgBox.exec();
	if (qt.log) {
		log_error(txt);
	}
}

static void gui_is_in_desktop(int *x, int *y) {
	QList<QScreen *> screens = QGuiApplication::screens();
	int i, x_min = 0, x_max = 0, y_min = 0, y_max = 0;

	for (i = 0; i < screens.count(); i++) {
		QRect g = screens[i]->availableGeometry();

		if (g.x() < x_min) {
			x_min = g.x();
		}
		if ((g.x() + g.width()) > x_max) {
			x_max = g.x() + g.width();
		}
		if (g.y() < y_min) {
			y_min = g.y();
		}
		if ((g.y() + g.height()) > y_max) {
			y_max = g.y() + g.height();
		}
	}
	if (((*x) == 0) || ((*x) < x_min) || ((*x) > x_max)) {
		(*x) = 80;
	}
	if (((*y) == 0) || ((*y) < y_min) || ((*y) > y_max)) {
		(*y) = 80;
	}
}
