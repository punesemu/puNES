// Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)

#include "detach_barcode.h"
#include "gui.h"

_detach_barcode detach_barcode;

void init_detach_barcode(BYTE reset) {
	detach_barcode.enabled = TRUE;
	detach_barcode.pos = 0;
	detach_barcode.out = 0;
	detach_barcode.count = 0;
	detach_barcode.data[0] = 0xFF;
	if ((reset == CHANGE_ROM) || (reset == POWER_UP)) {
		gui_detach_barcode_change_rom();
	}
}
BYTE detach_barcode_save_mapper(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, detach_barcode.out);
	save_slot_ele(mode, slot, detach_barcode.pos);
	save_slot_ele(mode, slot, detach_barcode.count);
	save_slot_ele(mode, slot, detach_barcode.data);
	return (EXIT_OK);
}

// THX to guys of NinitendulatorNRS for this
int detach_barcode_bcode(const uTCHAR *rcode) {
	int prefix_parity_type[10][6] = {
		{ 0, 0, 0, 0, 0, 0 }, { 0, 0, 1, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1 }, { 0, 0, 1, 1, 1, 0 },
		{ 0, 1, 0, 0, 1, 1 }, { 0, 1, 1, 0, 0, 1 }, { 0, 1, 1, 1, 0, 0 }, { 0, 1, 0, 1, 0, 1 },
		{ 0, 1, 0, 1, 1, 0 }, { 0, 1, 1, 0, 1, 0 }
	};
	int data_left_odd[10][7] = {
		{ 0, 0, 0, 1, 1, 0, 1 }, { 0, 0, 1, 1, 0, 0, 1 }, { 0, 0, 1, 0, 0, 1, 1 }, { 0, 1, 1, 1, 1, 0, 1 },
		{ 0, 1, 0, 0, 0, 1, 1 }, { 0, 1, 1, 0, 0, 0, 1 }, { 0, 1, 0, 1, 1, 1, 1 }, { 0, 1, 1, 1, 0, 1, 1 },
		{ 0, 1, 1, 0, 1, 1, 1 }, { 0, 0, 0, 1, 0, 1, 1 }
	};
	int data_left_even[10][7] = {
		{ 0, 1, 0, 0, 1, 1, 1 }, { 0, 1, 1, 0, 0, 1, 1 }, { 0, 0, 1, 1, 0, 1, 1 }, { 0, 1, 0, 0, 0, 0, 1 },
		{ 0, 0, 1, 1, 1, 0, 1 }, { 0, 1, 1, 1, 0, 0, 1 }, { 0, 0, 0, 0, 1, 0, 1 }, { 0, 0, 1, 0, 0, 0, 1 },
		{ 0, 0, 0, 1, 0, 0, 1 }, { 0, 0, 1, 0, 1, 1, 1 }
	};
	int data_right[10][7] = {
		{ 1, 1, 1, 0, 0, 1, 0 }, { 1, 1, 0, 0, 1, 1, 0 }, { 1, 1, 0, 1, 1, 0, 0 }, { 1, 0, 0, 0, 0, 1, 0 },
		{ 1, 0, 1, 1, 1, 0, 0 }, { 1, 0, 0, 1, 1, 1, 0 }, { 1, 0, 1, 0, 0, 0, 0 }, { 1, 0, 0, 0, 1, 0, 0 },
		{ 1, 0, 0, 1, 0, 0, 0 }, { 1, 1, 1, 0, 1, 0, 0 }
	};
	uint8_t code[13 + 1] = { 0 };
	uint32_t tmp_p =0;
	int i = 0, j = 0;
	int len = 0;

	for (i = 0; i < 13; i++) {
		if (!rcode[i]) {
			break;
		}
		code[i] = rcode[i] - '0';
		if (code[i] > 9) {
			return (EXIT_ERROR);
		}
		len++;
	}
	if ((len != 13) && (len != 12) && (len != 8) && (len != 7)) {
		return (EXIT_ERROR);
	}

#define BS(x) detach_barcode.data[tmp_p++] = x;
	for (j = 0; j < 32; j++) {
		BS(0x00);
	}
	// Left guard bars
	BS(1); BS(0); BS(1);
	if ((len == 13) || (len == 12)) {
		uint32_t csum = 0;

		for (i = 0; i < 6; i++) {
			if (prefix_parity_type[code[0]][i]) {
				for (j = 0; j < 7; j++) {
					BS(data_left_even[code[i + 1]][j]);
				}
			} else {
				for (j = 0; j < 7; j++) {
					BS(data_left_odd[code[i + 1]][j]);
				}
			}
		}

		/* Center guard bars */
		BS(0); BS(1); BS(0); BS(1); BS(0);

		for (i = 7; i < 12; i++) {
			for (j = 0; j < 7; j++) {
				BS(data_right[code[i]][j]);
			}
		}
		csum =0;
		for (i = 0; i < 12; i++) {
			csum += code[i] * ((i & 0x01) ? 3 : 1);
		}
		csum = (10 - (csum % 10)) % 10;
		for (j = 0; j < 7; j++) {
			BS(data_right[csum][j]);
		}
	} else if ((len == 8) || (len == 7)) {
		uint32_t csum =0;

		for (i = 0; i < 7; i++) {
			csum += (i & 1) ? code[i] : (code[i] * 3);
		}
		csum = (10 - (csum % 10)) % 10;
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 7; j++) {
				BS(data_left_odd[code[i]][j]);
			}
		}
		// Center guard bars
		BS(0); BS(1); BS(0); BS(1); BS(0);

		for (i = 4; i < 7; i++) {
			for (j = 0; j < 7; j++) {
				BS(data_right[code[i]][j]);
			}
		}

		for (j = 0; j < 7; j++) {
			BS(data_right[csum][j]);
		}
	}
	/* Right guard bars */
	BS(1); BS(0); BS(1);
	for (j = 0; j < 32; j++) {
		BS(0x00);
	}
	BS(0xFF);
#undef BS
	detach_barcode.pos = 0;
	detach_barcode.out = 0x08;
	detach_barcode.count = 0;
	return (EXIT_OK);
}
