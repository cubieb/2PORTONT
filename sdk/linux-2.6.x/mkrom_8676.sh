#!/bin/sh

MULTI_KEY=n
IC8671=n
IC8672=y
key="0xa0000003"
echo "key:"$key

flash_base=0xbfc00000
ImageOffsetHex=`echo "obase=16;ibase=10; $(($flash_base+$1))" | bc`

${CROSS_COMPILE}strip vmlinux --remove-section=.bss
${CROSS_COMPILE}objcopy -S -O binary vmlinux vmlinux.bin
lzma e vmlinux.bin vmlinux.lzma
ls vmlinux.lzma -l
../tools/packimg/kfspad ../rootfs vmlinux.lzma vmbin.img
../tools/packimg/genhead  -i vmbin.img -o vm.hdr -k $key -f 0x$ImageOffsetHex  -a 0x80000000 -e 0x80000000
cat vm.hdr vmbin.img > vm.img
ls -al vm.img
