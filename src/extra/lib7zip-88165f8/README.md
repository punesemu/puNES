# lib7zip
A library using 7z.dll/7z.so(from 7-Zip) to handle different archive types. lib7zip is based on 7zip/p7zip source code, but NOT including any source code from 7zip/p7zip.

Tips
====
* Build lib7zip
    * Under UNIX/LINUX like system
        * Get a copy of p7zip source code, and extract to a folder
        * Define a env P7ZIP_SOURCE_DIR point to the extracted folder
        * cmake -DBUILD_SHARED_LIB=OFF -DP7ZIP_SOURCE_DIR=${P7ZIP_SOURCE_DIR}
    * Under windows
        * Get mingw from http://www.mingw.org
        * Get a copy of original 7zip source code, NOT the p7zip for linux
        * Define a env P7ZIP_SOURCE_DIR point to the extracted folder
        * cmake -DBUILD_SHARED_LIB=OFF -DP7ZIP_SOURCE_DIR=${P7ZIP_SOURCE_DIR}
* Run lib7zip
    * Under UNIX/LINUX like system
        * install p7zip binary
        * find 7z.so path, export LD_LIBRARY_PATH=<where 7z.so existing>
    * Under Windows
        * install 7zip binary
        * copy 7z.dll to where your application existing

> __Any time or any problem about lib7zip, please feel free to write me an email.__

> __Any feature or patch request, please also feel free to write me an email.__

Thanks
====
* Many thanks to _Joe_ who provide so many great patches
* Many thanks to _Christoph_ who give so many great advises and patch.
* Many thanks to _Christoph Thielecke_ to provide great patches for OS2 and dynamic library

To Do
====
* Add Compress function to library

Related Projects
====
* Python Binding created by Mark, http://github.com/harvimt/pylib7zip

Change Log
====
3.0.0
----
1. move build system to cmake
2. fix bug when do signature detect for dmg files
3. fix bug of memory leaking when deal with sub archive

2.0.0
----
1. Make the library compiling with latest p7zip 15.9.0 and 7zip 15.10.0
2. Fix bug in test7zipmulti

1.6.5
----
1. Add new parameter bool fDetectFileTypeBySignature to OpenArchive, when fDetectFileTypeBySignature = true, lib7zip will using file signature instead file name extension to detect file type
2. remove out-of-dated visual studio files

1.6.4
----
1. add AUTHORS COPYING file
2. add LIB7ZIP_ prefix to error code enum,break the old client, please update your code
3. add APIs SetLib7ZipLocale and GetLib7ZipLocale, client could use these API to force lib7zip locale, otherwise lib7zip will use current user's locale
4. add list of path to find 7z.so when 7z.so is not in users ld path
5. fix Mac OSX compile fail problem

1.6.3
----
1. Add GetLastError to C7ZipLibrary and Error code define in lib7zip.h
2. open archive with password now work for archive created by 7za a -mhe -p, who encrypted the file names in archive

1.6.2
----
1. Fixed broken windows built system
2. Fixed build script for windows

1.6.1
----
1. Add OS2 support
2. create dynamic library along with static library

1.6.0
----
1. Add Multi-Volume support

1.5.0
----
1. Add Password support

1.4.1
----
1. Add GetProperty functions to C7ZipArchive to retrieve archive properties
2. Add kpidSize to return Item umcompressed size, the same as GetSize returning

1.4.0
----
1. Add patches from Christoph
2. make the test program works when no Test7Zip.7z found
3. Add functions to get more property about items in the archive
4. Tested on Mac OS X
5. Move source control to Mercurial for better distributed development

1.3.0
----
1. Add patches from Joe,
2. make the library work with latest p7zip 9.20

1.0.2
----
1. Add patches from Joe,
2. Add a method to get the compressed size
3. Add a method to expose whether the file is encrypted
4. Build scripts update
5. Small fix to make the lib working with the latest p7zip source

1.0.1
----
1. First release, support both LINUX and windows platform.
