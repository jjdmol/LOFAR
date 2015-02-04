#!/usr/bin/python

import laps.MsgBus


incoming = laps.MsgBus.FromBus("laps.DPUservice.incoming")
outgoing = laps.MsgBus.ToBus("laps.MetaInfoservice.incoming")

while True:
    msg = incoming.getmsg()
    outgoing.sendmsg(msg)
    incoming.ack(msg)

