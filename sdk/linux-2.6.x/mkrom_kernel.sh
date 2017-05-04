#!/bin/sh

MULTI_KEY=n
IC8671=n
IC8672=y

if [ $# -eq 0 ]; then
    if [ $MULTI_KEY != "y" ]; then
        key="0xa0000003"
    elif [ $IC8671 = "y" ]; then
        key="0xa0000003"
    elif [ $IC8672 = "y" ]; then
        key="0xa0000001c"
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
../tools/packimg/genhead  -i vmlinux.lzma -o vm.hdr -k $key -f 0xbfc30000  -a 0x80000000 -e 0x80000000
cat vm.hdr vmlinux.lzma > vm.img
ls -al vm.img
