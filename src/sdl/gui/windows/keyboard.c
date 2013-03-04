/*
 * keyboard.c
 *
 *  Created on: 02/nov/2011
 *      Author: fhorse
 */

#include <ctype.h>
#include <string.h>
#include "win.h"

struct keyvalElement {
	DBWORD keyval;
	char keyval_name[20];
};

static const struct keyvalElement kvlist[] = {
	{ 0,            "NULL"      },
	{ VK_SPACE,     "Spacebar"  },
	{ VK_INSERT,    "Insert"    },
	{ VK_DELETE,    "Delete"    },
	{ VK_HOME,      "Home"      },
	{ VK_END,       "End"       },
	{ VK_PRIOR,	    "PgUp"      },
	{ VK_NEXT,	    "PgDown"    },
	{ VK_UP,	    "Up"        },
	{ VK_DOWN,	    "Down"      },
	{ VK_LEFT,	    "Left"      },
	{ VK_RIGHT,	    "Right"     },
	{ VK_NUMPAD0,   "NumPad0"   },
	{ VK_NUMPAD1,   "NumPad1"   },
	{ VK_NUMPAD2,   "NumPad2"   },
	{ VK_NUMPAD3,   "NumPad3"   },
	{ VK_NUMPAD4,   "NumPad4"   },
	{ VK_NUMPAD5,   "NumPad5"   },
	{ VK_NUMPAD6,   "NumPad6"   },
	{ VK_NUMPAD7,   "NumPad7"   },
	{ VK_NUMPAD8,   "NumPad8"   },
	{ VK_NUMPAD9,   "NumPad9"   },
	{ VK_MULTIPLY,  "Multiply"  },
	{ VK_ADD,       "Add"       },
	{ VK_SEPARATOR, "Separator" },
	{ VK_SUBTRACT,  "Subtract"  },
	{ VK_DECIMAL,   "Decimal"   },
	{ VK_DIVIDE,    "Divide"    },
	{ VK_BACK,      "Backspace" },
	{ VK_RETURN,    "Enter"     }
};

DBWORD keyvalFromName(const char *keyval_name) {
	DBWORD keyval;
	BYTE index;

	for (index = 0; index < LENGTH(kvlist); index++) {
		if (!strcmp(keyval_name, kvlist[index].keyval_name)) {
			return (kvlist[index].keyval);
		}
	}

	keyval = VkKeyScan(keyval_name[0]) & 0x000000FF;

	return (keyval);
}
char *keyvalToName(const DBWORD keyval) {
	DBWORD lScan = MapVirtualKey(keyval, 0);
	BYTE index;
	static char str[20];

	memset(str, 0, 20);

	for (index = 0; index < LENGTH(kvlist); index++) {
		if (keyval == kvlist[index].keyval) {
			strcpy(str, kvlist[index].keyval_name);
			return (str);
		}
	}

	GetKeyNameText((lScan << 16), str, 20);

	return (str);
}
