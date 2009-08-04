# startBGL.sh jobName partition executable workingDir paramfile noNodes
#
# jobName
# partition
# executable      executable file (should be in a place that is readable from BG/L)
# workingDir      directory for output files (should be readable by BG/L)
# parameterfile   jobName.ps
# noNodes         number of nodes of the partition to use
#
# start the job and stores the jobID in jobName.jobID
#
# all ACC processes expect to be started with "ACC" as first parameter


# startBGL starts specific CEP run scripts directly
partition=`Run/src/getPartition.py --parset=../share/CNProc.parset`
stationlist=`Run/src/getStations.py --parset=../share/CNProc.parset`
clock=`Run/src/getSampleClock.py --parset=../share/CNProc.parset`
integrationtime=`Run/src/getIntegrationtime.py --parset=../share/CNProc.parset`

Run/src/Run.py --parset=../share/CNProc.parset --partition=$partition --stationlist=$stationlist --integrationtime=$integrationtime --clock=$clock >/opt/lofar/log/run.Run.py.log 2>&1 &

# start process

#echo "executing /usr/local/bin/submitjob $2 $3 $4 virtual_node_mode BGLMPI_SIZE=$6 ACC $5" > startBGL.output
#/usr/local/bin/submitjob $2 $3 $4 virtual_node_mode BGLMPI_SIZE=$6 ACC $5 2>&1 | awk '{split($1, a, "="); print a[2];}' > $1.jobID

