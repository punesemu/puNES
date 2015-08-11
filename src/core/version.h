/*
 * version.h
 *
 *  Created on: 08/set/2010
 *      Author: fhorse
 */

#ifndef VERSION_H_
#define VERSION_H_

#if !defined (RELEASE)
#include "build.h"
#endif

#define VER1        "0"
#define VER1_INT    0

#define VER2        "95"
#define VER2_INT    95

#define VERSION     VER1 "." VER2 " WIP"
#define NAME        "puNES"
#define AUTHOR      "FHorse"
#define COMPILED    "08082015121729"
#define WEBSITE     "http://forums.nesdev.com/viewtopic.php?f=3&amp;t=6928"
#define DONATE      "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=QPPXNRL5NAHDC"

#define COPYRIGTH   " 2015 by " AUTHOR
#define COPYRANSI   "(C)" COPYRIGTH
#define COPYRUTF8   "&#169;" COPYRIGTH

#endif /* VERSION_H_ */
