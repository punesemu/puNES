PROG=../../../../bin/7zr$(BINSUFFIX)

LOCAL_FLAGS= \
  -D_NO_CRYPTO \
  -D_NO_CRYPTO \
  -DBREAK_HANDLER \
  -DUNICODE -D_UNICODE

include ../../../../makefile.crc32
include ../../../../makefile.machine

PCH_NAME=$(PRE_COMPILED_HEADER)

LIBS=$(LOCAL_LIBS)

CONSOLE_OBJS = \
  BenchCon.o \
  ConsoleClose.o \
  ExtractCallbackConsole.o \
  List.o \
  Main.o \
  MainAr.o \
  OpenCallbackConsole.o \
  PercentPrinter.o \
  UpdateCallbackConsole.o \
  UserInputUtils.o \

# NewHandler.o StringConvert.o
COMMON_OBJS = \
  CommandLineParser.o \
  CRC.o \
  IntToString.o \
  ListFileUtils.o \
  StdInStream.o \
  StdOutStream.o \
  MyString.o \
  MyWindows.o \
  StringConvert.o \
  StringToInt.o \
  UTFConvert.o \
  MyVector.o \
  Wildcard.o \

# DLL.o MemoryLock.o
WIN_OBJS = \
  Error.o \
  FileDir.o \
  FileFind.o \
  FileIO.o \
  FileName.o \
  PropVariant.o \
  PropVariantConversions.o \
  Synchronization.o \
  System.o \
  Time.o \

7ZIP_COMMON_OBJS = \
  CreateCoder.o \
  CWrappers.o \
  FilePathAutoRename.o \
  FileStreams.o \
  FilterCoder.o \
  InBuffer.o \
  InOutTempBuffer.o \
  LimitedStreams.o \
  LockedStream.o \
  MethodId.o \
  MethodProps.o \
  OffsetStream.o \
  OutBuffer.o \
  ProgressUtils.o \
  StreamBinder.o \
  StreamObjects.o \
  StreamUtils.o \
  VirtThread.o \

# WorkDir.o
UI_COMMON_OBJS = \
  ArchiveCommandLine.o \
  ArchiveExtractCallback.o \
  ArchiveOpenCallback.o \
  Bench.o \
  DefaultName.o \
  EnumDirItems.o \
  Extract.o \
  ExtractingFilePath.o \
  LoadCodecs.o \
  OpenArchive.o \
  PropIDUtils.o \
  SetProperties.o \
  SortUtils.o \
  TempFiles.o \
  Update.o \
  UpdateAction.o \
  UpdateCallback.o \
  UpdatePair.o \
  UpdateProduce.o \

AR_OBJS = \
  LzmaHandler.o \
  SplitHandler.o \
  XzHandler.o \

AR_COMMON_OBJS = \
  CoderMixer2.o \
  CoderMixer2MT.o \
  CrossThreadProgress.o \
  DummyOutStream.o \
  HandlerOut.o \
  InStreamWithCRC.o \
  ItemNameUtils.o \
  MultiStream.o \
  OutStreamWithCRC.o \
  ParseProperties.o \

7Z_OBJS = \
  7zCompressionMode.o \
  7zDecode.o \
  7zEncode.o \
  7zExtract.o \
  7zFolderInStream.o \
  7zFolderOutStream.o \
  7zHandler.o \
  7zHandlerOut.o \
  7zHeader.o \
  7zIn.o \
  7zOut.o \
  7zProperties.o \
  7zRegister.o \
  7zSpecStream.o \
  7zUpdate.o \

COMPRESS_OBJS = \
  Bcj2Coder.o \
  Bcj2Register.o \
  BcjCoder.o \
  BcjRegister.o \
  BranchCoder.o \
  BranchMisc.o \
  BranchRegister.o \
  ByteSwap.o \
  CopyCoder.o \
  CopyRegister.o \
  DeltaFilter.o \
  Lzma2Decoder.o \
  Lzma2Encoder.o \
  Lzma2Register.o \
  LzmaDecoder.o \
  LzmaEncoder.o \
  LzmaRegister.o \

C_OBJS = \
  7zStream.o \
  Alloc.o \
  Bra.o \
  Bra86.o \
  BraIA64.o \
  Delta.o \
  LzFind.o \
  LzFindMt.o \
  Lzma2Dec.o \
  Lzma2Enc.o \
  LzmaDec.o \
  LzmaEnc.o \
  MtCoder.o \
  Sha256.o \
  Threads.o \
  Xz.o \
  XzCrc64.o \
  XzDec.o \
  XzEnc.o \
  XzIn.o \


OBJS = \
  wine_date_and_time.o \
  myAddExeFlag.o \
  mySplitCommandLine.o \
  $(CONSOLE_OBJS) \
  $(COMMON_OBJS) \
  $(WIN_OBJS) \
  $(7ZIP_COMMON_OBJS) \
  $(UI_COMMON_OBJS) \
  $(AR_OBJS) \
  $(AR_COMMON_OBJS) \
  $(7Z_OBJS) \
  $(COMPRESS_OBJS) \
  $(C_OBJS) \
  $(OBJ_CRC32) \

include ../../../../makefile.glb

