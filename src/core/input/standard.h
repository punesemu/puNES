/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#ifndef INPUT_STANDARD_H_
#define INPUT_STANDARD_H_

#include "../input.h"

BYTE input_wr_reg_standard(BYTE value);
BYTE input_rd_reg_standard(BYTE openbus, WORD **screen_index, BYTE nport);
void input_wr_standard(BYTE *value, BYTE nport);

BYTE input_wr_reg_standard_vs(BYTE value);
BYTE input_rd_reg_standard_vs(BYTE openbus, WORD **screen_index, BYTE nport);

void input_add_event_standard(BYTE index);
BYTE input_decode_event_standard(BYTE mode, DBWORD event, BYTE type, _port *port);

#endif /* INPUT_STANDARD_H_ */
