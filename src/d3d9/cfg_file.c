/*
 * cfg_file.c
 *
 *  Created on: 31/lug/2011
 *      Author: fhorse
 */

#include "cfg_file.h"

void cfg_file_init(void) {
	cfg = &cfg_from_file;
}
void cfg_file_parse(void) {
	info.on_cfg = TRUE;

	return;
}
void cfg_file_save(void) {
	return;
}
void cfg_file_pgs_parse(void) {
	return;
}
void cfg_file_pgs_save(void) {
	return;
}
void cfg_file_input_parse(void) {
	return;
}
void cfg_file_input_save(void) {
	return;
}
