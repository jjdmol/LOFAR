#!/bin/bash -l
HELP=$" This programs runs LOFAR regressions test in a standalone fashion. 
Usage: regression_test_runner.sh pipeline_type 

pipeline_types: 
  msss_imager_pipeline 
  msss_calibration_pipeline 
Default node worker directory: /data/scratch/$USER/regression_test_runner_<pipeline_type> 
                *** Warning: Directories will be cleared at start of the run (lce069-lce072)*** 
                    Should not be shared between worker nodes
[shared_dir]  : The root dir where source"

# Print usage
if [[ $# = 0 || $1 = "--help" ]]; then
  echo "$HELP"
  exit 1
fi

# test if we started in the correct directory
# we need to be able to grab and change installed files for full functionality
if [ -f lofarinit.sh ]
then
   :
else
    echo "*** Error ***"
    echo $"Could not find lofarinit at current working directory: $WORKSPACE"
    echo "script should be started from the base dir, eg: ~/build/gnu_debug "
    exit 1
fi

# *******************************************************
# 1) Check/set environment and set global variables

# Make sure aliases are expanded, which is not the default for non-interactive
# shells. Used for the use commands
shopt -s expand_aliases

# Some global named variables
PIPELINE=$1                                                             # First command line argument
WORKING_DIR=$"/data/scratch/$USER/regression_test_runner/$PIPELINE"     # Default working space
WORKSPACE=$PWD                                                          # Directory script is started from

# create the var run directory
mkdir -p $"$WORKSPACE/installed/var/run/pipeline"

# set up environment
use Lofar
use Pythonlibs
. $"$WORKSPACE/lofarinit.sh"

# *****************************************************
# 1) Clear old data:
# Clear var run directory: remove state, map and logfiles
rm $"$WORKSPACE/installed/var/run/pipeline/"* -rf

# Assure working directory exists
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
# 2) prepare the config file to run in a pipeline type depending but static location 
# copy config file to working dir:
cp $"$WORKSPACE/installed/share/pipeline/pipeline.cfg"  $"$WORKING_DIR/pipeline.cfg"

# insert the cluserdesc for test cluster (default install is for cep2)
sed -i 's/cep2.clusterdesc/cep1_test.clusterdesc/g' $"$WORKING_DIR/pipeline.cfg"
# specify the new working directory
sed -i $"s|working_directory = /data/scratch/$USER|working_directory = $WORKING_DIR|g" $"$WORKING_DIR/pipeline.cfg"


# copy parset to working dir (to allow output of meta information):
cp /data/lofar/testdata/regression_test_runner/msss_calibrator_pipeline/msss_calibrator_pipeline.parset $"$WORKING_DIR/msss_calibrator_pipeline.parset"

#python $"$WORKSPACE/installed/bin/msss_calibrator_pipeline.py" $"$WORKING_DIR/msss_calibrator_pipeline.parset" -c $"$WORKING_DIR/pipeline.cfg" -d




