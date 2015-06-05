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

import lofar.messagebus.MCQDaemon as MCQDaemon

import lofar.
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


class PipelineMCQDaemon(MCQDaemon.MCQDaemon):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval=10, daemon=True):
        super(PipelineMCQDaemon, self).__init__(broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval, daemon)
        pass



if __name__ == "__main__":
    # TODO: Read these parameters from a config file
    # TODO: Daemon parameter in the init should be removed
    broker = "locus102"
    busname = "testmcqdaemon"
    masterCommandQueueName = busname + "/" + "pipelineMasterCommandQueue"
    deadLetterQueueName = busname + "/" + ".proxy.deadletter"


    daemon = PipelineMCQDaemon(broker, busname, masterCommandQueueName,
                               deadLetterQueueName, 1, True)

    daemon.run()


