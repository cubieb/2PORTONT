#!/bin/sh

while [ $(ps | grep -c omci_app) -lt 2 ]; do
	echo "omci_app is not ready."
	sleep 1
done

echo "omci_app is up."

if [ -f  /var/config/europa.data ]; then
	echo  "/var/config/europa.data existed, do nothing."
else
	echo  "/var/config/europa.data not existed, generate it."
	/bin/europacli open default
fi
		
MIBPONMODE=$(/etc/scripts/flash get PON_MODE | awk -F'=' '{print $2}')
insmod europa_drv PON_MODE=$MIBPONMODE \
