// Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)

#include "upd7756_interface.h"
#include "upd7756.h"
#define EXTERNC extern "C"

EXTERNC void upd7756_load_sample_rom(unsigned char *data, size_t size) {
	UPD7756::loadSampleROM(data, size);
}

#undef EXTERNC
