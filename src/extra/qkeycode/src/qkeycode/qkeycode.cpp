/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../../to_compile.h"

#include "../../include/qkeycode/qkeycode.h"
#include "../../include/qkeycode/chromium/dom_code.h"
#include "../../include/qkeycode/chromium/keycode_converter.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtGui/QGuiApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QStyleHints>

#include <optional>

namespace qkeycode {

enum class KeyboardDriver { Unknown, Windows, Cocoa, Xkb, Evdev };

static KeyboardDriver keyboardDriverImpl()
{
    QString platformName = QGuiApplication::platformName();

    if (platformName == QLatin1String("windows"))
        return KeyboardDriver::Windows;

    if (platformName == QLatin1String("cocoa"))
        return KeyboardDriver::Cocoa;

    if (platformName == QLatin1String("xcb") || platformName == QLatin1String("wayland"))
        return KeyboardDriver::Xkb;

    return KeyboardDriver::Unknown;
}

static KeyboardDriver keyboardDriver()
{
    static KeyboardDriver cached = keyboardDriverImpl();
    return cached;
}

#if defined(Q_OS_MACOS)
// Qt swaps the Control and Meta keys on macOS (unless the attribute
// AA_MacDontSwapCtrlAndMeta is set). To preserve compatibility with Chromium we
// want to unswap them when forwarding events. The following two functions,
// qtKeyForKeyEvent and qtModifiersForEvent, should be used for accessing the
// key() and modifiers() properties to ensure that the unswapping is done
// consistently.
static int qtKeyForKeyEvent(const QKeyEvent *ev)
{
    int key = ev->key();
    if (keyboardDriver() == KeyboardDriver::Cocoa && !qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)) {
        if (key == Qt::Key_Control)
            return Qt::Key_Meta;
        if (key == Qt::Key_Meta)
            return Qt::Key_Control;
    }
    return key;
}
#endif

std::optional<quint32> asNonzero(quint32 x) {
    if (x != 0) {
        return x;
    } else {
        return std::nullopt;
    }
}

// The 'native key code' in Chromium refers to
//
//   - On Windows: the Windows OEM scancode.
//   - On macOS: the NSEvent's keyCode.
//   - On Linux: The XKB keycode.
static std::optional<quint32> nativeKeyCodeForKeyEvent(const QKeyEvent *ev)
{
    // Ifdefs here should match <chromium/keycode_converter.cc> kDomCodeMappings,
    // since NativeKeycodeToDomCode() is where the native key code is eventually used.
    //
    // Note that Xkb key codes are only supported under Linux (no BSDs,
    // Cygwin/X, etc). Also evdev key codes are *not* supported for the same
    // reason.
#if defined(Q_OS_WINDOWS)
    // "Extended keys" (like left/right win and right ctrl/alt)
    // have different scancodes in Qt and Chromium.
    // If you press right ctrl, Qt sends 0x11d (the Win32 flag+scancode).
    // But Chromium expects 0xe01d (the hardware 16-bit scancode).
    // So convert it to the format Chromium wants.
    //
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown#parameters
    // Win32 delivers a 9-bit flag+scancode between 0x000 and 0x1ff, in bits 16-23 and 24.
    // Qt processes these scancodes at QWindowsKeyMapper::translateKeyEventInternal().
    // The full list of extended keys is at
    // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown#remarks.
    //
    // Hardware scancodes are listed at https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html.
    // (Apparently these scancodes differ from USB keyboard events!)

    quint32 code = keyboardDriver() == KeyboardDriver::Windows ? ev->nativeScanCode() : 0;
    if (code & 0x100) {
        code = (code - 0x100) | 0xe000;
    }
    return asNonzero(code);
#elif defined(Q_OS_LINUX)
    return asNonzero(
        keyboardDriver() == KeyboardDriver::Xkb ? ev->nativeScanCode() : 0);
#elif defined(Q_OS_MACOS)
    // When Qt (filename qnsview_keys.mm) receives a native keypress through
    // - (void)keyDown:(NSEvent *)nsevent,
    // it calls - (bool)handleKeyEvent:(NSEvent *)nsevent eventType:(int)eventType.
    // This obtains nativeVirtualKey = [nsevent keyCode]
    // and passes it to QWindowSystemInterface::handleExtendedKeyEvent().
    //
    // When Qt receives a native modifier key press/release through
    // - (void)flagsChanged:(NSEvent *)nsEvent,
    // it doesn't obtain [nsevent keyCode] and instead calls
    // QWindowSystemInterface::handleKeyEvent() which sets all native keycodes to 0.
    //
    // So QtWebEngine loses information on whether the left or right key was pressed,
    // and must reconstruct dom_code from the Windows keycode
    // (computed from QKeyEvent::key()).
    //
    // Distinguishing left/right modifiers requires a change to Qt (qnsview_keys.mm)
    // to set QKeyEvent::native...() to their proper values.

    // BUG: On Qt 6.2.3 on macOS 12 on non-virtualized Mac,
    // ev->nativeScanCode() is always 0 even for real keypresses.
    // So just always return it.

    if (keyboardDriver() == KeyboardDriver::Cocoa) {
        return ev->nativeVirtualKey();
    } else {
        return std::nullopt;
    }
#else
#error Unsupported platform, cannot determine keycode
    return 0; // 0 means unknown, KeyboardEvent.code will be empty string.
#endif
}

KeyCode toKeycode(QKeyEvent *ev)
{
    // The dom_code field should contain the USB keycode of the *physical* key
    // that was pressed. Physical meaning independent of layout and modifiers.
    // Since this information is not available from QKeyEvent in portable form,
    // we try to compute it from the native key code.
    auto dom_code = KeyCode::NONE;

#if defined(Q_OS_MACOS)
    int qtKey = qtKeyForKeyEvent(ev);
    // On Mac, - (void)flagsChanged:(NSEvent *)nsevent
    // converts certain NSEventModifierFlag into Qt::Key.
    // Convert the corresponding Qt::Key to keycodes.
    //
    // As discussed in nativeKeyCodeForKeyEvent(),
    // we cannot tell if the left or right modifier was pressed, so assume left.
    switch (qtKey) {
    case Qt::Key_Shift:
        dom_code = KeyCode::SHIFT_LEFT; break;
    case Qt::Key_Meta:
        dom_code = KeyCode::META_LEFT; break;
    case Qt::Key_Control:
        dom_code = KeyCode::CONTROL_LEFT; break;
    case Qt::Key_Alt:
        dom_code = KeyCode::ALT_LEFT; break;
    case Qt::Key_CapsLock:
        dom_code = KeyCode::CAPS_LOCK; break;
    default:
        // Don't know which key... too bad :(
        break;
    }
#endif

    if (dom_code == KeyCode::NONE) {
        if (auto nativeKeyCodeMaybe = nativeKeyCodeForKeyEvent(ev)) {
            dom_code =
                ui::KeycodeConverter::NativeKeycodeToDomCode(*nativeKeyCodeMaybe);
        }
    }

    // The original QtWebEngine code synthesized dom_code from windows_key_code
    // when dom_code was unset, by calling UsLayoutKeyboardCodeToDomCode().
    // This fallback was a massive mistake. It covered up many Qt/QtWebEngine bugs
    // (non-QWERTY `a` key 0 on Mac, right modifier keys on Mac, extended keys on Windows)
    // and caused them to behave "almost correctly",
    // so you wouldn't notice misbehavior unless you paid attention
    // or were using a non-QWERTY keyboard layout.

    return dom_code;
}

} // namespace QtWebEngineCore
