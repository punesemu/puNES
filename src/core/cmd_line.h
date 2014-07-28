/*
 * cmd_line.h
 *
 *  Created on: 03/ago/2011
 *      Author: fhorse
 */

#ifndef CMD_LINE_H_
#define CMD_LINE_H_

#include "common.h"

BYTE cmd_line_parse(int argc, char **argv);
BYTE cmd_line_check_portable(int argc, char **argv);

#endif /* CMD_LINE_H_ */
