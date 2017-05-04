#!/bin/sh

# luna firmware upgrade  script
# $1 image destination (0 or 1) 
# Kernel and root file system images are assumed to be located at the same directory named uImage and rootfs respectively
# ToDo: use arugements to refer to kernel/rootfs location.

k_img="uImage"
r_img="rootfs"
img_ver="fwu_ver"
md5_cmp="md5.txt"
md5_cmd="/bin/md5sum"
#md5 run-time result
md5_tmp="md5_tmp" 
md5_rt_result="md5_rt_result.txt"

# Stop this script upon any error
set -e

echo "Updating image $1 with file $2"

# Find out kernel/rootfs mtd partition according to image destination
k_mtd="/dev/"`cat /proc/mtd | grep \"k"$1"\" | sed 's/:.*$//g'`
r_mtd="/dev/"`cat /proc/mtd | grep \"r"$1"\" | sed 's/:.*$//g'`
echo "kernel image is located at $k_mtd"
echo "rootfs image is located at $r_mtd"

# Extract kernel image
tar -xf $2 $k_img -O | md5sum | sed 's/-/'$k_img'/g' > $md5_rt_result
# Check integrity
grep $k_img $md5_cmp > $md5_tmp
diff $md5_rt_result $md5_tmp

if [ $? != 0 ]; then
    echo "$k_img""md5_sum inconsistent, aborted image updating !"
    exit 1
fi

# Extract rootfs image
tar -xf $2 $r_img -O | md5sum | sed 's/-/'$r_img'/g' > $md5_rt_result
# Check integrity
grep $r_img $md5_cmp > $md5_tmp
diff $md5_rt_result $md5_tmp

if [ $? != 0 ]; then
    # rm $r_img
    echo "$r_img""md5_sum inconsistent, aborted image updating !"
    exit 1
fi

echo "Integrity of $k_img & $r_img is okay, start updating"

# Erase kernel partition 
flash_eraseall $k_mtd
# Write kernel partition
echo "Writing $k_img to $k_mtd"
tar -xf $2 $k_img -O > $k_mtd

# Erase rootfs partition 
flash_eraseall $r_mtd
# Write rootfs partition
echo "Writing $r_img to $r_mtd"
tar -xf $2 $r_img -O > $r_mtd


# Write image version information 
tar -xf $2 $img_ver 
nv setenv sw_version"$1" "`cat $img_ver`"

# Clean up temporary files
rm -f $md5_cmp $md5_tmp $md5_rt_result $img_ver $2

# Post processing (for future extension consideration)

echo "Successfully updated image $1!!"

