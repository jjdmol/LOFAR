#!/bin/bash 
#set -x
#
#  createbus.sh
#
#  createbus <op> <busname> [<nodename>...] [ignore]
#
#     if no nodenames are given then localhost is assumed.
#     <op> :  'add'  creates 
#
#
#




# inlined bustools since paths are unknow at this time.
#---------------------------------------------------------------------------------
#
# Bus Tools for managing QPID dynamic routing message bus
#

# History:
# 23/07/2015 Initial version by Jan Rinze Peterzon
#

#
# description:
# 
# createbus <busname> <hub> <spoke> [<spoke> ..]
#   create a bus topology with name busname over the hub and spokes
#
# deletebus <busname> <hub> <spoke> [<spoke> ..]
#   delete the bus topology with name busname over the hub and spokes
#
# addlocalbus <busname> [nodename]
#   create the bus components on localhost or on node [nodename]
#
# dellocalbus <busname> [nodename]
#   delete the bus components on localhost or on node [nodename]

#
# TODO:
#   - add proper error handling
#   - add option for uni-directional spoke connections
#   - produce logging for automated tools
#   - refactor 'add' vs 'create'


function addlocalbus {
   myname=`hostname`
   if (( $# >= 1 ))
   then
     busname=$1
     if (( $# > 1 ))
     then
        myname=$2
     fi
     # 
     echo " creating exchanges on "$myname" for bus "$busname
     qpid-config -b $myname add exchange topic $busname".deadletterproxy" --durable
     qpid-config -b $myname add exchange direct $busname --durable --alternate-exchange=$busname".deadletterproxy"
     echo " creating deadletters queue on "$myname
     qpid-config -b $myname add queue $busname".deadletter" --durable
     qpid-config -b $myname bind $busname".deadletterproxy" $busname".deadletter" '#' --durable
   else
     echo "Usage: $FUNCNAME <busname> [nodename]"
   fi
}
 
function dellocalbus {
   myname=`hostname`
   if (( $# >= 1 ))
   then
     busname=$1
     if (( $# > 1 ))
     then
        myname=$2
     fi
     # 
     echo " deleting bus "$busname" on "$myname
     qpid-config -b $myname del exchange $busname 
     qpid-config -b $myname del exchange $busname".deadletterproxy"
     echo " deleting deadletters queue on "$myname
     qpid-config -b $myname del queue $busname".deadletter" --force
   else
     echo "Usage: $FUNCNAME <busname> [nodename]"
   fi
}



function connectbus {
     if (( $# == 3 ))
     then
         busname=$1
         firstnode=$2
         secondnode=$3
         qpid-route dynamic add $firstnode $secondnode $busname --durable --ack=1
         qpid-route dynamic add $secondnode $firstnode $busname --durable --ack=1
     else
         echo "Usage $FUNCNAME <busname> <firstnode> <secondnode>"
     fi
}
function disconnectbus {
     if (( $# == 3 ))
     then
         busname=$1
         firstnode=$2
         secondnode=$3
         qpid-route dynamic del $firstnode $secondnode $busname 
         qpid-route dynamic del $secondnode $firstnode $busname 
     else
         echo "Usage $FUNCNAME <busname> <firstnode> <secondnode>"
     fi
}

function createbus {
  if (( "$#" >=3 ))
  then
    busname=$1
    hubname=$2
    addlocalbus $busname $hubname
    shift 2
    for i in "$@"
    do
      addlocalbus $busname $i
      connectbus $busname $hubname $i
    done
  else
    echo "usage: $FUNCNAME <busname> <hubnode> <spokenode> [<spokenode>..]"
  fi
}
 
function deletebus {
  if (( "$#" >=3 ))
  then
    busname=$1
    hubname=$2
    shift 2
    for i in "$@"
    do
      disconnectbus $busname $hubname $i
      dellocalbus $busname $i
    done
    dellocalbus $busname $hubname
  else
    echo "usage: $FUNCNAME <busname> <hubnode> <spokenode> [<spokenode>..]"
  fi
}

# ----------------------------------------------------------------------------------

function usage {
    echo "'createbus' creates or deletes a dynamic routing bus between two nodes." 
    echo "   createbus <add|del> <busname> <hubnode> <spkokenode> [<spokenode>..] [ignore]" 
	echo ""
    echo "If only one hostname is given the second hostname is presumed to be the local node." 
    echo "to ensure one-time delivery of messages avoid loops in the bus topology." 
	echo "If ignore is provided errors will be ignored, this could be used to clean partly created"
	echo "bus structures/setups"
}


# validate the number of arguments
if (( $# < 2  ))
then
    usage
    exit 1
fi


operation="$1"
busname="$2"

# skip both the name of the process and the required operation
shift 2

# make an array of the node names
declare -a nodenames
declare -i numnodes
numnodes=0

# We need the option to ignore errors: It can be messy
# check if "ignore" is mentioned on the command line
ignore="False"
for tmp in $@
do 
   if [ "$tmp" == "ignore" ]
   then
     ignore="True"
   else
     nodenames[$numnodes]="$tmp"
     numnodes=$((numnodes + 1))
   fi
done

if [ "$ignore" == "True" ]
then
	# Do not exit on ignore
	echo "ignoring errors"
else
   	set -e   
fi	

# allow for zero nodes and use hostname as nodename.
if (( numnodes == 0 )); then
  nodenames[$numnodes]="localhost"
  numnodes=$((numnodes + 1))
fi

# depending on the first argument call the del or add  bus function
if [ "$operation" == "del" ]; then
   if (( numnodes == 1 )); then
      echo "dellocalbus $busname ${nodenames[*]}"
      dellocalbus $busname ${nodenames[*]}
   else
      echo "deletebus $busname ${nodenames[*]}"	
      deletebus $busname ${nodenames[*]}
   fi
elif [ "$operation" == "add" ]; then
   if (( numnodes == 1 )); then
      echo "addlocalbus  $busname ${nodenames[*]}"
      addlocalbus  $busname ${nodenames[*]}
   else
      echo "createbus $busname  ${nodenames[*]}"
      createbus $busname  ${nodenames[*]}
   fi
else
    usage
    exit 1
fi	

