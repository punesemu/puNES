PROG=../../../../bin/7zFM$(BINSUFFIX)

LOCAL_FLAGS=\
	-DUNICODE -D_UNICODE \
	-DLANG \
	-DNEW_FOLDER_INTERFACE \
	-DEXTERNAL_CODECS \
	-I. \
	`wx-config --unicode=yes --cxxflags`

include ../../../../makefile.crc32
include ../../../../makefile.machine

PCH_NAME=$(PRE_COMPILED_HEADER)

LIBS=`wx-config --unicode=yes --libs` $(LOCAL_LIBS_DLL)

OBJS=\
OutStreamWithCRC.o \
Agent.o \
AgentOut.o \
AgentProxy.o \
ArchiveFolder.o \
ArchiveFolderOpen.o \
ArchiveFolderOut.o \
UpdateCallbackAgent.o \
ClassDefs.o \
wxFM.o \
FM.o \
FM_rc.o \
App.o \
OpenCallback.o \
MyLoadMenu.o \
Panel.o \
PanelCrc.o \
PanelCopy.o \
PanelItems.o \
PanelListNotify.o \
PanelFolderChange.o \
PanelItemOpen.o \
PanelMenu.o \
PanelOperations.o \
PanelSelect.o \
PanelSort.o \
PanelSplitFile.o \
PropertyName.o \
RegistryUtils.o \
UpdateCallback100.o \
ViewSettings.o \
FileFolderPluginOpen.o \
RootFolder.o \
FSDrives.o \
FSFolder.o \
FSFolderCopy.o \
FormatUtils.o \
SysIconUtils.o \
TextPairs.o \
ExtractCallback.o \
ProgramLocation.o \
CopyDialog.o \
CopyDialog_rc.o \
ListViewDialog.o \
ListViewDialog_rc.o \
ProgressDialog2.o \
ProgressDialog2_rc.o \
SplitDialog.o \
SplitDialog_rc.o \
SplitUtils.o \
StringUtils.o \
ComboDialog.o \
ComboDialog_rc.o \
OverwriteDialog.o \
OverwriteDialog_rc.o \
PasswordDialog.o \
PasswordDialog_rc.o \
MessagesDialog.o \
MessagesDialog_rc.o \
ArchiveExtractCallback.o \
ArchiveOpenCallback.o \
LoadCodecs.o \
OpenArchive.o \
FilePathAutoRename.o \
FileStreams.o \
ProgressUtils.o \
StreamUtils.o \
DefaultName.o \
EnumDirItems.o \
ZipRegistry.o \
ExtractingFilePath.o \
PropIDUtils.o \
UpdateAction.o \
UpdateCallback.o \
UpdatePair.o \
UpdateProduce.o \
SortUtils.o \
TextConfig.o \
UTFConvert.o \
ArchiveName.o \
CompressCall.o \
WorkDir.o \
CopyCoder.o \
MyString.o \
MyVector.o \
StringConvert.o \
StringToInt.o \
IntToString.o \
Wildcard.o \
DLL.o \
Error.o \
FileDir.o \
FileFind.o \
FileIO.o \
FileName.o \
PropVariant.o \
PropVariantConversions.o \
Synchronization.o \
Clipboard.o \
Time.o \
System.o \
MyWindows.o \
Registry.o \
Window.o \
Dialog.o \
Controls.o \
Window2.o \
LangUtils.o \
Lang.o \
CRC.o \
wine_date_and_time.o \
wine_GetXXXDefaultLangID.o \
Sort.o \
Sha256.o \
Alloc.o \
Threads.o \
$(OBJ_CRC32)


OBJS2=\
RegistryAssociations.o \


OBJS_OUT=\
ArchiveCommandLine.o \
SetProperties.o \
TempFiles.o \
Update.o \
MyMessages.o \
FormatUtils.o \
StringUtils.o \
CommandLineParser.o \
ListFileUtils.o \

include ../../../../makefile.glb

