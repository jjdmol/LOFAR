#!/bin/bash
# 
# This script will set up the queues and routes needed for the LOFAR 
# Production system
# Run it as lofarsys on ccu001

# Host definitions
ccu="CCU001.control.lofar"
mcu="MCU001.control.lofar" 
sas="SAS001.contorl.lofar"
cobalt_root="cbt00" 
mom="LCS023.control.lofar"
head="lhn001.cep2.lofar"

# Run this on CCU001 or LHN001
host=`hostname -s`
if [ "$host" != "CCU001" && "$host" != "lhn001"]; then 
  echo "Run $0 on CCU001 or lhn001!!!"
  exit
fi

if [ "$1" == "-h" ] || [ "$1" == "-?" ] || [ "$1" == "--help" ]
then
   echo "usage: $0 [-h|-?|--help|--flush]"
   echo " -h , -? , --help produces this message"
   echo " --flush will flush the routing tables of the federated brokers in the TEST environment before setup."
   exit
else
   if [ "$1" == "--flush" ]
   then
      # flush all routing
      if [ "$host" == "CCU001" ]; then
        for name in $sas $ccu $mcu $mom $head
         do
           echo flushing routing tables of broker at $name
           qpid-route route flush $name
        done
        for i in 1 2 3 4 5 6 7 8 
        do 
           name=$cobalt_root$i".control.lofar"
           echo flushing routing tables of broker at $name
           qpid-route route flush $name
        done
      else # so on lhn001!
        for name in $(seq -f "locus%03g" 1 94)
        do
           echo flushing routing tables of broker at $name
           qpid-route route flush $name
        done
      fi
   fi
   exit
fi

# Start setup
if [ "$host" == "CCU001" ]; then 
  qpid-config -b $ccu add exchange topic mac.task.feedback.state
 
  # forcefed will force-delete the queues, create them and setup the 
  federation
  for i in 1 2 3 4 5 6 7 8
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

  echo "Ready! Now run on lhn001 as well!"
else
  for NODE in $(seq -f "locus%03g" 1 94)
  do
    fed lofar.task.feedback.dataproducts  $NODE lhn001.cep2.lofar
    fed lofar.task.feedback.processing    $NODE lhn001.cep2.lofar
    fed lofar.task.feedback.state         $NODE lhn001.cep2.lofar
  done
  echo "Ready! Now run on CCU001 as well!"
fi

