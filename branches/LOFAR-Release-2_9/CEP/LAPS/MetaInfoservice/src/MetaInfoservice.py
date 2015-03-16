#!/usr/bin/python

import laps.MsgBus


incoming = laps.MsgBus.Bus("laps.MetaInfoservice.incoming")
outgoing = laps.MsgBus.Bus("laps.MetaInfoservice.output")

while True:
    msg, subject = incoming.get()

    outgoing.send(msg,subject)
    incoming.ack()

