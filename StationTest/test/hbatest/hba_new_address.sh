# !/bin/bash
# hba_new_address.sh, version 1.0, date 22-06-2009,  E. Kooistra

#
# Description: Use this script to change the address of a server in an HBA tile.
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

echo    ""
echo -n "Give the old server address (default 127): "
read oldAddr
if [ -z $oldAddr ]; then
  oldAddr=127
fi

echo ""
echo "Try to read the old HBA server address" $newAddr "to check that it is present in the HBA tile."
python $PYTHONPATH/../verify.py --brd rsp$rspNr --fp blp$blpNr -v $vb --te $PYTHONPATH/../tc/hba_server.py --server $oldAddr --server_access uc --server_function gb --server_reg address --data $oldAddr


echo    ""
echo -n "Give the new address for HBA server: "
read newAddr

echo ""
echo "Try to read the new HBA server address" $newAddr "to check that it is not already present in the HBA tile."
python $PYTHONPATH/../verify.py --brd rsp$rspNr --fp blp$blpNr -v $vb --te $PYTHONPATH/../tc/hba_server.py --server $newAddr --server_access uc --server_function gb --server_reg address --data $newAddr

echo    ""
echo -n "Press y if you are sure to set HBA server" $oldAddr "to the new server address" $newAddr", else the script will stop: "
read answer

if [ $answer == "y" ]; then
  python $PYTHONPATH/../verify.py --brd rsp$rspNr --fp blp$blpNr --rep 1 -v $vb --te $PYTHONPATH/../tc/hba_server.py --server $oldAddr --server_access uc --server_function sb --server_reg address --data $newAddr

  echo ""
  echo "The new HBA server address has been written."
  echo ""
  echo "Try to read access the new HBA server."
  python $PYTHONPATH/../verify.py --brd rsp$rspNr --fp blp$blpNr -v $vb --te $PYTHONPATH/../tc/hba_server.py --server $newAddr --server_access uc --server_function gb --server_reg address --data $newAddr
else
  echo ""
  echo "No HBA server address has been changed."
fi
