application.cpp: designer/application.qrc
	$(RCC) $< -name resources -o $@

cheatObject.moc: cheatObject.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgApuChannels.moc: dlgApuChannels.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgCheats.moc: dlgCheats.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgInput.moc: dlgInput.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgOverscanBorders.moc: dlgOverscanBorders.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgPPUHacks.moc: dlgPPUHacks.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgStdPad.moc: dlgStdPad.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgUncomp.moc: dlgUncomp.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgVsSystem.moc: dlgVsSystem.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
mainWindow.moc: mainWindow.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
pMenu.moc: pMenu.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
pStyle.moc: pStyle.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
sbarWidget.moc: sbarWidget.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
screenWidget.moc: screenWidget.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
settingsObject.moc: settingsObject.hpp
	$(MOC) $< $(MOCDEFINES) $(CUSTOMDEFINES) -o $@

application.hh: designer/application.ui
	$(UIC) $< -o $@
dlgOverscanBorders.hh: designer/dlgOverscanBorders.ui
	$(UIC) $< -o $@
dlgApuChannels.hh: designer/dlgApuChannels.ui
	$(UIC) $< -o $@
dlgStdPad.hh: designer/dlgStdPad.ui
	$(UIC) $< -o $@
dlgCheats.hh: designer/dlgCheats.ui
	$(UIC) $< -o $@
dlgUncomp.hh: designer/dlgUncomp.ui
	$(UIC) $< -o $@
dlgVsSystem.hh: designer/dlgVsSystem.ui
	$(UIC) $< -o $@
dlgPPUHacks.hh: designer/dlgPPUHacks.ui
	$(UIC) $< -o $@
dlgInput.hh: designer/dlgInput.ui
	$(UIC) $< -o $@

clean-local:
	rm -f application.cpp *.{hh,moc}

BUILT_SOURCES = \
	cheatObject.moc \
	dlgApuChannels.moc \
	dlgCheats.moc \
	dlgInput.moc \
	dlgOverscanBorders.moc \
	dlgPPUHacks.moc \
	dlgStdPad.moc \
	dlgUncomp.moc \
	dlgVsSystem.moc \
	mainWindow.moc \
	pMenu.moc \
	pStyle.moc \
	sbarWidget.moc \
	screenWidget.moc \
	settingsObject.moc

BUILT_SOURCES += \
	application.cpp

BUILT_SOURCES += \
	application.hh \
	dlgOverscanBorders.hh \
	dlgApuChannels.hh \
	dlgStdPad.hh \
	dlgCheats.hh \
	dlgUncomp.hh \
	dlgVsSystem.hh \
	dlgPPUHacks.hh \
	dlgInput.hh

noinst_LIBRARIES = libgui.a
libgui_a_SOURCES = \
	application.hh \
	cgp.cpp \
	cgp.h \
	cheatObject.cpp \
	cheatObject.hpp \
	cmd_line.cpp \
	cmd_line.h \
	dlgApuChannels.cpp \
	dlgApuChannels.hpp \
	dlgCheats.cpp \
	dlgCheats.hpp \
	dlgInput.cpp \
	dlgInput.hpp \
	dlgOverscanBorders.cpp \
	dlgOverscanBorders.hpp \
	dlgPPUHacks.cpp \
	dlgPPUHacks.hpp \
	dlgStdPad.cpp \
	dlgStdPad.hpp \
	dlgUncomp.cpp \
	dlgUncomp.hpp \
	dlgVsSystem.cpp \
	dlgVsSystem.hpp \
	mainWindow.cpp \
	mainWindow.hpp \
	pMenu.cpp \
	pMenu.hpp \
	pStyle.cpp \
	pStyle.hpp \
	qt.cpp \
	qt.h \
	recent_roms.cpp \
	recent_roms.h \
	sbarWidget.cpp \
	sbarWidget.hpp \
	screenWidget.cpp \
	screenWidget.hpp \
	settings.cpp \
	settings.h \
	settingsObject.cpp \
	settingsObject.hpp \
	tas.cpp \
	tas.h
	
if LINUX
libgui_a_SOURCES += \
	linux/jstick.c \
	linux/jstick.h \
	linux/os_linux.h

if ENAB_QT5
AM_CXXFLAGS = -fPIC
endif

endif

if WINDOWS
libgui_a_SOURCES += \
	windows/jstick.c \
	windows/jstick.h \
	windows/os_windows.h \
	windows/win.h
endif
