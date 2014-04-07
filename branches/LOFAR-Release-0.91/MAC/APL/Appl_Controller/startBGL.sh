# startBGL.sh jobName partition executable workingDir paramfile noNodes
#
# jobName
# executable      executable file (should be in a place that is readable from BG/L)
# workingDir      directory for output files (should be readable by BG/L)
# parameterfile   jobName.ps
# observationID   observation number
# noNodes         number of nodes of the partition to use
#
# start the job and stores the jobID in jobName.jobID
#
# all ACC processes expect to be started with "ACC" as first parameter

JOBNAME=$1
PARSET=$4
OBSID=$5

. /opt/lofar/etc/BlueGeneControl.conf

# Remove values which runParset should derive
sed -i 's/.*OLAP.CNProc.integrationSteps.*//' $PARSET
sed -i 's/.*OLAP.IONProc.integrationSteps.*//' $PARSET

(
echo "OLAP.IONProc.PLC_controlled = T"
) >> $PARSET

# Inject the parset into the correlator
/opt/lofar/bin/runParset.py -P $PARTITION parset=$PARSET >>/opt/lofar/log/run.runParset.py.log 2>&1 &
