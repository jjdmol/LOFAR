# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

#import uuid
#import copy
#import os
#import logging
#import time

#import pwd
#import socket  # needed for username TODO: is misschien een betere manier os.environ['USER']
#import signal
import time
import unittest
import threading 

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

import lofarpipe.daemons.MCQLib as MCQLib
import CQDaemonTestFunctions as testFunctions
from lofarpipe.support.remotecommand import ProcessLimiter 

import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

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

class testMCQLib(unittest.TestCase):

    def __init__(self, arg):  
        super(testMCQLib, self).__init__(arg)

    # For now leave the setup and tearDown empty: single test
    # when the number of test increased it is an idea to implement them
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_init(self):
        busname = "testmcqdaemon"
        broker = "locus102"
        logger = logging.getLogger("testMCQLib")

        mcqlib = MCQLib.MCQLib(logger, broker, busname)

    def test_logtopic_handler(self):
        # TODO: Add test that the loglines are send to logging:
        #      test the _process_log_message member
        busname = "testmcqdaemon"
        broker = "locus102"
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
        busname = "testmcqdaemon"
        broker = "locus102"
        logger = logging.getLogger("testMCQLib")
        resultsQueueName = busname + "/resultsqueuehandler1"
        
        #Wrapper to catch the call to the handler functioinality
        class resultQueueHandlerrWrapper(MCQLib.resultQueueHandler):
            def __init__(self,broker, resultQueueName, 
                 running_jobs, 
                 running_jobs_lock, 
                 logger=None, 
                 poll_interval=1.0):
                super(resultQueueHandlerrWrapper, self).__init__(
                          broker,resultQueueName,
                          running_jobs, running_jobs_lock,
                          logger,  poll_interval)

                self._process_queue_message_called = False
                self._msg_payload = None

            def _process_queue_message(self, msg_content):
                self._process_queue_message_called = True
                self._msg_payload = msg_content

        # We need a jobs dict and the lock
        running_jobs = {}
        running_jobs_lock = threading.Lock()
        
         # create wrapped handler
        resulthandler = resultQueueHandlerrWrapper(broker, resultsQueueName,
                            running_jobs, running_jobs_lock,
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
        busname = "testmcqdaemon"
        broker = "locus102"
        logger = logging.getLogger("testMCQLib")
        resultsQueueName = busname + "/resultsqueuehandler"
       
        # We need a jobs dict and the lock
        
        job_uuid = "123456"

        running_jobs = {job_uuid:{}}
        running_jobs_lock = threading.Lock()
        
         # create wrapped handler
        resulthandler = MCQLib.resultQueueHandler(broker, resultsQueueName,
                            running_jobs, running_jobs_lock,
                            logger,0.5)
        
        resulthandler.start()  # start the thread

        # we need to send msgs to the handler        
        resultsQueue= msgbus.ToBus(resultsQueueName, broker = broker)

        payload = {'type': 'exit_value',
                   'exit_value': -1,
                   'job_uuid': job_uuid}

        msg = create_msg(payload)
        resultsQueue.send(msg)
        time.sleep(2)

        # THe exit value should have been added to the running jobs dict

        self.assertEqual({job_uuid:{"exit_value":-1}}, running_jobs)

        ## allow some time for the msg to arrive
        payload = {'type': 'output',
                   'output': "some data",
                   'job_uuid': job_uuid}

        msg = create_msg(payload)
        resultsQueue.send(msg)
        time.sleep(2)          
        self.assertEqual({job_uuid:{
                            "exit_value":-1,
                            "output":"some data"}},
                         running_jobs)


        resulthandler.setStopFlag()


    def test_run_job(self):
        # Create sut
        busname = "testmcqdaemon"
        broker = "locus102"
        logger = logging.getLogger("testMCQLib")
        mcqobj = MCQLib.MCQLib(logger, broker, busname)

        # parameters
        class job:
            def __init__(self):
              self.host = "locus102"
              self.results = {}

        limiter = ProcessLimiter(nproc=1)
        parameters = {
                      "node":"locus102",
                      "cmd":"ls"}
        jobObject = job()
        killswitch = threading.Event()
        
        # Start the run_job as a thread
        thread = threading.Thread(
                            target = mcqobj.run_job,
                     args = [parameters, jobObject, limiter, killswitch])
        thread.daemon = True  
        thread.start() 

        # We expect a job msg on the deadletterqueue
        #masterCommandQueueName = "masterCommandQueueName"
        deadLetterQueueName = busname + ".deadletter"
        deadLetterQueue = msgbus.FromBus(deadLetterQueueName)

        msg = testFunctions.try_get_msg(deadLetterQueue, 2) 
        if msg == None:
            commandQueueBus.close()
            daemon.close()
            raise Exception(
                 "Did not receive the expect msg on the deadletter queue")

        deadLetterQueue.ack(msg)         
        content = eval(msg.content().payload)

        # Queue test if the received msg is a run_job command
        self.assertTrue(content["command"] == 'run_job')

        # some data from the sent msg:
        uuid = content['uuid']
        job_uuid = content['job_uuid']


        #now send a results msg to the correct temp queue
        toBus = msgbus.ToBus(busname, broker=broker)
        payload = {'type': 'exit_value',
                   'exit_value': -1,
                   'job_uuid': job_uuid}

        msg = create_msg(payload)
        msg.set_subject("result_" +uuid)  # use subbject to get the correct temp queue
        toBus.send(msg)


        payload = {'type': 'output',
                   'output': {"some":"data"},
                   'job_uuid': job_uuid}
        msg = create_msg(payload)
        msg.set_subject("result_" +uuid)  # use subbject to get the correct temp queue
        toBus.send(msg)
        time.sleep(2) # results queue is emptied each 1 second
        
        # a duration should have been added
        self.assertTrue('job_duration' in jobObject.results)
        # an exit code
        self.assertEqual(jobObject.results['returncode'], -1)
        # results data
        self.assertEqual(jobObject.results['some'], 'data')


if __name__ == "__main__":
    unittest.main()  
