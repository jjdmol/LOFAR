#!/usr/bin/env python
#coding: iso-8859-15
#
#  MACIO.py: Base classes for using MAC messages
#
#  Copyright (C) 2013
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#

#  This program is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU Lesser General Public License for more details.
#  
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#
#  $Id: protocol.tpl 23417 2012-12-20 14:06:29Z loose $
#
import struct

class GCFEvent(object):
  'Base class of all events'
  sizePackedGCFEvent = 6

  def __init__(self, signal=None):
    if signal != None:
      self.signal = signal
    else:
      self.signal = 0
    self.length = 0
    self.buffer = ""

  def __str__(self):
    return "signal=%04x,length=%d" % (self.signal, self.length)

  def pack(self):
    self.length = len(self.buffer)
    self.buffer = struct.pack('=HI', self.signal, self.length) + self.buffer

  def unpack(self, someBuffer=None):
    if someBuffer != None:
      self.buffer = someBuffer
      self.length = len(someBuffer)
    (self.signal, self.length) = struct.unpack('=HI', self.buffer[0:GCFEvent.sizePackedGCFEvent])
    return GCFEvent.sizePackedGCFEvent

#
# Packing and unpacking strings
#
def packString(value, fixedlen=0):
    "Pack a string in a MAC-like way"
    if fixedlen > 0:
      vLen = fixedlen
    else:
      vLen = len(value)
    format = "=H%ds" % vLen
    buffer = struct.pack(format, vLen, value)
    return buffer

def unpackString(buffer):
    "Unpack a string from the buffer"
    vLen= struct.unpack("=H", buffer[0:2])[0]
    format = "=%ds" % vLen
    vLen += 2
    value = struct.unpack(format, buffer[2:vLen])[0]
    return (value, vLen)

def packArray(value, fixedlen=0):
    "Pack an array in a MAC-like way"
    if fixedlen > 0:
      vLen = fixedlen
    else:
      vLen = len(value)
    format = "=%ds" % vLen
    print "packSize=", vLen
    buffer = struct.pack(format, value)
    return buffer

def unpackArray(buffer, itemlen, count):
    "Unpack an array from the buffer"
    vLen = itemlen * count
    print "unpackSize=", vLen
    format = "=%ds" % vLen
    value = struct.unpack(format, buffer[0:vLen])[0]
    return value

#
# Protocol knowledge
#
def F_SIGNAL(protocol, signal, inout):
    "Merge the three values into a unique ID"
    return (inout & 0x3) << 14 | (protocol & 0x3f) << 8 | (signal & 0xff)

def F_ERROR(protocol, errNr):
    "Create unique error number"
    return (protocol & 0x3f) * 100 + errNr

def F_ERR_PROTCOL(errID):
    "Resolve protocol-id from given errorID"
    return (errID / 100) & 0x3f

def F_ERR_NR(errID):
    "Resolve errornumber from given errorID"
    return errID % 100

F_IN = 0x01
F_OUT = 0x02
F_INOUT = F_IN | F_OUT

F_FSM_PROTOCOL = 1
F_PORT_PROTOCOL = 2
F_GCF_PROTOCOL = 3
F_APL_PROTOCOL = 10
