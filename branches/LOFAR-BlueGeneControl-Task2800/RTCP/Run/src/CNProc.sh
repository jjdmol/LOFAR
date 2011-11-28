#!/bin/bash
PARTITION="R01-M0-N04-64"


function start() {
  source locations.sh

  mpirun -mode VN -partition $PARTITION -env DCMF_COLLECTIVES=0 -env BG_MAPPING=XYZT -env LD_LIBRARY_PATH=/bgsys/drivers/ppcfloor/comm/lib:/bgsys/drivers/ppcfloor/runtime/SPI:/globalhome/romein/lib.bgp -cwd $LOGDIR -exe $CNPROC >& $LOGSYMLINK/CNProc.log &
}

function stop() {
  # mpikill only works when mpirun has started running the application
  mpikill "$PID" || kill -9 "$PID"
}

. IdentityControl.sh
