#!/bin/sh
# script to start ADSL Router/Modem

ADSLCTL=/bin/adslctrl

LAN_INTERFACE=br0
BR_INTERFACE=br0

GETMIB="flash get"
MPOAD="mpoad"
MPOACTL="mpoactl"
BR_UTIL=brctl

FW_SCRIPT=firewall.sh
PPPoE_SCRIPT=adsl-ppp-start.sh
DHCPD_SCRIPT=dhcpd.sh
DHCPC_SCRIPT=dhcpc.sh
DNS_SCRIPT=dnsmasq.sh

eval `flash check`
if [ $DS_BANK = '0' ]; then
    echo 'DS Bank Error! Reset to default ...'
    flash default ds
#    exit 1
fi

if [ $HS_BANK = '0' ]; then
    echo 'HS Bank Error! Reset to default ...'
    flash default hs
#    exit 1
fi

if [ $CS_BANK = '0' ]; then
    echo 'CS Bank Error! Reset to default ...'
    flash reset
fi

### Kao added
ENCAP_VC=0
ENCAP_LLC=1

RT1483=4

BR_VC=0
BR_LLC=1
RT_VC=3
RT_LLC=4
RIP_ENABLE=0
# RIP_ENABLE=1

eval `$GETMIB INIT_LINE INIT_SCRIPT ADSL_MODE`
# init adsl line
#if [ $INIT_LINE = 1 ]; then
#	$ADSLCTL startAdsl $ADSL_MODE
# sleep 30
#fi

eval `$GETMIB ELAN_MAC_ADDR LAN_IP_ADDR LAN_SUBNET BR_STP_ENABLED BR_AGEING_TIME`
# init LAN and bridge interface
ifconfig eth0 hw ether $ELAN_MAC_ADDR
ifconfig eth0 up
$BR_UTIL addbr $BR_INTERFACE
$BR_UTIL addif $BR_INTERFACE eth0
$BR_UTIL setfd $BR_INTERFACE 0
$BR_UTIL stp $BR_INTERFACE off
ifconfig $LAN_INTERFACE $LAN_IP_ADDR netmask $LAN_SUBNET

# init router stuff
$MPOAD &
$DNS_SCRIPT $LAN_INTERFACE
if [ $RIP_ENABLE = 0 ]; then
dhcpd.sh $LAN_INTERFACE
spppd &
fi

eval `$GETMIB ATM_VC_TBL NUM`
echo 'ATM_VC_TBL_NUM='$ATM_VC_TBL_NUM

num=0
brInit=0
rtInit=0
if [ $INIT_SCRIPT = 1 ]; then
while [ $num -lt $ATM_VC_TBL_NUM ]; do
	eval `$GETMIB ATM_VC_TBL $num`
	
	if [ $ATM_VC_TBL_ENCAP = $ENCAP_VC ]; then
		if [ $ATM_VC_TBL_CMODE = $RT1483 ]; then
			ENCAPS=$RT_VC
		else
			ENCAPS=$BR_VC
		fi
	else
		if [ $ATM_VC_TBL_CMODE = $RT1483 ]; then
			ENCAPS=$RT_LLC
		else
			ENCAPS=$BR_LLC
		fi
	fi
	
	if [ $ATM_VC_TBL_QoS = 'nrt-vbr' ]; then
		ATM_VC_TBL_QoS=vbr
	fi
	if [ $ATM_VC_TBL_QoS = 'ubr' ] || [ $ATM_VC_TBL_QoS = 'cbr' ]; then
		QOS_PARM="pcr=$ATM_VC_TBL_PCR"
	else
		QOS_PARM="pcr=$ATM_VC_TBL_PCR,scr=$ATM_VC_TBL_SCR,mbs=$ATM_VC_TBL_MBS"
	fi
	
	if [ $ATM_VC_TBL_CMODE != '3' ]; then
		echo $MPOACTL 'add' vc$num pvc $ATM_VC_TBL_VPI.$ATM_VC_TBL_VCI 'encaps' $ENCAPS  qos $ATM_VC_TBL_QoS:$QOS_PARM
		$MPOACTL add vc$num pvc $ATM_VC_TBL_VPI.$ATM_VC_TBL_VCI encaps $ENCAPS qos $ATM_VC_TBL_QoS:$QOS_PARM
		ifconfig vc$num up
	fi
	
	# enable ip forarding
	if [ $ATM_VC_TBL_CMODE != '0' ]; then
		if [ $rtInit = '0' ]; then
			rtInit=1
		fi
	fi
	
	if [ $ATM_VC_TBL_CMODE = '0' ]; then
		echo "1483 bridged"
		if [ $brInit = '0' ]; then
			brInit=1
			
			if [ $BR_STP_ENABLED = '0' ]; then
				$BR_UTIL stp $BR_INTERFACE off
				$BR_UTIL setfd $BR_INTERFACE 0
			else
				$BR_UTIL stp $BR_INTERFACE on
			fi
			
			$BR_UTIL setageing $BR_INTERFACE $BR_AGEING_TIME
		fi
		$BR_UTIL addif $BR_INTERFACE vc$num
	elif [ $ATM_VC_TBL_CMODE = '1' ]; then
		echo "1483 MER"
		if [ $ATM_VC_TBL_DHCP = '0' ]; then
			ifconfig vc$num $ATM_VC_TBL_MYIP
			route add default gw $ATM_VC_TBL_REMOTEIP
		else
			echo "Enabling support for a dynamically assigned IP (ISP DHCP)..."
			iptables -A INPUT -i vc$num -p UDP --dport 69 -d 255.255.255.255 \
			  -m state --state NEW -j ACCEPT
			
			echo "1" > /proc/sys/net/ipv4/ip_dynaddr
			$DHCPC_SCRIPT vc$num
		fi
		if [ $ATM_VC_TBL_NAPT = '1' ]; then
			## Enable NAT
			iptables -t nat -A POSTROUTING -o vc$num -j MASQUERADE
		fi
	elif [ $ATM_VC_TBL_CMODE = '2' ]; then
		echo "PPPoE"
		$PPPoE_SCRIPT $num $ATM_VC_TBL_PPPNAME $ATM_VC_TBL_PPPPWD
		
		if [ $ATM_VC_TBL_NAPT = '1' ]; then
			## Enable NAT
			iptables -t nat -A POSTROUTING -o ppp$num -j MASQUERADE
		fi
	elif [ $ATM_VC_TBL_CMODE = '3' ]; then
		echo "PPPoA"
		spppctl add $num pppoa $ATM_VC_TBL_VPI.$ATM_VC_TBL_VCI \
			encaps $ATM_VC_TBL_ENCAP qos $ATM_VC_TBL_QoS:$QOS_PARM \
			username $ATM_VC_TBL_PPPNAME password $ATM_VC_TBL_PPPPWD gw 1
		
		if [ $ATM_VC_TBL_NAPT = '1' ]; then
			## Enable NAT
			iptables -t nat -A POSTROUTING -o ppp$num -j MASQUERADE
		fi
	elif [ $ATM_VC_TBL_CMODE = '4' ]; then
		echo "1483 routed"
		if [ $ATM_VC_TBL_DHCP = '0' ]; then
			ifconfig vc$num $ATM_VC_TBL_MYIP -arp -broadcast \
				pointopoint $ATM_VC_TBL_REMOTEIP \
				netmask 255.255.255.255
			route add default gw $ATM_VC_TBL_REMOTEIP
		else
			echo "Enabling support for a dynamically assigned IP (ISP DHCP)..."
			iptables -A INPUT -i vc$num -p UDP --dport 69 -d 255.255.255.255 \
			  -m state --state NEW -j ACCEPT
			
			echo "1" > /proc/sys/net/ipv4/ip_dynaddr
			$DHCPC_SCRIPT vc$num
		fi
		if [ $ATM_VC_TBL_NAPT = '1' ]; then
			## Enable NAT
			iptables -t nat -A POSTROUTING -o vc$num -j MASQUERADE
		fi
	fi
	
	num=`expr $num + 1`
done
fi

if [ $RIP_ENABLE = 1 ]; then
zebra.sh
fi

wlan.sh

if [ $rtInit = '1' ]; then
	echo 1 > /proc/sys/net/ipv4/ip_forward
	if [ $RIP_ENABLE = 0 ]; then
	# Firewall
	$FW_SCRIPT start $LAN_INTERFACE
	## Create chain which blocks new connections, except if coming from inside(LAN)
	iptables -N block
	iptables -A block -m state --state ESTABLISHED,RELATED -j ACCEPT   
	iptables -A block -m state --state NEW -i $LAN_INTERFACE -j ACCEPT   
	iptables -A block -j DROP
	## link the block chain
	iptables -A INPUT -j block
	iptables -A FORWARD -j block
	fi
fi

webs &
snmpd &

