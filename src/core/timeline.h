/*
 * timeline.h
 *
 *  Created on: 05/ago/2011
 *      Author: fhorse
 */

#ifndef TIMELINE_H_
#define TIMELINE_H_

#include "common.h"

enum timeline_misc {
	TL_NORMAL,
	TL_SAVE_SLOT,
	TL_SNAP_SEC = 5,
	TL_SNAPS_TOT = 14,
	TL_SNAP_FREE = TL_SNAPS_TOT - 1,
	TL_SNAPS = TL_SNAPS_TOT - 1
};

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC struct _timeline {
	BYTE *start;
	BYTE *snaps[TL_SNAPS_TOT];
	SWORD snap;
	SWORD snaps_fill;
	DBWORD snap_size;
	DBWORD preview;
	DBWORD frames;
	DBWORD frames_snap;
	BYTE update;
	BYTE button;
	BYTE key;
} tl;

EXTERNC BYTE timeline_init(void);
EXTERNC void timeline_snap(BYTE mode);
EXTERNC void timeline_preview(BYTE snap);
EXTERNC void timeline_back(BYTE mode, BYTE snap);
EXTERNC void timeline_quit(void);

#undef EXTERNC

#endif /* TIMELINE_H_ */
