#!/bin/bash

# David
# inputFolder="/media/vinicius/Expansion Drive3/Datasets/David/PlyFilesNormals"
# outputFolder="/media/vinicius/Expansion Drive3/Datasets/David/PlyFilesFlippedNormals"

inputFolder="/media/vinicius/Expansion Drive3/Datasets/StMathew/PlyFilesNormals"
outputFolder="/media/vinicius/Expansion Drive3/Datasets/StMathew/PlyFilesFlippedNormals"

cd "/home/vinicius/Projects/PointBasedGraphics/Vrip/vrip/bin"

for inputFile in "$inputFolder/"*.ply; do
	filename=$(basename "$inputFile")
	outputFile="$outputFolder/$filename"
	echo -e "Flipping normals from $inputFile to $outputFile\n"
	./plyflip -n "$inputFile" > "$outputFile"
done