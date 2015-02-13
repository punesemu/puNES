/*
 * palette.c
 *
 *  Created on: 07/giu/2014
 *      Author: fhorse
 */

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "palette.h"

void palette_save_on_file(const char *file) {
	FILE *fp;

	if ((fp = fopen(file, "wb")) == NULL) {
		fprintf(stderr, "ERROR: Impossible save palette file %s", file);
		return;
	}

	if (!(fwrite((BYTE *) palette_RGB, 64 * 3, 1, fp))) {
		;
	}

	fclose(fp);
}
BYTE palette_load_from_file(const char *file) {
	FILE *fp;

	memset((BYTE *) palette_base_file, 0x00, 64 * 3);

	if ((fp = fopen(file, "rb")) == NULL) {
		fprintf(stderr, "ERROR: open file %s\n", file);
		return (EXIT_ERROR);
	}

	fseek(fp, 0, SEEK_END);

	if (ftell(fp) < (64 * 3)) {
		fprintf(stderr, "ERROR: read file %s\n", file);
		fclose(fp);
		return (EXIT_ERROR);
	}

	rewind(fp);
	if (!(fread((BYTE *) palette_base_file, 64 * 3, 1, fp))) {
		;
	}

	fclose(fp);

	return (EXIT_OK);
}
