#!/bin/sh

arch=x86_64
extlib_prefix="../../usr"
cross_prefix=x86_64-w64-mingw32-
clean_build=true

for opt in "$@"
do
    case "$opt" in
    x86)
            arch=i686
            cross_prefix=i686-w64-mingw32-
            ;;
    x64 | amd64)
            arch=x86_64
            cross_prefix=x86_64-w64-mingw32-
            ;;
    quick)
            clean_build=false
            ;;
    *)
            echo "Unknown Option $opt"
            exit 1
    esac
done

EXTRA_CFLAGS="-fno-tree-vectorize -D_WIN32_WINNT=0x0600 -DWINVER=0x0600"
EXTRA_LDFLAGS=""
PKG_CONFIG_PREFIX_DIR=""
if [ "${arch}" == "x86_64" ]; then
  #OPTIONS="${OPTIONS} --cpu=x86_64"
  export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:${extlib_prefix}/lib/pkgconfig/"
  OPTIONS="${OPTIONS} --enable-cross-compile --cross-prefix=${cross_prefix} --target-os=mingw32 --pkg-config=pkg-config"
  EXTRA_CFLAGS="${EXTRA_CFLAGS} -I${extlib_prefix}/include -O3"
  EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L${extlib_prefix}/lib -static-libstdc++ -static"
  PKG_CONFIG_PREFIX_DIR="--define-variable=prefix=${extlib_prefix}"
else
  #OPTIONS="${OPTIONS} --cpu=i686"
  #EXTRA_CFLAGS="-fno-tree-vectorize -D_WIN32_WINNT=0x0501 -DWINVER=0x0501"
  EXTRA_CFLAGS="-fno-tree-vectorize"
  export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:${extlib_prefix}/lib/pkgconfig/"
  OPTIONS="${OPTIONS} --enable-cross-compile --cross-prefix=${cross_prefix} --target-os=mingw32 --pkg-config=pkg-config"
  EXTRA_CFLAGS="${EXTRA_CFLAGS} -I${extlib_prefix}/include -O3 -mmmx -msse -msse2 -mfpmath=sse -mstackrealign"
  EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L${extlib_prefix}/lib -static-libstdc++ -static"
  PKG_CONFIG_PREFIX_DIR="--define-variable=prefix=${extlib_prefix}"
fi

./configure \
${OPTIONS} \
--extra-ldflags="${EXTRA_LDFLAGS}" \
--extra-cflags="${EXTRA_CFLAGS}" \
--pkg-config-flags="--static ${PKG_CONFIG_PREFIX_DIR}" \
--prefix=/tmp/thirdparty \
--enable-shared \
--disable-static \
--enable-gpl \
--enable-version3 \
--enable-w32threads \
--disable-protocols \
--enable-protocol=file,pipe \
--enable-indevs \
--disable-demuxers \
--disable-decoders \
--enable-muxers \
--disable-sdl2 \
--enable-asm \
--disable-bsfs \
--disable-avresample \
--enable-avisynth \
--disable-avdevice \
--disable-postproc \
--enable-swresample \
--enable-encoders \
--disable-devices \
--disable-programs \
--disable-debug \
--disable-doc \
--disable-schannel \
--disable-cuda \
--disable-cuvid \
--disable-nvenc \
--disable-mediafoundation \
--enable-libdav1d \
--enable-libspeex \
--enable-libmp3lame \
--enable-libx264 \
--enable-libx265 \
--enable-libvpx \
--enable-libvorbis \
--enable-libopus \
--enable-zlib \
--enable-iconv \
--arch="${arch}"
