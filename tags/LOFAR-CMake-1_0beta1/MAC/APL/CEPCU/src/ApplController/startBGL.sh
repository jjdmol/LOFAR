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

# TODO: /opt/lofar/share is hardcoded in ApplController/lofarDirs.h
Run/runOLAP.py parset=/opt/lofar/share/CNProc.parset >/globalhome/lofarsystem/log/run.runOLAP.py.log 2>&1 &

#echo "executing /usr/local/bin/submitjob $2 $3 $4 virtual_node_mode BGLMPI_SIZE=$6 ACC $5" > startBGL.output
#/usr/local/bin/submitjob $2 $3 $4 virtual_node_mode BGLMPI_SIZE=$6 ACC $5 2>&1 | awk '{split($1, a, "="); print a[2];}' > $1.jobID

