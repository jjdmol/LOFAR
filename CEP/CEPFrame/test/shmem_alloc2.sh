#!/bin/sh
if [ "X$srcdir" = "X" ]; then
    ( ./shmem_alloc2 -server | ./shmem_alloc2 -client ) > shmem_alloc2_test.log 2>&1
else
    $srcdir/cleanipc                                        > shmem_alloc2_test.log 2>&1
       ( ./shmem_alloc2 -server | ./shmem_alloc2 -client ) >> shmem_alloc2_test.log 2>&1 \
    && $srcdir/checkipc                                    >> shmem_alloc2_test.log 2>&1
    $srcdir/cleanipc                                       >> shmem_alloc2_test.log 2>&1
fi
