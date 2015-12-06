#!/bin/bash
# This script can be used to check all X and Y dipoles on international station
# This makes it easier to walk through the field in a more logical way
# Version 1.0; 7-8-2015; M.J.Norden

#ANTENNAS="12 36 85 70 83 63 13 14 15 55 37 39 38 44 46 93 76 65 90 88 17 16 95 64 74 67 72 60 45 47 61 94 26 24 25 0 2 1 3 62 80 19 18 82 73 48 4 7 6 86 87 66 28 29 59 31 30 49 77 20 84 53 22 23 21 57 52 91 27 68 69 58 9 10 11 8 76 56 71 40 42 41 43 79 89 92 32 33 35 34 54 75 51 81 5 50" 

#Here you can select a group of 24 antennas
ANTENNAS="12 36 85 70 83 63 13 14 15 55 37 39 38 44 46 93 76 65 90 88 17 16 95 64" 
#ANTENNAS="74 67 72 60 45 47 61 94 26 24 25 0 2 1 3 62 80 19 18 82 73 48 4 7" 
#ANTENNAS="6 86 87 66 28 29 59 31 30 49 77 20 84 53 22 23 21 57 52 91 27 68 69 58" 
#ANTENNAS="9 10 11 8 78 56 71 40 42 41 43 79 89 92 32 33 35 34 54 75 51 81 5 50" 




echo "[ ----------- Select next LBA antenna ------------ ]"
echo "[     press any key to select next LBA antenna     ]"
echo "[       automatic LBA antennas power on            ]"

rspctl --rcumode=3
sleep 1
rspctl --rcuenable=1
sleep 1

counter=0

for idx in `echo $ANTENNAS` 
  do
  let counter+=1
  let "rcux = $idx*2"
  let "rcuy = $idx*2+1"

  echo " Antenna is " $idx
  echo " " $counter
#  echo " RCU x is " $rcux
#  echo " RCU y is " $rcuy

  rspctl --stati --sel=$rcux,$rcuy &
 
  # wait until key is pressed and kill gnuplot screen
  read -n 1 -s var_key
  PID=$!
  kill $PID
 
  done
