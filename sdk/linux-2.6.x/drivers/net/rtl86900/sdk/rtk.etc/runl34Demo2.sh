#! /bin/sh
#for 86907 
	brctl delif br0 eth0.2
	brctl delif br0 eth0.3
	brctl delif br0 eth0.4
	brctl delif br0 eth0.5
	brctl delif br0 nas0_0
	#config interface
	ifconfig eth0.2 down
	ifconfig eth0.3 down
	ifconfig eth0.4 down
	ifconfig eth0.5 down 

	ifconfig eth0 up
	#create new interface
	echo eth0 0 0 > /proc/net/netvd/config;
	echo eth0 1 0 > /proc/net/netvd/config;
	echo eth0 2 0 > /proc/net/netvd/config;
	echo eth0 3 100 > /proc/net/netvd/config;
	#PON PORT
	echo eth0 4 200 > /proc/net/netvd/config;
	#RG PORT
	#echo eth0 5 0 > /proc/net/netvd/config;

	brctl addif br0 eth0.0.0;
	brctl addif br0 eth0.1.0;
	brctl addif br0 eth0.2.0;
	brctl addbr br1;
	sleep 1
	brctl addif br1 eth0.3.100;
	brctl addbr br2;
	sleep 1
	brctl addif br2 eth0.4.200;

	ifconfig eth0.0.0 hw ether 00:E0:4C:86:70:01;
	ifconfig eth0.1.0 hw ether 00:E0:4C:86:70:01;
	ifconfig eth0.2.0 hw ether 00:E0:4C:86:70:01;
	ifconfig eth0.3.100 hw ether 00:E0:4C:86:70:02;
	ifconfig eth0.4.200 hw ether 00:E0:4C:86:70:03;
	ifconfig br0 hw ether 00:E0:4C:86:70:01;
	ifconfig br1 hw ether 00:E0:4C:86:70:02;
	ifconfig br2 hw ether 00:E0:4C:86:70:03;

	ifconfig eth0.0.0 0.0.0.0 up;
	ifconfig eth0.1.0 0.0.0.0 up;
	ifconfig eth0.2.0 0.0.0.0 up;
	ifconfig eth0.3.100 0.0.0.0 up;
	ifconfig eth0.4.200 0.0.0.0 up;
	#ifconfig eth0.5.0 0.0.0.0 up;
	ifconfig br0 192.168.1.1 netmask 255.255.255.0;
	ifconfig br1 10.10.1.2 netmask 255.255.255.0;
	ifconfig br2 10.10.2.2 netmask 255.255.255.0;
	#enable ip forwarding
	echo 1 > /proc/sys/net/ipv4/ip_forward
	#config netfilter 
	iptables -t filter -F
	iptables -t nat -F 
	iptables -t mangle -F 
	iptables -P INPUT ACCEPT
	iptables -t nat -A POSTROUTING -o br1 -j MASQUERADE
	iptables -t nat -A POSTROUTING -o br2 -j MASQUERADE

#for policy route test
	#iptables -t mangle -N rule
	#iptables -t mangle -A rule -s 192.168.1.10 -j MARK --set-mark 1 
	#iptables -t mangle -A rule -j ACCEPT
	#iptables -t mangle -A PREROUTING -j rule

	#ip route add table 1 default via 10.10.1.1 dev br1 
	#ip rule add fwmark 1 table 1

