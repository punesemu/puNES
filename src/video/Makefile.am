noinst_LIBRARIES = libvideo.a
libvideo_a_SOURCES = \
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
	filters/xBRZ.h

if WITH_OPENGL
libvideo_a_SOURCES += \
	sdl/gfx.c \
	sdl/opengl.c \
	sdl/opengl.h \
	sdl/sdl_wid.h \
	sdl/shaders.h
libvideo_a_SOURCES += \
	sdl/glew/glew.c \
	sdl/glew/glew.h \
	sdl/glew/glxew.h \
	sdl/glew/wglew.h
libvideo_a_SOURCES += \
	sdl/openGL/cube3d.c \
	sdl/openGL/cube3d.h \
	sdl/openGL/no_effect.c \
	sdl/openGL/no_effect.h
libvideo_a_SOURCES += \
	sdl/shaders/color.h \
	sdl/shaders/crt.h \
	sdl/shaders/dbl.h \
	sdl/shaders/interpolation.h \
	sdl/shaders/no_filter.h \
	sdl/shaders/phosphor.h \
	sdl/shaders/scanline.h
endif

if WITH_D3D9
libvideo_a_SOURCES += \
	d3d9/gfx.c \
	d3d9/shaders.h
libvideo_a_SOURCES += \
	d3d9/shaders/color.h \
	d3d9/shaders/crt.h \
	d3d9/shaders/dbl.h \
	d3d9/shaders/interpolation.h \
	d3d9/shaders/no_filter.h \
	d3d9/shaders/phosphor.h \
	d3d9/shaders/scanline.h
endif
