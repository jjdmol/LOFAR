#! /bin/bash
if [[ "$1" == "" ]] ; then
    echo usage: $0 \<version-name\>
else
    cd /tmp
    rm -rf aoexport
    mkdir -p aoexport/LOFAR/CEP/DP3
    mkdir -p aoexport/LOFAR/LCS/Common
    cd aoexport
    svn export https://svn.astron.nl/LOFAR/trunk/CEP/DP3/AOFlagger/ LOFAR/CEP/DP3/AOFlagger
    svn export https://svn.astron.nl/LOFAR/trunk/LCS/Common/include LOFAR/LCS/Common/include
    svn export https://svn.astron.nl/LOFAR/trunk/LCS/Common/src LOFAR/LCS/Common/src
    svn export https://svn.astron.nl/LOFAR/trunk/CMake LOFAR/CMake
    svn export https://svn.astron.nl/LOFAR/trunk/lofar_config.h.cmake LOFAR/lofar_config.h.cmake
    svn export https://svn.astron.nl/LOFAR/trunk/lofarinit.sh.in LOFAR/lofarinit.sh.in
    svn export https://svn.astron.nl/LOFAR/trunk/lofarinit.csh.in LOFAR/lofarinit.csh.in
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-root-without-lofarstman.txt LOFAR/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-CEP.txt LOFAR/CEP/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-DP3.txt LOFAR/CEP/DP3/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-AOFlagger-without-lofarstman.txt LOFAR/CEP/DP3/AOFlagger/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-LCS.txt LOFAR/LCS/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-Common-without-lofarstman.txt LOFAR/LCS/Common/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/scripts/CMakeLists-Common-src-without-lofarstman.txt LOFAR/LCS/Common/src/CMakeLists.txt
    cp LOFAR/CEP/DP3/AOFlagger/README LOFAR/README
    CONF_FILE="LOFAR/CEP/DP3/AOFlagger/include/AOFlagger/configuration.h"
    echo \#ifndef CONFIGURATION_H > ${CONF_FILE}
    echo \#define CONFIGURATION_H >> ${CONF_FILE}
    echo \#define NO_LOFARSTMAN >> ${CONF_FILE}
    echo \#endif >> ${CONF_FILE}
    rm -f AOFlagger-without-lofarstman-$1.tar.bz2
    tar -cjvf AOFlagger-without-lofarstman-$1.tar.bz2 LOFAR/
    rm -rf LOFAR/
    ls -alh AOFlagger-without-lofarstman-$1.tar.bz2
fi
