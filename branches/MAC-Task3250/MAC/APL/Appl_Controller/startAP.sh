# startAP.sh nodename procID executable paramfile
#
# nodename		    hostname[.domain]
# procID			processname<nr>
# executable		processname
# parameterfile		procID.ps
#
# start the given executable and creates a corresponding pid file for stopping the process.
#

# DISABLED this script: startBGL.sh starts all CEP processes
exit

# now all ACC processes expect to be started with "ACC" as first parameter

# start process
echo $3 ACC $4 $2 >../log/$2.startup
$3 ACC $4 $2 & 

# get its pid
pid=`echo $!`

# construct pid file for stop shell
echo "$pid" > /opt/lofar/share/$2.pid
