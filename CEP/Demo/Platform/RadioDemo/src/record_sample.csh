#!/bin/sh
#
# 03-09-2001  
#
# 
#
# 

cd $HOME/RadioDemo
#./sound-recorder/src/sound-recorder -c 1 -s 8000 -b 16 -k -P -S 0:01 -q ./WAVE/$1.wav 2>&1 > capture.out 
./vplay/vrec  -S -w -s 8000 -t 1 -b 16 ./WAVE/$1.wav 2>&1 > capture.out 
#touch ./WAVE/$1.wav.ready

