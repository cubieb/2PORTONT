#!/bin/sh

# luna firmware upgrade preparation script
#$1 image to be updated (0 or 1) 
#$2 image tar ball file name

case "$2" in
	/*)
		img="$2"
		;;
	*)
		img="`pwd`/$2"
		;;
esac
sentinel="fwu.sh"
md5_cmp="md5.txt"
md5_cmd="/bin/md5sum"
md5_tmp="md5_tmp" 
md5_rt_result="md5_rt_result.txt"

# Stop this script upon any error
set -e

# Parameter validation
if [ $# != 2 ]; then
    echo "Error: Incorrect arguments."
    echo "Usage: $0 <image ID to be updated> <image file>"
    echo "<image ID to be updated>: 0 | 1 "
    echo "<image file>: e.g., img.tar"
    exit 1
fi

if [ $1 != 0  ] && [ $1 != 1 ]; then
    echo "Error: <image ID to be updated>: 0 | 1 "
    echo "Usage: $0 <image ID to be updated> <image file>"
    exit 1
fi

if [ ! -f $2 ]; then
    echo "Error: $2 doesn't exist."
    echo "Usage: $0 <image ID to be updated> <image file>"
    exit 1
fi

tmp_dir="/tmp/fwu"
if [ -d $tmp_dir ]; then
    rm -rf $tmp_dir
fi
mkdir -p $tmp_dir

cd $tmp_dir 
tar -xf $img $sentinel $md5_cmp 
if [ $? != 0 ]; then 
    echo "Failed to extract $img, aborted image updating !"
    exit 1
fi

grep $sentinel $md5_cmp > $md5_tmp
$md5_cmd $sentinel > $md5_rt_result
diff $md5_rt_result $md5_tmp

if [ $? != 0 ]; then 
    echo "$sentinel md5_sum inconsistent, aborted image updating !"
    exit 1
fi

# Run firmware upgrade script extracted from image tar ball
sh $sentinel $1 $img 

