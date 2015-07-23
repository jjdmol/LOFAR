#!/bin/bash

createbus.sh add testSCQLib $(hostname) 

./runctest.sh testSCQLib

createbus.sh del testSCQLib $(hostname) 