# qkeycode

[![Gitter](https://badges.gitter.im/qkeycode/community.svg)](https://gitter.im/qkeycode/community)

Library for converting from QKeyEvent to platform-independent keycodes (compatible with DOM `KeyboardEvent.code`).

## Background

Qt 5 does not offer a platform-independent way to link QKeyEvent instances to the keyboard key that created them. This is useful for games, WASD keyboard controls, and note entry in music applications.

[QKeyEvent::key()](https://doc.qt.io/qt-5/qkeyevent.html#key) is dependent on keyboard layout and whether Shift is being pressed; the key-down and key-up events may have different key() values! [QKeyEvent::nativeScanCode()](https://doc.qt.io/qt-5/qkeyevent.html#nativeScanCode) and [nativeVirtualKey()](https://doc.qt.io/qt-5/qkeyevent.html#nativeVirtualKey) have different meanings on different operating systems.

There have been multiple Qt Forum threads over the past several years [(2015)](https://forum.qt.io/topic/57567/how-to-get-nativevirtualkey-for-arrow-keys-solved) [(2018)](https://forum.qt.io/topic/95527/dealing-with-keyboard-layouts-for-input-on-multiple-platforms) [(2020)](https://forum.qt.io/topic/116181/questions-about-qkeyevent-s-native-scan-code) [(2020)](https://forum.qt.io/topic/116544/how-is-qkeyevent-supposed-to-be-used-for-games) where people attempt to obtain physical keys from QKeyEvent events. Citra 3DS emulator uses QKeyEvent::key() for game input, and [is keyboard-layout-dependent](https://github.com/citra-emu/citra/issues/4243). Among music programs, Polyphone SF2 editor and BambooTracker treat the computer keyboard keys as a piano keyboard, use QKeyEvent::key() to identify key presses, and produce stuck notes if you press Shift after you press a number key (since the key-up event has a different key from the key-down).

QtWebEngine attempts to translate from Qt QKeyEvent to DOM `KeyboardEvent`, including extracting a platform-independent keycode from QKeyEvent. Unfortunately, this code has many bugs. I based qkeycode off of QtWebEngine and fixed most input bugs (except for some resulting from underlying Qt bugs).

## Install

qkeycode is built using CMake, but should be fairly easy to adapt to other build systems. It can be placed directly into a CMake project and imported using `add_subdirectory(qkeycode)`, which creates a `qkeycode` target. I don't know how to write a `find_package()` script, but am accepting contributions.

The project layout is approximately compatible with the new [dds C++ build system / dependency manager](https://github.com/vector-of-bool/dds), but I'm not sure if that build system supports Qt dependencies.

qkeycode depends on C++17 for `std::optional`. I may possibly accept pull requests to remove this dependency, especially if you're trying to upstream my changes to QtWebEngine.

## Usage

Converting a QKeyEvent to a keycode, then printing the name:

```cpp
#include <qkeycode/qkeycode.h>

void MyWidget::keyPressEvent(QKeyEvent * event) {
    qkeycode::KeyCode dom_code = qkeycode::toKeycode(event);
    fmt::print(
        "KeyPress {}=\"{}\"",
        dom_code,
        qkeycode::KeycodeConverter::DomCodeToCodeString(dom_code)
    );
    Super::keyPressEvent(event);
}
```

qkeycode/qkeycode.h forward-declares `qkeycode::KeyCode` to possibly reduce build times. The naming scheme is a bit inconsistent because Chromium calls this type `DomCode` rather than `KeyCode`. `qkeycode::KeycodeConverter` is taken directly from Chromium source code, and you should look at <include/qkeycode/chromium/keycode_converter.h> for function signatures.

If you find any bugs where keypresses aren't detected properly (`toKeycode()` returns `KeyCode::NONE`), [file an issue](https://github.com/nyanpasu64/qkeycode/issues/new) with the reproduction steps and contents of the `QKeyEvent`! This library is designed to return `NONE` if it can't determine the right keycode, instead of faking one based on `QKeyEvent::key()` like QtWebEngine does.

---

Obtaining the list of keycodes, and hard-coding a key's value:

```cpp
#include <qkeycode/values.h>

qkeycode::KeyCode get_key() {
    return qkeycode::KeyCode::US_Z;
}
```

Note that qkeycode/values.h uses macros to declare an enum with uppercase values (like `COMMA`), so you shouldn't `#define COMMA ,` in a header file prior to including qkeycode/values.h.

## QtWebEngine and qkeycode behavior/issues

I wrote about this in [a separate document](qtwebengine-behavior.md). The only remaining notable issues are all modifiers being registered as left keys on Mac, and dead keys being delayed on Windows.

I hope that my changes to QtWebEngine's code can be upstreamed to [web_event_factory.cpp](https://github.com/qt/qtwebengine/blob/dev/src/core/web_event_factory.cpp) `WebEventFactory::toWebKeyboardEvent(QKeyEvent *ev)` and `nativeKeyCodeForKeyEvent(const QKeyEvent *ev)`. I don't know if they'll accept my approach of returning a missing `dom_code` as-is, not synthesizing one from `windows_key_code`.

## Thanks

This library would not be possible without QtWebEngine and Chromium.

## Contributing

You can ask questions in the [Issues section](https://github.com/nyanpasu64/qkeycode/issues), or in [Gitter chat](https://gitter.im/qkeycode/community). Pull requests are accepted. I'm not using a code formatter or CI yet.

## License

Because this library includes code from QtWebEngine, it can be used under LGPL v3 or GPL v2+ (like Qt itself), but not more permissive licenses. Details are in the [LICENSE file](LICENSE).
