cmake_minimum_required(VERSION 3.15)

project(puNES)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

string(TIMESTAMP COMPILED_TIMESTAMP "%m%d%Y%H%M%S")

#variable_watch(ENABLE_WIN_STATIC_QT560)

# Options
option(ENABLE_GIT_INFO "Include Git info" OFF)
option(ENABLE_RELEASE "Build release version" ON)
option(ENABLE_FFMPEG "Enable FFMPEG support" ON)
option(ENABLE_OPENGL "Use OpenGL support instead of Direct3D 9 (only for Windows)" ON)
option(ENABLE_OPENGL_CG "Enable OpenGL nVidia Cg Toolkit support" OFF)
option(ENABLE_FULLSCREEN_RESFREQ "Enable Fullscreen resolution and auto frequency" ON)
option(ENABLE_QT5_LIBS "Enable support for QT5 libraries" OFF)
option(ENABLE_WIN_STATIC_QT5 "Use precompiled Qt static library (only for Windows)" OFF)
option(ENABLE_WIN_STATIC_QT560 "Use precompiled Qt 5.6.3 static library (only for Windows)" OFF)
option(DISABLE_PORTABLE_MODE "Disable compiling portable mode management (useful in sandbox environments like Flatpak)" OFF)

if (ENABLE_GIT_INFO)
	find_package(Git)

	if(Git_FOUND)
		execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_LAST_COMMIT
			ERROR_VARIABLE git_describe_error
			RESULT_VARIABLE git_describe_result
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)

		execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_LAST_COMMIT_HASH
			ERROR_VARIABLE git_describe_error
			RESULT_VARIABLE git_describe_result
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)

		execute_process(COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_COUNT_COMMITS
			ERROR_VARIABLE git_describe_error
			RESULT_VARIABLE git_describe_result
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)
	endif()
endif()

# For installation targets
include(GNUInstallDirs)

# sizeof checking for xdelta config.h generation
include(CheckTypeSize)
check_type_size("size_t" SIZEOF_SIZE_T)
check_type_size("unsigned int" SIZEOF_UNSIGNED_INT)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
check_type_size("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)
configure_file(xdelta_config.h.in config.h)

configure_file(compiled.h.in compiled.h)

include(TargetArch)
target_architecture(HOST_CPU)

# handling external dependencies
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS gui/designer)

set(QT_LIBS_USED "")
set(WIN_LIBS_USED "")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	find_package(ALSA REQUIRED)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(UDEV REQUIRED IMPORTED_TARGET libudev)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	find_package(Sndio REQUIRED)
	find_package(UsbHid REQUIRED)
	set(ENABLE_OPENGL_CG OFF)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
	if(ENABLE_FULLSCREEN_RESFREQ)
		find_package(X11 REQUIRED)
	endif()
	# No alternatives, force selection
	set(ENABLE_OPENGL ON)
	if(ENABLE_OPENGL_CG)
		find_package(Cg REQUIRED)
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(ENABLE_QT5_LIBS ON)
	if(ENABLE_WIN_STATIC_QT560)
		set(ENABLE_WIN_STATIC_QT5 ON)
	endif()
	# No alternatives, force selection
	set(ENABLE_OPENGL_CG ${ENABLE_OPENGL})
	set(ENABLE_FULLSCREEN_RESFREQ ON)
else()
	set(ENABLE_WIN_STATIC_QT5 OFF)
	set(ENABLE_WIN_STATIC_QT560 OFF)
endif()

# OpenGL is required for Qt, even in the case of D3D9.
if((CMAKE_SYSTEM_NAME STREQUAL "Linux") AND (NOT OpenGL_GL_PREFERENCE))
	# See CMP0072 for more details (cmake --help-policy CMP0072)
	set(OpenGL_GL_PREFERENCE "GLVND")
endif()
find_package(OpenGL REQUIRED)

set(QT_FOUND FALSE)
set(QT_VERSION "")

if(ENABLE_QT5_LIBS)
	find_package(Qt5 5.6.3 COMPONENTS Core LinguistTools Widgets Network Svg Gui)
	if(Qt5_FOUND)
		set(QT_FOUND TRUE)
		set(QT_VERSION ${Qt5_VERSION})
	else()
		message(FATAL_ERROR "Qt5 libraries not found.")
	endif()
else()
	find_package(Qt6 6.0.0 COMPONENTS Core LinguistTools Widgets Network Svg Gui OpenGLWidgets)
	if(Qt6_FOUND)
		set(QT_FOUND TRUE)
		set(QT_VERSION ${Qt6_VERSION})
	else()
		find_package(Qt5 5.6.3 QUIET COMPONENTS Core LinguistTools Widgets Network Svg Gui)
		if(Qt5_FOUND)
			set(QT_FOUND TRUE)
			set(QT_VERSION ${Qt5_VERSION})
		endif()
	endif()
endif()

if(NOT QT_FOUND)
	message(FATAL_ERROR "Qt libraries not found. Please install Qt5 or Qt6.")
endif()

message(STATUS "Found Qt: ${QT_VERSION}")

if(${QT_VERSION} VERSION_LESS "5.15.0")
	macro(qt_add_translation)
		qt5_add_translation(${ARGN})
	endmacro()
	macro(qt_add_resources)
		qt5_add_resources(${ARGN})
	endmacro()
endif()

if(ENABLE_WIN_STATIC_QT5)
	include(WinStaticQt)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(ENABLE_FFMPEG)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
		libavformat
		libavcodec
		libswresample
		libswscale
		libavutil
		)
endif()

if(${QT_VERSION} VERSION_LESS "5.15.0")
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt5::Core)
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt5::Network)
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt5::Widgets)
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt5::Svg)
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt5::Gui)
else()
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt::Core)
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt::Network)
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt::Widgets)
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt::Svg)
	set(QT_LIBS_USED ${QT_LIBS_USED} Qt::Gui)
	if(${QT_VERSION} VERSION_GREATER_EQUAL "6.0.0")
		set(QT_LIBS_USED ${QT_LIBS_USED} Qt::OpenGLWidgets)
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(WIN_LIBS_USED ${WIN_LIBS_USED} ws2_32)
	set(WIN_LIBS_USED ${WIN_LIBS_USED} imm32)
	set(WIN_LIBS_USED ${WIN_LIBS_USED} cg)
	if(ENABLE_OPENGL)
		if(ENABLE_OPENGL_CG)
			set(WIN_LIBS_USED ${WIN_LIBS_USED} cgGL)
		endif()
	else()
		set(WIN_LIBS_USED ${WIN_LIBS_USED} cgD3D9)
		set(WIN_LIBS_USED ${WIN_LIBS_USED} d3d9)
	endif()
endif()

add_subdirectory(src)

# Install misc files
install(FILES misc/io.github.punesemu.puNES.desktop.in
	DESTINATION ${CMAKE_INSTALL_DATADIR}/applications RENAME io.github.punesemu.puNES.desktop)

foreach(SIZE "16" "22" "24" "32" "48" "64" "256" "512")
	install(FILES misc/hicolor_apps_${SIZE}x${SIZE}_io.github.punesemu.puNES.png
		DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${SIZE}x${SIZE}/apps RENAME io.github.punesemu.puNES.png)
endforeach()

install(FILES misc/io.github.punesemu.puNES.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES misc/nes20db.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/puNES)
install(FILES misc/dip.cfg DESTINATION ${CMAKE_INSTALL_DATADIR}/puNES)

install(FILES README.md ChangeLog DESTINATION ${CMAKE_INSTALL_DOCDIR})
