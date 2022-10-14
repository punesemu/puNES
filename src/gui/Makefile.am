qrc_resources.cpp: designer/resources.qrc
	$(RCC) $(srcdir)/designer/resources.qrc -name resources -o $@

qrc_translations.cpp: designer/translations.qrc $(QM_SOURCES)
	$(RCC) $(srcdir)/designer/translations.qrc -name translations -o $@

.ts.qm:
	$(LRELEASE) -silent $< -qm $@

singleapplication_moc.cpp: ../extra/singleapplication/singleapplication.h
	$(MOC) $(srcdir)/../extra/singleapplication/singleapplication.h $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
singleapplication_p_moc.cpp: ../extra/singleapplication/singleapplication_p.h
	$(MOC) $(srcdir)/../extra/singleapplication/singleapplication_p.h $(MOCDEFINES) $(CUSTOMDEFINES) -o $@

.hpp_moc.cpp:
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@

ui_mainWindow.h: designer/mainWindow.ui
	$(UIC) $< -o $@
ui_dlgCfgNSCode.h: designer/dlgCfgNSCode.ui
	$(UIC) $< -o $@
ui_dlgCmdLineHelp.h: designer/dlgCmdLineHelp.ui
	$(UIC) $< -o $@
ui_dlgJsc.h: designer/dlgJsc.ui
	$(UIC) $< -o $@
ui_dlgKeyboard.h: designer/dlgKeyboard.ui
	$(UIC) $< -o $@
ui_dlgSettings.h: designer/dlgSettings.ui
	$(UIC) $< -o $@
ui_dlgStdPad.h: designer/dlgStdPad.ui
	$(UIC) $< -o $@
ui_dlgUncomp.h: designer/dlgUncomp.ui
	$(UIC) $< -o $@
ui_dlgVsSystem.h: designer/dlgVsSystem.ui
	$(UIC) $< -o $@
ui_wdgAPUChannels.h: designer/wdgAPUChannels.ui
	$(UIC) $< -o $@
ui_wdgKeyboardFB.h: designer/wdgKeyboardFB.ui
	$(UIC) $< -o $@
ui_wdgKeyboardSubor.h: designer/wdgKeyboardSubor.ui
	$(UIC) $< -o $@
ui_wdgCheatsEditor.h: designer/wdgCheatsEditor.ui
	$(UIC) $< -o $@
ui_wdgNTSCFilter.h: designer/wdgNTSCFilter.ui
	$(UIC) $< -o $@
ui_wdgOverlayUi.h: designer/wdgOverlayUi.ui
	$(UIC) $< -o $@
ui_wdgPaletteEditor.h: designer/wdgPaletteEditor.ui
	$(UIC) $< -o $@
ui_wdgRewind.h: designer/wdgRewind.ui
	$(UIC) $< -o $@
ui_wdgRotateScreen.h: designer/wdgRotateScreen.ui
	$(UIC) $< -o $@
ui_wdgSettingsAudio.h: designer/wdgSettingsAudio.ui
	$(UIC) $< -o $@
ui_wdgSettingsCheats.h: designer/wdgSettingsCheats.ui
	$(UIC) $< -o $@
ui_wdgSettingsGeneral.h: designer/wdgSettingsGeneral.ui
	$(UIC) $< -o $@
ui_wdgSettingsInput.h: designer/wdgSettingsInput.ui
	$(UIC) $< -o $@
ui_wdgSettingsPPU.h: designer/wdgSettingsPPU.ui
	$(UIC) $< -o $@
ui_wdgSettingsRecording.h: designer/wdgSettingsRecording.ui
	$(UIC) $< -o $@
ui_wdgSettingsVideo.h: designer/wdgSettingsVideo.ui
	$(UIC) $< -o $@
ui_wdgState.h: designer/wdgState.ui
	$(UIC) $< -o $@

clean-local:
	rm -f qrc_*.cpp *_moc.cpp ui_*.h $(srcdir)/designer/translations/*.qm

MOC_SOURCES = \
	dlgCmdLineHelp_moc.cpp \
	dlgJsc_moc.cpp \
	dlgKeyboard_moc.cpp \
	dlgSettings_moc.cpp \
	dlgStdPad_moc.cpp \
	dlgUncomp_moc.cpp \
	dlgVsSystem_moc.cpp \
	mainApplication_moc.cpp \
	mainWindow_moc.cpp \
	objCheat_moc.cpp \
	objSettings_moc.cpp \
	singleapplication_moc.cpp \
	singleapplication_p_moc.cpp \
	wdgAPUChannels_moc.cpp \
	wdgCheatsEditor_moc.cpp \
	wdgNTSCFilter_moc.cpp \
	wdgOverlayUi_moc.cpp \
	wdgPaletteEditor_moc.cpp \
	wdgRewind_moc.cpp \
	wdgRotateScreen_moc.cpp \
	wdgScreen_moc.cpp \
	wdgSettingsAudio_moc.cpp \
	wdgSettingsCheats_moc.cpp \
	wdgSettingsGeneral_moc.cpp \
	wdgSettingsInput_moc.cpp \
	wdgSettingsPPU_moc.cpp \
	wdgSettingsRecording_moc.cpp \
	wdgSettingsVideo_moc.cpp \
	wdgState_moc.cpp \
	wdgStatusBar_moc.cpp \
	wdgToolBar_moc.cpp

if WITH_OPENGL
MOC_SOURCES += \
	wdgOpenGL_moc.cpp
endif

QM_SOURCES = \
	$(srcdir)/designer/translations/en_EN.qm \
	$(srcdir)/designer/translations/tr_TR.qm \
	$(srcdir)/designer/translations/it_IT.qm \
	$(srcdir)/designer/translations/qt_it.qm \
	$(srcdir)/designer/translations/es_ES.qm \
	$(srcdir)/designer/translations/qt_es.qm \
	$(srcdir)/designer/translations/hu_HU.qm \
	$(srcdir)/designer/translations/qt_hu.qm \
	$(srcdir)/designer/translations/ru_RU.qm \
	$(srcdir)/designer/translations/qt_ru.qm \
	$(srcdir)/designer/translations/pt_BR.qm \
	$(srcdir)/designer/translations/qt_pt.qm \
	$(srcdir)/designer/translations/zh_CN.qm \
	$(srcdir)/designer/translations/qt_zh_CN.qm

QRC_SOURCES = \
	qrc_resources.cpp \
	qrc_translations.cpp

UI_SOURCES = \
	ui_dlgCfgNSCode.h \
	ui_dlgCmdLineHelp.h \
	ui_dlgJsc.h \
	ui_dlgKeyboard.h \
	ui_dlgSettings.h \
	ui_dlgStdPad.h \
	ui_dlgUncomp.h \
	ui_dlgVsSystem.h \
	ui_mainWindow.h \
	ui_wdgAPUChannels.h \
	ui_wdgCheatsEditor.h \
	ui_wdgKeyboardFB.h \
	ui_wdgKeyboardSubor.h \
	ui_wdgNTSCFilter.h \
	ui_wdgOverlayUi.h \
	ui_wdgPaletteEditor.h \
	ui_wdgRewind.h \
	ui_wdgRotateScreen.h \
	ui_wdgSettingsAudio.h \
	ui_wdgSettingsCheats.h \
	ui_wdgSettingsGeneral.h \
	ui_wdgSettingsInput.h \
	ui_wdgSettingsPPU.h \
	ui_wdgSettingsRecording.h \
	ui_wdgSettingsVideo.h \
	ui_wdgState.h

BUILT_SOURCES = \
	$(MOC_SOURCES) \
	$(UI_SOURCES) \
	$(QM_SOURCES) \
	$(QRC_SOURCES)

noinst_LIBRARIES = libgui.a

libgui_a_SOURCES = \
	$(MOC_SOURCES) \
	$(UI_SOURCES) \
	$(QRC_SOURCES) \
	cgp.cpp \
	cgp.h \
	cmd_line.cpp \
	cmd_line.h \
	dlgCmdLineHelp.cpp \
	dlgCmdLineHelp.hpp \
	dlgJsc.cpp \
	dlgJsc.hpp \
	dlgKeyboard.cpp \
	dlgKeyboard.hpp \
	dlgSettings.cpp \
	dlgSettings.hpp \
	dlgStdPad.cpp \
	dlgStdPad.hpp \
	dlgUncomp.cpp \
	dlgUncomp.hpp \
	dlgVsSystem.cpp \
	dlgVsSystem.hpp \
	../extra/qkeycode/src/qkeycode/chromium/keycode_converter.cc \
	../extra/qkeycode/src/qkeycode/qkeycode.cpp \
	../extra/qkeycode/include/qkeycode/qkeycode.h \
	../extra/qkeycode/include/qkeycode/values.h \
	../extra/qkeycode/include/qkeycode/chromium/dom_code.h \
	../extra/qkeycode/include/qkeycode/chromium/keycode_converter.h \
	../extra/qkeycode/include/qkeycode/chromium/macros.h \
	../extra/singleapplication/singleapplication.cpp \
	../extra/singleapplication/singleapplication.h \
	../extra/singleapplication/singleapplication_p.cpp \
	../extra/singleapplication/singleapplication_p.h \
	mainApplication.cpp \
	mainApplication.hpp \
	mainWindow.cpp \
	mainWindow.hpp \
	nscode.hpp \
	objCheat.cpp \
	objCheat.hpp \
	objSettings.cpp \
	objSettings.hpp \
	qt.cpp \
	qt.h \
	recent_roms.cpp \
	recent_roms.h \
	settings.cpp \
	settings.h \
	tas.cpp \
	tas.h \
	wdgAPUChannels.cpp \
	wdgAPUChannels.hpp \
	wdgCheatsEditor.cpp \
	wdgCheatsEditor.hpp \
	wdgMenuBar.cpp \
	wdgMenuBar.hpp \
	wdgNTSCFilter.cpp \
	wdgNTSCFilter.hpp \
	wdgOverlayUi.cpp \
	wdgOverlayUi.hpp \
	wdgPaletteEditor.cpp \
	wdgPaletteEditor.hpp \
	wdgRewind.cpp \
	wdgRewind.hpp \
	wdgRotateScreen.cpp \
	wdgRotateScreen.hpp \
	wdgScreen.cpp \
	wdgScreen.hpp \
	wdgSettingsAudio.cpp \
	wdgSettingsAudio.hpp \
	wdgSettingsCheats.cpp \
	wdgSettingsCheats.hpp \
	wdgSettingsGeneral.cpp \
	wdgSettingsGeneral.hpp \
	wdgSettingsInput.cpp \
	wdgSettingsInput.hpp \
	wdgSettingsPPU.cpp \
	wdgSettingsPPU.hpp \
	wdgSettingsRecording.cpp \
	wdgSettingsRecording.hpp \
	wdgSettingsVideo.cpp \
	wdgSettingsVideo.hpp \
	wdgState.cpp \
	wdgState.hpp \
	wdgStatusBar.cpp \
	wdgStatusBar.hpp \
	wdgToolBar.cpp \
	wdgToolBar.hpp

if WITH_OPENGL
libgui_a_SOURCES += \
	wdgOpenGL.cpp \
	wdgOpenGL.hpp
endif

if WITH_D3D9
libgui_a_SOURCES += \
	wdgD3D9.cpp \
	wdgD3D9.hpp
endif

if LINUX
libgui_a_SOURCES += \
	linux/os_jstick.c \
	linux/os_jstick.h \
	linux/os_linux.h

if USE_FULLSCREEN_RESFREQ
libgui_a_SOURCES += \
	linux/monitor.c
endif

AM_CXXFLAGS = -fPIC
endif

if BSD
libgui_a_SOURCES += \
	bsd/os_jstick.c \
	bsd/os_jstick.h \
	bsd/os_bsd.h

if USE_FULLSCREEN_RESFREQ
libgui_a_SOURCES += \
	linux/monitor.c
endif

AM_CXXFLAGS = -fPIC
endif

if WINDOWS
libgui_a_SOURCES += \
	windows/os_jstick.c \
	windows/os_jstick.h \
	windows/os_windows.h \
	windows/win.h

if USE_FULLSCREEN_RESFREQ
libgui_a_SOURCES += \
	windows/monitor.c
endif
endif

SUFFIXES = .hpp _moc.cpp .qm .ts
