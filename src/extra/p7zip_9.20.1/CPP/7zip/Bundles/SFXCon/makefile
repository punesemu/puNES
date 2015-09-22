PROG=../../../../bin/7zCon.sfx

LOCAL_FLAGS= \
  -DEXTRACT_ONLY \
  -DNO_READ_FROM_CODER \
  -D_SFX \
  -DUNICODE -D_UNICODE

include ../../../../makefile.crc32
include ../../../../makefile.machine

PCH_NAME=$(PRE_COMPILED_HEADER)

LIBS=$(LOCAL_LIBS)

SFX_CONSOLE_OBJS = \
  Main.o \

CONSOLE_OBJS = \
  ConsoleClose.o \
  ExtractCallbackConsole.o \
  List.o \
  MainAr.o \
  OpenCallbackConsole.o \
  UserInputUtils.o \

# NewHandler.o
COMMON_OBJS = \
  CommandLineParser.o \
  CRC.o \
  IntToString.o \
  StdInStream.o \
  StdOutStream.o \
  MyString.o \
  MyWindows.o\
  StringConvert.o \
  UTFConvert.o \
  MyVector.o \
  Wildcard.o \

# DLL.o 
WIN_OBJS = \
  Error.o \
  FileDir.o \
  FileFind.o \
  FileIO.o \
  FileName.o \
  PropVariant.o \
  PropVariantConversions.o \
  Synchronization.o \
  System.o

7ZIP_COMMON_OBJS = \
  CreateCoder.o \
  CWrappers.o \
  FilePathAutoRename.o \
  FileStreams.o \
  InBuffer.o \
  FilterCoder.o \
  LimitedStreams.o \
  LockedStream.o \
  OutBuffer.o \
  ProgressUtils.o \
  StreamBinder.o \
  StreamObjects.o \
  StreamUtils.o \
  VirtThread.o \

UI_COMMON_OBJS = \
  ArchiveExtractCallback.o \
  ArchiveOpenCallback.o \
  DefaultName.o \
  LoadCodecs.o \
  Extract.o \
  ExtractingFilePath.o \
  OpenArchive.o \
  PropIDUtils.o \

AR_COMMON_OBJS = \
  CoderMixer2.o \
  CoderMixer2MT.o \
  CrossThreadProgress.o \
  ItemNameUtils.o \
  OutStreamWithCRC.o \


7Z_OBJS = \
  7zDecode.o \
  7zExtract.o \
  7zFolderOutStream.o \
  7zHandler.o \
  7zHeader.o \
  7zIn.o \
  7zRegister.o \

COMPRESS_OBJS = \
  BranchCoder.o \
  Bcj2Coder.o \
  Bcj2Register.o \
  BcjCoder.o \
  BcjRegister.o \
  CopyCoder.o \
  CopyRegister.o \
  Lzma2Decoder.o \
  Lzma2Register.o \
  LzmaDecoder.o \
  LzmaRegister.o \
  PpmdDecoder.o \
  PpmdRegister.o \

CRYPTO_OBJS = \
  7zAes.o \
  7zAesRegister.o \
  MyAes.o \

C_OBJS = \
  Aes.o \
  Alloc.o \
  Bra86.o \
  Lzma2Dec.o \
  LzmaDec.o \
  Ppmd7.o \
  Ppmd7Dec.o \
  Sha256.o \
  Threads.o \

OBJS = \
  myGetTickCount.o\
  wine_date_and_time.o\
  mySplitCommandLine.o\
  $(SFX_CONSOLE_OBJS) \
  $(CONSOLE_OBJS) \
  $(COMMON_OBJS) \
  $(WIN_OBJS) \
  $(7ZIP_COMMON_OBJS) \
  $(UI_COMMON_OBJS) \
  $(AR_COMMON_OBJS) \
  $(7Z_OBJS) \
  $(COMPRESS_OBJS) \
  $(CRYPTO_OBJS) \
  $(C_OBJS) \
  $(OBJ_CRC32)


include ../../../../makefile.glb

