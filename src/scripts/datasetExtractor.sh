#!/bin/bash

path="/media/vinicius/Expansion Drive3/Datasets/David"
outputPath="$path/PlyFiles"

for i in `seq 1 15`;
do
	filename="$path/david_qtrmm_jul2011_$i.tar.gz"

	echo "Filename: $filename"

	tar -zxvf "$filename" -C "$outputPath"
done