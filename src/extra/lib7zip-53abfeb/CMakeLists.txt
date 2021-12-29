CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

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
  #set(warnings -Wall -Wextra -Werror -Wno-unused-parameter)
  #set(cxx_warnings -Wno-class-memaccess)
  set(warnings "")
  set(cxx_warnings "")
  set(no_undefined -Wl,--no-undefined)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #set(warnings -Wall -Wextra -Werror -Wno-inconsistent-missing-override -Wno-unused-parameter)
  set(warnings "")
  set(cxx_warnings "")
  set(no_undefined -Wl,-undefined,error)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings /W4 /WX /EHsc)
    set(no_undefined "")
    set(cxx_warnings "")
endif()

SET(CXX_STANDARD_REQUIRED OFF)
SET(CXX_EXTENSION NO)

if (NOT CONFIGURED_ONCE)
if ("${CMAKE_C_PLATFORM_ID}" STREQUAL "MinGW")
  ADD_COMPILE_OPTIONS(-DUNICODE -D_UNICODE -DWIN32)
endif()
ADD_COMPILE_OPTIONS(-fPIC -std=c++14 ${warnings} ${cxx_warnings})
ADD_LINK_OPTIONS(${no_undefined})
endif()

ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(test)

install(FILES dummy DESTINATION bin OPTIONAL)
