#!/bin/bash

for i in $( qls | grep laps | awk '{ print $1 }' ) ; do echo $i ; purgequeue $i ; done

cleanupq laps

