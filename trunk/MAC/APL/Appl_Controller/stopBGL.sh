# stopAP.sh partition jobName
#
# jobName       The name of the job
# observationID Observation number
#

#echo -n "Killing job " $2
#killjob $1 `cat $2.jobID`
#rm -f $2.pid $2.ps

. /opt/lofar/etc/BlueGeneControl.conf

JOB=$1
OBSID=$2

# stopBGL.sh is used both to abort an observation and at the end of
# an observation. Sleep a bit so that OLAP can temrinate on its own
# in the latter case.

# observations are cancelled through PLC, so there is no need to repeat that
#/opt/lofar/bin/commandOLAP.py -P $PARTITION cancel $OBSID
