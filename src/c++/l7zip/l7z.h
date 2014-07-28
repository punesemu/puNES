/*
 * l7z.h
 *
 *  Created on: 22/dic/2013
 *      Author: fhorse
 */

#ifndef L7Z_H_
#define L7Z_H_

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include "common.h"
#include "uncompress.h"

EXTERNC BYTE l7z_init(void);
EXTERNC void l7z_quit(void);
EXTERNC BYTE l7z_present(void);
EXTERNC BYTE l7z_control_ext(char *ext);
EXTERNC BYTE l7z_control_in_archive(void);
EXTERNC BYTE l7z_file_from_archive(_uncomp_file_data *file);
EXTERNC BYTE l7z_name_file_compress(_uncomp_file_data *file);

#undef EXTERNC

#endif /* L7Z_H_ */
