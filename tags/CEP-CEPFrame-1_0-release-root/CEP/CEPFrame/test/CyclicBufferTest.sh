#!/bin/sh

#
# Limit the running time of this test to get out of possible deadlock situation
#
   echo 'call "../../../test/CyclicBufferTest.gnuplot" "CB.writers" 0 20' >  gnuplot.cmd \
&& echo 'call "../../../test/CyclicBufferTest.gnuplot" "CB.readers" 0 20' >> gnuplot.cmd \
&& $common_top_builddir/src/LimitExec 90 ./CyclicBufferTest > CB.log 2> CB.err \
&& cat CB.*.write > CB.writers \
&& cat CB.*.read  > CB.readers \
&& gnuplot < gnuplot.cmd \
&& (cat CB.writers | cut -d\  -f4 | sort -n > CB.writers.sort) \
&& (cat CB.readers | cut -d\  -f4 | sort -n > CB.readers.sort) \
&& diff -C1 CB.writers.sort CB.readers.sort > CB.diff 2>&1
