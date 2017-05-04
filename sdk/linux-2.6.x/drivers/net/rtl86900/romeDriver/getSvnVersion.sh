#!/bin/bash
echo `svn info $1 --xml | grep -e revision | sed -n '2p' | sed '1s/[^0-9]*//g;q'`
