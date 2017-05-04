#!/bin/sh

# Mount /var/config based mtd partitin name "config"
# $1: arguments for mount command (file system type and etc) 
# $2: mtd name
# $3: directory to be mounted onto

# Parameters validation
if [ $# != 3 ]; then
    echo "Argument is $#, but it should be 3"
    echo "Usage: $0 <arguments for mount command> <mtd_name> <directory>"
    echo "<arguments for mount command> e.g., \"-t jffs2\", or \"-t  yaffs2 -o tags-ecc-off\""
    echo "<mtd_name> e.g., \"config\""
    echo "<directory> e.g., \"/var/config\""
    exit 1
fi

# Check existence of destination direcotry 
if [ ! -d "$3" ]; then
    # Try to create it if it doesn't exist
    mkdir -p $3
    if [ $? != 0 ]; then
        echo "Error: directory $3 doesn't exist and cannot be created"
        # exit if fails
	exit 2
    fi
fi


# Find out mtd ID by name
cfgfs_mtd=`cat /proc/mtd | grep "$2" | sed 's/^mtd\(.*\):.*$/\1/g'`
echo "Mounting /dev/mtdblock"$cfgfs_mtd" onto $3 as the configuration data storage"

mtd_valid=`ls "/dev/mtdblock""$cfgfs_mtd"`
if [ "$mtd_valid" == "" ]; then
    echo "Warning: cannot find the corresponding MTD to mount."
    exit 2
fi

fs_ok="$3/"".cfgfs_ok_"
# Try mounting 
try_format="no"
mount $1 "/dev/mtdblock""$cfgfs_mtd" $3
# Try formating the paration if fails 
if [ $? != 0 ]; then
    echo "Error: failed to mount "/dev/mtdblock""$cfgfs_mtd" onto "$3""
    try_format="failed to mount"
else 
    fs_sig=`ls ${fs_ok}*`
    if [ "$fs_sig" == "" ]; then
        echo "The partition is fresh."
        echo "$3 is successfully created with `cat /proc/version`" 
        echo "" > "$fs_ok"`cat /proc/version | sed 's/ /_/g'`
    fi
fi

# if [ $try_format != 0 ]; then
if [ "$try_format" != "no" ]; then
    echo "Trying to format "/dev/mtd""$cfgfs_mtd"... "
    umount -f $3
    echo "Erasing partition... "
    flash_eraseall "/dev/mtd""$cfgfs_mtd"
    if [ $? != 0 ]; then
        echo "Error: failed to format "dev/mtd""$cfgfs_mtd""
        exit 3
    else
        echo "Format ok, trying to mount again..."
        mount $1 "/dev/mtdblock""$cfgfs_mtd" $3
        # exit if fails
        if [ $? != 0 ]; then
            echo "Error: Failed to mount "/dev/mtdblock""$cfgfs_mtd" onto "$3""
            exit 4
        else
            echo "$3 is successfully created with `cat /proc/version`" 
            echo "" > "$fs_ok"`cat /proc/version | sed 's/ /_/g'`
        fi
    fi
fi


