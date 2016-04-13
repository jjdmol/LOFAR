#!/bin/bash -x
#
#
# Script to create a feedback file if it has not been created automatically 
# by OLAP.
#
# Usage: createFeedbackFile <SASID>
#
# Must be run on lhn001 as user lofarsys. This scripts creates a feedback file
# called Observation<SASID>_feedback
#

# Directory for temporary storage of parsets
tmp_rootdir=/data/parsets

if [ $# -lt 1 -o $# -gt 1 ]; then
  echo "Provide a SASID to create the feedback file for."
  exit
fi

# Create runtime directory
date=`date +%Y-%m-%dT%H:%M:%S`
tmpdir=${tmp_rootdir}/$date

# For debugging only
tmpdir=$tmp_rootdir

mkdir -p $tmpdir
cd $tmpdir

sasid=$1

# Get original parset file
scp cbt001-10gb01:/opt/lofar/var/run/rtcp-${sasid}.parset ./L${sasid}.parset 2>&1 1>/dev/null

if [ $? -ne 0 ]; then 
  echo "Problem finding or copying parset file of this observation from cbt001-10gb01"
  exit 1
fi

# Generate MSes/h5 files; we only care about the loginfo so remove any created products
# Output goes into file tmp that can be parsed for feedback info
/globalhome/lofarsystem/production/OutputProc/sbin/createFeedback L${sasid}.parset >createFeedback_${sasid}.log 2>&1
if [ $? -ne 0 ]; then
  echo "Problem creating feedback information with createFeedback L${sasid}.parset"
  exit 1 
fi  

# Move the created feedback, since we need to adjust it still
mv Observation${sasid}_feedback Observation${sasid}_feedback.createFeedback

# Activate LOFAR pipeline environment
. /opt/cep/login/bashrc 2>&1 1>/dev/null
use Lofar 2>&1 1>/dev/null

# Make a map file to prepare for get_metadata run
createParsetMap.py $sasid 2>&1 1>createParsetMap_${sasid}.log

# Create a feedback file based on MS info; thiswill be used to extract 
# duration/size/percentagewritten info afterwards (the dynamic keys)
python $LOFARROOT/lib/python2.6/dist-packages/lofarpipe/recipes/master/get_metadata.py -d -c $LOFARROOT/share/pipeline/pipeline.cfg -j L${sasid} --product-type=Correlated --parset-file=L${sasid}.tmp L${sasid}.map 2>&1 1>get_metadata_${sasid}.log

if [ $? -ne 0 ]; then 
  echo "Could not extract metadata from data; see logfile ${tmpdir}/get_metadata_${sasid}.log"
  exit 1
fi

# Combine all information
(
# process feedback to extact the static keys:
#  1. Add LOFAR.ObsSW prefix
#  2. Remove dynamic keys (size, duration, percentageWritten)
<Observation${sasid}_feedback.createFeedback perl -ne '
  print "LOFAR.ObsSW.$_" unless /\.(duration|size|percentageWritten)=/;
'

# add dynamic keys from the parsetMap:
#  1. Add LOFAR.ObsSW.Observation.DataProducts. prefix
#  2. Only use dynamic keys (size, duration, percentageWritten)
<L${sasid}.tmp perl -ne '
  print "LOFAR.ObsSW.Observation.DataProducts.$_" if /\.(duration|size|percentageWritten)=/;
'

# Sort to create a consistent key order. As a bonus, sort the key arrays numerically.
) | sort -t[ -k 2 -n > Observation${sasid}_feedback

# Cleanup
rm -f createParsetMap_${sasid}.log get_metadata_${sasid}.log createFeedback_${sasid}.log L${sasid}.tmp L${sasid}.map L${sasid}.parset

# Ready now!
echo "Done; created file ${tmpdir}/Observation${sasid}_feedback"

