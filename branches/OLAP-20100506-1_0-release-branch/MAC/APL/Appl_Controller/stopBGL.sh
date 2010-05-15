# stopAP.sh partition jobName
#
# partition BG/L partition the job is running on
# jobName   The name of the job
#

#echo -n "Killing job " $2
#killjob $1 `cat $2.jobID`
#rm -f $2.pid $2.ps
PARTITION=$1
JOB=$2
/opt/lofar/bin/commandOLAP.py -P $PARTITION cancel $JOB
