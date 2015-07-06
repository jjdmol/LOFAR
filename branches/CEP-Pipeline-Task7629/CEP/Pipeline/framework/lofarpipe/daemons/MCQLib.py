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
import uuid
import copy
import time
import threading 
import signal
import pickle

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message   # thread stop handlers break if
                                             # this line is removed.. TODO

# Handler for stopping the logTopicHandler and signalling the master that the
# session has ended
logTopicStopFlag = threading.Event()
resultQueueStopFlag = threading.Event()
workerThreadKillswitch = threading.Event()  # will be overwritten in 
                                            # MCQLib.set_killswitch
stopFunction = None

def treadStopHandler(signum, stack):
    # stop the threads
    logTopicStopFlag.set()
    resultQueueStopFlag.set()
    workerThreadKillswitch.set()
    # now signal the master
    if not stopFunction is None:
      stopFunction()

    exit(signum)

    #print "***************************************"
    ##code snipped to display registered signal handlers
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

    TODO: Candidate to move to LCS
    """
    def __init__(self, broker,logTopicName, logger=None,  poll_interval=1.0):
        """
        Create the usage stat object. Create events for starting and stopping.
        By default the Process creating the object is tracked.
        Default polling interval is 10 seconds
        """
        threading.Thread.__init__(self)

        self.daemon = True  # run as daemon: thread dies on owner death
        self._broker = broker
        self.logger = logger
        self.stopFlag = logTopicStopFlag    # from 'global' scope
        # Set it to working (allows re entrant usage of this class)
        self.stopFlag.clear()
        self._poll_interval = poll_interval       
        self._logTopicName = logTopicName

        self.logger.debug(
              "Connecting to logTopic: {0}".format(self._logTopicName))
        self._logTopic = msgbus.FromBus(self._logTopicName, 
            broker = self._broker)

    def _process_log_message(self, msg_content):
        """
        Parse log message and send it to the logger at the correct level
        """
        level = msg_content['level']       
        log_data = msg_content['log_data'].strip()  #remove trailing whitespace
        sender = msg_content['sender']

        formatted_line = sender + ": " + log_data
        if level == 'CRITICAL':
            self.logger.critical(formatted_line)
        elif level == 'ERROR':
            self.logger.error(formatted_line)
        elif level == 'WARNING':
            self.logger.warning(formatted_line)
        elif level == 'INFO':
            self.logger.info(formatted_line)
        elif level == 'DEBUG':
            self.logger.debug(formatted_line)
        else:
            self.logger.debug(formatted_line)

    def run(self):
        """
        Run function, the work horse of the handler

        While no stopflag is set:
        a. Listen for log lines, process and ack
        b. sleep for poll_interval
        """
        while not self.stopFlag.isSet():           
            try: 
              # reset the counter to zero, and restart the wait period
              self.logger.debug("Polling logTopic")

              # now empty the queue
              while True:
                  # get is blocking, always use timeout.
                  msg = self._logTopic.get(0.1)  
                  if msg == None:
                      break   # break the loop
                  
                  
                  # process the log data
                  #self.logger.error("******************************")
                  #self.logger.error(msg.content())
                  #self.logger.error("******************************")
                  msg_content = eval(msg.content().payload)
                  
                  self._process_log_message(msg_content)
                  self._logTopic.ack(msg.content())

            # Catch all exception!!!!!
            except Exception, ex:
                self.logger.warning("LogTopic handler received uncaught exception:")
                self.logger.warning(str(ex))
                self.logger.warning("Expected behaviour on manual abort")
                
            # sleep
            time.sleep(self._poll_interval)
 
    def setStopFlag(self):
        """
        Stop the monitor
        """
        self.stopFlag.set()
        self._logTopic.close()

    def __del__(self):
        """
        """
        pass
        #self._logTopic.close()

class resultQueueHandler(threading.Thread):
    """
    Class used for listening to a results queue

    It retrieves the job id from the msg and inserts the results msg details
    in running_jobs dict. Inserts are performed in a lock

    usage:
    After initiation it must be started using the start() method
    setStopFlag() stops the forwarder
    """
    def __init__(self,broker, resultQueueName, 
                 running_jobs, 
                 running_jobs_lock, 
                 logger=None, 
                 poll_interval=1.0):
        """
        Create the results handler. Create events for starting and stopping.
        By default the Process creating the object is tracked.
        Default polling interval is 10 seconds
        """
        threading.Thread.__init__(self)
        self.daemon = True  # Kill thread when the owned dies
        self._broker = broker
        self.logger = logger
        self.stopFlag = resultQueueStopFlag    # from 'global' scope
        # unset the stopflag, allows re entrant usage of the class
        self.stopFlag.clear()
        self.poll_interval = poll_interval

        self._resultQueueName = resultQueueName
        self.logger.debug(
              "Connecting to session specific resultQueue: {0}".format(
                                         self._resultQueueName))
        self._resultQueue = msgbus.FromBus(self._resultQueueName, 
            broker = self._broker)
        self.logger.debug("COnnection with resultQueue established")

        # Unique for the result queue handler
        self._running_jobs = running_jobs
        self._running_jobs_lock = running_jobs_lock

    def _process_queue_message(self, msg_content):
        """
        Parse return messages 
        """


        # get the data from the msg
        type = msg_content['type']
        self.logger.debug("Result for: {0}".format(msg_content['job_uuid']))
        if type == 'exit_value':
            exit_value = msg_content['exit_value']
            job_uuid = msg_content['job_uuid']

            with self._running_jobs_lock:
                self._running_jobs[job_uuid]['exit_value']=exit_value

                # if the exit value is invalid (different then 0)
                # do not expect any output
                self._running_jobs[job_uuid]['output']=[]


        elif type == 'output':
            output = msg_content['output']
            job_uuid = msg_content['job_uuid']

            with self._running_jobs_lock:
                self._running_jobs[job_uuid]['output']=output


    def run(self):
        """
        Run function, the work horse of the handler

        While no stopflag is set:
        sleep for poll_interval
        """
        while not self.stopFlag.isSet():
            try:
                self.logger.debug("Polling resultQueue: {0}".format(
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

            except Exception, ex:
                self.logger.warning("Results handler received uncaught exception:")
                self.logger.warning(str(ex))
                self.logger.warning("Expected behaviour on manual abort")

            #self.logger.error(self._running_jobs)
            time.sleep(self.poll_interval)
 
    def setStopFlag(self):
        """
        Stop the monitor
        """
        self.stopFlag.set()
        self._resultQueue.close()

    def __del__(self):
        """
        """
        pass



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
    def __init__(self, logger, broker, busname):
        # Each MCQDaemonLib triggers a session with a uuid, generate and store
        # as a hex
        self.logger = logger
        self._broker = broker
        self._busname = busname
        # It is the dict that 'owns' the session information, the container
        # for state information
        self._running_jobs = {}
        self._running_jobs_lock = threading.Lock()

        # some static information for this session
        self._sessionUUID = uuid.uuid4().hex
        self._returnQueueName = self._busname + "/result_" + self._sessionUUID
        self._logTopicName = self._busname +"/log_" + self._sessionUUID
        self._masterCommandQueueName = self._busname + "/pipelineMasterCommandQueue"

        # Place holders of the queues owned by the lib
        self._resultQueue = None
        self._logTopic    = None

        # Check state, raise exception if incorrect
        self._connect_to_master(self._masterCommandQueueName)

        # Send a start session command with the correct details to the
        # HCQDaemon
        #self._send_start_session_to_daemon()

        self._start_log_and_result_handlers()

    def _start_log_and_result_handlers(self):
        """
        Helper function used for collecting the starting of the handler
        threads on a single location
        """
        # Start the log poller (in a seperate thread).
        # If we receive a SIGTERM, shut down processing.       
        self._logTopicForwarder = logTopicHandler(self._broker, self._logTopicName,
                   self.logger)

        self._resultQueueForwarder = resultQueueHandler(
                   self._broker, self._returnQueueName,
                   self._running_jobs , self._running_jobs_lock, self.logger)

        # Register correct handlers for killing the threads
        signal.signal(signal.SIGTERM, 
                      treadStopHandler)   # from 'global' scope
        signal.signal(signal.SIGINT, 
                      treadStopHandler)   

        self._logTopicForwarder.daemon = True
        self._logTopicForwarder.start()
        self._resultQueueForwarder.daemon = True
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
            broker = self._broker)

        # Send and wait for acknoledge on a connection msg
        # Check for consumers: We need to know if the deamon is active before
        # we can send commands
        # TODO Implement
        self.logger.debug("Connection established and Master is active")


    def __del__(self):

        #self._release()
        logTopicStopFlag.set()
        resultQueueStopFlag.set()
        pass


    def _release(self):
        """
        delete the attached queue and topic
        Send to msg to the HCQDaemon command queue to remove the registered
        session uuid 
        """
        self.logger.debug("Sending stop_session command to master")
        
        # stop the handlers
        self._logTopicForwarder.setStopFlag()
        self._resultQueueForwarder.setStopFlag()

        # Send MCQDaemon that we are stopping
        payload = {"command":"stop_session", "uuid":self._sessionUUID}
        msg =self.create_msg(payload)      
        self._masterCommandQueue.send(msg)

    def set_killswitch(self, killswitch):
        """
        The threads are managed outside of this object. The main comm method
        is via the killswith
        """
        global workerThreadKillswitch
        workerThreadKillswitch = killswitch



    def run_job(self, job_parameters, job, limiter, killswitch):
        """

        """
        #TODO: THIS APPENDING OF QUEUE NAMES SHOULD BE MOVED TO THE RUN
        # COMMAND IN  THE FRAMEWORK
        host = job_parameters['node']
        limiter[job.host].acquire()
        time_info_start = time.time()
        job_uuid = None

        try:
            # We could have received a stop (ctrl-c) so check here if it is set
            if killswitch.isSet():
                self.logger.debug("Shutdown in progress: not starting remote job")
                self.results = {}
                self.results['returncode'] = 1
                return 1


            job_parameters['cmd'] = " ".join([job_parameters['cmd'], self._returnQueueName,
                                self._logTopicName])
            job_uuid = uuid.uuid4().hex

            payload = {"command":"run_job", 
                       "session_uuid":self._sessionUUID,
                       'job_uuid':job_uuid,
                       "parameters":job_parameters}

            msg = self.create_msg(payload)       
            self._masterCommandQueue.send(msg)
            with self._running_jobs_lock:
                self._running_jobs[job_uuid] = {'payload':copy.deepcopy(msg.payload),
                                            'completed':False}

            # wait until the job returns then return, this needs a lock around the
            # running jobs object.
            poll_interval = 1  # check for results each second
            while True:
                time.sleep(poll_interval)
                if killswitch.isSet():
                    self.logger.debug("Shutdown in progress: not starting remote job")
                    self.results = {}
                    self.results['returncode'] = 1
                    return 1

                with self._running_jobs_lock:
                    # If both the exit_value (of the recipe executable)
                    # and the output (generated in the recipe and send there)
                    # are present we are done with the job
                    if 'exit_value' in self._running_jobs[job_uuid] and \
                       'output'     in self._running_jobs[job_uuid]:
                        break
        finally:
            limiter[job.host].release()  # always release the node lock

        time_info_end = time.time()

        # Now retrieve all the results and set correct values on the job
        job.results["job_duration"] = str(time_info_end - time_info_start)
        job.results['returncode'] = self._running_jobs[job_uuid]['exit_value']
        job.results.update(self._running_jobs[job_uuid]['output'])


        return self._running_jobs[job_uuid]['exit_value']

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
