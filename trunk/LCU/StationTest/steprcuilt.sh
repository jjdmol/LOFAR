#!/bin/bash
#
# Step through Antenna statistics with the RCU's
#
# Version 1.0   15 nov 2012   H.J. Meulman

declare el=0
echo "[ ------ Step throug Antenna statistics ------ ]"
echo "[ u = up        ][ n = down      ][ CTRL+C = stop ]"
echo "[ d = RCU  0:31 ][ e = RCU 32:63 ][ f = RCU 64:95 ]"
echo "[ 1 = RCU  0:7  ][ 2 = RCU  8:15 ][ 3 = RCU 16:23 ]"
echo "[ 4 = RCU 24:31 ][ 5 = RCU 32:39 ][ 6 = RCU 40:47 ]"
echo "[ 7 = RCU 48:55 ][ 8 = RCU 56:63 ][ 9 = RCU 64:71 ]"
echo "[ a = RCU 72:79 ][ b = RCU 80:87 ][ c = RCU 88:95 ]"
echo "[ 0 = all RCU's off ]          [ o = all RCU's on ]"

echo "all RCU's off"
rspctl --rcuenable=0 2>&1 /dev/null

for (( ; ; ))
do
   # Read the key and determine action
   read -n 1 -s var_key
   if [ "$var_key" == u ]; then
      let el+=1
      if [ $el -eq 29 ]; then
         let el-=1
      fi
   fi
   if [ "$var_key" == n ]; then
      let el-=1
      if [ $el -eq -1 ]; then
         let el+=1
      fi
   fi
   case $var_key in
   [0-9]*)
      let el=$var_key
      ;;
   esac
   if [ "$var_key" == a ]; then
      let el=10
   fi
   if [ "$var_key" == b ]; then
      let el=11
   fi
   if [ "$var_key" == c ]; then
      let el=12
   fi
   if [ "$var_key" == d ]; then
      let el=13
   fi
   if [ "$var_key" == e ]; then
      let el=14
   fi
   if [ "$var_key" == f ]; then
      let el=15
   fi
   if [ "$var_key" == o ]; then
      let el=16
   fi

   # Execute Actions
   case "$el" in
   0)
      echo "all RCU's off"
      rspctl --rcuenable=0 2>&1 /dev/null
     ;;
   1)
      echo "RCU 0:7 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=0:7 2>&1 /dev/null
     ;;
   2)
      echo "RCU 8:15 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=8:15 2>&1 /dev/null
      ;;
   3)
      echo "RCU 16:23 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=16:23 2>&1 /dev/null
     ;;
   4)
      echo "RCU 24:31 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=24:31 2>&1 /dev/null
     ;;
   5)
      echo "RCU 32:39 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=32:39 2>&1 /dev/null
     ;;
   6)
      echo "RCU 40:47 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=40:47 2>&1 /dev/null
     ;;
   7)
      echo "RCU 48:55 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=48:55 2>&1 /dev/null
     ;;
   8)
      echo "RCU 56:63 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=56:63 2>&1 /dev/null
     ;;
   9)
      echo "RCU 64:71 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=64:71 2>&1 /dev/null
     ;;
   10)
      echo "RCU 72:79 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=72:79 2>&1 /dev/null
     ;;
   11)
      echo "RCU 80:87 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=80:87 2>&1 /dev/null
     ;;
   12)
      echo "RCU 88:95 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=88:95 2>&1 /dev/null
     ;;
   13)
      echo "RCU 0:31 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=0:31 2>&1 /dev/null
     ;;
   14)
      echo "RCU 32:63 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=32:63 2>&1 /dev/null
     ;;
   15)
      echo "RCU 64:95 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=64:95 2>&1 /dev/null
     ;;
   16)
      echo "RCU 0:95 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=0:95 2>&1 /dev/null
     ;;
   17)
      echo "RCU 96:103 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=96:104 2>&1 /dev/null
     ;;
   18)
      echo "RCU 104:111 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=104:111 2>&1 /dev/null
      ;;
   19)
      echo "RCU 112:119 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=112:119 2>&1 /dev/null
     ;;
   20)
      echo "RCU 120:127 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=120:127 2>&1 /dev/null
     ;;
   21)
      echo "RCU 128:135 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=128:135 2>&1 /dev/null
     ;;
   22)
      echo "RCU 136:143 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=136:143 2>&1 /dev/null
     ;;
   23)
      echo "RCU 144:151 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=144:151 2>&1 /dev/null
     ;;
   24)
      echo "RCU 152:159 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=152:159 2>&1 /dev/null
     ;;
   25)
      echo "RCU 160:167 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=160:167 2>&1 /dev/null
     ;;
   26)
      echo "RCU 168:175 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=168:175 2>&1 /dev/null
     ;;
   27)
      echo "RCU 176:183 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=176:183 2>&1 /dev/null
     ;;
   28)
      echo "RCU 184:191 on"
      rspctl --rcuenable=0 2>&1 /dev/nul
      sleep 1
      rspctl --rcuenable=1 --sel=184:191 2>&1 /dev/null
     ;;

   *)
     echo "Illegal..."
     ;;
   esac

done


