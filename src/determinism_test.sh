#!/bin/bash
#Determinism test! 
#NOTE: COPY SCRIPT TO BINARY DIR BEFORE RUNNING TO AVOID SEGFAULT
#We compare the output of infinite GoL simulations to a known-good* master file
#until (if ever) we get an output that doesn't match the master file. This signals
#a bug causing a break in determinism.
BINDIR="../"
DATADIR="../data/"

if [ ! -e gameoflife ]; then
	echo "Copy this script to the dir containing \"gameoflife\" binary before running!"
	exit
fi

if [ "$1" == "" ]; then
	threads="2"
else
	threads="$1"
fi

#generate the control
gameoflife -l data/r-pentomino.life -o control.life -g 15000 -h 304 -t "$threads" > /dev/null
echo "Created control.life file."

totalIterations=1000
iterations=0
while [ $iterations -lt $totalIterations ]; do
	./gameoflife -l data/r-pentomino.life -o test.life -g 15000 -h 304 -t "$threads" > /dev/null
	diff test.life control.life
	if [ $? != 0 ]; then
		echo "Test failed!"
		break;
	else
		echo "Test passed!"
	fi
	let iterations+=1
	sleep 1
done

echo "Test exited after $iterations iterations."

#cleanup 
rm test.life
rm control.life
