/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include "common.h"

enum debugger_mode { DBG_NODBG, DBG_GO, DBG_STEP, DBG_BREAKPOINT, DBG_SLEEP };

typedef struct _debugger {
	BYTE mode;
	WORD breakpoint;
	BYTE breakframe;
} _debugger;
typedef struct _debugger_breakpoint {

} _debugger_breakpoint;

extern _debugger debugger;

#endif /* DEBUGGER_H_ */
