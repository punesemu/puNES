CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

CMAKE_POLICY(
    SET CMP0048 NEW
)

PROJECT (lib7zip
	VERSION 4.0.0
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

# Use submodule 7zip source by default, allow override with SEVENZIP_SOURCE_DIR
IF(NOT DEFINED SEVENZIP_SOURCE_DIR)
  SET(SEVENZIP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/7zip")
ENDIF()

IF (NOT IS_DIRECTORY ${SEVENZIP_SOURCE_DIR})
  MESSAGE(FATAL_ERROR "7zip source not found at ${SEVENZIP_SOURCE_DIR}. Please ensure git submodule is initialized: git submodule update --init --recursive")
ENDIF()

SET(SEVENZIP_INCLUDE_PATH   "${SEVENZIP_SOURCE_DIR}"
  "${SEVENZIP_SOURCE_DIR}/CPP"
  "${SEVENZIP_SOURCE_DIR}/CPP/Common"
  "${SEVENZIP_SOURCE_DIR}/CPP/Windows"
  "${SEVENZIP_SOURCE_DIR}/C"
  "${CMAKE_CURRENT_SOURCE_DIR}/includes"
)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(warnings -Wall -Wextra -Werror -Wno-unused-parameter)
  set(cxx_warnings -Wno-class-memaccess)
  set(no_undefined -Wl,--no-undefined)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(warnings -Wall -Wextra -Werror -Wno-inconsistent-missing-override -Wno-unused-parameter)
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
ADD_COMPILE_OPTIONS(-fPIC -std=c++14 ${warnings} ${cxx_warnings})
ADD_LINK_OPTIONS(${no_undefined})
endif()

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)
