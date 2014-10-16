#!/bin/bash -eu

# Download location for latest IERS measures tables
#
# See also:
#  https://svn.cv.nrao.edu/svn/casa-data/trunk 
#  ftp://ftp.atnf.csiro.au/pub/software/measures_data (not updated since 2013?)
MEASURES_SOURCE="ftp://ftp.astron.nl/outgoing/Measures/WSRT_Measures.ztar"

# Download location for the latest DAL source
DAL_SOURCE="https://github.com/nextgen-astrodata/DAL.git"

# Download location for the casacore source.
#
# Note: if a new version is used, update:
#   lofarsys/bashrc                   (to use the new version in $PATH)
#   /CMake/variants/variants.cbm001   (to compile against the new version)
CASACORE_SOURCE="ftp://ftp.atnf.csiro.au/pub/software/casacore/casacore-1.7.0.tar.bz2"

function error {
  echo "$@" >&2
  exit 1
}

function strip_extensions {
  # strip all extensions that are non-numeric
  sed -r 's/([.][A-Za-z][A-Za-z0-9]*)*$//'
}

function postinstall_lofarsys {
  # ********************************************
  #  Populate ~/.ssh directory
  #
  #  We draw a copy from /globalhome/lofarsystem
  #  to stay in sync with the rest of LOFAR.
  # ********************************************
  echo "Configuring ssh..."
  cp -a /globalhome/lofarsystem/.ssh ~

  echo "  Testing..."
  ssh localhost true >/dev/null

  # ********************************************
  #  Install bash initialisation scripts
  # ********************************************
  echo "Configuring bash..."
  cp lofarsys/bashrc ~/.bashrc
  cp lofarsys/bash_profile ~/.bash_profile

  echo "  Validating .bashrc..."
  source ~/.bashrc >/dev/null

  # ********************************************
  #  Create directories for LOFAR's
  #  temporary files
  # ********************************************
  echo "Configuring /opt/lofar/var..."
  mkdir -p ~/lofar/var/{log,run}
}

function postinstall_lofarbuild {
  # ********************************************
  #  Install latest IERS measures tables
  #
  #  into /localhome/lofar/IERS/IERS-YYYY-MM-DDTHH:MM:SS/data
  # ********************************************
  echo "Configuring measures tables..."
  mkdir -p /localhome/lofar/IERS
  pushd /localhome/lofar/IERS >/dev/null

  IERS_DIRNAME="IERS-`date +%FT%T`"
  mkdir $IERS_DIRNAME
  cd $IERS_DIRNAME

  echo "  Downloading..."
  wget -q -N --tries=3 $MEASURES_SOURCE
  echo "  Extracting..."
  # we need data/ephemerides/ and data/geodetic/
  mkdir data
  cd data
  tar xf ../`basename $MEASURES_SOURCE` # produces ephemerides/ and geodetic/
  cd ..

  # TODO: validate tables? Alexander/Michiel have (partial) scripts

  echo "  Latest is `readlink -f .`"
  ln -sfT `readlink -f .` /localhome/lofar/IERS/current
  popd >/dev/null

  # ********************************************
  #  Install latest DAL
  #
  #  into /localhome/lofar/DAL
  # ********************************************
  echo "Configuring DAL..."
  DALDIR=`mktemp -d`
  pushd $DALDIR >/dev/null

  echo "  Downloading..."
  git clone -q $DAL_SOURCE
  cd DAL

  echo "  Configuring..."
  mkdir build
  cd build
  cmake -DCMAKE_INSTALL_PREFIX=/localhome/lofar/DAL .. > cmake.log

  echo "  Building..."
  make -j 8 > make.log

  echo "  Testing..."
  ctest -j 8 > ctest.log

  echo "  Installing..."
  make -j 8 install > make_install.log

  echo "  Cleaning up..."
  popd >/dev/null
  rm -rf "$DALDIR"

  # ********************************************
  #  Install specified casacore
  #
  #  into /localhome/lofar/casacore-X.Y.Z
  # ********************************************
  echo "Configuring casacore..."
  CASACOREDIR=`mktemp -d`
  pushd $CASACOREDIR >/dev/null

  CASACOREVERSION=`basename "$CASACORE_SOURCE" | strip_extensions`
  echo "  Version is $CASACOREVERSION"

  echo "  Downloading..."
  wget -q -N --tries=3 $CASACORE_SOURCE
  echo "  Extracting..."
  tar xf `basename $CASACORE_SOURCE`
  cd $CASACOREVERSION

  echo "  Configuring..."
  mkdir build
  cd build
  # Note that the -DDATA_DIR option does not fully override the measures tables search paths.
  # In fact, some of ., ./data, $HOME/aips++/data, $HOME/data, $HOME/casacore/data are still searched first.
  cmake .. -DCMAKE_INSTALL_PREFIX=/localhome/lofar/$CASACOREVERSION \
    -DUSE_HDF5=ON -DUSE_FFTW3=ON -DFFTW3_DISABLE_THREADS=OFF \
    -DUSE_THREADS=ON -DUSE_OPENMP=ON \
    -DDATA_DIR=/localhome/lofar/IERS/current/data > cmake.log

  echo "  Building..."
  make -j 8 > make.log 2>&1 # casacore build is NOT clean

  echo "  Installing..."
  make -j 8 install > make_install.log

  echo "  Validating measures tables..."
  /localhome/lofar/$CASACOREVERSION/bin/findmeastable >/dev/null

  echo "  Cleaning up..."
  popd >/dev/null
  rm -rf "$CASACOREDIR"
}

case "`whoami`" in
  lofarsys)
    postinstall_lofarsys
    ;;
  lofarbuild)
    postinstall_lofarbuild
    ;;
  *)
    error "Need to be either lofarsys or lofarbuild!"
    ;;
esac

echo "Done!"

