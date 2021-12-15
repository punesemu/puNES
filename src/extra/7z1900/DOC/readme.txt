7-Zip 19.00 Sources
-------------------

7-Zip is a file archiver for Windows. 

7-Zip Copyright (C) 1999-2019 Igor Pavlov.


License Info
------------

7-Zip is free software distributed under the GNU LGPL 
(except for unRar code).
read License.txt for more infomation about license.

Notes about unRAR license:

Please check main restriction from unRar license:

   2. The unRAR sources may be used in any software to handle RAR
      archives without limitations free of charge, but cannot be used
      to re-create the RAR compression algorithm, which is proprietary.
      Distribution of modified unRAR sources in separate form or as a
      part of other software is permitted, provided that it is clearly
      stated in the documentation and source comments that the code may
      not be used to develop a RAR (WinRAR) compatible archiver.

In brief it means:
1) You can compile and use compiled files under GNU LGPL rules, since 
   unRAR license almost has no restrictions for compiled files.
   You can link these compiled files to LGPL programs.
2) You can fix bugs in source code and use compiled fixed version.
3) You can not use unRAR sources to re-create the RAR compression algorithm.


LZMA SDK
--------

This package also contains some files from LZMA SDK
You can download LZMA SDK from:
  http://www.7-zip.org/sdk.html
LZMA SDK is written and placed in the public domain by Igor Pavlov.


How to compile
--------------

To compile the sources to Windows binaries you need Visual Studio compiler and/or Windows SDK.
You can use latest Windows Studio 2017 to compile binaries for x86, x64 and arm64 platforms.
Also you can use old compilers for some platforms:
  x86   : Visual C++ 6.0 with Platform SDK
  x64   : Windows Server 2003 R2 Platform SDK
  arm64 : Windows Studio 2017
  arm   : Windows Studio 2017
  ia64 (itanium)     : Windows Server 2003 R2 Platform SDK
  arm for Windows CE : Standard SDK for Windows CE 5.0

If you use MSVC6, specify also Platform SDK directories at top of directories lists:
Tools / Options / Directories
  - Include files
  - Library files

Also you need Microsoft Macro Assembler:
  - ml.exe for x86 
  - ml64.exe for x64
You can use ml.exe from Windows SDK for Windows Vista or some later versions.

There are two ways to compile 7-Zip binaries:
1) via makefile in command line.
2) via dsp file in Visual Studio.

The dsp file compiling can be used for development and debug purposes.
The final 7-Zip binaries are compiled via makefiles, that provide best 
optimization options.

How to compile with makefile
----------------------------

Some macronames can be defined for compiling with makefile:

PLATFORM
  with possible values: x64, x86, arm64, arm, ia64

OLD_COMPILER
  for old VC compiler, like MSCV 6.0.

MY_DYNAMIC_LINK
  for dynamic linking to the run-time library (msvcrt.dll). 
  The default makefile option is static linking to the run-time library.



Compiling under Unix/Linux
--------------------------
Check this site for Posix/Linux version:
http://sourceforge.net/projects/p7zip/


Notes:
------
7-Zip consists of COM modules (DLL files).
But 7-Zip doesn't use standard COM interfaces for creating objects.
Look at
7zip\UI\Client7z folder for example of using DLL files of 7-Zip. 
Some DLL files can use other DLL files from 7-Zip.
If you don't like it, you must use standalone version of DLL.
To compile standalone version of DLL you must include all used parts
to project and define some defs. 
For example, 7zip\Bundles\Format7z is a standalone version  of 7z.dll 
that works with 7z format. So you can use such DLL in your project 
without additional DLL files.


Description of 7-Zip sources package
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DOC                Documentation
---
  7zFormat.txt   - 7z format description
  copying.txt    - GNU LGPL license
  unRarLicense.txt - License for unRAR part of source code
  src-history.txt  - Sources history
  Methods.txt    - Compression method IDs
  readme.txt     - Readme file
  lzma.txt       - LZMA compression description
  7zip.nsi       - installer script for NSIS
  7zip.wix       - installer script for WIX


Asm - Source code in Assembler (optimized code for CRC calculation and Intel-AES encryption)

C   - Source code in C

CPP - Source code in C++

Common            common files for C++ projects

Windows           common files for Windows related code

7zip

  Common          Common modules for 7-zip

  Archive         files related to archiving

  Bundle          Modules that are bundles of other modules (files)

    Alone         7za.exe: Standalone version of 7-Zip console that supports only 7z/xz/cab/zip/gzip/bzip2/tar.
    Alone7z       7zr.exe: Standalone version of 7-Zip console that supports only 7z (reduced version)
    Fm            Standalone version of 7-Zip File Manager
    Format7z            7za.dll:  .7z support
    Format7zExtract     7zxa.dll: .7z support, extracting only
    Format7zR           7zr.dll:  .7z support, reduced version
    Format7zExtractR    7zxr.dll: .7z support, reduced version, extracting only
    Format7zF           7z.dll:   all formats
    LzmaCon       lzma.exe: LZMA compression/decompression
    SFXCon        7zCon.sfx: Console 7z SFX module
    SFXWin        7z.sfx: Windows 7z SFX module
    SFXSetup      7zS.sfx: Windows 7z SFX module for Installers

  Compress        files for compression/decompression

  Crypto          files for encryption / decompression

  UI

    Agent         Intermediary modules for FAR plugin and Explorer plugin
    Client7z      Test application for 7za.dll 
    Common        Common UI files
    Console       7z.exe : Console version
    Explorer      7-zip.dll: 7-Zip Shell extension
    Far           plugin for Far Manager
    FileManager   7zFM.exe: 7-Zip File Manager
    GUI           7zG.exe: 7-Zip GUI version



---
Igor Pavlov
http://www.7-zip.org
