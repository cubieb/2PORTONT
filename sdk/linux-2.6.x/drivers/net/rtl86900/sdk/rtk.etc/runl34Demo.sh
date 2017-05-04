#! /bin/sh

insmod netvd.ko
ifconfig eth0 up
#create new interface
echo eth0 0 0 > /proc/net/netvd/config;
echo eth0 1 0 > /proc/net/netvd/config;
#RG PORT
echo eth0 2 0 > /proc/net/netvd/config;
#PON PORT
echo eth0 3 0 > /proc/net/netvd/config;
echo eth0 4 0 > /proc/net/netvd/config;
echo eth0 5 0 > /proc/net/netvd/config;

#bridge 
ifconfig br0 down
brctl delbr br0
sleep 1
brctl addbr br0
brctl addif br0 eth0.0.0;
brctl addif br0 eth0.1.0;
brctl addif br0 eth0.4.0;
#brctl addif br0 eth0.5.0;
brctl addbr br1;
sleep 1
brctl addif br1 eth0.5.0;
brctl addbr br2;
sleep 1
brctl addif br2 eth0.3.0;

#config interface
ifconfig eth0.2 down
ifconfig eth0.3 down
ifconfig eth0.4 down
ifconfig eth0.5 down 

ifconfig eth0.0.0 hw ether 00:E0:4C:86:70:01;
ifconfig eth0.1.0 hw ether 00:E0:4C:86:70:01;
ifconfig eth0.5.0 hw ether 00:E0:4C:86:70:02;
ifconfig eth0.4.0 hw ether 00:E0:4C:86:70:01;
#ifconfig eth0.5.0 hw ether 00:E0:4C:86:70:01;
ifconfig eth0.3.0 hw ether 00:E0:4C:86:70:03;
ifconfig br0 hw ether 00:E0:4C:86:70:01;
ifconfig br1 hw ether 00:E0:4C:86:70:02;
ifconfig br2 hw ether 00:E0:4C:86:70:03;

ifconfig eth0.0.0 0.0.0.0 up;
ifconfig eth0.1.0 0.0.0.0 up;
ifconfig eth0.2.0 0.0.0.0 up;
ifconfig eth0.3.0 0.0.0.0 up;
ifconfig eth0.5.0 0.0.0.0 up;
ifconfig br0 192.168.1.1 netmask 255.255.255.0;
ifconfig br1 10.10.1.2 netmask 255.255.255.0;
ifconfig br2 10.10.2.2 netmask 255.255.255.0;
#enable ip forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward
#cibfug netfilter 
iptables -t filter -F
iptables -t nat -F 
iptables -t mangle -F 
iptables -P INPUT ACCEPT
#iptables -t nat -A POSTROUTING -j CONNMARK --set-mark 0x1
iptables -t nat -A POSTROUTING -o br1 -j MASQUERADE
iptables -t nat -A POSTROUTING -o br2 -j MASQUERADE
