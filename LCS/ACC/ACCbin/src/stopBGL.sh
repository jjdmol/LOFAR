# stopAP.sh partition procID 
#
# partition BG/L partition the job is running on
# procID    The name of the job
#

echo -n "Killing job " procID
killjob $1 `cat $2.jobID`
rm -f $2.pid $2.ps
