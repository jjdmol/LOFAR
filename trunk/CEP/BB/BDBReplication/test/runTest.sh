#!/bin/sh

killall repQuote

NO_CLIENTS=2

rm dir0 -rf
mkdir dir0

echo Starting master ...
#xterm -e "src/repQuote -i 0 -p 100 -h dir0 -m localhost:8020 -o localhost:8020 > repQuote0.out" &
xterm -e "src/repQuote -i 0 -p 100 -h dir0 -m localhost:8020 -o localhost:8020" &

declare -i i
for i in `seq $NO_CLIENTS`
do
  rm dir$i -rf
  mkdir dir$i
#  sleep 1
  echo Starting client $i ...
  #xterm -e "src/repQuote -i $i -p 5$i -h dir$i -m localhost:802$i -o localhost:8020 > repQuote${i}.out" &
  xterm -e "src/repQuote -i $i -p 5$i -h dir$i -m localhost:802$i -o localhost:8020" &
done



# TH_Socket
# logging
# 
