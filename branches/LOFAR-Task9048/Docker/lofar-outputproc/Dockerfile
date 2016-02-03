#
# base
#
FROM lofar-base

#
# *******************
#   LOFAR
# *******************
#

ENV LOFAR_BRANCH=trunk

# Run-time dependencies
RUN sudo apt-get install -y liblog4cplus-1.0-4 libxml2 libboost-thread${BOOST_VERSION}.0 libboost-filesystem${BOOST_VERSION}.0 libboost-date-time${BOOST_VERSION}.0 libpng12-0 libsigc++-2.0-dev libxml++2.6-2 libboost-regex${BOOST_VERSION}.0

# Install
RUN sudo apt-get install -y subversion cmake g++ gfortran bison flex autogen liblog4cplus-dev libhdf5-dev libblitz0-dev libboost-dev libboost-python${BOOST_VERSION}-dev libxml2-dev pkg-config libpng12-dev libfftw3-dev libunittest++-dev libxml++2.6-dev libboost-filesystem${BOOST_VERSION}-dev libboost-date-time${BOOST_VERSION}-dev libboost-thread${BOOST_VERSION}-dev libboost-regex${BOOST_VERSION} binutils-dev && \
    mkdir -p ${INSTALLDIR}/lofar/build/gnu_opt && \
    cd ${INSTALLDIR}/lofar && \
    svn --non-interactive -q --username lofar-guest --password lofar-guest co -N https://svn.astron.nl/LOFAR/${LOFAR_BRANCH} src; \
    svn --non-interactive -q up src/CMake && \
    cd ${INSTALLDIR}/lofar/build/gnu_opt && cmake -DBUILD_PACKAGES=Online_OutputProc -DCMAKE_INSTALL_PREFIX=${INSTALLDIR}/lofar/ -DCASACORE_ROOT_DIR=${INSTALLDIR}/casacore/ -DLOG4CPLUS_ROOT_DIR=${INSTALLDIR}/log4cplus/ -DQPID_ROOT_DIR=/opt/qpid/ -DDAL_ROOT_DIR=${INSTALLDIR}/DAL -DUSE_OPENMP=True ${INSTALLDIR}/lofar/src/ && \
    cd ${INSTALLDIR}/lofar/build/gnu_opt && sed -i '29,31d' include/ApplCommon/PosixTime.h && \
    cd ${INSTALLDIR}/lofar/build/gnu_opt && make -j ${J} && \
    cd ${INSTALLDIR}/lofar/build/gnu_opt && make install && \
    bash -c "strip ${INSTALLDIR}/lofar/{bin,sbin,lib64}/* || true" && \
    bash -c "rm -rf ${INSTALLDIR}/lofar/{build,src}" && \
    sudo apt-get purge -y subversion cmake g++ gfortran bison flex autogen liblog4cplus-dev libhdf5-dev libblitz0-dev libboost-dev libboost-python${BOOST_VERSION}-dev libxml2-dev pkg-config libpng12-dev libfftw3-dev libunittest++-dev libxml++2.6-dev libboost-filesystem${BOOST_VERSION}-dev libboost-date-time${BOOST_VERSION}-dev libboost-thread${BOOST_VERSION}-dev binutils-dev && \
    sudo apt-get autoremove -y

