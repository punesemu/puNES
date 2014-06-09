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

void palette_save_on_file(char *file) {
	const char pext[] = ".pal";
	char name[LENGTH_FILE_NAME_LONG], *ext;
	FILE *fp;

	memset((BYTE *) name, 0x00, sizeof(name));
	strncpy(name, file, sizeof(name) - 100);

	if (((ext = strrchr(name, '.')) == NULL) || (strcasecmp(ext, pext) != 0)) {
		strcat(name, pext);
	}

	if ((fp = fopen(name, "wb")) == NULL) {
		fprintf(stderr, "ERROR: Impossible save palette file %s", name);
		return;
	}

	fwrite((BYTE *) palette_RGB, 64 * 3, 1, fp);

	fclose(fp);
}
BYTE palette_load_from_file(char *file) {
	FILE *fp;

	memset((BYTE *) palette_base_file, 0x00, 64 * 3);

	printf( "000 %s\n", file);

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
    fread((BYTE *) palette_base_file, 64 * 3, 1, fp);

	fclose(fp);

	return (EXIT_OK);
}
