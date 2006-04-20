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
/usr/local/bin/submitjob $1 $3 $WRKDIR virtual_node_mode BGLMPI_SIZE=$5 $4 2>&1 | awk '{print $2}' > $2.jobID
