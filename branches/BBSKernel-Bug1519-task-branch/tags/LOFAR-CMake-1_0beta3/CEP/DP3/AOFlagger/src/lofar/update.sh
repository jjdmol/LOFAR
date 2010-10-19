#! /bin/bash
if [[ "$1" == "" ]] ; then
  LOCATION="/data/users/offringa/lofar-svn"
else
	LOCATION="$1"
fi
echo Updating from SVN location: ${LOCATION}
cp ${LOCATION}/LCS/Common/include/Common/*.h include/Common/
cp ${LOCATION}/RTCP/LofarStMan/include/LofarStMan/*.h include/LofarStMan/
cp ${LOCATION}/RTCP/LofarStMan/src/*.cc .
echo Done.
