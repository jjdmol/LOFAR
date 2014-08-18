#!/usr/bin/python
from LAPS.MsgBus.Bus import Bus


print 'test'

msgbus = Bus()

parset = """
key=value
"""

msgbus.send(parset,"Observation123456")