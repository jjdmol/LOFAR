# stopAP.sh partition jobName
#
# partition BG/L partition the job is running on
# jobName   The name of the job
#

ssh $USER@bglsn /opt/lofar/bin/stopBGL.py --blockid=$1
