#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.13])

AC_INIT([puNES], [1.0])

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

AC_ARG_WITH([d3d9], AS_HELP_STRING([--with-d3d9], [Build with support for D3D9 (only windows) @<:@default: yes@:>@]), [with_d3d9=$withval], [with_d3d9=yes])
AC_ARG_WITH([opengl], AS_HELP_STRING([--with-opengl], [Build with support for OpenGL (only windows) @<:@default: no@:>@)]), [with_opengl=$withval], [with_opengl=no])
AC_ARG_WITH([dsdk], AS_HELP_STRING([--with-dsdk=path], [Path for DSDK (only windows)]), [path_dsdk=${with_dsdk}], [with_dsdk=no])
AC_ARG_WITH([qt-include], AS_HELP_STRING([--with-qt-include=@<:@PATH@:>@], [Force path for QT include]), [path_qt_inc=${with_qt_include}], [path_qt_inc=no])
AC_ARG_WITH([qt-lib], AS_HELP_STRING([--with-qt-lib=@<:@PATH@:>@], [Force path for QT libs]), [path_qt_lib=${with_qt_lib}], [path_qt_lib=no])
AC_ARG_WITH([qt-bin], AS_HELP_STRING([--with-qt-bin=@<:@PATH@:>@], [Force path for QT binaries]), [path_qt_bin=${with_qt_bin}], [path_qt_bin=no])
AC_ARG_WITH([sdl-include], AS_HELP_STRING([--with-sdl-include=@<:@PATH@:>@], [Force path for SDL include]), [path_sdl_inc=${with_sdl_include}], [path_sdl_inc=no])
AC_ARG_WITH([sdl-lib], AS_HELP_STRING([--with-sdl-lib=@<:@PATH@:>@], [Force path for SDL libs]), [path_sdl_lib=${with_sdl_lib}], [path_sdl_lib=no])
AC_ARG_WITH([lib7zip-precomp], AS_HELP_STRING([--with-lib7zip-precomp=@<:@PATH@:>@], [Use precompiled Lib7zip located in @<:@PATH@:>@]), [path_7zip_lib=${with_lib7zip_precomp}], [path_7zip_lib=no])

AC_ARG_ENABLE([qt5], AS_HELP_STRING([--enable-qt5], [Use QT5 instead of QT4]), [HAVE_QT5=yes], [HAVE_QT5=no])

TOP_SRCDIR="$srcdir"
AS_CASE(["${TOP_SRCDIR}"],
	[[\/]*],[
		# Path is already absolute
	],
	[
		TOP_SRCDIR="${ac_pwd}/${TOP_SRCDIR}"
	]
)
TOP_BUILDDIR="$builddir"
AS_CASE(["${TOP_BUILDDIR}"],
	[[\/]*],[
		# Path is already absolute
	],
	[
		TOP_BUILDDIR="${ac_pwd}/${TOP_BUILDDIR}"
	]
)

my_CFLAGS="-ffast-math -fmessage-length=0 -finline-functions -Winline"
my_CPPFLAGS="-Wall -ffast-math -I${TOP_SRCDIR}/src -I${TOP_SRCDIR}/src/gui -I${TOP_SRCDIR}/src/core"
my_CXXFLAGS="-fmessage-length=0 -finline-functions --param inline-unit-growth=200 --param large-function-growth=500"
my_LDFLAGS=""
my_LIBS=""

# verifico l'host
AS_CASE(["$host_os"],
	[linux*],[
		my_CPPFLAGS+=" -DSDL -DGLEW_STATIC -I${TOP_SRCDIR}/src/gui/linux -I${TOP_SRCDIR}/src/video/sdl"
		linux=yes
		windows=no
		with_d3d9=no
		with_opengl=yes
	],
	[mingw32*],[
		my_CPPFLAGS+=" -I${TOP_SRCDIR}/src/gui/windows"
		linux=no
		windows=yes
		AS_IF([test "x$with_opengl" = xyes],[
			with_d3d9=no
			my_CPPFLAGS+=" -DSDL -DGLEW_STATIC -I${TOP_SRCDIR}/src/video/sdl"
		])
		AS_IF([test "x$with_d3d9" = xyes],[
			with_opengl=no
			my_CPPFLAGS+=" -DD3D9"
			my_LIBS+=" -ld3d9"
			AS_CASE(["$host_cpu"],
				[i?86],[
					my_LIBS+=" -ld3dx9"
				],
				[x86_64],[
					my_LIBS+=" -ld3dx9_41"
				],
			)
		])
	]
)

AM_CONDITIONAL(LINUX, test "x$linux" = xyes)
AM_CONDITIONAL(WINDOWS, test "x$windows" = xyes)
AM_CONDITIONAL(ENAB_QT5, test "x$enable_qt5" = xyes)
AM_CONDITIONAL(WITH_D3D9, test "x$with_d3d9" = xyes)
AM_CONDITIONAL(WITH_OPENGL, test "x$with_opengl" = xyes)
AM_CONDITIONAL(PRECOMP_QT, test "x$path_qt_inc" != xno -a "x$path_qt_inc" != x -o \
				"x$path_qt_lib" != xno -a "x$path_qt_lib" != x -o \
				"x$path_qt_bin" != xno -a "x$path_qt_bin" != x)
AM_CONDITIONAL(PRECOMP_SDL, test "x$path_sdl_inc" != xno -a "x$path_sdl_inc" != x -o \
				 "x$path_sdl_lib" != xno -a "x$path_sdl_lib" != x)
AM_CONDITIONAL(COMPILE_LIB7ZIP, test "x$path_7zip_lib" = xno -o "x$path_7zip_lib" = x)

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
#AS_IF([test "x$PATCH" = "xno"],[
#	AC_MSG_ERROR([
#	*** The command patch could not be found. Make sure it is in your path.
#	])
#])
AC_PATH_TOOL([PKG_CONFIG], [pkg-config], [no])
AS_IF([test "x$PKG_CONFIG" = "xno"],[
	AC_MSG_ERROR([
	*** The pkg-config script could not be found. Make sure it is
	*** in your path, or set the PKG_CONFIG environment variable
	*** to the full path to pkg-config.
	])
])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h float.h inttypes.h limits.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/time.h unistd.h utime.h])

AX_CXX_COMPILE_STDCXX_11([noext])

OLD_CFLAGS="$CFLAGS"
OLD_LIBS="$LIBS"

# Checks for libraries.
AM_COND_IF([WINDOWS],[
	AC_MSG_CHECKING([if DSDK path is present])
	AS_IF([test "x$with_dsdk" = xno],[
		AC_MSG_ERROR([Needed DSDK root path])
	])
	my_CPPFLAGS+=" -I${path_dsdk}/Include -I${path_dsdk}/vc10/include"
	AC_MSG_RESULT(yes)
])

AM_COND_IF([PRECOMP_QT],[
	AC_MSG_CHECKING([if QT include path is present])
	AS_IF([test "x$path_qt_inc" = xno -o "x$path_qt_inc" = x],[
		AC_MSG_ERROR([Needed QT include path])
	])
	QT_CFLAGS="-I${path_qt_inc}"
	AC_MSG_RESULT(yes)

	AC_MSG_CHECKING([if QT binaries path is present])
	AS_IF([test "x$path_qt_bin" = xno -o "x$path_qt_bin" = x],[
		AC_MSG_ERROR([Needed QT binaries path])
	])
	AC_MSG_RESULT(yes)

	AC_MSG_CHECKING([if QT lib path is present])
	AS_IF([test "x$path_qt_lib" = xno -o "x$path_qt_lib" = x],[
		AC_MSG_ERROR([Needed QT lib path])
	])
	AC_MSG_RESULT(yes)

	AM_COND_IF([ENAB_QT5],[
		AM_COND_IF([WINDOWS],[
			QT_LIBS="-L${path_qt_lib} -lQt5Widgets -lqwindows -lQtGui -lQtCore -lqtharfbuzzng"
		],[
			QT_LIBS="-L${path_qt_lib} -lQt5Widgets -lQtGui -lQtCore"
		])
		AC_CHECK_TOOLS(MOC, [moc-qt5 moc], [no], [$path_qt_bin])
		AC_CHECK_TOOLS(UIC, [uic-qt5 uic], [no], [$path_qt_bin])
		AC_CHECK_TOOLS(RCC, [rcc-qt5 rcc], [no], [$path_qt_bin])
	],[
		QT_LIBS="-L${path_qt_lib} -lQtGui -lQtCore"
		AC_CHECK_TOOLS(MOC, [moc-qt4 moc], [no], [$path_qt_bin])
		AC_CHECK_TOOLS(UIC, [uic-qt4 uic], [no], [$path_qt_bin])
		AC_CHECK_TOOLS(RCC, [rcc-qt4 rcc], [no], [$path_qt_bin])
	])

	AS_IF([test "x$MOC" != xno],[
		MOC=$path_qt_bin/$MOC
		AC_SUBST([MOC])
	])
	AS_IF([test "x$UIC" != xno],[
		UIC=$path_qt_bin/$UIC
		AC_SUBST([UIC])
	])
	AS_IF([test "x$RCC" != xno],[
		RCC=$path_qt_bin/$RCC
		AC_SUBST([RCC])
	])

	my_CPPFLAGS+=" ${QT_CFLAGS}"
	my_LIBS+=" ${QT_LIBS}"

],[
	AM_COND_IF([ENAB_QT5],[
		PKG_CHECK_MODULES(QT5, [Qt5Core >= 5.0.0 Qt5Gui >= 5.0.0 Qt5Widgets >= 5.0.0], [
			QT5_PREFIX="`$PKG_CONFIG --variable=host_bins Qt5Core`"
			AC_CHECK_TOOLS(MOC, [moc-qt5 moc], no, [$QT5_PREFIX])
			AC_CHECK_TOOLS(UIC, [uic-qt5 uic], no, [$QT5_PREFIX])
			AC_CHECK_TOOLS(RCC, [rcc-qt5 rcc], no, [$QT5_PREFIX])
			AS_IF([test "x$MOC" != xno],[
				MOC=$QT5_PREFIX/$MOC
				AC_SUBST([MOC])
			])
			AS_IF([test "x$UIC" != xno],[
				UIC=$QT5_PREFIX/$UIC
				AC_SUBST([UIC])
			])
			AS_IF([test "x$RCC" != xno],[
				RCC=$QT5_PREFIX/$RCC
				AC_SUBST([RCC])
			])
		],[
			AC_MSG_ERROR([Qt5 library not found])
		])

		my_CPPFLAGS+=" ${QT5_CFLAGS}"
		my_LIBS+=" ${QT5_LIBS}"
	],[
		PKG_CHECK_MODULES(QT4, [QtCore >= 4.6.0 QtGui >= 4.6.0], [
			AC_CHECK_TOOLS(MOC, [moc-qt4 moc])
			AC_CHECK_TOOLS(UIC, [uic-qt4 uic])
			AC_CHECK_TOOLS(RCC, [rcc-qt4 rcc])
		],[
			AC_MSG_ERROR([Qt4 library not found])
		])

		my_CPPFLAGS+=" ${QT4_CFLAGS}"
		my_LIBS+=" ${QT4_LIBS}"
	])
])
AS_IF([test "x$MOC" = xno],[
	AC_MSG_ERROR([Could not find moc binaries])
])
AS_IF([test "x$UIC" = xno],[
	AC_MSG_ERROR([Could not find uic binaries])
])
AS_IF([test "x$RCC" = xno],[
	AC_MSG_ERROR([Could not find rcc binaries])
])

AM_COND_IF([WITH_OPENGL],[
	AM_COND_IF([PRECOMP_SDL],[
		AC_MSG_CHECKING([if SDL include path is present])
		AS_IF([test "x$path_sdl_inc" = xno -o "x$path_sdl_inc" = x],[
			AC_MSG_ERROR([Needed SDL include path])
		])
		SDL_CFLAGS="-I${path_sdl_inc}"
		AC_SUBST([SDL_CFLAGS])
		AC_MSG_RESULT(yes)
	
		AC_MSG_CHECKING([if SDL lib path is present])
		AS_IF([test "x$path_sdl_lib" = xno -o "x$path_sdl_lib" = x],[
			AC_MSG_ERROR([Needed SDL lib path])
		])
		SDL_LIBS="-L${path_sdl_lib} -lSDL"
		AC_SUBST([SDL_LIBS])
		AC_MSG_RESULT(yes)

		my_CPPFLAGS+=" ${SDL_CFLAGS}"
		my_LIBS+=" ${SDL_LIBS}"
	],[
		SDL_VERSION=1.2.0
		AM_PATH_SDL($SDL_VERSION, :, AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
		#AM_PATH_SDL($SDL_VERSION, :, [PKG_CHECK_MODULES(sdl, [sdl])])
		my_CPPFLAGS+=" ${SDL_CFLAGS}"
		my_LIBS+=" ${SDL_LIBS}"

		CFLAGS="$CFLAGS ${SDL_CFLAGS}"
		LIBS="$LIBS ${SDL_LIBS}"
		# Check if SDL is 1.2.x (1.3 not supported)
		AC_MSG_CHECKING([SDL version only being 1.2.X])
		AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include "SDL.h"
void blah(){
#if SDL_MINOR_VERSION != 2
#error "Only SDL 1.2 supported"
#endif
	;
}
		])],
		AC_MSG_RESULT([yes]), [AC_MSG_RESULT([no])
		AC_MSG_ERROR([Only libSDL 1.2.X supported])])
		CFLAGS="${OLD_CFLAGS}"
		LIBS="${OLD_LIBS}"
	])

	AM_COND_IF([WINDOWS],[
		my_LIBS+=" -lopengl32"
	],[
		my_LIBS+=" -lGL"
	])
])

AM_COND_IF([LINUX],[
	PKG_CHECK_MODULES(alsa, [alsa])
	#my_CPPFLAGS+=" ${alsa_CFLAGS}"
	my_LIBS+=" ${alsa_LIBS}"
])

AS_CASE(["$host_os"],
	[mingw32*],[
		AC_CHECK_TOOL(WINDRES, windres, [no])
	],[
		WINDRES=no
	]
)
AM_CONDITIONAL(HAVE_WINDRES, test x$WINDRES != xno)
AC_SUBST(WINDRES)

LIB7ZIP_SRC="lib7zip-1.6.5"
LIB7ZIP_P7ZIP="p7zip_9.20.1"
LIB7ZIP_7Z="7z922"
AM_COND_IF([COMPILE_LIB7ZIP],[
	AM_COND_IF([WINDOWS],[
		export P7ZIP_SOURCE_DIR="${TOP_SRCDIR}/src/extra/${LIB7ZIP_7Z}"
	],[
		export P7ZIP_SOURCE_DIR="${TOP_SRCDIR}/src/extra/${LIB7ZIP_P7ZIP}"
	])
	#AC_CONFIG_COMMANDS_PRE([])
	AC_CONFIG_SUBDIRS([src/extra/lib7zip-1.6.5])
	LIB7ZIP_LIBS="${TOP_BUILDDIR}src/extra/${LIB7ZIP_SRC}/Lib7Zip/lib7zip.a"
],[
	LIB7ZIP_LIBS="${path_7zip_lib}/lib7zip.a"
])
AC_SUBST(LIB7ZIP_LIBS)

# The End
AM_COND_IF([WINDOWS],[
	my_LDFLAGS+=" -Wl,-subsystem,console -static-libgcc -static-libstdc++"
	my_LIBS+=" -lwinmm -luuid -lole32 -lws2_32 -ladvapi32 -lshell32 -luser32 -lkernel32 -lgdi32 -loleaut32 -lcomdlg32 -limm32 -lwinspool"
])
AM_COND_IF([LINUX],[
	my_LIBS+=" -ldl -lpthread"
])

CFLAGS="$CFLAGS ${my_CPPFLAGS} ${my_CFLAGS}"
CXXFLAGS="$CXXFLAGS ${my_CPPFLAGS} ${my_CXXFLAGS}"
LDFLAGS="$LDFLAGS ${my_LDFLAGS}"
LIBS="$LIBS ${my_LIBS}"

AC_CONFIG_FILES([
Makefile
misc/punes.desktop
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
