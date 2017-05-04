#!/bin/sh

MULTI_KEY=n
IC8671=n
IC8672=y

if [ $CONFIG_E8_DOUBLE_IMAGE ]; then
	ROOT_ADDR=0xbd100000
else	# single image
	if [ $CONFIG_IMAGE_TINY ]; then
		ROOT_ADDR=0xbd100000
	else
		ROOT_ADDR=0xbd300000
	fi
fi

if [ $# -eq 0 ]; then
    if [ $MULTI_KEY != "y" ]; then
        key="0xa0000003"
    elif [ $IC8671 = "y" ]; then
        key="0xa0000003"
    elif [ $IC8672 = "y" ]; then
        key="0xa000001c"
    else
        echo "IC type don't specified"
        key="0xa0000003"
    fi
elif [ $# -eq 1 ]; then
    case $1 in
        "IC8671")
            key="0xa0000003";;
        "IC8672")
            key="0xa000001c";;
        "IC8671B")
            key="0xa000001c";;
        "IC8671B_costdown")
            key="0xa000001c";;
        *)
            echo "bad IC type"
            key="0xa0000003";;
    esac
else
    echo "too many parameter"
    key="0xa0000003"
fi

echo "key:"$key

${CROSS_COMPILE}strip vmlinux --remove-section=.bss
${CROSS_COMPILE}objcopy -S -O binary vmlinux vmlinux.bin
lzma e vmlinux.bin vmlinux.lzma
ls vmlinux.lzma -l
../tools/packimg/kfspad ../rootfs vmlinux.lzma vmbin.img
../tools/packimg/genhead  -i vmbin.img -o vm.hdr -k $key -f $ROOT_ADDR  -a 0x80000000 -e 0x80000000
cat vm.hdr vmbin.img > vm.img
../tools/packimg/kfspad vm.hdr vmlinux.lzma vmbin_tmp.img
../tools/packimg/kfspad ../rootfs vmbin_tmp.img vmbin2.img
rm vmbin_tmp.img
../tools/packimg/genhead  -i vmbin2.img -o vm2.hdr -k $key -f $ROOT_ADDR  -a 0x80000000 -e 0x80000000
cat vm2.hdr vmbin2.img > vm2.img

ls -al vm2.img
