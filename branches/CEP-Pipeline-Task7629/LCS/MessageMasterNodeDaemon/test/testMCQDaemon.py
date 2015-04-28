# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

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


# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("MessageBus")

#Handler for stopping the logTopicHandler
logTopicStopFlag = threading.Event()
def logTopicStopHandler(signum, stack):
    logTopicStopFlag.set()
    exit(signum)

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
            self.logger.info("Polling logTopic")
            # We do not want to listen in a tight loop. read the msg each 
            # pollinterval           
            if self.poll_counter < self.poll_interval:
                self.poll_counter += 0.1
                time.sleep(0.1)
                continue

            # reset the counter to zero, and restart the wait period
            self.poll_counter = 0       

            # now empty the queue
            while True:
                # get is blocking, always use timeout.
                msg = self._logTopic.get(0.1)  
                if msg == None:
                    break   # break the loop

                # process the log data
                msg_content = eval(msg.content().payload)
                self._process_log_message(msg_content)
 
    def setStopFlag(self):
        """
        Stop the monitor
        """
        self.stopFlag.set()

    def __del__(self):
        """
        Clean up the temp file after the file is not in use anymore
        """
        self._logTopic.close()


class MCQDaemonLib(object):
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
        signal.signal(signal.SIGTERM, 
                      logTopicStopHandler)   # from 'global' scope
        signal.signal(signal.SIGINT, 
                      logTopicStopHandler)
        self._logTopicForwarder.start()

  
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
        self._resultQueue = msgbus.FromBus(self._returnQueueName, 
            options = "create:always, node: { type: queue, durable: True}",
            broker = self._broker)
        self.logger.info("Connecting to returnQueue and logTopic. Done")

    def __del__(self):
        self._release()

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
        self._resultQueue.close()        

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
                      return self._running_jobs[job_uuid]['return_value']


            

    
       

if __name__ == "__main__":
    MCQLib = MCQDaemonLib(logger)

    environment = dict(
            (k, v) for (k, v) in os.environ.iteritems()
                if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
        )

    parameters = {'node':'locus102',
                  #'cmd': '/home/klijn/build/7629/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/nodes/test_recipe.py',
                  'environment':environment,
                  'cmd': 'echo test',
                  #'cmd': """echo 'print "test"' | python """,
                  #'cmd':""" echo  "test" """,
                  'cdw': '/home/klijn',
                  'job_parameters':{'par1':'par1'}}



    MCQLib.run_job(parameters)

    MCQLib.run_job(parameters)
    # Connect to the HCQDaemon
    time.sleep(5)
    MCQLib._release()

    time.sleep(1)

    #if __name__ == "__main__":
    #    daemon = MCQ.MCQDaemon("daemon_state_file.pkl", 1, 2)


    ## we are testing
    ## put some commands in the queue

    ## skip one loop
    #daemon._commandQueue.append({'command':'no_msg'})  

    ## add a new pipeline session
    #daemon._commandQueue.append({'command':'start_session', 'uuid':"uuid_001"})

    ## skip one loop
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'add_consumer', 'uuid':"uuid_001"})

    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    ## delete the consumer on the session
    #daemon._commandQueue.append({'command':'del_consumer', 'uuid':"uuid_001"})
    ## This results in the session to be removed 
    #daemon._commandQueue.append({'command':'no_msg'})
    ## Create a second session: What happens if a queue is deleted when there are consumers???
    ## interesting
    #daemon._commandQueue.append({'command':'start_session', 'uuid':"uuid_002"})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'add_consumer', 'uuid':"uuid_002"})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'stop_session', 'uuid':"uuid_002"})
    #daemon._commandQueue.append({'command':'no_msg'})

    #daemon._commandQueue.append({'command':'quit',"clear_state":"true"})



    #daemon.run()