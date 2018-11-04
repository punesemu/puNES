noinst_LIBRARIES = libaudio.a
libaudio_a_SOURCES = \
	blip_buf.c \
	blipbuf.c \
	blip_buf.h \
	blipbuf.h \
	channels.c \
	channels.h \
	delay.c \
	delay.h \
	handler.c \
	mono.c \
	mono.h \
	panning.c \
	panning.h \
	snd.h \
	wave.c \
	wave.h

if LINUX
libaudio_a_SOURCES += \
	alsa/snd.c
endif

if OPENBSD
libaudio_a_SOURCES += \
	sndio/snd.c
endif

if WINDOWS
libaudio_a_SOURCES += \
	xaudio/snd.c
endif
