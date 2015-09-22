PROG=../../../../bin/7zG$(BINSUFFIX)

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

GUI_OBJS = \
 BenchmarkDialog.o \
 BenchmarkDialog_rc.o \
 CompressDialog.o \
 CompressDialog_rc.o \
 ExtractDialog.o \
 ExtractDialog_rc.o \
 ExtractGUI.o \
 GUI.o \
 wxGUI.o \
 UpdateCallbackGUI.o \
 UpdateGUI.o \

COMMON_OBJS = \
CommandLineParser.o \
CRC.o \
IntToString.o \
Lang.o \
ListFileUtils.o \
MyString.o \
MyVector.o \
StringConvert.o \
StringToInt.o \
TextConfig.o \
UTFConvert.o \
Wildcard.o \

# CommonDialog.o MemoryLock.o ResourceString.o ResourceString.o Shell.o
WIN_OBJS = \
DLL.o \
Error.o \
FileDir.o \
FileFind.o \
FileIO.o \
FileName.o \
PropVariant.o \
PropVariantConversions.o \
Registry.o \
Synchronization.o \
System.o \
Time.o \
Window.o \

WIN_CTRL_OBJS = \
Dialog.o \
Controls.o \

7ZIP_COMMON_OBJS = \
CreateCoder.o \
FilePathAutoRename.o \
FileStreams.o \
FilterCoder.o \
ProgressUtils.o \
StreamUtils.o \

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
WorkDir.o \
ZipRegistry.o \

AR_COMMON_OBJS = \
 OutStreamWithCRC.o

FM_OBJS = \
ExtractCallback.o \
FormatUtils.o \
LangUtils.o \
ProgramLocation.o \
SplitUtils.o \
StringUtils.o \
MessagesDialog.o \
MessagesDialog_rc.o \
OverwriteDialog.o \
OverwriteDialog_rc.o \
PasswordDialog.o \
PasswordDialog_rc.o \
ProgressDialog2.o \
ProgressDialog2_rc.o \


C_OBJS = \
  Alloc.o \
  Threads.o \

OBJS=\
  $(GUI_OBJS) \
  $(COMMON_OBJS) \
  $(WIN_OBJS) \
  $(WIN_CTRL_OBJS) \
  $(7ZIP_COMMON_OBJS) \
  $(UI_COMMON_OBJS) \
  $(AR_COMMON_OBJS) \
  $(FM_OBJS)\
  MyMessages.o \
  CopyCoder.o \
  $(C_OBJS) \
  MyWindows.o \
  wine_date_and_time.o \
  wine_GetXXXDefaultLangID.o \
  $(OBJ_CRC32)

include ../../../../makefile.glb

