# startAP.sh nodename procID executable paramfile
#
# $1 jobName             identifier for this job
# $2 machinefile         procID.machinefile
# $3 executable          processname
# $4 parameterfile       procID.ps
# $5 numberOfNodes
#
# start the given executable and creates a corresponding pid file for stopping the process.
#

# now all ACC processes expect to be started with "ACC" as first parameter

./prepare_$3.py

# start process
# TODO: in future something like: rsh $1 start_script $2 $3 $4
./$3 ACC $4 $1>>/opt/lofar/log/$3.log 2>&1 & 

# get its pid
pid=`echo $!`

# construct pid file for stop shell
echo "$pid" > $2.pid
