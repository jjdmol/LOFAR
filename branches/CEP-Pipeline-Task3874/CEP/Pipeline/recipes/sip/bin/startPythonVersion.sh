# Script that parses an optional parset key specifying the Software version to use for the current pipeline run
# This script is to be sourced by the startPython.sh script and never standalone.
# Expects the folowing environment variable from the sourcing script: $parsetFile

# extract the wanted version from the parset
versionString="$(getparsetvalue $parsetFile "ObsSW.Observation.ObservationControl.PythonControl.softwareVersion" -d "notFound")"

if [ $versionString != "notFound" ]; then
  # construct the path from the red value
  versionPath=/opt/cep/lofar/lofar_versions/$versionString/lofar_build/
  echo "Using parset supplied software version: $versionString"
 
  # Provide Lofar with the new lofarversion: correctness is validated there
  use Lofar_test $versionString 
  
fi

