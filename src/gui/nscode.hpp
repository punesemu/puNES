/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef NSCODE_H_
#define NSCODE_H_

#include "extra/qkeycode/include/qkeycode/qkeycode.h"
#include "extra/qkeycode/include/qkeycode/chromium/dom_code.h"

#define NSCODE_Sleep qkeycode::KeyCode::SLEEP
#define NSCODE_WakeUp qkeycode::KeyCode::WAKE_UP

#define NSCODE_A qkeycode::KeyCode::US_A
#define NSCODE_B qkeycode::KeyCode::US_B
#define NSCODE_C qkeycode::KeyCode::US_C
#define NSCODE_D qkeycode::KeyCode::US_D

#define NSCODE_E qkeycode::KeyCode::US_E
#define NSCODE_F qkeycode::KeyCode::US_F
#define NSCODE_G qkeycode::KeyCode::US_G
#define NSCODE_H qkeycode::KeyCode::US_H
#define NSCODE_I qkeycode::KeyCode::US_I
#define NSCODE_J qkeycode::KeyCode::US_J
#define NSCODE_K qkeycode::KeyCode::US_K
#define NSCODE_L qkeycode::KeyCode::US_L

#define NSCODE_M qkeycode::KeyCode::US_M
#define NSCODE_N qkeycode::KeyCode::US_N
#define NSCODE_O qkeycode::KeyCode::US_O
#define NSCODE_P qkeycode::KeyCode::US_P
#define NSCODE_Q qkeycode::KeyCode::US_Q
#define NSCODE_R qkeycode::KeyCode::US_R
#define NSCODE_S qkeycode::KeyCode::US_S
#define NSCODE_T qkeycode::KeyCode::US_T

#define NSCODE_U qkeycode::KeyCode::US_U
#define NSCODE_V qkeycode::KeyCode::US_V
#define NSCODE_W qkeycode::KeyCode::US_W
#define NSCODE_X qkeycode::KeyCode::US_X
#define NSCODE_Y qkeycode::KeyCode::US_Y
#define NSCODE_Z qkeycode::KeyCode::US_Z
#define NSCODE_1 qkeycode::KeyCode::DIGIT1
#define NSCODE_2 qkeycode::KeyCode::DIGIT2

#define NSCODE_3 qkeycode::KeyCode::DIGIT3
#define NSCODE_4 qkeycode::KeyCode::DIGIT4
#define NSCODE_5 qkeycode::KeyCode::DIGIT5
#define NSCODE_6 qkeycode::KeyCode::DIGIT6
#define NSCODE_7 qkeycode::KeyCode::DIGIT7
#define NSCODE_8 qkeycode::KeyCode::DIGIT8
#define NSCODE_9 qkeycode::KeyCode::DIGIT9
#define NSCODE_0 qkeycode::KeyCode::DIGIT0

#define NSCODE_Return qkeycode::KeyCode::ENTER
#define NSCODE_Escape qkeycode::KeyCode::ESCAPE
#define NSCODE_Backspace qkeycode::KeyCode::BACKSPACE
#define NSCODE_Tab qkeycode::KeyCode::TAB
#define NSCODE_Space qkeycode::KeyCode::SPACE
#define NSCODE_Minus qkeycode::KeyCode::MINUS
#define NSCODE_Equal qkeycode::KeyCode::EQUAL
#define NSCODE_BracketLeft qkeycode::KeyCode::BRACKET_LEFT

#define NSCODE_BracketRight qkeycode::KeyCode::BRACKET_RIGHT
#define NSCODE_Backslash qkeycode::KeyCode::BACKSLASH
#define NSCODE_Semicolon qkeycode::KeyCode::SEMICOLON
#define NSCODE_Apostrophe qkeycode::KeyCode::QUOTE
#define NSCODE_QuoteLeft qkeycode::KeyCode::BACKQUOTE
#define NSCODE_Comma qkeycode::KeyCode::COMMA
#define NSCODE_Period qkeycode::KeyCode::PERIOD
#define NSCODE_Slash qkeycode::KeyCode::SLASH

#define NSCODE_CapsLock qkeycode::KeyCode::CAPS_LOCK
#define NSCODE_F1 qkeycode::KeyCode::F1
#define NSCODE_F2 qkeycode::KeyCode::F2
#define NSCODE_F3 qkeycode::KeyCode::F3
#define NSCODE_F4 qkeycode::KeyCode::F4
#define NSCODE_F5 qkeycode::KeyCode::F5
#define NSCODE_F6 qkeycode::KeyCode::F6
#define NSCODE_F7 qkeycode::KeyCode::F7
#define NSCODE_F8 qkeycode::KeyCode::F8
#define NSCODE_F9 qkeycode::KeyCode::F9
#define NSCODE_F10 qkeycode::KeyCode::F10
#define NSCODE_F11 qkeycode::KeyCode::F11
#define NSCODE_F12 qkeycode::KeyCode::F12
#define NSCODE_Print qkeycode::KeyCode::PRINT_SCREEN
#define NSCODE_ScrollLock qkeycode::KeyCode::SCROLL_LOCK

#define NSCODE_Pause qkeycode::KeyCode::PAUSE
#define NSCODE_Insert qkeycode::KeyCode::INSERT
#define NSCODE_Home qkeycode::KeyCode::HOME
#define NSCODE_PageUp qkeycode::KeyCode::PAGE_UP
#define NSCODE_Delete qkeycode::KeyCode::DEL
#define NSCODE_End qkeycode::KeyCode::END
#define NSCODE_PageDown qkeycode::KeyCode::PAGE_DOWN
#define NSCODE_Right qkeycode::KeyCode::ARROW_RIGHT

#define NSCODE_Left qkeycode::KeyCode::ARROW_LEFT
#define NSCODE_Down qkeycode::KeyCode::ARROW_DOWN
#define NSCODE_Up qkeycode::KeyCode::ARROW_UP
#define NSCODE_NumLock qkeycode::KeyCode::NUM_LOCK

#define NSCODE_KSlash qkeycode::KeyCode::NUMPAD_DIVIDE
#define NSCODE_KAsterisk qkeycode::KeyCode::NUMPAD_MULTIPLY
#define NSCODE_KMinus qkeycode::KeyCode::NUMPAD_SUBTRACT
#define NSCODE_KPlus qkeycode::KeyCode::NUMPAD_ADD

#define NSCODE_Enter qkeycode::KeyCode::NUMPAD_ENTER
#define NSCODE_K1 qkeycode::KeyCode::NUMPAD1
#define NSCODE_K2 qkeycode::KeyCode::NUMPAD2
#define NSCODE_K3 qkeycode::KeyCode::NUMPAD3
#define NSCODE_K4 qkeycode::KeyCode::NUMPAD4
#define NSCODE_K5 qkeycode::KeyCode::NUMPAD5
#define NSCODE_K6 qkeycode::KeyCode::NUMPAD6
#define NSCODE_K7 qkeycode::KeyCode::NUMPAD7
#define NSCODE_K8 qkeycode::KeyCode::NUMPAD8
#define NSCODE_K9 qkeycode::KeyCode::NUMPAD9
#define NSCODE_K0 qkeycode::KeyCode::NUMPAD0
#define NSCODE_KPeriod qkeycode::KeyCode::NUMPAD_DECIMAL

#define NSCODE_IntlBackslash qkeycode::KeyCode::INTL_BACKSLASH
#define NSCODE_Menu qkeycode::KeyCode::CONTEXT_MENU
#define NSCODE_Power qkeycode::KeyCode::POWER
#define NSCODE_KEqual qkeycode::KeyCode::NUMPAD_EQUAL

#define NSCODE_F13 qkeycode::KeyCode::F13
#define NSCODE_F14 qkeycode::KeyCode::F14
#define NSCODE_F15 qkeycode::KeyCode::F15
#define NSCODE_F16 qkeycode::KeyCode::F16
#define NSCODE_F17 qkeycode::KeyCode::F17
#define NSCODE_F18 qkeycode::KeyCode::F18
#define NSCODE_F19 qkeycode::KeyCode::F19
#define NSCODE_F20 qkeycode::KeyCode::F20

#define NSCODE_F21 qkeycode::KeyCode::F21
#define NSCODE_F22 qkeycode::KeyCode::F22
#define NSCODE_F23 qkeycode::KeyCode::F23
#define NSCODE_F24 qkeycode::KeyCode::F24
#define NSCODE_Open qkeycode::KeyCode::OPEN
#define NSCODE_Help qkeycode::KeyCode::HELP

#define NSCODE_Undo qkeycode::KeyCode::UNDO
#define NSCODE_Cut qkeycode::KeyCode::CUT
#define NSCODE_Copy qkeycode::KeyCode::COPY
#define NSCODE_Paste qkeycode::KeyCode::PASTE
#define NSCODE_VolMute qkeycode::KeyCode::VOLUME_MUTE

#define NSCODE_VolUp qkeycode::KeyCode::VOLUME_UP
#define NSCODE_VolDown qkeycode::KeyCode::VOLUME_DOWN
#define NSCODE_KComma qkeycode::KeyCode::NUMPAD_COMMA

#define NSCODE_IntlRo qkeycode::KeyCode::INTL_RO
#define NSCODE_KanaMode qkeycode::KeyCode::KANA_MODE
#define NSCODE_IntlYen qkeycode::KeyCode::INTL_YEN
#define NSCODE_Convert qkeycode::KeyCode::CONVERT
#define NSCODE_NonConvert qkeycode::KeyCode::NON_CONVERT

#define NSCODE_Lang1 qkeycode::KeyCode::LANG1
#define NSCODE_Lang2 qkeycode::KeyCode::LANG2
#define NSCODE_Lang3 qkeycode::KeyCode::LANG3
#define NSCODE_Lang4 qkeycode::KeyCode::LANG4

#define NSCODE_LControl qkeycode::KeyCode::CONTROL_LEFT
#define NSCODE_LShift qkeycode::KeyCode::SHIFT_LEFT
#define NSCODE_Alt qkeycode::KeyCode::ALT_LEFT
#define NSCODE_Super_L qkeycode::KeyCode::META_LEFT
#define NSCODE_RControl qkeycode::KeyCode::CONTROL_RIGHT
#define NSCODE_RShift qkeycode::KeyCode::SHIFT_RIGHT
#define NSCODE_AltGr qkeycode::KeyCode::ALT_RIGHT
#define NSCODE_Super_R qkeycode::KeyCode::META_RIGHT

#define NSCODE_MediaTrackNext qkeycode::KeyCode::MEDIA_TRACK_NEXT
#define NSCODE_MediaTrackPrevious qkeycode::KeyCode::MEDIA_TRACK_PREVIOUS
#define NSCODE_MediaStop qkeycode::KeyCode::MEDIA_STOP
#define NSCODE_Eject qkeycode::KeyCode::EJECT
#define NSCODE_MediaPlayPause qkeycode::KeyCode::MEDIA_PLAY_PAUSE
#define NSCODE_MediaSelect qkeycode::KeyCode::MEDIA_SELECT
#define NSCODE_LaunchMail qkeycode::KeyCode::LAUNCH_MAIL
#define NSCODE_LaunchApp2 qkeycode::KeyCode::LAUNCH_APP2
#define NSCODE_LaunchApp1 qkeycode::KeyCode::LAUNCH_APP1
#define NSCODE_BrowserSearch qkeycode::KeyCode::BROWSER_SEARCH
#define NSCODE_BrowserHome qkeycode::KeyCode::BROWSER_HOME
#define NSCODE_BrowserBack qkeycode::KeyCode::BROWSER_BACK
#define NSCODE_BrowserForward qkeycode::KeyCode::BROWSER_FORWARD
#define NSCODE_BrowserStop qkeycode::KeyCode::BROWSER_STOP
#define NSCODE_BrowserRefresh qkeycode::KeyCode::BROWSER_REFRESH
#define NSCODE_BrowserFavorites qkeycode::KeyCode::BROWSER_FAVORITES

#endif /* NSCODE_H_ */
