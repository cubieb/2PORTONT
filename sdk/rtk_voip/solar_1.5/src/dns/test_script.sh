#!/bin/sh

#./dns_task &
while [ true ]; do 
	./dns_test2 www.hinet.net &
	./dns_test www.hinet.net &
	./dns_test www.hinet.net &
	./dns_test www.kimo.com.tw &
	./dns_test3 www.hinet.net &
	./dns_test2 www.kimo.com.tw &
	./dns_test3 www.seed.net.tw &
	./dns_test2 www.nctu.edu.tw &
	./dns_test3 www.google.com.tw &
	./dns_test2 www.msn.com.tw &
	./dns_test3 www.hinet.net &
	./dns_test2 www.kimo.com.tw &
	./dns_test3 www.seed.net.tw &
	./dns_test2 www.nctu.edu.tw &
	./dns_test3 www.google.com.tw &
	./dns_test2 www.msn.com.tw &
	./dns_test www.msn.com.tw &
	./dns_test www.hinet.net &
	./dns_test www.kimo.com.tw &
	./dns_test dump.dns_task.rtk_voip
#	./dns_test exit.dns_task.rtk_voip
#	sleep 10
	break
done
