#!/bin/bash

source $9

cd ${10}

exec `dirname $0`/readms-part.py $@

