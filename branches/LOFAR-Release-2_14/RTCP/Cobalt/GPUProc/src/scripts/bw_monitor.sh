#!/bin/bash
#
# Report the in/output bandwidth of cobalt nodes 1-8.
#

function transfer_balance {
  # fields:
  #
  # cobaltnr ethnr timestamp recv send
  CBMNR="$1"
  ssh cbm00"$CBMNR" ifconfig 2>/dev/null | perl -ne '
    # monitor eth2..5
    $eth = $1 if /^eth([2345])/;
    next if not $eth;

    # section ends on empty line
    $eth = -1 if /^\s+$/;
    next if $eth == -1;

    # process RX/TX bytes
    if (/RX bytes:([0-9]+).*TX bytes:([0-9]+)/) {
      $rx = $1;
      $tx = $2;
      print "'$CBMNR' $eth '`date +%s.%N`' $rx $tx\n";
    }'
}

function full_balance {
  # The Cobalt nodes to monitor
  CBMNRS="1 2 3 4 5 6 7 8"

  echo "# ----------------------------------------------------------------"
  echo "# cobalt-nr eth-nr timestamp rx-bytes tx-bytes"
  echo "# 0 0 = subsums for total transfer rates"

  for CBMNR in $CBMNRS; do
    BALANCE=`transfer_balance "$CBMNR"`
    echo "$BALANCE"

    # create a line with totals
    echo "$BALANCE" | awk '
    {
      ts = $3;
      totalrx += $4;
      totaltx += $5;
    }

    END {
      print "0 0", ts, totalrx, totaltx;
    }'
  done
}

function balance_diff {
  BEFORE="$1"
  AFTER="$2"

  # compare before and after, and print difference in totals
  # in Gbit/s.
  paste <(echo "$BEFORE") <(echo "$AFTER") | awk '
  /^0 0/ {
    rx_delta += $9 - $4;
    tx_delta += $10 - $5;
    ts_delta = $8 - $3;
    ts = ($8+$3)/2;
  }

  END {
    gbps = 1024*1024*1024/8;
    printf "# ";
    system(sprintf("date +\"%%F %%T\" -d @%.0f",ts));
    printf "# total in: %.2f Gbps out: %.2f Gbps\n",(rx_delta/ts_delta/gbps),(tx_delta/ts_delta/gbps);
  }'
}

PREV_BALANCE=`full_balance`
while :; do
  sleep 2
  CURR_BALANCE=`full_balance`

  echo "$CURR_BALANCE"
  balance_diff "$PREV_BALANCE" "$CURR_BALANCE"
  PREV_BALANCE="$CURR_BALANCE"
done

