#!/bin/bash 

# Script for starting and stopping a set of brokers

function create_named_numbered_node_list {
    if (( $# == 3 ))
    then
        node_root=$1
        index_start=$2
        index_end=$3
        #
        #           Use space seperate the instances
        for i in `seq $index_start $index_end`
        do
          printf " %s%03d" "$node_root" "$i"
        done
    else
        echo "Usage $FUNCNAME <node_name_root> <index_start> <index_end> (ranges are inclusive)"
        exit 1
    fi
}

function usage {
    echo "'startBrokers.sh' starts and stops a set of brokers" 
    echo "   startBrokers.sh <start|stop> <data-dir> <port> <federation_tag_postfix> <node_name_root> <index_start> <index_end> (ranges are inclusive)"
    echo "   startBrokers.sh <start|stop> <data-dir> <port> <federation_tag_postfix> <node_name>"
    echo ""
    echo " Start or stop a (set of) broker(s) with persistent storage in <data-dir>"
    echo " It expects the data-dir to be already created"
    echo " Script can only be used if password less access to the target hosts is available as the user qpid"
    echo " The default qpid port is 5672 and should NOT be used lightly for it may break production"
}

function start_broker {
    data_dir=$1
    port=$2
    federation_tag_postfix=$3
    node_name=$4

    ssh -tt qpid@$node_name "/bin/bash -l -c ' qpidd -d  --auth no --log-enable info+ -p '"$port"' --federation-tag \`hostname --fqdn\`'"$federation_tag_postfix"' --data-dir $data_dir --log-to-file '"$data_dir"'/qpid.log'"
}

function stop_broker {
    port=$1
    node_name=$2
    ssh -tt qpid@$node_name "/bin/bash -l -c ' qpidd --quit -p '"$port"' '"
}

if (( $# <  5 ))
# validate the number of arguments
then
    usage
    exit 1
fi

operation=$1
data_dir=$2
port=$3
federation_tag_postfix=$4

if (( $# == 5  ))
then
    node_name=$5
    if [ "$operation" == "start" ]
    then   
        start_broker  $data_dir $port $federation_tag_postfix $node_name    
    elif [ "$operation" == "stop" ]
    then
        echo "stop_broker $port $node"
        stop_broker $port $node_name   
    else
        echo "place select start or stop"
        usage
    fi
    
elif (( $# == 7  ))
then
    node_name_root=$5
    index_start=$6
    index_end=$7
    
    node_string=$(create_named_numbered_node_list $node_name_root $index_start $index_end)
    for node in $node_string
    do
        echo "starting on node $node"
        if [ "$operation" == "start" ]
        then   
            echo "start_broker  $data_dir $port $federation_tag_postfix $node"
            start_broker  $data_dir $port $federation_tag_postfix $node    
        elif [ "$operation" == "stop" ]
        then
            echo "stop_broker $port $node"
            stop_broker $port $node   
        else
            echo "place select start or stop"
            usage
        fi
    done
else
    usage    
fi

