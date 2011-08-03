#!/bin/bash

OBSID=
VERBOSE=0

while getopts ":o:v" opt
do
  case $opt in
    v) VERBOSE=1
       ;;
    o) OBSID=$OPTARG
       ;;
    \?) echo "Invalid option: -$OPTARG"
       ;;
   esac
done

if [ "$OBSID" == "" ]
then
  echo Usage: `basename $0` "-o obsid [-v]"
  echo
  echo '  -o obsid     parse observation "obsid", written as either "Lxxxxx" or "xxxxx"'
  echo '  -v           verbose output'
  echo '
Prints:                verbose:       default:

station flagging       all stations   stations with >1% flagged
file blocks dropped    all files      files with >10 blocks dropped
errors                 all errors     first 10 errors
'
  exit 1
fi

# strip optional L (Lxxxxx -> xxxxx)
OBSID=`tr -d L <<<$OBSID`

LOGDIR="/globalhome/lofarsystem/log/L$OBSID"

if [ ! -d "$LOGDIR" ]
then
  echo Cannot find log directory $LOGDIR
  exit 1
fi

PARSET="$LOGDIR/L$OBSID.parset"
LOGFILES=`(ls -1 $LOGDIR/run.IONProc.log.* | sort;ls -1 $LOGDIR/run.IONProc.log) 2>/dev/null`

echo L$OBSID Parset $PARSET

# flags
grep -h "obs $OBSID" $LOGFILES | perl -e '
  while(<>) {
    next unless /flags/;
    $station = $1 if /station ([A-Z0-9]+)/;

    while(/([0-9.]+)%/g) {
      $x = $1;

      $sum{$station} += $x;
      $n{$station}++;

      $mean = $sum{$station}/$n{$station};
      $var{$station} += ($x - $old_mean{$station}) * ($x - $mean);
      $old_mean{$station} = $mean;
    }
  }

  while(my ($station, $flags) = each(%sum)) {
    $num      = $n{$station};
    $mean     = $flags/$num;
    $variance = $var{$station}/($num-1); # $num is always >1
    if('$VERBOSE' or $mean > 1) {
      printf "L'$OBSID' Station %s has %6.2f%% flagged (stddev %6.2f%%).\n", $station, $mean, sqrt($variance);
    }
  }
' | sort

# dropped blocks
grep -h "obs $OBSID" $LOGFILES | perl -e '
  while(<>) {
    $node = $1 if /Storage_main@([a-z0-9]+) /;

    if(/\[obs ([^]]+)\].*Writing to (.*)/) {
      $filenames{$1} = "$node:$2";
      $dropped{$1} = 0;
    }

    if(/\[obs ([^]]+)\]Dropped ([0-9]+) blocks/) {
      $dropped{$1} = $2;
    }
  }

  while(my ($streamid, $filename) = each(%filenames)) {
    if('$VERBOSE' or $dropped{$streamid}>10) {
      printf "L'$OBSID' File %s has %u dropped blocks.\n", $filename, $dropped{$streamid};
    }
  }
' | sort

# errors
grep -h "obs $OBSID" $LOGFILES | perl -e '
  while(<>) {
   if(/ERROR|FATAL/) {
     $n++;

     if(not '$VERBOSE' and $n>10) {
       print "L'$OBSID' Error ..and more!\n";
       exit;
     }
     print "L'$OBSID' Error $_";
   }
  }
'
