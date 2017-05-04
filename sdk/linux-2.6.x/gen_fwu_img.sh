#!/bin/sh
#Generate luna firmware upgarde image tar ball

f_list="fwu.sh rootfs uImage fwu_ver"
md5_result="md5.txt"
dst_dir="../images"
img_tar_file="img.tar"

cp fwu.sh $dst_dir
cp fwu_ver $dst_dir
cd $dst_dir
md5sum $f_list > $md5_result
tar -cf $img_tar_file $f_list $md5_result
ls -l $img_tar_file
