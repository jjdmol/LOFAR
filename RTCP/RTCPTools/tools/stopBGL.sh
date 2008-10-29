# stopAP.sh partition jobName
#
# partition BG/L partition the job is running on
# jobName   The name of the job
#

killall -9 mpirun

kill `ps -ef |grep '\-[w]dir' |grep -v 'sh \-c'|awk '{ print $2 }'`
