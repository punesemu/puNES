PROG=../../../../bin/TestUI$(BINSUFFIX)

LOCAL_FLAGS=\
	-DUNICODE -D_UNICODE \
	-DLANG \
	-DEXTERNAL_LZMA \
	-DEXTERNAL_CODECS \
	-DBENCH_MT \
	-I. \
	`wx-config --unicode=yes --cxxflags`

include ../../../../makefile.crc32
include ../../../../makefile.machine

PCH_NAME=$(PRE_COMPILED_HEADER)

LIBS=`wx-config --unicode=yes --libs` $(LOCAL_LIBS_DLL)

C_OBJS = \
  Threads.o \

OBJS=\
PasswordDialog.o \
PasswordDialog_rc.o \
Window.o \
Dialog.o \
Controls.o \
  $(C_OBJS) \
TestUI.o


include ../../../../makefile.glb

