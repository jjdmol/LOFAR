# startBGL.sh jobName partition executable workingDir paramfile noNodes
#
# jobName
# partition
# executable      executable file (should be in a place that is readable from BG/L)
# workingDir      directory for output files (should be readable by BG/L)
# observationID   observation number
# parameterfile   jobName.ps
# noNodes         number of nodes of the partition to use
#
# start the job and stores the jobID in jobName.jobID
#
# all ACC processes expect to be started with "ACC" as first parameter

. /opt/lofar/etc/BlueGeneControl.conf

# startBGL starts specific CEP run scripts directly
PARSET=/opt/lofar/share/CNProc.parset

sed -i 's/.*OLAP.CNProc.integrationSteps.*//' $PARSET
sed -i 's/.*OLAP.IONProc.integrationSteps.*//' $PARSET

$BINPATH/runParset.py -P $PARTITION parset=$PARSET >>/opt/lofar/log/run.runParset.py.log 2>&1 &
