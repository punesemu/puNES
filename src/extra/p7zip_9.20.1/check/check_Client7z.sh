#! /bin/sh

sure()
{
  eval $*
  if [ "$?" != "0" ]
  then
    echo "ERROR during : $*"
    echo "ERROR during : $*" > last_error
    exit 1
  fi
}

LD_LIBRARY_PATH=`cd ../bin ; pwd`:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH
echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"

PZIP7=`pwd`"/$1"

REP=TMP_$$
echo "REP=${REP}"

chmod -R 777 ${REP} 2> /dev/null
rm -fr   ${REP}
mkdir -p ${REP}

cd ${REP}

# sure rm -fr 7za433_ref 7za433_7zip_bzip2 7za433_7zip_lzma 7za433_7zip_lzma_crypto 7za433_7zip_ppmd 7za433_tar
# sure rm -fr 7za433_7zip_bzip2.7z 7za433_7zip_lzma.7z 7za433_7zip_lzma_crypto.7z 7za433_7zip_ppmd.7z 7za433_tar.tar

# for Beos or MacOSX
ln -s ../../bin/7z.so .

echo ""
echo "# TESTING ..."
echo "#############"

sure ${PZIP7} l ../test/7za433_7zip_lzma.7z
# sure ${PZIP7} t -pqwerty ../test/7za433_7zip_lzma_crypto.7z
sure ${PZIP7} l ../test/7za433_7zip_ppmd.7z
sure ${PZIP7} l ../test/7za433_7zip_bzip2.7z



echo ""
echo "# EXTRACTING ..."
echo "################"

sure tar xf ../test/7za433_tar.tar
sure mv 7za433_tar 7za433_ref

sure ${PZIP7} x ../test/7za433_7zip_lzma.7z
sure diff -r 7za433_ref 7za433_7zip_lzma

# sure ${PZIP7} x -pqwerty ../test/7za433_7zip_lzma_crypto.7z
# sure diff -r 7za433_ref 7za433_7zip_lzma_crypto

sure ${PZIP7} x ../test/7za433_7zip_ppmd.7z
sure diff -r 7za433_ref 7za433_7zip_ppmd

sure ${PZIP7} x ../test/7za433_7zip_bzip2.7z
sure diff -r 7za433_ref 7za433_7zip_bzip2

echo ""
echo "# Archiving ..."
echo "###############"

sure ${PZIP7} a 7za433_7zip_lzma.7z 7za433_7zip_lzma/bin/7za.exe 7za433_7zip_lzma/readme.txt 7za433_7zip_lzma/doc/copying.txt

echo ""
echo "# EXTRACTING (PASS 2) ..."
echo "#########################"

sure rm -fr 7za433_7zip_bzip2 7za433_7zip_lzma 7za433_7zip_lzma_crypto 7za433_7zip_ppmd 7za433_tar

sure ${PZIP7} x 7za433_7zip_lzma.7z
sure diff -r 7za433_ref 7za433_7zip_lzma

cd ..

# ./clean_all.sh
chmod -R 777 ${REP} 2> /dev/null
rm -fr   ${REP}

echo ""
echo "========"
echo "ALL DONE"
echo "========"
echo ""
