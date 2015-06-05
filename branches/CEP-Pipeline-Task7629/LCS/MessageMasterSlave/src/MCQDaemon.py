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
# $Id$

from datetime import datetime   # needed for duration
import time

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
import lofar.messagebus.CQDaemon as CQDaemon

# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

"""
Master command queue. Extends the command queue interface with the 
start job message and functionality
1. A single commands is support 
   (b) msg with content {'command':'run_job',
                       'node':job_node,
                       'job':{}} 
  will result in a msg being send to the command queue for job_node with the 
  copied content.

It add a bus to object where to send the job msg to
And a run_job member function performing the actual posting.

The default deadletter queue functionality is currently used (print and ignore)
"""
class MCQDaemon(CQDaemon.CQDaemon):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval=10, daemon=True):
        super(MCQDaemon, self).__init__(broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval, daemon)

        # Connect to bus ( used for sending job msg to slaves)
        self._toSlaveBus = msgbus.ToBus(self._busname, broker = self._broker)


    def close(self):
        """
        close all  connections owned by subclass
        """
        self._toSlaveBus.close()       # First our own bus
        super(MCQDaemon, self).close() # then the superclass


    def process_commands(self, command, unpacked_msg_content, msg):
        """
        Process_commands, add the run_job command
        """
        # Default behaviour:
        if command == 'run_job':
            self._process_run_job(unpacked_msg_content)
            return True

        return False
  
    def _process_run_job(self, unpacked_msg_content):
        """
        The starting of a job on one of the node servers.
        """
        node = unpacked_msg_content['node']        
        # create new msg
        # TODO: FOrwarding of the received msg instead of creating a new one.
        msg = message.MessageContent()
        # set content
        msg.payload = unpacked_msg_content
        # set subject needed for dynamic routing
        msg.set_subject(node)

        # send to bus using the slave as msg name allows for dynamic routing
        self._toSlaveBus.send(msg)
