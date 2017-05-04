#! /bin/sh
# $Id: iptables_flush.sh,v 1.1 2008/05/23 09:06:59 adsmt Exp $
IPTABLES=iptables

#flush all rules owned by miniupnpd
$IPTABLES -t nat -F MINIUPNPD
$IPTABLES -t filter -F MINIUPNPD

