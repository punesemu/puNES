include(CheckLibraryExists)
include(CheckCSourceCompiles)

set(ORIG_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
set(ORIG_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")

set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -include stdint.h")

check_library_exists(usbhid hid_init "" LIBUSBHID)

if(LIBUSBHID)
	check_include_file(usbhid.h HAVE_USBHID_H)
	if(HAVE_USBHID_H)
		add_definitions(-DHAVE_USBHID_H)
		set(USB_CFLAGS -DHAVE_USBHID_H)
	endif()

	check_include_file(libusbhid.h HAVE_LIBUSBHID_H)
	if(HAVE_LIBUSBHID_H)
		add_definitions(-DHAVE_LIBUSBHID_H)
		set(USB_CFLAGS "${USB_CFLAGS} -DHAVE_LIBUSBHID_H")
	endif()
	set(LIBUSBHID usbhid)
else()
	check_include_file(usb.h HAVE_USB_H)
	if(HAVE_USB_H)
		set(USB_CFLAGS -DHAVE_USB_H)
	endif()
	check_include_file(libusb.h HAVE_LIBUSB_H)
	if(HAVE_LIBUSB_H)
		set(USB_CFLAGS "${USB_CFLAGS} -DHAVE_LIBUSB_H")
	endif()
	check_library_exists(usb hid_init "" LIBUSBHID)
	if(LIBUSBHID)
		set(LIBUSBHID usb)
	endif()
endif()

if (NOT LIBUSBHID)
	message(FATAL_ERROR "usb library not_found")
endif()

find_library(USBHID_LIB ${LIBUSBHID})
if(NOT USBHID_LIB)
	message(FATAL_ERROR "usb library not_found")
endif()
set(USB_LIBS ${USBHID_LIB})

set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS} ${USB_CFLAGS}")
set(CMAKE_REQUIRED_LIBRARIES "${USB_LIBS}")

check_c_source_compiles("
	#include <sys/types.h>
	#if defined(HAVE_USB_H)
	#include <usb.h>
	#endif
	#ifdef __DragonFly__
	# include <bus/u4b/usb.h>
	# include <bus/u4b/usbhid.h>
	#else
	# include <dev/usb/usb.h>
	# include <dev/usb/usbhid.h>
	#endif
	#if defined(HAVE_USBHID_H)
	#include <usbhid.h>
	#elif defined(HAVE_LIBUSB_H)
	#include <libusb.h>
	#elif defined(HAVE_LIBUSBHID_H)
	#include <libusbhid.h>
	#endif
	int main(int argc, char **argv) {
		struct report_desc *repdesc;
		struct usb_ctl_report *repbuf;
		hid_kind_t hidkind;
		return 0;
	}" HAVE_USBHID)
if(HAVE_USBHID)
	check_c_source_compiles("
		#include <sys/types.h>
		#if defined(HAVE_USB_H)
		#include <usb.h>
		#endif
		#ifdef __DragonFly__
		# include <bus/u4b/usb.h>
		# include <bus/u4b/usbhid.h>
		#else
		# include <dev/usb/usb.h>
		# include <dev/usb/usbhid.h>
		#endif
		#if defined(HAVE_USBHID_H)
		#include <usbhid.h>
		#elif defined(HAVE_LIBUSB_H)
		#include <libusb.h>
		#elif defined(HAVE_LIBUSBHID_H)
		#include <libusbhid.h>
		#endif
		int main(int argc, char** argv) {
			struct usb_ctl_report buf;
			if (buf.ucr_data) { }
			return 0;
		}" HAVE_USBHID_UCR_DATA)
	if(HAVE_USBHID_UCR_DATA)
		add_definitions(-DUSBHID_UCR_DATA)
		set(USB_CFLAGS "${USB_CFLAGS} -DUSBHID_UCR_DATA")
	endif()

	check_c_source_compiles("
		#include <sys/types.h>
		#if defined(HAVE_USB_H)
		#include <usb.h>
		#endif
		#ifdef __DragonFly__
		#include <bus/u4b/usb.h>
		#include <bus/u4b/usbhid.h>
		#else
		#include <dev/usb/usb.h>
		#include <dev/usb/usbhid.h>
		#endif
		#if defined(HAVE_USBHID_H)
		#include <usbhid.h>
		#elif defined(HAVE_LIBUSB_H)
		#include <libusb.h>
		#elif defined(HAVE_LIBUSBHID_H)
		#include <libusbhid.h>
		#endif
		int main(int argc, char **argv) {
			report_desc_t d;
			hid_start_parse(d, 1, 1);
			return 0;
		}" HAVE_USBHID_NEW)
	if(HAVE_USBHID_NEW)
		add_definitions(-DUSBHID_NEW)
		set(USB_CFLAGS "${USB_CFLAGS} -DUSBHID_NEW")
	endif()

	check_c_source_compiles("
		#include <machine/joystick.h>
		int main(int argc, char** argv) {
			struct joystick t;
			return 0;
		}" HAVE_MACHINE_JOYSTICK)
	if(HAVE_MACHINE_JOYSTICK)
		add_definitions(-DUSBHID_MACHINE_JOYSTICK)
		set(USB_CFLAGS "${USB_CFLAGS} -DUSBHID_MACHINE_JOYSTICK")
	endif()

	set(CMAKE_REQUIRED_FLAGS "${ORIG_CMAKE_REQUIRED_FLAGS}")
	set(CMAKE_REQUIRED_LIBRARIES "${ORIG_CMAKE_REQUIRED_LIBRARIES}")

	if(NOT TARGET UsbHid::UsbHid)
		add_library(UsbHid::UsbHid UNKNOWN IMPORTED)
		set_target_properties(UsbHid::UsbHid PROPERTIES
			IMPORTED_LOCATION "${USB_LIBS}"
		)
	endif()
endif()
