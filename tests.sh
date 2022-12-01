#! /bin/sh

testFiles=$(ls *.um)


for file in $testFiles ; do
    echo "running test with $file: "
    filename=$(echo $file | sed -E 's/(.*).um/\1/')
    if [ -f $filename.0 ] ; then
        if [ -f $filename.1 ] ; then
            ./um $file < $filename.0 > $filename.1
            um $file < $filename.0 > theirs.out
            diff $filename.1 theirs.out
        else
            ./um $file < $filename.0
            um $file < $filename.0
            echo "no output"
        fi
    else
        if [ -f $filename.1 ] ; then
            ./um $file > $filename.1
            um $file > theirs.out
            diff $filename.1 theirs.out
        else
            ./um $file
            echo "no input or output"
        fi
    fi

done