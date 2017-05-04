#!/bin/bash
# This shell script is designated to update soc.h at CONFIG_USE_PRELOADER_PARAMETERS=y
# It only considers being called from $(ROOT_DIR)/Makefile

# Check existence of $(ROOT_DIR) to make sure being called from $(ROOT_DIR)/Makefile 
# By Bo-Hung Wu Nov. 7, 2012

if [ "${ROOTDIR}" = "" ] ; then
    echo "Error: ROOTDIR is not set!!"
    exit -1
fi

soc_h_dst=${ROOTDIR}/${LINUXDIR}/include/soc/soc.h
soc_h_src=${PLR_INC}/soc.h

if [ -f ${soc_h_src} ] ; then
    echo "Checking SRC DST soc.h difference"
    if [ -f ${soc_h_dst} ] ; then
        result=`diff ${soc_h_src} ${soc_h_dst}`
    else
        result="${soc_h_dst} doesn't exist"
    fi

    if [ "${result}" = "" ] ; then
        echo "SRC & DST soc.h are the same, no update needed"
    else
        echo "SRC & DST soc.h are different"
        chmod a+w ${soc_h_dst} 2>/dev/null
        cp ${soc_h_src} ${soc_h_dst}
        chmod a-w ${soc_h_dst}
    fi
else 
    if [ -f ${soc_h_dst} ] ; then
        echo "INFO: No source soc.h valid for checking"
    else 
        echo "ERROR: soc.h is not valid at CONFIG_USE_PRELOADER_PARAMETERS=y"
        exit -1
    fi
fi

