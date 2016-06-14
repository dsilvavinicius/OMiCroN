#!/bin/bash

inputFolder="/media/vinicius/Expansion Drive3/Datasets/David/PlyFiles"
outputFolder="/media/vinicius/Expansion Drive3/Datasets/David/PlyFilesNormals"

cd "/home/vinicius/Projects/PointBasedGraphics/Vrip/vrip/bin"

for inputFile in "$inputFolder/"*.ply; do
	filename=$(basename "$inputFile")
	outputFile="$outputFolder/$filename"
	echo -e "Calculating normals from $inputFile to $outputFile\n"
	./plynormals -a "$inputFile" > "$outputFile"
done