# stopMPI.sh procID 
#
# procID			processname
#
# Stops the given process by killing the process whose pid is in the
# proces.pid file.

# TODO: for some mpi versions it is not enough to kill mpirun
#       we could "killall executable", but that would also kill
#       processes started by another ApplicationController
echo -n "Killing process "; cat $1.pid
kill -s 15 `cat $1.pid`
rm -f $1.pid $1*.ps
