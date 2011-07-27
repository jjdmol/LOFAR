#!/bin/bash

OBSID=$1

if [ "$OBSID" == "" ]
then
  echo Usage: [VERBOSE=1] `basename $0` obsid
  echo
  echo '
Prints:                VERBOSE=1:     VERBOSE=0 (default):

station flagging       all stations   stations with >1% flagged
file blocks dropped    all files      files with >10 blocks dropped
errors                 all errors     first 10 errors
'
  exit 1
fi

if [ "$VERBOSE" == "" ]
then
  VERBOSE=0
fi

LOGDIR="/globalhome/lofarsystem/log/L$OBSID"

if [ ! -d "$LOGDIR" ]
then
  echo Cannot find log directory $LOGDIR
  exit 1
fi

PARSET="$LOGDIR/L$OBSID.parset"
LOGFILES=`(ls -1 $LOGDIR/run.IONProc.log.* | sort;ls -1 $LOGDIR/run.IONProc.log) 2>/dev/null`

echo Observation $OBSID
echo Parset $PARSET

# flags
grep -h "obs $OBSID" $LOGFILES | perl -e '
  while(<>) {
    next unless /flags/;
    $station = $1 if /station ([A-Z0-9]+)/;

    while(/([0-9]+)%/g) {
      $s{$station} += $1;
      $n{$station}++;
    }
  }

  while(my ($station, $flags) = each(%s)) {
    if('$VERBOSE' or $flags/$n{$station} > 1) {
      printf "Station %s has %6.2f%% flagged.\n", $station, $flags/$n{$station};
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
      printf "File %s has %u dropped blocks.\n", $filename, $dropped{$streamid};
    }
  }
' | sort

# errors
grep -h "obs $OBSID" $LOGFILES | perl -e '
  while(<>) {
   if(/ERROR|FATAL/) {
     $n++;
     print "\n" if($n==1);

     if(not '$VERBOSE' and $n>10) {
       print "..and more!\n";
       exit;
     }
     print;
   }
  }
'
