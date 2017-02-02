#!/bin/bash

# David
#inputFolder="/media/vinicius/Expansion Drive3/Datasets/David/PlyFiles"
#outputFolder="/media/vinicius/Expansion Drive3/Datasets/David/PlyFilesNormals"

# StMathew
# inputFolder="/media/vinicius/Expansion Drive3/Datasets/StMathew/PlyFiles"
# outputFolder="/media/vinicius/Expansion Drive3/Datasets/StMathew/PlyFilesNormals"

# Atlas
inputFolder="/media/vinicius/Expansion Drive3/Datasets/Atlas/PlyFiles"
outputFolder="/media/vinicius/Expansion Drive3/Datasets/Atlas/PlyFilesNormals"

cd "/home/vinicius/Projects/PointBasedGraphics/Vrip/vrip/bin"

for inputFile in "$inputFolder/"*.ply; do
	filename=$(basename "$inputFile")
	outputFile="$outputFolder/$filename"
	echo -e "Calculating normals from $inputFile to $outputFile\n"
	./plynormals -a -f "$inputFile" > "$outputFile"
done