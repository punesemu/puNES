PROG=../../../../bin/Client7z$(BINSUFFIX)

LOCAL_FLAGS=-DUNICODE -D_UNICODE

include ../../../../makefile.crc32
include ../../../../makefile.machine

PCH_NAME=$(PRE_COMPILED_HEADER)

LIBS=$(LOCAL_LIBS_DLL)

OBJS = \
wine_date_and_time.o \
MyWindows.o \
DLL.o \
FileDir.o \
FileFind.o \
FileIO.o \
FileName.o \
PropVariant.o \
PropVariantConversions.o \
IntToString.o \
MyString.o \
StringConvert.o \
UTFConvert.o \
MyVector.o \
Wildcard.o \
FileStreams.o \
Threads.o \
Client7z.o

include ../../../../makefile.glb

