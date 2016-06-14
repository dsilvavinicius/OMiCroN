#!/bin/bash

for i in `seq 7 15`;
do
	filename="david_qtrmm_jul2011_$i.tar.gz"
	output="/media/vinicius/Expansion Drive3/Datasets/David/$filename"
	url="http://graphics.stanford.edu/dmich-archive/models/david/vrip/$filename"
	echo "Filename: $filename"
	echo "Output: $output"
	echo "Url: $url"
	curl -u ricardo-marroquim:rimadm4me -o "$output" "$url"
done
