# startBGL.sh partitionname procID executable paramfile noNodes
#
# partitionname   hostname[.domain]
# procID          processname<nr>
# executable      executable file
# parameterfile   procID.ps
# noNodes         number of nodes of the partition to use
#
# start the given executable and creates a corresponding pid file for stopping the process.
#

# now all ACC processes expect to be started with "ACC" as first parameter

# start process
# TODO: in future something like: rsh $1 start_script $2 $3 $4

WRKDIR=`dirname $3`
echo "executing /usr/local/bin/submitjob $1 $3 $WRKDIR virtual_node_mode BGLMPI_SIZE=$5 ACC $4" > startBGL.output
/usr/local/bin/submitjob $1 $3 $WRKDIR virtual_node_mode BGLMPI_SIZE=$5 ACC $4 2>&1 | awk '{split($1, a, "="); print a[2];}' > $2.jobID

