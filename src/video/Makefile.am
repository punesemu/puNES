noinst_LIBRARIES = libvideo.a
libvideo_a_SOURCES = \
	gfx.h \
	gfx_thread.c \
	gfx_thread.h \
	effects/pause.c \
	effects/pause.h \
	effects/tv_noise.c \
	effects/tv_noise.h \
	filters/hq2x.c \
	filters/hq3x.c \
	filters/hq4x.c \
	filters/hqx.c \
	filters/hqx.h \
	filters/hqx_inline.h \
	filters/nes_ntsc.c \
	filters/nes_ntsc.h \
	filters/ntsc.c \
	filters/ntsc.h \
	filters/scale2x.c \
	filters/scale2x.h \
	filters/scale.c \
	filters/scale.h \
	filters/xBRZ.c \
	filters/xBRZ.h \
	shaders/shaders.c \
	shaders/shaders.h

if WITH_OPENGL
libvideo_a_SOURCES += \
	opengl/gfx.c \
	opengl/opengl.c \
	opengl/opengl.h \
	opengl/shdcode.h \
	opengl/glew/glew.c \
	opengl/glew/glew.h \
	opengl/glew/glxew.h \
	opengl/glew/wglew.h
endif

if WITH_D3D9
libvideo_a_SOURCES += \
	d3d9/d3d9.c \
	d3d9/d3d9.h \
	d3d9/gfx.c \
	d3d9/shdcode.h
endif
