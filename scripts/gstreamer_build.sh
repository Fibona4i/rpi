#!/bin/bash

#sudo apt-get install bison flex glib-2.0 libglib2.0-0 libglib2.0-dev
#sudo apt-get install libalsa-ocaml libalsa-ocaml-dev
#sudo apt-get install libvo-aacenc-0 libvo-aacenc-dev
#sudo apt-get install libmp4v2-2 libmp4v2-dev
#sudo apt-get install libavcodec-extra libavcodec-extra-56
#sudo apt-get install v4l2loopback-dkms
#sudo apt-get install libx264-dev libpango1.0-dev

sudo apt-get install bison flex glib-2.0 libglib2.0-0 libglib2.0-dev libalsa-ocaml libalsa-ocaml-dev libvo-aacenc-0 libvo-aacenc-dev libmp4v2-2 libmp4v2-dev libavcodec-extra libavcodec-extra-56 v4l2loopback-dkms libx264-dev libpango1.0-dev libv4l-dev

# you might need these additonal packages:
# apt-get install  bison flex  libusb-1.0-0-dev libgudev-1.0-dev libxv-dev

# from http://lists.freedesktop.org/archives/gstreamer-openmax/2013-March/000724.html
# apt-get install build-essential autotools-dev automake autoconf libtool autopoint libxml2-dev zlib1g-dev libglib2.0-dev pkg-config bison flex python
# apt-get install libasound2-dev libgudev-1.0-dev libxt-dev libvorbis-dev libcdparanoia-dev libpango1.0-dev libtheora-dev libvisual-0.4-dev iso-codes libgtk-3-dev libraw1394-dev libiec61883-dev libavc1394-dev libv4l-dev libcairo2-dev libcaca-dev libspeex-dev libpng-dev libshout3-dev libjpeg-dev libaa1-dev libflac-dev libdv4-dev libtag1-dev libwavpack-dev libpulse-dev libsoup2.4-dev libbz2-dev libcdaudio-dev libdc1394-22-dev ladspa-sdk libass-dev libcurl4-gnutls-dev libdca-dev libdirac-dev libdvdnav-dev libexempi-dev libexif-dev libfaad-dev libgme-dev libgsm1-dev libiptcdata0-dev libkate-dev libmimic-dev libmms-dev libmodplug-dev libmpcdec-dev libofa0-dev libopus-dev librsvg2-dev librtmp-dev libschroedinger-dev libslv2-dev libsndfile1-dev libsoundtouch-dev libspandsp-dev libx11-dev libxvidcore-dev libzbar-dev libzvbi-dev liba52-0.7.4-dev libcdio-dev libdvdread-dev libmad0-dev libmp3lame-dev libmpeg2-4-dev libopencore-amrnb-dev libopencore-amrwb-dev libsidplay1-dev libtwolame-dev libx264-dev
# source: http://gstreamer.freedesktop.org/wiki/HowToCompileForEmbedded
#
# to build with libav support
# apt-get install libavutil-dev
# apt-get install yasm
#
# and for our stream.sh script:
# apt-get install  inotify-tools
#
# For reference: there is also an ppa, but it currently seems to only offer gstreamer 1.0, 
# which does not include uvch264src
# add-apt-repository ppa:gstreamer-developers/ppa
# apt-get update
# apt-get install gstreamer1.0*



SOURCE=$HOME/gstreamer_sources
#BUILD=$HOME/gstreamer_build
BUILD=/usr
if [ ! -d "$SOURCE" ]; then
	mkdir $SOURCE
fi
#if [ ! -d "$BUILD" ]; then
#	mkdir $BUILD
#fi

cd $SOURCE
wget http://gstreamer.freedesktop.org/src/gstreamer/gstreamer-1.6.3.tar.xz
tar -xJf gstreamer-1.6.3.tar.xz
wget http://gstreamer.freedesktop.org/src/orc/orc-0.4.24.tar.gz
tar -xzf orc-0.4.24.tar.gz
wget http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-1.6.3.tar.xz
tar -xJf gst-plugins-base-1.6.3.tar.xz
wget http://gstreamer.freedesktop.org/src/gst-plugins-good/gst-plugins-good-1.6.3.tar.xz
tar -xJf gst-plugins-good-1.6.3.tar.xz
#wget http://gstreamer.freedesktop.org/src/gst-plugins-ugly/gst-plugins-ugly-1.6.3.tar.xz
#tar -xJf gst-plugins-ugly-1.6.3.tar.xz
wget http://gstreamer.freedesktop.org/src/gst-plugins-bad/gst-plugins-bad-1.6.3.tar.xz
tar -xJf gst-plugins-bad-1.6.3.tar.xz
#wget http://gstreamer.freedesktop.org/src/gst-libav/gst-libav-1.6.3.tar.xz
#tar -xJf gst-libav-1.6.3.tar.xz
wget http://gstreamer.freedesktop.org/src/gst-omx/gst-omx-1.2.0.tar.xz
tar -xJf gst-omx-1.2.0.tar.xz

make_install () {
	make -j4
	sudo make install
}

make_clean () {
	make clean
}

cd orc-0.4.24
make_clean
./configure --prefix=$BUILD
make_install

cd ../gstreamer-1.6.3
make_clean
./configure --prefix=$BUILD
make_install

cd ../gst-plugins-base-1.6.3
make_clean
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure --prefix=$BUILD  --enable-orc --enable-alsa --enable-pango
make_install

cd ../gst-plugins-good-1.6.3
make_clean
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --enable-orc --enable-gst_v4l2 --with-libv4l2 --enable-experimental
make_install

cd ../gst-plugins-bad-1.6.3
make_clean
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --enable-orc --enable-voaacenc --enable-uvch264
make_install

#cd ../gst-libav-1.6.3
#make_clean
#export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --enable-orc
#make_install

cd ../gst-omx-1.2
make_clean
export PKG_CONFIG_PATH=$BUILD/lib/pkgconfig:$PKG_CONFIG_PATH; ./configure  --prefix=$BUILD  --with-omx-header-path=/opt/vc/include/IL --with-omx-target=rpi
make_install

echo ; echo "GStreamer 1.6.3 installed sucessfully!" ; echo

# to completly remove gstreamer, simpely delete the two directories:
# rm -rf $SOURCE
# rm -rf $BUILD

# to use this version of gstreamer, add it to your PATH:
# echo -e "\n\n#Path for a local version of gstreamer\nPATH=\"$BUILD/bin:\$PATH\"" >> $HOME/.profile
