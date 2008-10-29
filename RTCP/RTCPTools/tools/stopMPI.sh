# stopMPI.sh execName 
#
#
# Stops the given process by killing the process whose pid is in the
# proces.pid file.

# TODO: for some mpi versions it is not enough to kill mpirun
#       we could "killall executable", but that would also kill
#       processes started by another ApplicationController

ssh listfen cexec :2-3 killall -9 Storage orted
killall -9 mpirun
rm -f $1*.ps

kill `ps -ef |grep '\-[w]dir' |grep -v 'sh \-c'|awk '{ print $2 }'`
