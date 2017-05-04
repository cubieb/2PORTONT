#!/bin/bash

echo "XXXXXXXXXXXXXXXXXX"
if [ "${ROOTDIR}" = "" ] ; then
    echo "Error: ROOTDIR is not set!!"
    exit -1
fi

slave=`grep -c "^CONFIG_ARCH_LUNA_SLAVE=y" ${ROOTDIR}/${LINUXDIR}/.config`
echo "slave=$slave"
if [ "$slave" = "0" ]; then
	cat ./tools/luna_kernel_debug.conf >> ${ROOTDIR}/${LINUXDIR}/.config
fi