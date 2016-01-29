#!/bin/bash
# This script parses all the pipeline logfiles
# of pipelines exit with an error an attempt is made to find the cause
# of the error.

# parse command line parameter:
commandLineArgument=$1

# Try parsing the command line parameters, If a specific range or id is provided
# only analyse matching logfile
if [ $# -gt 0 ]  # if there are arguments
then
  if [ "$1" = "--help" ]  # if help is called
  then
    echo $1
    echo "Display an analysis of encountered errors in pipeline logs"
    echo Usage: $0 "[Obsid range] [--display] [--gz]"
	echo
	echo example: $0 '78*' 
	echo "will result in a detailed analysis of all logfiles for Observation78*"
	echo "--display forces the displaying of the error for each analyzed logfile"
    echo "--gz will also analyse gzipped files (default for old logfiles) Warning: SLOW !!"
    echo "     For this to work the script MUST be started in a directory with write rights."	
	echo "Providing no argument will result in a general analysis without detailed error information"
	exit
  fi
  
  obsIDExpression=$1
  
  if [ "$2" = "--display" ]
  then  
	
	displayProblem=true
  fi
  
  
else
  obsIDExpression="*"
  displayProblem=false
fi

# Running count of number of logfiles == number of pipeline runs, a
nPipelineByType[0]=0  # unknown_type
nPipelineByType[1]=0  # msss_imager_pipeline
nPipelineByType[2]=0  # msss_calibrator_pipeline
nPipelineByType[3]=0  # msss_target_pipeline
nPipelineByType[4]=0  # calibration_pipeline
nPipelineByType[5]=0  # preprocessing_pipeline
nPipelineByType[6]=0  # long_baseline_pipeline
nPipelineByType[7]=0  # pulsar_pipeline
nPipelineByType[8]=0  # imaging_pipeline

nPipelineByTypeFail=(0 0 0 0 0 0 0 0 0)         # count the number of failed pipeline runs
nPipelineByTypeFailExplained=(0 0 0 0 0 0 0 0 0) # When pipeline failed, the actual error can be pinpointed, explained

# Create the array containing the grep string and the message to display
problemGrep[0]='Demix averaging .* must be multiple of output averaging'
problemMSG[0]='NPPP parset: incorrect values in the parset: Demix average'

problemGrep[1]='time window should fit final averaging integrally'
problemMSG[1]='NPPP parset: time window incorrect (itsNTimeChunk \* itsNTimeAvg) % itsNTimeAvgSubtr != 0'

problemGrep[2]='expected_fluxes_in_fov raise exception'
problemMSG[2]='GSM:An error occured in a call to the Global Sky model'

problemGrep[3]="'tuple' object has no attribute 'host'"
problemMSG[3]='FIXED: Incorroct parset or mapfile implementation: tuple object has no attribute '

problemGrep[4]='0 Output_SkyImage data products specified'
problemMSG[4]='PARSET: 0 Output_SkyImage data products specified'

problemGrep[5]='msss_target_pipeline.bbs_reducer: Validation of input data mapfiles failed'
problemMSG[5]='FIXED: Incorrect number of input data products'

problemGrep[6]='is an invalid value for FileField skymodel'
problemMSG[6]='BBS PARSET: <x>.skymodel is an invalid value for FileField skymodel'

problemGrep[7]='contains no solvable parameters for step'
problemMSG[7]='BBS DATA: Key xx contains no solvable parameters for step xx: bbs aborted on a step '

problemGrep[8]='Patch name .* multiply defined in '
problemMSG[8]='GSM/SOURCEDB: Patch name x multiply defined. Incorrect local sky model from GSM'

problemGrep[9]='vdsmaker: Exception caught: .* No such file or directory'
problemMSG[9]='VDSMAKER MISSING INPUTS: Exception caught: No such file or directory'

problemGrep[10]="KeyError: 'returncode'"
problemMSG[10]='FIXED: Race issue between thread and socked connection: solved'

problemGrep[11]="vdsmaker: Exception caught: list assignment index out of range"
problemMSG[11]='FIXED: incorrect deletion of vds files, possible duplicate input dataproducts'

problemGrep[12]='WARNING .*: Processing interrupted: shutting down'
problemMSG[12]='MANUALLY ABORTED: ctrl-c (-or- abort from mac )  '

problemGrep[13]='DEBUG - Assertion: itsAnt1.*itsAntMap.size.*'
problemMSG[13]='Nr. antennas in data is different to meta data bug #8157'

problemGrep[14]='Validation of input.*output data product specification failed'
problemMSG[14]='The number of input and output dataproducts is incorrect'

problemGrep[15]='pulp has crashed' 
problemMSG[15]='An error occured in a pulsar pipeline, inform Vlad Condratu'

problemGrep[16]='ssh: connect to host locus'
problemMSG[16]='Suspected connectivity problems on locus nodes, test ssh connection to affected nodes'

problemGrep[17]='The system is going down for reboot in'
problemMSG[17]='The pipeline was stopped due to reboot of system, (stopday?)'

problemGrep[18]='is an invalid value for ExecField'
problemMSG[18]='Incorrect executable configuration, warn pipeline developer'

problemGrep[19]='ImportError.*libcasa.*undefined symbol'
problemMSG[19]='Casacore problem, warn pipeline developer or Ger van Diepen'

problemGrep[20]='imag nor amplitude/phase parameters found in'
problemMSG[20]='An input data set is empty, check predecessor'

problemGrep[21]='Segmentation fault'
problemMSG[21]='Major error in on of the executables. Check input data, else warn pipeline dev.'

problemGrep[22]='Errno 104.* Connection reset by peer'
problemMSG[22]='Network reset while communicating with node. If problem persist for the same node suspect faulty hardware, else restart'

problemGrep[23]='global name .* is not defined'
problemMSG[23]='Known and solved bug in pulp/pipeline integration'

problemGrep[24]='pulsar_pipeline.* object has no attribute .*parset_file'
problemMSG[24]='Known and solved bug in pulp/pipeline integration'

problemGrep[25]='ImportError.*cannot open shared object file: No such file or directory'
problemMSG[25]='Executable has problem finding librairy, suspect deployment error'

problemGrep[26]='Failed to validate data map'
problemMSG[26]='Internal pipeline problem warn pipeline developer'

problemGrep[27]='All makevds processes failed.* Bailing out'
problemMSG[27]='Makevds failed on all nodes, suspect corrupt input data.'

problemGrep[28]='Target source name cannot be given if ignoretarget'
problemMSG[28]='NDPPP specification error'

problemGrep[29]='bbs-reducer.* returned non-zero exit status 1'
problemMSG[29]='Error in BBS, check inputdata and specification'

problemGrep[30]='Number of entries in the source and target map is not the same'
problemMSG[30]='suspect incorrect specification of input or output products'


lengthProblemArray=${#problemGrep[@]}
lengthProblemArray=$((lengthProblemArray-1)) # substract one to get correct rage in for loop
for i in `seq 0 $lengthProblemArray`  # for all the known problems
do
	problemCount[$i]=0
done

# A set of more complex problems: An error is matched if it occurs more then 20 times
complexProblemGrep[0]='rsync: change_dir .* failed:'
complexProblemMSG[0]='MISSING INPUT FILES: A large number of copier steps failed'

complexProblemGrep[1]='vdsmaker: Dataset .* does not exi'
complexProblemMSG[1]='MISSING INPUT FILES: vdsmaker reported A large number of missing input measurement sets'

complexProblemGrep[2]='copier: rsync: link_stat .* failed: No such file or directory'
complexProblemMSG[2]='MISSING INPUT FILES: copier: rsync: No such file or directory '

lengthComplexProblemArray=${#complexProblemGrep[@]}
lengthComplexProblemArray=$((lengthComplexProblemArray-1)) # substract one to get correct rage in for loop
for i in `seq 0 $lengthComplexProblemArray`  # for all the known problems
do
	complexProblemCount[$i]=0
done

skipObsIDArray=(78815 78863 274027 344688 346978)

echo "-------------------------------------------------------------"

nPipelines=0
nPipelineFail=0
nPipelineSkip=0
# Loop all the files
echo "Analysis of large number of files is time consuming, no progress indication possible in the first steps"
echo "logfiles containing unknown error:"
for f in /opt/lofar/var/run/pipeline/Observation$obsIDExpression/logs/*/pipeline.log*
do
  # if we have a gz file we first need to unpack it
  logloc=$f

  if [ "${f: -2}" == "gz" ] 
  then
    gunzip -c $f > pipeline.log
    f="pipeline.log"
  fi
  # Count the number of pipeline logs
  nPipelines=$((nPipelines+1)) 
  # The first logline contains as the 4th word the name of the pipeline
  pipelineName=$(head -1 $f | awk '{print $4}')  
  # convert to integer pipeline id
  if [ "$pipelineName" == "msss_imager_pipeline:" ] 
  then
    pipelineType=1
  elif [ "$pipelineName" == "msss_calibrator_pipeline:" ] 
  then
    pipelineType=2   
  elif [ "$pipelineName" == "msss_target_pipeline:" ]
  then
    pipelineType=3  
  elif [ "$pipelineName" == "calibration_pipeline:" ]
  then
    pipelineType=4 
  elif [ "$pipelineName" == "preprocessing_pipeline:" ]
  then
    pipelineType=5	
  elif [ "$pipelineName" == "long_baseline_pipeline:" ]
  then
    pipelineType=6	
  elif [ "$pipelineName" == "pulsar_pipeline:" ]
  then
    pipelineType=7		
  elif [ "$pipelineName" == "imaging_pipeline:" ]
  then
    pipelineType=8		
  else
  echo $pipelineName
    pipelineType=0  
  fi
 

  # Use the pipeline idx to increase the corresponding array with counts for type 
  nPipelineByType[$pipelineType]=$((nPipelineByType[$pipelineType] + 1))

  # Look for failing pipelines
  if tail -1 $f | grep "_pipeline completed with errors" > /dev/null
  then   # If failing pipeline

      nPipelineFail=$((nPipelineFail+1)) 
      nPipelineByTypeFail[$pipelineType]=$((nPipelineByTypeFail[$pipelineType]+1)) # count by type

	  # Loop the known simple problems, count the total error per pipeline and stop with file process
      errorFound=false
      for i in `seq 0 $lengthProblemArray`  # for all the known problems
	  do

	    if grep -c "${problemGrep[$i]}" $f > /dev/null  # grab with the literal string
	    then
		  problemCount[$i]=$((problemCount[$i]+1))

		  if $displayProblem
		  then
		    echo "---------------------------------------------------"
		    echo $logloc
			echo ${problemMSG[$i]}    # Echo the message
			echo "Manually: " "grep -c \"${problemGrep[$i]}\"" "<file>"
		  fi
		  errorFound=true
	      nPipelineByTypeFailExplained[$pipelineType]=$((nPipelineByTypeFailExplained[$pipelineType]+1))  # count the solved problems
		  
		  break
	    fi
	  done
	  # Stop processing file of the error was found in the simple match loop
	  if $errorFound
	  then
	    continue
	  fi

	  # Loop the known complex problems, count the total error per pipeline and stop with file process
      errorFound=false
      for i in `seq 0 $lengthComplexProblemArray`  # for all the known problems
	  do
	    countOfMatches=$(grep -c "${complexProblemGrep[$i]}" $f )
	    if [ $countOfMatches -ge 8 ] # grab with the literal string
	    then
		  complexProblemCount[$i]=$((complexProblemCount[$i]+1))
		  if $displayProblem
		  then
		    echo "---------------------------------------------------"
		    echo $logloc
			echo ${complexProblemMSG[$i]}    # Echo the message
			echo "Manually: " "grep -c \"${complexProblemGrep[$i]}\"" "<file>"
		  fi
		  errorFound=true
	      nPipelineByTypeFailExplained[$pipelineType]=$((nPipelineByTypeFailExplained[$pipelineType]+1))  # count the solved problems
		  
		  break
	    fi
	  done
	  # Stop processing file of the error was found in the simple match loop
	  if $errorFound
	  then
	    continue
	  fi	  

	  # Test of the current file is in the skipped obsid list: Allows skipping
	  # of logfiles with unique errors
	  skipObsID=false
	  for entry in  "${skipObsIDArray[@]}" 
	  do
	     if [[ $logloc == *$entry* ]]
		 then
		   skipObsID=true
		   nPipelineSkip=$((nPipelineSkip+1)) 
		   break
		 fi
	  done
	  
	  if $skipObsID
	  then
	    continue
	  fi	
	  
	  # Print logfile if cause is unknown
	  echo $logloc
  fi

done
# Get the number of parsets ( minus one for the pipeline subdir)
nparsets=$(ls -1 /opt/lofar/var/run/pipeline/ | wc -l)
nparsets=$((nparsets-1)) 
# Display the result
echo
echo "------------------------------------------------"
echo "number of parsets      : " $nparsets
echo "total number of pipeline logfiles: " $nPipelines
echo "Failing pipelines                : " $nPipelineFail       
echo

echo pipeline type "                   : total , error , explained "
echo -------------------------------------------------
echo unknown_pipeline "                :" ${nPipelineByType[0]} "," ${nPipelineByTypeFail[0]} "," ${nPipelineByTypeFailExplained[0]}
echo msss_imager_pipeline "            :" ${nPipelineByType[1]} "," ${nPipelineByTypeFail[1]} "," ${nPipelineByTypeFailExplained[1]}
echo msss_calibrator_pipeline "        :" ${nPipelineByType[2]} "," ${nPipelineByTypeFail[2]} "," ${nPipelineByTypeFailExplained[2]}
echo msss_target_pipeline "            :" ${nPipelineByType[3]} "," ${nPipelineByTypeFail[3]} "," ${nPipelineByTypeFailExplained[3]}
echo calibration_pipeline "            :" ${nPipelineByType[4]} "," ${nPipelineByTypeFail[4]} "," ${nPipelineByTypeFailExplained[4]}
echo preprocessing_pipeline "          :" ${nPipelineByType[5]} "," ${nPipelineByTypeFail[5]} "," ${nPipelineByTypeFailExplained[5]}
echo long_baseline_pipeline "          :" ${nPipelineByType[6]} "," ${nPipelineByTypeFail[6]} "," ${nPipelineByTypeFailExplained[6]}
echo pulsar_pipeline "                 :" ${nPipelineByType[7]} "," ${nPipelineByTypeFail[7]} "," ${nPipelineByTypeFailExplained[7]}
echo imaging_pipeline "                :" ${nPipelineByType[8]} "," ${nPipelineByTypeFail[8]} "," ${nPipelineByTypeFailExplained[8]}
echo

echo "------------------------------------------------"
echo "detailed information simple grep"
echo "count : msg"
for i in `seq 0 $lengthProblemArray`  # for all the known problems
do
  echo ${problemCount[$i]} " : " ${problemMSG[$i]} 
done
echo $nPipelineSkip " :  OTHER: Failing pipelines with unique singular cause of failure"
echo

echo "------------------------------------------------"
echo "detailed information counted greps"
echo "count : msg"
for i in `seq 0 $lengthComplexProblemArray`  # for all the known problems
do
  echo  ${complexProblemCount[$i]} " : " ${complexProblemMSG[$i]} 
done
echo "------------------------------------------------"
