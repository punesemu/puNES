#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.13])

AC_INIT([puNES],[1.0])

# Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_BUILD

AM_INIT_AUTOMAKE(subdir-objects foreign [-Wall])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR

# Required macros for xdelta stubbed config.h
AC_CHECK_SIZEOF(size_t)
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long)
AC_CHECK_SIZEOF(unsigned long long)

AC_ARG_WITH([d3d9],
	AS_HELP_STRING([--with-d3d9],
		[Build with support for D3D9 (only windows) @<:@default: yes@:>@]
	),
	[with_d3d9=$withval], [with_d3d9=yes])
AC_ARG_WITH([opengl],
	AS_HELP_STRING([--with-opengl],
		[Build with support for OpenGL (only windows) @<:@default: no@:>@)]
	),
	[with_opengl=$withval], [with_opengl=no])
AC_ARG_WITH([opengl-nvidia-cg],
	AS_HELP_STRING([--with-opengl-nvidia-cg],
		[Build with support for Nvidia CG Toolkit for OpenGL (only linux) @<:@default: yes@:>@)]
	),
	[with_opengl_nvidia_cg=$withval], [with_opengl_nvidia_cg=yes])
AC_ARG_WITH([ffmpeg],
	AS_HELP_STRING([--with-ffmpeg],
		[Build with support for FFmpeg [autodetect] @<:@default: yes@:>@]
	),
	[with_ffmpeg=$withval], [with_ffmpeg=yes])

AC_ARG_ENABLE([release],
	AS_HELP_STRING([--disable-release],
		[Disable release version @<:@default: no@:>@]
	),
	[enable_release=$enableval], [enable_release=yes])

TOP_SRCDIR="$srcdir"
AS_CASE(["${TOP_SRCDIR}"],
	[[\/]*], [
		# Path is already absolute
	], [
		TOP_SRCDIR="${ac_pwd}/${TOP_SRCDIR}"
	]
)
TOP_BUILDDIR="$builddir"
AS_CASE(["${TOP_BUILDDIR}"],
	[[\/]*], [
		# Path is already absolute
	], [
		TOP_BUILDDIR="${ac_pwd}/${TOP_BUILDDIR}"
	]
)

my_CFLAGS=""
my_CPPFLAGS="-Wall -Wextra -ffast-math -fmessage-length=0 -finline-functions -I${TOP_SRCDIR}/src -I${TOP_SRCDIR}/src/gui -I${TOP_SRCDIR}/src/core -I${TOP_SRCDIR}/src/video/shaders"
my_CXXFLAGS="--param inline-unit-growth=200 --param large-function-growth=500"
my_LDFLAGS=""
my_LIBS=""
CUSTOMDEFINES=""
MOCDEFINES=""

# verifico l'host
AS_CASE(["$host_os"],
	[linux*], [
		MOCDEFINES="${MOCDEFINES} -D__linux__"
		CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_OPENGL -DGLEW_STATIC"
		my_CPPFLAGS="${my_CPPFLAGS} -I${TOP_SRCDIR}/src/gui/linux -I${TOP_SRCDIR}/src/video/opengl"
		linux=yes
		bsd=no
		windows=no
		with_d3d9=no
		with_opengl=yes
	],
	[freebsd*], [
		LDFLAGS="${LDFLAGS} -L/usr/local/lib"
		MOCDEFINES="${MOCDEFINES} -D__FreeBSD__"
		CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_OPENGL -DGLEW_STATIC"
		my_CPPFLAGS="${my_CPPFLAGS} -I${TOP_SRCDIR}/src/gui/bsd -I${TOP_SRCDIR}/src/video/opengl"
		linux=no
		bsd=yes
		windows=no
		with_d3d9=no
		with_opengl=yes
		my_CXXFLAGS=""
		export CC
		export CXX
	],
	[openbsd*], [
		LDFLAGS="${LDFLAGS} -L/usr/local/lib"
		MOCDEFINES="${MOCDEFINES} -D__OpenBSD__"
		CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_OPENGL -DGLEW_STATIC"
		my_CPPFLAGS="${my_CPPFLAGS} -I${TOP_SRCDIR}/src/gui/bsd -I${TOP_SRCDIR}/src/video/opengl"
		linux=no
		bsd=yes
		windows=no
		with_d3d9=no
		with_opengl=yes
		my_CXXFLAGS=""
		export CC
		export CXX
	],
	[mingw32*|msys], [
		MOCDEFINES="${MOCDEFINES} -D_WIN32"
		my_CPPFLAGS="${my_CPPFLAGS} -I${TOP_SRCDIR}/src/gui/windows"
		linux=no
		bsd=no
		windows=yes
		AS_IF([test "x$with_opengl" = xyes], [
			with_d3d9=no
			with_opengl_nvidia_cg=yes
			CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_OPENGL -DWITH_OPENGL_CG -DGLEW_STATIC"
			my_CPPFLAGS="${my_CPPFLAGS} -I${TOP_SRCDIR}/src/video/opengl"
		])
		AS_IF([test "x$with_d3d9" = xyes], [
			with_opengl=no
			with_opengl_nvidia_cg=no
			CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_D3D9"
			my_CPPFLAGS="${my_CPPFLAGS} -I${TOP_SRCDIR}/src/video/d3d9"
			my_LIBS="${my_LIBS} -ld3d9"
			AS_CASE(["$host_cpu"],
				[i?86], [
					my_LIBS="${my_LIBS} -ld3dx9"
				],
				[x86_64], [
					my_LIBS="${my_LIBS} -ld3dx9_41"
				],
			)
		])
	]
)

AM_CONDITIONAL(LINUX,[test "x$linux" = xyes])
AM_CONDITIONAL(BSD,[test "x$bsd" = xyes])
AM_CONDITIONAL(WINDOWS,[test "x$windows" = xyes])
AM_CONDITIONAL(ENAB_RELEASE,[test "x$enable_release" = xyes])
AM_CONDITIONAL(WITH_D3D9,[test "x$with_d3d9" = xyes])
AM_CONDITIONAL(WITH_OPENGL,[test "x$with_opengl" = xyes])
AM_CONDITIONAL(WITH_OPENGL_CG,[test "x$with_opengl_nvidia_cg" = xyes])
AM_CONDITIONAL(WITH_FFMPEG,[test "x$with_ffmpeg" = xyes])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
#AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([floor gettimeofday memmove memset mkdir pow setenv sqrt strcasecmp strrchr utime])

#AC_PATH_TOOL([PATCH], [patch], [no])
#AS_IF([test "x$PATCH" = "xno"], [
#	AC_MSG_ERROR([
#	*** The command patch could not be found. Make sure it is in your path.
#	])
#])
AC_PATH_TOOL([PKG_CONFIG], [pkg-config], [no])
AS_IF([test "x$PKG_CONFIG" = "xno"], [
	AC_MSG_ERROR([
	*** The pkg-config script could not be found. Make sure it is
	*** in your path, or set the PKG_CONFIG environment variable
	*** to the full path to pkg-config.
	])
])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h float.h inttypes.h limits.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/time.h unistd.h utime.h])

AX_CXX_COMPILE_STDCXX_14([noext])

OLD_CFLAGS="$CFLAGS"
OLD_LIBS="$LIBS"

# git
AM_CONDITIONAL(USE_GIT_INFO,[test x$WIPS_VERSION = xtrue])
AM_COND_IF([ENAB_RELEASE], [
	AM_COND_IF([USE_GIT_INFO], [
		AC_CHECK_TOOLS(GIT, [git], [no])
	], [
		GIT="no"
	])
	CUSTOMDEFINES="${CUSTOMDEFINES} -DRELEASE"
	], [
	AC_CHECK_TOOLS(GIT, [git], [no])
])
AM_CONDITIONAL(WITH_GIT,[test "x$GIT" != xno])
AM_COND_IF([WITH_GIT], [
	CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_GIT_INFO"
])

# cmake
AC_CHECK_PROG(CMAKE, cmake, yes)
AS_IF([test "x$CMAKE" != xyes], [
	AC_MSG_ERROR([Please install cmake before configuring.])
])

# Checks for libraries.
AM_COND_IF([WINDOWS], [
	# aggiungo path per include del DSDK
	my_CPPFLAGS="${my_CPPFLAGS} -I${TOP_SRCDIR}/src/extra/windows/DSDK/Include -I${TOP_SRCDIR}/src/extra/windows/DSDK/vc10/include"
	# aggiungo path per include e librerie del Toolkit Cg Nvidia
	my_CPPFLAGS="${my_CPPFLAGS} -I${TOP_SRCDIR}/src/extra/windows/Cg/include"
	AS_CASE(["$host_cpu"],
		[i?86], [
			my_LIBS="${my_LIBS} -L${TOP_SRCDIR}/src/extra/windows/Cg/lib"
		],
		[x86_64], [
			my_LIBS="${my_LIBS} -L${TOP_SRCDIR}/src/extra/windows/Cg/lib.x64"
		],
	)
	AS_IF([test "x$with_opengl" = xyes], [
		my_LIBS="${my_LIBS} -lcg -lcgGL"
	])
	AS_IF([test "x$with_d3d9" = xyes], [
		my_LIBS="${my_LIBS} -lcg -lcgD3D9"
	])
])

# Qt5
AM_COND_IF([WINDOWS], [
	OLD_PKG_CONFIG="$PKG_CONFIG"
	PKG_CONFIG="$PKG_CONFIG --static"
	PKG_CHECK_MODULES(QT5, [Qt5Core Qt5Gui Qt5Widgets Qt5Network Qt5Svg], [
		MOC="`$PKG_CONFIG --variable=host_bins Qt5Core`/moc"
		RCC="`$PKG_CONFIG --variable=host_bins Qt5Core`/rcc"
		UIC="`$PKG_CONFIG --variable=host_bins Qt5Core`/uic"
		LRELEASE="`$PKG_CONFIG --variable=host_bins Qt5Core`/lrelease"
	], [
		AC_MSG_ERROR([Qt5 library not found])
	])
	PKG_CONFIG="$OLD_PKG_CONFIG"

	QT5LIBDIR="`$PKG_CONFIG --variable=libdir Qt5Core`"
			
	AC_MSG_CHECKING([whether QT5 Plugin QWindows exists])
	AS_IF([test -f ${QT5LIBDIR}/plugins/platforms/libqwindows.a], [
		AC_MSG_RESULT([yes])
		CUSTOMDEFINES="${CUSTOMDEFINES} -DQT5_PLUGIN_QWINDOWS"
		my_LIBS="${my_LIBS} ${QT5LIBDIR}/plugins/platforms/libqwindows.a"
			
		AC_MSG_CHECKING([whether libQt5PlatformSupport.a exists])
		AS_IF([test -f ${QT5LIBDIR}/libQt5PlatformSupport.a], [
			AC_MSG_RESULT([yes])
			my_LIBS="${my_LIBS} ${QT5LIBDIR}/libQt5PlatformSupport.a"
		], [
			AC_MSG_RESULT([no])
		])

		AC_MSG_CHECKING([whether libQt5EventDispatcherSupport.a exists])
		AS_IF([test -f ${QT5LIBDIR}/libQt5EventDispatcherSupport.a], [
			AC_MSG_RESULT([yes])
			my_LIBS="${my_LIBS} ${QT5LIBDIR}/libQt5EventDispatcherSupport.a"
		], [
			AC_MSG_RESULT([no])
		])

		AC_MSG_CHECKING([whether libQt5FontDatabaseSupport.a exists])
		AS_IF([test -f ${QT5LIBDIR}/libQt5FontDatabaseSupport.a], [
			AC_MSG_RESULT([yes])
			my_LIBS="${my_LIBS} ${QT5LIBDIR}/libQt5FontDatabaseSupport.a"
		], [
			AC_MSG_RESULT([no])
		])

		AC_MSG_CHECKING([whether libqtfreetype.a exists])
		AS_IF([test -f ${QT5LIBDIR}/libqtfreetype.a], [
			AC_MSG_RESULT([yes])
			my_LIBS="${my_LIBS} ${QT5LIBDIR}/libqtfreetype.a"
		], [
			AC_MSG_RESULT([no])
		])

		AC_MSG_CHECKING([whether libQt5ThemeSupport.a exists])
		AS_IF([test -f ${QT5LIBDIR}/libQt5ThemeSupport.a], [
			AC_MSG_RESULT([yes])
			my_LIBS="${my_LIBS} ${QT5LIBDIR}/libQt5ThemeSupport.a"
		], [
			AC_MSG_RESULT([no])
		])
	], [
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([libqwindows.a not found])
	])

	AC_MSG_CHECKING([whether QT5 Plugin QWindowsVistaStyle exists])
	AS_IF([test -f ${QT5LIBDIR}/plugins/styles/libqwindowsvistastyle.a], [
		AC_MSG_RESULT([yes])
		CUSTOMDEFINES="${CUSTOMDEFINES} -DQT_PLUGIN_QWINDOWSVISTASTYLE"
		my_LIBS="${my_LIBS} ${QT5LIBDIR}/plugins/styles/libqwindowsvistastyle.a"
			
		AC_CHECK_LIB(uxtheme, main, [
			my_LIBS="${my_LIBS} -luxtheme"
		])
	], [
		AC_MSG_RESULT([no])
	])

	AC_MSG_CHECKING([whether QT5 Plugin QGif exists])
	AS_IF([test -f ${QT5LIBDIR}/plugins/imageformats/libqgif.a], [
		AC_MSG_RESULT([yes])
		CUSTOMDEFINES="${CUSTOMDEFINES} -DQT_PLUGIN_QGIF"
		my_LIBS="${my_LIBS} ${QT5LIBDIR}/plugins/imageformats/libqgif.a"
	], [
		AC_MSG_RESULT([no])
	])

	AC_MSG_CHECKING([whether QT5 Plugin QICO exists])
	AS_IF([test -f ${QT5LIBDIR}/plugins/imageformats/libqico.a], [
		AC_MSG_RESULT([yes])
		CUSTOMDEFINES="${CUSTOMDEFINES} -DQT_PLUGIN_QICO"
		my_LIBS="${my_LIBS} ${QT5LIBDIR}/plugins/imageformats/libqico.a"
	], [
		AC_MSG_RESULT([no])
	])

	AC_MSG_CHECKING([whether QT5 Plugin QJpeg exists])
	AS_IF([test -f ${QT5LIBDIR}/plugins/imageformats/libqjpeg.a], [
		AC_MSG_RESULT([yes])
		CUSTOMDEFINES="${CUSTOMDEFINES} -DQT_PLUGIN_QJPEG"
		my_LIBS="${my_LIBS} ${QT5LIBDIR}/plugins/imageformats/libqjpeg.a"
	], [
		AC_MSG_RESULT([no])
	])

	AC_MSG_CHECKING([whether QT5 Plugin QSvg exists])
	AS_IF([test -f ${QT5LIBDIR}/plugins/imageformats/libqsvg.a], [
		AC_MSG_RESULT([yes])
		CUSTOMDEFINES="${CUSTOMDEFINES} -DQT_PLUGIN_QSVG"
		my_LIBS="${my_LIBS} ${QT5LIBDIR}/plugins/imageformats/libqsvg.a"
	], [
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([libqsvg.a not found])
	])
], [
	PKG_CHECK_MODULES(QT5, [Qt5Core Qt5Gui Qt5Widgets Qt5Network Qt5Svg], [
		MOC="`$PKG_CONFIG --variable=host_bins Qt5Core`/moc"
		RCC="`$PKG_CONFIG --variable=host_bins Qt5Core`/rcc"
		UIC="`$PKG_CONFIG --variable=host_bins Qt5Core`/uic"
		LRELEASE="`$PKG_CONFIG --variable=host_bins Qt5Core`/lrelease"
	], [
		AC_MSG_ERROR([Qt5 library not found])
	])
])

AC_SUBST([MOC])
AC_SUBST([RCC])
AC_SUBST([UIC])
AC_SUBST([LRELEASE])

my_CPPFLAGS="${my_CPPFLAGS} ${QT5_CFLAGS}"
my_LIBS="${my_LIBS} ${QT5_LIBS}"

AS_IF([test "x$MOC" = xno], [
	AC_MSG_ERROR([Could not find moc binaries])
])
AS_IF([test "x$UIC" = xno], [
	AC_MSG_ERROR([Could not find uic binaries])
])
AS_IF([test "x$RCC" = xno], [
	AC_MSG_ERROR([Could not find rcc binaries])
])
AS_IF([test "x$LRELEASE" = xno], [
	AC_MSG_ERROR([Could not find lrelease binaries])
])

# libudev
AM_COND_IF([LINUX], [
	AC_CHECK_HEADERS([libudev.h], [], [
		AC_MSG_ERROR([udev support requested but libudev header not installed])
	])
	AC_CHECK_LIB(udev, main, [
		my_LIBS="${my_LIBS} -ludev"
	], [
		AC_MSG_ERROR([libudev.so not found])
	])
])

# OpenGL
AM_COND_IF([WINDOWS], [
	my_LIBS="${my_LIBS} -lopengl32"
])

# NVidia CG toolkit
AM_COND_IF([LINUX], [
	PKG_CHECK_MODULES(GL, [gl], [
		my_CPPFLAGS="${my_CPPFLAGS} ${GL_CFLAGS}"
		my_LIBS="${my_LIBS} ${GL_LIBS}"
	], [
		AC_MSG_ERROR([gl not found])
	])
	AM_COND_IF([WITH_OPENGL_CG], [
		PKG_CHECK_MODULES(nvidia_cg_toolkit_pkgconfig, [nvidia-cg-toolkit-gl], [
			CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_OPENGL_CG"
			my_CPPFLAGS="${my_CPPFLAGS} ${nvidia_cg_toolkit_pkgconfig_CFLAGS}"
			my_LIBS="${my_LIBS} ${nvidia_cg_toolkit_pkgconfig_LIBS}"
		], [
			# Check if NVIDIA CG toolkit is present without pkg-config
			AC_MSG_CHECKING([NVIDIA CG toolkit is present])
			AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include "Cg/cg.h"
#include "Cg/cgGL.h"
void blah(){
	CGcontext cgx = cgCreateContext();
}])
			], [
				AC_MSG_RESULT([yes])
				CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_OPENGL_CG"
				my_LIBS="${my_LIBS} -lCg -lCgGL"
			], [
				AC_MSG_RESULT([no])
			])
		])
	])
	PKG_CHECK_MODULES(alsa, [alsa])
	my_LIBS="${my_LIBS} ${alsa_LIBS}"
])

AM_COND_IF([BSD], [
	PKG_CHECK_MODULES(GL, [gl], [
		my_CPPFLAGS="${my_CPPFLAGS} ${GL_CFLAGS}"
		my_LIBS="${my_LIBS} ${GL_LIBS}"
	], [
		AC_MSG_ERROR([gl not found])
	])
	AM_COND_IF([WITH_OPENGL_CG], [
		PKG_CHECK_MODULES(nvidia_cg_toolkit_pkgconfig, [nvidia-cg-toolkit-gl], [
			CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_OPENGL_CG"
			my_CPPFLAGS="${my_CPPFLAGS} ${nvidia_cg_toolkit_pkgconfig_CFLAGS}"
			my_LIBS="${my_LIBS} ${nvidia_cg_toolkit_pkgconfig_LIBS}"
		], [
			# Check if NVIDIA CG toolkit is present without pkg-config
			AC_MSG_CHECKING([NVIDIA CG toolkit is present])
			AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include "Cg/cg.h"
#include "Cg/cgGL.h"
void blah(){
	CGcontext cgx = cgCreateContext();
}])
			], [
				AC_MSG_RESULT([yes])
				CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_OPENGL_CG"
				my_LIBS="${my_LIBS} -lCg -lCgGL"
			], [
				AC_MSG_RESULT([no])
			])
		])
	])
	AC_CHECK_LIB(sndio, main, [
		my_LIBS="${my_LIBS} -lsndio"
	], [
		AC_MSG_ERROR([libsdnio.so not found])
	])
])

# Management fullscreen resolution and auto frequency
AS_CASE(["$host_os"],
	[linux*|*bsd*], [
		PKG_CHECK_MODULES(FULLSCRFREQ, [x11 xrandr], [
			my_CPPFLAGS="${my_CPPFLAGS} ${FULLSCRFREQ_CFLAGS}"
			my_LIBS="${my_LIBS} ${FULLSCRFREQ_LIBS}"
			FULLSCREEN_RESFREQ="yes"
		], [
			AC_MSG_RESULT([fullscreen resolution and auto frequency disabled])
			FULLSCREEN_RESFREQ="no"
		])
	],
	[mingw32*|msys], [
		FULLSCREEN_RESFREQ="yes"
	], [
		FULLSCREEN_RESFREQ="no"
	]
)
AM_CONDITIONAL(USE_FULLSCREEN_RESFREQ,[test "x$FULLSCREEN_RESFREQ" != xno])
AM_COND_IF([USE_FULLSCREEN_RESFREQ], [
	CUSTOMDEFINES="${CUSTOMDEFINES} -DFULLSCREEN_RESFREQ"
])

# ffmpeg
AM_COND_IF([WITH_FFMPEG], [
	AM_COND_IF([WINDOWS], [
		AS_CASE(["$host_cpu"],
			[i?86], [
				PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${TOP_SRCDIR}/src/extra/windows/ffmpeg/lib/pkgconfig"
			],
			[x86_64], [
				PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${TOP_SRCDIR}/src/extra/windows/ffmpeg/lib.x64/pkgconfig"
			],
		)
	])
	PKG_CHECK_MODULES(ffmpeg, [libavcodec >= 58.18.100 libavutil >= 56.14.100 libavformat >= 58.12.100 libswscale >= 5.1.100 libswresample >= 3.1.100], [
		CUSTOMDEFINES="${CUSTOMDEFINES} -DWITH_FFMPEG"
		my_CPPFLAGS="${my_CPPFLAGS} ${ffmpeg_CFLAGS}"
		my_LIBS="${my_LIBS} ${ffmpeg_LIBS}"
	], [
		with_ffmpeg=no
	])
])
AM_CONDITIONAL(WITH_FFMPEG,[test "x$with_ffmpeg" = xyes])
AC_SUBST([WITH_FFMPEG])

# BSD gamepads
AS_CASE(["$host_os"],
	[*bsd*], [
		AC_CHECK_LIB(usbhid, hid_init, have_libusbhid=yes)
		AS_IF([test "x$have_libusbhid" = xyes], [
			AC_CHECK_HEADER(usbhid.h, [CUSTOMDEFINES="${CUSTOMDEFINES} -DHAVE_USBHID_H"])
			AC_CHECK_HEADER(libusbhid.h, [CUSTOMDEFINES="${CUSTOMDEFINES} -DHAVE_LIBUSBHID_H"])
			my_LIBS="${my_LIBS} -lusbhid"
		], [
			AC_CHECK_HEADER(usb.h, [CUSTOMDEFINES="${CUSTOMDEFINES} -DHAVE_USB_H"])
			AC_CHECK_HEADER(libusb.h, [CUSTOMDEFINES="${CUSTOMDEFINES} -DHAVE_LIBUSB_H"])
			AC_CHECK_LIB(usb, hid_init, [my_LIBS="${my_LIBS} -lusb"])
		])

		save_CFLAGS="${CFLAGS}"
		CFLAGS="${CFLAGS} ${CUSTOMDEFINES}"

		have_usbhid=no
		AC_MSG_CHECKING([for usbhid])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#include <sys/types.h>
			#if defined(HAVE_USB_H)
			#include <usb.h>
			#endif
			#ifdef __DragonFly__
			# include <bus/usb/usb.h>
			# include <bus/usb/usbhid.h>
			#else
			# include <dev/usb/usb.h>
			# include <dev/usb/usbhid.h>
			#endif
			#if defined(HAVE_USBHID_H)
			#include <usbhid.h>
			#elif defined(HAVE_LIBUSB_H)
			#include <libusb.h>
			#elif defined(HAVE_LIBUSBHID_H)
			#include <libusbhid.h>
			#endif
		]], [[
			struct report_desc *repdesc;
			struct usb_ctl_report *repbuf;
			hid_kind_t hidkind;
		]])],[
			have_usbhid=yes
		],[])
		AC_MSG_RESULT([$have_usbhid])

		AS_IF([test "x$have_usbhid" = xyes], [
			have_usbhid_ucr_data=no
			AC_MSG_CHECKING([for ucr_data member of usb_ctl_report])
			AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
				#include <sys/types.h>
				#if defined(HAVE_USB_H)
				#include <usb.h>
				#endif
				#ifdef __DragonFly__
				# include <bus/usb/usb.h>
				# include <bus/usb/usbhid.h>
				#else
				# include <dev/usb/usb.h>
				# include <dev/usb/usbhid.h>
				#endif
				#if defined(HAVE_USBHID_H)
				#include <usbhid.h>
				#elif defined(HAVE_LIBUSB_H)
				#include <libusb.h>
				#elif defined(HAVE_LIBUSBHID_H)
				#include <libusbhid.h>
				#endif
			]], [[
				struct usb_ctl_report buf;
				if (buf.ucr_data) { }
			]])],[
				have_usbhid_ucr_data=yes
			],[])
			AS_IF([test "x$have_usbhid_ucr_data" = xyes], [
				CUSTOMDEFINES="${CUSTOMDEFINES} -DUSBHID_UCR_DATA"
			])
			AC_MSG_RESULT([$have_usbhid_ucr_data])

			have_usbhid_new=no
			AC_MSG_CHECKING([for new usbhid API])
			AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
				#include <sys/types.h>
				#if defined(HAVE_USB_H)
				#include <usb.h>
				#endif
				#ifdef __DragonFly__
				#include <bus/usb/usb.h>
				#include <bus/usb/usbhid.h>
				#else
				#include <dev/usb/usb.h>
				#include <dev/usb/usbhid.h>
				#endif
				#if defined(HAVE_USBHID_H)
				#include <usbhid.h>
				#elif defined(HAVE_LIBUSB_H)
				#include <libusb.h>
				#elif defined(HAVE_LIBUSBHID_H)
				#include <libusbhid.h>
				#endif
			]], [[
				report_desc_t d;
				hid_start_parse(d, 1, 1);
			]])],[
				have_usbhid_new=yes
			],[])
			AS_IF([test "x$have_usbhid_new" = xyes], [
				CUSTOMDEFINES="${CUSTOMDEFINES} -DUSBHID_NEW"
			])
			AC_MSG_RESULT([$have_usbhid_new])

			have_machine_joystick=no
			AC_MSG_CHECKING([for struct joystick in machine/joystick.h])
			AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
				#include <machine/joystick.h>
			]], [[
				struct joystick t;
			]])],[
				have_machine_joystick=yes
			],[])
			AS_IF([test "x$have_machine_joystick" = xyes], [
				CUSTOMDEFINES="${CUSTOMDEFINES} -DUSBHID_MACHINE_JOYSTICK"
			])
			AC_MSG_RESULT([$have_machine_joystick])
		])
		CFLAGS="$save_CFLAGS"
	]
)

# windres
AS_CASE(["$host_os"],
	[mingw32*|msys], [
		AC_CHECK_TOOL(WINDRES, windres, [no])
	], [
		WINDRES=no
	]
)
AM_CONDITIONAL(HAVE_WINDRES, [test x$WINDRES != xno])
AC_SUBST(WINDRES)

# lib7zip
LIB7ZIP_SRC="lib7zip-53abfeb"
LIB7ZIP_P7ZIP="p7zip-17.04"
LIB7ZIP_7Z="7z1900"
AM_COND_IF([WINDOWS], [
	export P7ZIP_SOURCE_DIR="${TOP_SRCDIR}/src/extra/${LIB7ZIP_7Z}"
], [
	export P7ZIP_SOURCE_DIR="${TOP_SRCDIR}/src/extra/${LIB7ZIP_P7ZIP}"
])
AC_CONFIG_SUBDIRS([src/extra/lib7zip-53abfeb])
LIB7ZIP_LIBS="${TOP_BUILDDIR}src/extra/${LIB7ZIP_SRC}/src/lib7zip.a"
AC_SUBST(LIB7ZIP_SRC)
AC_SUBST(LIB7ZIP_LIBS)

# The End
AM_COND_IF([LINUX], [
	my_LIBS="${my_LIBS} -ldl -lpthread"
])
AM_COND_IF([BSD], [
	my_LIBS="${my_LIBS} -lc++ -lpthread"
])
AM_COND_IF([WINDOWS], [
	AM_COND_IF([ENAB_RELEASE], [
		my_LDFLAGS="${my_LDFLAGS} -Wl,-subsystem,windows"
	], [
		my_LDFLAGS="${my_LDFLAGS} -Wl,-subsystem,console"
	])
	my_LDFLAGS="${my_LDFLAGS} -static-libgcc -static-libstdc++"
	my_LIBS="${my_LIBS} -lwinmm -luuid -lole32 -lws2_32 -ladvapi32 -lshell32 -luser32 -lkernel32 -lgdi32 -loleaut32 -lcomdlg32 -limm32 -lwinspool -lwtsapi32"
])

AC_SUBST(CUSTOMDEFINES)
AC_SUBST(MOCDEFINES)

CFLAGS="$CFLAGS ${CUSTOMDEFINES} ${my_CPPFLAGS} ${my_CFLAGS}"
CXXFLAGS="$CXXFLAGS ${CUSTOMDEFINES} ${my_CPPFLAGS} ${my_CXXFLAGS}"
LDFLAGS="$LDFLAGS ${my_LDFLAGS}"
LIBS="$LIBS ${my_LIBS}"

AC_CONFIG_FILES([
Makefile
misc/com.github.punesemu.puNES.desktop
misc/Makefile
src/Makefile
src/audio/Makefile
src/c++/Makefile
src/extra/Makefile
src/core/Makefile
src/gui/Makefile
src/video/Makefile
])
AC_OUTPUT
