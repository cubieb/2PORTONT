#!/bin/sh

if [ $# != 1 ] ; then
    printf "Usage: luna_build_test.sh \$sdk_dir\n"
    exit -1
fi

src_dir=$1
build_dir="build_test_"`date +%Y%m%d`

# create $build_dir under $src_dir
cd $src_dir
if [ -d $build_dir ]; then
    rm -rf $build_dir
fi
mkdir $build_dir

# preconfig for linux+ecos and nfjrom
# mlinux_pcfg_list=`ls -d $src_dir/vendors/Realtek/luna/conf2630/8696_nfjrom`
mlinux_pcfg_list=`ls -d vendors/Realtek/luna/conf2630/* | grep -v "DL"`
make_cmd="make -f Makefile"

for i in $mlinux_pcfg_list 
do 
    pcfg=`basename $i`
    if [ -d $i ]; then
        printf "Building for $pcfg: "
        $make_cmd preconfig2630_$pcfg > /dev/null 2>&1
        yes "x" | $make_cmd menuconfig_phase1 > /dev/null 2>&1
        yes "x" | $make_cmd linux_menuconfig > /dev/null 2>&1
        yes "x" | $make_cmd config_menuconfig > /dev/null 2>&1
        config/setconfig final > /dev/null 2>&1
#        yes "x" | $make_cmd menuconfig > /dev/null 2>&1
        make sdk.shell_tree > /dev/null 2>&1
        $make_cmd all > $build_dir/$pcfg.log 2>&1

        mkdir $build_dir/$pcfg
        if [ -f images/rootfs ] && [ -f images/uImage ]; then
            cp images/* $build_dir/$pcfg
            printf "PASSED\n"
            printf "$pcfg build PASSED\n" >> $build_dir/build_log
        else
            printf "FAILED\n"
            printf "$pcfg build FAILED\n" >> $build_dir/build_log
        fi
        rm -rf images
    fi
done

# dual linux precofig
# dual_linux_pcfg_list=`ls -d $src_dir/vendors/Realtek/luna/conf2630/DL8696RG_demo`
dual_linux_pcfg_list=`ls -d $src_dir/vendors/Realtek/luna/conf2630/* | grep "DL"`
make_cmd="make -f LDLMakefile"

for i in $dual_linux_pcfg_list 
do 
    pcfg=`basename $i`
    if [ -d $i ]; then
        printf "Building for $pcfg: "
        $make_cmd preconfig2630_$pcfg > /dev/null 2>&1
        yes "x" | $make_cmd menuconfig > /dev/null 2>&1
        $make_cmd all > $build_dir/$pcfg.log 2>&1

        mkdir $build_dir/$pcfg
        if [ -f images/rootfs ] && [ -f images/uImage ]; then
            cp images/* $build_dir/$pcfg
            printf "PASSED\n"
            printf "$pcfg build PASSED\n" >> $build_dir/build_log
        else
            printf "FAILED\n"
            printf "$pcfg build FAILED\n" >> $build_dir/build_log
        fi
        rm -rf images
    fi
done

