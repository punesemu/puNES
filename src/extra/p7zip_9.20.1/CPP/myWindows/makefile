PROG=test_emul

LOCAL_FLAGS=-I. -I.. -I../include_windows -DUNICODE -D_UNICODE

include ../../makefile.crc32
include ../../makefile.machine

LIBS=$(LOCAL_LIBS)

OBJS=myGetTickCount.o \
	wine_date_and_time.o \
	mySplitCommandLine.o \
	myAddExeFlag.o \
	wine_GetXXXDefaultLangID.o \
	test_emul.o \
	MyVector.o \
	MyString.o \
	MyWindows.o

include ../../makefile.glb

