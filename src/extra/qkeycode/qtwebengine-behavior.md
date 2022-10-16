## QtWebEngine issues

QtWebEngine keyboard input has different bugs on different platforms. I used [W3C's Keyboard Event Viewer](https://w3c.github.io/uievents/tools/key-event-viewer.html) to test browser behavior. I have more in-depth explorations, and links to the Qt code in question, in a [Google doc](https://docs.google.com/document/d/1GBskBIcBoL5jj0Jb9ZD5H-pvUUKoPp2zxk2p4mpqvx4/edit#heading=h.kdanylbal2rb).

### macOS, Qt 5.15 (2020) (QWebEngine demo browser)

macOS has the most interesting forms of misbehavior, resulting from the `a` key having value 0.

- Doesn't recognize the QWERTY `a` key (with Mac keycode 0) and falls back to QKeyEvent::key().
    - Reported at https://bugreports.qt.io/browse/QTBUG-85660
- Treats all modifier keys as left.
    - QKeyEvent::nativeVirtualKey() for modifier keys is always 0, so QtWebEngine falls back to QKeyEvent::key() which always assumes the left key was pressed.
    - Commented at https://bugreports.qt.io/browse/QTBUG-69608.
- Dead keys don't produce proper keyDown/keyUp events.
    - Not reported. https://keycode.info/ thinks dead keys are unknown.
    - W3C says neither keyDown nor keyUp occurs, only some composition and input events. When you press deadkey+character, both deadkey down/up are missing, and character down is missing, but character up is present.

### Windows, Qt 5.12 (2018) (Falkon browser)

QtWebEngine 5.12 (an older version) is especially broken. It takes QKeyEvent::key() and converts it into a hardware keycode (assuming QWERTY). The resulting keycode is scrambled on non-QWERTY layouts, and the conversion fails entirely on non-Latin layouts.

### Windows, Qt 5.15 (QWebEngine demo browser)

Windows has less severe issues.

- Discards the first press of dead keys (Greek ΄ key, AZERTY ^). If you press the dead key twice, two key downs, then two key ups, are registered.
    - blame win32 apparently. See Stack Overflow below.
- Right Ctrl and Alt are seen as left keys.
    - When you press an "extended key" (right Ctrl/Alt, any Windows key, pageup/down), QKeyEvent::nativeScanCode() returns a 0x1xx value, but Chromium expects a 0xe0xx value and fails to convert the scancode. So QtWebEngine falls back to QKeyEvent::key() which always assumes the left key was pressed.
    - Reported at https://bugreports.qt.io/browse/QTBUG-85661

### Linux xkb/IBus, Qt 5.15 (Falkon browser)

- When pressing a dead key, the dead key and following keypress do not generate press events, only release events.

## qkeycode behavior

Most QtWebEngine issues were fixed, except for modifier keys on Mac. I did not specifically address dead key issues; some still occur (probably due to Qt rather than qkeycode/QtWebEngine), and others don't (probably due to QtWebEngine or Chromium). I don't fully understand the behavior and interactions though.

### Windows, Qt 5.15

- Bug: dead keys are still delayed, unfortunately. Qt chooses to delay delivering QKeyEvent, so you can't extract keycodes from keypresses that you don't see. I found an inactive [question on Stack Overflow](https://stackoverflow.com/questions/3872085) about this issue.

### macOS, Qt 5.15

- Bug: treats all modifier keys as left. Qt itself throws away the true keys pressed before constructing a QKeyEvent (QKeyEvent::nativeVirtualKey() is always 0). Qt would have to be fixed, for this problem to go away.
- No issues with dead keys.

### Linux xkb/IBus, Qt 5.15

- No issues as far as I can tell. The dead key issue does not manifest in Qt-only demo programs, without QtWebEngine on top. So qkeycode was already unaffected even before I started fixing bugs.

## qkeycode and nonstandard input devices

### Windows, AutoHotkey

- `SendMode Event` (default): works like manual typing
- `SendMode Input`: works like manual typing
- `SendMode Play`: no keys detected, doesn't work in text editors either
    - AutoHotkey is probably broken.

Fun things happen when you switch key layouts. In both Event and Input mode, on non-QWERTY layouts, the keycodes are sometimes reverse-permuted and sometimes pretend to be QWERTY. On non-Latin layouts, all keypresses resolve to `KeyCode::NONE` (unknown key location).

## Thoughts

The original QtWebEngine code synthesized `dom_code` (physical keycode) from `windows_key_code` (layout-dependent key) when `dom_code` was unset. This fallback was a massive mistake. It covered up many Qt/QtWebEngine bugs and caused QtWebEngine to behave "almost correctly" for common configurations, but fail to distinguish left and right modifier keys, and misbehave on non-QWERTY keyboard layouts.
