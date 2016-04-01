#!/bin/bash 
# $Id$

if [ "$#" -ne 1 ]; then
    echo "Usage: ./ltaingest_build.sh <tag>"
    echo "where tag is a name or version number which is added to the tarballs."
    echo "This script creates two build flavours (test/lexar) in source_root_dir/build and builds LTAIngest"
    echo "Then it performs a local install (in the each specific build dir) and creates a deployable tarball"
    echo "Final result is two tarballs in source_root_dir/build which can be copied to the ingest servers"
    exit 1
fi

#get path of this build script and determine source root from there
REL_PATH="`dirname \"$0\"`"
ABS_PATH="`( cd \"$REL_PATH\" && pwd )`"
SOURCE_ROOT="$ABS_PATH/../.."

echo "Using '$SOURCE_ROOT' as source route"

BUILD_TAG="$1"
echo "Using Build tag $BUILD_TAG"

TEST_BUILD_DIR=$SOURCE_ROOT/build/test/gnu_debug
LEXAR_BUILD_DIR=$SOURCE_ROOT/build/lexar/gnu_debug

mkdir -p $TEST_BUILD_DIR
mkdir -p $LEXAR_BUILD_DIR

cd $TEST_BUILD_DIR && cmake -DBUILD_PACKAGES=LTAIngest -DLTAINGEST_BUILD_TARGET=test -DCMAKE_INSTALL_PREFIX=/globalhome/ingesttest/ltaingest_$BUILD_TAG $SOURCE_ROOT
cd $LEXAR_BUILD_DIR && cmake -DBUILD_PACKAGES=LTAIngest -DLTAINGEST_BUILD_TARGET=lexar -DCMAKE_INSTALL_PREFIX=/globalhome/ingest/ltaingest_$BUILD_TAG $SOURCE_ROOT

cd $TEST_BUILD_DIR && make && make test && rm -rf ./local_install && make DESTDIR=./local_install install
cd $LEXAR_BUILD_DIR && make && rm -rf ./local_install && make DESTDIR=./local_install install

cd $TEST_BUILD_DIR/local_install/globalhome/ingesttest && tar cvzf $SOURCE_ROOT/build/ltaingest_"$BUILD_TAG"_test.tgz ltaingest_$BUILD_TAG
cd $LEXAR_BUILD_DIR/local_install/globalhome/ingest && tar cvzf $SOURCE_ROOT/build/ltaingest_"$BUILD_TAG"_lexar.tgz ltaingest_$BUILD_TAG
