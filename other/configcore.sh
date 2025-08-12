#! /bin/bash

ROOT_UID=0
etcfile="/etc/profile"
openlimit="ulimit -c unlimited"
dumpdir="/home/coredumps"

if [ $EUID != $ROOT_UID ];then
	echo "Error:please use root run the shell script."
	exit -1
fi

hostLineNo=`grep -n "$openlimit" $etcfile`
if [ "X$hostLineNo" == "X" ];then
	echo $openlimit >> /etc/profile
fi

mkdir -p $dumpdir
echo "${dumpdir}/core.%e.%p" > /proc/sys/kernel/core_pattern
