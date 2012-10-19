#!/bin/sh
echo "impl" > tConverter.in
./runctest.sh -stdout tConverter > tConverter_impl.log
echo "client localhost 31339" > tConverter.in
./runctest.sh -stdout tConverter > tConverter_client.log
