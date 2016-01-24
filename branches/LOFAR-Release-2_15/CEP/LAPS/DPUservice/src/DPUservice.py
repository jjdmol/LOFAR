#!/usr/bin/python

import laps.MsgBus


incoming = laps.MsgBus.Bus("laps.DPUservice.incoming")
outgoing = laps.MsgBus.Bus("laps.MetaInfoservice.incoming")

while True:
    msg, subject = incoming.get()

    outgoing.send(msg,subject)
    incoming.ack()

