#!/usr/bin/python
import sys
import laps.MsgBus

if (len(sys.argv) > 2):
	broker=sys.argv[1]
	queuename=sys.argv[2]
	
else:
    print "usage:"
    print "purgequeue.sh broker queuename"
    exit(-1)


num_processed = -1

t = laps.MsgBus.Bus(broker, queuename)
msg="bla"
while ( msg != "None") :
  num_processed += 1
  msg, subject = t.get(0.5)
  #print " received : %s\n "  %( msg )
  t.ack()

print "Total messages processed %d " %( num_processed )
