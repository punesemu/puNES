PROG=../../../../bin/p7zip$(BINSUFFIX)

LOCAL_FLAGS=\
	-DUNICODE -D_UNICODE \
	-I. \
	`wx-config --unicode=yes --cxxflags`

include ../../../../makefile.crc32
include ../../../../makefile.machine

PCH_NAME=$(PRE_COMPILED_HEADER)

LIBS=`wx-config --unicode=yes --libs` $(LOCAL_LIBS_DLL)

OBJS=\
  FileDir.o \
  FileFind.o \
  MyString.o \
  MyVector.o \
 IntToString.o \
StringConvert.o \
wine_date_and_time.o \
Threads.o \
  wxP7ZIP.o

include ../../../../makefile.glb

