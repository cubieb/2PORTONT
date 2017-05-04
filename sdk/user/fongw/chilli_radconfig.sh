#!/bin/sh

#PATH=/sbin:/bin:/usr/sbin:/usr/bin
#DAEMON=/usr/sbin/chilli
RADCONFIG=chilli_radconfig
NAME=chilli
DESC=chilli
CONFFILE=/var/chilli.conf
TEMPORAL_CONFFILE=/tmp/chilli.conf
NASID=00-18-84-D0-0E-C9
DHCPIF=wlan0-vap0
RADIUSSERVER1=radconfig01.fon.com
RADIUSSERVER2=radconfig02.fon.com
RADIUSSECRET=garrafon
#DEVICE=$(cat /etc/fon_device)
#VERSION=$(cat /etc/fon_version)
#REVISION=$(cat /etc/fon_revision)
RADIUSADMUSR=FONGW-fonera20-2.1.0.1
#RADIUSADMUSR=FON02-${DEVICE}-${VERSION}.${REVISION}
RADIUSADMPWD=chillispot
#LANIP=192.168.182.1
WHITELIST=/tmp/whitelist.dnsmasq
echo "chilli radconfig"

rm -rf $TEMPORAL_CONFFILE
$RADCONFIG -c /dev/null \
--radiusserver1="$RADIUSSERVER1" \
--radiusserver2="$RADIUSSERVER2" \
--radiussecret="$RADIUSSECRET" \
--adminuser="$RADIUSADMUSR" \
--adminpasswd="$RADIUSADMPWD" \
--radiusnasid="$NASID" \
--dhcpif $DHCPIF \
--wwwbin=/bin/true \
--ipup=/bin/true \
--ipdown=/bin/true \
> $TEMPORAL_CONFFILE

if [ -s $TEMPORAL_CONFFILE ]; then
	awk '/newdomain/ { print $2 }' $TEMPORAL_CONFFILE | awk -F. '/[a-zA-Z0-9\-\_]/ { print }' > /tmp/tt.tt
	#cat /tmp/tt.tt
	sed 's/,/\n/g' /tmp/tt.tt > $WHITELIST
	rm /tmp/tt.tt
	#cat $WHITELIST
	#remove IPs that may have skipped the validation
	cat $WHITELIST | grep -v [0-9]$ > $WHITELIST
	sed '/^newdomain/d' -i $TEMPORAL_CONFFILE
fi
diff $TEMPORAL_CONFFILE $CONFFILE > /dev/null
if [ ! $? -eq 0 ]; then 
	[ -s $TEMPORAL_CONFFILE ] && cp $TEMPORAL_CONFFILE $CONFFILE	
fi
exit 0
