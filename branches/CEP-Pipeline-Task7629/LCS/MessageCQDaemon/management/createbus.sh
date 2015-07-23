#!/bin/bash 

function usage {
    echo "'createbus' creates or deletes a dynamic routing bus between two nodes." 
    echo "   createbus [add/del] <busname> <node1> <node2> [ignore]" 
    echo ""
    echo "If only one hostname is given the second hostname is presumed to be the local node." 
    echo "to ensure one-time delivery of messages avoid loops in the bus topology." 
    echo "If ignore is provided errors will be ignored, this could be used to clean partly created"
    echo "bus structures/setups"
	echo " if <node1> == <node2> only the deadletter and bus structure will be created, no links"
}

function deadletter_topic {
    local action=$1
    local hostname=$2
    local busname=$3    
    local option=""
    local type=""
    
    if [ "$action" == "add" ]; then
        option="--durable"
        type="topic"
    fi
    
    echo "qpid-config -b $hostname $action exchange $type  $busname\".proxy.deadletter\" $option" 
    qpid-config -b $hostname $action  exchange  $type  $busname".proxy.deadletter" $option
}

function deadletter_alternate_exchange {
    local action=$1
    local hostname=$2
    local busname=$3
    
    local option=""
    local type=""
    if [ "$action" == "add" ]; then
        # TODO: programmically creation of this command failed.
        echo "qpid-config -b $hostname add exchange direct $busname --durable --alternate-exchange=$busname\".proxy.deadletter\""
        qpid-config -b $hostname add exchange direct $busname --durable --alternate-exchange=$busname".proxy.deadletter" 
    else
        echo "qpid-config -b $hostname exchange $busname"
        qpid-config -b $hostname del exchange $busname
    fi
    
}

function deadletter_queue {
    local action=$1
    local hostname=$2
    local busname=$3
    
    local option=""
    if [ "$action" == "add" ]; then
        option="--durable"
    fi
            
    echo "qpid-config -b $hostname $action queue $busname\".deadletter\" $option" 
    qpid-config -b $hostname $action queue $busname".deadletter" $option
}

function deadletter_topic_to_queue {
    local action=$1
    local hostname=$2
    local busname=$3
    
    local option=""
    if [ "$action" == "bind" ]; then
        option="--durable"
    fi
        
    echo "qpid-config -b $hostname $action $busname\".proxy.deadletter\" $busname\".deadletter\" '*' $option" 
    qpid-config -b $hostname $action $busname".proxy.deadletter" $busname".deadletter" '*' $option 
}

# Depending on the first argument adds or deletes a route between two nodes
function routing {
    local action=$1
    local hostname=$2
    local remotehost=$3
    local busname=$4  

    echo "qpid-route -d dynamic $action $remotehost $hostname $busname" 
    qpid-route -d dynamic $action $remotehost $hostname $busname 
}

# createbus attempts to create a full instantiated bus structure between two nodes
function addbus {   
    busname=$1
    remotehost=$2
    hostname=$3
        
    deadletter_topic add $hostname $busname 
    deadletter_alternate_exchange add $hostname $busname 
    deadletter_queue add $hostname $busname 
    deadletter_topic_to_queue bind $hostname $busname 

    if [ $remotehost != $hostname ]
    then
		deadletter_topic add $remotehost $busname 
	    deadletter_alternate_exchange add $remotehost $busname 
		deadletter_queue add $remotehost $busname 
	    deadletter_topic_to_queue bind $remotehost $busname 
		
        routing add $remotehost $hostname $busname
        routing add $hostname $remotehost $busname 
    fi
}

function delbus {
    busname=$1
    remotehost=$2
    hostname=$3
    
    deadletter_topic_to_queue unbind $hostname $busname 
    deadletter_queue del $hostname $busname 
    deadletter_alternate_exchange del $hostname $busname 
	deadletter_topic del $hostname $busname 

    if [ $remotehost != $hostname ]
    then        
		routing del $remotehost $hostname $busname
		routing del $hostname $remotehost $busname  
		
		deadletter_topic_to_queue unbind $remotehost $busname 
		deadletter_queue del $remotehost $busname
		deadletter_alternate_exchange del $remotehost $busname 
		deadletter_topic del $remotehost $busname 
    fi
}

# validate the number of arguments
if [ $# -le 3 ] || [ $# -ge 6 ]
then
    usage
    exit 1
fi

# We need the option to ignore errors: It can be messy
if [ $# == 5 ] && [ "$5" == "ignore" ]
then
    # Do not exit on ignore
    echo "ignoring errors"
else
    set -e   
fi  

# depending on the first argument call the del or add  bus function
if [ "$1" == "del" ]; then
    delbus $2 $3 $4
elif [ "$1" == "add" ]; then
    addbus $2 $3 $4
else
    usage
    exit 1
fi  

    
    
