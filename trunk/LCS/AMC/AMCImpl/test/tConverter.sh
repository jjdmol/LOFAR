#!/bin/sh
echo "impl" > tConverter.in
./runctest.sh -stdout tConverter
echo "client localhost 31339" > tConverter.in
./runctest.sh -stdout tConverter
