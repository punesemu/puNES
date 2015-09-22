
QT -= core \
    gui
TARGET = test_emul.x
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES +=  \
 ../../../myWindows/myGetTickCount.cpp \
 ../../../myWindows/wine_date_and_time.cpp \
 ../../../myWindows/myAddExeFlag.cpp \
 ../../../myWindows/mySplitCommandLine.cpp \
 ../../../myWindows/wine_GetXXXDefaultLangID.cpp \
 ../../../myWindows/test_emul.cpp \
 ../../../Common/MyString.cpp \
 ../../../Common/MyWindows.cpp \
 ../../../Common/MyVector.cpp \
 ../../../../C/7zCrc.c \
 ../../../../C/7zCrcOpt.c


INCLUDEPATH = ../../../myWindows \
	../../../ \
	../../../include_windows

DEFINES += _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE NDEBUG _REENTRANT ENV_UNIX BREAK_HANDLER UNICODE _UNICODE
