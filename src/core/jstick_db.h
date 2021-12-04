/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#ifndef _JSTICK_DB_H_
#define _JSTICK_DB_H_

#include "common.h"
#include "jstick_ids.h"
#include "os_jstick.h"

enum _js_db_misc {
	JS_DB_NO_VENDOR_ID = 0xFFFFFFFF,
	JS_DB_NO_PRODUCT_ID = JS_DB_NO_VENDOR_ID,
};

#define JS_BTN_DEF_BIT 0x20000
#define JS_BTN_DEF(a) a | JS_BTN_DEF_BIT
#define JS_IS_BTN_DEF(a) a & JS_BTN_DEF_BIT
#define JS_ABS_DEF_BIT(b) ((b & 0x01) << 16)
#define JS_ABS_DEF(a, b) a | JS_ABS_DEF_BIT(b)
#define JS_BTNABS_UNDEF(a) a & 0xFFFF

typedef struct _js_db_device_icon_desc {
	DBWORD offset;
	const uTCHAR *icon;
	const uTCHAR *desc;
} _js_db_device_icon_desc;
typedef struct _js_db_device {
	enum _js_gamepad_type type;
	BYTE is_default;
	DBWORD vendor_id;
	DBWORD product_id;
	DBWORD std_pad_default[MAX_STD_PAD_BUTTONS];
	_js_db_device_icon_desc btn[16];
	_js_db_device_icon_desc axs[16];
} _js_db_device;

static const _js_db_device js_db_devices[] = {
	// Default generico e Xbox360
	{
		JS_SC_MS_XBOX_360_GAMEPAD,
		TRUE,
		JS_DB_NO_VENDOR_ID,
		JS_DB_NO_PRODUCT_ID,
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_B),
			/* BUT_B  */ JS_BTN_DEF(BTN_A),
			/* SELECT */ JS_BTN_DEF(BTN_SELECT),
			/* START  */ JS_BTN_DEF(BTN_START),
			/* UP     */ JS_ABS_DEF(ABS_Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_Y),
			/* TRB_B  */ JS_BTN_DEF(BTN_X)
		},
		{
			{ BTN_A,          uL(":/icon/icons/Xbox_360_A.svg"),             uL("A")         },
			{ BTN_B,          uL(":/icon/icons/Xbox_360_B.svg"),             uL("B")         },
			{ BTN_X,          uL(":/icon/icons/Xbox_360_X.svg"),             uL("X")         },
			{ BTN_Y,          uL(":/icon/icons/Xbox_360_Y.svg"),             uL("Y")         },
			{ BTN_TL,         uL(":/icon/icons/Xbox_Left_Bumper.svg"),       uL("LB")        },
			{ BTN_TR,         uL(":/icon/icons/Xbox_Right_Bumper.svg"),      uL("RB")        },
			{ BTN_SELECT,     uL(":/icon/icons/Xbox_360_Back.svg"),          uL("BACK")      },
			{ BTN_START,      uL(":/icon/icons/Xbox_360_Start.svg"),         uL("START")     },
#if defined (__OpenBSD__) || defined (__FreeBSD__)
			{ BTN_THUMBR,     uL(":/icon/icons/Xbox_360_Home.svg"),          uL("HOME")      },
			{ BTN_MODE,       uL(":/icon/icons/Xbox_Left_Stick_Click.png"),  uL("LS CLICK")  },
			{ BTN_THUMBL,     uL(":/icon/icons/Xbox_Right_Stick_Click.png"), uL("RS CLICK")  },
#else
			{ BTN_MODE,       uL(":/icon/icons/Xbox_360_Home.svg"),          uL("HOME")      },
			{ BTN_THUMBL,     uL(":/icon/icons/Xbox_Left_Stick_Click.png"),  uL("LS CLICK")  },
			{ BTN_THUMBR,     uL(":/icon/icons/Xbox_Right_Stick_Click.png"), uL("RS CLICK")  },
#endif
			{ BTN_DPAD_LEFT,  uL(":/icon/icons/Xbox_360_Dpad_Left.png"),     uL("DPAD LEFT") },
			{ BTN_DPAD_RIGHT, uL(":/icon/icons/Xbox_360_Dpad_Right.png"),    uL("DPAD RIGHT")},
			{ BTN_DPAD_UP,    uL(":/icon/icons/Xbox_360_Dpad_Up.png"),       uL("DPAD UP")   },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/Xbox_360_Dpad_Down.png"),     uL("DPAD DOWN") }
		},
		{
			{ ABS_X,          uL(":/icon/icons/Xbox_Left_Stick_Left.svg"),   uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/Xbox_Left_Stick_Right.svg"),  uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/Xbox_Left_Stick_Up.svg"),     uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/Xbox_Left_Stick_Down.svg"),   uL("LS DOWN")   },
			{ ABS_Z,          uL(":/icon/icons/Xbox_Left_Trigger.svg"),      uL("LT")        },
			{ ABS_Z,          uL(":/icon/icons/Xbox_Left_Trigger.svg"),      uL("LT")        },
			{ ABS_RX,         uL(":/icon/icons/Xbox_Right_Stick_Left.svg"),  uL("RS LEFT")   },
			{ ABS_RX,         uL(":/icon/icons/Xbox_Right_Stick_Right.svg"), uL("RS RIGHT")  },
			{ ABS_RY,         uL(":/icon/icons/Xbox_Right_Stick_Up.svg"),    uL("RS UP")     },
			{ ABS_RY,         uL(":/icon/icons/Xbox_Right_Stick_Down.svg"),  uL("RS DOWN")   },
			{ ABS_RZ,         uL(":/icon/icons/Xbox_Right_Trigger.svg"),     uL("RT")        },
			{ ABS_RZ,         uL(":/icon/icons/Xbox_Right_Trigger.svg"),     uL("RT")        },
			{ ABS_HAT0X,      uL(":/icon/icons/Xbox_360_Dpad_Left.png"),     uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/Xbox_360_Dpad_Right.png"),    uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/Xbox_360_Dpad_Up.png"),       uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/Xbox_360_Dpad_Down.png"),     uL("DPAD DOWN") }
		}
	},
	// Xbox One
	{
		JS_SC_MS_XBOX_ONE_GAMEPAD,
		TRUE,
		JS_DB_NO_VENDOR_ID,
		JS_DB_NO_PRODUCT_ID,
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_B),
			/* BUT_B  */ JS_BTN_DEF(BTN_A),
			/* SELECT */ JS_BTN_DEF(BTN_SELECT),
			/* START  */ JS_BTN_DEF(BTN_START),
			/* UP     */ JS_ABS_DEF(ABS_Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_Y),
			/* TRB_B  */ JS_BTN_DEF(BTN_X)
		},
		{
			{ BTN_A,          uL(":/icon/icons/Xbox_One_A.svg"),               uL("A")         },
			{ BTN_B,          uL(":/icon/icons/Xbox_One_B.svg"),               uL("B")         },
			{ BTN_X,          uL(":/icon/icons/Xbox_One_X.svg"),               uL("X")         },
			{ BTN_Y,          uL(":/icon/icons/Xbox_One_Y.svg"),               uL("Y")         },
			{ BTN_TL,         uL(":/icon/icons/Xbox_Left_Bumper.svg"),         uL("LB")        },
			{ BTN_TR,         uL(":/icon/icons/Xbox_Right_Bumper.svg"),        uL("RB")        },
			{ BTN_SELECT,     uL(":/icon/icons/Xbox_One_View.svg"),            uL("VIEW")      },
			{ BTN_START,      uL(":/icon/icons/Xbox_One_Menu.svg"),            uL("MODE")      },
#if defined (__OpenBSD__) || defined (__FreeBSD__)
			{ BTN_THUMBR,     uL(":/icon/icons/Xbox_360_Home.svg"),            uL("HOME")      },
			{ BTN_MODE,       uL(":/icon/icons/Xbox_Left_Stick_Click.png"),    uL("LS CLICK")  },
			{ BTN_THUMBL,     uL(":/icon/icons/Xbox_Right_Stick_Click.png"),   uL("RS CLICK")  },
#else
			{ BTN_MODE,       uL(":/icon/icons/Xbox_360_Home.svg"),            uL("HOME")      },
			{ BTN_THUMBL,     uL(":/icon/icons/Xbox_Left_Stick_Click.png"),    uL("LS CLICK")  },
			{ BTN_THUMBR,     uL(":/icon/icons/Xbox_Right_Stick_Click.png"),   uL("RS CLICK")  },
#endif
			{ BTN_DPAD_LEFT,  uL(":/icon/icons/PlayStation_4_Dpad_Left.svg"),  uL("DPAD LEFT") },
			{ BTN_DPAD_RIGHT, uL(":/icon/icons/PlayStation_4_Dpad_Right.svg"), uL("DPAD RIGHT")},
			{ BTN_DPAD_UP,    uL(":/icon/icons/PlayStation_4_Dpad_Up.svg"),    uL("DPAD UP")   },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/PlayStation_4_Dpad_Down.svg"),  uL("DPAD DOWN") }
		},
		{
			{ ABS_X,          uL(":/icon/icons/Xbox_Left_Stick_Left.svg"),     uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/Xbox_Left_Stick_Right.svg"),    uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/Xbox_Left_Stick_Up.svg"),       uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/Xbox_Left_Stick_Down.svg"),     uL("LS DOWN")   },
			{ ABS_Z,          uL(":/icon/icons/Xbox_Left_Trigger.svg"),        uL("LT")        },
			{ ABS_Z,          uL(":/icon/icons/Xbox_Left_Trigger.svg"),        uL("LT")        },
			{ ABS_RX,         uL(":/icon/icons/Xbox_Right_Stick_Left.svg"),    uL("RS LEFT")   },
			{ ABS_RX,         uL(":/icon/icons/Xbox_Right_Stick_Right.svg"),   uL("RS RIGHT")  },
			{ ABS_RY,         uL(":/icon/icons/Xbox_Right_Stick_Up.svg"),      uL("RS UP")     },
			{ ABS_RY,         uL(":/icon/icons/Xbox_Right_Stick_Down.svg"),    uL("RS DOWN")   },
			{ ABS_RZ,         uL(":/icon/icons/Xbox_Right_Trigger.svg"),       uL("RT")        },
			{ ABS_RZ,         uL(":/icon/icons/Xbox_Right_Trigger.svg"),       uL("RT")        },
			{ ABS_HAT0X,      uL(":/icon/icons/PlayStation_4_Dpad_Left.svg"),  uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/PlayStation_4_Dpad_Right.svg"), uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/PlayStation_4_Dpad_Up.svg"),    uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/PlayStation_4_Dpad_Down.svg"),  uL("DPAD DOWN") }
		}
	},
	// Playstation 4
	{
		JS_SC_SONY_PS4_GAMEPAD,
		TRUE,
		JS_DB_NO_VENDOR_ID,
		JS_DB_NO_PRODUCT_ID,
#if defined (__linux__)
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_B),
			/* BUT_B  */ JS_BTN_DEF(BTN_A),
			/* SELECT */ JS_BTN_DEF(BTN_SELECT),
			/* START  */ JS_BTN_DEF(BTN_START),
			/* UP     */ JS_ABS_DEF(ABS_Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_Y),
			/* TRB_B  */ JS_BTN_DEF(BTN_X)
		},
		{
			{ BTN_A,          uL(":/icon/icons/PlayStation_X.svg"),            uL("CROSS")     },
			{ BTN_B,          uL(":/icon/icons/PlayStation_C.svg"),            uL("CIRCLE")    },
			{ BTN_X,          uL(":/icon/icons/PlayStation_S.svg"),            uL("SQUARE")    },
			{ BTN_Y,          uL(":/icon/icons/PlayStation_T.svg"),            uL("TRIANGLE")  },
			{ BTN_TL,         uL(":/icon/icons/PlayStation_4_L1.svg"),         uL("L1")        },
			{ BTN_TR,         uL(":/icon/icons/PlayStation_4_R1.svg"),         uL("R1")        },
			{ BTN_TL2,        uL(":/icon/icons/PlayStation_4_L2.svg"),         uL("L2")        },
			{ BTN_TR2,        uL(":/icon/icons/PlayStation_4_R2.svg"),         uL("R2")        },
			{ BTN_SELECT,     uL(":/icon/icons/PlayStation_4_Share.svg"),      uL("SHARE")     },
			{ BTN_START,      uL(":/icon/icons/PlayStation_4_Options.svg"),    uL("OPTIONS")   },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/PlayStation_Home.svg"),         uL("HOME")      },
			{ BTN_THUMBL,     uL(":/icon/icons/Xbox_Left_Stick_Click.png"),    uL("LS CLICK")  },
			{ BTN_THUMBR,     uL(":/icon/icons/Xbox_Right_Stick_Click.png"),   uL("RS CLICK")  }
		},
		{
			{ ABS_X,          uL(":/icon/icons/Xbox_Left_Stick_Left.svg"),     uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/Xbox_Left_Stick_Right.svg"),    uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/Xbox_Left_Stick_Up.svg"),       uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/Xbox_Left_Stick_Down.svg"),     uL("LS DOWN")   },
			{ ABS_Z,          uL(":/icon/icons/PlayStation_4_L2.svg"),         uL("L2")        },
			{ ABS_Z,          uL(":/icon/icons/PlayStation_4_L2.svg"),         uL("L2")        },
			{ ABS_RX,         uL(":/icon/icons/Xbox_Right_Stick_Left.svg"),    uL("RS LEFT")   },
			{ ABS_RX,         uL(":/icon/icons/Xbox_Right_Stick_Right.svg"),   uL("RS RIGHT")  },
			{ ABS_RY,         uL(":/icon/icons/Xbox_Right_Stick_Up.svg"),      uL("RS UP")     },
			{ ABS_RY,         uL(":/icon/icons/Xbox_Right_Stick_Down.svg"),    uL("RS DOWN")   },
			{ ABS_RZ,         uL(":/icon/icons/PlayStation_4_R2.svg"),         uL("R2")        },
			{ ABS_RZ,         uL(":/icon/icons/PlayStation_4_R2.svg"),         uL("R2")        },
			{ ABS_HAT0X,      uL(":/icon/icons/PlayStation_4_Dpad_Left.svg"),  uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/PlayStation_4_Dpad_Right.svg"), uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/PlayStation_4_Dpad_Up.svg"),    uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/PlayStation_4_Dpad_Down.svg"),  uL("DPAD DOWN") }
		}
#else
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_X),
			/* BUT_B  */ JS_BTN_DEF(BTN_B),
			/* SELECT */ JS_BTN_DEF(BTN_MODE),
			/* START  */ JS_BTN_DEF(BTN_THUMBL),
			/* UP     */ JS_ABS_DEF(ABS_Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_Y),
			/* TRB_B  */ JS_BTN_DEF(BTN_A)
		},
		{
			{ BTN_B,          uL(":/icon/icons/PlayStation_X.svg"),            uL("CROSS")     },
			{ BTN_X,          uL(":/icon/icons/PlayStation_C.svg"),            uL("CIRCLE")    },
			{ BTN_A,          uL(":/icon/icons/PlayStation_S.svg"),            uL("SQUARE")    },
			{ BTN_Y,          uL(":/icon/icons/PlayStation_T.svg"),            uL("TRIANGLE")  },
			{ BTN_TL,         uL(":/icon/icons/PlayStation_4_L1.svg"),         uL("L1")        },
			{ BTN_TR,         uL(":/icon/icons/PlayStation_4_R1.svg"),         uL("R1")        },
			{ BTN_SELECT,     uL(":/icon/icons/PlayStation_4_L2.svg"),         uL("L2")        },
			{ BTN_START,      uL(":/icon/icons/PlayStation_4_R2.svg"),         uL("R2")        },
			{ BTN_MODE,       uL(":/icon/icons/PlayStation_4_Share.svg"),      uL("SHARE")     },
			{ BTN_THUMBL,     uL(":/icon/icons/PlayStation_4_Options.svg"),    uL("OPTIONS")   },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/PlayStation_Home.svg"),         uL("HOME")      },
			{ BTN_THUMBR,     uL(":/icon/icons/Xbox_Left_Stick_Click.png"),    uL("LS CLICK")  },
			{ BTN_DPAD_UP,    uL(":/icon/icons/Xbox_Right_Stick_Click.png"),   uL("RS CLICK")  },
			{ BTN_DPAD_LEFT,  uL(":/icon/icons/PlayStation_4_Tpad_Click.svg"), uL("TPAD CLICK")}
		},
		{
			{ ABS_X,          uL(":/icon/icons/Xbox_Left_Stick_Left.svg"),     uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/Xbox_Left_Stick_Right.svg"),    uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/Xbox_Left_Stick_Up.svg"),       uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/Xbox_Left_Stick_Down.svg"),     uL("LS DOWN")   },
			{ ABS_RX,         uL(":/icon/icons/PlayStation_4_L2.svg"),         uL("L2")        },
			{ ABS_RX,         uL(":/icon/icons/PlayStation_4_L2.svg"),         uL("L2")        },
			{ ABS_Z,          uL(":/icon/icons/Xbox_Right_Stick_Left.svg"),    uL("RS LEFT")   },
			{ ABS_Z,          uL(":/icon/icons/Xbox_Right_Stick_Right.svg"),   uL("RS RIGHT")  },
			{ ABS_RZ,         uL(":/icon/icons/Xbox_Right_Stick_Up.svg"),      uL("RS UP")     },
			{ ABS_RZ,         uL(":/icon/icons/Xbox_Right_Stick_Down.svg"),    uL("RS DOWN")   },
			{ ABS_RY,         uL(":/icon/icons/PlayStation_4_R2.svg"),         uL("R2")        },
			{ ABS_RY,         uL(":/icon/icons/PlayStation_4_R2.svg"),         uL("R2")        },
			{ ABS_HAT0X,      uL(":/icon/icons/PlayStation_4_Dpad_Left.svg"),  uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/PlayStation_4_Dpad_Right.svg"), uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/PlayStation_4_Dpad_Up.svg"),    uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/PlayStation_4_Dpad_Down.svg"),  uL("DPAD DOWN") }
		}
#endif
	},
	// Hama Game USB Joystick
	{
		JS_SC_UNKNOWN,
		FALSE,
		0xF766,
		0x0001,
#if defined (__linux__)
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_THUMB2),
			/* BUT_B  */ JS_BTN_DEF(BTN_THUMB),
			/* SELECT */ JS_BTN_DEF(BTN_BASE3),
			/* START  */ JS_BTN_DEF(BTN_BASE4),
			/* UP     */ JS_ABS_DEF(ABS_HAT0Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_HAT0Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_HAT0X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_HAT0X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_TOP),
			/* TRB_B  */ JS_BTN_DEF(BTN_TRIGGER)
		},
		{},
		{}
#else
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_X),
			/* BUT_B  */ JS_BTN_DEF(BTN_B),
			/* SELECT */ JS_BTN_DEF(BTN_MODE),
			/* START  */ JS_BTN_DEF(BTN_THUMBL),
			/* UP     */ JS_ABS_DEF(ABS_HAT0Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_HAT0Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_HAT0X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_HAT0X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_Y),
			/* TRB_B  */ JS_BTN_DEF(BTN_A)
		},
		{},
		{}
#endif
	}
};

#endif /* _JSTICK_DB_H_ */
