#!/bin/bash -eu

# We need to be lofarbuild to have the proper writing rights
[ "`whoami`" == "lofarbuild" ]

# Download location for latest IERS measures tables
#
# See also:
#  https://svn.cv.nrao.edu/svn/casa-data/trunk 
#  ftp://ftp.atnf.csiro.au/pub/software/measures_data (not updated since 2013?)
MEASURES_SOURCE="ftp://ftp.astron.nl/outgoing/Measures/WSRT_Measures.ztar"

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

