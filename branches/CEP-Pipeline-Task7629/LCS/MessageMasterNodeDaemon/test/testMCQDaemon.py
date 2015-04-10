# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

import uuid

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

from qmf.console import Session as QMFSession

import logging
import time
# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("MessageBus")


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
    def __init__(self):
        # Each MCQDaemonLib triggers a session with a uuid, generate and store
        # as a hex
        self._sessionUUID = uuid.uuid4().hex

        # should be moved to a config file
        self.broker="127.0.0.1" 
        self._returnQueueTemplate = "MCQDaemon.return.{0}"
        self._logTopicTemplate = "MCQDaemon.log.{0}"
        
        self._returnQueueName = self._returnQueueTemplate.format(self._sessionUUID)
        self._logTopicName = self._logTopicTemplate.format(self._sessionUUID)
        self.queueName = "username.LOCUS102.MCQueueDaemon.CommandQueue"


        # Connect to the HCQDaemon
        self._sendCommandQueue = msgbus.ToBus(self.queueName, 
            options = "create:always, node: { type: queue, durable: True}",
            broker = self.broker)

        # Check state, raise exception if incorrect
        self._check_queue_and_daemon_state()

        # Send a start session command with the correct details to the
        # HCQDaemon
        self._start_session()

        # Now connect to the created topic and resultQ
        self._connect_to_queue_and_topic()
  
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
        session.addBroker(self.broker)  # is the assignment needed?
        queues = session.getObjects(_class="queue",
                                    _package="org.apache.qpid.broker")

        nr_consumers_of_CQ = None
        for queue_item in queues:
            if self.queueName in queue_item.name :
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
            broker = self.broker)

        self._logTopic = msgbus.FromBus(self._logTopicTemplate, 
            options = "create:always, node: { type: topic, durable: True}",
            broker = self.broker)

    #def __del__(self)

    def _release(self):
        """
        delete the attached queue and topic
        Send to msg to the HCQDaemon command queue to remove the registered
        session uuid 
        """
        # First disconnect from the queues
        self._resultQueue.close()
        self._logTopic.close()
         
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

    def __del__():
        """

        """
        self._release()
        

if __name__ == "__main__":
    print "Hello world"

    MCQLib = MCQDaemonLib()

    # Connect to the HCQDaemon
    time.sleep(10)
    MCQLib._release()

    time.sleep(10)

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