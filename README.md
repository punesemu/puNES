<h1>puNES</h1>

<span class="badge-paypal"><a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=QPPXNRL5NAHDC" title="Donate to this project using Paypal"><img src="https://img.shields.io/badge/paypal-donate-yellow.svg" alt="PayPal donate button" /></a></span>

Description
-----------

Nintendo Entertaiment System emulator

WIP
-----------
always updated to the last commit:
* Windows 32 bit : [`SDL`](https://www.dropbox.com/s/3b15hk5ad2mbepr/punes32.wip.sdl.zip?dl=0) - [`D3D9`](https://www.dropbox.com/s/avvmels3hi060zw/punes32.wip.d3d9.zip?dl=0)
* Windows 64 bit : [`SDL`](https://www.dropbox.com/s/jki4udcrwwq3hym/punes64.wip.sdl.zip?dl=0) - [`D3D9`](https://www.dropbox.com/s/29bzoqvru983ix6/punes64.wip.d3d9.zip?dl=0)

How to Compile
-----------

### Linux
-----------
#### Dependencies
* Qt4 or Qt5
* nvidia-cg
* SDL 1.xx
* alsa
#### Compilation of puNES
```bash
git clone https://github.com/punesemu/punes
cd punes
./autogen.sh
```
if you want compile Qt4 version :
```bash
./configure
make
```
otherwise :
```bash
./configure --enable-qt5
make
```
the executable `punes` is in the `src` directory.
#### Linux Debug version
If you need the debug version then you need to replace the `./configure` command of the previous examples with the following:
```bash
CFLAGS="-g -DDEBUG" CXXFLAGS="-g -DDEBUG" ./configure --disable-release [...]
```
where `[...]` are the other necessary options listed above.
### Windows
-----------
#### Dependencies
* Qt4 (Windows XP and above) or Qt5 (Windows 7 and above)
* SDL 1.xx
#### Development Environment installation
1) install MSYS2 (https://www.msys2.org/)
2) open "MSYS2 MinGW 64-bit" shell (or 32 bit if you want compile the 32 bit version of puNES)
```bash
pacman -Syu
```
 3) close the MSYS2 window and run it again from Start menu
```bash
pacman -Su
pacman -S base-devel git wget p7zip unzip
pacman -S perl ruby python2 mingw-w64-i686-toolchain mingw-w64-x86_64-toolchain
exit
```
4) open a new MSYS2 shell and download the necessary libraries
#### Compilation of the Qt4 libraries
5) download and unzip the sources
```bash
wget http://download.qt.io/archive/qt/4.8/4.8.7/qt-everywhere-opensource-src-4.8.7.zip
unzip -o qt-everywhere-opensource-src-4.8.7.zip
unzip qt-everywhere-opensource-src-4.8.7.zip qt-everywhere-opensource-src-4.8.7/configure.exe
```
6) compile the libraries
```bash
cd qt-everywhere-opensource-src-4.8.7
./configure.exe -prefix $MINGW_PREFIX -libdir $MINGW_PREFIX/lib/qt4 -plugindir $MINGW_PREFIX/lib/qt4/plugins -importdir $MINGW_PREFIX/lib/qt4/imports -bindir $MINGW_PREFIX/lib/qt4/bin -headerdir $MINGW_PREFIX/include/qt4 -datadir $MINGW_PREFIX/share/qt4 -translationdir $MINGW_PREFIX/share/qt4/translations -examplesdir $MINGW_PREFIX/share/qt4/examples -demosdir $MINGW_PREFIX/share/qt4/demos -docdir $MINGW_PREFIX/share/doc/qt4 -platform win32-g++ -nomake docs -nomake demos -nomake examples -nomake tests -release -opensource -confirm-license -static -no-ltcg -fast -exceptions -no-accessibility -no-stl -no-qt3support -no-opengl -no-openvg -no-nis -no-neon -iconv -no-inotify -largefile -no-fontconfig -no-system-proxies -qt-zlib -qt-libpng -qt-libmng -qt-libtiff -qt-libjpeg -no-dsp -no-vcproj -no-incredibuild-xge -mmx -3dnow -sse -sse2 -no-openssl -no-dbus -no-phonon -no-phonon-backend -no-multimedia -no-audio-backend -no-webkit -no-script -no-scripttools -no-declarative -no-directwrite -no-native-gestures -no-mp -no-cups -no-declarative -no-xmlpatterns
QMAKEPATH=$(pwd) make
```
7) and finally install them
```bash
sed -i -e 's/-l.*..\\..\\lib\\pkgconfig\\/-l/g' lib/pkgconfig/*
make install
cp -v $MINGW_PREFIX/lib/qt4/pkgconfig/* $MINGW_PREFIX/lib/pkgconfig/.
cd ..
```
#### Compilation of the Qt5 libraries
5) download and unzip the sources
```bash
wget http://download.qt.io/archive/qt/5.11/5.11.1/submodules/qtbase-everywhere-src-5.11.1.zip
unzip qtbase-everywhere-src-5.11.1.zip
mv qtbase-everywhere-src-5.11.1 qt5
```
the renaming of the directory is necessary to not generate a compile-time error caused by the 255 characters maximum path length limitation on Windows, This is the typical error message you might encounter:
```code
"../../../../include/QtEventDispatcherSupport/5.11.1/QtEventDispatcherSupport/private/qwindowsguieventdispatcher_p.h:1:10: fatal error: ../../../../../src/platformsupport/eventdispatchers/qwindowsguieventdispatcher_p.h: No such file or directory"
```
6) compile the libraries
```bash
cd qt5
echo -e "QMAKE_LFLAGS += -static -static-libgcc\nDEFINES += QT_STATIC_BUILD\n" >> mkspecs/win32-g++/qmake.conf
./configure.bat -prefix $MINGW_PREFIX -extprefix $MINGW_PREFIX -bindir $MINGW_PREFIX/lib/qt5/bin -headerdir $MINGW_PREFIX/include/qt5 -libdir $MINGW_PREFIX/lib/qt5 -archdatadir $MINGW_PREFIX/lib/qt5 -plugindir $MINGW_PREFIX/lib/qt5/plugins -libexecdir $MINGW_PREFIX/lib/qt5/bin -datadir $MINGW_PREFIX/share/qt5 -docdir $MINGW_PREFIX/share/doc/qt5 -translationdir $MINGW_PREFIX/share/qt5/translations -sysconfdir $MINGW_PREFIX/etc/xdg -examplesdir $MINGW_PREFIX/share/qt5/examples -testsdir $MINGW_PREFIX/share/qt5/tests -platform win32-g++ -nomake examples -nomake tests -nomake tools -no-compile-examples -release -opensource -confirm-license -static -c++std c++11 -sse2 -static-runtime -make libs -no-ltcg -no-dbus -no-accessibility -no-inotify -no-iconv -no-icu -no-openssl -no-system-proxies -no-cups -no-fontconfig -no-opengl -no-angle -gif -ico -qt-libpng -qt-libjpeg -qt-pcre -qt-zlib -qt-freetype
make
```
7) and finally install them
```bash
make install
sed -i -e s,Qt5Concurrentd,Qt5Concurrent,g -e s,Qt5Cored,Qt5Core,g -e s,Qt5Guid,Qt5Gui,g -e s,Qt5Networkd,Qt5Network,g -e s,Qt5PrintSupportd,Qt5PrintSupport,g -e s,Qt5Sqld,Qt5Sql,g -e s,Qt5Testd,Qt5Test,g -e s,Qt5Widgetsd,Qt5Widgets,g -e s,Qt5Xmld,Qt5Xml,g Qt5Gui.pc -e s,libqtpcre2d,libqtpcre2,g -e s,libqtlibpngd,libqtlibpng,g -e s,libqtharfbuzzd,libqtharfbuzz,g $MINGW_PREFIX/lib/qt5/pkgconfig/*
cp -v $MINGW_PREFIX/lib/qt5/pkgconfig/* $MINGW_PREFIX/lib/pkgconfig/.
cd ..
```
#### Compilation of the SDL libraries
8) with the SDL libraries it's a bit simpler
```bash
wget https://www.libsdl.org/release/SDL-1.2.15.zip
unzip SDL-1.2.15.zip
cd SDL-1.2.15
./configure --disable-static
make
make install
cd ..
```
#### Compilation of puNES
9) Now you have everything you need to compile correctly puNES
```bash
git clone https://github.com/punesemu/punes
cd punes
./autogen.sh
```
10) if you use Qt5 :
```bash
./configure --with-d3d9 --enable-qt5
make
```
or
```bash
./configure --with-opengl --enable-qt5
make
```
and if you use Qt4 libraries don't use the option `--enable-qt5`

the executable `punes.exe` is in the `src` directory but in order to run it you need the following dlls:
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
* SDL.dll

that you can download here : [`64bit`](https://www.dropbox.com/s/yt5bgacnwexdghs/puNES_x86_64_dlls.zip?dl=0) version or [`32bit`](https://www.dropbox.com/s/7afebuhjy06n9uh/puNES_i686_dlls.zip?dl=0) version.
#### Windows Debug version
If you need the debug version then you need to replace the `./configure` command of the previous examples with the following:
```bash
CFLAGS="-g -DDEBUG" CXXFLAGS="-g -DDEBUG" ./configure --disable-release [...]
```
where `[...]` are the other necessary options listed above.
