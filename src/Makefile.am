SUBDIRS = audio c++ core gui video extra

if ENAB_QT5
AM_CXXFLAGS = -fPIC
endif

if HAVE_WINDRES
ico_stuff = gui/windows/resources.rc
endif

.rc.o:
	$(WINDRES) -o $@ $<

bin_PROGRAMS = punes

punes_SOURCES = \
	core/main.c \
	core/matrix.c \
	gui/application.cpp \
	core/palette.c \
	$(ico_stuff)

punes_LDADD = \
	core/libcore.a \
	video/libvideo.a \
	c++/libcplus.a \
	gui/libgui.a \
	audio/libaudio.a \
	$(LIB7ZIP_LIBS)
