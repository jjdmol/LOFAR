#!/bin/sh

   cat CB.*.write > CB.writers \
&& cat CB.*.read  > CB.readers \
&& gnuplot < gnuplot.cmd \
&& (cat CB.writers | cut -d\  -f4 | sort -n > CB.writers.sort) \
&& (cat CB.readers | cut -d\  -f4 | sort -n > CB.readers.sort) \
&& diff -C1 CB.writers.sort CB.readers.sort > CB.diff 2>&1
