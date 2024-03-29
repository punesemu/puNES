
# WARNING : automatically generated by utils/generate.py

QT -= core gui
TARGET = Lzham

CONFIG += dll
TEMPLATE = lib

DESTDIR = ../../../../bin

unix: LIBS += -ldl

DEFINES+=USE_LIB7Z_DLL

INCLUDEPATH = \
  ../../../../CPP/7zip/Compress/Lzham/include \
  ../../../../CPP/7zip/Compress/Lzham/lzhamcomp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp \
  ../../../myWindows \
  ../../../ \
  ../../../../ \
  ../../../include_windows \

DEFINES += EXTERNAL_CODECS
DEFINES += _FILE_OFFSET_BITS=64
DEFINES += _LARGEFILE_SOURCE
DEFINES += _REENTRANT
DEFINES += ENV_UNIX
DEFINES += BREAK_HANDLER
DEFINES += UNICODE
DEFINES += _UNICODE
DEFINES += UNIX_USE_WIN_FILE

SOURCES +=  \
  ../../../../C/Alloc.c \
  ../../../../CPP/7zip/Common/StreamUtils.cpp \
  ../../../../CPP/7zip/Compress/CodecExports.cpp \
  ../../../../CPP/7zip/Compress/DllExportsCompress.cpp \
  ../../../../CPP/7zip/Compress/Lzham/LzhamRegister.cpp \
  ../../../../CPP/Common/MyWindows.cpp \
  ../../../../CPP/Windows/System.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamcomp/lzham_lzbase.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamcomp/lzham_lzcomp.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamcomp/lzham_lzcomp_internal.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamcomp/lzham_lzcomp_state.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamcomp/lzham_match_accel.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamcomp/lzham_pthreads_threading.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_assert.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_checksum.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_huffman_codes.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_lzdecomp.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_lzdecompbase.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_mem.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_platform.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_prefix_coding.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_symbol_codec.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_timer.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamdecomp/lzham_vector.cpp \
  ../../../../CPP/7zip/Compress/Lzham/lzhamlib/lzham_lib.cpp \

macx: LIBS += -framework CoreFoundation

