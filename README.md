<h1>puNES</h1>

<span class="badge-paypal"><a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=QPPXNRL5NAHDC" title="Donate to this project using Paypal"><img src="https://img.shields.io/badge/paypal-donate-yellow.svg" alt="PayPal donate button" /></a></span>

Description
-----------

Nintendo Entertaiment System emulator

WIP  [![Build status](https://ci.appveyor.com/api/projects/status/t820vxmvtvbfbll7?svg=true)](https://ci.appveyor.com/project/punesemu/punes)
-----------
always updated to the last commit:
* Windows 32 bit : [`OpenGL`](https://ci.appveyor.com/api/projects/punesemu/puNES/artifacts/punes32.wip.opengl.zip) - [`D3D9`](https://ci.appveyor.com/api/projects/punesemu/puNES/artifacts/punes32.wip.d3d9.zip)
* Windows 64 bit : [`OpenGL`](https://ci.appveyor.com/api/projects/punesemu/puNES/artifacts/punes64.wip.opengl.zip) - [`D3D9`](https://ci.appveyor.com/api/projects/punesemu/puNES/artifacts/punes64.wip.d3d9.zip)

How to Compile
-----------

### Linux
-----------
#### Dependencies
* Qt5 with OpenGL support (qtbase, qtsvg and qttools)
* nvidia-cg
* alsa
#### Compilation of puNES
```bash
git clone https://github.com/punesemu/punes
cd punes
./autogen.sh
./configure
make
```
the executable `punes` is in the `src` directory.
#### Linux Debug version
If you need the debug version then you need to replace the `./configure` command of the previous examples with the following:
```bash
CFLAGS="-g -DDEBUG" CXXFLAGS="-g -DDEBUG" ./configure --disable-release [...]
```
where `[...]` are the other necessary options.
### OpenBSD
-----------
#### Dependencies
* Qt5 with OpenGL support (qtbase, qtsvg and qttools)
* sndio
#### Compilation of puNES
```bash
git clone https://github.com/punesemu/punes
cd punes
./autogen.sh
CC=cc CXX=c++ ./configure
make
```
the executable `punes` is in the `src` directory.
#### OpenBSD Debug version
If you need the debug version then you need to replace the `./configure` command of the previous examples with the following:
```bash
CFLAGS="-g -DDEBUG" CXXFLAGS="-g -DDEBUG" CC=cc CXX=c++ ./configure --disable-release [...]
```
where `[...]` are the other necessary options.
### Windows
-----------
#### Dependencies
* Qt5 with OpenGL support (5.6.3 is the last if you want the support for Windows XP)
#### Development Environment installation
1. install MSYS2 (https://www.msys2.org/)
2. open "MSYS2 MinGW 64-bit" shell (or 32 bit if you want compile the 32 bit version of puNES)
```bash
pacman -Syu
```
3. close the MSYS2 window and run it again from Start menu
```bash
pacman -Su
pacman -S base-devel git wget p7zip unzip
pacman -S perl ruby python2 mingw-w64-i686-toolchain mingw-w64-x86_64-toolchain
exit
```
4. open a new MSYS2 shell and build the necessary libraries
#### Compilation of the Qt5 libraries
5. download and unzip the sources
```bash
wget http://download.qt.io/archive/qt/5.13/5.13.0/submodules/qtbase-everywhere-src-5.13.0.zip
unzip qtbase-everywhere-src-5.13.0.zip
mv qtbase-everywhere-src-5.13.0 qt5
```
the renaming of the directory is necessary to not generate a compile-time error caused by the 255 characters maximum path length limitation on Windows, This is the typical error message you might encounter:
```code
"../../../../include/QtEventDispatcherSupport/5.13.0/QtEventDispatcherSupport/private/qwindowsguieventdispatcher_p.h:1:10: fatal error: ../../../../../src/platformsupport/eventdispatchers/qwindowsguieventdispatcher_p.h: No such file or directory"
```
6. compile the libraries
```bash
cd qt5
echo -e "QMAKE_LFLAGS += -static -static-libgcc\nDEFINES += QT_STATIC_BUILD\n" >> mkspecs/win32-g++/qmake.conf
./configure.bat -prefix $MINGW_PREFIX -extprefix $MINGW_PREFIX -bindir $MINGW_PREFIX/lib/qt5/bin -headerdir $MINGW_PREFIX/include/qt5 -libdir $MINGW_PREFIX/lib/qt5 -archdatadir $MINGW_PREFIX/lib/qt5 -plugindir $MINGW_PREFIX/lib/qt5/plugins -libexecdir $MINGW_PREFIX/lib/qt5/bin -datadir $MINGW_PREFIX/share/qt5 -docdir $MINGW_PREFIX/share/doc/qt5 -translationdir $MINGW_PREFIX/share/qt5/translations -sysconfdir $MINGW_PREFIX/etc/xdg -examplesdir $MINGW_PREFIX/share/qt5/examples -testsdir $MINGW_PREFIX/share/qt5/tests -platform win32-g++ -nomake examples -nomake tests -nomake tools -no-compile-examples -release -opensource -confirm-license -static -c++std c++11 -sse2 -static-runtime -make libs -no-ltcg -no-dbus -no-accessibility -no-inotify -no-iconv -no-icu -no-openssl -no-system-proxies -no-cups -no-fontconfig -opengl desktop -no-angle -gif -ico -qt-libpng -qt-libjpeg -qt-pcre -qt-zlib -qt-freetype
make
```
7. and finally install them
```bash
make install
sed -i -e s,Qt5OpenGLd,Qt5OpenGL,g -e s,Qt5OpenGLExtensionsd,Qt5OpenGLExtensions,g -e s,Qt5Concurrentd,Qt5Concurrent,g -e s,Qt5Cored,Qt5Core,g -e s,Qt5Guid,Qt5Gui,g -e s,Qt5Networkd,Qt5Network,g -e s,Qt5PrintSupportd,Qt5PrintSupport,g -e s,Qt5Sqld,Qt5Sql,g -e s,Qt5Testd,Qt5Test,g -e s,Qt5Widgetsd,Qt5Widgets,g -e s,Qt5Xmld,Qt5Xml,g -e s,libqtpcre2d,libqtpcre2,g -e s,libqtlibpngd,libqtlibpng,g -e s,libqtharfbuzzd,libqtharfbuzz,g $MINGW_PREFIX/lib/qt5/pkgconfig/*
cp -v $MINGW_PREFIX/lib/qt5/pkgconfig/* $MINGW_PREFIX/lib/pkgconfig/.
cd ..
```
8. now it's time for the SVG module...
```bash
wget http://download.qt.io/archive/qt/5.13/5.13.0/submodules/qtsvg-everywhere-src-5.13.0.zip
unzip qtsvg-everywhere-src-5.13.0.zip
mv qtsvg-everywhere-src-5.13.0 qt5svg
cd qt5svg
$MINGW_PREFIX/lib/qt5/bin/qmake
make
make install
sed -i -e s,Qt5Svgd,Qt5Svg,g -e s,Qt5Cored,Qt5Core,g -e s,Qt5Guid,Qt5Gui,g -e s,Qt5Widgetsd,Qt5Widgets,g $MINGW_PREFIX/lib/qt5/pkgconfig/*
cp -v $MINGW_PREFIX/lib/qt5/pkgconfig/* $MINGW_PREFIX/lib/pkgconfig/.
cd ..
```
9. ...and for the tools
```bash
wget http://download.qt.io/archive/qt/5.13/5.13.0/submodules/qttools-everywhere-src-5.13.0.zip
unzip qttools-everywhere-src-5.13.0.zip
mv qttools-everywhere-src-5.13.0 qt5tools
cd qt5tools
$MINGW_PREFIX/lib/qt5/bin/qmake
make
make install
cd ..
```
#### Compilation of puNES
10. Now you have everything you need to compile correctly puNES
```bash
git clone https://github.com/punesemu/punes
cd punes
./autogen.sh
```
if you want D3D9 version :
```bash
./configure --with-d3d9
make
```
otherwise :
```bash
./configure --with-opengl
make
```
The executable `punes.exe` is in the `src` directory but in order to run it you need the following dlls:
##### D3D9 version :
* 7z.dl
* cg.dll
* cgD3D9.dll
* libwinpthread-1.dll
##### OpenGL version :
* 7z.dll
* cg.dll
* cgGL.dll
* libwinpthread-1.dll

that you can download here : [`64bit`](https://www.dropbox.com/s/yt5bgacnwexdghs/puNES_x86_64_dlls.zip?dl=1) version or [`32bit`](https://www.dropbox.com/s/7afebuhjy06n9uh/puNES_i686_dlls.zip?dl=1) version.
#### Windows Debug version
If you need the debug version then you need to replace the `./configure` command of the previous examples with the following:
```bash
CFLAGS="-g -DDEBUG" CXXFLAGS="-g -DDEBUG" ./configure --disable-release [...]
```
where `[...]` are the other necessary options.
