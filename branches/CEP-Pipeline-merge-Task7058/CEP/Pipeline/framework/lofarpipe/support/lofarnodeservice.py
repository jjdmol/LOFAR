#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                         Node recipe service
#                                                         Wouter Klijn
#                                                      klijn@astron.nl
# ------------------------------------------------------------------------------


#!/usr/bin/python
from lofar.messagebus.msgbus import FromBus, ToBus
from lofar.messagebus.message import Message

# Send a message
tbus = ToBus("test")
tmsg = Message()
tmsg.payload = "foo"
tbus.send(tmsg)

# Receive it
fbus = FromBus("test")
fmsg = fbus.get(1)
assert fmsg != None

# Verify the content
assert fmsg.payload == "foo"
