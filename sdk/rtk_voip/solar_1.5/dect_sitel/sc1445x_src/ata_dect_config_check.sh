#!/bin/bash
if test -e ./common/si_linux_autoconf.h ; then
	echo autoconf.h file exists
else
	echo autoconf.h file missing
	echo PLEASE BUILD LINUX image first!!!!
fi
