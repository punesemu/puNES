<p align="center">
  <img src="https://user-images.githubusercontent.com/14859058/132302943-a466d3d5-75c2-4bac-b0b2-7f0aeb8c058d.png" alt="puNES"/><br>
</p>
<h3 align="center">Qt-based Nintendo Entertainment System emulator and NSF/NSFe Music Player</h3>

<p align="center">
  <a href="https://repology.org/project/punes/versions">
    <img src="https://repology.org/badge/version-for-repo/gentoo/punes.svg" alt="Gentoo"/>
    <img src="https://repology.org/badge/version-for-repo/aur/punes.svg" alt="AUR]"/>
    <img src="https://repology.org/badge/version-for-repo/slackbuilds/punes.svg" alt="SlackBuilds"/>
    <img src="https://repology.org/badge/version-for-repo/nix_unstable/punes.svg" alt="Nixpkgs unstable"/>
    <img src="https://repology.org/badge/version-for-repo/wikidata/punes.svg" alt="Wikidata"/>
</p>

:floppy_disk: Work in Progress (WIP) Builds
-----------
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/punesemu/puNES?style=flat) [![CodeFactor](https://www.codefactor.io/repository/github/punesemu/punes/badge/master)](https://www.codefactor.io/repository/github/punesemu/punes/overview/master) [![Build status](https://github.com/punesemu/puNES/actions/workflows/build.yml/badge.svg)](https://github.com/punesemu/puNES/actions)

These executables are always updated to the latest commit:
* Windows 32 bit : :link:[`OpenGL`](https://nightly.link/punesemu/puNES/workflows/build/master/punes32.wip.opengl.zip) - :link:[`D3D9`](https://nightly.link/punesemu/puNES/workflows/build/master/punes32.wip.d3d9.zip)
* Windows 64 bit : :link:[`OpenGL`](https://nightly.link/punesemu/puNES/workflows/build/master/punes64.wip.opengl.zip) - :link:[`D3D9`](https://nightly.link/punesemu/puNES/workflows/build/master/punes64.wip.d3d9.zip)

Note : 32 bit versions are Windows XP compatible.

:beer: Support
-----------
If you want buy me a beer : <span class="badge-paypal"><a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=QPPXNRL5NAHDC" title="Donate to this project using Paypal"><img src="https://img.shields.io/badge/paypal-donate-yellow.svg" alt="PayPal donate button" /></a></span>

:camera: Screenshots
-----------
<p align="center">
  <img src="https://user-images.githubusercontent.com/14859058/153704977-bacb2e1a-7394-49f0-b29a-f1700d1e1991.png" width="400" alt="puNES main window"/>
  <img src="https://user-images.githubusercontent.com/14859058/142155318-22d86ae5-3a3f-486a-8ae8-e78c8eeabb2f.png" width="400" alt="puNES NSFE Player"/>
  <img src="https://user-images.githubusercontent.com/14859058/153705022-299874d6-e2f6-4a50-a394-a91cefe150a9.png" width="400" alt="puNES Slot Preview"/>
  <img src="https://user-images.githubusercontent.com/14859058/135748722-ea73e6b3-db6b-4cf2-b13d-755ca4824975.png" width="400" alt="puNES General Settings"/>
  <img src="https://user-images.githubusercontent.com/14859058/140854472-60a88023-e719-4637-9289-08991a367ddd.png" width="400" alt="puNES Video Filters Settings"/>
  <img src="https://user-images.githubusercontent.com/14859058/135748586-fe41d0d1-18d0-4c14-b7c4-4d6deacb4fbb.png" width="400" alt="puNES Cheat Editor"/>
  <img src="https://user-images.githubusercontent.com/14859058/146680023-2c40f991-67d2-4502-963f-39c8b5bfc4ca.png" width="400" alt="puNES Xbox360 Standard Controller Settings"/>
  <img src="https://user-images.githubusercontent.com/14859058/146680033-5fc36150-c52f-494a-ba5c-9d7d675e43d6.png" width="400" alt="puNES PS4 Standard Controller Settings"/>
</p>

:keyboard: Configuration
-----------
To run it in portable mode, rename the executable by adding the suffix `_p`.
Examples: `punes.exe -> punes_p.exe` or `punes64.exe -> punes64_p.exe`

To see a list of available command-line options, start puNES with the `-h` argument.

:electric_plug: Supported Mappers
-----------
|0   |1   |2   |3   |4   |5   |6   |7   |8   |9   |10  |
|----|----|----|----|----|----|----|----|----|----|----|
|11  |12  |13  |14  |15  |16  |17  |18  |19  |20  |21  |
|22  |23  |24  |25  |26  |27  |28  |29  |30  |31  |32  |
|33  |34  |35  |36  |37  |38  |    |40  |41  |42  |43  |
|44  |45  |46  |47  |48  |49  |50  |51  |52  |53  |    |
|55  |56  |57  |58  |59  |60  |61  |62  |63  |64  |65  |
|66  |67  |68  |69  |70  |71  |72  |73  |74  |75  |76  |
|77  |78  |79  |80  |81  |82  |83  |    |85  |86  |87  |
|88  |89  |90  |91  |92  |93  |94  |95  |96  |97  |    |
|99  |    |101 |    |103 |104 |105 |106 |107 |108 |    |
|    |111 |112 |113 |114 |115 |116 |117 |118 |119 |120 |
|121 |    |123 |    |125 |126 |    |    |    |    |    |
|132 |133 |134 |    |136 |137 |138 |139 |140 |141 |142 |
|143 |144 |145 |146 |147 |148 |149 |150 |151 |152 |153 |
|154 |155 |156 |    |158 |159 |    |    |162 |163 |164 |
|165 |166 |167 |168 |    |    |171 |172 |173 |    |175 |
|176 |177 |178 |179 |180 |    |182 |183 |184 |185 |186 |
|187 |188 |189 |190 |191 |192 |193 |194 |195 |196 |197 |
|198 |199 |200 |201 |202 |203 |204 |205 |206 |207 |208 |
|209 |210 |211 |212 |213 |214 |215 |216 |217 |218 |219 |
|    |221 |222 |    |224 |225 |226 |227 |228 |229 |230 |
|231 |232 |233 |234 |235 |236 |237 |238 |    |240 |241 |
|242 |243 |244 |245 |246 |    |248 |249 |250 |    |252 |
|253 |254 |255 |256 |    |258 |259 |260 |261 |262 |263 |
|264 |265 |266 |267 |268 |269 |    |271 |    |    |274 |
|    |    |    |    |    |    |281 |282 |283 |284 |285 |
|286 |287 |288 |289 |290 |    |292 |    |    |295 |    |
|297 |298 |299 |300 |301 |302 |303 |304 |305 |306 |307 |
|308 |309 |    |    |312 |313 |314 |315 |    |    |    |
|319 |320 |    |322 |323 |324 |325 |    |327 |328 |329 |
|    |331 |332 |333 |    |335 |336 |337 |338 |339 |340 |
|341 |342 |343 |344 |345 |346 |347 |348 |349 |350 |351 |
|352 |353 |    |355 |356 |357 |358 |359 |360 |361 |    |
|    |    |    |    |    |368 |369 |370 |    |372 |    |
|374 |375 |    |377 |    |    |380 |381 |382 |    |    |
|    |386 |387 |388 |389 |390 |    |    |393 |394 |395 |
|396 |397 |398 |399 |400 |401 |    |403 |404 |    |406 |
|    |    |409 |410 |411 |412 |413 |414 |415 |416 |417 |
|    |    |420 |    |422 |    |    |    |    |    |428 |
|429 |    |431 |432 |433 |434 |    |436 |437 |438 |    |
|    |    |442 |    |    |    |    |447 |    |    |    |
|    |452 |    |    |455 |456 |457 |    |    |    |    |
|    |    |    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |    |    |    |    |    |
|    |    |    |    |    |    |512 |513 |    |    |516 |
|    |518 |519 |    |521 |522 |    |524 |525 |526 |527 |
|528 |529 |530 |    |532 |    |534 |    |536 |537 |538 |
|539 |540 |541 |    |543 |    |    |    |547 |    |    |
|550 |    |    |    |    |    |556 |557 |558 |559 |560 |


:electric_plug: Unif
-----------
NROM, NROM-128, NROM-256, SLROM, UOROM, CNROM, TLROM, TBROM, TKROM, TFROM, ANROM, SL1632, SC-127, SuperHIK8in1, STREETFIGTER-GAME4IN1, Supervision16in1, MARIO1-MALEE2, T3H53, D1038, NTBROM, VRC7, TEK90, BB, H2288, LH32, 22211, SA-72008, Sachen-8259D, Sachen-8259B, Sachen-8259C, Sachen-8259A, KS7032, SA-NROM, SA-72007, SA-016-1M, TC-U01-1.5M, SA-0037, SA-0036, Sachen-74LS374N, FS304, FK23C, FK23CA, Super24in1SC03, WAIXING-FS005, NovelDiamond9999999in1, JC-016-2, 8237, 8237A, N625092, WAIXING-FW01, 42in1ResetSwitch, 70in1, 70in1B, 603-5052, OneBus, DANCE, 158B, F-15, HPXX, HP2018-A, 810544-C-A1, SHERO, KOF97, YOKO, T-262, CITYFIGHT, COOLBOY, MINDKIDS, 22026, 80013-B, GS-2004, GS-2013, A65AS, DRIPGAME, BS-5, 411120-C, K-3088, 60311C, NTD-03, DRAGONFIGHTER, TF1201, 11160, 190in1, 8157, KS7057, KS7017, SMB2J, KS7031, KS7016, KS7037, TH2131-1, LH51, KS7013B, RESET-TXROM, 64in1NoRepeat, 830134C, HP898F, 830425C-4391T, K-3033, FARID_SLROM_8-IN-1, FARID_UNROM_8-IN-1, MALISB, 10-24-C-A1, RT-01, EDU2000, 12-IN-1, WS, 8-IN-1, NEWSTAR-GRM070-8IN1, CTC-09, K-3046, CTC-12IN1, SA005-A, K-3006, K-3036, TJ-03, COOLGIRL, RESETNROM-XIN1, GN-26, L6IN1, KS7012, KS7030, 830118C, G-146, 891227, KS106C, 3D-BLOCK, SB-5013, N49C-300, 830752C, BS-400R, BS-4040R, K-3010, K-3071, SA-9602B, DANCE2000, EH8813A, DREAMTECH01, LH10, 900218, KS7021A, BJ-56, AX-40G, 831128C, T-230, AX5705, CHINA_ER_SAN2, 82112C, KONAMI-QTAI, Ghostbusters63in1, 43272, AC08, CC-21, BOY

:information_source: How to Compile
-----------

* :penguin: [Linux](#penguin-linux)
* :smiling_imp: [FreeBSD](#smiling_imp-freebsd)
* :blowfish: [OpenBSD](#blowfish-openbsd)
* :computer: [Windows](#computer-windows)

## :penguin: Linux
#### Dependencies
* [Qt5](https://www.qt.io) with OpenGL support (qtcore, qtgui, qtwidgets, qtnetwork, qtsvg and qttools)
* [nvidia-cg](https://developer.nvidia.com/cg-toolkit)
* [alsa](https://www.alsa-project.org)
* [cmake >= 3.13](https://cmake.org)
* libudev
* [libX11 and libXrandr](https://www.x.org)
* (optional) [ffmpeg libraries >= 4.0](https://ffmpeg.org) if you want video and audio recording support (libavcodec, libavformat, libavutil, libswresample and libswscale). See [notes](#movie_camera-ffmpeg).
#### Compilation of puNES
```bash
git clone https://github.com/punesemu/puNES
cd puNES
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
#### Example on how to compile on Ubuntu 21.10
```bash
sudo apt-get install git cmake autotools-dev autoconf libtool build-essential pkg-config libudev-dev libasound2-dev
sudo apt-get install qtbase5-dev qttools5-dev-tools libqt5svg5-dev nvidia-cg-toolkit libx11-dev libxrandr-dev 
sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswresample-dev libswscale-dev
git clone https://github.com/punesemu/puNES
cd puNES
./autogen.sh
./configure
make
```
to start the emulator
```bash
./src/punes
```
## :smiling_imp: FreeBSD
#### Dependencies
* [Qt5](https://www.qt.io) with OpenGL support (qtcore, qtgui, qtwidgets, qtnetwork, qtsvg and qttools)
* [sndio](http://www.sndio.org)
* [cmake >= 3.13](https://cmake.org)
* [libX11 and libXrandr](https://www.x.org)
* (optional) [ffmpeg libraries >= 4.0](https://ffmpeg.org) if you want video and audio recording support (libavcodec, libavformat, libavutil, libswresample and libswscale). See [notes](#movie_camera-ffmpeg).
#### Compilation of puNES
```bash
git clone https://github.com/punesemu/puNES
cd puNES
./autogen.sh
CC=cc CXX=c++ ./configure
make
```
the executable `punes` is in the `src` directory.
#### FreeBSD Debug version
If you need the debug version then you need to replace the `./configure` command of the previous examples with the following:
```bash
CFLAGS="-g -DDEBUG" CXXFLAGS="-g -DDEBUG" CC=cc CXX=c++ ./configure --disable-release [...]
```
where `[...]` are the other necessary options.
## :blowfish: OpenBSD 
#### Dependencies
* [Qt5](https://www.qt.io) with OpenGL support (qtcore, qtgui, qtwidgets, qtnetwork, qtsvg and qttools)
* [sndio](http://www.sndio.org)
* [cmake >= 3.13](https://cmake.org)
* [libX11 and libXrandr](https://www.x.org)
* (optional) [ffmpeg libraries >= 4.0](https://ffmpeg.org) if you want video and audio recording support (libavcodec, libavformat, libavutil, libswresample and libswscale). See [notes](#movie_camera-ffmpeg).
#### Compilation of puNES
```bash
git clone https://github.com/punesemu/puNES
cd puNES
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
## :computer: Windows
#### Dependencies
* [Qt5](https://www.qt.io) with OpenGL support (5.6.3 is the last if you want the support for Windows XP)
#### Development Environment installation
1. install [MSYS2](https://www.msys2.org/)
2. open "MSYS2 MinGW 64-bit" shell (or 32 bit if you want compile the 32 bit version of puNES)
```bash
pacman -Syu
```
3. close the MSYS2 window and run it again from Start menu
```bash
pacman -Su
pacman -S base-devel git wget p7zip unzip mingw-w64-i686-cmake mingw-w64-x86_64-cmake
pacman -S perl ruby python2 mingw-w64-i686-toolchain mingw-w64-x86_64-toolchain
exit
```
4. open a new MSYS2 shell and build the necessary libraries
#### Compilation of the Qt5 libraries
5. download and unzip the sources
```bash
wget http://download.qt.io/archive/qt/5.15/5.15.0/submodules/qtbase-everywhere-src-5.15.0.zip
unzip qtbase-everywhere-src-5.15.0.zip
mv qtbase-everywhere-src-5.15.0 qt5
```
the renaming of the directory is necessary to not generate a compile-time error caused by the 255 characters maximum path length limitation on Windows, This is the typical error message you might encounter:
```code
"../../../../include/QtEventDispatcherSupport/5.15.0/QtEventDispatcherSupport/private/qwindowsguieventdispatcher_p.h:1:10: fatal error: ../../../../../src/platformsupport/eventdispatchers/qwindowsguieventdispatcher_p.h: No such file or directory"
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
wget http://download.qt.io/archive/qt/5.15/5.15.0/submodules/qtsvg-everywhere-src-5.15.0.zip
unzip qtsvg-everywhere-src-5.15.0.zip
mv qtsvg-everywhere-src-5.15.0 qt5svg
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
wget http://download.qt.io/archive/qt/5.15/5.15.0/submodules/qttools-everywhere-src-5.15.0.zip
unzip qttools-everywhere-src-5.15.0.zip
mv qttools-everywhere-src-5.15.0 qt5tools
cd qt5tools
$MINGW_PREFIX/lib/qt5/bin/qmake
make
make install
cd ..
```
#### Compilation of puNES
10. Now you have everything you need to compile correctly puNES
```bash
git clone https://github.com/punesemu/puNES
cd puNES
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
* 7z.dl
* avcodec-58.dll
* avformat-58.dll
* avutil-56.dll
* cg.dll
* cgD3D9.dll (only for D3D9 version)
* cgGL.dll (only for OpenGL version)
* libwinpthread-1.dll
* swresample-3.dll
* swscale-5.dll

that you can download here : :link:[`64bit`](https://www.dropbox.com/s/d632cjezybz6a74/puNES_x86_64_dlls.zip?dl=1) version or :link:[`32bit`](https://www.dropbox.com/s/ye00129nyacdl05/puNES_i686_dlls.zip?dl=1) version.
#### Windows Debug version
If you need the debug version then you need to replace the `./configure` command of the previous examples with the following:
```bash
CFLAGS="-g -DDEBUG" CXXFLAGS="-g -DDEBUG" ./configure --disable-release [...]
```
where `[...]` are the other necessary options.

-----------

#### :movie_camera: FFmpeg
It is always possible to disable audio/video recording support by specifying the `configure` parameter `--without-ffmpeg`.
If the installed version is lower than 4.0 the support will be disabled automatically.

Supported audio recording formats:
* WAV Audio
* MP3 Audio ([lame](https://xiph.org/vorbis/)) (*)
* AAC Audio
* Flac Audio
* Ogg Audio ([vorbis](https://xiph.org/vorbis/)) (*)
* Opus Audio ([libopus](https://www.opus-codec.org)) (*)

Supported video recording formats:
* MPEG 1 Video
* MPEG 2 Video
* MPEG 4 Video
* MPEG H264 Video ([libx264](https://www.videolan.org/developers/x264.html)) (*)
* High Efficiency Video Codec ([libx265](https://www.videolan.org/developers/x265.html)) (*)
* WebM Video ([libvpx](https://www.webmproject.org/code)) (*)
* Windows Media Video
* AVI FF Video
* AVI Video

(*) if compiled in FFmpeg.
