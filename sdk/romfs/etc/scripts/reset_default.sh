if grep -q rst2dfl=1 /proc/cmdline; then
	rm /etc/config/lastgood.xml
	echo "reset to default"
else
	echo "no need to do reset to default"
fi
