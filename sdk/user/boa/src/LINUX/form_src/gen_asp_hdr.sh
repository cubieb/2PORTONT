#!/bin/bash

MULTILANG_FINAL_HRD="./multilang_final.h"
START_PAR_STR="LANG_STR_NULL,"
END_PAR_STR="LANG_TAG_MAX"
ASP_HDR="./asp.h"
COMMENT1="//"
COMMENT2="/*"

cnt=0
num=0
i=0
	
test -f $ASP_HDR && rm -rf $ASP_HDR
while read -r line; do
	if [ "$line" = $START_PAR_STR ]; then
		echo "Prepare the asp.h ..."
		printf "["
		while IFS=, read -ra line; do
			num=${#line[@]}
			for ((i=0; i<$num; i++)); do
				line=$(echo ${line[$i]} | sed -e "s/ //g")
				if [ "$line" = $END_PAR_STR ]; then
					echo "]"
					break 2
				else
					#if [[ "$line" == $COMMENT1*  || "$line" == $COMMENT2* ]]; then
					#	echo "skip comment $line"
					#	continue
					#fi
					cnt=$((cnt+1))
					echo "#define $line \"$cnt\" \"$line\"" >> $ASP_HDR
					printf "."
				fi
			done
		done
	fi
done < $MULTILANG_FINAL_HRD
