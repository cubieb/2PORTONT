#!/bin/bash

action=$1
precfgdir=$2
rootdir=$3
user_config=$4
def_bb=$5
ufsbbdir="$6"

confbz=""
busyboxdir=""

#### when toolchain >= 447, busybox should >=1.22.1 #######  
busyfor447="busybox-1.22.1"
oldtoolchain="rsdk-1.5.9"
##### busybux version and directory #####
busybox_ver=(busybox-1.12.4 busybox-1.22.1)

busybox_conf=(CONFIG_USER_BUSYBOX_BUSYBOX1124 CONFIG_USER_BUSYBOX_BUSYBOX1221)
busybox_pref=(config_busybox config_busybox_1221)
######################################
function check_toolchain(){
toolchain=`grep -o "rsdk-[[:digit:]{,2}]\.[[:digit:]{,2}]\.[[:digit:]{,2}]" ${rootdir}/.config | head -n 1`

  if [[ $toolchain > $oldtoolchain ]] && [[ $busyboxdir < ${busyfor447} ]] ; then
						echo "ERROR"
					exit 1
	fi
}
####main########
if [ "$action" = "dir" ]; then
	confbz=`grep "^CONFIG_USER_BUSYBOX_BUSYBOX.*=y" $user_config | awk -F= '{print $1}'`
	for ((i=0; i<${#busybox_conf[@]}; i++)); do 
		if [ "$confbz" =  "${busybox_conf[$i]}" ]; then
			busyboxdir="${busybox_ver[$i]}"
			break
		fi
	done
	if [ "$busyboxdir" = "" ];then
			busyboxdir="$def_bb"
	else
			busyboxdir="$busyboxdir"
	fi

	echo "user/${busyboxdir}"

elif [ "$action" = "prepare" ]; then
	for ((i=0; i<${#busybox_pref[@]}; i++)); do 
	  #echo " cp -f $precfgdir/${busybox_pref[$i]} $rootdir/user/${busybox_ver[$i]}/.config "
		if [ -f $precfgdir/${busybox_pref[$i]} ] && [ -d $rootdir/user/${busybox_ver[$i]} ]; then
			cp -f $precfgdir/${busybox_pref[$i]} $rootdir/user/${busybox_ver[$i]}/.config
		fi
	done
	
elif [ "$action" = "master" ]; then
	for ((i=0; i<${#busybox_pref[@]}; i++)); do 
		if [ -f $precfgdir/master/${busybox_pref[$i]} ] && [ -d $rootdir/user/${busybox_ver[$i]} ]; then
			if [ "$ufsbbdir" = "" ]; then
				cp $precfgdir/master/${busybox_pref[$i]} $rootdir/user/${busybox_ver[$i]}/.config
			else
				cp $precfgdir/master/${busybox_pref[$i]} $ufsbbdir/user/${busybox_ver[$i]}/.config
			fi
		fi
	done
elif [ "$action" = "slave" ]; then
	for ((i=0; i<${#busybox_pref[@]}; i++)); do 
		if [ -f $precfgdir/slave/${busybox_pref[$i]} ] && [ -d $rootdir/user/${busybox_ver[$i]} ]; then
			if [ "$ufsbbdir" = "" ]; then
				cp $precfgdir/slave/${busybox_pref[$i]} $rootdir/user/${busybox_ver[$i]}/.config
			else
				cp $precfgdir/slave/${busybox_pref[$i]} $ufsbbdir/user/${busybox_ver[$i]}/.config
			fi
		fi
	done
elif [ "$action" = "check" ]; then
  confbz=`grep "^CONFIG_USER_BUSYBOX_BUSYBOX.*=y" $user_config | awk -F= '{print $1}'`
	for ((i=0; i<${#busybox_conf[@]}; i++)); do 
		if [ "$confbz" =  "${busybox_conf[$i]}" ]; then
			busyboxdir="${busybox_ver[$i]}"
			break
		fi
	done
	check_toolchain
	
fi
