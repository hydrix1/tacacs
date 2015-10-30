#!/bin/bash

for d in test_configs/*; do
    cd $d
    echo "========================"
    echo "===   `basename $d`"
    echo "========================"
    for s in *.script; do
	b=`basename $s .script`
	echo "===   $b   ==="
	awk '{print "      "$0}' $s
	if [ -f $b.input ]; then
	    echo "+ input:"
	    awk '{print "      "$0}' $b.input
	fi
	res=0
	if [ -f $b.output ]; then
	    echo "= output:"
	    awk '{print "      "$0}' $b.output
	    res=1
	fi
	if [ -f $b.error ]; then
	    echo "= error:"
	    awk '{print "      "$0}' $b.error
	    res=1
	fi
	if [ $res -eq 0 ]; then
	    echo "**************************"
	    echo "*******             ******"
	    echo "******* NO RESULTS! ****** (for $d/$b)"
	    echo "*******             ******"
	    echo "**************************"
	fi
    done
    cd ../..
done

