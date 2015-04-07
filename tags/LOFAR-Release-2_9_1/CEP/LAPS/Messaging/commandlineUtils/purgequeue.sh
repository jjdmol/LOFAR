#!/usr/bin/python
import sys
import laps.MsgBus

if (len(sys.argv) > 1):
	queuename=sys.argv[1]
else:
	queuename="testqueue"

num_processed = -1

t = laps.MsgBus.Bus(queuename)
msg="bla"
while ( msg != "None") :
  num_processed += 1
  msg, subject = t.get(0.5)
  #print " received : %s\n "  %( msg )
  t.ack()

print "Total messages processed %d " %( num_processed )
