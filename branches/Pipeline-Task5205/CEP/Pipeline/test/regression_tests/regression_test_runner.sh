#!/bin/bash -l
HELP=$" This programs runs LOFAR regressions test in a standalone fashion. 
Usage: regression_test_runner.sh pipeline_type [host1 host2]
Run the regressions test for pipeline_type. Perform the work on host1 and host2
Test should be started from the base installation directory eg: ~/build/gnu_debug

pipeline_types: 
  msss_calibratior_pipeline -or- 1
  msss_target_pipeline      -or- 2 
  msss_imager_pipeline      -or- 3
default hosts: 
  lce068 and lce069

All input, output and temporary files are stored in the following directory:
    /data/scratch/$USER/regression_test_runner/<pipeline_type> 
    
*** Warning: Directory will be cleared at start of the run (lce069-lce072)*** 
               Should not be shared between worker nodes"

# ******************************************************
# 1) validate input and print help if needed
# Print usage
echo "Validating environment and input"
if [[ $# < 3 || $1 = "--help" ]]; then
  echo "$HELP"
  exit 1
fi

# Some global named variables
PIPELINE=$1                                                             # First command line argument
WORKING_DIR=$"/data/scratch/$USER/regression_test_runner/$PIPELINE"     # Default working space
WORKSPACE=$PWD                                                          # Directory script is started from
HOST1=$2
HOST2=$3

# test if we started in the correct directory
# we need to be able to grab and change installed files for full functionality
if [ ! -f lofarinit.sh ]
then
    echo "*** Error ***"
    echo $"Could not find lofarinit at current working directory: $WORKSPACE"
    echo "script should be started from the base dir, eg: ~/build/gnu_debug "
    exit 1
fi

# Test if the selected pipeline is valid
if [ -f $"$WORKSPACE/installed/bin/$PIPELINE.py" ]
then
  # Test if the testdata dir is present (do not test the full tree just the parset)
  if [ -f /data/lofar/testdata/regression_test_runner/${PIPELINE}/${PIPELINE}.parset ]
  then
    :
  else
    echo "*** Error ***"
    echo $"The selected pipeline does not exist as entry in the test data dir:"
    echo $"/data/lofar/testdata/regression_test_runner/$PIPELINE"
    exit 1
  fi
else
    echo "*** Error ***"
    echo $"The selected pipeline does not exist as executable in:"
    echo $"$WORKSPACE/installed/bin/"
    exit 1
fi

# todo increase commentingmof this step
SECONDHOST=false
if [ -d $"/data/lofar/testdata/regression_test_runner/$PIPELINE/input_data/host2" ]; 
then
  echo "Validating performance of work performed on second node"
  SECONDHOST=true
fi

# *******************************************************
# 2) Set environment and set global variables
echo "Settings up environment"
# Make sure aliases are expanded, which is not the default for non-interactive
# shells. Used for the use commands
shopt -s expand_aliases

# create the var run directory
mkdir -p $"$WORKSPACE/installed/var/run/pipeline"

# set up environment
use Lofar
use Pythonlibs
. $"$WORKSPACE/lofarinit.sh"

# *****************************************************
# 3) Clear old data:
# Clear var run directory: remove state, map and logfiles
echo "Clearing working directories"
rm $"$WORKSPACE/installed/var/run/pipeline/"* -rf

# Assure working directory exists
#todo onlu clear target host
ssh lce068 $"mkdir $WORKING_DIR -p" 
ssh lce069 $"mkdir $WORKING_DIR -p" 
ssh lce070 $"mkdir $WORKING_DIR -p" 
ssh lce071 $"mkdir $WORKING_DIR -p" 
ssh lce072 $"mkdir $WORKING_DIR -p" 

# now remove all files in these dirs
ssh lce068 $"rm $WORKING_DIR/* -rf" 
ssh lce069 $"rm $WORKING_DIR/* -rf" 
ssh lce070 $"rm $WORKING_DIR/* -rf" 
ssh lce071 $"rm $WORKING_DIR/* -rf" 
ssh lce072 $"rm $WORKING_DIR/* -rf" 

# ******************************************************
# 4) prepare the config and parset to run in a pipeline type depending but static location 
# copy config file to working dir:
echo "copy input data to working directory at correct nodes"

# copy input data from data storage to the target host
# copy full input data batch to the target hosts
ssh $HOST1 $"mkdir $WORKING_DIR/input_data"
scp -r $"/data/lofar/testdata/regression_test_runner/$PIPELINE/input_data/host1/"* $HOST1:$"$WORKING_DIR/input_data"
if [ $SECONDHOST == true ]
then
  ssh $HOST2 $"mkdir $WORKING_DIR/input_data"
  scp -r $"/data/lofar/testdata/regression_test_runner/$PIPELINE/input_data/host2/"* $HOST2:$"$WORKING_DIR/input_data"
fi

echo "Updating configuration and parset file to reflect correct host and data locations"
cp $"$WORKSPACE/installed/share/pipeline/pipeline.cfg"  $"$WORKING_DIR/pipeline.cfg"

# insert the cluserdesc for test cluster (default install is for cep2)
sed -i 's/cep2.clusterdesc/cep1_test.clusterdesc/g' $"$WORKING_DIR/pipeline.cfg"
# specify the new working directory
sed -i $"s|working_directory = /data/scratch/$USER|working_directory = $WORKING_DIR|g" $"$WORKING_DIR/pipeline.cfg"


# copy parset to working dir (to allow output of meta information):
cp /data/lofar/testdata/regression_test_runner/$PIPELINE/$PIPELINE.parset $"$WORKING_DIR/$PIPELINE.parset"

# update the parset to reflect to correct input data set location
# Add the hosts to run the pipeline on
# The hosts ( will find both the input and the output hosts
sed -i  $"s|host1_placeholder|$HOST1|g" $"$WORKING_DIR/$PIPELINE.parset"
sed -i  $"s|host2_placeholder|$HOST2|g" $"$WORKING_DIR/$PIPELINE.parset"
# input data path
sed -i  $"s|input_path1_placeholder|$WORKING_DIR/input_data|g" $"$WORKING_DIR/$PIPELINE.parset"
sed -i  $"s|input_path2_placeholder|$WORKING_DIR/input_data|g" $"$WORKING_DIR/$PIPELINE.parset"
# output data paths will find all output paths
sed -i  $"s|output_path1_placeholder|$WORKING_DIR/output_data|g" $"$WORKING_DIR/$PIPELINE.parset"
sed -i  $"s|output_path2_placeholder|$WORKING_DIR/output_data|g" $"$WORKING_DIR/$PIPELINE.parset"

# *********************************************************************
# 5) Run the pipeline
python $"$WORKSPACE/installed/bin/$PIPELINE.py" $"$WORKING_DIR/$PIPELINE.parset" -c $"$WORKING_DIR/pipeline.cfg" -d

# ***********************************************************************
# 6) validate output
echo "validating output"

# copy the output data to the local node
mkdir $WORKING_DIR/output_data/host1 -p

scp -r $HOST1:$WORKING_DIR/output_data/* $WORKING_DIR/output_data/host1
if [ $SECONDHOST ]
then
  mkdir $WORKING_DIR/output_data/host2 -p
  scp -r $HOST2:$WORKING_DIR/output_data/* $WORKING_DIR/output_data/host2 
fi
# copy the target data to a writable location
mkdir $WORKING_DIR/target_data/host1 -p
cp -r /data/lofar/testdata/regression_test_runner/$PIPELINE/target_data/host1/*  $WORKING_DIR/target_data/host1
if [ $SECONDHOST == true ]
then
  mkdir $WORKING_DIR/target_data/host2 -p
  cp -r /data/lofar/testdata/regression_test_runner/$PIPELINE/target_data/host2/*  $WORKING_DIR/target_data/host2 || echo "*** Skipping comparison of node 2 ***"
fi
# get the source dir method allows for symlinks and other fancy methods of scripting
# http://stackoverflow.com/questions/59895/can-a-bash-script-tell-what-directory-its-stored-in
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
REGRESSION_TEST_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

# run the regression test for the pipeline: provide all the files in the directory
DELTA=0.0001
python $"$REGRESSION_TEST_DIR/$PIPELINE"_test.py $WORKING_DIR/output_data/host1/* $WORKING_DIR/target_data/host1/* $DELTA || { echo $"regressiontest failed on data in dir $WORKING_DIR/output_data/host1" ; exit 1; }
if [ $SECONDHOST == true ]
then
  python $"$REGRESSION_TEST_DIR/$PIPELINE"_test.py $WORKING_DIR/output_data/host2/* $WORKING_DIR/target_data/host2/* $DELTA || { echo $"regressiontest failed on data in dir $WORKING_DIR/output_data/host1" ; exit 1; }
fi


