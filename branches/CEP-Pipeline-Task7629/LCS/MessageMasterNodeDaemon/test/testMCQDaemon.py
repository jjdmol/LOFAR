# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

import uuid
import copy
import os
import logging
import time
import threading 

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
from qmf.console import Session as QMFSession


# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("MessageBus")

class logTopicForwarder(threading.Thread):
    """
    Class used for listening a emptying a log topic

    usage:
    After initiation it must be started using the start() method
    setStopFlag() stops the forwarder
    """
    def __init__(self, logTopic, logger=None,  poll_interval=1.0):
        """
        Create the usage stat object. Create events for starting and stopping.
        By default the Process creating the object is tracked.
        Default polling interval is 10 seconds
        """
        threading.Thread.__init__(self)
        self.logger = logger
        self.stopFlag = threading.Event()
        self.lock = threading.Lock()
        self.poll_interval = poll_interval
        self.poll_counter = 0
            
    def __del__(self):
        """
        Clean up the temp file after the file is not in use anymore
        """
        pass
            

    def run(self):
        """
        Run function. 

        While no stopflag is set:
        sleep for poll_interval
        """
        while not self.stopFlag.isSet():
            # If the timer waits for the full 10 minutes using scripts
            # appear halting for lnog durations after ending
            # poll in a tight wait loop to allow quick stop
            if self.poll_counter < self.poll_interval:
                self.poll_counter += 0.1
                time.sleep(0.1)
                continue

            # reset the counter to zero
            self.poll_counter = 0           
 
    def setStopFlag(self):
        """
        Stop the monitor
        """
        self.stopFlag.set()
     


class MCQDaemonLib(object):
    """
    Interface lib connecting to a local MCQDaemon command queue.
    Hides all queue interactions behind function specific functions.

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
        self._sessionUUID = uuid.uuid4().hex

        # should be moved to a config file
        self._broker="127.0.0.1" 
        self._returnQueueTemplate = "MCQDaemon.return.{0}"
        self._logTopicTemplate = "MCQDaemon.log.{0}"
        
        self._returnQueueName = self._returnQueueTemplate.format(self._sessionUUID)
        self._logTopicName = self._logTopicTemplate.format(self._sessionUUID)
        self._queueName = "username.LOCUS102.MCQueueDaemon.CommandQueue"

        self._resultQueue = None
        self._logTopic    = None

        # It is the lib that 'owns' the session and 
        self._running_jobs = {}

        # Connect to the HCQDaemon
        self._sendCommandQueue = msgbus.ToBus(self._queueName, 
            options = "create:always, node: { type: queue, durable: True}",
            broker = self._broker)

        # Check state, raise exception if incorrect
        self._check_queue_and_daemon_state()

        # Send a start session command with the correct details to the
        # HCQDaemon
        self._start_session()

        # Now connect to the created topic and resultQ
        self._connect_to_queue_and_topic()

        # Start the log poller in a seperate thread.
        self._logTopicForwarder = logTopicForwarder(self._logTopic, self.logger)
        self._logTopicForwarder.start()

  
    def _check_queue_and_daemon_state(self):
        """
        Helper function for the __init__ member

        Checks if the connect with the commandqueue has been established
        Checks if there is a MAster daemon listening (by checking the number
        of consumers of the command queue.)

        Raise Exceptions on errors
        else returns with void
        """
        # Check for consumers: We need to know if the deamon is active before
        # we can send commands
        session=QMFSession()
        session.addBroker(self._broker) 
        queues = session.getObjects(_class="queue",
                                    _package="org.apache.qpid.broker")

        nr_consumers_of_CQ = None
        for queue_item in queues:
            if self._queueName in queue_item.name :
                nr_consumers_of_CQ = queue_item.consumerCount

        if nr_consumers_of_CQ == None:
            raise Exception("Could not find command queue, is QPID enabled?")

        if nr_consumers_of_CQ == 0:
            raise Exception("No HeadNodeCommandQueueDaemon detected, aborting")


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
        self._sendCommandQueue.send(msg)

    def _connect_to_queue_and_topic(self):
        """ 
        Connect the topic and command queue that have been created by send
        session command to the daemon
        """
        self._resultQueue = msgbus.FromBus(self._returnQueueName, 
            options = "create:always, node: { type: queue, durable: True}",
            broker = self._broker)

        self._logTopic = msgbus.FromBus(self._logTopicTemplate, 
            options = "create:always, node: { type: topic, durable: True}",
            broker = self._broker)

    #def __del__(self)

    def _release(self):
        """
        delete the attached queue and topic
        Send to msg to the HCQDaemon command queue to remove the registered
        session uuid 
        """
        #
        print "we have received a quit command!!!!!!!"
        # First disconnect from the queues
        self._resultQueue.close()
        self._logTopic.close()
        
        self._logTopicForwarder.setStopFlag()

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
        self._sendCommandQueue.send(msg)


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

        self._sendCommandQueue.send(msg)

        self._running_jobs[job_uuid] = {'payload':copy.deepcopy(msg.payload),
                                        'completed':False}

        # wait until the job returns then return, this needs a lock around the
        # running jobs object.



        

if __name__ == "__main__":
    print "Hello world"

    MCQLib = MCQDaemonLib(logger)

    environment = dict(
            (k, v) for (k, v) in os.environ.iteritems()
                if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
        )


    parameters = {'node':'locus102',
                  #'cmd': '/home/klijn/build/7629/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/nodes/test_recipe.py',
                  'environment':environment,
                  'cmd': 'ls',
                  #'cmd': """echo 'print "test"' | python """,
                  #'cmd':""" echo  "test" """,
                  'cdw': '/home/klijn',
                  'job_parameters':{'par1':'par1'}}



    MCQLib.run_job(parameters)

    MCQLib.run_job(parameters)
    # Connect to the HCQDaemon
    time.sleep(2)
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