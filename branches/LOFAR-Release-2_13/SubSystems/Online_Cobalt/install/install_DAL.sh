#!/bin/bash -eu

# We need to be lofarbuild to have the proper writing rights
[ "`whoami`" == "lofarbuild" ]

# Download location for the latest DAL source
DAL_SOURCE="https://github.com/nextgen-astrodata/DAL.git"

# ********************************************
#  Install latest DAL
#
#  into /opt/DAL
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
cmake -DCMAKE_INSTALL_PREFIX=/opt/DAL .. > cmake.log

echo "  Building..."
make -j 8 > make.log

echo "  Testing..."
ctest -j 8 > ctest.log

echo "  Installing..."
make -j 8 install > make_install.log

echo "  Cleaning up..."
popd >/dev/null
rm -rf "$DALDIR"

