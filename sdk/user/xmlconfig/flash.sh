#!/bin/ash
#
# usage: flash.sh [cmd] ...
#

DEFAULT_FILE="/etc/config_default.xml"
DEFAULT_HS_FILE="/etc/config_default_hs.xml"
LASTGOOD_FILE="/var/config/lastgood.xml"
LASTGOOD_HS_FILE="/var/config/lastgood_hs.xml"

# for array type in hw_setting
specific_mib_patten="(^HW(_|_WLAN0_|_WLAN1_)TX_POWER*)|(^HW_FON_KEYWORD$)"

rename_mib_patten="^HW_(NIC[0-1]|WLAN[0-1]_WLAN)_ADDR"
rename_mib_name="ELAN_MAC_ADDR"

hw_mib="^HW_|^SUPER_NAME$|^SUPER_PASSWORD$|^BOOT_MODE$|^ELAN_MAC_ADDR#|^WLAN_MAC_ADD$|^WAN_PHY_PORT$|^WIFI_SUPPORT$|^BYTE$|^WORD$|^DWORD$|^INT1$|^INT2$"
var=""


case "$1" in
  "all")
#  	echo "------ [$1] Display all settings ------"
  	if [ $# -eq 1 ] || [ "$2" = "hs" ]; then
  		/bin/xmlconfig -os -hs
	fi
	if [ $# -eq 1 ] || [ "$2" = "cs" ]; then
	        /bin/xmlconfig -os
	fi
	exit 0
  	;;
  "default")
#  	echo "------ [$1] Restore to default configurationg ------"
  	if [ "$2" = "cs" ]; then
		/bin/xmlconfig -def_mib
		/bin/xmlconfig -if $DEFAULT_FILE -nodef && /bin/xmlconfig -of $LASTGOOD_FILE
		echo "Reset CS to default configuration success."
	elif [ "$2" = "hs" ]; then
		/bin/xmlconfig -def_mib -hs
		/bin/xmlconfig -if $DEFAULT_HS_FILE -nodef && /bin/xmlconfig -of $LASTGOOD_HS_FILE
		echo "Reset HS to default configuration success."
	elif [ "$2" = "voip" ]; then
		/bin/xmlconfig -def_voip_mib
		/bin/xmlconfig -of $LASTGOOD_FILE
		echo "Reset VoIP to default configuration success."
	else
		echo "Restore to default configurationg fail."	
		/bin/sh $0 -h
		exit 1
	fi
	echo "Please reboot system."
	exit 0
	;;
  "get" | "gethw")
# 	echo "------ [$1] Get a specific mib parameter from flash memory. ------"
	if [ "$2" != "" ]; then
		para=$2
		if [ `echo $para | egrep $rename_mib_patten` ]; then
			para=$rename_mib_name
		fi
		#echo "/bin/xmlconfig -g $para"
		if [ `echo $para | egrep $specific_mib_patten` ]; then
			/bin/xmlconfig -g $para | sed -r "s/$rename_mib_name+/$2/g" | sed -r "s/,+//g"
		else
			/bin/xmlconfig -g $para | sed -r "s/$rename_mib_name+/$2/g"
		fi
		if [ "$?" = "0" ]; then
			exit 0
		fi
	else
		/bin/sh $0 -h
		exit 1
	fi
	;;
  "set" | "sethw")
 # 	echo "------ [$1] Set a specific mib parameter into flash memory. ------"
  	if [ "$2" != "" ] && [ "$3" != "" ]; then
		para=$2
		if [ `echo $para | egrep $rename_mib_patten` ]; then
			$para=$rename_mib_name
		fi

		if [ $# -eq 3 ]; then
			var=$3
		else
			while [ $# -ge 3 ]
			do
				# for multiple decimal values: dec2hex and concatenate all setting value
				if [ "$3" = "08" ] || [ "$3" = "09" ]; then
					# 08 & 09 are not invalid octal number
					var=$var$3
				else
					var=$var`printf "%02x" $3`
				fi

				shift
				if [ $# -ge 3 ]; then var=$var","; fi
			done
		fi
		#echo "/bin/xmlconfig -s $para $var"

		/bin/xmlconfig -s $para $var
		if [ "`echo $2 | egrep $hw_mib`" = "" ]; then
			/bin/xmlconfig -of $LASTGOOD_FILE
		fi
  		/bin/xmlconfig -of -hs $LASTGOOD_HS_FILE && exit 0
	else
		/bin/sh $0 -h
		exit 1
	fi
	;;
  "-h")
		echo 'Usage: flash.sh [cmd]'
		echo 'cmd:'
		echo '  all <cs/hs>               : Show all settings.'
		echo '  default <cs/hs>           : Restore to default configuration.'
		echo '  get MIB-NAME              : get a specific mib parameter from flash memory.'
		echo '  set MIB-NAME MIB-VALUE    : set a specific mib parameter into flash memory.'
		echo 
		echo '  Note: When set the MIB_ARRAY or MIB_VALUE overflowed,'
		echo '        xmlconfig will truncate the redundant part.'
		echo '        Take signed integer for example:'
		echo '        1. Set value=-6442450944(0xfffffffe80000000),'
		echo '           and get value=-2147483648(0x80000000)'
		echo '        2. Set value=-2147483649(0xffffffff7fffffff),'
		echo '           and get value=2147483647(0x7fffffff)'
		echo '        3. Set value=2147483648(0x80000000),'
		echo '           and get value=-2147483648(0x80000000)'
		echo '        4. Set value=4294967296(0x100000000), and get value=0(0x0)'
		echo 
	;;
  *)
  	/bin/sh $0 -h
		exit 1
	;;
esac
