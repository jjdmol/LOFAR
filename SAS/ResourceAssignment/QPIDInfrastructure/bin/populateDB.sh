#!/bin/bash

# -----------------------------------------
#   Configuration
# -----------------------------------------

# Whether to modify production (true) or test (false)
if [ "$LOFARENV" == "prod" ]; then
  PROD=true
else
  PROD=false
fi


# Host names to use
if $PROD; then
  echo "----------------------------------------------"
  echo "Populating database for PRODUCTION environment"
  echo "----------------------------------------------"

  CCU=ccu001.control.lofar
  MCU=mcu001.control.lofar
  SCU=scu001.control.lofar
  SAS=sas001.control.lofar

  MOM_USER=lcs023.control.lofar
  MOM_INGEST=lcs029.control.lofar

  COBALT="`seq -f cbt%03.0f.control.lofar 1 8`"

  CEP2="`seq -f locus%03.0f.cep2.lofar 1 94`"
  CEP2HEAD=lhn001.cep2.lofar

  CEP4="`seq -f cpu%02.0f.cep4.control.lofar 1 50`"
  CEP4HEAD="head01.cep4.control.lofar head02.cep4.control.lofar"
else
  CCU=ccu099.control.lofar
  MCU=mcu099.control.lofar
  SCU=scu099.control.lofar
  SAS=sas099.control.lofar

  MOM_USER=lcs028.control.lofar
  MOM_INGEST=lcs028.control.lofar

  COBALT="cbt009.control.lofar"

  CEP2="locus098.cep2.lofar locus099.cep2.lofar"
  CEP2HEAD=locus102.cep2.lofar

  CEP4="`seq -f cpu%02.0f.cep4.control.lofar 1 50`"
  CEP4HEAD="head01.cep4.control.lofar head02.cep4.control.lofar"
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

addtoQPIDDB.py --broker $CCU --queue mom.task.feedback.dataproducts --federation $MOM_USER
addtoQPIDDB.py --broker $CCU --queue mom.task.feedback.processing --federation $MOM_USER
addtoQPIDDB.py --broker $CCU --queue mom.task.feedback.state --federation $MOM_USER

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
addtoQPIDDB.py --broker $CCU --queue mom.task.specification.system --federation $MOM_USER

# -----------------------------------------
#   MoM <-> MoM-OTDB-Adapter
# -----------------------------------------

addtoQPIDDB.py --broker $SAS --queue mom.command --federation $MOM_USER
addtoQPIDDB.py --broker $SAS --queue mom.importxml --federation $MOM_USER
addtoQPIDDB.py --broker $MOM_USER --queue mom-otdb-adapter.importxml --federation $SAS

# -----------------------------------------
#   MoM Services
# -----------------------------------------
addtoQPIDDB.py --broker $MOM_USER --exchange lofar.mom.bus
addtoQPIDDB.py --broker $MOM_INGEST --exchange lofar.mom.bus

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

