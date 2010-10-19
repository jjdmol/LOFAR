#!/bin/sh
echo "impl" > tConverter.in
$lofar_sharedir/runtest.sh tConverter > tConverter_impl.log
echo "client" > tConverter.in
$lofar_sharedir/runtest.sh tConverter > tConverter_client.log
