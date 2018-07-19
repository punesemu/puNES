<h1>puNES</h1>

<span class="badge-paypal"><a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=QPPXNRL5NAHDC" title="Donate to this project using Paypal"><img src="https://img.shields.io/badge/paypal-donate-yellow.svg" alt="PayPal donate button" /></a></span>

Description
-----------

Nintendo Entertaiment System emulator

WIP
-----------
always updated to the last commit:
* Windows 32 bit : [SDL](https://www.dropbox.com/s/3b15hk5ad2mbepr/punes32.wip.sdl.zip?dl=0) - [D3D9](https://www.dropbox.com/s/avvmels3hi060zw/punes32.wip.d3d9.zip?dl=0)
* Windows 64 bit : [SDL](https://www.dropbox.com/s/jki4udcrwwq3hym/punes64.wip.sdl.zip?dl=0) - [D3D9](https://www.dropbox.com/s/29bzoqvru983ix6/punes64.wip.d3d9.zip?dl=0)
* Linux 32 bit : [SDL](https://www.dropbox.com/s/yt66qgzn2dukqj1/punes32?dl=0)
* Linux 64 bit : [SDL](https://www.dropbox.com/s/5n74roezzf5a2qy/punes64?dl=0)

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
the executable [`punes`] is in the `src` directory 

### Windows
-----------

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

#### Dependencies

4) open a new MSYS2 shell and download the necessary libraries
```bash
wget http://download.qt.io/archive/qt/4.8/4.8.7/qt-everywhere-opensource-src-4.8.7.zip
wget https://www.libsdl.org/release/SDL-1.2.15.zip
```

#### Compilation of the Qt4 libraries

5) unzip the sources
```bash
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

#### Compilation of the SDL libraries

8) with the SDL libraries it's a bit simpler
```bash
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
10) if you want compile D3D9 version :
```bash
./configure --with-d3d9
```
otherwise :
```bash
./configure --with-opengl
```
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

that you can find in the compressed files of the puNES releases.
