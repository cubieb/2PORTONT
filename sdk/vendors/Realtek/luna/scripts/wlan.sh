#!/bin/sh
# script to start WLAN

iwpriv wlan0 set_mib regdomain=1
ifconfig wlan0 hw ether 00e04c090706	
iwpriv wlan0 set_mib led_type=2	
iwpriv wlan0 set_mib ssid="8670_AP"  
iwpriv wlan0 set_mib opmode=16	
iwpriv wlan0 set_mib RFChipID=7	
iwpriv wlan0 set_mib Diversity=0	
iwpriv wlan0 set_mib DefaultAnt=0

iwpriv wlan0 set_mib TxPowerCCK=0,11
iwpriv wlan0 set_mib TxPowerOFDM=0,17
iwpriv wlan0 set_mib TxPowerCCK=1,11
iwpriv wlan0 set_mib TxPowerOFDM=1,17
iwpriv wlan0 set_mib TxPowerCCK=2,11
iwpriv wlan0 set_mib TxPowerOFDM=2,17
iwpriv wlan0 set_mib TxPowerCCK=3,11
iwpriv wlan0 set_mib TxPowerOFDM=3,17
iwpriv wlan0 set_mib TxPowerCCK=4,11
iwpriv wlan0 set_mib TxPowerOFDM=4,17
iwpriv wlan0 set_mib TxPowerCCK=5,11
iwpriv wlan0 set_mib TxPowerOFDM=5,17
iwpriv wlan0 set_mib TxPowerCCK=6,11
iwpriv wlan0 set_mib TxPowerOFDM=6,17
iwpriv wlan0 set_mib TxPowerCCK=7,11
iwpriv wlan0 set_mib TxPowerOFDM=7,17
iwpriv wlan0 set_mib TxPowerCCK=8,11
iwpriv wlan0 set_mib TxPowerOFDM=8,17
iwpriv wlan0 set_mib TxPowerCCK=9,11
iwpriv wlan0 set_mib TxPowerOFDM=9,17
iwpriv wlan0 set_mib TxPowerCCK=10,11
iwpriv wlan0 set_mib TxPowerOFDM=10,17
iwpriv wlan0 set_mib TxPowerCCK=11,11
iwpriv wlan0 set_mib TxPowerOFDM=11,17
iwpriv wlan0 set_mib TxPowerCCK=12,11
iwpriv wlan0 set_mib TxPowerOFDM=12,17
iwpriv wlan0 set_mib TxPowerCCK=13,11
iwpriv wlan0 set_mib TxPowerOFDM=13,17

iwpriv wlan0 set_mib bcnint=100
iwpriv wlan0 set_mib channel=6
iwpriv wlan0 set_mib basicrates=15
iwpriv wlan0 set_mib oprates=4095
iwpriv wlan0 set_mib autorate=1		
iwpriv wlan0 set_mib rtsthres=2347
iwpriv wlan0 set_mib fragthres=2346
iwpriv wlan0 set_mib expired_time=30000	
iwpriv wlan0 set_mib preamble=0		
iwpriv wlan0 set_mib hiddenAP=0		
iwpriv wlan0 set_mib dtimperiod=1            
iwpriv wlan0 set_mib aclnum=0                
iwpriv wlan0 set_mib aclmode=0		
iwpriv wlan0 set_mib authtype=2
iwpriv wlan0 set_mib encmode=0		

iwpriv wlan0 set_mib iapp_enable=1
iwpriv wlan0 set_mib band=3
iwpriv wlan0 set_mib shortretry=6
iwpriv wlan0 set_mib longretry=6

ifconfig wlan0 up
brctl addif br0 wlan0

#iwcontrol auth&
#auth wlan0 br0 auth /bin/wpa.conf&
