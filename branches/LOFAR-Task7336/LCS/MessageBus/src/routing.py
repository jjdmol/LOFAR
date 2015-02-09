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

import msgbus
import threading
from ConfigParser import SafeConfigParser

class BusMulticast(threading.Thread):
  """
  Sets up the routing from one inbus to several outbusses
  """
  def __init__(self, source, destlist):
    threading.Thread.__init__(self)
    self.source   = source
    self.destlist = destlist

  def run(self):
    inbus = msgbus.FromBus(self.source)
    outbusses = [msgbus.ToBus(dest) for dest in self.destlist]

    print "Listening at: ", self.source

    while True:
      # TODO: Use a transaction
      msg = inbus.getmsg()
      print "Msg received at: ", self.source

      for outbus in outbusses:
        outbus.send(msg)
      print "forwarded message to ", self.destlist

      inbus.ack(msg)

class RoutingConfig(SafeConfigParser):
  def __init__(self, filename):
    SafeConfigParser.__init__(self)

    # set defaults
    self.add_section('routing')

    # read configuration
    self.read(filename)

  def sources(self):
    return self.options('routing')

  def destinations(self, source):
    return [field.strip() for field in self.get('routing', source).split(',')]

if __name__ == "__main__":
  # read config file and process lines
  config = RoutingConfig('routing.conf')

  threadlist = []

  # set up routing
  for source in config.sources():
    destlist = config.destinations(source)

    for dest in destlist:
      print source, "->", dest

    t = BusMulticast(source, destlist)
    t.start()
    threadlist.append(t)

  # wait for join (forever)
  for t in threadlist:
    t.join()

