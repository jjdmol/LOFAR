#!/bin/bash
# 
# This script will set up the queues and routes needed for the LOFAR 
# test system
# Run it as lofarsys on ccu099

# Host definitions
ccu="CCU099.control.lofar"
mcu="mcu099" # see /etc/hosts
sas="sas099" # see /etc/hosts
cobalt1="cbt009" # see /etc/hosts
mom="LCS028.control.lofar"
head="locus102.cep2.lofar"
node1="locus098.cep2.lofar"
node2="locus099.cep2.lofar"


# Run this on CCU099!!!!
if [ "`hostname -s`" != "CCU099" ]; then 
  echo "Run $0 on CCU099!!!"
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
      for name in $sas $ccu $mcu $mom $cobalt1 $head $node1 $node2
      do
         echo flushing routing tables of broker at $name
         qpid-route route flush $name
      done
   fi
   exit
fi

# Start setup
qpid-config -b $ccu add exchange topic mac.task.feedback.state
 
# forcefed will force-delete the queues, create them and setup the federation
fed lofar.task.feedback.processing    $cobalt1 $ccu
fed lofar.task.feedback.dataproducts  $cobalt1 $ccu
fed lofar.task.feedback.state         $cobalt1 $ccu

fed lofar.task.feedback.processing    $head    $ccu
fed lofar.task.feedback.dataproducts  $head    $ccu
fed lofar.task.feedback.state         $head    $ccu

fed lofar.task.feedback.processing    $node1   $ccu
fed lofar.task.feedback.dataproducts  $node1   $ccu
fed lofar.task.feedback.state         $node1   $ccu
 
fed lofar.task.feedback.processing    $node2   $ccu
fed lofar.task.feedback.dataproducts  $node2   $ccu
fed lofar.task.feedback.state         $node2   $ccu
 
fed otdb.task.feedback.processing     $ccu     $mcu
fed otdb.task.feedback.dataproducts   $ccu     $mcu
 
fed mom.task.feedback.processing      $ccu     $mom
fed mom.task.feedback.dataproducts    $ccu     $mom
fed mom.task.feedback.state           $ccu     $mom
 
fed lofar.task.specification.system   $mcu     $ccu
fed mom.task.specification.system     $ccu     $mom
 
fed mom.command                       $sas     $mom
fed mom.importxml                     $sas     $mom
fed mom-otdb-adapter.importxml        $mom     $sas

echo "Ready!"
