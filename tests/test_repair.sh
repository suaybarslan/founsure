#!/bin/bash



dir="Coding/"
orgf="test1"
basen=$orgf"_disk00"

for i in 0{1..9} {10..50} ; do
    file=$basen$i
    echo "processing..." $file 
    file2rep=$dir$file

	if [ -f $file2rep ] ; then
		mv $file2rep ./
	fi

	rm -rf $file2rep
	founsureRep -f test1
	diff $file2rep $file
	if [ $? -ne 0 ] ; then
    	echo "The file could not be repaired....."
    	exit 1
    fi
	rm -rf $file
done





