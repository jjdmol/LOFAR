#!/bin/sh

if [ $EUID -ne 0 ]; then
  echo ERROR: `basename $0`: This test must be run as 'root'
  exit 77
fi

# start the BeamServer
../src/BeamServer -sfp abstest > avtstub.1.log 2>&1 &
beamserver_pid=$!

# start the AVTStub test
./AVTStub -sfp abstest > avtstub.2.log
retcode=$?

# cleanup
trap '(kill $beamserver_pid > /dev/null 2>&1) && wait' EXIT

exit $retcode
