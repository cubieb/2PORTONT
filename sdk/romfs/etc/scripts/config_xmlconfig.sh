#!/bin/ash
#
# usage: config-xmlconfig.sh [option] ...
#

DEFAULT_FILE="/etc/config_default.xml"
DEFAULT_HS_FILE="/etc/config_default_hs.xml"
LASTGOOD_FILE="/var/config/lastgood.xml"
LASTGOOD_HS_FILE="/var/config/lastgood_hs.xml"
CONFIGD_RUNFILE="/var/run/configd.pid"
SHELL_NAME="ash"

case "$1" in
  "-b")
  	echo "------ [$1]Bootup_config ------"
  	wait_count=10
	while [  $wait_count -gt 0 ]; do
		[ -f $CONFIGD_RUNFILE ] && break
		echo "Wait for configd initialize 'MsgQ' and 'Shm'... "
		sleep 1
		wait_count=$(( $wait_count - 1 ))
	done
	[ -f $CONFIGD_RUNFILE ] || echo "WARNING: Configd has not finish initiation, xmlconfig may has some problem!"

	/bin/xmlconfig -def_mib && /bin/xmlconfig -def_mib -hs && echo "Pre-fetch mib data from program default done." || echo "Pre-fetch mib data from program default FAIL."
	[ -f $LASTGOOD_HS_FILE ]  && /bin/xmlconfig -c $LASTGOOD_HS_FILE
	if [ "$?" = '0' ]; then
		/bin/xmlconfig -def $DEFAULT_HS_FILE -if $LASTGOOD_HS_FILE
	else
		/bin/xmlconfig -nodef -if $DEFAULT_HS_FILE && /bin/xmlconfig -of -hs $LASTGOOD_HS_FILE
	fi
	if [ "$?" = '0' ]; then
		echo "[xml_INFO]: Load HS configuration success."
	else
		echo "[xml_ERR]: Load HS configuration FAIL."
	fi
	[ -f $LASTGOOD_FILE ] && /bin/xmlconfig -c $LASTGOOD_FILE
	if [ "$?" = '0' ]; then
		/bin/xmlconfig -def $DEFAULT_FILE -if $LASTGOOD_FILE
	else
		/bin/xmlconfig -nodef -if $DEFAULT_FILE && /bin/xmlconfig -of $LASTGOOD_FILE
	fi
        if [ "$?" = '0' ]; then
                echo "[xml_INFO]: Load CS configuration success."
        else
                echo "[xml_ERR]: Load CS configuration FAIL."
		exit 1
        fi
	exit 0
  	;;
  "-c")
  	echo "------ [$1]Check_config ------"
  	if [ "$2" != "" ]; then
  		/bin/xmlconfig -c $2
  		if [ "$?" = "0" ]; then
			echo "[xml_INFO]: The input configuration file is available."
			exit 0
		fi
	else
		echo "[xml_ERR]: No input configuration file."
		/bin/$SHELL_NAME $0 -h
		exit 1
	fi
	echo "[xml_ERR]: The input configuration file is invalid."
	exit 1
  	;;
  "-d")
  	echo "------ [$1]Default_config ------"
	/bin/xmlconfig -def_mib && /bin/xmlconfig -if $DEFAULT_FILE -nodef && /bin/xmlconfig -of $LASTGOOD_FILE
	if [ "$?" = "0" ]; then
		echo "[xml_INFO]: Reset to default configuration success, please reboot the system."
		exit 0
	else
		rm $LASTGOOD_FILE
		echo "[xml_ERR]: Reset to default configuration FAIL!!"
		echo "Sysyem has removed the current setting, please reboot to reset to default."
		exit 1
	fi
	;;
  "-l")
  	echo "------ [$1]Load_input_config ------"
  	if [ "$2" != "" ] && [ -s $2 ]; then
  		/bin/xmlconfig -c $2 && cp $2 $LASTGOOD_FILE
		if [ "$?" = "0" ]; then
			echo "[xml_INFO]: Load the configuration file success, please reboot the system."
			exit 0;
		fi
	else
		echo "[xml_ERR]: No input configuration file."
		/bin/$SHELL_NAME $0 -h
	fi
	echo "[xml_ERR]: Upload the configuration file FAIL!! Need to Reboot~"
	exit 1
	;;
  "-s")
  	echo "------ [$1]Save_config ------"
	if [ "$2" != "" ]; then
 		/bin/xmlconfig -of $2 && /bin/xmlconfig -c $2
 		if [ "$?" = "0" ]; then
			echo "[xml_INFO]: Save cofiguration to $2 success."
			exit 0
		fi
	else
		echo "[xml_ERR]: No input configuration file."
		/bin/$SHELL_NAME $0 -h
	fi
	echo "[xml_ERR]: Save cofiguration FAIL."
	exit 1
	;;
  "-u" | "-u cs")
	echo "------ [$1]Update cs setting ------"
	/bin/xmlconfig -of $LASTGOOD_FILE
	if [ "$?" = "0" ]; then
		echo "[xml_INFO]: Update cs configuration success."
		exit 0
	fi
	echo "[xml_ERR]: Update cs configuration FAIL!!"
	exit 1
	;;
  "-u hs")
	echo "------ [$1]Update hs setting ------"
	/bin/xmlconfig -of -hs $LASTGOOD_HS_FILE
	if [ "$?" = "0" ]; then
		echo "[xml_INFO]: Update hs configuration success."
		exit 0
	fi
	echo "[xml_ERR]: Update hs configuration FAIL!!"
	exit 1
	;;
  "-h")
	echo 'usage: config_xmlconfig.sh [option]...'
	echo 'options:'
	echo '  -h        : print this help'
	echo '  -b        : Boot up process with configuration file'
	echo '  -c <file> : Check input configuration file'
	echo '  -d        : Restore to default configuration'
	echo '  -l <file> : Load from input file'
	echo '  -s <file> : Save current configuration to specific file'
	echo '  -u cs/hs  : Update configuration to flash'
  	;;
  *)
  	echo "[xml_ERR]: insufficient arguments"
  	/bin/$SHELL_NAME $0 -h
	exit 1
	;;
esac
