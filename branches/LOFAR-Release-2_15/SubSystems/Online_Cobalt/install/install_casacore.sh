#!/bin/bash -eu

# We need to be lofarbuild to have the proper writing rights
[ "`whoami`" == "lofarbuild" ]

# Download location for the casacore source.
#
# Note: if a new version is used, update:
#   lofarsys/bashrc                   (to use the new version in $PATH)
#   /CMake/variants/variants.cbm001   (to compile against the new version)

#CASACORE_SOURCE="ftp://ftp.atnf.csiro.au/pub/software/casacore/casacore-1.7.0.tar.bz2"
CASACORE_SOURCE="https://github.com/casacore/casacore/archive/v2.0.1.tar.gz"

function strip_extensions {
  # strip all extensions that are non-numeric
  sed -r 's/([.][A-Za-z][A-Za-z0-9]*)*$//'
}

# ********************************************
#  Install specified casacore
#
#  into /localhome/lofar/casacore-X.Y.Z
# ********************************************
echo "Configuring casacore..."
CASACOREDIR=`mktemp -d`
pushd $CASACOREDIR >/dev/null

echo "  Downloading..."
wget -q -N --tries=3 "$CASACORE_SOURCE"
echo "  Analyzing..."
TARBALL_FILENAME=`basename $CASACORE_SOURCE`
CASACORE_VERSION=`tar tf "$TARBALL_FILENAME" | head -n 1 | cut -d/ -f1`
echo "  Version is $CASACORE_VERSION"
echo "  Extracting..."
tar xf "$TARBALL_FILENAME"
cd $CASACORE_VERSION

echo "  Configuring..."
mkdir build
cd build
# Note that the -DDATA_DIR option does not fully override the measures tables search paths.
# In fact, some of ., ./data, $HOME/aips++/data, $HOME/data, $HOME/casacore/data are still searched first.
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/$CASACORE_VERSION \
  -DUSE_HDF5=ON -DUSE_FFTW3=ON -DFFTW3_DISABLE_THREADS=OFF \
  -DUSE_THREADS=ON -DUSE_OPENMP=ON \
  -DDATA_DIR=/localhome/lofar/IERS/current/data > cmake.log

echo "  Building..."
make -j 8 > make.log 2>&1 # casacore build is NOT clean

echo "  Installing..."
make -j 8 install > make_install.log

echo "  Validating measures tables..."
/opt/$CASACORE_VERSION/bin/findmeastable >/dev/null

echo "  Creating softlink /opt/casacore..."
ln -sfT /opt/$CASACORE_VERSION /opt/casacore

echo "  Cleaning up..."
popd >/dev/null
rm -rf "$CASACOREDIR"

