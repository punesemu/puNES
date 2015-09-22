include ../../../../makefile.machine

PROG = lzma
LIB = $(LOCAL_LIBS)
RM = rm -f
CFLAGS = -c -I. -I../../../  -I../../../myWindows  -I../../../include_windows \
         -UENV_UNIX -DCOMPRESS_MF_MT -DBENCH_MT

CXXFLAGS=$(CFLAGS)

OBJS = \
  LzmaAlone.o \
  LzmaBench.o \
  LzmaBenchCon.o \
  LzmaDecoder.o \
  LzmaEncoder.o \
  CWrappers.o \
  InBuffer.o \
  OutBuffer.o \
  FileStreams.o \
  StreamUtils.o \
  C_FileIO.o \
  CommandLineParser.o \
  CRC.o \
  IntToString.o \
  MyString.o \
  StringConvert.o \
  UTFConvert.o \
  StringToInt.o \
  MyVector.o \
  7zCrc.o \
  7zCrcOpt.o \
  Alloc.o \
  Bra86.o \
  LzFind.o \
  LzFindMt.o \
  LzmaDec.o \
  LzmaEnc.o \
  Lzma86Dec.o \
  Lzma86Enc.o \
  System.o \
  Threads.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) -o $(PROG) $(LDFLAGS) $(OBJS) $(LIB)

LzmaAlone.o: LzmaAlone.cpp
	$(CXX) $(CXXFLAGS) LzmaAlone.cpp

LzmaBench.o: LzmaBench.cpp
	$(CXX) $(CXXFLAGS) LzmaBench.cpp

LzmaBenchCon.o: LzmaBenchCon.cpp
	$(CXX) $(CXXFLAGS) LzmaBenchCon.cpp

LzmaRam.o: LzmaRam.cpp
	$(CXX) $(CXXFLAGS) LzmaRam.cpp

LzmaDecoder.o: ../LzmaDecoder.cpp
	$(CXX) $(CXXFLAGS) ../LzmaDecoder.cpp

LzmaEncoder.o: ../LzmaEncoder.cpp
	$(CXX) $(CXXFLAGS) ../LzmaEncoder.cpp

RangeCoderBit.o: ../RangeCoderBit.cpp
	$(CXX) $(CXXFLAGS) ../RangeCoderBit.cpp

CWrappers.o: ../../Common/CWrappers.cpp
	$(CXX) $(CXXFLAGS) ../../Common/CWrappers.cpp

InBuffer.o: ../../Common/InBuffer.cpp
	$(CXX) $(CXXFLAGS) ../../Common/InBuffer.cpp

OutBuffer.o: ../../Common/OutBuffer.cpp
	$(CXX) $(CXXFLAGS) ../../Common/OutBuffer.cpp

FileStreams.o: ../../Common/FileStreams.cpp
	$(CXX) $(CXXFLAGS) ../../Common/FileStreams.cpp

StreamUtils.o: ../../Common/StreamUtils.cpp
	$(CXX) $(CXXFLAGS) ../../Common/StreamUtils.cpp

C_FileIO.o: ../../../Common/C_FileIO.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/C_FileIO.cpp

CommandLineParser.o: ../../../Common/CommandLineParser.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/CommandLineParser.cpp

CRC.o: ../../../Common/CRC.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/CRC.cpp

MyWindows.o: ../../../Common/MyWindows.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/MyWindows.cpp

IntToString.o: ../../../Common/IntToString.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/IntToString.cpp

MyString.o: ../../../Common/MyString.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/MyString.cpp

StringConvert.o: ../../../Common/StringConvert.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/StringConvert.cpp

UTFConvert.o: ../../../Common/UTFConvert.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/UTFConvert.cpp

StringToInt.o: ../../../Common/StringToInt.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/StringToInt.cpp

MyVector.o: ../../../Common/MyVector.cpp
	$(CXX) $(CXXFLAGS) ../../../Common/MyVector.cpp

7zCrc.o: ../../../../C/7zCrc.c
	$(CC) $(CFLAGS) ../../../../C/7zCrc.c

7zCrcOpt.o: ../../../../C/7zCrcOpt.c
	$(CC) $(CFLAGS) ../../../../C/7zCrcOpt.c

LzmaRamDecode.o: LzmaRamDecode.c
	$(CC) $(CFLAGS) LzmaRamDecode.c

System.o : ../../../Windows/System.cpp
	$(CXX) $(CXXFLAGS) ../../../Windows/System.cpp

Threads.o : ../../../../C/Threads.c
	$(CC) $(CFLAGS) ../../../../C/Threads.c

Alloc.o: ../../../../C/Alloc.c
	$(CC) $(CFLAGS) ../../../../C/Alloc.c

Bra86.o: ../../../../C/Bra86.c
	$(CC) $(CFLAGS) ../../../../C/Bra86.c

LzFind.o: ../../../../C/LzFind.c
	$(CC) $(CFLAGS) ../../../../C/LzFind.c

LzFindMt.o: ../../../../C/LzFindMt.c
	$(CC) $(CFLAGS) ../../../../C/LzFindMt.c

LzmaDec.o: ../../../../C/LzmaDec.c
	$(CC) $(CFLAGS) ../../../../C/LzmaDec.c

LzmaEnc.o: ../../../../C/LzmaEnc.c
	$(CC) $(CFLAGS) ../../../../C/LzmaEnc.c

Lzma86Dec.o: ../../../../C/LzmaUtil/Lzma86Dec.c
	$(CC) $(CFLAGS) ../../../../C/LzmaUtil/Lzma86Dec.c

Lzma86Enc.o: ../../../../C/LzmaUtil/Lzma86Enc.c
	$(CC) $(CFLAGS) ../../../../C/LzmaUtil/Lzma86Enc.c

clean:
	-$(RM) $(PROG) $(OBJS) testfile* *.exe ir.out
	-$(RM) -fr SunWS_cache

test: $(PROG)
	cp LzmaAlone.cpp testfile
	./$(PROG) e testfile testfile.lzma
	./$(PROG) d testfile.lzma testfile.lzma.d 
	diff -s testfile testfile.lzma.d
	./$(PROG) e testfile testfile.lzma
	./$(PROG) d testfile.lzma testfile.lzma.d 
	diff -s testfile testfile.lzma.d
	./$(PROG) e -si -so < testfile > testfile.lzma.2
	./$(PROG) d -si -so < testfile.lzma.2 > testfile.lzma.2.d 
	diff -s testfile testfile.lzma.2.d
	@echo =========
	@echo All Done 
	@echo =========


