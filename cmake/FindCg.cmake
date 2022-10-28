find_package(PkgConfig)
pkg_check_modules(PC_Cg QUIET nvidia-cg-toolkit)
pkg_check_modules(PC_CgGL QUIET nvidia-cg-toolkit-gl)

find_path(Cg_INCLUDE_DIR
	NAMES Cg/cg.h
	PATHS ${PC_Cg_INCLUDE_DIRS}
)
find_library(Cg_LIBRARY
	NAMES Cg
	PATHS ${PC_Cg_LIBRARY_DIRS}
)

find_path(CgGL_INCLUDE_DIR
	NAMES Cg/cgGL.h
	PATHS ${PC_CgGL_INCLUDE_DIRS}
)
find_library(CgGL_LIBRARY
	NAMES CgGL
	PATHS ${PC_CgGL_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cg
	DEFAULT_MSG
	Cg_LIBRARY CgGL_LIBRARY
	Cg_INCLUDE_DIR CgGL_INCLUDE_DIR
)

if(Cg_FOUND)
	if(NOT TARGET Cg::Cg)
		add_library(Cg::Cg UNKNOWN IMPORTED)
		set_target_properties(Cg::Cg PROPERTIES
			IMPORTED_LOCATION "${Cg_LIBRARY}"
			INTERFACE_COMPILE_OPTIONS "${PC_Cg_CFLAGS_OTHER}"
			INTERFACE_INCLUDE_DIRECTORIES "${Cg_INCLUDE_DIR}"
		)
	endif()
	if(NOT TARGET Cg::CgGL)
		add_library(Cg::CgGL UNKNOWN IMPORTED)
		set_target_properties(Cg::CgGL PROPERTIES
			IMPORTED_LOCATION "${CgGL_LIBRARY}"
			INTERFACE_COMPILE_OPTIONS "${PC_CgGL_CFLAGS_OTHER}"
			INTERFACE_INCLUDE_DIRECTORIES "${CgGL_INCLUDE_DIR}"
			)
		target_link_libraries(Cg::CgGL INTERFACE Cg::Cg)
	endif()
endif()

mark_as_advanced(
	Cg_INCLUDE_DIR
	CgGL_INCLUDE_DIR
	Cg_LIBRARY
	CgGL_LIBRARY
)
