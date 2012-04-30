/*
 * timeline.h
 *
 *  Created on: 05/ago/2011
 *      Author: fhorse
 */

#ifndef TIMELINE_H_
#define TIMELINE_H_

#include "common.h"

enum {
	TLNORMAL,
	TLSAVESTATE,
	TLSNAPSEC = 5,
	TLSNAPSTOT = 14,
	TLSNAPFREE = TLSNAPSTOT - 1,
	TLSNAPS = TLSNAPSTOT - 1
};

struct _timeline {
	BYTE *start;
	BYTE *snaps[TLSNAPSTOT];
	SWORD snap;
	SWORD snapsFill;
	DBWORD snapSize;
	DBWORD preview;
	DBWORD frames;
	DBWORD framesSnap;
	BYTE update;
	BYTE button;
	BYTE key;
} tl;

BYTE timelineInit(void);
void timelineSnap(BYTE mode);
void timelinePreview(BYTE snap);
void timelineBack(BYTE mode, BYTE snap);
void timelineQuit(void);

#endif /* TIMELINE_H_ */
