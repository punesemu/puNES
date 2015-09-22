PROG=../../../../bin/Codecs/Rar29.so

LOCAL_FLAGS=$(CC_SHARED) -DUNICODE -D_UNICODE

MY_WINDOWS=

include ../../../../makefile.crc32
include ../../../../makefile.machine

LOCAL_SHARED=$(LINK_SHARED)
LIBS=$(LOCAL_LIBS)

OBJS = \
MyWindows.o \
CRC.o\
MyVector.o\
CodecExports.o \
DllExports.o \
Rar1Decoder.o \
Rar2Decoder.o \
Rar3Decoder.o \
Rar3Vm.o \
RarCodecsRegister.o \
$(OBJ_CRC32) \
InBuffer.o \
OutBuffer.o \
StreamUtils.o \
LzOutWindow.o \
Ppmd7.o \
Ppmd7Dec.o \
Alloc.o

include ../../../../makefile.glb

