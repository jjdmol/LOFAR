#!/bin/bash

# -----------------------------------------
#   Configuration
# -----------------------------------------

# Whether to modify production (true) or test (false)
if [ "$LOFARENV" == "PRODUCTION" ]; then
  PROD=true
  PREFIX=
elif [ "$LOFARENV" == "TEST" ]; then
  PROD=false
  PREFIX="test."
else
  PROD=false
  PREFIX="devel."
fi


# Host names to use
if $PROD; then
  echo "----------------------------------------------"
  echo "Populating database for PRODUCTION environment"
  echo ""
  echo "Press ENTER to continue, or ^C to abort"
  echo "----------------------------------------------"
  read

  CCU=ccu001.control.lofar
  MCU=mcu001.control.lofar
  SCU=scu001.control.lofar
  SAS=sas001.control.lofar

  MOM_USER=lcs023.control.lofar
  MOM_INGEST=lcs029.control.lofar

  COBALT="`seq -f cbm%03.0f.control.lofar 1 8`"

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

  COBALT="cbm009.control.lofar"

  CEP2="locus098.cep2.lofar locus099.cep2.lofar"
  CEP2HEAD=locus102.cep2.lofar

  CEP4="`seq -f cpu%02.0f.cep4.control.lofar 1 50`"
  CEP4HEAD="head01.cep4.control.lofar head02.cep4.control.lofar"
fi

# -----------------------------------------
#   Cobalt GPUProc -> MessageRouter
# -----------------------------------------

for fnode in $COBALT
do
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.dataproducts --federation $CCU
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.processing --federation $CCU
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.state --federation $CCU
done


# -----------------------------------------
#   Cobalt OutputProc & Pipelines -> MessageRouter
# -----------------------------------------

for tnode in $CEP4HEAD
do
  # CEP4 -> CEP4HEAD
  for fnode in $CEP4
  do
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.dataproducts --federation $tnode
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.processing --federation $tnode
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.state --federation $tnode
  done

  # CEP4HEAD -> CCU
  addtoQPIDDB.py --broker $tnode --queue ${PREFIX}lofar.task.feedback.dataproducts --federation $CCU
  addtoQPIDDB.py --broker $tnode --queue ${PREFIX}lofar.task.feedback.processing --federation $CCU
  addtoQPIDDB.py --broker $tnode --queue ${PREFIX}lofar.task.feedback.state --federation $CCU
done

for tnode in $CEP2HEAD
do
  # CEP2 -> CEP2HEAD
  for fnode in $CEP2
  do
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.dataproducts --federation $tnode
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.processing --federation $tnode
    addtoQPIDDB.py --broker $fnode --queue ${PREFIX}lofar.task.feedback.state --federation $tnode
  done

  # CEP2HEAD -> CCU
  addtoQPIDDB.py --broker $tnode --queue ${PREFIX}lofar.task.feedback.dataproducts --federation $CCU
  addtoQPIDDB.py --broker $tnode --queue ${PREFIX}lofar.task.feedback.processing --federation $CCU
  addtoQPIDDB.py --broker $tnode --queue ${PREFIX}lofar.task.feedback.state --federation $CCU
done

# -----------------------------------------
#   MessageRouter -> MoM
# -----------------------------------------

addtoQPIDDB.py --broker $CCU --queue ${PREFIX}mom.task.feedback.dataproducts --federation $MOM_USER
addtoQPIDDB.py --broker $CCU --queue ${PREFIX}mom.task.feedback.processing --federation $MOM_USER
addtoQPIDDB.py --broker $CCU --queue ${PREFIX}mom.task.feedback.state --federation $MOM_USER

# -----------------------------------------
#   MessageRouter -> OTDB
# -----------------------------------------

addtoQPIDDB.py --broker $CCU --queue ${PREFIX}otdb.task.feedback.dataproducts --federation $MCU
addtoQPIDDB.py --broker $CCU --queue ${PREFIX}otdb.task.feedback.processing --federation $MCU

# -----------------------------------------
#   MessageRouter -> MAC
# -----------------------------------------

addtoQPIDDB.py --broker $CCU --exchange ${PREFIX}mac.task.feedback.state

# -----------------------------------------
#   MACScheduler -> MessageRouter -> MoM
# -----------------------------------------

addtoQPIDDB.py --broker $MCU --queue ${PREFIX}lofar.task.specification.system --federation $CCU
addtoQPIDDB.py --broker $CCU --queue ${PREFIX}mom.task.specification.system --federation $MOM_USER

# -----------------------------------------
#   MoM <-> MoM-OTDB-Adapter
# -----------------------------------------

addtoQPIDDB.py --broker $SAS --queue mom.command --federation $MOM_USER
addtoQPIDDB.py --broker $SAS --queue mom.importxml --federation $MOM_USER
addtoQPIDDB.py --broker $MOM_USER --queue mom-otdb-adapter.importxml --federation $SAS

# -----------------------------------------
#   MoM Services
# -----------------------------------------
addtoQPIDDB.py --broker $MOM_USER --exchange ${PREFIX}lofar.mom.bus
addtoQPIDDB.py --broker $MOM_INGEST --exchange ${PREFIX}lofar.mom.bus
addtoQPIDDB.py --broker $MOM_USER --exchange ${PREFIX}lofar.mom.command
addtoQPIDDB.py --broker $MOM_USER --exchange ${PREFIX}lofar.mom.notification

# -----------------------------------------
#   MoM Services <-> ResourceAssignment
# -----------------------------------------

addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.mom.bus --federation $MOM_USER
addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.mom.command --federation $MOM_USER
addtoQPIDDB.py --broker $MOM_USER --exchange ${PREFIX}lofar.mom.notification --federation $SCU

# -----------------------------------------
#   ResourceAssignment
# -----------------------------------------

addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.ra.command
addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.ra.notification
addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.otdb.command
addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.otdb.notification
addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.ssdb.command
addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.ssdb.notification

# TODO: messages will be duplicated?
for head in head{01..02}.cep4
do
  for cpu in cpu{01..50}.cep4
  do
    addtoQPIDDB.py --broker $cpu --exchange ${PREFIX}lofar.otdb.command --federation $head
    addtoQPIDDB.py --broker $head --exchange ${PREFIX}lofar.otdb.notification --federation $cpu
  done

  addtoQPIDDB.py --broker $head --exchange ${PREFIX}lofar.otdb.command --federation $SCU
  addtoQPIDDB.py --broker $SCU --exchange ${PREFIX}lofar.otdb.notification --federation $head
done

