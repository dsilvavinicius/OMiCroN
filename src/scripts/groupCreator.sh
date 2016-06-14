#!/bin/bash

path="/media/vinicius/Expansion Drive3/Datasets/David/PlyFiles"
groupFile="$path/David.gp"

for filename in "$path/"*.ply; do
	echo "Filename: $filename"
    echo -e "\"$filename\"" >> "$groupFile"
done