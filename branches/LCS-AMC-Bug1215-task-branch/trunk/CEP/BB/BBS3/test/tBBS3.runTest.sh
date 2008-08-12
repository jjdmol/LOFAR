#!/bin/sh
#
# this script can be called with the testName and usernm
#   tBBS3.runTest.sh testDefault zwart
# it can also be called with only a username(which shouldn't start with test) or only a testname (which should start with test)
# if the username is not given, the environment variable $USER is used
# if the testname is not given, all test are run (all files matching tBBS3.test* are used)

# if a file run.$TESTNAME exists, that file is executed instead of the default script

# check if the first parameter starts with test...
TESTNAME=all
# if the first parameter is a testname do that test otherwise do all tests
if [ "${1:0:4}" == "test" ]; then
  TESTNAME=$1
  shift
fi
# the other argument is a username
USERNAME=$USER
if [ "$1" != "" ]; then
  USERNAME=$1
fi

echo Test = $TESTNAME, user = $USERNAME

if [ "$TESTNAME" == "all" ]; then
  for TEST in `ls tBBS3.test*`; do
    TESTNAME=${TEST##tBBS3.}
    ./tBBS3.runTest.sh $TESTNAME $USERNAME &> ${TESTNAME}.log
  done
  exit 0
fi

# check if there is a run.test.. file
if [ -e "run.$TESTNAME" ]; then
  ./run.$TESTNAME $USERNAME
  exit 0
fi

PSREADER=../../src/psReader

# Do ${TESTNAME}
echo ""
echo "Doing ${TESTNAME}"

PSET=tBBS3.${TESTNAME}
GETPARAM="$PSREADER $PSET"

#execute commands that are defined in the parameterFile
# with the prefix runscript.<name>....
doPSCommands () {
    `$GETPARAM runscript.$1`
}

doPSCommands initCommand

# read variables from ParameterSet
DBTYPE=`$GETPARAM CTRLparams.SC1params.MSDBparams.DBType`
MEQTN=`$GETPARAM CTRLparams.SC1params.MSDBparams.meqTableName`
GSMTN=`$GETPARAM CTRLparams.SC1params.MSDBparams.skyTableName`

echo "  Generating parm tables ..."
# use functions in initparms to initialize parmtables
. ./initparms

# the parameterSet defines which functions from initparms should be called
beginParmdbInputFile "dbtype='$DBTYPE', user='$USERNAME', tablename='$MEQTN'"
doPSCommands meqCommand
applyfile

beginParmdbInputFile "dbtype='$DBTYPE', user='$USERNAME', tablename='$GSMTN'"
doPSCommands gsmCommand
applyfile

echo "  Preparing BlackBoard ..."
time ./clearBB -user $USERNAME
echo "  Starting solve ..."
doPSCommands preCommand

NONODES=`$GETPARAM nrPrediffers`
NONODES=`expr $NONODES + 2`
RUNCOMMAND="time "
pwd | grep -e "_mpich/test" && RUNCOMMAND=" mpirun_mpich -np $NONODES -machinefile tBBS3_tmp.machinefile "
pwd | grep -e "_scampi/test" && RUNCOMMAND=" mpirun -np $NONODES -machinefile tBBS3_tmp.machinefile "
TOTRUNCMD="$RUNCOMMAND ./tBBS3 $PSET $USERNAME &> `pwd`/${TESTNAME}.out"
echo "The run command is: $TOTRUNCMD"
`$TOTRUNCMD`
doPSCommands postCommand
