application.cpp: designer/application.qrc
	$(RCC) $(srcdir)/designer/application.qrc -name resources -o $@

cheatObject.moc: cheatObject.hpp
	$(MOC) $(srcdir)/cheatObject.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgApuChannels.moc: dlgApuChannels.hpp
	$(MOC) $(srcdir)/dlgApuChannels.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgCheats.moc: dlgCheats.hpp
	$(MOC) $(srcdir)/dlgCheats.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgInput.moc: dlgInput.hpp
	$(MOC) $(srcdir)/dlgInput.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgOverscanBorders.moc: dlgOverscanBorders.hpp
	$(MOC) $(srcdir)/dlgOverscanBorders.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgPPUHacks.moc: dlgPPUHacks.hpp
	$(MOC) $(srcdir)/dlgPPUHacks.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgStdPad.moc: dlgStdPad.hpp
	$(MOC) $(srcdir)/dlgStdPad.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgUncomp.moc: dlgUncomp.hpp
	$(MOC) $(srcdir)/dlgUncomp.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
dlgVsSystem.moc: dlgVsSystem.hpp
	$(MOC) $(srcdir)/dlgVsSystem.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
mainWindow.moc: mainWindow.hpp
	$(MOC) $(srcdir)/mainWindow.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
pMenu.moc: pMenu.hpp
	$(MOC) $(srcdir)/pMenu.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
pStyle.moc: pStyle.hpp
	$(MOC) $(srcdir)/pStyle.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
sbarWidget.moc: sbarWidget.hpp
	$(MOC) $(srcdir)/sbarWidget.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
screenWidget.moc: screenWidget.hpp
	$(MOC) $(srcdir)/screenWidget.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@
settingsObject.moc: settingsObject.hpp
	$(MOC) $(srcdir)/settingsObject.hpp $(MOCDEFINES) $(CUSTOMDEFINES) -o $@

application.hh: designer/application.ui
	$(UIC) $(srcdir)/designer/application.ui -o $@
dlgOverscanBorders.hh: designer/dlgOverscanBorders.ui
	$(UIC) $(srcdir)/designer/dlgOverscanBorders.ui -o $@
dlgApuChannels.hh: designer/dlgApuChannels.ui
	$(UIC) $(srcdir)/designer/dlgApuChannels.ui -o $@
dlgStdPad.hh: designer/dlgStdPad.ui
	$(UIC) $(srcdir)/designer/dlgStdPad.ui -o $@
dlgCheats.hh: designer/dlgCheats.ui
	$(UIC) $(srcdir)/designer/dlgCheats.ui -o $@
dlgUncomp.hh: designer/dlgUncomp.ui
	$(UIC) $(srcdir)/designer/dlgUncomp.ui -o $@
dlgVsSystem.hh: designer/dlgVsSystem.ui
	$(UIC) $(srcdir)/designer/dlgVsSystem.ui -o $@
dlgPPUHacks.hh: designer/dlgPPUHacks.ui
	$(UIC) $(srcdir)/designer/dlgPPUHacks.ui -o $@
dlgInput.hh: designer/dlgInput.ui
	$(UIC) $(srcdir)/designer/dlgInput.ui -o $@

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

if OPENBSD
libgui_a_SOURCES += \
	openbsd/jstick.c \
	openbsd/jstick.h \
	openbsd/os_openbsd.h

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
