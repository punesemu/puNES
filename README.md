<p align="center">
  <img src="https://user-images.githubusercontent.com/14859058/132302943-a466d3d5-75c2-4bac-b0b2-7f0aeb8c058d.png" alt="puNES"/><br>
</p>
<h3 align="center">Qt-based Nintendo Entertainment System emulator and NSF/NSF2/NSFe Music Player</h3>

<p align="center">
  <a href="https://github.com/punesemu/puNES/releases/latest">
    <img src="https://img.shields.io/github/release/punesemu/puNES.svg?label=latest%20release" alt="GitHub release"/>
  </a>
  <a href="https://github.com/punesemu/puNES/blob/master/COPYING">
    <img src="https://img.shields.io/github/license/punesemu/puNES.svg" alt="License"/>
  </a>
  <a href="https://crowdin.com/project/punes">
    <img src="https://badges.crowdin.net/punes/localized.svg" alt="Crowdin"/>
  </a>
  <a href="https://github.com/punesemu/puNES">
    <img src="https://img.shields.io/github/languages/code-size/punesemu/puNES?style=flat" alt="GitHub code size in bytes"/>
  </a>
  <a href="https://www.codefactor.io/repository/github/punesemu/punes/overview/master">
    <img src="https://www.codefactor.io/repository/github/punesemu/punes/badge/master" alt="CodeFactor"/>
  </a>
  <a href="https://repology.org/project/punes/versions">
    <img src="https://repology.org/badge/tiny-repos/punes.svg" alt="Packaging status"/>
  </a>
</p>
<p align="center">
  <a href='https://flathub.org/apps/details/io.github.punesemu.puNES'>
    <img width='180' height='60' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-en.svg'/>
  </a>
</p>

## :floppy_disk: Work in Progress (WIP) Builds [![Build status](https://github.com/punesemu/puNES/actions/workflows/build.yml/badge.svg)](https://github.com/punesemu/puNES/actions)

These executables are always updated to the latest commit:

- Linux AppImage : :link:[`x86_64`](https://nightly.link/punesemu/puNES/workflows/build/master/puNES-x86_64.AppImage.zip)
- Windows 32 bit : :link:[`OpenGL`](https://nightly.link/punesemu/puNES/workflows/build/master/punes32.wip.opengl.zip) - :link:[`D3D9`](https://nightly.link/punesemu/puNES/workflows/build/master/punes32.wip.d3d9.zip)
- Windows 64 bit : :link:[`OpenGL`](https://nightly.link/punesemu/puNES/workflows/build/master/punes64.wip.opengl.zip) - :link:[`D3D9`](https://nightly.link/punesemu/puNES/workflows/build/master/punes64.wip.d3d9.zip)

Notes:

- WARNING save states of version 0.110 or earlier are no longer compatible.
- Due to the many changes I'm making to the core of the emulator, new bugs may have been introduced, if you find that the roms no longer work properly compared to version 0.110, please let me know.
- 32 bit versions are Windows XP compatible.

## :beer: Support

If you want buy me a beer :

[![GitHub Sponsors](https://img.shields.io/badge/GitHub-Donate-EA4AAA?style=for-the-badge&logo=githubsponsors)](https://github.com/sponsors/punesemu)
[![PayPal](https://img.shields.io/badge/PayPal-Donate-blue?style=for-the-badge&logo=paypal)](https://paypal.me/punesemu)
[![kofi](https://img.shields.io/badge/Ko--Fi-Donate-orange?style=for-the-badge&logo=ko-fi)](https://ko-fi.com/punesemu)

## Multilingual Support

A big thank you to everyone who contributed to the translations:

- Arabic
- Chinese
- English
- French
- German
- Hungarian
- Italian
- Polish
- Portuguese
- Russian
- Spanish
- Turkish

### Help with Translations [here](https://crowdin.com/project/punes)

## :camera: Screenshots

<p align="center">
  <img src="https://github.com/user-attachments/assets/801b85b5-78e7-47b7-9430-4b0904f876a8" width="400" alt="puNES main window"/>
  <img src="https://github.com/user-attachments/assets/a05a88de-1feb-41e7-a531-13a4e6ba6937" width="400" alt="puNES NSF2 Player"/>
  <img src="https://github.com/user-attachments/assets/9e3b034d-9e54-42cc-9cd6-26842ebb431b" width="400" alt="puNES Slot Preview"/>
  <img src="https://github.com/user-attachments/assets/3a1a0a82-d3b5-4213-9f73-ec28461ce188" width="400" alt="puNES General Settings"/>
  <img src="https://github.com/user-attachments/assets/eb135545-a3d4-4ca1-91f9-4b49aafe3cef" width="400" alt="puNES Video Filters Settings"/>
  <img src="https://github.com/user-attachments/assets/9358391c-4a16-4eff-9610-d56cc18c647b" width="400" alt="puNES Cheat Editor"/>
  <img src="https://github.com/user-attachments/assets/61a152b9-3d82-46a5-8b85-322d0a908ade" width="400" alt="puNES Xbox360 Standard Controller Settings"/>
  <img src="https://github.com/user-attachments/assets/d4c9dccb-1394-44fe-9f7e-8736b4c9f548" width="400" alt="puNES PS4 Standard Controller Settings"/>
  <img src="https://github.com/user-attachments/assets/3f7e4550-a9cf-4319-83e5-a126cb079b37" width="800" alt="puNES Family BASIC Virtual Keyboard"/>
  <img src="https://github.com/user-attachments/assets/4df74b2a-7889-4cb5-948e-66360fd64707" width="800" alt="puNES Subor Virtual Keyboard"/>
</p>

## :keyboard: Configuration

To run in portable mode there is 3 distinct ways:

1. If the executable is in a folder containing the puNES.cfg file.
2. Rename the executable by adding the suffix `_p`.
   - Examples: `punes.exe -> punes_p.exe` or `punes64.exe -> punes64_p.exe`
3. Run the emulator with the "--portable" option.

To see a list of available command-line options, start puNES with the `-h` argument.

## :electric_plug: Supported Mappers

| 0   | 1   | 2   | 3   | 4   | 5   | 6   | 7   | 8   | 9   | 10  |
|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| 11  | 12  | 13  | 14  | 15  | 16  | 17  | 18  | 19  | 20  | 21  |
| 22  | 23  | 24  | 25  | 26  | 27  | 28  | 29  | 30  | 31  | 32  |
| 33  | 34  | 35  | 36  | 37  | 38  |     | 40  | 41  | 42  | 43  |
| 44  | 45  | 46  | 47  | 48  | 49  | 50  | 51  | 52  | 53  |     |
| 55  | 56  | 57  | 58  | 59  | 60  | 61  | 62  | 63  | 64  | 65  |
| 66  | 67  | 68  | 69  | 70  | 71  | 72  | 73  | 74  | 75  | 76  |
| 77  | 78  | 79  | 80  | 81  | 82  | 83  |     | 85  | 86  | 87  |
| 88  | 89  | 90  | 91  | 92  | 93  | 94  | 95  | 96  | 97  |     |
| 99  | 100 | 101 |     | 103 | 104 | 105 | 106 | 107 | 108 |     |
|     | 111 | 112 | 113 | 114 | 115 | 116 | 117 | 118 | 119 | 120 |
| 121 | 122 | 123 |     | 125 | 126 |     |     |     |     |     |
| 132 | 133 | 134 |     | 136 | 137 | 138 | 139 | 140 | 141 | 142 |
| 143 | 144 | 145 | 146 | 147 | 148 | 149 | 150 | 151 | 152 | 153 |
| 154 | 155 | 156 | 157 | 158 | 159 |     |     | 162 | 163 | 164 |
| 165 | 166 | 167 | 168 |     | 170 | 171 | 172 | 173 |     | 175 |
| 176 | 177 | 178 | 179 | 180 |     | 182 | 183 | 184 | 185 | 186 |
| 187 | 188 | 189 | 190 | 191 | 192 | 193 | 194 | 195 | 196 | 197 |
| 198 | 199 | 200 | 201 | 202 | 203 | 204 | 205 | 206 | 207 | 208 |
| 209 | 210 | 211 | 212 | 213 | 214 | 215 | 216 | 217 | 218 | 219 |
|     | 221 | 222 |     | 224 | 225 | 226 | 227 | 228 | 229 | 230 |
| 231 | 232 | 233 | 234 | 235 | 236 | 237 | 238 |     | 240 | 241 |
| 242 | 243 | 244 | 245 | 246 |     | 248 | 249 | 250 |     | 252 |
| 253 | 254 | 255 | 256 |     | 258 | 259 | 260 | 261 | 262 | 263 |
| 264 | 265 | 266 | 267 | 268 | 269 |     | 271 | 272 |     | 274 |
|     |     |     |     |     |     | 281 | 282 | 283 | 284 | 285 |
| 286 | 287 | 288 | 289 | 290 | 291 | 292 |     |     | 295 |     |
| 297 | 298 | 299 | 300 | 301 | 302 | 303 | 304 | 305 | 306 | 307 |
| 308 | 309 |     | 311 | 312 | 313 | 314 | 315 |     |     |     |
| 319 | 320 |     | 322 | 323 | 324 | 325 |     | 327 | 328 | 329 |
|     | 331 | 332 | 333 | 334 | 335 | 336 | 337 | 338 | 339 | 340 |
| 341 | 342 | 343 | 344 | 345 | 346 | 347 | 348 | 349 | 350 | 351 |
| 352 | 353 | 354 | 355 | 356 | 357 | 358 | 359 | 360 | 361 | 362 |
|     |     |     | 366 |     | 368 | 369 | 370 |     | 372 |     |
| 374 | 375 |     | 377 |     |     | 380 | 381 | 382 |     | 384 |
|     | 386 | 387 | 388 | 389 | 390 |     |     | 393 | 394 | 395 |
| 396 | 397 | 398 | 399 | 400 | 401 |     | 403 | 404 |     | 406 |
|     |     | 409 | 410 | 411 | 412 | 413 | 414 | 415 | 416 | 417 |
|     |     | 420 | 421 | 422 |     |     |     |     |     | 428 |
| 429 |     | 431 | 432 | 433 | 434 |     | 436 | 437 | 438 |     |
|     |     | 442 |     |     |     | 446 | 447 |     |     |     |
| 451 | 452 |     |     | 455 | 456 | 457 |     |     |     |     |
|     |     |     |     |     |     |     |     |     | 471 |     |
|     |     |     |     |     |     |     |     | 481 |     |     |
|     |     |     |     |     |     |     |     |     |     |     |
|     |     |     |     |     |     |     |     |     |     |     |
|     |     |     |     |     |     | 512 | 513 |     |     | 516 |
| 517 | 518 | 519 |     | 521 | 522 |     | 524 | 525 | 526 | 527 |
| 528 | 529 | 530 |     | 532 |     | 534 |     | 536 | 537 | 538 |
| 539 | 540 | 541 |     | 543 |     |     |     | 547 |     |     |
| 550 | 551 | 552 |     | 554 | 555 | 556 | 557 | 558 | 559 | 560 |
| 561 | 562 |     | 564 |     |     |     |     |     |     |     |

## :electric_plug: UNIF boards

1. 3D-BLOCK
2. 8-IN-1
3. 10-24-C-A1
4. 12-IN-1
5. 13in1JY110
6. 42in1ResetSwitch
7. 64in1NoRepeat
8. 70in1
9. 70in1B
10. 150in1A
11. 158B
12. 190in1
13. 212-HONG-KONG
14. 603-5052
15. 8157
16. 8237
17. 8237A
18. 11160
19. 22026
20. 22211
21. 43272
22. 60311C
23. 80013-B
24. 82112C
25. 411120-C
26. 810544-C-A1
27. 820561C
28. 830118C
29. 830134C
30. 830425C-4391T
31. 830752C
32. 831128C
33. 891227
34. 900218
35. A60AS
36. A65AS
37. AC08
38. AMROM
39. ANROM
40. AOROM
41. AX5705
42. AX-40G
43. BB
44. BJ-56
45. BOY
46. BS-5
47. BS-400R
48. BS-4040R
49. CC-21
50. CHINA_ER_SAN2
51. CITYFIGHT
52. CNROM
53. COOLBOY
54. COOLGIRL
55. CTC-09
56. CTC-12IN1
57. D1038
58. DANCE
59. DANCE2000
60. DRAGONFIGHTER
61. DREAMTECH01
62. DRIPGAME
63. EDU2000
64. EH8813A
65. F-15
66. FARID_SLROM_8-IN-1
67. FARID_UNROM_8-IN-1
68. FC-28-5027
69. FK23C
70. FK23CA
71. FS304
72. G-146
73. Ghostbusters63in1
74. GKCXIN1
75. GN-26
76. GS-2004
77. GS-2013
78. H2288
79. HP898F
80. HP2018-A
81. HPXX
82. JC-016-2
83. K-3006
84. K-3010
85. K-3033
86. K-3036
87. K-3046
88. K-3071
89. K-3088
90. KOF97
91. KONAMI-QTAI
92. KS106C
93. KS7012
94. KS7013B
95. KS7016
96. KS7017
97. KS7021A
98. KS7030
99. KS7031
100. KS7032
101. KS7037
102. KS7057
103. L6IN1
104. LH09
105. LH10
106. LH32
107. LH51
108. M2C52A
109. MALISB
110. MARIO1-MALEE2
111. MINDKIDS
112. N49C-300
113. N625092
114. NEWSTAR-GRM070-8IN1
115. NovelDiamond9999999in1
116. NROM
117. NROM-128
118. NROM-256
119. NTBROM
120. NTD-03
121. OneBus
122. RESET-TXROM
123. RESETNROM-XIN1
124. RT-01
125. S-2009
126. SA005-A
127. SA-0036
128. SA-0037
129. SA-016-1M
130. SA-9602B
131. SA-72007
132. SA-72008
133. SA-NROM
134. Sachen-74LS374N
135. Sachen-8259A
136. Sachen-8259B
137. Sachen-8259C
138. Sachen-8259D
139. SB-5013
140. SC-127
141. SHERO
142. SL1632
143. SLROM
144. SMB2J
145. STREETFIGTER-GAME4IN1
146. Super24in1SC03
147. SuperHIK8in1
148. Supervision16in1
149. T3H53
150. T4A54A
151. T-230
152. T-262
153. TBROM
154. TC-U01-1.5M
155. TEK90
156. TF1201
157. TFROM
158. TH2131-1
159. TJ-03
160. TKROM
161. TLROM
162. Transformer
163. UNROM
164. UOROM
165. VRC7
166. WAIXING-FS005
167. WAIXING-FW01
168. WS
169. YOKO

## :information_source: How to Compile

- :penguin: [Linux](#penguin-linux)
- :smiling_imp: [FreeBSD](#smiling_imp-freebsd)
- :blowfish: [OpenBSD](#blowfish-openbsd)
- :computer: [Windows](#computer-windows)

## CMake Options

| CMake Option              | Description                                                                        | Default |
| ------------------------- | ---------------------------------------------------------------------------------- | ------- |
| ENABLE_RELEASE            | Build release version                                                              | ON      |
| ENABLE_FFMPEG             | Enable FFMPEG support                                                              | ON      |
| ENABLE_OPENGL             | Use OpenGL support instead of Direct3D 9 (only for Windows)                        | ON      |
| ENABLE_OPENGL_CG          | Enable OpenGL nVidia Cg Toolkit support                                            | OFF     |
| ENABLE_FULLSCREEN_RESFREQ | Enable Fullscreen resolution and auto frequency                                    | ON      |
| ENABLE_QT6_LIBS           | Enable support for QT6 libraries                                                   | OFF     |
| DISABLE_PORTABLE_MODE     | Disable portable mode handling (useful with sandbox<br/>environments like Flatpak) | OFF     |

## :penguin: Linux

<details>
<summary>Expand</summary>

#### Dependencies

- [CMake >= 3.14](https://cmake.org) ([Ninja](https://ninja-build.org) build system is optional)
- [Qt5](https://www.qt.io) or [Qt6](https://www.qt.io) with OpenGL support (qtcore, qtgui, qtwidgets, qtnetwork, qtsvg and qttools)
- [nvidia-cg](https://developer.nvidia.com/cg-toolkit)
- [alsa](https://www.alsa-project.org)
- libudev
- [libX11 and libXrandr](https://www.x.org)
- [p7zip](https://github.com/p7zip-project/p7zip) for compressed file support (lib7zip uses the 7z.so library on unix-like systems)
- (optional) [ffmpeg libraries >= 4.0](https://ffmpeg.org) if you want video and audio recording support (libavcodec, libavformat, libavutil, libswresample and libswscale). See [notes](#movie_camera-ffmpeg).

#### Compilation of puNES

```bash
git clone https://github.com/punesemu/puNES
cd puNES
cmake -B build -G Ninja -DENABLE_FFMPEG:BOOL=ON -DENABLE_OPENGL_CG:BOOL=ON
cmake --build build -j2
```

if you don't want to use the Ninja build system and prefer the classic Makefile:

```bash
cmake -B build -DENABLE_FFMPEG:BOOL=ON -DENABLE_OPENGL_CG:BOOL=ON
make -j2
```

the executable `punes` is in the `build/src` directory.

#### Linux Debug version

If you need the debug version then you need to replace the `cmake -B build -G Ninja` command of the previous examples with the following:

```bash
cmake -B build -G Ninja -DCMAKE_C_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_CXX_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_RELEASE:BOL=OFF [...]
cmake --build build -j2
```

or if you prefer the classic Makefile:

```bash
cmake -B build -DCMAKE_C_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_CXX_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_RELEASE:BOL=OFF [...]
make -j2
```

where `[...]` are the other necessary options.

#### Example on how to compile on Ubuntu 22.04

```bash
sudo apt-get install git cmake ninja-build libtool build-essential pkg-config libudev-dev libasound2-dev
sudo apt-get install qtbase5-dev qttools5-dev qttools5-dev-tools libqt5svg5-dev nvidia-cg-toolkit libx11-dev libxrandr-dev
sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libswresample-dev libswscale-dev p7zip-full
git clone https://github.com/punesemu/puNES
cd puNES
cmake -B build -G Ninja
cmake --build build -j2
```

to start the emulator

```bash
./build/src/punes
```

</details>

## :smiling_imp: FreeBSD

<details>
<summary>Expand</summary>

#### Dependencies

- [CMake >= 3.14](https://cmake.org) ([Ninja](https://ninja-build.org) build system is optional)
- [Qt5](https://www.qt.io) or [Qt6](https://www.qt.io) with OpenGL support (qtcore, qtgui, qtwidgets, qtnetwork, qtsvg and qttools)
- [sndio](http://www.sndio.org)
- [libX11 and libXrandr](https://www.x.org)
- [p7zip](https://github.com/p7zip-project/p7zip) for compressed file support (lib7zip uses the 7z.so library on unix-like systems)
- (optional) [ffmpeg libraries >= 4.0](https://ffmpeg.org) if you want video and audio recording support (libavcodec, libavformat, libavutil, libswresample and libswscale). See [notes](#movie_camera-ffmpeg).

#### Compilation of puNES

```bash
sudo pkg install -y devel/cmake devel/ninja devel/pkgconf devel/git multimedia/ffmpeg audio/sndio devel/qt5-qmake
sudo pkg install -y devel/qt5-buildtools devel/qt5-core x11-toolkits/qt5-gui x11-toolkits/qt5-widgets graphics/qt5-svg
sudo pkg install -y devel/qt5-linguisttools
git clone https://github.com/punesemu/puNES
cd puNES
cmake -B build -G Ninja -DQt5_DIR=/usr/local/lib/qt5/cmake/Qt5 -DENABLE_FFMPEG:BOOL=ON
cmake --build build -j2
```

if you don't want to use the Ninja build system and prefer the classic Makefile:

```bash
cmake -B build -DQt5_DIR=/usr/local/lib/qt5/cmake/Qt5 -DENABLE_FFMPEG:BOOL=ON
make -j2
```

the executable `punes` is in the `build/src` directory.

#### FreeBSD Debug version

If you need the debug version then you need to replace the `cmake -B Build -G Ninja` command of the previous examples with the following:

```bash
cmake -B build -G Ninja -DCMAKE_C_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_CXX_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_RELEASE:BOOL=OFF [...]
cmake --build build -j2
```

or if you prefer the classic Makefile:

```bash
cmake -B build -DCMAKE_C_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_CXX_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_RELEASE:BOOL=OFF [...]
make -j2
```

where `[...]` are the other necessary options.

</details>

## :blowfish: OpenBSD

<details>
<summary>Expand</summary>

#### Dependencies

- [CMake >= 3.14](https://cmake.org) ([Ninja](https://ninja-build.org) build system is optional)
- [Qt5](https://www.qt.io) or [Qt6](https://www.qt.io) with OpenGL support (qtcore, qtgui, qtwidgets, qtnetwork, qtsvg and qttools)
- [sndio](http://www.sndio.org)
- [libX11 and libXrandr](https://www.x.org)
- [p7zip](https://github.com/p7zip-project/p7zip) for compressed file support (lib7zip uses the 7z.so library on unix-like systems)
- (optional) [ffmpeg libraries >= 4.0](https://ffmpeg.org) if you want video and audio recording support (libavcodec, libavformat, libavutil, libswresample and libswscale). See [notes](#movie_camera-ffmpeg)

#### Compilation of puNES

```bash
git clone https://github.com/punesemu/puNES
cd puNES
cmake -B build -G Ninja -DQt5_DIR=/usr/local/lib/qt5/cmake/Qt5 -DENABLE_FFMPEG:BOOL=ON
cmake --build build -j2
```

if you don't want to use the Ninja build system and prefer the classic Makefile:

```bash
cmake -B build -DQt5_DIR=/usr/local/lib/qt5/cmake/Qt5 -DENABLE_FFMPEG:BOOL=ON
make -j2
```

the executable `punes` is in the `buid/src` directory.

#### OpenBSD Debug version

If you need the debug version then you need to replace the `cmake -B Build -G Ninja` command of the previous examples with the following:

```bash
cmake -B build -G Ninja -DCMAKE_C_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_CXX_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_RELEASE:BOOL=OFF [...]
cmake --build build -j2
```

or if you prefer the classic Makefile:

```bash
cmake -B build -DCMAKE_C_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_CXX_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_RELEASE:BOOL=OFF [...]
make -j2
```

where `[...]` are the other necessary options.

</details>

## :computer: Windows

<details>
<summary>Expand</summary>

#### Dependencies

- [Qt5](https://www.qt.io) with OpenGL support (5.6.3 is the last if you want the support for Windows XP)

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
pacman -S mingw-w64-i686-ffmpeg mingw-w64-x86_64-ffmpeg
exit
```

4. open a new MSYS2 shell and build the necessary libraries

#### Compilation of the Qt5 libraries

5. download and unzip the sources

```bash
wget http://download.qt.io/archive/qt/5.15/5.15.8/submodules/qtbase-everywhere-opensource-src-5.15.8.zip
unzip qtbase-everywhere-opensource-src-5.15.8.zip
mv qtbase-everywhere-src-5.15.8 qt5
```

the renaming of the directory is necessary to not generate a compile-time error caused by the 255 characters maximum path length limitation on Windows, This is the typical error message you might encounter:

```code
"../../../../include/QtEventDispatcherSupport/5.15.8/QtEventDispatcherSupport/private/qwindowsguieventdispatcher_p.h:1:10: fatal error: ../../../../../src/platformsupport/eventdispatchers/qwindowsguieventdispatcher_p.h: No such file or directory"
```

6. compile the libraries

```bash
cd qt5
echo -e "QMAKE_LFLAGS += -static -static-libgcc\nDEFINES += QT_STATIC_BUILD\n" >> mkspecs/win32-g++/qmake.conf
./configure.bat -prefix $MINGW_PREFIX -extprefix $MINGW_PREFIX -bindir $MINGW_PREFIX/lib/qt5/bin -headerdir $MINGW_PREFIX/include/qt5 -libdir $MINGW_PREFIX/lib/qt5 -archdatadir $MINGW_PREFIX/lib/qt5 -plugindir $MINGW_PREFIX/lib/qt5/plugins -libexecdir $MINGW_PREFIX/lib/qt5/bin -datadir $MINGW_PREFIX/share/qt5 -docdir $MINGW_PREFIX/share/doc/qt5 -translationdir $MINGW_PREFIX/share/qt5/translations -sysconfdir $MINGW_PREFIX/etc/xdg -examplesdir $MINGW_PREFIX/share/qt5/examples -testsdir $MINGW_PREFIX/share/qt5/tests -platform win32-g++ -nomake examples -nomake tests -nomake tools -no-compile-examples -release -opensource -confirm-license -static -c++std c++17 -sse2 -static-runtime -make libs -no-ltcg -no-dbus -no-accessibility -no-inotify -no-iconv -no-icu -no-openssl -no-system-proxies -no-cups -no-fontconfig -no-zstd -opengl desktop -no-angle -gif -ico -qt-libpng -qt-libjpeg -qt-pcre -qt-zlib -qt-freetype
make
```

7. and finally install them

```bash
make install
cp -v $MINGW_PREFIX/lib/qt5/pkgconfig/* $MINGW_PREFIX/lib/pkgconfig/.
cd ..
```

8. now it's time for the SVG module...

```bash
wget http://download.qt.io/archive/qt/5.15/5.15.8/submodules/qtsvg-everywhere-opensource-src-5.15.8.zip
unzip qtsvg-everywhere-opensource-src-5.15.8.zip
mv qtsvg-everywhere-src-5.15.8 qt5svg
cd qt5svg
$MINGW_PREFIX/lib/qt5/bin/qmake
make
make install
cp -v $MINGW_PREFIX/lib/qt5/pkgconfig/* $MINGW_PREFIX/lib/pkgconfig/.
cd ..
```

9. ...and for the tools

```bash
wget http://download.qt.io/archive/qt/5.15/5.15.8/submodules/qttools-everywhere-opensource-src-5.15.8.zip
unzip qttools-everywhere-opensource-src-5.15.8.zip
mv qttools-everywhere-src-5.15.8 qt5tools
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
```

if you want D3D9 version :

```bash
cmake -B build -G Ninja -DENABLE_FFMPEG:BOOL=ON -DENABLE_OPENGL:BOOL=OFF
cmake --build build -j2
```

otherwise :

```bash
cmake -B build -G Ninja -DENABLE_FFMPEG:BOOL=ON
cmake --build build -j2
```

The executable `punes.exe` is in the `build/src` directory but in order to run it you need the following dlls:

- 7z.dl
- avcodec-58.dll
- avformat-58.dll
- avutil-56.dll
- cg.dll
- cgD3D9.dll (only for D3D9 version)
- cgGL.dll (only for OpenGL version)
- libwinpthread-1.dll
- swresample-3.dll
- swscale-5.dll

that you can download here : :link:[`64bit`](https://www.dropbox.com/s/d632cjezybz6a74/puNES_x86_64_dlls.zip?dl=1) version or :link:[`32bit`](https://www.dropbox.com/s/ye00129nyacdl05/puNES_i686_dlls.zip?dl=1) version.

#### Windows Debug version

If you need the debug version then you need to replace the `cmake -B build -G Ninja` command of the previous examples with the following:

```bash
cmake -B build -G Ninja -DCMAKE_C_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_CXX_FLAGS_DEBUG:STRING='-O0 -g -DDEBUG' -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_RELEASE:BOOL=OFF [...]
```

where `[...]` are the other necessary options.

</details>

---

#### :movie_camera: FFmpeg

It is always possible to disable audio/video recording support by specifying the `configure` parameter `--without-ffmpeg`.
If the installed version is lower than 4.0 the support will be disabled automatically.

Supported audio recording formats:

- WAV Audio
- MP3 Audio ([lame](https://xiph.org/vorbis/)) (\*)
- AAC Audio
- Flac Audio
- Ogg Audio ([vorbis](https://xiph.org/vorbis/)) (\*)
- Opus Audio ([libopus](https://www.opus-codec.org)) (\*)

Supported video recording formats:

- MPEG 1 Video
- MPEG 2 Video
- MPEG 4 Video
- MPEG H264 Video ([libx264](https://www.videolan.org/developers/x264.html)) (\*)
- High Efficiency Video Codec ([libx265](https://www.videolan.org/developers/x265.html)) (\*)
- WebM Video ([libvpx](https://www.webmproject.org/code)) (\*)
- Windows Media Video
- AVI FF Video
- AVI Video

(\*) if compiled in FFmpeg.
