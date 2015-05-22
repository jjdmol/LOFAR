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
import lofar.messagebus.CQConfig as CQConfig


class QPIDLoggerHandler(logging.Handler):
    def __init__(self, logTopicName):
        """
        INit function connects to the QPID logging topic supplied
        """
        logging.Handler.__init__(self)
        self._logTopicName = logTopicName
        self._logTOpic = msgbus.ToBus(self._logTopicName, 
              options = "create:always, node: { type: topic, durable: False}",
              broker = CQConfig.broker)

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
        msg = CQConfig.create_validated_log_msg(level, log_data,
                                                CQConfig.hostname )
        self._logTOpic.send(msg)

class NCQLib(object):
    """
    class combining objects and function needed for communication via QPID,
    using the NCQDaemon framework.
    """
    def __init__(self, returnQueueName, logTopicName,
                      parameterQueueName):
        """
        Init function, connects to the logtopic and resultQueue.

        Registers the log handler and performs the integration with the 
        framework. Exit state is retrieved by the NCQDaemon, using the exit 
        value of the script
        """
        self._parameterQueueName = parameterQueueName
        self._logTopicName = logTopicName
        self._returnQueueName = returnQueueName
        
        self._resultQueue = msgbus.ToBus(self._returnQueueName, 
              options = "create:always, node: { type: queue, durable: False}",
              broker = CQConfig.broker)

        self._parameterQueue = msgbus.FromBus(self._parameterQueueName, 
              options = "create:always, delete:always, node: { type: queue, durable: False}",
              broker = CQConfig.broker)

        self.QPIDLoggerHandler = QPIDLoggerHandler(self._logTopicName)

        self._job_dict = None
        self.environment = None


    def getArguments(self):
        """
        Retrieves the arguments for the current run from the command queue and
        returns them as a new dict.
        """
        # wait for the parameters: do this for max 10 seconds
        wait_counter = 0
        while True:
            if wait_counter == 10:
                raise Exception(
                      "Did not receive any arguments from the Daemon")

            msg = self._parameterQueue.get(0.1)
            if msg != None:                
                wait_counter += 1
                time.sleep(1)
                break

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
        msg_dict = {'type': 'output',
                    'output': output}       
        msg_dict.update(self._job_dict) # add the _job_dict contains uuid etc.

        msg = CQConfig.create_validated_output_msg(msg_dict, CQConfig.hostname)
        msg.payload = msg_dict
        self._resultQueue.send(msg)
