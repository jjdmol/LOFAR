#!/bin/bash


QUEUES=`echo "select hostname,queuename from persistentqueues INNER join hosts on (hid=hostid) INNER JOIN queues on (qid=queueid);" | psql qpidinfra`
EXCHANGES=`echo "select hostname,exchangename from persistentexchanges INNER join hosts on (hid=hostid) INNER JOIN exchanges on (eid=exchangeid);" | psql qpidinfra`
QUEUEANS="       hostname       |    queuename     
----------------------+------------------
 scu001.control.lofar | TreeStatus
 scu001.control.lofar | TaskSpecified
 scu001.control.lofar | ResourceAssigner
(3 rows)"

EXCHANS="       hostname       |      exchangename       
----------------------+-------------------------
 scu001.control.lofar | lofar.ra.command
 scu001.control.lofar | lofar.ra.notification
 scu001.control.lofar | lofar.otdb.command
 scu001.control.lofar | lofar.otdb.notification
 scu001.control.lofar | lofar.sm.command
 scu001.control.lofar | lofar.sm.notification
 scu001.control.lofar | lofar.mom.command
 scu001.control.lofar | lofar.mom.notification
(8 rows)"


ret=0
if [ "$QUEUES" != "$QUEUEANS" ]
then
    echo "Queues failed test"
    ret=1
else
    echo "Queues test passed"
fi

if [ "$EXCHANGES" != "$EXCHANS" ]
then
    echo "Exchanges failed test"
    ret=1
else
    echo "Exchanges test passed"
fi

exit $ret




