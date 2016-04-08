#!/bin/bash

PROD=false

# Define systems to use
if $PROD; then
  CCU=ccu001.control.lofar
  MCU=mcu001.control.lofar
  SCU=scu001.control.lofar
else
  CCU=ccu099.control.lofar
  MCU=mcu099.control.lofar
  SCU=scu099.control.lofar
fi

# -----------------------------------------
#   Queues for processing feedback
# -----------------------------------------

# cpuXX.cep4 -> headXX.cep4 -> ccu001
for tnode in head{01..02}.cep4.control.lofar
do
  for fnode in cpu{01..50}.cep4.control.lofar
  do
    addtoQPIDDB.py --broker $fnode --queue lofar.task.feedback.dataproducts --federation $tnode
    addtoQPIDDB.py --broker $fnode --queue lofar.task.feedback.processing --federation $tnode
    addtoQPIDDB.py --broker $fnode --queue lofar.task.feedback.state --federation $tnode
  done

  addtoQPIDDB.py --broker $tnode --queue lofar.task.feedback.dataproducts --federation $CCU
  addtoQPIDDB.py --broker $tnode --queue lofar.task.feedback.processing --federation $CCU
  addtoQPIDDB.py --broker $tnode --queue lofar.task.feedback.state --federation $CCU
done

# -----------------------------------------
#   Exchanges for ResourceAssignment
# -----------------------------------------
addtoQPIDDB.py --broker $SCU --exchange lofar.ra.command
addtoQPIDDB.py --broker $SCU --exchange lofar.ra.notification
addtoQPIDDB.py --broker $SCU --exchange lofar.otdb.command
addtoQPIDDB.py --broker $SCU --exchange lofar.otdb.notification
addtoQPIDDB.py --broker $SCU --exchange lofar.ssdb.command
addtoQPIDDB.py --broker $SCU --exchange lofar.ssdb.notification

# cpuXX.cep4 -> headXX.cep4 -> scu001
for tnode in head{01..02}.cep4.control.lofar
do
  for fnode in cpu{01..50}.cep4.control.lofar
  do
    addtoQPIDDB.py --broker $fnode --exchange lofar.otdb.command --federation $tnode
  done

  addtoQPIDDB.py --broker $tnode --exchange lofar.otdb.command --federation $SCU
done

