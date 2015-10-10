/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef VERSION_H_
#define VERSION_H_

//#if !defined (RELEASE)
//#include "build.h"
//#endif

#define VER1        "0"
#define VER1_INT    0

#define VER2        "99"
#define VER2_INT    99

#if defined (RELEASE)
#define VERSION     VER1 "." VER2
#else
#define VERSION     VER1 "." VER2 " WIP"
#endif

#define NAME        "puNES"
#define AUTHOR      "FHorse"
#define COMPILED    "10092015191317"
#define WEBSITE     "http://forums.nesdev.com/viewtopic.php?f=3&amp;t=6928"
#define DONATE      "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=QPPXNRL5NAHDC"

#define COPYRIGTH   " 2015 by " AUTHOR
#define COPYRANSI   "(C)" COPYRIGTH
#define COPYRUTF8   "&#169;" COPYRIGTH

#endif /* VERSION_H_ */
