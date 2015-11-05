#!/bin/bash

case `hostname` in
    kis001)
        lcurun -s $1 -c "ppstune.sh $2 $3 $4 $5 $6 $7 $8 $9" 2>&1 | grep -v 'TRACE  TRC'

        lcurun -s $1 -c 'grep -e "\(ERR\|WARN\)" /localhome/ppstune/log/pps-tuning-*.log' |grep `date +'%Y-%m-%d'`|sort -k 1,4 ;;

    *) 
       cd /opt/stationtest
       ppstune.py $1 $2 $3 $4 $5 $6 $7 $8 $9 ;;
esac

