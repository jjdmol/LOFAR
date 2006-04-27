# startMPI.sh jobName machinefile executable paramfile noNodes
#
# $1 jobName             identifier for this job
# $2 machinefile         procID.machinefile
# $3 executable          processname
# $4 parameterfile       procID.ps
# $5 numberOfNodes
#
# calls mpirun and remembers the pid
#

# now all ACC processes expect to be started with ACC as first parameter

# start process
# TODO: on some hosts, mpirun has a different name (or a specific path)
#       on some hosts, we should use -hostfile instead of -machinefile
echo "mpirun -np $5 -machinefile $2 $3 ACC $4 " > startMPI.output
( ( mpirun -np $5 -machinefile $2 $3 ACC $4 ) &> $1.output ) &
fi

# get its pid
pid=`echo $!`

# construct pid file for stop shell
echo "$pid" > $4.pid
