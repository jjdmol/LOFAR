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
import logging
import time
import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message


def create_msg(payload):
        """
        TODO: should be moved into a shared code lib
        Creates a minimal valid msg with payload
        """
        msg = message.MessageContent(
                    from_="test",
                    forUser="",
                    summary="summary",
                    protocol="protocol",
                    protocolVersion="test", 
                    #momid="",
                    #sasid="", 
                    #qpidMsg=None
                          )
        msg.payload = payload
        return msg

class QPIDLoggerHandler(logging.Handler):
    """
    TODO: Candidate to move to LCS
    """
    def __init__(self, broker, logTopicName):
        """
        INit function connects to the QPID logging topic supplied
        """
        logging.Handler.__init__(self)
        self._broker = broker
        self._logTopicName = logTopicName
        self._logTOpic = msgbus.ToBus(self._logTopicName, 
              broker = self._broker)

    def flush(self):
        """
        Not needed for this handler
        """
        pass

    def emit(self, record):
        """
        Called upon receiving a log record.
        """
        self._send_log_message(record.getMessage(),
                               record.levelname)
           
    def _send_log_message(self, log_data, level='INFO'):
        """
        Send a logging msg  with log_data to the logTOpic at the level

        msg_details:
        {'level'=level, 'log_data':log_data}
        """
        # add the data to send
        payload = {'level':   level,
                   'log_data':log_data,
                   'sender':self._broker }  #TODO USE CORRECT HOSTNAME

        msg = create_msg(payload)
        self._logTOpic.send(msg)

class SCQLib(object):
    """
    class combining objects and function needed for communication via QPID,
    using the NCQDaemon framework.
    """
    def __init__(self, broker, busname, session_uuid, job_uuid):
        """
        Init function, connects to the logtopic and resultQueue.

        Registers the log handler and performs the integration with the 
        framework. Exit state is retrieved by the NCQDaemon, using the exit 
        value of the script
        """
        self._session_uuid = session_uuid
        self._job_uuid = job_uuid

        self._parameterQueueName = busname + "/parameters_" + session_uuid + "_" + job_uuid

        self._logTopicName = busname + "/log_" + session_uuid
        self._returnQueueName = busname + "/result_" + session_uuid
        
        self._resultQueue = msgbus.ToBus(self._returnQueueName, 
              broker = broker)

        self._parameterQueue = msgbus.FromBus(self._parameterQueueName, 
              broker = broker)

        self.QPIDLoggerHandler = QPIDLoggerHandler(broker,
                                                   self._logTopicName)

        self._job_dict = None
        self.environment = None

    def getArguments(self):
        """
        Retrieves the arguments for the current run from the command queue and
        returns them as a new dict.
        """
        # wait for the parameters: do this for max 10 seconds
        wait_counter = 0
        msg = None

        while True:
            if wait_counter == 10:
                raise Exception(
                  "Did not receive any arguments from the Daemon: {0}".format(
                   self._parameterQueueName))

            msg = self._parameterQueue.get(0.1)
            if not msg is None:
                break
            
            wait_counter += 1
            time.sleep(1)
            

        self._job_dict =  eval(msg.content().payload)  # raises on parse error
        self._parameterQueue.ack(msg)
        self.environment = self._job_dict['parameters']['environment']

        return self._job_dict['parameters']['job_parameters']

    def send_results(self, output):
        """
        Send the outputs to the results queue
        include the job dict containing the uuid and job_uuid
        """
        # Create the output dict
        payload = {'type': 'output',
                    'output': output,
                    'job_msg': self._job_dict,
                    'job_uuid':self._job_uuid,
                    'session_uuid':self._session_uuid,
                    'node':self._job_dict['node']}


        msg = self.create_msg(payload)

        self._resultQueue.send(msg)

    def create_msg(self, payload):
        """
        TODO: should be moved into a shared code lib
        Creates a minimal valid msg with payload
        """
        msg = message.MessageContent(
                    from_="test",
                    forUser="",
                    summary="summary",
                    protocol="protocol",
                    protocolVersion="test", 
                    #momid="",
                    #sasid="", 
                    #qpidMsg=None
                          )
        msg.payload = payload
        return msg

def validParameterQueueName(queueName):
      """
      The parameter queue name is profided on the command line.
      This interface is also used by the old framework. In these instances 
      port numbers are used.

      We need a function to check if the queuename is correct. And validate that
      we are in a correct state. (you can have qpid enabled on a node) but
      state the recipe on the old socked manner.
      """
      # TODO: Implement
      return True


