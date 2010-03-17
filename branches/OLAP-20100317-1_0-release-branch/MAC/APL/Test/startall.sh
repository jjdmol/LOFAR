#!/bin/sh

basepath=/home/lofartest/wierenga/LOFAR/installed/gnunew_opt
#basepath=/opt/lofar

if [ "$USER" != "root" ]; then
	echo `basename $0` must be run as root
	exit 1
fi

mkdir -p $basepath/var/log
cd $basepath/var/log

# list of all services
services="amcserver ServiceBroker RSPDriver" # CalServer BeamServer

# kill old services
for service in $services; do
	echo -n Killing $service...
	killall $service
	echo done
done

echo Waiting for 5 seconds...
sleep 5

# start new services
for service in $services; do
	echo -n Starting $service...
	nohup ../../bin/$service > ../../var/log/$service_out.log 2>&1 &
	sleep 1 # wait for service to start listening for connections
	echo done
done

