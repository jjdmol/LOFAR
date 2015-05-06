import uuid
import copy
import os
import logging
import time
import threading 
import pwd
import socket  # needed for username TODO: is misschien een betere manier os.environ['USER']
import signal

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
from qmf.console import Session as QMFSession



# Handler for stopping the logTopicHandler and signalling the master that the
# session has ended
logTopicStopFlag = threading.Event()
resultQueueStopFlag = threading.Event()
stopFunction = None

def treadStopHandler(signum, stack):
    # stop the threads
    logTopicStopFlag.set()
    resultQueueStopFlag.set()
    # now signal the master
    if not stopFunction is None:
      stopFunction()

    exit(signum)

    #print "***************************************"
    # code snipped to display registered signal handlers
    #signals_to_names = {}
    #for n in dir(signal):
    #    if n.startswith('SIG') and not n.startswith('SIG_'):
    #        signals_to_names[getattr(signal, n)] = n

    #for s, name in sorted(signals_to_names.items()):
    #    handler = signal.getsignal(s)
    #    if handler is signal.SIG_DFL:
    #        handler = 'SIG_DFL'
    #    elif handler is signal.SIG_IGN:
    #        handler = 'SIG_IGN'
    #    print '%-10s (%2d):' % (name, s), handler
    #print "***************************************"



class logTopicHandler(threading.Thread):
    """
    Class used for listening to a log topic

    usage:
    After initiation it must be started using the start() method
    setStopFlag() stops the forwarder
    """
    def __init__(self, logTopicName, logger=None,  poll_interval=1.0,):
        """
        Create the usage stat object. Create events for starting and stopping.
        By default the Process creating the object is tracked.
        Default polling interval is 10 seconds
        """
        threading.Thread.__init__(self)
        self.logger = logger
        self.stopFlag = logTopicStopFlag    # from 'global' scope
        self.poll_interval = poll_interval
        self.poll_counter = 0

        self._broker="127.0.0.1" 
        self._logTopicName = logTopicName
        self.logger.info(
              "Connecting to session specific logTopic: {0}".format(
                                                           self._logTopicName))
        self._logTopic = msgbus.FromBus(self._logTopicName, 
            options = "create:always, node: { type: topic, durable: True}",
            broker = self._broker)
        self.logger.info("COnnection with logTopic established")

    def _process_log_message(self, msg_content):
        """
        Parse log message and send it to the logger at the correct level
        """
        level = msg_content['level']
        log_data = msg_content['log_data'].strip()  #remove trailing whitespace
        if level == 'critical':
            self.logger.critical(log_data)
        elif level == 'error':
            self.logger.error(log_data)
        elif level == 'warning':
            self.logger.warning(log_data)
        elif level == 'info':
            self.logger.info(log_data)
        elif level == 'debug':
            self.logger.debug(log_data)
        else:
            self.logger.debug(log_data)

    def run(self):
        """
        Run function, the work horse of the handler

        While no stopflag is set:
        sleep for poll_interval
        """
        while not self.stopFlag.isSet():            
            # We do not want to listen in a tight loop. read the msg each 
            # pollinterval           
            if self.poll_counter < self.poll_interval:
                self.poll_counter += 0.1
                time.sleep(0.1)
                continue

            # reset the counter to zero, and restart the wait period
            self.poll_counter = 0       
            self.logger.info("Polling logTopic")
            # now empty the queue
            while True:
                # get is blocking, always use timeout.
                msg = self._logTopic.get(0.1)  
                if msg == None:
                    break   # break the loop

                # process the log data
                msg_content = eval(msg.content().payload)
                self._process_log_message(msg_content)
                self._logTopic.ack(msg)

 
    def setStopFlag(self):
        """
        Stop the monitor
        """
        self.stopFlag.set()

    def __del__(self):
        """
        """
        self._logTopic.close()

class resultQueueHandler(threading.Thread):
    """
    Class used for listening to a log topic

    usage:
    After initiation it must be started using the start() method
    setStopFlag() stops the forwarder
    """
    def __init__(self, resultQueueName, 
                 running_jobs , 
                 running_jobs_lock, 
                 logger=None, 
                 poll_interval=1.0):
        """
        Create the usage stat object. Create events for starting and stopping.
        By default the Process creating the object is tracked.
        Default polling interval is 10 seconds
        """
        threading.Thread.__init__(self)
        self.logger = logger
        self.stopFlag = resultQueueStopFlag    # from 'global' scope
        self.poll_interval = poll_interval
        self.poll_counter = 0

        self._broker="127.0.0.1" 
        self._resultQueueName = resultQueueName
        self.logger.info(
              "Connecting to session specific resultQueue: {0}".format(
                                         self._resultQueueName))
        self._resultQueue = msgbus.FromBus(self._resultQueueName, 
            options = "create:always, node: { type: queue, durable: False}",
            broker = self._broker)
        self.logger.info("COnnection with resultQueue established")

        # Unique for the result queue handler
        self._running_jobs = running_jobs
        self._running_jobs_lock = running_jobs_lock

    def _process_queue_message(self, msg_content):
        """
        Parse return messages 
        """
        # get the data from the msg
        type = msg_content['type']
        # TODO: For now assume only exit_value msg: the results should
        # be added also!!!!!!

        exit_value = msg_content['exit_value']
        uuid = msg_content['uuid']
        job_uuid = msg_content['job_uuid']

        with self._running_jobs_lock:
            self._running_jobs[job_uuid]['exit_value']=exit_value
            self._running_jobs[job_uuid]['completed']=True



    def run(self):
        """
        Run function, the work horse of the handler

        While no stopflag is set:
        sleep for poll_interval
        """
        while not self.stopFlag.isSet():

            # We do not want to listen in a tight loop. read the msg each 
            # pollinterval           
            if self.poll_counter < self.poll_interval:
                self.poll_counter += 0.1
                time.sleep(0.1)
                continue
            self.logger.info("Polling resultQueue: {0}".format(
                                                   self._resultQueueName))

            # reset the counter to zero, and restart the wait period
            self.poll_counter = 0       
            # now empty the queue
            while True:
                # get is blocking, always use timeout.
                msg = self._resultQueue.get(0.2)  
                if msg == None:
                    break   # break the loop

                # process the log data
                msg_content = eval(msg.content().payload)
                self._process_queue_message(msg_content)
                self._resultQueue.ack(msg)
 
    def setStopFlag(self):
        """
        Stop the monitor
        """
        self.stopFlag.set()

    def __del__(self):
        """
        """
        self._resultQueue.close()


class MCQLib(object):
    """
    Interface lib connecting to a local MCQDaemon command queue.
    Hides all queue interactions behind functions

    1. Connect to the command queue

    2. Init session
      a. Establish connection with the headnode CommandQueue
      a. Send command to create temp queues
      b. Connect to queues 

    """
    def __init__(self, logger):
        # Each MCQDaemonLib triggers a session with a uuid, generate and store
        # as a hex
        self.logger = logger

        # It is the dict that 'owns' the session information, the container
        # for state information
        self._running_jobs = {}
        self._running_jobs_lock = threading.Lock()
        # some static information for this session
        self._hostname = socket.gethostname()
        self._username = pwd.getpwuid(os.getuid()).pw_name       
        self._sessionUUID = uuid.uuid4().hex

        # Create the queue names based on the static information. 
        # TODO: This should be moved to a config file, shared between
        # the components of the MasterNodeCommandQueue Framework
        self._broker="127.0.0.1" 
        self._returnQueueTemplate = "MCQDaemon.{0}.return.{1}"
        self._logTopicTemplate = "MCQDaemon.{0}.log.{1}"
        
        self._returnQueueName = self._returnQueueTemplate.format(self._username,
                                                            self._sessionUUID)
        self._logTopicName = self._logTopicTemplate.format(self._username, 
                                                           self._sessionUUID)
        self._masterCommandQueueName = \
                "{0}.{1}.MCQueueDaemon.CommandQueue".format(self._username,
                                                            self._hostname) 

        # Place holders of the queues owned by the lib
        self._resultQueue = None
        self._logTopic    = None

        # Check state, raise exception if incorrect
        self._connect_to_master(self._masterCommandQueueName)

        # Send a start session command with the correct details to the
        # HCQDaemon
        self._start_session()

        # Now connect to the created topic and resultQ
        self._connect_to_queue_and_topic()

        # Start the log poller (in a seperate thread).
            # If we receive a SIGTERM, shut down processing.
        
        self._logTopicForwarder = logTopicHandler(self._logTopicName,
                   self.logger)

        self._resultQueueForwarder = resultQueueHandler(self._returnQueueName,
                   self._running_jobs , self._running_jobs_lock, self.logger)


        signal.signal(signal.SIGTERM, 
                      treadStopHandler)   # from 'global' scope
        signal.signal(signal.SIGINT, 
                      treadStopHandler)   

        self._logTopicForwarder.start()
        self._resultQueueForwarder.start()
        global stopFunction
        stopFunction = self._release

  
    def _connect_to_master(self, masterCommandQueueName):
        """
        Helper function for the __init__ member

        Checks if the connect with the commandqueue has been established
        Checks if there is a MAster daemon listening (by checking the number
        of consumers of the command queue.)

        Raise Exceptions on errors
        else returns with void
        """
        # Connect to the master queue
        self.logger.info("Connection to masterCommandQueue:{0}".format(
                                       masterCommandQueueName))
        self._masterCommandQueue = msgbus.ToBus(masterCommandQueueName, 
            options = "create:always, node: { type: queue, durable: True}",
            broker = self._broker)

        # Check for consumers: We need to know if the deamon is active before
        # we can send commands
        session=QMFSession()
        session.addBroker(self._broker) 
        queues = session.getObjects(_class="queue",
                                    _package="org.apache.qpid.broker")

        nr_consumers_of_CQ = None
        for queue_item in queues:
            if self._masterCommandQueueName in queue_item.name :
                nr_consumers_of_CQ = queue_item.consumerCount
                break # We have the info we need, break the loop over all queues

        if nr_consumers_of_CQ == None:
            raise Exception("Could not find command queue, is QPID enabled?")

        if nr_consumers_of_CQ == 0:
            raise Exception("No HeadNodeCommandQueueDaemon detected, aborting"
                            " A MasterDaemon should be active.")

        self.logger.info("Connection established and Master is active")


    def _start_session(self):
        """
        Send a register session command to the MCQDaemon
        """
        msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemonLib.{0}".format(
                                                            self._sessionUUID),
                forUser="USERRNAME.LOCUS102.MSQDaemon",
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        msg.payload = {"command":"start_session", "uuid":self._sessionUUID}
        self._masterCommandQueue.send(msg)

    def _connect_to_queue_and_topic(self):
        """ 
        Connect the topic and command queue that have been created by send
        session command to the daemon
        """
        self.logger.info("Connecting to returnQueue.")
        #self._resultQueue = msgbus.FromBus(self._returnQueueName, 
        #    options = "create:always, node: { type: queue, durable: True}",
        #    broker = self._broker)
        self.logger.info("Connecting to returnQueue and logTopic. Done")

    def __del__(self):
        #self._release()
        pass


    def _release(self):
        """
        delete the attached queue and topic
        Send to msg to the HCQDaemon command queue to remove the registered
        session uuid 
        """
        #
        self.logger.info(
                   "End of session. Sending stop_session command to master")
        
        # First disconnect from the queues
        self._logTopicForwarder.setStopFlag()
        self._resultQueueForwarder.setStopFlag()
        #self._resultQueue.close()        

        # create the header for the stop command
        msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemonLib.{0}".format(
                                                            self._sessionUUID),
                forUser="USERRNAME.LOCUS102.MSQDaemon",
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        # the content
        msg.payload = {"command":"stop_session", "uuid":self._sessionUUID}
        self._masterCommandQueue.send(msg)


    def run_job(self, parameters):
        """

        """
        #TODO: THIS APPENDING OF QUEUE NAMES SHOULD BE MOVED TO THE RUN
        # COMMAND IN  THE FRAMEWORK
        parameters['cmd'] = " ".join([parameters['cmd'], self._returnQueueName,
                            self._logTopicName])



        #/END TODO
        job_uuid = uuid.uuid4().hex
        msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemonLib.{0}".format(
                                                            self._sessionUUID),
                forUser="USERRNAME.LOCUS102.MSQDaemon",
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        msg.payload = {"command":"run_job", 
                       "uuid":self._sessionUUID,
                       'job_uuid':job_uuid,
                       "parameters":parameters}

        self.logger.info("Starting job: {0}".format(msg.payload))
        self._masterCommandQueue.send(msg)
        with self._running_jobs_lock:
            self._running_jobs[job_uuid] = {'payload':copy.deepcopy(msg.payload),
                                        'completed':False}

        # wait until the job returns then return, this needs a lock around the
        # running jobs object.
        poll_interval = 1  # check for results each second
        while True:
            time.sleep(poll_interval)

            with self._running_jobs_lock:
                if self._running_jobs[job_uuid]['completed']: 
                      self.logger.info("Exit value: {0}".format(
                                  self._running_jobs[job_uuid]['exit_value']))    
                      return self._running_jobs[job_uuid]['exit_value']