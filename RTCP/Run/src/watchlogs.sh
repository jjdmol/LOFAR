#!/bin/bash

FLAGS="-n 10000"

# ----- find multitail
PATH=$PATH:/globalhome/broekema/bin

# ----- find the logging directory
#   .                        = current directory, to check first
#   $HOME/log/latest         = default logging directory for production
#   $HOME/projects/LOFAR/log = default logging directory for development

function set_logdir {
  LOGDIR=$1

  CNPROC=$LOGDIR/run.CNProc.log
  IONPROC=$LOGDIR/run.IONProc.log
}

for d in . "$HOME/production/lofar/bgfen/log/latest" "$HOME/log/latest" "$HOME/projects/LOFAR/log"
do
  set_logdir $d
  if [ -e $IONPROC ]
  then
    break
  fi
done

# ----- find the configuration for multitail
for c in `dirname $0` /opt/lofar/etc $HOME/projects/LOFAR/RTCP/Run/src
do
  CONFDIR=$c
  if [ -e $CONFDIR/multitail-olap.conf ]
  then
    break
  fi
done

echo Reading logs from $LOGDIR
echo Reading multitail configuration from $CONFDIR

ERRORLOGS=

if [ "$USER" == "lofarsys" ]
then
  for l in /opt/lofar/log/BlueGeneControl.log $HOME/log/run.runParset.py.log
  do
    echo Reading additional error log $l
    ERRORLOGS="$ERRORLOGS $FLAGS -cS olap -fr errors -I $l"
  done 
fi


multitail --no-mark-change --follow-all --retry-all -m 10240 --basename -F $CONFDIR/multitail-olap.conf \
  $FLAGS -t "-- FLAGS --"  -fr flags -ks flags -i $IONPROC \
  $FLAGS -t "-- ERRORS --" -fr errors          -i $IONPROC \
  $FLAGS                   -fr errors          -I $CNPROC \
  $ERRORLOGS \
  $FLAGS -t "IONProc/Storage"                  -i $IONPROC \
  $FLAGS -t "CNProc"       -wh 5               -i $CNPROC

