SET(common_src
    stdafx.cpp stdafx.h
)

SET (test_src Test7Zip.cpp              Test7ZipCryptFileName.cpp Test7ZipMulti.cpp         Test7ZipSignature.cpp
Test7Zip2.cpp             Test7ZipDmg.cpp           Test7ZipRar5.cpp test_archive.cpp
)

FOREACH(f ${test_src})

GET_FILENAME_COMPONENT(test_target_name ${f} NAME_WE)

MESSAGE("generate target ${test_target_name} for ${f}")

ADD_EXECUTABLE(${test_target_name}
    ${common_src}
    ${f}
)

TARGET_INCLUDE_DIRECTORIES(${test_target_name} PRIVATE
     "../src"
)

TARGET_LINK_LIBRARIES(${test_target_name}
  lib7zip
  ${CMAKE_DL_LIBS}
)

ENDFOREACH()
