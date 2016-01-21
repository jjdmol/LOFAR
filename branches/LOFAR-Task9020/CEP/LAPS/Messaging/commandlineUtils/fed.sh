#!/bin/bash
# fed:
#      (2014) Jan Rinze Peterzon
#
# This sets up a queue on two nodes
# and subsequently will add a forwarding route
# between the two queues

# Usage:
#
#    fed <source node> <destination node> <queue name>
#
#
# Example:
#    fed Groningen Singapore Inbox.Toni
#    fed Singapore Groningen Inbox.JR
#
#  At Groningen it is now possible to send a 
#  message to InboxToni and the message will
#  automatically be forwarded to Singapore.
#
#  JR@Groningen $> sendmsg -a Inbox.Toni -m "Hi Toni!"
#  
#  Toni@Singapore $> receivemsg -a Inbox.Toni
#  Received message: Hi Toni!
#
#  Toni@Singapore $> sendmsg -a Inbox.JR -m "Thanks JR!"
#
#  JR@Groningen $> receivemsg -a Inbox.JR
#  Received message: Thanks JR!
#
argc=$#
echo "num, args: " $argc
myname=$( echo $0 |sed 's|.*/||g' )
if [ "$argc" -lt 3 ]; then
	echo $myname " can be used to setup a forwarded queue."
	echo "Usage: " $myname " <QueueName> <SourceNode> <DestinationNode>"
	exit -1
fi

tmpfile=$( mktemp )
haserror=0

echo "setup queue " $1 " at " $2
now=$( date )
echo -n "$now : qpid-config -b $2 add queue $1 :" >>"$tmpfile"
qpid-config -b $2 add queue $1 2>>"$tmpfile" >>"$tmpfile"
if [ "$?" -gt 0 ]; then
	echo "failed to create queue " $1 " on " $2
	haserror=1
else
	echo "OK" >> "$tmpfile"
fi

echo "setup queue " $1 " at " $3
echo -n "$now qpid-config -b $3 add queue $1 :" >>"$tmpfile"
qpid-config -b $3 add queue $1 2>>"$tmpfile" >>"$tmpfile"
if [ "$?" -gt 0 ]; then
        echo "failed to create queue " $1 " on " $2
        haserror=1
else
        echo "OK" >> "$tmpfile"
fi
echo "setup forward route for queue " $1 " from " $2  " to " $3
echo -n "$now qpid-route queue add $3 $2 '' $1 :" >>"$tmpfile"
qpid-route queue add $3 $2 '' $1 2>>"$tmpfile" >>"$tmpfile"
if [ "$?" -gt 0 ]; then
        echo "failed to create forward route for queue " $1 " from " $2 " to " $3
        haserror=1
else
        echo "OK" >> "$tmpfile"
fi

if [ "$haserror" -gt 0 ]; then
     echo "log is in "$tmpfile
else
     rm $tmpfile
fi


#ping -c 1 $1 2>/dev/null >/dev/null
#result=$?
#if [ "$result" -lt 1 ]; then
#	echo Host $1 is alive
#else
#	echo Host $1 is not reachable
#fi
#
