echo Killing existing rficonsoles...
rm out -rvf
mkdir -p out
# how many ms per node?
declare -i CPN=4
declare -i i=1
declare -i totalms=0
sets="`cat sets.txt`"
declare -i nodecount=`cat nodes.txt|wc -l`
echo Node count: ${nodecount}
declare -i thisnodei=0
for setname in ${sets} ; do
    thisnodei=${thisnodei}+1
    if (( ${thisnodei} > ${CPN} )) ; then
	i=$i+1
	thisnodei=1
	if (( $i > ${nodecount} )) ; then
	    echo Not enough nodes to process \(${CPN} sets per node requested\)
		exit;
	fi
    fi
    totalms=${totalms}+1
    A[${i}]="${A[${i}]} ${setname}"
done
echo Starting to process $totalms measurement sets...
nodes="`cat nodes.txt`"
i=0
rm -f /tmp/shuffle.txt /tmp/cmds.txt
for node in ${nodes} ; do
    i=$i+1
    echo ssh ${node} "-C" ~/distributed/single-eor.sh ${A[i]} >> /tmp/cmds.txt
done
i=0
cat /tmp/cmds.txt|sort >/tmp/shuffled.txt
exec</tmp/shuffled.txt
while read line
do
    i=$i+1
    ${line} &
    echo Started \#$i: ${line}
    sleep 60
done
