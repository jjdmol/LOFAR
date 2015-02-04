#!/usr/bin/python

import laps.MsgBus


incoming = laps.MsgBus.FromBus("laps.MetaInfoservice.incoming")
outgoing = laps.MsgBus.ToBus("laps.MetaInfoservice.output")

while True:
    msg = incoming.get()

    outgoing.sendmsg(msg)
    incoming.ack(msg)

