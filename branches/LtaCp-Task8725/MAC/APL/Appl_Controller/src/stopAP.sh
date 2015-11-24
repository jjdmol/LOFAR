# stopAP.sh nodename procID 
#
# nodename		    hostname[.domain]
# procID			processname<nr>
#
# Stops the given process by killing the process whose pid is in the
# proces.pid file.

# DISABLED this script: startBGL.sh starts all CEP processes
exit

echo -n "Killing process "; cat $LOFARROOT/var/run/$2.pid
kill -9 `cat $LOFARROOT/var/run/$2.pid`
rm -f $LOFARROOT/var/run/$2.pid $LOFARROOT/var/run/$2.parset
