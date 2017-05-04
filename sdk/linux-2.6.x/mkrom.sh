#!/bin/sh
/rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-110915/bin/rsdk-linux-objcopy -O binary -R .note -R .comment -S vmlinux linux.bin
rm linux.bin.gz
gzip -9 linux.bin
../tools/u_boot_mkimage -A mips -O linux -T kernel -C gzip -a 80000000 -e 80000000 -n "Linux Kernel Image" -d linux.bin.gz uImage
cp uImage ~

key="0xa0000203"
echo "key:"$key

flash_base=0xbd000000
ImageOffsetHex=`echo "obase=16;ibase=10; $(($flash_base))" | bc`

../tools/packimg/genhead -i uImage -o uImage.hdr -k $key -f $ImageOffsetHex -a 0x80000000 -e 0x80000000
cat uImage.hdr uImage > uImage_new
ls -al uImage_new


key="0xa0000403"
echo "key:"$key
../tools/packimg/genhead -i ../rootfs -o rootfs.hdr -k $key -f $ImageOffsetHex -a 0x80000000 -e 0x80000000
cat rootfs.hdr ../rootfs > rootfs_new
ls -al rootfs_new

cat uImage_new rootfs_new > vm.img
ls -al vm.img

