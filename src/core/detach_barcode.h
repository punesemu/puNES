// Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)

#ifndef DETACH_BARCODE_H_
#define DETACH_BARCODE_H_

#include "common.h"
#include "save_slot.h"

typedef struct _detach_barcode {
	BYTE enabled;
	BYTE data[256];
	DBWORD pos;
	DBWORD count;
	BYTE out;
} _detach_barcode;

extern _detach_barcode detach_barcode;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void init_detach_barcode(BYTE reset);
EXTERNC BYTE detach_barcode_save_mapper(BYTE mode, BYTE slot, FILE *fp);

EXTERNC int detach_barcode_bcode(const uTCHAR *rcode);

#undef EXTERNC

#endif /* DETACH_BARCODE_H_ */
