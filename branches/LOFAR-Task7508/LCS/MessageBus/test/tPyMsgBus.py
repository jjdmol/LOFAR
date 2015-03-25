#!/usr/bin/python
from lofar.messagebus.msgbus import FromBus, ToBus
from lofar.messagebus.message import Message, MessageContent

# Send a message (send MessageContent)
tbus = ToBus("test")
tmsg = MessageContent()
tmsg.payload = "foo"
tbus.send(tmsg)

# Receive it
fbus = FromBus("test")
fmsg = fbus.get(1)

# Verify the content
assert fmsg != None
assert fmsg.content().payload == "foo"

# Resend it! (send Message)
tbus.send(fmsg)
rmsg = fbus.get(1)

# Verify the content
assert rmsg != None
assert rmsg.content().payload == "foo"
