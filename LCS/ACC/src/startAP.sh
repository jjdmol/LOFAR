# startAP.sh procID executable paramfile
#
# start the given executable and creates a corresponding stop script
#
$2 $3 &
pid=`echo $!`
echo "pid=$pid"
echo "$pid" > $1.pid
