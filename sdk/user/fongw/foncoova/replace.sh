#!/bin/bash
# $Id: replace.sh,v 1.1 2012/09/20 03:39:12 paula Exp $
source=$2
dest=$1
if [ ! -d $(dirname $dest) ]; then
	mkdir -p $(dirname $dest)
fi
cp $source $dest
