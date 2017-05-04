#!/bin/sh
PIDFILE=/var/run/dhcrelay.pid
#eval `flash get UPNP`
flash get UPNP
if [ $1 = 'bound' ] || [ $1 = 'renew' ]; then
	/var/udhcpc/udhcpc.$interface	
	if [ "$ip" != '' ]; then
		if [ $UPNP = 1 ]; then 
			/bin/upnpctrl down $interface br0
			/bin/upnpctrl up $interface br0
		fi
		/bin/updateddctrl $interface 1
		/bin/ethctl route add		
		if [ -f $PIDFILE ]; then
			PID=`cat $PIDFILE`
			echo "Kick dhcrelay to sync interface-1"
			kill -17 $PID		
		fi
	fi
elif [ $1 = 'deconfig' ]; then
	/etc/scripts/udhcpc.deconfig
	
elif [ $1 = 'nak' ]; then
	echo "dhcp nak event occurs"
fi


