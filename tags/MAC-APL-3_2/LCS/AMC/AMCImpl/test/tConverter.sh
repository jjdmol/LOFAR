#!/bin/sh
echo "impl" > tConverter.in
$lofar_sharedir/runtest.sh tConverter > tConverter_impl.log
echo "client localhost 31339" > tConverter.in
$lofar_sharedir/runtest.sh tConverter > tConverter_client.log
