# stopMPI.sh execName 
#
#
# Stops the given process by killing the process whose pid is in the
# proces.pid file.

# TODO: for some mpi versions it is not enough to kill mpirun
#       we could "killall executable", but that would also kill
#       processes started by another ApplicationController

cexec killall -9 $1

rm -f $1*.ps
