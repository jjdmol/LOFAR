# startBGL.sh jobName partition executable workingDir paramfile noNodes
#
# jobName
# partition
# executable      executable file (should be in a place that is readable from BG/L)
# workingDir      directory for output files (should be readable by BG/L)
# parameterfile   jobName.ps
# noNodes         number of nodes of the partition to use
#
# start the job and stores the jobID in jobName.jobID
#
# all ACC processes expect to be started with "ACC" as first parameter

# start process

if [ -f ../share/Correlator.parset ]
then
    echo "../share/Correlator.parset file exist"
else
    echo "Sorry, ../share/Correlator.parset file does not exist"
fi

./prepare_$3.py

cd $4; mpirun -partition $2 -mode VN -label -cwd $4 $4/$3 $4/CS1_BGL_Processing.parset 5422 >> ../log/$3.log 2>&1 &
