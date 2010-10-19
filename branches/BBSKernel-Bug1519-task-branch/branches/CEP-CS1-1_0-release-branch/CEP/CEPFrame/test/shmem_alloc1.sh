#!/bin/sh
if [ "X$srcdir" = "X" ]; then
       ( ./shmem_alloc1 -log     > shmem_alloc1_test.log 2>&1 ) \
    && ( ./shmem_alloc1 -linear >> shmem_alloc1_test.log 2>&1 )
else
    $srcdir/cleanipc             > shmem_alloc1_test.log 2>&1
       ( ./shmem_alloc1 -log    >> shmem_alloc1_test.log 2>&1 ) \
    && ( ./shmem_alloc1 -linear >> shmem_alloc1_test.log 2>&1 ) \
    && $srcdir/checkipc         >> shmem_alloc1_test.log 2>&1
    $srcdir/cleanipc            >> shmem_alloc1_test.log 2>&1
fi
