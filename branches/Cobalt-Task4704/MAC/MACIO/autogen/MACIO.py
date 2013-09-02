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
from Numeric import *

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
# Some handy routines
#
def typename(x):
  'Returns the name of the type of x'
  try:
    return x.__name__
  except:
    return str(type(x)).split(' ')[1].split("'")[1] 

def isBasicType(x):
  'Returns true of x is a basic type'
  try:
    if x.__name__:
      return True
  except:
    return False

# array support
def isCdefArray(Ctype):
  "Returns true is the given C-type is an array."
  return Ctype.endswith("]") and Ctype.find("[")>0

def CdefArrayType(Ctype):
  "Returns the basic type of the array."
  if not(isCdefArray(Ctype)):
    return Ctype
  return Ctype[0:Ctype.find("[")]

def CdefArraySize(Ctype):
  "Returns the size of the C array. Returns 0 for undefined lengths"
  "Does NOT yet work for multi-dimensional arrays"
  if not(isCdefArray(Ctype)):
    return None
  if Ctype.find("[]")>0:
    return 0
  return int(Ctype[Ctype.find("[")+1:-1])

# vector support
def isCdefVector(Ctype):
  "Returns true is the given C-type is an vector."
  return Ctype.startswith("vector") and Ctype.endswith(">") and Ctype.find("<")>0

def CdefVectorType(Ctype):
  "Returns the basic type of the vector."
  if not(isCdefVector(Ctype)):
    return Ctype
  return Ctype[Ctype.find("<")+1:-1]

# map support
def isCdefMap(Ctype):
  "Returns true is the given C-type is a map."
  return Ctype.startswith("map") and Ctype.endswith(">") and Ctype.find("<")>0

def CdefMapType(Ctype):
  "Returns the basic types of the map."
  if not(isCdefMap(Ctype)):
    return Ctype
  return Ctype[Ctype.find("<")+1:-1]

#
# Packing and unpacking strings
#
def packString(value, fixedlen=0):
    "Pack a string in a MAC-like way"
    if fixedlen > 0:
      vLen = fixedlen
    else:
      vLen = len(value)
    format = "=i%ds" % vLen
    buffer = struct.pack(format, vLen, value)
    return buffer

def unpackString(buffer):
    "Unpack a string from the buffer"
    vLen= struct.unpack("=i", buffer[0:4])[0]
    format = "=%ds" % vLen
    vLen += 4
    value = struct.unpack(format, buffer[4:vLen])[0]
    return (value, vLen)

def packArray(value, fixedlen=0):
    "Pack an array in a MAC-like way"
    if fixedlen > 0:
      vLen = fixedlen
    else:
      vLen = len(value)
    format = "=%ds" % vLen
    buffer = struct.pack(format, value)
    return buffer

def unpackArray(buffer, itemlen, count):
    "Unpack an array from the buffer"
    vLen = itemlen * count
    format = "=%ds" % vLen
    value = struct.unpack(format, buffer[0:vLen])[0]
    return value

def packVector(value, typestr):
    "Pack a vector of 'something' in a MAC-like way"
    print "packVector:", value , ":", typestr
    items = len(value)
    buffer = struct.pack("=i", items)
    for _elem in value:
      buffer += packCdefinedVariable(_elem, typestr)
    return buffer

def unpackVector(buffer, typestr):
    "Unpack a vector of 'something'"
    items = struct.unpack("=i", buffer[0:4])[0]
    print "unpackVector:", items, " ", typestr, ":", " ".join(x.encode('hex') for x in buffer)
    list = []
    offset = struct.calcsize("=i")
    elemSize = packSize(typestr)
    for _elem in range(items):
      if typestr == "string":
        (_value, elemSize)=unpackString(buffer[offset:])
        list.append(_value)
      else:
        list.append(unpackCdefinedVariable(buffer[offset:offset+elemSize], typestr, elemSize))
      offset += elemSize
    return  (list, offset)

def packMap(value, typestr):
    "Pack a map of 'something' in a MAC-like way"
    print "packMap:", value , ":", typestr
    items     = len(value)
    buffer    = struct.pack("=i", items)
    keyType   = typestr.split(",")[0]
    valueType = typestr.split(",")[1]
    for _elem in value.items():
      buffer += packCdefinedVariable(_elem[0], keyType)
      buffer += packCdefinedVariable(_elem[1], valueType)
    return buffer

def unpackMap(buffer, typestr):
    "Unpack a map of 'something'"
    items = struct.unpack("=i", buffer[0:4])[0]
    offset = struct.calcsize("=i")
    print "unpackMap:", items, " ", typestr, ":", " ".join(x.encode('hex') for x in buffer)
    dict = {}
    keyType   = typestr.split(",")[0]
    valueType = typestr.split(",")[1]
    keySize   = packSize(keyType)
    valueSize = packSize(valueType)
    for _elem in range(items):
      if keyType == "string":
        (_keyValue, keySize)=unpackString(buffer[offset:])
      else:
        _keyValue = unpackCdefinedVariable(buffer[offset:offset+keySize], keyType, keySize)
      offset += keySize
      if valueType == "string":
        (_value, valueSize)=unpackString(buffer[offset:])
      else:
        _value = unpackCdefinedVariable(buffer[offset:offset+valueSize], valueType, valueSize)
      offset += valueSize
      dict[_keyValue]=_value
    return  (dict, offset)

def recvEvent(tcpsocket):
    "Wait for a message to receive on the given socket"
    buffer = tcpsocket.recv(GCFEvent.sizePackedGCFEvent)
    (signal, length) = struct.unpack('=HI', buffer)
    if length > 0:
      buffer += tcpsocket.recv(length)
    return buffer

#
# Toplevel marshalling
#
global gMarshallingFormatTable
gMarshallingFormatTable = {
      "bool"    : "=b",
      "char"    : "=c",
      "double"  : "=d",
      "float"   : "=f",
      "int16"   : "=h",
      "int32"   : "=i",
      "int64"   : "=q",
      "uint16"  : "=H",
      "uint32"  : "=I",
      "uint64"  : "=Q", 
      "uint8"   : "=B", }

def packSize(typestr):
    "Return the size of fixed size variables or 0"
    if gMarshallingFormatTable.has_key(typestr):
      return struct.calcsize(gMarshallingFormatTable[typestr])
    return 0

def packCdefinedVariable(value, typestr):
    "Pack the variable according to its type"
    if isCdefArray(typestr):
      if typestr.startswith("char["):
        return packArray(value, CdefArraySize(typestr))
      print "VALUE:", value
      return packArray(value.tostring())
    if isCdefVector(typestr):
      return packVector(value, CdefVectorType(typestr))
    if isCdefMap(typestr):
      return packMap(value, CdefMapType(typestr))
    if typestr == "string":
      return packString(value) 
    return struct.pack(gMarshallingFormatTable[typestr], value)

def unpackCdefinedVariable(buffer, typestr, varSize):
    "Unpack the variable according to its type"
    if isCdefVector(typestr):
      return unpackVector(buffer, CdefVectorType(typestr))
    if typestr == "string":
      return unpackString(buffer) 
    if gMarshallingFormatTable.has_key(typestr):
      return struct.unpack(gMarshallingFormatTable[typestr], buffer[:varSize])[0]
    return None

def pyArrayType(typestr):
    "Return the array type conform the c_type"
    pyArrayTypeTable = {
      "double"  : Float,
      "float"   : Float32,
      "int16"   : Int16,
      "int32"   : Int32 }
    basicType=CdefArrayType(typestr)
    if pyArrayTypeTable.has_key(basicType):
      return (pyArrayTypeTable[basicType])
    return None

def testValue(typestr, idx=-1):
    "Return a testvalue for generating tests"
    testValueTable = {
      "bool"    : True,
      "char"    : 'q',
      "double"  : 3.14152926535,
      "float"   : 2.7182818,
      "int16"   : -32100,
      "int32"   : -2000111222,
      "int64"   : -9222111000000000000,
      "uint16"  : 65432,
      "uint32"  : 4111222333,
      "uint64"  : 18444888000000000000,
      "uint8"   : 250}
    print "testValue:", typestr
    if testValueTable.has_key(typestr):
      if idx<0:
        return (testValueTable[typestr])
      else:
        return (testValueTable[typestr]+idx)
    if typestr == "string":
      if idx<0:
        return "This is a test string."
      else:
        return ("aap","noot","mies","wim","zus","jet","teun","vuur","gijs","does")[idx]
    if isCdefArray(typestr):
      arrSize = CdefArraySize(typestr)
      if (arrSize == 0):
        arrSize = 5
      print "arrsz:", arrSize
      if typestr.startswith("char["):
        return "%*s" % (arrSize, "Just another string for testing...")
      else:
        res = zeros(arrSize, pyArrayType(typestr))
        for x in range(arrSize):
          res[x] = testValue(CdefArrayType(typestr))+x
      return res
    if isCdefVector(typestr):
      baseType = CdefVectorType(typestr)
      lres = []
      for x in range(5):
        lres.append(testValue(baseType,x))
      return lres
    if isCdefMap(typestr):
      baseType = CdefMapType(typestr)
      keyType   = baseType.split(",")[0]
      valueType = baseType.split(",")[1]
      dres={}
      for x in range(5):
        keyVal   = testValue(keyType, x)
        valueVal = testValue(valueType, x+2)
        dres[keyVal] = valueVal
      return dres


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
