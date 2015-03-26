#!/usr/bin/python
#
# Test the basic functionality of FromBus and ToBus, both
# to send and to forward messages.
#
# Note that without QPID support, the classes are still usable,
# but messages won't arrive. We consider that case also in this test.
#

from lofar.messagebus.msgbus import FromBus, ToBus, enabled
from lofar.messagebus.message import Message, MessageContent

# Send a message (send MessageContent)
tbus = ToBus("test")
tmsg = MessageContent()
tmsg.payload = "foo"
tbus.send(tmsg)

# Receive it
fbus = FromBus("test")
fmsg = fbus.get(1)

# Verify the content (only if QPID support is enabled)
if enabled:
  assert fmsg != None
  assert fmsg.content().payload == "foo"

# Resend it! (send Message)
tbus.send(fmsg)
rmsg = fbus.get(1)

# Verify the content (only if QPID support is enabled)
if enabled:
  assert rmsg != None
  assert rmsg.content().payload == "foo"
