#!/bin/bash

createbus.sh add testPipelineSCQDaemon $(hostname) $(hostname) 

./runctest.sh testPipelineSCQDaemon

createbus.sh del testPipelineSCQDaemon $(hostname) $(hostname) ignore