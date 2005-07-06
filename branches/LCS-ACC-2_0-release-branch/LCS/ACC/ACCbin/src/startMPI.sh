# startMPI.sh mpi_variant number_of_processes machinefile procID executable paramfile
#
# $1 mpi_variant         mpich or scampi
# $2 number_of_processes number
# $3 machinefile         procID.machinefile
# $4 procID              ???
# $5 executable          processname
# $6 parameterfile       procID.ps
#
# calls mpirun and remembers the pid
#

# start process
# TODO: in future something like: rsh $1 start_script $2 $3 $4
if [ "$1" = "scampi" ]; then
    echo "mpirun -np $2 -machinefile $3 $5 $6 " > startMPI.output
    ( ( mpirun -np $2 -machinefile $3 $5 $6 ) &> $4.output ) &
fi

if [ "$1" = "mpich" ]; then
  ( mpirun_mpich -np $2 -mf $3 $5 $6 ) &
fi

# get its pid
pid=`echo $!`

# construct pid file for stop shell
echo "$pid" > $4.pid
