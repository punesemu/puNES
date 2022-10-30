set(WIN_STATIC_QT_LIBS "")
set(win_static_qt_libs_list
	plugins/platforms/libqwindows.a
	libQt5PlatformSupport.a
	libQt5EventDispatcherSupport.a
	libQt5FontDatabaseSupport.a
	libqtfreetype.a
	libQt5ThemeSupport.a
	libqwindowsvistastyle.a
	plugins/imageformats/libqgif.a
	plugins/imageformats/libqico.a
	plugins/imageformats/libqjpeg.a
	plugins/imageformats/libqsvg.a
	libqtpcre.a
	libqtharfbuzzng.a
	)

get_target_property(QT_LIB_LOC Qt5::Core IMPORTED_LOCATION_RELEASE)
get_filename_component(QT_LIB_LOC ${QT_LIB_LOC} DIRECTORY)

foreach(win_static_qt_lib ${win_static_qt_libs_list})
	get_filename_component(file_name ${win_static_qt_lib} NAME)
	message(STATUS "Looking for ${file_name}")
	if(EXISTS ${QT_LIB_LOC}/${win_static_qt_lib})
		message(STATUS "Looking for ${file_name} - found")
		set(WIN_STATIC_QT_LIBS ${WIN_STATIC_QT_LIBS} ${QT_LIB_LOC}/${win_static_qt_lib})

		if(file_name STREQUAL "libqwindows.a")
			add_compile_definitions(QT5_PLUGIN_QWINDOWS)
		elseif(file_name STREQUAL "libqwindowsvistastyle.a")
			add_compile_definitions(QT_PLUGIN_QWINDOWSVISTASTYLE)
			include(CheckLibraryExists)
			check_library_exists(uxtheme main "" UXTHEME_LIB)
			if(UXTHEME_LIB)
				find_library(UXTHEME_LIB ${UXTHEME_LIB})
				if(NOT USBHID_LIB)
					message(FATAL_ERROR "uxtheme library not found")
				endif()
				set(WIN_STATIC_QT_LIBS ${WIN_STATIC_QT_LIBS} ${UXTHEME_LIB})
			endif()
		elseif(file_name STREQUAL "libqgif.a")
			add_compile_definitions(QT_PLUGIN_QGIF)
		elseif(file_name STREQUAL "libqico.a")
			add_compile_definitions(QT_PLUGIN_QICO)
		elseif(file_name STREQUAL "libqjpeg.a")
			add_compile_definitions(QT_PLUGIN_QJPEG)
		elseif(file_name STREQUAL "libqsvg.a")
			add_compile_definitions(QT_PLUGIN_QSVG)
		endif()
	else()
		if(file_name STREQUAL "libqwindows.a")
			message(FATAL_ERROR "libqwindows.a not found")
		else()
			message(STATUS "Looking for ${file_name} - not found")
		endif()
	endif()
endforeach()
