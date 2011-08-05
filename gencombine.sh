#!/bin/bash
OUT=$1
rm -f $OUT
while true ; do
	shift 1
	if [ "$1" == "" ] ; then
		break;
	fi;
	echo "#include \"$1\"" >> $OUT
done

