# stopAP.sh partition jobName
#
# partition BG/L partition the job is running on
# jobName   The name of the job
#

for i in 5 6; do ssh 10.170.0.$i killall -9 IONProc; done; 
