#!/bin/sh

pon_mode=`flash get PON_MODE`
if [ -d /proc/rg ]; then
	drvMode=1
else
	drvMode=0
fi
if [ "$pon_mode" == "PON_MODE=1" ]; then
	dualMgmt=`flash get DUAL_MGMT_MODE | sed 's/DUAL_MGMT_MODE=//g'`
    if [ -f  /lib/modules/omcidrv.ko ]; then
        insmod /lib/modules/omcidrv.ko
        if [ "$drvMode" = "0" ]; then
			if [ -f /lib/modules/pf_rtk.ko ]; then
				insmod /lib/modules/pf_rtk.ko
			else
				echo "Warning no exist rtk platform"
			fi
		else
			if [ -f /lib/modules/pf_rg.ko ]; then
				insmod /lib/modules/pf_rg.ko
			else
				echo "Warning no exist rg platform"
			fi

			if [ "$dualMgmt" == "1" ]; then
				if [ -f /etc/rtk_tr142.sh ]; then
					/etc/rtk_tr142.sh
				else
					echo "warning no exit rtk_tr142.sh"
				fi
			fi
		fi
    else
        echo "Warning no insert OMCI module in GPON mode"
    fi
fi

