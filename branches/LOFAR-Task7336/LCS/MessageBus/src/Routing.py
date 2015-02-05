#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# id.. TDB
"""Very basic messagebus router that routs the messages according to the settings in Routing.conf
"""

from MsgBus import *
import threading

class oneBusDistribute(threading.Thread):
  """
  Sets up the routing from one inbus to several outbusses
  """
  def __init__(self, source, destlist):
    threading.Thread.__init__(self)
    self.source   = source
    self.destlist = destlist

  def run(self):
    inbus = FromBus(self.source)
    outbusses = [ ToBus(dest) for dest in self.destlist]
    print "Listening at: ", self.source
    while 1:
      msg = inbus.getmsg()
      print "Msg received at: ", self.source
      for outbus in outbusses:
        outbus.send(msg)
      print "forwarded message to ", self.destlist
      inbus.ack(msg)

if __name__ == "__main__":
  # read config file and process lines
  infile =  open("Routing.conf", 'r')
  for line in infile:
    # skip comment
    hashpos = line.find('#')
    if hashpos >= 0:
      line = line[0:hashpos]
    if len(line) == 0:
      continue

    # split in source and destinations
    fields = [ field.strip() for field in line.split(":")]
    if len(fields) < 2 or len(fields[1].strip()) == 0:
      print "SYNTAX ERROR IN LINE: ", line
      continue
    source = fields[0].strip()
    destlist = [ field.strip() for field in fields[1].split(',')]

    # set up routing
    threadList = []
    for dest in destlist:
      print source, "->", dest.strip()
      t = oneBusDistribute(source , destlist)
      t.start()
      threadlist.append(t)

  infile.close()

  # wait for join (forver)
  for t in threadList:
    t.join()
    
