
add_definitions(-DLANG -DNEW_FOLDER_INTERFACE -DEXTERNAL_CODECS)  

IF(APPLE)
  add_definitions(-DENV_MACOSX)

ENDIF(APPLE)


find_package(wxWidgets COMPONENTS core base adv REQUIRED)

include( ${wxWidgets_USE_FILE} )

add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)

include_directories(
	../../../myWindows
	../../../
	../../../include_windows
)




add_executable(7zFM


../../UI/Agent/Agent.cpp
../../UI/Agent/AgentOut.cpp
../../UI/Agent/AgentProxy.cpp
../../UI/Agent/ArchiveFolder.cpp
../../UI/Agent/ArchiveFolderOpen.cpp
../../UI/Agent/ArchiveFolderOut.cpp
../../UI/Agent/UpdateCallbackAgent.cpp

 ../../UI/FileManager/ClassDefs.cpp
 ../../UI/FileManager/App.cpp
 ../../UI/FileManager/wxFM.cpp
 ../../UI/FileManager/FM.cpp
 ../../UI/FileManager/FM_rc.cpp
 ../../UI/FileManager/OpenCallback.cpp
 ../../UI/FileManager/MyLoadMenu.cpp
 ../../UI/FileManager/Panel.cpp
 ../../UI/FileManager/PanelCrc.cpp
 ../../UI/FileManager/PanelCopy.cpp
 ../../UI/FileManager/PanelItems.cpp
 ../../UI/FileManager/PanelListNotify.cpp
 ../../UI/FileManager/PanelFolderChange.cpp
 ../../UI/FileManager/PanelItemOpen.cpp
 ../../UI/FileManager/PanelMenu.cpp
 ../../UI/FileManager/PanelOperations.cpp
 ../../UI/FileManager/PanelSelect.cpp
 ../../UI/FileManager/PanelSort.cpp
 ../../UI/FileManager/PanelSplitFile.cpp
 ../../UI/FileManager/PropertyName.cpp
 ../../UI/FileManager/RegistryUtils.cpp
 ../../UI/FileManager/ViewSettings.cpp
 ../../UI/FileManager/UpdateCallback100.cpp
 ../../UI/FileManager/FileFolderPluginOpen.cpp
 ../../UI/FileManager/RootFolder.cpp
 ../../UI/FileManager/FSDrives.cpp
 ../../UI/FileManager/FSFolder.cpp
 ../../UI/FileManager/FSFolderCopy.cpp
 ../../UI/FileManager/FormatUtils.cpp
 ../../UI/FileManager/SysIconUtils.cpp
 ../../UI/FileManager/TextPairs.cpp

 ../../UI/FileManager/ExtractCallback.cpp
 ../../UI/FileManager/ProgramLocation.cpp
 ../../UI/FileManager/CopyDialog.cpp
 ../../UI/FileManager/CopyDialog_rc.cpp
 ../../UI/FileManager/ListViewDialog.cpp
 ../../UI/FileManager/ListViewDialog_rc.cpp
 ../../UI/FileManager/SplitDialog.cpp
 ../../UI/FileManager/SplitDialog_rc.cpp
 ../../UI/FileManager/SplitUtils.cpp
 ../../UI/FileManager/StringUtils.cpp
 ../../UI/FileManager/ProgressDialog2.cpp
 ../../UI/FileManager/ProgressDialog2_rc.cpp
 ../../UI/FileManager/MessagesDialog.cpp
 ../../UI/FileManager/MessagesDialog_rc.cpp
 ../../UI/FileManager/ComboDialog.cpp
 ../../UI/FileManager/ComboDialog_rc.cpp
 ../../UI/FileManager/OverwriteDialog.cpp
 ../../UI/FileManager/OverwriteDialog_rc.cpp
 ../../UI/FileManager/PasswordDialog.cpp
 ../../UI/FileManager/PasswordDialog_rc.cpp
 ../../UI/FileManager/LangUtils.cpp
  
../../UI/Common/ArchiveName.cpp
../../UI/Common/CompressCall.cpp
../../UI/Common/WorkDir.cpp  
../../UI/Common/ArchiveExtractCallback.cpp
../../UI/Common/ArchiveOpenCallback.cpp
../../UI/Common/DefaultName.cpp
../../UI/Common/EnumDirItems.cpp
../../UI/Common/LoadCodecs.cpp
../../UI/Common/OpenArchive.cpp
../../UI/Common/ZipRegistry.cpp
../../UI/Common/ExtractingFilePath.cpp
../../UI/Common/PropIDUtils.cpp
../../UI/Common/SortUtils.cpp
../../UI/Common/UpdateAction.cpp
../../UI/Common/UpdateCallback.cpp
../../UI/Common/UpdatePair.cpp
../../UI/Common/UpdateProduce.cpp

        ../../Archive/Common/OutStreamWithCRC.cpp
        ../../Common/FilePathAutoRename.cpp
        ../../Common/FileStreams.cpp
        ../../Common/ProgressUtils.cpp
        ../../Common/StreamUtils.cpp
        ../../../Common/TextConfig.cpp
        ../../../Common/UTFConvert.cpp
        ../../Compress/CopyCoder.cpp
        ../../../Common/MyString.cpp
        ../../../Common/MyVector.cpp
        ../../../Common/StringConvert.cpp
        ../../../Common/StringToInt.cpp
        ../../../Common/IntToString.cpp        
        ../../../Common/MyWindows.cpp
        ../../../Common/Wildcard.cpp
        
        ../../../Windows/DLL.cpp    
        ../../../Windows/Error.cpp
        ../../../Windows/FileDir.cpp
        ../../../Windows/FileFind.cpp
        ../../../Windows/FileIO.cpp
        ../../../Windows/FileName.cpp
        ../../../Windows/PropVariant.cpp
        ../../../Windows/PropVariantConversions.cpp
        ../../../Windows/Registry.cpp
        ../../../Windows/Synchronization.cpp
        ../../../Windows/Time.cpp
        ../../../Windows/Clipboard.cpp
        ../../../Windows/System.cpp
        ../../../Windows/Window.cpp
        ../../../Windows/Control/Dialog.cpp
        ../../../Windows/Control/Controls.cpp
        ../../../Windows/Control/Window2.cpp

        ../../../Common/Lang.cpp
        ../../../Common/CRC.cpp
        ../../../myWindows/wine_date_and_time.cpp
        ../../../myWindows/wine_GetXXXDefaultLangID.cpp

	../../../../C/Alloc.c
	../../../../C/Sha256.c
	../../../../C/Sort.c
	../../../../C/Threads.c
	../../../../C/7zCrc.c
	../../../../C/7zCrcOpt.c
 )

IF(HAVE_PTHREADS)
 TARGET_LINK_LIBRARIES(7zFM 

 ${wxWidgets_LIBRARIES}

 ${CMAKE_THREAD_LIBS_INIT})
ENDIF(HAVE_PTHREADS)


