MKDIR = mkdir -p

DEST_HOME=/usr/local
DEST_BIN=$(DEST_HOME)/bin
DEST_SHARE=$(DEST_HOME)/lib/p7zip
DEST_SHARE_DOC=$(DEST_HOME)/share/doc/p7zip
DEST_MAN=$(DEST_HOME)/man

.PHONY: default all all2 7za 7zG 7zFM sfx 7z 7zr Client7z common common7z clean tar_bin depend test test_7z test_7zr test_7zG test_Client7z all_test app

default:7za

all:7za sfx

all2: 7za sfx 7z

all3: 7za sfx 7z 7zr

all4: 7za sfx 7z 7zr Client7z 7zG 7zFM

all_test : test test_7z test_7zr test_Client7z
	$(MAKE) -C CPP/7zip/Compress/LZMA_Alone  test

common:
	$(MKDIR) bin

7za: common
	$(MAKE) -C CPP/7zip/Bundles/Alone all
	$(MAKE) -C check/my_86_filter  all

7zr: common
	$(MAKE) -C CPP/7zip/Bundles/Alone7z  all

Client7z: common
	$(MKDIR) bin/Codecs
	$(MAKE) -C CPP/7zip/Bundles/Format7zFree all
	$(MAKE) -C CPP/7zip/UI/Client7z          all

app: common 7zFM 7zG 7z sfx
	rm -fr               p7zip.app
	$(MKDIR)             p7zip.app
	cp -rp GUI/Contents  p7zip.app/
	$(MKDIR)          p7zip.app/Contents/MacOS
	cp bin/7zFM       p7zip.app/Contents/MacOS/
	cp bin/7zG        p7zip.app/Contents/MacOS/
	cp bin/7z.so      p7zip.app/Contents/MacOS/
	cp bin/7zCon.sfx  p7zip.app/Contents/MacOS/
	cp -rp bin/Codecs p7zip.app/Contents/MacOS/
	cp -rp GUI/Lang   p7zip.app/Contents/MacOS/
	cp -rp GUI/help   p7zip.app/Contents/MacOS/

depend:
	$(MAKE) -C CPP/7zip/Bundles/Alone         depend
	$(MAKE) -C CPP/7zip/Bundles/Alone7z       depend
	$(MAKE) -C CPP/7zip/Bundles/SFXCon        depend
	$(MAKE) -C CPP/7zip/UI/Client7z           depend
	$(MAKE) -C CPP/7zip/UI/Console            depend
	$(MAKE) -C CPP/7zip/Bundles/Format7zFree  depend
	$(MAKE) -C CPP/7zip/Compress/Rar          depend
	$(MAKE) -C CPP/7zip/UI/GUI                depend
	$(MAKE) -C CPP/7zip/UI/FileManager        depend
	$(MAKE) -C check/my_86_filter             depend

sfx: common
	$(MKDIR) bin
	$(MAKE) -C CPP/7zip/Bundles/SFXCon  all

common7z:common
	$(MKDIR) bin/Codecs
	$(MAKE) -C CPP/7zip/Bundles/Format7zFree all
	$(MAKE) -C CPP/7zip/Compress/Rar         all

7z: common7z
	$(MAKE) -C CPP/7zip/UI/Console           all

7zG: common7z
	cd bin ; rm -f Lang ; ln -s ../GUI/Lang .
	cd bin ; rm -f help ; ln -s ../GUI/help .
	$(MAKE) -C CPP/7zip/UI/GUI               all

7zFM: common7z
	cd bin ; rm -f Lang ; ln -s ../GUI/Lang .
	cd bin ; rm -f help ; ln -s ../GUI/help .
	$(MAKE) -C CPP/7zip/UI/FileManager       all

clean:
	$(MAKE) -C CPP/myWindows                 clean
	$(MAKE) -C CPP/7zip/Bundles/Alone        clean
	$(MAKE) -C CPP/7zip/Bundles/Alone7z      clean
	$(MAKE) -C CPP/7zip/Bundles/SFXCon       clean
	$(MAKE) -C CPP/7zip/UI/Client7z          clean
	$(MAKE) -C CPP/7zip/UI/Console           clean
	$(MAKE) -C CPP/7zip/UI/FileManager       clean
	$(MAKE) -C CPP/7zip/UI/GUI               clean
	$(MAKE) -C CPP/7zip/Bundles/Format7zFree clean
	$(MAKE) -C CPP/7zip/Compress/Rar         clean
	$(MAKE) -C CPP/7zip/Compress/LZMA_Alone  clean
	$(MAKE) -C CPP/7zip/Bundles/AloneGCOV    clean
	$(MAKE) -C CPP/7zip/TEST/TestUI          clean
	$(MAKE) -C check/my_86_filter            clean
	rm -fr bin
	rm -fr p7zip.app
	rm -fr CPP/7zip/P7ZIP.*
	rm -fr CPP/7zip/CMAKE/P7ZIP.*
	rm -fr CPP/7zip/PREMAKE/P7ZIP.*
	rm -f  CPP/7zip/QMAKE/*/*.o
	rm -f  CPP/7zip/QMAKE/*/Makefile
	rm -f  CPP/7zip/QMAKE/*/*.pro.user
	rm -f  CPP/7zip/QMAKE/*/*.x
	rm -f make.log 1 2
	rm -f check/7z.so
	rm -fr p7zip.app/Contents/MacOS
	find . -name "*~"        -exec rm -f {} \;
	find . -name "*.orig"    -exec rm -fr {} \;
	find . -name ".*.swp"    -exec rm -f {} \;
	find . -name "*.[ch]"    -exec chmod -x {} \;
	find . -name "*.cpp"     -exec chmod -x {} \;
	find . -name "*.asm"     -exec chmod -x {} \;
	find . -name "makefile*" -exec chmod -x {} \;
	find . -name ".DS_Store" -exec rm -f {} \;
	find . -name "._*"       -exec rm -f {} \;
	chmod -x ChangeLog README TODO man1/* DOCS/*.txt
	chmod +x contrib/VirtualFileSystemForMidnightCommander/u7z
	chmod +x contrib/gzip-like_CLI_wrapper_for_7z/p7zip
	chmod +x install.sh check/check.sh check/clean_all.sh check/check_7zr.sh 
	cd check                  ; ./clean_all.sh

test: 7za sfx
	cd check ; ./check.sh ../bin/7za

test_7z: 7z sfx
	cd check ; ./check.sh ../bin/7z

test_7zr: 7zr sfx
	cd check ; ./check_7zr.sh ../bin/7zr

test_7zG: 7zG sfx
	cd check ; ./check.sh ../bin/7zG

test_Client7z: Client7z
	cd check ; ./check_Client7z.sh ../bin/Client7z

install:
	./install.sh $(DEST_BIN) $(DEST_SHARE) $(DEST_MAN) $(DEST_SHARE_DOC) $(DEST_DIR)

REP=$(shell pwd)
ARCHIVE=$(shell basename $(REP))

.PHONY: tar_all tar_all2 src_7z tar_bin tar_bin2

tar_all : clean
	rm -f  ../$(ARCHIVE)_src_all.tar.bz2
	cp makefile.linux_any_cpu makefile.machine
	cd .. ; (tar cf - $(ARCHIVE) | bzip2 -9 > $(ARCHIVE)_src_all.tar.bz2)

tar_all2 : clean
	rm -f  ../$(ARCHIVE)_src_all.tar.bz2
	cp makefile.linux_any_cpu makefile.machine
	cd .. ; (tar cf - $(ARCHIVE) | 7za a -mx=9 -tbzip2 -si $(ARCHIVE)_src_all.tar.bz2 )

src_7z : clean
	rm -f  ../$(ARCHIVE)_src.7z
	cd .. ; 7za a -mx=9 -m0=ppmd:mem=128m:o=32 $(ARCHIVE)_src.7z $(ARCHIVE)

tar_bin:
	rm -f  ../$(ARCHIVE)_x86_linux_bin.tar.bz2
	chmod +x install.sh contrib/VirtualFileSystemForMidnightCommander/u7z contrib/gzip-like_CLI_wrapper_for_7z/p7zip
	cd .. ; (tar cf - $(ARCHIVE)/bin $(ARCHIVE)/contrib $(ARCHIVE)/man1 $(ARCHIVE)/install.sh $(ARCHIVE)/ChangeLog $(ARCHIVE)/DOCS $(ARCHIVE)/README $(ARCHIVE)/TODO | bzip2 -9 > $(ARCHIVE)_x86_linux_bin.tar.bz2)

tar_bin2:
	rm -f  ../$(ARCHIVE)_x86_linux_bin.tar.bz2
	chmod +x install.sh contrib/VirtualFileSystemForMidnightCommander/u7z contrib/gzip-like_CLI_wrapper_for_7z/p7zip
	cd .. ; (tar cf - $(ARCHIVE)/bin $(ARCHIVE)/contrib $(ARCHIVE)/man1 $(ARCHIVE)/install.sh $(ARCHIVE)/ChangeLog $(ARCHIVE)/DOCS $(ARCHIVE)/README $(ARCHIVE)/TODO | 7za a -mx=9 -tbzip2 -si $(ARCHIVE)_x86_linux_bin.tar.bz2)

