#!/bin/bash

rm -rf ./images/
tar -xvzf ./utils/images.tar.gz

# resize input image
listImages=`find ./images -name "*.ppm"`
for fileIn in $listImages
do
	echo "Convert $fileIn to 16M"
	convert $fileIn -resize 4096x4096 -quality 100 "$fileIn.16M" 
	echo "Convert $fileIn to 36M"
	convert $fileIn -resize 6144x6144 -quality 100 "$fileIn.36M"
	echo "Convert $fileIn to 64M"
	convert $fileIn -resize 8192x8192 -quality 100 "$fileIn.64M"
done
