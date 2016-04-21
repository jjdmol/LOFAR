#
# base
#
FROM ubuntu:12.04

#
# common-environment
#
ENV USER=lofar
ENV INSTALLDIR=/opt

#
# environment
#
ENV DEBIAN_FRONTEND=noninteractive \
    PYTHON_VERSION=2.7

#
# versions
# Requires boost 1.48
# Remove casacore?
#
ENV CASACORE_VERSION=2.0.3 \
    CASAREST_VERSION=1.4.1 \
    PYTHON_CASACORE_VERSION=2.0.1 \
    BOOST_VERSION=1.48

#
# set-uid
#
ENV UID=1000

#
# set-build-options
#
ENV J=6

#
# Base and runtime dependencies
#
#RUN sed -i 's/archive.ubuntu.com/osmirror.rug.nl/' /etc/apt/sources.list 
RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y sudo 
#python2.7 libpython2.7 
#    apt-get install -y libblas3 liblapacke python-numpy libcfitsio3 libwcs4 libfftw3-bin libhdf5-7 libboost-python${BOOST_VERSION}.0 && \
#    apt-get install -y nano

#
# setup-account
#
RUN (getent group sudo &>/dev/null || groupadd sudo) && \
    echo "useradd -m ${USERADD_FLAGS} ${USER}" && \
    useradd -m -u ${UID} ${USER} && \
    usermod -a -G sudo ${USER} && \
    echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
    sed -i 's/requiretty/!requiretty/g' /etc/sudoers

#
# setup install dir
#
RUN mkdir -p ${INSTALLDIR} && chown ${USER}.${USER} ${INSTALLDIR}

USER ${USER}


#
# *******************
# Lofar User Software
# *******************
#
RUN sudo apt-get update && sudo apt-get upgrade -y && \
    sudo apt-get install -y g++ gfortran flex swig bison subversion \
                     zlib1g-dev libatlas-base-dev liblapack-dev \
                     libncurses5-dev libfreetype6-dev libpng12-dev \
                     python-dev python-tk python-pyfits tk8.5-dev fftw3-dev \
                     libbz2-dev libghc-readline-dev \
                     git git git-core git-doc git-man git-svn \
                     valgrind 

RUN sudo apt-get install -y libboost${BOOST_VERSION}-all-dev \
                     wcslib-dev \
                     cmake cmake-doc cmake-curses-gui make \
                     libgsl0-dev \
                     python-matplotlib \
                     python-sphinx \
                     libcfitsio3-dev \
                     python-numpy \
                     num-utils \
                     python-scipy \
                     libblas-dev \
                     python-sip-dev \
                     openmpi-bin openmpi-common \
                     ipython

RUN cd $INSTALLDIR && svn co http://usg.lofar.org/svn/code/trunk lofarsoft && \
    export LOFARSOFT=${INSTALLDIR}/lofarsoft && . $LOFARSOFT/devel_common/scripts/init.sh && \
    cd $LOFARSOFT && ./bootstrap && cd build && cmake -DCASACORE_FROM_LATEST_SVN_REVISION=ON . && make rebuild_cache    

ENV LOFARSOFT=${INSTALLDIR}/lofarsoft

RUN . $LOFARSOFT/devel_common/scripts/init.sh && cd $LOFARSOFT/build && make hdf5 

RUN . $LOFARSOFT/devel_common/scripts/init.sh && cd $LOFARSOFT/build && make dal1



#
# config
#
COPY bashrc /opt/bashrc

#
# entry
#
COPY chuser.sh /usr/local/bin/chuser.sh
WORKDIR /home/${USER}
ENTRYPOINT ["sudo","-E","/usr/local/bin/chuser.sh"]

