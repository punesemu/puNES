/*
 * unif.h
 *
 *  Created on: 03/mag/2014
 *      Author: fhorse
 */

#ifndef UNIF_H_
#define UNIF_H_

#include "common.h"

enum { UNIF_MAPPER = 0x1002 };

struct _unif {
	BYTE finded;
	WORD internal_mapper;
	char board[64];
	char *stripped_board;
	char name[256];

	struct _header {
		char identification[4];
		uint32_t revision;
		BYTE expansion[24];
	} header;
	struct _chunk {
		char id[4];
		uint32_t length;
	} chunk;
	struct _unif_chr {
		size_t size;
		BYTE *pnt;
	} chr;
} unif;

BYTE unif_load_rom(void);

#endif /* UNIF_H_ */
