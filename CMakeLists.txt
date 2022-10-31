cmake_minimum_required(VERSION 3.12)

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
option(ENABLE_WIN_STATIC_QT "Use precompiled Qt static library (only for Windows)" OFF)
option(ENABLE_WIN_STATIC_QT560 "Use precompiled Qt 5.6.3 static library (only for Windows)" OFF)

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
	if(ENABLE_WIN_STATIC_QT560)
		set(ENABLE_WIN_STATIC_QT ON)
	endif()
	# No alternatives, force selection
	set(ENABLE_OPENGL_CG ${ENABLE_OPENGL})
	set(ENABLE_FULLSCREEN_RESFREQ ON)
else()
	set(ENABLE_WIN_STATIC_QT OFF)
	set(ENABLE_WIN_STATIC_QT560 OFF)
endif()

# OpenGL is required for Qt, even in the case of D3D9.
find_package(OpenGL REQUIRED)

find_package(Qt5 5.6.3 COMPONENTS Core LinguistTools Widgets Network Svg Gui REQUIRED)

if(ENABLE_WIN_STATIC_QT)
	include(WinStaticQt)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(ENABLE_FFMPEG)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
		libavformat>=58.12.100
		libavcodec>=58.18.100
		libswresample>=3.1.100
		libswscale>=5.1.100
		libavutil>=56.14.100
		)
endif()

add_subdirectory(src)
