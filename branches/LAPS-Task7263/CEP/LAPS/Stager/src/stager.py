#!/usr/bin/python

import laps.MsgBus


incoming = laps.MsgBus.FromBus("laps.staging.request")
outgoing = laps.MsgBus.ToBus("laps.staging.staged")

while True:
    msg = incoming.getmsg()
    # msg.content is message body
    # msg.subject is message subject
    outgoing.sendmsg(msg)
    incoming.ack(msg)
