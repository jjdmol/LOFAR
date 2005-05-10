# stopMPI.sh procID 
#
# procID			processname
#
# Stops the given process by killing the process whose pid is in the
# proces.pid file.
echo -n "Killing process "; cat $1.pid
kill -s 15 `cat $1.pid`
rm -f $1.pid $1*.ps
