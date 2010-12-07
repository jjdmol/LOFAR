# stopAP.sh partition jobName
#
# jobName       The name of the job
# partition     BG/L partition the job is running on
# observationID Observation number
#

#echo -n "Killing job " $2
#killjob $1 `cat $2.jobID`
#rm -f $2.pid $2.ps

JOB=$1
PARTITION=$2
OBSID=$3

# stopBGL.sh is used both to abort an observation and at the end of
# an observation. Sleep a bit so that OLAP can temrinate on its own
# in the latter case.
sleep 10

/opt/lofar/bin/commandOLAP.py -P $PARTITION cancel $OBSID
