#!/bin/bash

function iperf_torture()
{
    for locus in `seq -w $1 $2`;  do 
	echo locus${locus}
	iperf -t 120 -y c -c locus${locus} >>  iperf-`hostname`.log &
	done
}

function remote_exec()
{
    remote_host=$1
    remote_command=$2
    ssh -AYCtt $remote_host "$remote_command"
}


case `hostname` in
    cbt001)
        iperf_torture 001 010 ;;

    cbt002)
        iperf_torture 011 020 ;;

    cbt003)
        iperf_torture 021 030 ;;

    cbt004)
        iperf_torture 031 040 ;;

    cbt005)
        iperf_torture 041 050 ;;

    cbt006)
        iperf_torture 051 060 ;;

    cbt007)
        iperf_torture 061 070 ;;

    cbt008)
        iperf_torture 071 080 ;;
esac
