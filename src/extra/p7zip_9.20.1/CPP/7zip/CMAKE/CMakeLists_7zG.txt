
add_definitions( -DLANG -DEXTERNAL_LZMA -DEXTERNAL_CODECS -DBENCH_MT )  

IF(APPLE)
  add_definitions(-DENV_MACOSX  -D__WXMAC__)
ENDIF(APPLE)


find_package(wxWidgets COMPONENTS core base adv REQUIRED)

include( ${wxWidgets_USE_FILE} )

add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGE_FILES)

include_directories(
	../../../myWindows
	../../../
	../../../include_windows
)
   



add_executable(7zG

        ../../UI/GUI/CompressDialog.cpp
        ../../UI/GUI/CompressDialog_rc.cpp
        ../../UI/GUI/ExtractDialog.cpp
        ../../UI/GUI/ExtractDialog_rc.cpp
        ../../UI/GUI/BenchmarkDialog.cpp
        ../../UI/GUI/BenchmarkDialog_rc.cpp
        ../../UI/GUI/ExtractGUI.cpp
        ../../UI/GUI/GUI.cpp
        ../../UI/GUI/wxGUI.cpp
        ../../UI/GUI/UpdateCallbackGUI.cpp
        ../../UI/GUI/UpdateGUI.cpp
        ../../UI/Common/ArchiveCommandLine.cpp
        ../../UI/Common/ArchiveExtractCallback.cpp
        ../../UI/Common/ArchiveOpenCallback.cpp
        ../../UI/Common/Bench.cpp
        ../../UI/Common/DefaultName.cpp
        ../../UI/Common/EnumDirItems.cpp
        ../../UI/Common/Extract.cpp
        ../../UI/Common/ExtractingFilePath.cpp
        ../../UI/Common/LoadCodecs.cpp
        ../../UI/Common/OpenArchive.cpp
        ../../UI/Common/PropIDUtils.cpp
        ../../UI/Common/SetProperties.cpp
        ../../UI/Common/SortUtils.cpp
        ../../UI/Common/TempFiles.cpp
        ../../UI/Common/Update.cpp
        ../../UI/Common/UpdateAction.cpp
        ../../UI/Common/UpdateCallback.cpp
        ../../UI/Common/UpdatePair.cpp
        ../../UI/Common/UpdateProduce.cpp
        ../../UI/Common/WorkDir.cpp
        ../../UI/Common/ZipRegistry.cpp
        ../../UI/Explorer/MyMessages.cpp


        ../../UI/FileManager/FormatUtils.cpp
        ../../UI/FileManager/ProgramLocation.cpp
        ../../UI/FileManager/ProgressDialog2.cpp
        ../../UI/FileManager/ProgressDialog2_rc.cpp
        ../../UI/FileManager/MessagesDialog.cpp
        ../../UI/FileManager/MessagesDialog_rc.cpp
        ../../UI/FileManager/OverwriteDialog.cpp
        ../../UI/FileManager/OverwriteDialog_rc.cpp
        ../../UI/FileManager/PasswordDialog.cpp
        ../../UI/FileManager/PasswordDialog_rc.cpp
        ../../UI/FileManager/ExtractCallback.cpp
        ../../UI/FileManager/SplitUtils.cpp
        ../../UI/FileManager/StringUtils.cpp
        ../../UI/FileManager/LangUtils.cpp
        ../../Archive/Common/OutStreamWithCRC.cpp
        ../../Common/CreateCoder.cpp
        ../../Common/FilePathAutoRename.cpp
        ../../Common/FileStreams.cpp
        ../../Common/FilterCoder.cpp
        ../../Common/ProgressUtils.cpp
        ../../Common/StreamUtils.cpp
        ../../Compress/CopyCoder.cpp
        ../../../Common/CommandLineParser.cpp
        ../../../Common/CRC.cpp
        ../../../Common/IntToString.cpp
        ../../../Common/Lang.cpp
        ../../../Common/ListFileUtils.cpp
        ../../../Common/MyString.cpp
        ../../../Common/MyVector.cpp
        ../../../Common/StringConvert.cpp
        ../../../Common/StringToInt.cpp
        ../../../Common/TextConfig.cpp
        ../../../Common/UTFConvert.cpp
        ../../../Common/Wildcard.cpp
        ../../../Common/MyWindows.cpp
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
        ../../../Windows/System.cpp
        ../../../Windows/Window.cpp
        ../../../Windows/Control/Dialog.cpp
        ../../../Windows/Control/Controls.cpp

        ../../../myWindows/wine_date_and_time.cpp
        ../../../myWindows/wine_GetXXXDefaultLangID.cpp

 ../../../../C/Alloc.c
 ../../../../C/Threads.c
  ../../../../C/7zCrc.c
 ../../../../C/7zCrcOpt.c
 
 )


IF(HAVE_PTHREADS)
 TARGET_LINK_LIBRARIES(7zG

 ${wxWidgets_LIBRARIES}

 ${CMAKE_THREAD_LIBS_INIT})
ENDIF(HAVE_PTHREADS)


