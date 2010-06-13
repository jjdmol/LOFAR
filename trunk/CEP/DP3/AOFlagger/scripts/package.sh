#! /bin/bash
if [[ "$1" == "" ]] ; then
    echo usage: $0 \<version-name\>
else
    cd /tmp
    rm -rf aoexport
    mkdir -p aoexport/LOFAR/CEP/DP3
    mkdir -p aoexport/LOFAR/LCS
    cd aoexport
    svn export https://svn.astron.nl/LOFAR/trunk/CEP/DP3/AOFlagger/ LOFAR/CEP/DP3/AOFlagger
    svn export https://svn.astron.nl/LOFAR/trunk/LCS/Common LOFAR/LCS/Common
    svn export https://svn.astron.nl/LOFAR/trunk/CMake LOFAR/CMake
    svn export https://svn.astron.nl/LOFAR/trunk/RTCP/LofarStMan LOFAR/RTCP/LofarStMan
    svn export https://svn.astron.nl/LOFAR/trunk/lofar_config.h.cmake LOFAR/lofar_config.h.cmake
    svn export https://svn.astron.nl/LOFAR/trunk/lofarinit.sh.in LOFAR/lofarinit.sh.in
    svn export https://svn.astron.nl/LOFAR/trunk/lofarinit.csh.in LOFAR/lofarinit.csh.in
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-root.txt LOFAR/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-CEP.txt LOFAR/CEP/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-DP3.txt LOFAR/CEP/DP3/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-LCS.txt LOFAR/LCS/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-RTCP.txt LOFAR/RTCP/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/ LOFAR/README
    tar -cjvf AOFlagger-$1.tar.bz2 LOFAR/
fi
