# !/bin/bash
# hba_new_address.sh, version 1.0, date 22-06-2009,  E. Kooistra

#
# Description: Use this script to verify that all 16 servers are present in an HBA tile.
#

vb=11       # verbosity level

echo    ""
echo -n "Give number of the RCU that controls the HBA tile: "
read rcuNr

c_nof_rcu_per_rsp=8
c_nof_rcu_per_blp=2

rspNr=$(eval "echo \"scale=0;  $rcuNr / $c_nof_rcu_per_rsp\" | bc -l")
blpNr=$(eval "echo \"scale=0; ($rcuNr % $c_nof_rcu_per_rsp) / $c_nof_rcu_per_blp\" | bc -l")

echo ""
echo "The HBA tile is controlled by RCU-"$rcuNr "via RSP-"$rspNr "and BLP-"$blpNr"."

echo ""
echo "Try to read access HBA server 1,2,...,16 and 127 to see which HBA servers are present in the HBA tile."

c_nof_server_per_hba=16
for ((si=1; si <= $c_nof_server_per_hba; si++)) do
  python $PYTHONPATH/../verify.py --brd rsp$rspNr --fp blp$blpNr -v $vb --te $PYTHONPATH/../tc/hba_server.py --server $si --server_access uc --server_function gb --server_reg address --data $si
done
python $PYTHONPATH/../verify.py --brd rsp$rspNr --fp blp$blpNr -v $vb --te $PYTHONPATH/..tc/hba_server.py --server 127 --server_access uc --server_function gb --server_reg address --data 127
