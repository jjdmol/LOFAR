function create_named_numbered_node_list {
    if (( $# == 4 ))
    then
        return_value=$1
        node_root=$2
        index_start=$3
        index_end=$4
        #
        #           Use space seperate the instances
        list=`printf ' locus%03d' {98..99}`      
        # For bash higher then 4.0 use: locus{001..094}
        echo $list
    else
        echo "Usage $FUNCNAME node_name_root index_start index_end (inclusive)"
        exit 1
    fi
}


return_value2=`create_named_numbered_node_list return_value2 $1 $2 $3`