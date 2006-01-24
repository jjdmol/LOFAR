#!/bin/sh

# wrapper
# delay
# input
# bgl
# storage
# plotter
USERNAME=coolen
BASEGEOMETRY=60x20

if test "$1" = "wrapper"; then
  GEOMETRY=$BASEGEOMETRY+0+0
  NAME=SocketWrapper
  HOST=dop66.astron.nl
  REMOTECMD="echo 'wrapper started, sleeping 50';sleep 50"
  KILLCMD="killall TFC_SimpleRecorder"
fi
if test "$1" = "delay"; then
  GEOMETRY=$BASEGEOMETRY+400+0
  NAME=DelayCompensation
  HOST=dop66.astron.nl
  REMOTECMD="echo 'delay started, sleeping 50';sleep 50"
  KILLCMD="killall TFC_DelayCompensation"
fi
if test "$1" = "input"; then
  GEOMETRY=$BASEGEOMETRY+0+200
  NAME=InputSection
  HOST=dop66.astron.nl
  REMOTECMD="sleep45;echo 'input started, sleeping 50';sleep 50"
  KILLCMD="killall mpirun; killall TFC_InputSection"
fi
if test "$1" = "bgl"; then
  GEOMETRY=$BASEGEOMETRY+400+200
  NAME=PPF_And_Correlator
  HOST=dop66.astron.nl
  REMOTECMD="echo 'bgl started, sleeping 50';sleep 50"
  KILLCMD="sleep 1"
fi
if test "$1" = "storage"; then
  GEOMETRY=$BASEGEOMETRY+0+400
  USERNAME=coolen
  NAME=Storage
  HOST=dop66.astron.nl
  REMOTECMD="echo 'storage started, sleeping 50';sleep 50"
  KILLCMD="killall TFC_Storage"
fi
if test "$1" = "postproc"; then
  GEOMETRY=$BASEGEOMETRY+400+400
  NAME=PostProcessing
  HOST=dop66.astron.nl
  REMOTECMD="echo 'postproc started, sleeping 50';sleep 50"
  KILLCMD="killall ReadObservation.m; killall octave"
fi

CMD=$REMOTECMD
if test "$2" = "stop"; then
//  CMD=$KILLCMD
fi
SSHCMD="ssh ${USERNAME}@$HOST $CMD"
STARTCMD="xterm -geometry $GEOMETRY -e "$SSHCMD" "

rm -f $NAME.ready

echo Starting $NAME ...
echo Executing $STARTCMD
$STARTCMD

touch $NAME.ready
