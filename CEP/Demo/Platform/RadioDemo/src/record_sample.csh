#!/bin/sh
#
# 03-09-2001  
#
# 
#
# 
export HOME=/home/wierenga
cd $HOME/ICT_KENNIS_DEMO/RadioCapture
./sound-recorder/src/sound-recorder -c 2 -s 8000 -k -P -S 0:01 -q /home/wierenga/ICT_KENNIS_DEMO/RadioCapture/WAVE/$1.wav 2>&1 > capture.out 
touch ./WAVE/$1.wav.ready

