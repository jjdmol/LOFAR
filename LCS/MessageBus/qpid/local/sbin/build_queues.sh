#!/bin/bash
# 
# This script will set up the queues and routes needed for the LOFAR 
# Production system
# Run it as lofarsys on ccu001 or ccu099
# Dependning on the system, it will set up the queues for prod or test
#

# Run this on CCU001 or CCU099
host=`hostname -s`
if [[ "$host" != "CCU001" && "$host" != "CCU099" ]]; then 
  echo "Run $0 on CCU001 or CCU099!!!"
  exit
fi

if [ "$host" == "CCU001" ]; then
  # Host definitions: PRODUCTION
  ccu="CCU001.control.lofar"
  mcu="MCU001.control.lofar" 
  sas="SAS001.control.lofar"
  cobalt_root="cbm00" 
  cobalt_start=1
  cobalt_end=8
  mom="LCS023.control.lofar"
  head="lhn001.cep2.lofar"
  node_start=1
  node_end=94
else
  # Host definitions: TEST
  ccu="CCU099.control.lofar"
  mcu="mcu099" # see /etc/hosts
  sas="sas099" # see /etc/hosts
  cobalt_root="cbm00" 
  cobalt_start=9
  cobalt_end=9
  mom="LCS028.control.lofar"
  head="locus102.cep2.lofar"
  node_start=98
  node_end=99
fi

if [ "$1" == "-h" ] || [ "$1" == "-?" ] || [ "$1" == "--help" ]; then
   echo "usage: $0 [-h|-?|--help|--flush]"
   echo " -h , -? , --help produces this message"
   echo " --flush will flush the routing tables of the federated brokers in the TEST environment before setup."
   exit
else
   if [ "$1" == "--flush" ]; then
      # flush all routing
       for name in $sas $ccu $mcu $mom $head
       do
         echo flushing routing tables of broker at $name
         echo qpid-route route flush $name
         qpid-route route flush $name
      done
      for i in $(seq $cobalt_start $cobalt_end) 
      do 
         name=$cobalt_root$i".control.lofar"
         echo flushing routing tables of broker at $name
         echo qpid-route route flush $name
         qpid-route route flush $name
      done
      for name in $(seq -f "locus%03g.cep2.lofar" $node_start $node_end)
      do
         echo flushing routing tables of broker at $name
         echo qpid-route route flush $name
         qpid-route route flush $name
      done
      exit
   fi
fi

# Start setup
qpid-config -b $ccu add exchange topic mac.task.feedback.state

 
# forcefed will force-delete the queues, create them and setup the 
# federation
for i in $(seq $cobalt_start $cobalt_end)
do 
    name=$cobalt_root$i".control.lofar"
    fed lofar.task.feedback.processing    $name   $ccu
    fed lofar.task.feedback.dataproducts  $name   $ccu
    fed lofar.task.feedback.state         $name   $ccu 
done

fed lofar.task.feedback.processing    $head    $ccu
fed lofar.task.feedback.dataproducts  $head    $ccu
fed lofar.task.feedback.state         $head    $ccu

fed otdb.task.feedback.processing     $ccu     $mcu
fed otdb.task.feedback.dataproducts   $ccu     $mcu
fed lofar.task.specification.system   $mcu     $ccu
  
fed mom.task.feedback.processing      $ccu     $mom
fed mom.task.feedback.dataproducts    $ccu     $mom
fed mom.task.feedback.state           $ccu     $mom
fed mom.task.specification.system     $ccu     $mom
  
fed mom.command                       $sas     $mom
fed mom.importxml                     $sas     $mom
fed mom-otdb-adapter.importxml        $mom     $sas

for NODE in $(seq -f "locus%03g.cep2.lofar" $node_start $node_end)
do
    fed lofar.task.feedback.dataproducts  $NODE $head
    fed lofar.task.feedback.processing    $NODE $head
    fed lofar.task.feedback.state         $NODE $head
done

