# startAP.sh nodename procID executable paramfile
#
# nodename		    hostname[.domain]
# procID			processname<nr>
# executable		processname
# parameterfile		procID.ps
#
# start the given executable and creates a corresponding pid file for stopping the process.
#

# now all ACC processes expect to be started with "ACC" as first parameter

# start process
# TODO: in future something like: rsh $1 start_script $2 $3 $4
$3 ACC $4 & 

# get its pid
pid=`echo $!`

# construct pid file for stop shell
echo "$pid" > $2.pid
