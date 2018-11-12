application.cpp: designer/application.qrc
	$(RCC) $(srcdir)/designer/application.qrc -name resources -o $@

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
wdgSettingsVideo.moc: wdgSettingsVideo.hpp
	$(MOC) $(srcdir)/wdgSettingsVideo.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgScreen.moc: wdgScreen.hpp
	$(MOC) $(srcdir)/wdgScreen.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
wdgStatusBar.moc: wdgStatusBar.hpp
	$(MOC) $(srcdir)/wdgStatusBar.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@

application.hh: designer/application.ui
	$(UIC) $(srcdir)/designer/application.ui -o $@
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
wdgSettingsVideo.hh: designer/wdgSettingsVideo.ui
	$(UIC) $(srcdir)/designer/wdgSettingsVideo.ui -o $@

clean-local:
	rm -f application.cpp *.{hh,moc}

BUILT_SOURCES = \
	dlgSettings.moc \
	dlgStdPad.moc \
	dlgUncomp.moc \
	dlgVsSystem.moc \
	mainWindow.moc \
	objCheat.moc \
	objSettings.moc \
	wdgAPUChannels.moc \
	wdgCheatsEditor.moc \
	wdgSettingsAudio.moc \
	wdgSettingsCheats.moc \
	wdgSettingsGeneral.moc \
	wdgSettingsInput.moc \
	wdgSettingsPPU.moc \
	wdgSettingsVideo.moc \
	wdgScreen.moc \
	wdgStatusBar.moc

BUILT_SOURCES += \
	application.cpp

BUILT_SOURCES += \
	application.hh \
	dlgSettings.hh \
	dlgStdPad.hh \
	dlgUncomp.hh \
	dlgVsSystem.hh \
	wdgAPUChannels.hh \
	wdgCheatsEditor.hh \
	wdgSettingsAudio.hh \
	wdgSettingsCheats.hh \
	wdgSettingsGeneral.hh \
	wdgSettingsInput.hh \
	wdgSettingsPPU.hh \
	wdgSettingsVideo.hh

noinst_LIBRARIES = libgui.a
libgui_a_SOURCES = \
	application.hh \
	cgp.cpp \
	cgp.h \
	cmd_line.cpp \
	cmd_line.h \
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
	wdgSettingsVideo.cpp \
	wdgSettingsVideo.hpp \
	wdgScreen.cpp \
	wdgScreen.hpp \
	wdgStatusBar.cpp \
	wdgStatusBar.hpp

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

AM_CXXFLAGS = -fPIC
endif

if OPENBSD
libgui_a_SOURCES += \
	openbsd/jstick.c \
	openbsd/jstick.h \
	openbsd/os_openbsd.h

AM_CXXFLAGS = -fPIC
endif

if WINDOWS
libgui_a_SOURCES += \
	windows/jstick.c \
	windows/jstick.h \
	windows/os_windows.h \
	windows/win.h
endif
