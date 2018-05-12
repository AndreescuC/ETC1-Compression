#!/bin/bash

ETCPACK=./utils/etcpack
PPMCOMPARE=./build/compare_ppm

IMAGES_ORIG=./images
IMAGES_INOUT=./inout
DIRSRC=$DIRTEST
RESULTS=results_$DIRTEST.txt

# remove previous images
rm -rf $IMAGES_INOUT 
cp -r $IMAGES_ORIG $IMAGES_INOUT

# prepare OpenCL
module load libraries/cuda

# build the project
rm -r build/
mkdir -p build/
cd build && cmake ../$DIRSRC && make && cd ..

cp $DIRSRC/*.cl .

listIn=`find $IMAGES_INOUT -name "*.ppm*M"`
listCompressors=`find build/ -type f -executable -name "compress_*"`

# refresh results
rm -f $RESULTS
echo > $RESULTS

for binCompressor in $listCompressors
do
	echo "------> TEST SPEED -------->" >> $RESULTS
	echo "BIN: $binCompressor" >> $RESULTS
	$binCompressor $listIn >> $RESULTS
	echo "" >> $RESULTS

	# check quality
	echo "------> TEST QUALITY -------->" >> $RESULTS
	for fileIn in $listIn
	do
    	$ETCPACK $fileIn.pkm $IMAGES_INOUT
    	$PPMCOMPARE $fileIn $fileIn.ppm >> $RESULTS
	done
	echo "" >> $RESULTS
done

echo "------> GRADING -------->" >> $RESULTS
python utils/score.py $RESULTS >> $RESULTS

cat $RESULTS

# cleanup
rm -f *.cl
