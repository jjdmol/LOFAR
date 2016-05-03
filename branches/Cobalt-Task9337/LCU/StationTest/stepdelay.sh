#!/bin/bash
#
# Step through delay setting HBA
#
# Version 1.0   13 nov 2012   H.J. Meulman

declare el=0
echo "[ -------- Sends hbadelay string to HBA's -------- ]"
echo "[ u for up ]    [ n for down ]    [ CTRL+C to stop ]"
echo "[ 1,2,3,4,5,6,7,8,9,a,b,c,d,e,f,g   for element on ]"
echo "[ 0 for all elements off ] [ o for all elements on ]"

echo "all elements off"
rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null

for (( ; ; ))
do
   read -n 1 -s var_key
   if [ "$var_key" == u ]; then
      let el+=1
      if [ $el -eq 18 ]; then
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
   if [ "$var_key" == g ]; then
      let el=16
   fi
   if [ "$var_key" == o ]; then
      let el=17
   fi
   case "$el" in
   0)
      echo "all elements off"
      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   1)
      echo "element 1 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=253,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   2)
      echo "element 2 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,253,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      ;;
   3)
      echo "element 3 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,253,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   4)
      echo "element 4 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,253,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   5)
      echo "element 5 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,253,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   6)
      echo "element 6 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,253,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   7)
      echo "element 7 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,253,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   8)
      echo "element 8 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,253,2,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   9)
      echo "element 9 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,2,253,2,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   10)
      echo "element 10 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,253,2,2,2,2,2,2 2>&1 /dev/null
     ;;
   11)
      echo "element 11 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,253,2,2,2,2,2 2>&1 /dev/null
     ;;
   12)
      echo "element 12 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,253,2,2,2,2 2>&1 /dev/null
     ;;
   13)
      echo "element 13 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,253,2,2,2 2>&1 /dev/null
     ;;
   14)
      echo "element 14 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,253,2,2 2>&1 /dev/null
     ;;
   15)
      echo "element 15 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,253,2 2>&1 /dev/null
     ;;
   16)
      echo "element 16 on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,253 2>&1 /dev/null
     ;;
   17)
      echo "all elements on"
#      rspctl --hbadelays=2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 2>&1 /dev/null
      rspctl --hbadelays=253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253 2>&1 /dev/null
     ;;
   *)
     echo "Illegal..."
     ;;
   esac

done


