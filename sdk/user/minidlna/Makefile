ifneq ($(CONFIG_E8B),y)
DLNA_REDUCE=true
else
DLNA_REDUCE=false
endif
DLNA_INSTALL_PREFIX=$(shell pwd)/tmp
DLNA_ARCH=mips
DLNA_CROSS_COMPILER=$(CROSS_COMPILE)
LIBJPEG_FLAG=--prefix=$(DLNA_INSTALL_PREFIX) CC=$(DLNA_CROSS_COMPILER)gcc --host=$(DLNA_ARCH)-linux --enable-static --disable-shared
LIBEXIF_FLAG=--prefix=$(DLNA_INSTALL_PREFIX) CC=$(DLNA_CROSS_COMPILER)gcc --host=$(DLNA_ARCH)-linux --enable-static --disable-shared --disable-nls
ID3TAG_FLAG=--prefix=$(DLNA_INSTALL_PREFIX) CC=$(DLNA_CROSS_COMPILER)gcc --host=$(DLNA_ARCH)-linux CPPFLAGS="-I$(ROOTDIR)/lib/zlib" LDFLAGS="-L$(ROOTDIR)/lib/zlib" --enable-static --disable-shared --disable-debugging
OGG_FLAG=--prefix=$(DLNA_INSTALL_PREFIX) CC=$(DLNA_CROSS_COMPILER)gcc --host=$(DLNA_ARCH)-linux --enable-static --disable-shared
FLAC_FLAG=--prefix=$(DLNA_INSTALL_PREFIX) CC=$(DLNA_CROSS_COMPILER)gcc --host=$(DLNA_ARCH)-linux --disable-cpplibs --disable-ogg --disable-xmms-plugin --disable-oggtest --enable-static --disable-shared
VORBIS_FLAG=--prefix=$(DLNA_INSTALL_PREFIX) CC=$(DLNA_CROSS_COMPILER)gcc --host=$(DLNA_ARCH)-linux OGG_CFLAGS="-I$(DLNA_INSTALL_PREFIX)/include" OGG_LIBS="-L$(DLNA_INSTALL_PREFIX)/lib" --enable-static --disable-shared
SQLITE_FLAG=--prefix=$(DLNA_INSTALL_PREFIX) CC=$(DLNA_CROSS_COMPILER)gcc --host=$(DLNA_ARCH)-linux --enable-static --disable-shared 

ifneq ($(DLNA_REDUCE), true)
FFMPEG_FLAG=--cross-prefix=$(DLNA_CROSS_COMPILER) --target-os=linux --enable-cross-compile --arch=$(DLNA_ARCH) --prefix=$(DLNA_INSTALL_PREFIX) --enable-static --disable-shared  --enable-gpl --disable-network --disable-zlib --disable-vhook --disable-ffmpeg --disable-ffserver --disable-ffplay --enable-small --disable-encoders --disable-decoders --disable-muxers --disable-bsfs --disable-devices --enable-debug --disable-protocol=udp --disable-protocol=tcp --disable-protocol=rtp --disable-protocol=pipe --disable-protocol=http --disable-parser=cavsvideo --disable-parser=dca --disable-parser=dirac --disable-parser=dnxhd --disable-parser=mjpeg --disable-parser=mlp --disable-parser=pnm --disable-parser=vp3 --disable-demuxer=amr --disable-demuxer=apc --disable-demuxer=ape --disable-demuxer=ass --disable-demuxer=bethsoftvid --disable-demuxer=bfi --disable-demuxer=c93 --disable-demuxer=daud --disable-demuxer=dnxhd --disable-demuxer=dsicin --disable-demuxer=dxa --disable-demuxer=ffm --disable-demuxer=gsm \
			--disable-demuxer=gxf --disable-demuxer=idcin --disable-demuxer=iff --disable-demuxer=image2 --disable-demuxer=image2pipe --disable-demuxer=ingenient --disable-demuxer=ipmovie --disable-demuxer=lmlm4 --disable-demuxer=mm --disable-demuxer=mmf --disable-demuxer=msnwc_tcp --disable-demuxer=mtv --disable-demuxer=mxf --disable-demuxer=nsv --disable-demuxer=nut --disable-demuxer=oma --disable-demuxer=pva --disable-demuxer=rawvideo --disable-demuxer=rl2 --disable-demuxer=roq --disable-demuxer=rpl --disable-demuxer=segafilm --disable-demuxer=shorten --disable-demuxer=siff --disable-demuxer=smacker --disable-demuxer=sol --disable-demuxer=str --disable-demuxer=thp --disable-demuxer=tiertexseq --disable-demuxer=tta --disable-demuxer=txd --disable-demuxer=vmd --disable-demuxer=voc --disable-demuxer=wc3 --disable-demuxer=wsaud --disable-demuxer=wsvqa --disable-demuxer=xa --disable-demuxer=yuv4mpegpipe --enable-demuxer=matroska --enable-decoder=rawvideo --enable-decoder=h264
else
FFMPEG_FLAG=--cross-prefix=$(DLNA_CROSS_COMPILER) --target-os=linux --enable-cross-compile --arch=$(DLNA_ARCH) --prefix=$(DLNA_INSTALL_PREFIX) --enable-static --disable-shared  --enable-gpl --disable-network --disable-zlib --disable-vhook --disable-ffmpeg --disable-ffserver --disable-ffplay --enable-small --disable-encoders --disable-decoders --disable-muxers --disable-bsfs --disable-devices --enable-debug --disable-protocol=udp --disable-protocol=tcp --disable-protocol=rtp --disable-protocol=pipe --disable-protocol=http --disable-parser=cavsvideo --disable-parser=dca --disable-parser=dirac --disable-parser=dnxhd --disable-parser=mjpeg --disable-parser=mlp --disable-parser=pnm --disable-parser=vp3 --disable-demuxers --enable-demuxer=matroska --enable-decoder=rawvideo --enable-decoder=h264 \
			--enable-demuxer=mpegps --enable-demuxer=mpegts --enable-demuxer=mpegtsraw --enable-demuxer=mpegvideo \
			--enable-demuxer=m4v --enable-demuxer=mlp --enable-demuxer=mov --enable-demuxer=mp3 --enable-demuxer=mpc 			
endif

export DLNA_INSTALL_PREFIX
export DLNA_MAKE=$(MAKE)
export DLNA_INSTALL=$(INSTALL)
export ROOTDIR
export DLNA_LDFLAGS=$(LDFLAGS)
export DLNA_LNXOBJS=$(LNXOBJS)
export DLNA_CFLAGS=$(CFLAGS)
export DLNA_REDUCE
ifneq ($(DLNA_REDUCE), true)
export CFLAGS
else
export CFLAGS+=-DDLNA_TUNE_CODE_SIZE
endif
export LDFLAGS+=-L$(ROOTDIR)/lib/zlib
#export LDFLAGS+=-L$(DLNA_INSTALL_PREFIX)/lib

#-include $(ROOTDIR)/include/linux/config.h

#ifeq ($(CONFIG_USER_MINI_UPNPD),y)
#export USE_MINI_UPNPD = 1
#export CFLAGS+=-DUSE_MINI_UPNPD
#endif

#ifneq($(DLNA_REDUCE),true)
.PHONY: all jpeg exif id3tag ogg flac vorbis sqlite ffmpeg minidlna
all: jpeg exif id3tag ogg flac vorbis sqlite ffmpeg minidlna
#else
#.PHONY: all jpeg exif id3tag sqlite ffmpeg minidlna
#all: jpeg exif id3tag sqlite ffmpeg minidlna
#endif
#all: minidlna

romfs:
	$(ROMFSINST) ./tmp/sbin/minidlna /bin/minidlna

jpeg:
	cd jpeg-7 && ./configure $(LIBJPEG_FLAG)
	$(MAKE) -C jpeg-7
	$(MAKE) -C jpeg-7 install 

exif:
	cd libexif-0.6.20 && ./configure $(LIBEXIF_FLAG)
	$(MAKE) -C libexif-0.6.20
	$(MAKE) -C libexif-0.6.20 install

id3tag:
	cd libid3tag-0.15.1b && ./configure $(ID3TAG_FLAG)
	$(MAKE) -C libid3tag-0.15.1b
	$(MAKE) -C libid3tag-0.15.1b install
	
ogg:
	cd libogg-1.2.2 && ./configure $(OGG_FLAG)
	$(MAKE) -C libogg-1.2.2
	$(MAKE) -C libogg-1.2.2 install

flac:
	cd flac-1.2.1 && ./configure $(FLAC_FLAG)
	$(MAKE) -C flac-1.2.1
	$(MAKE) -C flac-1.2.1 install

vorbis:
	cd libvorbis-1.3.2 && ./configure $(VORBIS_FLAG)
	$(MAKE) -C libvorbis-1.3.2
	$(MAKE) -C libvorbis-1.3.2 install

sqlite:
	cd sqlite-3.7.2-src && ./configure $(SQLITE_FLAG)
	$(MAKE) -C sqlite-3.7.2-src
	$(MAKE) -C sqlite-3.7.2-src install

ffmpeg:
	cd ffmpeg-0.5.2 && ./configure $(FFMPEG_FLAG)
	$(MAKE) -C ffmpeg-0.5.2
	$(MAKE) -C ffmpeg-0.5.2 install

minidlna:
	$(MAKE) -C minidlna-1.0.19
	$(MAKE) -C minidlna-1.0.19 install
	$(DLNA_CROSS_COMPILER)strip $(DLNA_INSTALL_PREFIX)/sbin/minidlna
	
clean:
	-$(MAKE) -C jpeg-7 clean
	-$(MAKE) -C libexif-0.6.20 clean
	-$(MAKE) -C libid3tag-0.15.1b clean
	-$(MAKE) -C libogg-1.2.2 clean
	-$(MAKE) -C flac-1.2.1 clean
	-$(MAKE) -C libvorbis-1.3.2 clean
	-$(MAKE) -C sqlite-3.7.2-src clean
	-$(MAKE) -C ffmpeg-0.5.2 clean
	-$(MAKE) -C minidlna-1.0.19 clean
	$(RM) -r $(DLNA_INSTALL_PREFIX)/*

distclean:
	-$(MAKE) -C jpeg-7 distclean
	-$(MAKE) -C libexif-0.6.20 distclean
	-$(MAKE) -C libid3tag-0.15.1b distclean
	-$(MAKE) -C libogg-1.2.2 distclean
	-$(MAKE) -C flac-1.2.1 distclean
	-$(MAKE) -C libvorbis-1.3.2 distclean
	-$(MAKE) -C sqlite-3.7.2-src distclean
	-$(MAKE) -C ffmpeg-0.5.2 distclean
	-$(MAKE) -C minidlna-1.0.19 distclean
	$(RM) -r $(DLNA_INSTALL_PREFIX)/*
