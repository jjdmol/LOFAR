#!/bin/bash

createbus.sh add testSCQLib $(hostname) $(hostname) 

./runctest.sh testSCQLib

createbus.sh del testSCQLib $(hostname) $(hostname) 