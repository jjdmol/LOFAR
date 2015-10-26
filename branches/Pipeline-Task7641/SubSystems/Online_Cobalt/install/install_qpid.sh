#!/bin/bash -eu

# Note: this script is derived from the QPID build procedure
# described on http://www.lofar.org/wiki/doku.php?id=qpid:build

# We need to be lofarbuild to have the proper writing rights
[ "`whoami`" == "lofarbuild" ]

# Download location for the latest QPID source
PROTON_SOURCE="http://svn.apache.org/repos/asf/qpid/proton/branches/0.8"
QPID_SOURCE="http://svn.apache.org/repos/asf/qpid/branches/0.30/qpid/"

SOURCE_DIR=`dirname "$0"`
echo "$SOURCE_DIR"
exit 1

QPID_INSTALLDIR=/opt/qpid-0.30
QPID_SYMLINK=/opt/qpid

# ********************************************
#  Install latest PROTON & QPID
#
#  into $QPID_INSTALLDIR
# ********************************************
echo "Configuring PROTON and QPID..."
mkdir -p $QPID_INSTALLDIR

QPID_BUILDDIR=`mktemp -d`
pushd $QPID_BUILDDIR >/dev/null

echo "  Downloading PROTON..."
svn co $PROTON_SOURCE proton >/dev/null

echo "  Configuring PROTON..."
pushd proton >/dev/null
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$QPID_INSTALLDIR -DBUILD_PERL=OFF .. > cmake.log

echo "  Building PROTON..."
make -j 8 > make.log

echo "  Installing PROTON..."
make -j 8 install > make_install.log

# back to QPID_BUILDDIR
popd >/dev/null

echo "  Downloading QPID..."
svn co $QPID_SOURCE qpid >/dev/null

echo "  Configuring QPID C bindings..."
pushd qpid/cpp >/dev/null
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$QPID_INSTALLDIR -DProton_DIR=/localhome/lofar/qpid/lib/cmake/Proton -DBUILD_XML=OFF -DBUILD_SSL=OFF -DBUILD_BINDING_RUBY=OFF .. > cmake.log

echo "  Building QPID C bindings..."
make -j 8 > make.log

echo "  Installing QPID C bindings..."
make -j 8 install > make_install.log

# back to QPID_BUILDDIR
popd >/dev/null

echo "  Building and installing QPID Python bindings..."
pushd qpid/python >/dev/null
./setup.py build > setup_build.log
./setup.py install --home=$QPID_INSTALLDIR > setup_install.log

echo "  Building and installing QPID Python QMF..."
popd >/dev/null
pushd qpid/extras/qmf >/dev/null
./setup.py build > setup_build.log
./setup.py install --home=$QPID_INSTALLDIR > setup_install.log

echo "  Building and installing QPID Python tools..."
popd >/dev/null
pushd qpid/tools >/dev/null
./setup.py build > setup_build.log
./setup.py install --home=$QPID_INSTALLDIR > setup_install.log

# Go back to original dir
popd >/dev/null

echo "  Creating .profile..."
PYTHONVERSION=`python -c 'import platform; print "%s.%s" % platform.python_version_tuple()[0:2]'`
cat > $QPID_INSTALLDIR/.profile << EOF
export PATH=\$PATH:$QPID_INSTALLDIR/sbin/:$QPID_INSTALLDIR/bin/:$QPID_INSTALLDIR/local/bin/
export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$QPID_INSTALLDIR/lib:$QPID_INSTALLDIR/local/lib/
export PYTHONPATH=\$PYTHONPATH:$QPID_INSTALLDIR/lib/python/:$QPID_INSTALLDIR/lib/python$PYTHONVERSION/site-packages/
EOF

echo "  Installing configuration files..."
cp $SOURCE_DIR/qpidd.conf $QPID_INSTALLDIR/etc/qpid/

echo "  Creating symlink $QPID_SYMLINK..."
ln -sfT $QPID_INSTALLDIR $QPID_SYMLINK

echo "  Cleaning up..."
rm -rf "$QPID_BUILDDIR"

