import time
import unittest
import threading 
import socket
import logging
from contextlib import nested   #>2.7 allows nesting out of the box

from lofarpipe.support.remotecommand import ProcessLimiter 

import lofar.messagebus.CQExceptions as CQExceptions
import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

import lofarpipe.daemons.MCQLib as MCQLib
from lofarpipe.daemons.inMemoryZip import get_zipstring_from_string

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
HOST_NAME = socket.gethostname()

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

def try_get_msg(queue, wait_period=10):
    """
    Helper function, try to get msg from queue. raise exception if not gotten
    after 10 sec. return msg if received

    """
    # We expect a deadletter on the 
    idx = 0
    msg_received = None
    while (True):
        if idx >= wait_period:
            print "Did not receive a msg after {0} seconds!!".format(
                      wait_period)
            return None

        msg_received = queue.get(1)
        if msg_received is None:
            print "Did not receive a msg on  queue"
            idx += 1
            time.sleep(1)
            continue

        queue.ack(msg_received)
        break

    return msg_received

class testMCQLib(unittest.TestCase):

    def __init__(self, arg):  
        super(testMCQLib, self).__init__(arg)

    # For now leave the setup and tearDown empty: single test
    # when the number of test increased it is an idea to implement them
    def setUp(self):
        pass

    def tearDown(self):
        busname = "testMCQLib"
        broker = HOST_NAME
        logger = logging.getLogger("testMCQLib")
        deadLetterQueueName = busname + ".deadletter"
        deadLetterQueue = msgbus.FromBus(deadLetterQueueName)
        while True:
            print "Clearing up the deadletter queue, to prevent crosstalk between tests"
            msg = try_get_msg(deadLetterQueue, 0.1) 
            if msg == None:
                break

        deadLetterQueue.close()
        print "Clearing up the deadletter queue, Done"


    def test_logtopic_handler(self):
        # TODO: Add test that the loglines are send to logging:
        #      test the _process_log_message member
        busname = "testMCQLib"
        broker = HOST_NAME
        logger = logging.getLogger("testMCQLib")
        logTopicName = busname + "/logtopichandler"
        
        #Wrapper to catch the call to the content handler
        class logTopicHandlerWrapper(MCQLib.logTopicHandler):
            def __init__(self, broker,logTopicName, logger=None,  poll_interval=1.0):  
                super(logTopicHandlerWrapper, self).__init__(
                           broker,logTopicName, logger,  poll_interval)

                self._process_log_message_called = False
                self._log_payload = None

            def _process_log_message(self, msg_content):
                self._process_log_message_called = True
                self._log_payload = msg_content

        
         # create wrapped handler
        logHandler = logTopicHandlerWrapper(broker, logTopicName, logger,1.0)
        
        logHandler.start()  # start the thread
        
        # create a to bus and send log msg        
        logToBus = msgbus.ToBus(logTopicName, broker = broker)

        payload = {'level': 'ERROR',
                   'sender': 'STATIC HOSTNAME',
                   'log_data': 'Send using qpid'}
        msg = create_msg(payload)
        logToBus.send(msg)

        # allow some time for the msg to arrive
        time.sleep(0.1)  
        self.assertEqual(logHandler._log_payload, payload)
        logHandler.setStopFlag()


    def test_results_handler(self):
        # TODO: Add test that the loglines are send to logging:
        #      test the _process_log_message member
        busname = "testMCQLib"
        broker = HOST_NAME
        logger = logging.getLogger("testMCQLib")
        resultsQueueName = busname + "/resultsqueuehandler1"
        
        #Wrapper to catch the call to the handler functioinality
        class resultQueueHandlerrWrapper(MCQLib.resultQueueHandler):
            def __init__(self,
                         broker, 
                         resultQueueName, 
                         running_jobs, 
                         running_jobs_lock, 
                         master_echo,
                         logger=None, 
                         poll_interval=1.0):
                super(resultQueueHandlerrWrapper, self).__init__(
                          broker,
                          resultQueueName,
                          running_jobs, 
                          running_jobs_lock,
                          master_echo,
                          logger,  
                          poll_interval)

                self._process_queue_message_called = False
                self._msg_payload = None

            def _process_queue_message(self, msg_content):
                self._process_queue_message_called = True
                self._msg_payload = msg_content

        # We need a jobs dict and the lock
        running_jobs = {}
        running_jobs_lock = threading.Lock()
        master_echo= {"received":False}  
         # create wrapped handler
        resulthandler = resultQueueHandlerrWrapper(broker, resultsQueueName,
                            running_jobs, running_jobs_lock, master_echo,
                            logger,1.0)
        
        resulthandler.start()  # start the thread
        
        # create a to bus and send log msg        
        resultsQueue= msgbus.ToBus(resultsQueueName, broker = broker)

        payload = {'type': 'exit_value',
                   'exit_value': -1,
                   'job_uuid': '123456'}

        msg = create_msg(payload)
        resultsQueue.send(msg)

        ### allow some time for the msg to arrive
        time.sleep(0.1)  
        self.assertEqual(resulthandler._msg_payload, payload)
        resulthandler.setStopFlag()


    def test_results_handler_exit_output(self):
        # TODO: Add test that the loglines are send to logging:
        #      test the _process_log_message member
        busname = "testMCQLib"
        broker = HOST_NAME
        logger = logging.getLogger("testMCQLib")
        resultsQueueName = busname + "/resultsqueuehandler"
       
        # We need a jobs dict and the lock
        
        job_uuid = "123456"

        running_jobs = {job_uuid:{'node':HOST_NAME}}
        running_jobs_lock = threading.Lock()
        master_echo= {"received":False}  
         # create wrapped handler
        resulthandler = MCQLib.resultQueueHandler(broker, resultsQueueName,
                            running_jobs, running_jobs_lock, master_echo, 
                            logger,0.5)
        
        resulthandler.start()  # start the thread

        # we need to send msgs to the handler        
        resultsQueue= msgbus.ToBus(resultsQueueName, broker = broker)

        payload = {'node':HOST_NAME,
                   'type': 'exit_value',
                   'exit_value': -1,
                   'job_uuid': job_uuid}

        msg = create_msg(payload)
        resultsQueue.send(msg)
        time.sleep(0.6)

        # THe exit value should have been added to the running jobs dict
        expected_output = {job_uuid:{"exit_value":-1, 
                                     'node':HOST_NAME,
                                     'output':[]}}   # is added by the lib
        self.assertEqual(expected_output, running_jobs)

        ## allow some time for the msg to arrive
        payload = {'type': 'output',
                   'output': get_zipstring_from_string(str(["some data"])), # pack the data
                   'job_uuid': job_uuid}

        msg = create_msg(payload)
        resultsQueue.send(msg)
        time.sleep(0.6)          
        self.assertEqual({job_uuid:{
                            "exit_value":-1,
                            'node':HOST_NAME,
                            "output":["some data"]}},
                         running_jobs)

        resulthandler.setStopFlag()


    def test_run_job(self):
        busname = "testMCQLib"
        broker = HOST_NAME
        logger = logging.getLogger("testMCQLib")
        deadLetterQueueName = busname + ".deadletter"

        toBus = msgbus.ToBus(busname, broker=broker)
        deadLetterQueue = msgbus.FromBus(deadLetterQueueName)

        with nested(toBus, deadLetterQueue
                     ) as (
                      toBus, deadLetterQueue):        

            mcqobj = MCQLib.MCQLib(logger, broker, busname, master_echo=True)

            # parameters
            class job:
                def __init__(self):
                  self.host = HOST_NAME
                  self.results = {}

            limiter = ProcessLimiter(nproc=1)
            parameters = {
                          "node":HOST_NAME,  # must be valid
                          "cmd":"ls"}         # must exist
            jobObject = job()
            killswitch = threading.Event()
        
            # Start the run_job as a thread
            thread = threading.Thread(
                                target = mcqobj.run_job,
                         args = [parameters, jobObject, limiter, killswitch])
            thread.daemon = True  
            thread.start() 

            # We expect a job msg on the deadletterqueue
            # The echo msg for the master should be received on the deadletter bus
            msg = try_get_msg(deadLetterQueue, 2) 
            if msg == None:
                raise Exception(
                     "Did not receive the expect msg on the deadletter queue")  
            content = eval(msg.content().payload)                
            self.assertEqual(content["command"],'echo')

            # Now to get the job msg
            msg = try_get_msg(deadLetterQueue, 2) 
            if msg == None:
                raise Exception(
                     "Did not receive the expect msg on the deadletter queue")  
            content = eval(msg.content().payload)      
        
            self.assertEqual(content["command"],'run_job')



            # some data from the sent msg:
            uuid = content['session_uuid']
            job_uuid = content['job_uuid']


            #now send a results msg to the correct temp queue
            payload = {'type': 'exit_value',
                       'exit_value': -1,
                       'job_uuid': job_uuid}

            msg = create_msg(payload)
            msg.set_subject("result_" +uuid)  # use subbject to get the correct temp queue
            toBus.send(msg)


            payload = {'type': 'output',
                       'output': get_zipstring_from_string(str({"some":"data"})),
                       'job_uuid': job_uuid}
            msg = create_msg(payload)
            msg.set_subject("result_" +uuid)  # use subbject to get the correct temp queue
            toBus.send(msg)
            time.sleep(2) # results queue is emptied each 1 second
        
            # a duration should have been added
            print jobObject.results
            self.assertTrue('job_duration' in jobObject.results)
            # an exit code
            self.assertEqual(jobObject.results['returncode'], -1)
            # results data
            self.assertEqual(jobObject.results['some'], 'data')


if __name__ == "__main__":
    unittest.main()  
