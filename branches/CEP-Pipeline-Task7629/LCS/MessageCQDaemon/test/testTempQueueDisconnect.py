#!usr/bin/python
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
# $Id$


import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message


def test_msg_to_temp_queue_disconnect_before_read():
    broker =  "locus102"
    busname = "testmcqdaemon"

    tempName = "tempQueueDisconnectTest"

    toBusBUs =  msgbus.ToBus(busname, broker = broker)


    tempBusName = busname + "/" + tempName
    tempBusFrom = msgbus.FromBus(tempBusName, broker = broker)

    msg = message.MessageContent(
                from_="test",
                forUser="MCQDaemon",
                summary="summary",
                protocol="protocol",
                protocolVersion="test", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
    msg.payload = "heart beat"

    msg.set_subject(tempName)

    toBusBUs.send(msg)

    tempBusFrom.close()

    toBusBUs.send(msg)





if __name__ == "__main__":
      test_msg_to_temp_queue_disconnect_before_read()