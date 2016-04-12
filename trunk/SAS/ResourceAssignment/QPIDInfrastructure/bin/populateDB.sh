#!/bin/bash

# -----------------------------------------
#   Configuration
#
# TODO: Pull from locally deployed config file?
# -----------------------------------------

# Whether to modify production (true) or test (false)
PROD=false

# Host names to use
if $PROD; then
  CCU=ccu001
  MCU=mcu001
  SCU=scu001
  SAS=sas001

  LCS=lcs023

  COBALT="cbt001 cbt002 cbt003 cbt004 cbt005 cbt006 cbt007 cbt008"

  CEP2="`seq -f locus%03.0f 1 94`"
  CEP2HEAD=lhn001

  CEP4="`seq -f cpu%02.0f.cep4 1 50`"
  CEP4HEAD="head01.cep4 head02.cep4"
else
  CCU=ccu099
  MCU=mcu099
  SCU=scu099
  SAS=sas099

  LCS=lcs028

  COBALT="cbt009"

  CEP2="locus098 locus099"
  CEP2HEAD=locus102

  CEP4="`seq -f cpu%02.0f.cep4 1 50`"
  CEP4HEAD="head01.cep4 head02.cep4"
fi

# -----------------------------------------
#   Cobalt & Pipelines -> MessageRouter
# -----------------------------------------

for tnode in $CEP4HEAD
do
  for fnode in $CEP4
  do
    addtoQPIDDB.py --broker $fnode --queue lofar.task.feedback.dataproducts --federation $tnode
    addtoQPIDDB.py --broker $fnode --queue lofar.task.feedback.processing --federation $tnode
    addtoQPIDDB.py --broker $fnode --queue lofar.task.feedback.state --federation $tnode
  done

  addtoQPIDDB.py --broker $tnode --queue lofar.task.feedback.dataproducts --federation $CCU
  addtoQPIDDB.py --broker $tnode --queue lofar.task.feedback.processing --federation $CCU
  addtoQPIDDB.py --broker $tnode --queue lofar.task.feedback.state --federation $CCU
done

for tnode in $CEP2HEAD
do
  for fnode in $CEP2
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
#   MessageRouter -> MoM
# -----------------------------------------

addtoQPIDDB.py --broker $CCU --queue mom.task.feedback.dataproducts --federation $LCS
addtoQPIDDB.py --broker $CCU --queue mom.task.feedback.processing --federation $LCS
addtoQPIDDB.py --broker $CCU --queue mom.task.feedback.state --federation $LCS

# -----------------------------------------
#   MessageRouter -> OTDB
# -----------------------------------------

addtoQPIDDB.py --broker $CCU --queue otdb.task.feedback.dataproducts --federation $MCU
addtoQPIDDB.py --broker $CCU --queue otdb.task.feedback.processing --federation $MCU

# -----------------------------------------
#   MessageRouter -> MAC
# -----------------------------------------

addtoQPIDDB.py --broker $CCU --exchange mac.task.feedback.state

# -----------------------------------------
#   MACScheduler -> MessageRouter -> MoM
# -----------------------------------------

addtoQPIDDB.py --broker $MCU --queue lofar.task.specification.system --federation $CCU
addtoQPIDDB.py --broker $CCU --queue mom.task.specification.system --federation $LCS

# -----------------------------------------
#   MoM <-> MoM-OTDB-Adapter
# -----------------------------------------

addtoQPIDDB.py --broker $SAS --queue mom.command --federation $LCS
addtoQPIDDB.py --broker $SAS --queue mom.importxml --federation $LCS
addtoQPIDDB.py --broker $LCS --queue mom-otdb-adapter.importxml --federation $SAS

# -----------------------------------------
#   ResourceAssignment
# -----------------------------------------

addtoQPIDDB.py --broker $SCU --exchange lofar.ra.command
addtoQPIDDB.py --broker $SCU --exchange lofar.ra.notification
addtoQPIDDB.py --broker $SCU --exchange lofar.otdb.command
addtoQPIDDB.py --broker $SCU --exchange lofar.otdb.notification
addtoQPIDDB.py --broker $SCU --exchange lofar.ssdb.command
addtoQPIDDB.py --broker $SCU --exchange lofar.ssdb.notification

# TODO: messages will end up at $SCU twice?
for tnode in head{01..02}.cep4
do
  for fnode in cpu{01..50}.cep4
  do
    addtoQPIDDB.py --broker $fnode --exchange lofar.otdb.command --federation $tnode
  done

  addtoQPIDDB.py --broker $tnode --exchange lofar.otdb.command --federation $SCU
done

