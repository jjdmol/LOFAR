#!/bin/sh

for d in lib tests example ; do
    (cd $d; ./build.sh "$@") || exit 1
done
