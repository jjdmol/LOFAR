# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

import logging
import unittest

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

import lofarpipe.daemons.SCQLib as SCQLib

import socket
HOST_NAME = socket.gethostname()


class testForwardOfJobMsgToQueueuSlave(
                unittest.TestCase):

    def __init__(self, arg):  
        super(testForwardOfJobMsgToQueueuSlave, self).__init__(arg)

    # For now leave the setup and tearDown empty: single test
    # when the number of test increased it is an idea to implement them
    def setUp(self):
        pass

    def tearDown(self):
        pass
  
    def test_use_logging_handler_to_queue(self):
        return
        broker = HOST_NAME 
        job_uuid = "123456"
        session_uuid = "654321"
        busname = "testSCQLib"
        topicName = busname + "/logging_" + session_uuid

        return
        # Queue where logmsg will be send to
        fromTopic = msgbus.FromBus(topicName, broker=broker)

        # THe SCQLib object
        sCQLibObject = SCQLib.SCQLib(broker, busname,
                                    session_uuid,  job_uuid)

        # We will be adapting the logger handler
        logger = logging.getLogger("SCQLib")

        # remove the default handler
        loghandlers = logger.handlers[:]
        for hdlr in loghandlers:  
            logger.removeHandler(hdlr) 
        # Insert our own
        logger.addHandler(sCQLibObject.QPIDLoggerHandler)
        logger.propagate = False

        # Send a msg on the logger
        logger.error("Send using qpid")

        # Now read a msg from the topic
        msg = fromTopic.get(.5)
        if not msg == None:
            msg_content = eval(msg.content().payload)
        else:
            print 'Did not receive a log msg on the logging topic'

        # validate that the content is correct
        expected_msg_content= {'level': 'ERROR', 'sender': 'STATIC HOSTNAME',
                               'log_data': 'Send using qpid'}
        if msg_content != expected_msg_content:
              raise Exception("did not receive correct msg content")



    def test_SCQLib_with_parameters(self):
        broker = HOST_NAME
        job_uuid = "123456"
        session_uuid = "654321"
        busname = "testSCQLib"
        topicName =busname + "/logging_" + session_uuid

        parameterQueueName = busname + "/parameters_" + session_uuid + "_" + job_uuid


        # Queue where logmsg will be send to
        fromTopic = msgbus.FromBus(topicName, broker=broker)

        # THe SCQLib object
        sCQLibObject = SCQLib.SCQLib(broker, busname,
                                    session_uuid,  job_uuid)

        # send a startjob msg to the correct queue
        parameterQ = msgbus.ToBus(parameterQueueName, broker=broker)
        send_payload =  {'command':'run_job',
                         'session_uuid':"123456321654",
                         'job_uuid': "654321",
                         'node':"ANODE",
                         'parameters':{
                           'cdw': "/home",
                           "job_parameters":{},
                           'environment':  {"ENV":"Value"},
                           'cmd': "ls"}}

        msg = SCQLib.create_msg(send_payload)

        parameterQ.send(msg)

        parameters = sCQLibObject.getArguments()



# TODO: The amount of testing on this class does not feel right. It should be 
# increased


if __name__ == "__main__":
    unittest.main()
