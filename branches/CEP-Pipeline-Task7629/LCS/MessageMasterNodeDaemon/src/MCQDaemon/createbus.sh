#!/bin/sh 

function usage {
    echo "'createbus' creates a dynamic routing bus between two nodes." 
    echo "   createbus <busname> <node1> <node2>" 
	echo ""
    echo "If only one hostname is given the second hostname is presumed to be the local node." 
    echo "to ensure one-time delivery of messages avoid loops in the bus topology." 
	echo ""
}

function add_deadletter_topic {
    local hostname=$1
	local busname=$2
    echo "qpid-config -b $hostname add exchange topic $busname\".proxy.deadletter\" --durable" 
    qpid-config -b $hostname add exchange topic $busname".proxy.deadletter" --durable 
}

function add_deadletter_alternate_exchange {
    local hostname=$1
    local busname=$2
    echo "qpid-config -b $hostname add exchange direct $busname --durable --alternate-exchange=$busname\".proxy.deadletter\"" 
    qpid-config -b $hostname add exchange direct $busname --durable --alternate-exchange=$busname".proxy.deadletter" 
}

function add_deadletter_queue {
    local hostname=$1
    local busname=$2
    echo "qpid-config -b $hostname add queue $busname\".deadletter\" --durable" 
    qpid-config -b $hostname add queue $busname".deadletter" --durable 
}

function bind_deadletter_topic_to_queue {
    local hostname=$1
    local busname=$2
    echo "qpid-config -b $hostname bind $busname\".proxy.deadletter\" $busname\".deadletter\" '*' --durable" 
    qpid-config -b $hostname bind $busname".proxy.deadletter" $busname".deadletter" '*' --durable 
}

function add_routing {
    local hostname=$1
	local remotehost=$2
    local busname=$3
    echo "qpid-route -d dynamic add $remotehost $hostname $busname" 
    qpid-route -d dynamic add $remotehost $hostname $busname 
}

# createbus attempts to create a full instantiated bus structure between two nodes
function createbus {
    busname=$1
    remotehost=$2
    hostname=$3
		
	add_deadletter_topic $hostname $busname 
	add_deadletter_topic $remotehost $busname 

    add_deadletter_alternate_exchange $hostname $busname 
	add_deadletter_alternate_exchange $remotehost $busname 

    add_deadletter_queue $hostname $busname 
	add_deadletter_queue $remotehost $busname 

	bind_deadletter_topic_to_queue $hostname $busname 
	bind_deadletter_topic_to_queue $remotehost $busname 

	add_routing $remotehost $hostname $busname
	add_routing $hostname $remotehost $busname 
}
set -e
if [ $# != 3 ]
then
    usage
    return 1
fi

createbus $1 $2 $3