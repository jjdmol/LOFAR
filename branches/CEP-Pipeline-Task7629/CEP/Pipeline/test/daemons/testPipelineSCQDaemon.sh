#!/bin/bash

# Create the a bus structure to be used by the test program
createbus.sh add testPipelineSCQDaemon $(hostname) 

./runctest.sh testPipelineSCQDaemon

# ate the a bus structure to be used by the test program
createbus.sh del testPipelineSCQDaemon $(hostname) ignore