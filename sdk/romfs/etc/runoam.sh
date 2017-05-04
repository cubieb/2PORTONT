#!/bin/sh

exeCmd="eponoamd "
i=0
result=`flash get EPON_LLID_TBL.$i | grep 'fail'`
while [ "$result" == "" ]
do
macaddr=`flash get EPON_LLID_TBL.$i | sed 's/macAddr=//g'`
macaddr0=`echo $macaddr | sed 's/:[0-9a-fA-F]*:[0-9a-fA-F]*:://g'`
macaddr1=`echo $macaddr | sed 's/:[0-9a-fA-F]*:://g' | sed 's/[0-9a-fA-F]*://g'`
macaddr2=`echo $macaddr | sed 's/^[0-9a-fA-F]*:[0-9a-fA-F]*://g' | sed 's/:://g'`
macaddr0=`echo "0x"$macaddr0`
macaddr1=`echo "0x"$macaddr1`
macaddr2=`echo "0x"$macaddr2`
macaddr=`printf %04x%04x%04x $macaddr0 $macaddr1 $macaddr2`

exeCmd=$exeCmd"-mac "$i" "$macaddr" "

i=$((i+1))
result=`flash get EPON_LLID_TBL.$i | grep 'fail'`
done

$exeCmd &

