#!/bin/bash 
# $Id: ltastorageoverview_build.sh 32113 2015-08-03 10:07:57Z schaap $

if [ "$#" -ne 1 ]; then
    echo "Usage: ./ltastorageoverview_build.sh <tag>"
    echo "where tag is a name or version number which is added to the tarballs."
    echo "This script creates two build flavours (local_dev/lexar) in source_root_dir/build and builds ltastorageoverview"
    echo "Then it performs a local install (in the each specific build dir) and creates a deployable tarball"
    echo "Final result is a tarball in source_root_dir/build which can be copied to the ingest servers"
    exit 1
fi

#get path of this build script and determine source root from there
REL_PATH="`dirname \"$0\"`"
ABS_PATH="`( cd \"$REL_PATH\" && pwd )`"
SOURCE_ROOT="$ABS_PATH/../.."

echo "Using '$SOURCE_ROOT' as source route"

BUILD_TAG="$1"
echo "Using Build tag $BUILD_TAG"

LOCAL_DEV_BUILD_DIR=$SOURCE_ROOT/build/local_dev/gnu_debug
LOCAL_DEV_INSTALL_DIR=$LOCAL_DEV_BUILD_DIR/local_install

mkdir -p $LOCAL_DEV_BUILD_DIR

cd $LOCAL_DEV_BUILD_DIR && cmake -DBUILD_PACKAGES=ltastorageoverview -DCMAKE_INSTALL_PREFIX=$LOCAL_DEV_INSTALL_DIR/ltastorageoverview__$BUILD_TAG $SOURCE_ROOT
cd $LOCAL_DEV_BUILD_DIR && make && make local_dev && rm -rf $LOCAL_DEV_INSTALL_DIR && make install


LEXAR_BUILD_DIR=$SOURCE_ROOT/build/lexar/gnu_debug
mkdir -p $LEXAR_BUILD_DIR

cd $LEXAR_BUILD_DIR && cmake -DBUILD_PACKAGES=ltastorageoverview -DCMAKE_INSTALL_PREFIX=/globalhome/ingest/ltastorageoverview_$BUILD_TAG $SOURCE_ROOT
cd $LEXAR_BUILD_DIR && make && rm -rf ./local_install && make DESTDIR=./local_install install
cd $LEXAR_BUILD_DIR/local_install/globalhome/ingest && tar cvzf $SOURCE_ROOT/build/ltastorageoverview_"$BUILD_TAG"_lexar.tgz ltastorageoverview_$BUILD_TAG
