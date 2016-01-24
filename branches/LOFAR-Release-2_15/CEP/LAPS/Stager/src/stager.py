#!/usr/bin/python

import laps.MsgBus


incoming = laps.MsgBus.Bus("laps.staging.request")
outgoing = laps.MsgBus.Bus("laps.staging.staged")

while True:
    msg, subject = incoming.get()

    outgoing.send(msg,subject)
    incoming.ack()

