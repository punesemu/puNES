resources.cpp: designer/resources.qrc $(TRADUCTION_QM)
	$(RCC) $(srcdir)/designer/resources.qrc -name resources -o $@

dlgCmdLineHelp.moc: dlgCmdLineHelp.hpp
	$(MOC) $(srcdir)/dlgCmdLineHelp.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgSettings.moc: dlgSettings.hpp
	$(MOC) $(srcdir)/dlgSettings.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgStdPad.moc: dlgStdPad.hpp
	$(MOC) $(srcdir)/dlgStdPad.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgUncomp.moc: dlgUncomp.hpp
	$(MOC) $(srcdir)/dlgUncomp.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgVsSystem.moc: dlgVsSystem.hpp
	$(MOC) $(srcdir)/dlgVsSystem.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
mainWindow.moc: mainWindow.hpp
	$(MOC) $(srcdir)/mainWindow.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
objCheat.moc: objCheat.hpp
	$(MOC) $(srcdir)/objCheat.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
objSettings.moc: objSettings.hpp
	$(MOC) $(srcdir)/objSettings.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgAPUChannels.moc: wdgAPUChannels.hpp
	$(MOC) $(srcdir)/wdgAPUChannels.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgCheatsEditor.moc: wdgCheatsEditor.hpp
	$(MOC) $(srcdir)/wdgCheatsEditor.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
if WITH_OPENGL
wdgOpenGL.moc: wdgOpenGL.hpp
	$(MOC) $(srcdir)/wdgOpenGL.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
endif
wdgOverlayUi.moc: wdgOverlayUi.hpp
	$(MOC) $(srcdir)/wdgOverlayUi.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgPaletteEditor.moc: wdgPaletteEditor.hpp
	$(MOC) $(srcdir)/wdgPaletteEditor.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgRewind.moc: wdgRewind.hpp
	$(MOC) $(srcdir)/wdgRewind.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgRotateScreen.moc: wdgRotateScreen.hpp
	$(MOC) $(srcdir)/wdgRotateScreen.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgSettingsAudio.moc: wdgSettingsAudio.hpp
	$(MOC) $(srcdir)/wdgSettingsAudio.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgSettingsCheats.moc: wdgSettingsCheats.hpp
	$(MOC) $(srcdir)/wdgSettingsCheats.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgSettingsGeneral.moc: wdgSettingsGeneral.hpp
	$(MOC) $(srcdir)/wdgSettingsGeneral.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgSettingsInput.moc: wdgSettingsInput.hpp
	$(MOC) $(srcdir)/wdgSettingsInput.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgSettingsPPU.moc: wdgSettingsPPU.hpp
	$(MOC) $(srcdir)/wdgSettingsPPU.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgSettingsRecording.moc: wdgSettingsRecording.hpp
	$(MOC) $(srcdir)/wdgSettingsRecording.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@	
wdgSettingsVideo.moc: wdgSettingsVideo.hpp
	$(MOC) $(srcdir)/wdgSettingsVideo.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgScreen.moc: wdgScreen.hpp
	$(MOC) $(srcdir)/wdgScreen.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgState.moc: wdgState.hpp
	$(MOC) $(srcdir)/wdgState.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgStatusBar.moc: wdgStatusBar.hpp
	$(MOC) $(srcdir)/wdgStatusBar.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgToolBar.moc: wdgToolBar.hpp
	$(MOC) $(srcdir)/wdgToolBar.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@

mainWindow.hh: designer/mainWindow.ui
	$(UIC) $(srcdir)/designer/mainWindow.ui -o $@
dlgCmdLineHelp.hh: designer/dlgCmdLineHelp.ui
	$(UIC) $(srcdir)/designer/dlgCmdLineHelp.ui -o $@
dlgSettings.hh: designer/dlgSettings.ui
	$(UIC) $(srcdir)/designer/dlgSettings.ui -o $@
dlgStdPad.hh: designer/dlgStdPad.ui
	$(UIC) $(srcdir)/designer/dlgStdPad.ui -o $@
dlgUncomp.hh: designer/dlgUncomp.ui
	$(UIC) $(srcdir)/designer/dlgUncomp.ui -o $@
dlgVsSystem.hh: designer/dlgVsSystem.ui
	$(UIC) $(srcdir)/designer/dlgVsSystem.ui -o $@
wdgAPUChannels.hh: designer/wdgAPUChannels.ui
	$(UIC) $(srcdir)/designer/wdgAPUChannels.ui -o $@
wdgCheatsEditor.hh: designer/wdgCheatsEditor.ui
	$(UIC) $(srcdir)/designer/wdgCheatsEditor.ui -o $@
wdgOverlayUi.hh: designer/wdgOverlayUi.ui
	$(UIC) $(srcdir)/designer/wdgOverlayUi.ui -o $@
wdgPaletteEditor.hh: designer/wdgPaletteEditor.ui
	$(UIC) $(srcdir)/designer/wdgPaletteEditor.ui -o $@
wdgRewind.hh: designer/wdgRewind.ui
	$(UIC) $(srcdir)/designer/wdgRewind.ui -o $@
wdgRotateScreen.hh: designer/wdgRotateScreen.ui
	$(UIC) $(srcdir)/designer/wdgRotateScreen.ui -o $@
wdgSettingsAudio.hh: designer/wdgSettingsAudio.ui
	$(UIC) $(srcdir)/designer/wdgSettingsAudio.ui -o $@
wdgSettingsCheats.hh: designer/wdgSettingsCheats.ui
	$(UIC) $(srcdir)/designer/wdgSettingsCheats.ui -o $@
wdgSettingsGeneral.hh: designer/wdgSettingsGeneral.ui
	$(UIC) $(srcdir)/designer/wdgSettingsGeneral.ui -o $@
wdgSettingsInput.hh: designer/wdgSettingsInput.ui
	$(UIC) $(srcdir)/designer/wdgSettingsInput.ui -o $@
wdgSettingsPPU.hh: designer/wdgSettingsPPU.ui
	$(UIC) $(srcdir)/designer/wdgSettingsPPU.ui -o $@
wdgSettingsRecording.hh: designer/wdgSettingsRecording.ui
	$(UIC) $(srcdir)/designer/wdgSettingsRecording.ui -o $@
wdgSettingsVideo.hh: designer/wdgSettingsVideo.ui
	$(UIC) $(srcdir)/designer/wdgSettingsVideo.ui -o $@
wdgState.hh: designer/wdgState.ui
	$(UIC) $(srcdir)/designer/wdgState.ui -o $@

$(srcdir)/designer/translations/en_EN.qm: designer/translations/en_EN.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/en_EN.ts -qm $@
$(srcdir)/designer/translations/tr_TR.qm: designer/translations/tr_TR.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/tr_TR.ts -qm $@
$(srcdir)/designer/translations/it_IT.qm: designer/translations/it_IT.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/it_IT.ts -qm $@
$(srcdir)/designer/translations/qt_it.qm: designer/translations/qt_it.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/qt_it.ts -qm $@
$(srcdir)/designer/translations/es_ES.qm: designer/translations/es_ES.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/es_ES.ts -qm $@
$(srcdir)/designer/translations/qt_es.qm: designer/translations/qt_es.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/qt_es.ts -qm $@
$(srcdir)/designer/translations/hu_HU.qm: designer/translations/hu_HU.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/hu_HU.ts -qm $@
$(srcdir)/designer/translations/qt_hu.qm: designer/translations/qt_hu.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/qt_hu.ts -qm $@
$(srcdir)/designer/translations/ru_RU.qm: designer/translations/ru_RU.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/ru_RU.ts -qm $@
$(srcdir)/designer/translations/qt_ru.qm: designer/translations/qt_ru.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/qt_ru.ts -qm $@
$(srcdir)/designer/translations/pt_BR.qm: designer/translations/pt_BR.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/pt_BR.ts -qm $@
$(srcdir)/designer/translations/qt_pt.qm: designer/translations/qt_pt.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/qt_pt.ts -qm $@
$(srcdir)/designer/translations/zh_CN.qm: designer/translations/zh_CN.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/zh_CN.ts -qm $@
$(srcdir)/designer/translations/qt_zh_CN.qm: designer/translations/qt_zh_CN.ts
	$(LRELEASE) -silent $(srcdir)/designer/translations/qt_zh_CN.ts -qm $@

clean-local:
	rm -f resources.cpp *.{hh,moc} $(srcdir)/designer/translations/*.qm

TRADUCTION_QM = \
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

BUILT_SOURCES = \
	resources.cpp

BUILT_SOURCES += \
	dlgCmdLineHelp.moc \
	dlgSettings.moc \
	dlgStdPad.moc \
	dlgUncomp.moc \
	dlgVsSystem.moc \
	mainWindow.moc \
	objCheat.moc \
	objSettings.moc \
	wdgAPUChannels.moc \
	wdgCheatsEditor.moc \
	wdgOverlayUi.moc \
	wdgPaletteEditor.moc \
	wdgRewind.moc \
	wdgRotateScreen.moc \
	wdgSettingsAudio.moc \
	wdgSettingsCheats.moc \
	wdgSettingsGeneral.moc \
	wdgSettingsInput.moc \
	wdgSettingsPPU.moc \
	wdgSettingsRecording.moc \
	wdgSettingsVideo.moc \
	wdgScreen.moc \
	wdgState.moc \
	wdgStatusBar.moc \
	wdgToolBar.moc

BUILT_SOURCES += \
	mainWindow.hh \
	dlgCmdLineHelp.hh \
	dlgSettings.hh \
	dlgStdPad.hh \
	dlgUncomp.hh \
	dlgVsSystem.hh \
	wdgAPUChannels.hh \
	wdgCheatsEditor.hh \
	wdgOverlayUi.hh \
	wdgPaletteEditor.hh \
	wdgRewind.hh \
	wdgRotateScreen.hh \
	wdgSettingsAudio.hh \
	wdgSettingsCheats.hh \
	wdgSettingsGeneral.hh \
	wdgSettingsInput.hh \
	wdgSettingsPPU.hh \
	wdgSettingsRecording.hh \
	wdgSettingsVideo.hh \
	wdgState.hh

noinst_LIBRARIES = libgui.a
libgui_a_SOURCES = \
	cgp.cpp \
	cgp.h \
	cmd_line.cpp \
	cmd_line.h \
	dlgCmdLineHelp.cpp \
	dlgCmdLineHelp.hpp \
	dlgSettings.cpp \
	dlgSettings.hpp \
	dlgStdPad.cpp \
	dlgStdPad.hpp \
	dlgUncomp.cpp \
	dlgUncomp.hpp \
	dlgVsSystem.cpp \
	dlgVsSystem.hpp \
	mainWindow.cpp \
	mainWindow.hpp \
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
	wdgOverlayUi.cpp \
	wdgOverlayUi.hpp \
	wdgPaletteEditor.cpp \
	wdgPaletteEditor.hpp \
	wdgRewind.cpp \
	wdgRewind.hpp \
	wdgRotateScreen.cpp \
	wdgRotateScreen.hpp \
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
	wdgScreen.cpp \
	wdgScreen.hpp \
	wdgState.cpp \
	wdgState.hpp \
	wdgStatusBar.cpp \
	wdgStatusBar.hpp \
	wdgToolBar.cpp \
	wdgToolBar.hpp

if WITH_OPENGL
BUILT_SOURCES += \
	wdgOpenGL.moc

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
	linux/jstick.c \
	linux/jstick.h \
	linux/os_linux.h

if USE_FULLSCREEN_RESFREQ
libgui_a_SOURCES += \
	linux/monitor.c
endif

AM_CXXFLAGS = -fPIC
endif

if BSD
libgui_a_SOURCES += \
	bsd/jstick.c \
	bsd/jstick.h \
	bsd/os_bsd.h

if USE_FULLSCREEN_RESFREQ
libgui_a_SOURCES += \
	linux/monitor.c
endif

AM_CXXFLAGS = -fPIC
endif

if WINDOWS
libgui_a_SOURCES += \
	windows/jstick.c \
	windows/jstick.h \
	windows/os_windows.h \
	windows/win.h

if USE_FULLSCREEN_RESFREQ
libgui_a_SOURCES += \
	windows/monitor.c
endif
endif
