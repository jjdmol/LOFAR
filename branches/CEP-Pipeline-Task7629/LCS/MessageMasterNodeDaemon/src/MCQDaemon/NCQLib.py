import socket
import logging
import sys
import pwd
import os
import time

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

class QPIDLoggerHandler(logging.Handler):
    def __init__(self, logTopicName):
        """
        INit function connects to the QPID logging topic supplied
        """
        logging.Handler.__init__(self)
        #super(QPIDLoggerHandler, self).__init__()
        self._hostname = socket.gethostname()
        self._username = pwd.getpwuid(os.getuid()).pw_name
        broker =  "127.0.0.1" 
        self._logTopicName = logTopicName
        self._logTOpic = msgbus.ToBus(self._logTopicName, 
              options = "create:always, node: { type: topic, durable: False}",
              broker = broker)

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
           
    def _send_log_message(self, log_data, level='info'):
        """
        Send a logging msg  with log_data to the logTOpic at the level

        msg_details:
        {'level'=level, 'log_data':log_data}
        """
        
        msg = message.MessageContent(
                from_="{0}.{1}.NCQlib".format(
                        self._username, self._hostname),
                forUser="{0}.MSQDaemon".format(self._username),
                summary="NCQDaemon log message",
                protocol="CommandQUeueLogMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        msg.payload = {'level':   level,
                       'log_data':log_data}

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
        self._broker = "127.0.0.1" 
        self._hostname = socket.gethostname()
        self._username = pwd.getpwuid(os.getuid()).pw_name
        self._parameterQueueName = parameterQueueName
        self._logTopicName = logTopicName
        self._returnQueueName = returnQueueName
        
        self._resultQueue = msgbus.ToBus(self._returnQueueName, 
              options = "create:always, node: { type: queue, durable: False}",
              broker = self._broker)

        self._parameterQueue = msgbus.FromBus(self._parameterQueueName, 
              options = "create:always, delete:always, node: { type: queue, durable: False}",
              broker = self._broker)

        self.QPIDLoggerHandler = QPIDLoggerHandler(self._logTopicName)

        self._job_dict = None
        self.environment = None


    def getArguments(self):
        """
        Retrieves the arguments for the current run from the command queue and
        returns them as a new dict.
        """
        msg = None
        
        # wait for the parameters: do this for max 10 seconds
        wait_counter = 0
        while True:
            if wait_counter == 10:
                raise Exception("Did not receive any arguments from the Daemon")
            msg = self._parameterQueue.get(0.1)
            if msg != None:
                
                wait_counter += 1
                time.sleep(1)
                break

        self._job_dict =  eval(msg.content().payload)
        self._parameterQueue.ack(msg)
        self.environment = self._job_dict['parameters']['environment']


        return self._job_dict['parameters']['job_parameters']

    def send_results(self, output):
        """
        Send the received outputs to the results queue
        """

        msg = message.MessageContent(
                from_="{0}.{1}.NCQlib".format(
                        self._username, self._hostname),
                forUser="{0}.MSQDaemon".format(self._username),
                summary="NCQDaemon results message",
                protocol="CommandQUeueLogMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        # Create the output dict
        msg_dict = {'type': 'output',
                    'output': output}

        # add the job specific information
        msg_dict.update(self._job_dict)

        msg.payload = msg_dict
        self._resultQueue.send(msg)

