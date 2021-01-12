CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

CMAKE_POLICY(
    SET CMP0048 NEW
)

PROJECT (lib7zip
	VERSION 3.0.0
)

INCLUDE(ExternalProject)

IF(APPLE)
  SET(CMAKE_MACOSX_RPATH 1)
  SET(CMAKE_PREFIX_PATH /usr/local)
ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "DEBUG")
  #SET(CMAKE_BUILD_TYPE "RELEASE")
  #SET(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
  #SET(CMAKE_BUILD_TYPE "MINSIZEREL")
ENDIF()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

OPTION(BUILD_SHARED_LIB "build shared library" OFF)
SET(P7ZIP_SOURCE_DIR "" CACHE PATH "pzip/7zip source code path")

IF (NOT IS_DIRECTORY ${P7ZIP_SOURCE_DIR})
  MESSAGE(FATAL_ERROR "must proivde p7zip/7zip source path using -DP7ZIP_SOURCE_DIR")
ENDIF()

SET(P7ZIP_INCLUDE_PATH   "${P7ZIP_SOURCE_DIR}"
  "${P7ZIP_SOURCE_DIR}/CPP"
  "${P7ZIP_SOURCE_DIR}/CPP/myWindows"
  "${P7ZIP_SOURCE_DIR}/CPP/include_windows"
)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  #set(warnings "-Wall -Wextra -Wno-unused-parameter -Wno-class-memaccess")
  set(no_undefined "-Wl,--no-undefined")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #set(warnings "-Wall -Wextra -Wno-inconsistent-missing-override -Wno-unused-parameter")
  set(no_undefined "-Wl,-undefined,error")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(warnings "/W4 /WX /EHsc")
  set(no_undefined "")
endif()

SET(CXX_STANDARD_REQUIRED OFF)
SET(CXX_EXTENSION NO)

if (NOT CONFIGURED_ONCE)
  if ("${CMAKE_C_PLATFORM_ID}" STREQUAL "MinGW")
    set(CMAKE_C_FLAGS "-DUNICODE -D_UNICODE -DWIN32  -fPIC ${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_CXX_FLAGS "-DUNICODE -D_UNICODE -DWIN32  -fPIC ${warnings} -std=c++14"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_LINK_FLAGS "${no_undefined}"
        CACHE STRING "Flags used by the linker during all build types." FORCE)
  else()
    set(CMAKE_C_FLAGS "-fPIC ${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_CXX_FLAGS "-fPIC ${warnings} -std=c++14"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_LINK_FLAGS "${no_undefined}"
        CACHE STRING "Flags used by the linker during all build types." FORCE)
  endif()
endif()

ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(test)

INSTALL(CODE "MESSAGE(\"Nothing to install.\")")
