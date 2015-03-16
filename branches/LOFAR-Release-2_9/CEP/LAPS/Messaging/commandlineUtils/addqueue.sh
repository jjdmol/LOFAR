#!/bin/bash

qpid-config -a localhost add queue $1 --durable
