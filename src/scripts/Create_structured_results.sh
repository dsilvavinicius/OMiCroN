#!/bin/bash

FILE=FastParallelOctreeStressTest.log

while read line; do
	firstToken=$(awk '{print $1;}' $line)
	if [ "$firstToken" = "Threads:" ]; then
		echo "$line"
	fi

	#case "$firstToken" in
	#	test)
	#	;;
	#esac
	#for token in "${line[@]}"; do
	#	echo $token
	#done
done < $FILE