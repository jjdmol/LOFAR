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
    echo "'createbusfromrange' creates or deletes a dynamic routing bus between a range of named nodes."
    echo " It uses the createbus.sh to perform the actual creation of the bus" 
    echo ""
    echo "   createbusfromrange <add|del> <busname> <hubnode> <rootname> <range min> <range max>[ignore]" 
    echo " eg:"
    echo "  createbusfromrange add testbus lhn001 locus 1 94  ignore"
    echo ""
    echo "If only one hostname is given the second hostname is presumed to be the local node." 
    echo "to ensure one-time delivery of messages avoid loops in the bus topology." 
    echo "If ignore is provided errors will be ignored, this could be used to clean partly created"
    echo "bus structures/setups"
}

if (( $# < 7  ))
# validate the number of arguments
then
    usage
    exit 1
fi

node_list=`create_named_numbered_node_list $4 $5 $6`

my_dir="$(dirname "$0")"

"$my_dir/createbus.sh" $1 $2 $3 $node_list $7


