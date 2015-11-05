#!/usr/bin/python
import sys
import LAPS.MsgBus

if (len(sys.argv) > 1):
	queuename=sys.argv[1]
else:
	queuename="testqueue"

num_processed = -1

t = LAPS.MsgBus.Bus(queuename)


msg="Purging"
while ( msg != "None") :
  num_processed += 1
  msg, subject = t.get()
  #print " received : %s\n "  %( msg )
  t.ack()

print "Total messages processed %d " %( num_processed )
