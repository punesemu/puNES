// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "../../../to_compile.h"

#include "../../../include/qkeycode/chromium/keycode_converter.h"

#include "../../../include/qkeycode/chromium/dom_code.h"

#include <QtCore/QDebug>

namespace qkeycode::ui {

namespace {

// Table of USB codes (equivalent to DomCode values), native scan codes,
// and DOM Level 3 |code| strings.
#if defined(Q_OS_WINDOWS)
#define DOM_CODE(usb, evdev, xkb, win, mac, code, id) \
  { usb, win, code }
#elif defined(Q_OS_ANDROID)
#define DOM_CODE(usb, evdev, xkb, win, mac, code, id) \
  { usb, evdev, code }
#elif defined(Q_OS_LINUX)
#define DOM_CODE(usb, evdev, xkb, win, mac, code, id) \
  { usb, xkb, code }
#elif defined(Q_OS_MACOS)
#define DOM_CODE(usb, evdev, xkb, win, mac, code, id) \
  { usb, mac, code }
#else
#error Unsupported platform
#endif
#define DOM_CODE_DECLARATION const KeycodeMapEntry kDomCodeMappings[] =
#include "../../../include/qkeycode/chromium/dom_code_data.inc"
#undef DOM_CODE
#undef DOM_CODE_DECLARATION

}  // namespace

// static
int KeycodeConverter::InvalidNativeKeycode() {
  return kDomCodeMappings[0].native_keycode;
}

// TODO(zijiehe): Most of the following functions can be optimized by using
// either multiple arrays or unordered_map.

// static
DomCode KeycodeConverter::NativeKeycodeToDomCode(int native_keycode) {
  for (auto& mapping : kDomCodeMappings) {
    if (mapping.native_keycode == native_keycode)
      return static_cast<DomCode>(mapping.usb_keycode);
  }
  return DomCode::NONE;
}

// static
int KeycodeConverter::DomCodeToNativeKeycode(DomCode code) {
  return UsbKeycodeToNativeKeycode(static_cast<uint32_t>(code));
}

// static
DomCode KeycodeConverter::CodeStringToDomCode(const std::string& code) {
  if (code.empty())
    return DomCode::NONE;
  for (auto& mapping : kDomCodeMappings) {
    if (mapping.code && code == mapping.code) {
      return static_cast<DomCode>(mapping.usb_keycode);
    }
  }
  qWarning() << "unrecognized code string '" << code.c_str() << "'";
  return DomCode::NONE;
}

// static
const char* KeycodeConverter::DomCodeToCodeString(DomCode dom_code) {
  for (auto& mapping : kDomCodeMappings) {
    if (mapping.usb_keycode == static_cast<uint32_t>(dom_code)) {
      if (mapping.code)
        return mapping.code;
      break;
    }
  }
  return "";
}

// static
DomKeyLocation KeycodeConverter::DomCodeToLocation(DomCode dom_code) {
  static const struct {
    DomCode code;
    DomKeyLocation location;
  } kLocations[] = {{DomCode::CONTROL_LEFT, DomKeyLocation::LEFT},
                    {DomCode::SHIFT_LEFT, DomKeyLocation::LEFT},
                    {DomCode::ALT_LEFT, DomKeyLocation::LEFT},
                    {DomCode::META_LEFT, DomKeyLocation::LEFT},
                    {DomCode::CONTROL_RIGHT, DomKeyLocation::RIGHT},
                    {DomCode::SHIFT_RIGHT, DomKeyLocation::RIGHT},
                    {DomCode::ALT_RIGHT, DomKeyLocation::RIGHT},
                    {DomCode::META_RIGHT, DomKeyLocation::RIGHT},
                    {DomCode::NUMPAD_DIVIDE, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_MULTIPLY, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_SUBTRACT, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_ADD, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_ENTER, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD1, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD2, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD3, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD4, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD5, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD6, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD7, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD8, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD9, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD0, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_DECIMAL, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_EQUAL, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_COMMA, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_PAREN_LEFT, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_PAREN_RIGHT, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_BACKSPACE, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_MEMORY_STORE, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_MEMORY_RECALL, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_MEMORY_CLEAR, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_MEMORY_ADD, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_MEMORY_SUBTRACT, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_SIGN_CHANGE, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_CLEAR, DomKeyLocation::NUMPAD},
                    {DomCode::NUMPAD_CLEAR_ENTRY, DomKeyLocation::NUMPAD}};
  for (const auto& key : kLocations) {
    if (key.code == dom_code)
      return key.location;
  }
  return DomKeyLocation::STANDARD;
}

// USB keycodes
// Note that USB keycodes are not part of any web standard.
// Please don't use USB keycodes in new code.

// static
uint32_t KeycodeConverter::InvalidUsbKeycode() {
  return kDomCodeMappings[0].usb_keycode;
}

// static
int KeycodeConverter::UsbKeycodeToNativeKeycode(uint32_t usb_keycode) {
  // Deal with some special-cases that don't fit the 1:1 mapping.
  if (usb_keycode == 0x070032)  // non-US hash.
    usb_keycode = 0x070031;     // US backslash.
#if defined(OS_MACOSX)
  if (usb_keycode == 0x070046) // PrintScreen.
    usb_keycode = 0x070068; // F13.
#endif

  for (auto& mapping : kDomCodeMappings) {
    if (mapping.usb_keycode == usb_keycode)
      return mapping.native_keycode;
  }
  return InvalidNativeKeycode();
}

// static
uint32_t KeycodeConverter::NativeKeycodeToUsbKeycode(int native_keycode) {
  for (auto& mapping : kDomCodeMappings) {
    if (mapping.native_keycode == native_keycode)
      return mapping.usb_keycode;
  }
  return InvalidUsbKeycode();
}

// static
DomCode KeycodeConverter::UsbKeycodeToDomCode(uint32_t usb_keycode) {
  for (auto& mapping : kDomCodeMappings) {
    if (mapping.usb_keycode == usb_keycode)
      return static_cast<DomCode>(usb_keycode);
  }
  return DomCode::NONE;
}

// static
uint32_t KeycodeConverter::DomCodeToUsbKeycode(DomCode dom_code) {
  for (auto& mapping : kDomCodeMappings) {
    if (mapping.usb_keycode == static_cast<uint32_t>(dom_code))
      return mapping.usb_keycode;
  }
  return InvalidUsbKeycode();
}

// static
uint32_t KeycodeConverter::CodeStringToUsbKeycode(const std::string& code) {
  if (code.empty())
    return InvalidUsbKeycode();

  for (auto& mapping : kDomCodeMappings) {
    if (mapping.code && code == mapping.code) {
      return mapping.usb_keycode;
    }
  }
  return InvalidUsbKeycode();
}

// static
int KeycodeConverter::CodeStringToNativeKeycode(const std::string& code) {
  return UsbKeycodeToNativeKeycode(CodeStringToUsbKeycode(code));
}

}  // namespace ui
