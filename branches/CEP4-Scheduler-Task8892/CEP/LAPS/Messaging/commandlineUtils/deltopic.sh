#!/bin/bash

qpid-config -a localhost add exchange topic $1 
#--durable
