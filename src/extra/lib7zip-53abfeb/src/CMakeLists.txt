SET(lib7zip_src
7ZipArchive.cpp             7ZipCompressCodecsInfo.cpp  7ZipFunctions.h             OSFunctions_UnixLike.cpp
7ZipArchiveItem.cpp         7ZipCompressCodecsInfo.h    7ZipInStreamWrapper.cpp     HelperFuncs.cpp             OSFunctions_UnixLike.h
7ZipArchiveOpenCallback.cpp 7ZipDllHandler.cpp          7ZipInStreamWrapper.h       HelperFuncs.h               OSFunctions_Win32.cpp
7ZipArchiveOpenCallback.h   7ZipDllHandler.h            7ZipObjectPtrArray.cpp      OSFunctions.h               OSFunctions_Win32.h
7ZipCodecInfo.cpp           7ZipFormatInfo.cpp          7ZipOpenArchive.cpp         OSFunctions_OS2.cpp         lib7zip.h
7ZipCodecInfo.h             7ZipFormatInfo.h            7zipLibrary.cpp             OSFunctions_OS2.h
)

if ("${CMAKE_C_PLATFORM_ID}" STREQUAL "MinGW")
    SET(lib7zip_src ${lib7zip_src} GUIDs.h GUIDs.cpp)
endif()

SET(lib7zip_NODIST_SOURCES ${P7ZIP_SOURCE_DIR}/CPP/Common/MyWindows.cpp
			   ${P7ZIP_SOURCE_DIR}/CPP/Windows/PropVariant.cpp
)

ADD_LIBRARY(lib7zip STATIC ${lib7zip_src}
   ${lib7zip_NODIST_SOURCES}
   )

SET_TARGET_PROPERTIES(lib7zip PROPERTIES
    OUTPUT_NAME "7zip"
)

SET_TARGET_PROPERTIES(lib7zip PROPERTIES LINKER_LANGUAGE CXX)

TARGET_INCLUDE_DIRECTORIES(lib7zip PRIVATE
  "${P7ZIP_INCLUDE_PATH}"
)

IF (BUILD_SHARED_LIB)
ADD_LIBRARY(lib7zip_shared SHARED ${lib7zip_src}
   ${lib7zip_NODIST_SOURCES}
)
SET_TARGET_PROPERTIES(lib7zip_shared PROPERTIES
    OUTPUT_NAME "7zip"
)

SET_TARGET_PROPERTIES(lib7zip_shared PROPERTIES LINKER_LANGUAGE CXX)

TARGET_INCLUDE_DIRECTORIES(lib7zip_shared PRIVATE
  "${P7ZIP_INCLUDE_PATH}"
)
ENDIF()
