/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
	JS_DB_ICON_DESC_ELEMENTS = 21,
	JS_DB_NO_VENDOR_ID = 0xFFFFFFFF,
	JS_DB_NO_PRODUCT_ID = JS_DB_NO_VENDOR_ID
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
	_js_db_device_icon_desc btn[JS_DB_ICON_DESC_ELEMENTS];
	_js_db_device_icon_desc axs[JS_DB_ICON_DESC_ELEMENTS];
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
			{ BTN_A,          uL(":/icon/icons/gamepad_xbox_360_a.svgz"),             uL("A")         },
			{ BTN_B,          uL(":/icon/icons/gamepad_xbox_360_b.svgz"),             uL("B")         },
			{ BTN_X,          uL(":/icon/icons/gamepad_xbox_360_x.svgz"),             uL("X")         },
			{ BTN_Y,          uL(":/icon/icons/gamepad_xbox_360_y.svgz"),             uL("Y")         },
			{ BTN_TL,         uL(":/icon/icons/gamepad_xbox_lb.svgz"),                uL("LB")        },
			{ BTN_TR,         uL(":/icon/icons/gamepad_xbox_rb.svgz"),                uL("RB")        },
			{ BTN_SELECT,     uL(":/icon/icons/gamepad_xbox_360_back.svgz"),          uL("BACK")      },
			{ BTN_START,      uL(":/icon/icons/gamepad_xbox_360_start.svgz"),         uL("START")     },
#if defined (__OpenBSD__) || defined (__FreeBSD__)
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_xbox_360_home.svgz"),          uL("HOME")      },
			{ BTN_MODE,       uL(":/icon/icons/gamepad_xbox_left_stick_click.svgz"),  uL("LS CLICK")  },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_xbox_right_stick_click.svgz"), uL("RS CLICK")  },
#else
			{ BTN_MODE,       uL(":/icon/icons/gamepad_xbox_360_home.svgz"),          uL("HOME")      },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_xbox_left_stick_click.svgz"),  uL("LS CLICK")  },
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_xbox_right_stick_click.svgz"), uL("RS CLICK")  },
#endif
			{ BTN_DPAD_LEFT,  uL(":/icon/icons/gamepad_xbox_360_dpad_left.svgz"),     uL("DPAD LEFT") },
			{ BTN_DPAD_RIGHT, uL(":/icon/icons/gamepad_xbox_360_dpad_right.svgz"),    uL("DPAD RIGHT")},
			{ BTN_DPAD_UP,    uL(":/icon/icons/gamepad_xbox_360_dpad_up.svgz"),       uL("DPAD UP")   },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/gamepad_xbox_360_dpad_down.svgz"),     uL("DPAD DOWN") }
		},
		{
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_left.svgz"),   uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_right.svgz"),  uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_up.svgz"),     uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_down.svgz"),   uL("LS DOWN")   },
			{ ABS_Z,          uL(":/icon/icons/gamepad_xbox_lt.svgz"),                uL("LT")        },
			{ ABS_Z,          uL(":/icon/icons/gamepad_xbox_lt.svgz"),                uL("LT")        },
			{ ABS_RX,         uL(":/icon/icons/gamepad_xbox_right_stick_left.svgz"),  uL("RS LEFT")   },
			{ ABS_RX,         uL(":/icon/icons/gamepad_xbox_right_stick_right.svgz"), uL("RS RIGHT")  },
			{ ABS_RY,         uL(":/icon/icons/gamepad_xbox_right_stick_up.svgz"),    uL("RS UP")     },
			{ ABS_RY,         uL(":/icon/icons/gamepad_xbox_right_stick_down.svgz"),  uL("RS DOWN")   },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_xbox_rt.svgz"),                uL("RT")        },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_xbox_rt.svgz"),                uL("RT")        },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_xbox_360_dpad_left.svgz"),     uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_xbox_360_dpad_right.svgz"),    uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_xbox_360_dpad_up.svgz"),       uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_xbox_360_dpad_down.svgz"),     uL("DPAD DOWN") }
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
			{ BTN_A,          uL(":/icon/icons/gamepad_xbox_one_a.svgz"),               uL("A")         },
			{ BTN_B,          uL(":/icon/icons/gamepad_xbox_one_b.svgz"),               uL("B")         },
			{ BTN_X,          uL(":/icon/icons/gamepad_xbox_one_x.svgz"),               uL("X")         },
			{ BTN_Y,          uL(":/icon/icons/gamepad_xbox_one_y.svgz"),               uL("Y")         },
			{ BTN_TL,         uL(":/icon/icons/gamepad_xbox_lb.svgz"),                  uL("LB")        },
			{ BTN_TR,         uL(":/icon/icons/gamepad_xbox_rb.svgz"),                  uL("RB")        },
			{ BTN_SELECT,     uL(":/icon/icons/gamepad_xbox_one_view.svgz"),            uL("VIEW")      },
			{ BTN_START,      uL(":/icon/icons/gamepad_xbox_one_menu.svgz"),            uL("MODE")      },
#if defined (__OpenBSD__) || defined (__FreeBSD__)
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_xbox_360_home.svgz"),            uL("HOME")      },
			{ BTN_MODE,       uL(":/icon/icons/gamepad_xbox_left_stick_click.svgz"),    uL("LS CLICK")  },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_xbox_right_stick_click.svgz"),   uL("RS CLICK")  },
#else
			{ BTN_MODE,       uL(":/icon/icons/gamepad_xbox_360_home.svgz"),            uL("HOME")      },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_xbox_left_stick_click.svgz"),    uL("LS CLICK")  },
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_xbox_right_stick_click.svgz"),   uL("RS CLICK")  },
#endif
			{ BTN_DPAD_LEFT,  uL(":/icon/icons/gamepad_xbox_one_dpad_left.svgz"),       uL("DPAD LEFT") },
			{ BTN_DPAD_RIGHT, uL(":/icon/icons/gamepad_xbox_one_dpad_right.svgz"),      uL("DPAD RIGHT")},
			{ BTN_DPAD_UP,    uL(":/icon/icons/gamepad_xbox_one_dpad_up.svgz"),         uL("DPAD UP")   },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/gamepad_xbox_one_dpad_down.svgz"),       uL("DPAD DOWN") }
		},
		{
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_left.svgz"),     uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_right.svgz"),    uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_up.svgz"),       uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_down.svgz"),     uL("LS DOWN")   },
			{ ABS_Z,          uL(":/icon/icons/gamepad_xbox_lt.svgz"),                  uL("LT")        },
			{ ABS_Z,          uL(":/icon/icons/gamepad_xbox_lt.svgz"),                  uL("LT")        },
			{ ABS_RX,         uL(":/icon/icons/gamepad_xbox_right_stick_left.svgz"),    uL("RS LEFT")   },
			{ ABS_RX,         uL(":/icon/icons/gamepad_xbox_right_stick_right.svgz"),   uL("RS RIGHT")  },
			{ ABS_RY,         uL(":/icon/icons/gamepad_xbox_right_stick_up.svgz"),      uL("RS UP")     },
			{ ABS_RY,         uL(":/icon/icons/gamepad_xbox_right_stick_down.svgz"),    uL("RS DOWN")   },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_xbox_rt.svgz"),                  uL("RT")        },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_xbox_rt.svgz"),                  uL("RT")        },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_xbox_one_dpad_left.svgz"),       uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_xbox_one_dpad_right.svgz"),      uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_xbox_one_dpad_up.svgz"),         uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_xbox_one_dpad_down.svgz"),       uL("DPAD DOWN") }
		}
	},
	// Playstation 3
	{
		JS_SC_SONY_PS3_GAMEPAD,
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
			/* TRB_A  */ JS_BTN_DEF(BTN_X),
			/* TRB_B  */ JS_BTN_DEF(BTN_Y)
		},
		{
			{ BTN_A,          uL(":/icon/icons/gamepad_playstation_x.svgz"),            uL("CROSS")     },
			{ BTN_B,          uL(":/icon/icons/gamepad_playstation_c.svgz"),            uL("CIRCLE")    },
			{ BTN_Y,          uL(":/icon/icons/gamepad_playstation_s.svgz"),            uL("SQUARE")    },
			{ BTN_X,          uL(":/icon/icons/gamepad_playstation_t.svgz"),            uL("TRIANGLE")  },
			{ BTN_TL,         uL(":/icon/icons/gamepad_playstation_3_l1.svgz"),         uL("L1")        },
			{ BTN_TR,         uL(":/icon/icons/gamepad_playstation_3_r1.svgz"),         uL("R1")        },
			{ BTN_TL2,        uL(":/icon/icons/gamepad_playstation_3_l2.svgz"),         uL("L2")        },
			{ BTN_TR2,        uL(":/icon/icons/gamepad_playstation_3_r2.svgz"),         uL("R2")        },
			{ BTN_SELECT,     uL(":/icon/icons/gamepad_playstation_3_select.svgz"),     uL("SELECT")    },
			{ BTN_START,      uL(":/icon/icons/gamepad_playstation_3_start.svgz"),      uL("START")     },
			{ BTN_MODE,       uL(":/icon/icons/gamepad_playstation_home.svgz"),         uL("HOME")      },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_xbox_left_stick_click.svgz"),    uL("LS CLICK")  },
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_xbox_right_stick_click.svgz"),   uL("RS CLICK")  },
			{ BTN_DPAD_UP,    uL(":/icon/icons/gamepad_playstation_3_dpad_up.svgz"),    uL("DPAD UP")   },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/gamepad_playstation_3_dpad_down.svgz"),  uL("DPAD DOWN") },
			{ BTN_DPAD_LEFT,  uL(":/icon/icons/gamepad_playstation_3_dpad_left.svgz"),  uL("DPAD LEFT") },
			{ BTN_DPAD_RIGHT, uL(":/icon/icons/gamepad_playstation_3_dpad_right.svgz"), uL("DPAD RIGHT")}
		},
		{
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_left.svgz"),     uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_right.svgz"),    uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_up.svgz"),       uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_down.svgz"),     uL("LS DOWN")   },
			{ ABS_Z,          uL(":/icon/icons/gamepad_playstation_3_l2.svgz"),         uL("L2")        },
			{ ABS_Z,          uL(":/icon/icons/gamepad_playstation_3_l2.svgz"),         uL("L2")        },
			{ ABS_RX,         uL(":/icon/icons/gamepad_xbox_right_stick_left.svgz"),    uL("RS LEFT")   },
			{ ABS_RX,         uL(":/icon/icons/gamepad_xbox_right_stick_right.svgz"),   uL("RS RIGHT")  },
			{ ABS_RY,         uL(":/icon/icons/gamepad_xbox_right_stick_up.svgz"),      uL("RS UP")     },
			{ ABS_RY,         uL(":/icon/icons/gamepad_xbox_right_stick_down.svgz"),    uL("RS DOWN")   },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_playstation_3_r2.svgz"),         uL("R2")        },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_playstation_3_r2.svgz"),         uL("R2")        },
		}
#else
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_B),
			/* BUT_B  */ JS_BTN_DEF(BTN_X),
			/* SELECT */ JS_BTN_DEF(BTN_MODE),
			/* START  */ JS_BTN_DEF(BTN_THUMBL),
			/* UP     */ JS_ABS_DEF(ABS_Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_A),
			/* TRB_B  */ JS_BTN_DEF(BTN_Y)
		},
		{
			{ BTN_X,          uL(":/icon/icons/gamepad_playstation_x.svgz"),            uL("CROSS")     },
			{ BTN_B,          uL(":/icon/icons/gamepad_playstation_c.svgz"),            uL("CIRCLE")    },
			{ BTN_Y,          uL(":/icon/icons/gamepad_playstation_s.svgz"),            uL("SQUARE")    },
			{ BTN_A,          uL(":/icon/icons/gamepad_playstation_t.svgz"),            uL("TRIANGLE")  },
			{ BTN_SELECT,     uL(":/icon/icons/gamepad_playstation_3_l1.svgz"),         uL("L1")        },
			{ BTN_START,      uL(":/icon/icons/gamepad_playstation_3_r1.svgz"),         uL("R1")        },
			{ BTN_TL,         uL(":/icon/icons/gamepad_playstation_3_l2.svgz"),         uL("L2")        },
			{ BTN_TR,         uL(":/icon/icons/gamepad_playstation_3_r2.svgz"),         uL("R2")        },
			{ BTN_MODE,       uL(":/icon/icons/gamepad_playstation_3_select.svgz"),     uL("SELECT")    },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_playstation_3_start.svgz"),      uL("START")     },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/gamepad_playstation_home.svgz"),         uL("HOME")      },
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_xbox_left_stick_click.svgz"),    uL("LS CLICK")  },
			{ BTN_DPAD_UP,    uL(":/icon/icons/gamepad_xbox_right_stick_click.svgz"),   uL("RS CLICK")  }
		},
		{
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_left.svgz"),     uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_right.svgz"),    uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_up.svgz"),       uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_down.svgz"),     uL("LS DOWN")   },
			{ ABS_RX,         uL(":/icon/icons/gamepad_playstation_3_l2.svgz"),         uL("L2")        },
			{ ABS_RX,         uL(":/icon/icons/gamepad_playstation_3_l2.svgz"),         uL("L2")        },
			{ ABS_Z,          uL(":/icon/icons/gamepad_xbox_right_stick_left.svgz"),    uL("RS LEFT")   },
			{ ABS_Z,          uL(":/icon/icons/gamepad_xbox_right_stick_right.svgz"),   uL("RS RIGHT")  },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_xbox_right_stick_up.svgz"),      uL("RS UP")     },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_xbox_right_stick_down.svgz"),    uL("RS DOWN")   },
			{ ABS_RY,         uL(":/icon/icons/gamepad_playstation_3_r2.svgz"),         uL("R2")        },
			{ ABS_RY,         uL(":/icon/icons/gamepad_playstation_3_r2.svgz"),         uL("R2")        },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_playstation_3_dpad_left.svgz"),  uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_playstation_3_dpad_right.svgz"), uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_playstation_3_dpad_up.svgz"),    uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_playstation_3_dpad_down.svgz"),  uL("DPAD DOWN") }
		}
#endif
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
			{ BTN_A,          uL(":/icon/icons/gamepad_playstation_x.svgz"),            uL("CROSS")     },
			{ BTN_B,          uL(":/icon/icons/gamepad_playstation_c.svgz"),            uL("CIRCLE")    },
			{ BTN_X,          uL(":/icon/icons/gamepad_playstation_s.svgz"),            uL("SQUARE")    },
			{ BTN_Y,          uL(":/icon/icons/gamepad_playstation_t.svgz"),            uL("TRIANGLE")  },
			{ BTN_TL,         uL(":/icon/icons/gamepad_playstation_4_l1.svgz"),         uL("L1")        },
			{ BTN_TR,         uL(":/icon/icons/gamepad_playstation_4_r1.svgz"),         uL("R1")        },
			{ BTN_TL2,        uL(":/icon/icons/gamepad_playstation_4_l2.svgz"),         uL("L2")        },
			{ BTN_TR2,        uL(":/icon/icons/gamepad_playstation_4_r2.svgz"),         uL("R2")        },
			{ BTN_SELECT,     uL(":/icon/icons/gamepad_playstation_4_share.svgz"),      uL("SHARE")     },
			{ BTN_START,      uL(":/icon/icons/gamepad_playstation_4_options.svgz"),    uL("OPTIONS")   },
			{ BTN_MODE,       uL(":/icon/icons/gamepad_playstation_home.svgz"),         uL("HOME")      },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_xbox_left_stick_click.svgz"),    uL("LS CLICK")  },
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_xbox_right_stick_click.svgz"),   uL("RS CLICK")  }
		},
		{
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_left.svgz"),     uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_right.svgz"),    uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_up.svgz"),       uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_down.svgz"),     uL("LS DOWN")   },
			{ ABS_Z,          uL(":/icon/icons/gamepad_playstation_4_l2.svgz"),         uL("L2")        },
			{ ABS_Z,          uL(":/icon/icons/gamepad_playstation_4_l2.svgz"),         uL("L2")        },
			{ ABS_RX,         uL(":/icon/icons/gamepad_xbox_right_stick_left.svgz"),    uL("RS LEFT")   },
			{ ABS_RX,         uL(":/icon/icons/gamepad_xbox_right_stick_right.svgz"),   uL("RS RIGHT")  },
			{ ABS_RY,         uL(":/icon/icons/gamepad_xbox_right_stick_up.svgz"),      uL("RS UP")     },
			{ ABS_RY,         uL(":/icon/icons/gamepad_xbox_right_stick_down.svgz"),    uL("RS DOWN")   },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_playstation_4_r2.svgz"),         uL("R2")        },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_playstation_4_r2.svgz"),         uL("R2")        },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_playstation_4_dpad_left.svgz"),  uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_playstation_4_dpad_right.svgz"), uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_playstation_4_dpad_up.svgz"),    uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_playstation_4_dpad_down.svgz"),  uL("DPAD DOWN") }
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
			{ BTN_B,          uL(":/icon/icons/gamepad_playstation_x.svgz"),            uL("CROSS")     },
			{ BTN_X,          uL(":/icon/icons/gamepad_playstation_c.svgz"),            uL("CIRCLE")    },
			{ BTN_A,          uL(":/icon/icons/gamepad_playstation_s.svgz"),            uL("SQUARE")    },
			{ BTN_Y,          uL(":/icon/icons/gamepad_playstation_t.svgz"),            uL("TRIANGLE")  },
			{ BTN_TL,         uL(":/icon/icons/gamepad_playstation_4_l1.svgz"),         uL("L1")        },
			{ BTN_TR,         uL(":/icon/icons/gamepad_playstation_4_r1.svgz"),         uL("R1")        },
			{ BTN_SELECT,     uL(":/icon/icons/gamepad_playstation_4_l2.svgz"),         uL("L2")        },
			{ BTN_START,      uL(":/icon/icons/gamepad_playstation_4_r2.svgz"),         uL("R2")        },
			{ BTN_MODE,       uL(":/icon/icons/gamepad_playstation_4_share.svgz"),      uL("SHARE")     },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_playstation_4_options.svgz"),    uL("OPTIONS")   },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/gamepad_playstation_home.svgz"),         uL("HOME")      },
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_xbox_left_stick_click.svgz"),    uL("LS CLICK")  },
			{ BTN_DPAD_UP,    uL(":/icon/icons/gamepad_xbox_right_stick_click.svgz"),   uL("RS CLICK")  },
			{ BTN_DPAD_LEFT,  uL(":/icon/icons/gamepad_playstation_4_tpad_click.svgz"), uL("TPAD CLICK")}
		},
		{
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_left.svgz"),     uL("LS LEFT")   },
			{ ABS_X,          uL(":/icon/icons/gamepad_xbox_left_stick_right.svgz"),    uL("LS RIGHT")  },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_up.svgz"),       uL("LS UP")     },
			{ ABS_Y,          uL(":/icon/icons/gamepad_xbox_left_stick_down.svgz"),     uL("LS DOWN")   },
			{ ABS_RX,         uL(":/icon/icons/gamepad_playstation_4_l2.svgz"),         uL("L2")        },
			{ ABS_RX,         uL(":/icon/icons/gamepad_playstation_4_l2.svgz"),         uL("L2")        },
			{ ABS_Z,          uL(":/icon/icons/gamepad_xbox_right_stick_left.svgz"),    uL("RS LEFT")   },
			{ ABS_Z,          uL(":/icon/icons/gamepad_xbox_right_stick_right.svgz"),   uL("RS RIGHT")  },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_xbox_right_stick_up.svgz"),      uL("RS UP")     },
			{ ABS_RZ,         uL(":/icon/icons/gamepad_xbox_right_stick_down.svgz"),    uL("RS DOWN")   },
			{ ABS_RY,         uL(":/icon/icons/gamepad_playstation_4_r2.svgz"),         uL("R2")        },
			{ ABS_RY,         uL(":/icon/icons/gamepad_playstation_4_r2.svgz"),         uL("R2")        },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_playstation_4_dpad_left.svgz"),  uL("DPAD LEFT") },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_playstation_4_dpad_right.svgz"), uL("DPAD RIGHT")},
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_playstation_4_dpad_up.svgz"),    uL("DPAD UP")   },
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_playstation_4_dpad_down.svgz"),  uL("DPAD DOWN") }
		}
#endif
	},
#if defined (__linux__)
	// Steam Controller
	{
		JS_SC_MS_XBOX_360_GAMEPAD,
		FALSE,
		0x28DE,
		0x1102,
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
			{ BTN_A,          uL(":/icon/icons/gamepad_xbox_one_a.svgz"),              uL("A")          },
			{ BTN_B,          uL(":/icon/icons/gamepad_xbox_one_b.svgz"),              uL("B")          },
			{ BTN_X,          uL(":/icon/icons/gamepad_xbox_one_x.svgz"),              uL("X")          },
			{ BTN_Y,          uL(":/icon/icons/gamepad_xbox_one_y.svgz"),              uL("Y")          },
			{ BTN_TL,         uL(":/icon/icons/gamepad_steam_lb.svgz"),                uL("LB")         },
			{ BTN_TR,         uL(":/icon/icons/gamepad_steam_rb.svgz"),                uL("RB")         },
			{ BTN_TL2,        uL(":/icon/icons/gamepad_steam_lt.svgz"),                uL("LT")         },
			{ BTN_TR2,        uL(":/icon/icons/gamepad_steam_rt.svgz"),                uL("RT")         },
			{ BTN_SELECT,     uL(":/icon/icons/gamepad_steam_back.svgz"),              uL("BACK")       },
			{ BTN_START,      uL(":/icon/icons/gamepad_steam_start.svgz"),             uL("START")      },
			{ BTN_MODE,       uL(":/icon/icons/gamepad_steam_home.svgz"),              uL("HOME")       },
			{ BTN_THUMBL,     uL(":/icon/icons/gamepad_stick_click.svgz"),             uL("STICK CLICK")},
			{ BTN_THUMBR,     uL(":/icon/icons/gamepad_steam_right_track_click.svgz"), uL("RTCK CLICK") },
			{ BTN_DPAD_LEFT,  uL(":/icon/icons/gamepad_steam_left_track_left.svgz"),   uL("LTCK LEFT")  },
			{ BTN_DPAD_RIGHT, uL(":/icon/icons/gamepad_steam_left_track_right.svgz"),  uL("LTCK RIGHT") },
			{ BTN_DPAD_UP,    uL(":/icon/icons/gamepad_steam_left_track_up.svgz"),     uL("LTCK UP")    },
			{ BTN_DPAD_DOWN,  uL(":/icon/icons/gamepad_steam_left_track_down.svgz"),   uL("LTCK DOWN")  },
			{ BTN_THUMB,      uL(":/icon/icons/gamepad_steam_left_track_click.svgz"),  uL("LTCK TOUCH") },
			{ BTN_THUMB2,     uL(":/icon/icons/gamepad_steam_right_track_click.svgz"), uL("RTCK TOUCH") },
			{ BTN_GEAR_DOWN,  uL(":/icon/icons/gamepad_steam_left_grip.svgz"),         uL("LGRIP")      },
			{ BTN_GEAR_UP,    uL(":/icon/icons/gamepad_steam_right_grip.svgz"),        uL("RGRIP")      }
		},
		{
			{ ABS_X,          uL(":/icon/icons/gamepad_stick_left.svgz"),              uL("STICK LEFT") },
			{ ABS_X,          uL(":/icon/icons/gamepad_stick_right.svgz"),             uL("STICK RIGHT")},
			{ ABS_Y,          uL(":/icon/icons/gamepad_stick_up.svgz"),                uL("STICK UP")   },
			{ ABS_Y,          uL(":/icon/icons/gamepad_stick_down.svgz"),              uL("STICK DOWN") },
			{ ABS_RX,         uL(":/icon/icons/gamepad_steam_right_track_left.svgz"),  uL("RTCK LEFT")  },
			{ ABS_RX,         uL(":/icon/icons/gamepad_steam_right_track_right.svgz"), uL("RTCK RIGHT") },
			{ ABS_RY,         uL(":/icon/icons/gamepad_steam_right_track_up.svgz"),    uL("RTCK UP")    },
			{ ABS_RY,         uL(":/icon/icons/gamepad_steam_right_track_down.svgz"),  uL("RTCK DOWN")  },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_steam_left_track_left.svgz"),   uL("LTCK LEFT")  },
			{ ABS_HAT0X,      uL(":/icon/icons/gamepad_steam_left_track_right.svgz"),  uL("LTCK RIGHT") },
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_steam_left_track_up.svgz"),     uL("LTCK UP")    },
			{ ABS_HAT0Y,      uL(":/icon/icons/gamepad_steam_left_track_down.svgz"),   uL("LTCK DOWN")  },
			{ ABS_HAT2X,      uL(":/icon/icons/gamepad_steam_rt.svgz"),                uL("RT")         },
			{ ABS_HAT2X,      uL(":/icon/icons/gamepad_steam_rt.svgz"),                uL("RT")         },
			{ ABS_HAT2Y,      uL(":/icon/icons/gamepad_steam_lt.svgz"),                uL("LT")         },
			{ ABS_HAT2Y,      uL(":/icon/icons/gamepad_steam_lt.svgz"),                uL("LT")         },
		}
	},
#endif
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
	},
	// Retro-Bit NES adapter
	{
		JS_SC_UNKNOWN,
		FALSE,
		0x1292,
		0x4643,
#if defined (__linux__)
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_A),
			/* BUT_B  */ JS_BTN_DEF(BTN_B),
			/* SELECT */ JS_BTN_DEF(BTN_X),
			/* START  */ JS_BTN_DEF(BTN_Y),
			/* UP     */ JS_ABS_DEF(ABS_HAT0Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_HAT0Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_HAT0X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_HAT0X, 1),
			/* TRB_A  */ 0x000,
			/* TRB_B  */ 0x000
		},
		{},
		{}
#else
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_A),
			/* BUT_B  */ JS_BTN_DEF(BTN_B),
			/* SELECT */ JS_BTN_DEF(BTN_X),
			/* START  */ JS_BTN_DEF(BTN_Y),
			/* UP     */ JS_ABS_DEF(ABS_HAT0Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_HAT0Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_HAT0X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_HAT0X, 1),
			/* TRB_A  */ 0x000,
			/* TRB_B  */ 0x000
		},
		{},
		{}
#endif
	},
	// SNES adapter
	{
		JS_SC_UNKNOWN,
		FALSE,
		0x0E8F,
		0x3013,
#if defined (__linux__)
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_B),
			/* BUT_B  */ JS_BTN_DEF(BTN_A),
			/* SELECT */ JS_BTN_DEF(BTN_SELECT),
			/* START  */ JS_BTN_DEF(BTN_START),
			/* UP     */ JS_ABS_DEF(ABS_HAT0Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_HAT0Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_HAT0X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_HAT0X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_Y),
			/* TRB_B  */ JS_BTN_DEF(BTN_X)
		},
		{},
		{}
#else
		{
			/* BUT_A  */ JS_BTN_DEF(BTN_B),
			/* BUT_B  */ JS_BTN_DEF(BTN_X),
			/* SELECT */ JS_BTN_DEF(BTN_MODE),
			/* START  */ JS_BTN_DEF(BTN_THUMBL),
			/* UP     */ JS_ABS_DEF(ABS_HAT0Y, 0),
			/* DOWN   */ JS_ABS_DEF(ABS_HAT0Y, 1),
			/* LEFT   */ JS_ABS_DEF(ABS_HAT0X, 0),
			/* RIGHT  */ JS_ABS_DEF(ABS_HAT0X, 1),
			/* TRB_A  */ JS_BTN_DEF(BTN_A),
			/* TRB_B  */ JS_BTN_DEF(BTN_Y)
		},
		{},
		{}
#endif
	},
	// Mamba Retro SNES adapter
	{
			JS_SC_UNKNOWN,
			FALSE,
			0x081F,
			0xE401,
#if defined (__linux__)
			{
				/* BUT_A  */ JS_BTN_DEF(BTN_THUMB),
				/* BUT_B  */ JS_BTN_DEF(BTN_THUMB2),
				/* SELECT */ JS_BTN_DEF(BTN_BASE3),
				/* START  */ JS_BTN_DEF(BTN_BASE4),
				/* UP     */ JS_ABS_DEF(ABS_Y, 0),
				/* DOWN   */ JS_ABS_DEF(ABS_Y, 1),
				/* LEFT   */ JS_ABS_DEF(ABS_X, 0),
				/* RIGHT  */ JS_ABS_DEF(ABS_X, 1),
				/* TRB_A  */ JS_BTN_DEF(BTN_TRIGGER),
				/* TRB_B  */ JS_BTN_DEF(BTN_TOP)
			},
			{},
			{}
#else
			{
				/* BUT_A  */ JS_BTN_DEF(BTN_B),
				/* BUT_B  */ JS_BTN_DEF(BTN_X),
				/* SELECT */ JS_BTN_DEF(BTN_MODE),
				/* START  */ JS_BTN_DEF(BTN_THUMBL),
				/* UP     */ JS_ABS_DEF(ABS_HAT0Y, 0),
				/* DOWN   */ JS_ABS_DEF(ABS_HAT0Y, 1),
				/* LEFT   */ JS_ABS_DEF(ABS_HAT0X, 0),
				/* RIGHT  */ JS_ABS_DEF(ABS_HAT0X, 1),
				/* TRB_A  */ JS_BTN_DEF(BTN_A),
				/* TRB_B  */ JS_BTN_DEF(BTN_Y)
			},
			{},
			{}
#endif
		}
};

#endif /* _JSTICK_DB_H_ */
