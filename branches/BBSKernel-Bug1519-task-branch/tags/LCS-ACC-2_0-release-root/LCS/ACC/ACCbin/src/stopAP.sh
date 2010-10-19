# stopAP.sh nodename procID 
#
# nodename		    hostname[.domain]
# procID			processname<nr>
#
# Stops the given process by killing the process whose pid is in the
# proces.pid file.
echo -n "Killing process "; cat $2.pid
kill -9 `cat $2.pid`
rm -f $2.pid $2.ps
