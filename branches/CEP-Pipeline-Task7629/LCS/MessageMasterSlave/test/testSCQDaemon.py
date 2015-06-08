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
import logging

import lofar.messagebus.MCQDaemon as MCQDaemon
import CQDaemonTestFunctions as testFunctions


# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("testMCQDaemon")

def test_first_run_job_for_session_to_deadletter():
    """
    A msg with the command run_job should be forwarded to jobnode
    """
    job_node = 'locus102'
    daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
        testFunctions.prepare_test( MCQDaemon.MCQDaemon)

    slaveCommandQueueBusName = "testmcqdaemon" + "/" + job_node
    slaveCommandQueueBus = testFunctions.get_from_bus( 
            slaveCommandQueueBusName, "locus102")

    # Test1: Create a test job payuoad
    send_payload =  {'command':'run_job',
                     'node':job_node,
                     'job':{}}

    msg = testFunctions.create_test_msg(send_payload)
    commandQueueBus.send(msg)

    # start the daemon processing
    daemon._process_commands()
  

    # validate that a job is received on the slave queue
    # wait on the slave command queue
    msg_received = testFunctions.try_get_msg(slaveCommandQueueBus)

    # unpack received data
    received_payload = eval(msg_received.content().payload)

    # validate correct content
    if received_payload != send_payload:
        raise Exception("Send data not the same as received data")

    # Cleanup sut
    commandQueueBus.close()
    slaveCommandQueueBus.close()
    deadletterQueue.close()
    daemon.close()



if __name__ == "__main__":
    print "test_forwarding_of_job_msg_to_queue()"    
    test_forwarding_of_job_msg_to_queue()

