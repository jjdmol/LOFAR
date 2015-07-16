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
import os
import unittest
import time
from contextlib import nested   #>2.7 allows nesting out of the box

import CQDaemonTestFunctions as testFunctions
import lofarpipe.daemons.pipelineSCQDaemonImp as PipelineSCQDaemonImp

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

import socket
HOST_NAME = socket.gethostname()


# Wraps the actual slave implementation, allows to catch calls to internal 
# function we need to validate.
class testForwardOfJobMsgToQueueuSlaveWrapper(
            PipelineSCQDaemonImp.PipelineSCQDaemonImp):
    def __init__(self, 
                 broker,
                 busname, 
                 commandQueueName,
                 deadLetterQueueName,
                 deadletterfile,
                 logfile,
                 loop_interval,
                 daemon,
                 n_repost):
        super(testForwardOfJobMsgToQueueuSlaveWrapper, self).__init__(
           broker,
           busname, 
           commandQueueName,
           deadLetterQueueName,
           deadletterfile, 
           logfile,
           loop_interval, 
           daemon,
           n_repost)
        pass
        self._start_subprocess_called = False
        self._process_deadletter_parameters_msg_called = False

    def _start_subprocess(self):
        """
        This function hides the actual run subprocess module
        """
        self._start_subprocess_called = True

    def process_deadletter(self, msg, unpacked_msg_content, msg_type):
        """

        """
        self._process_deadletter_parameters_msg_called = True
        #return True



class testForwardOfJobMsgToQueueuSlave(
                unittest.TestCase):

    def __init__(self, arg):  
        super(testForwardOfJobMsgToQueueuSlave, self).__init__(arg)

    def setUp(self):
        self.logfile = "/tmp/testPipelineSCQDaemon.log"
        open( self.logfile , 'a').close()
        self.deadletterfile = "/tmp/testPipelineSCQDaemonDeadletter.log"
        open( self.deadletterfile , 'a').close()
        job_node = 'locus102'

    def tearDown(self):
        pass
        os.remove(self.logfile)
        os.remove(self.deadletterfile)

  
    def test_run_job_results_in_parameters_msg_on_bus(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """
        slaveCommandQueueNameTemplate = "slaveCommandQueue_{0}"
        daemon, commandQueueBus,  deadletterToQueue = \
            testFunctions.prepare_test_SCQ(testForwardOfJobMsgToQueueuSlaveWrapper,
                             self.logfile, self.deadletterfile )

        with nested(daemon, commandQueueBus, 
                     deadletterToQueue) as (
                      daemon, commandQueueBus, deadletterToQueue):

            # Create the daemon and get all the default queues
            job_node = 'locus102'

            # Test1: Create a test job payload
            send_payload =  {'type': 'command',
                             'command':'run_job',
                             'session_uuid':"123456321654",
                             'job_uuid': "654321",
                             'node':job_node,
                             'parameters':{
                               'cdw': "/home",
                               'environment':  {"ENV":"Value"},
                               'cmd': "ls"}}

            msg = testFunctions.create_test_msg(send_payload)
            commandQueueBus.send(msg)

            # Run the process loop, parameters will be send to a bus adress that does
            # not exist, it should end up in the deadletter queue
            daemon._process_command_queue()

            # read from the deadletter queue
            msg = testFunctions.try_get_msg(daemon._deadletterFromBus, 2) 
            if msg == None:
                raise Exception(
                     "Did not receive the expect msg on the deadletter queue")
            #daemon._deadletterFromBus.ack(msg) 
            #send_payload['info']['subject'] ='parameters_123456321654_654321'
            # check that the correct msg is receive in the deadletter queue 
            
            target_payload = send_payload
            target_payload['type'] = 'parameters'    # the msg is of type parameters
                                      # Added info by starter
            target_payload['info']= {'target': 'SCQLib', 'sender': 'subprocessStarter', 'subject': 'parameters_123456321654_654321'}
                   
            unpacked_msg_data = eval(msg.content().payload)

            self.assertEqual(unpacked_msg_data, send_payload)
        
        

    def test_deadletterQueue_startjob_processing(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """
        slaveCommandQueueNameTemplate = "slaveCommandQueue_{0}"
        daemon, commandQueueBus, deadletterToQueue = \
            testFunctions.prepare_test_SCQ(testForwardOfJobMsgToQueueuSlaveWrapper,
                             self.logfile, self.deadletterfile )

        with nested(daemon, commandQueueBus, 
                    deadletterToQueue) as (
                      daemon, commandQueueBus,  deadletterToQueue):
            # Test1: Create a test job payload
            send_payload =  {'type':'command',
                             'command':'run_job',
                             'session_uuid':"123456321654",
                             'job_uuid': "654321",
                             'node':HOST_NAME,
                             'parameters':{
                               'cdw': "/home",
                               'environment':  {"ENV":"Value"},
                               'cmd': "ls"}}

            msg = testFunctions.create_test_msg(send_payload)
            commandQueueBus.send(msg)

            # Run the process loop, The job will be send to a bus adress that does
            # not exist, it should end up in the deadletter queue
            daemon._process_command_queue()

            # Run the deadletter processer
            daemon._process_deadletter_queue()

            self.assertTrue(daemon._process_deadletter_parameters_msg_called)
        
    def test_start_node_recipe_no_connection_with_lib_2times(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        If the recipe does not receive the parameter msg it should be resend
        """
        # Create the daemon and get all the default queues

        slaveCommandQueueNameTemplate = "slaveCommandQueue_{0}"
        daemon, commandQueueBus, deadletterToQueue = \
            testFunctions.prepare_test_SCQ(PipelineSCQDaemonImp.PipelineSCQDaemonImp,
                             self.logfile, self.deadletterfile )

        with nested(daemon, commandQueueBus, 
                    deadletterToQueue) as (
                      daemon, commandQueueBus,  deadletterToQueue):
            job_node = HOST_NAME
            environment = dict(
                (k, v) for (k, v) in os.environ.iteritems()
                    if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
            )

            # Test1: Create a test job payload
            send_payload =  {'type':'command',
                             'command':'run_job',
                             'session_uuid':"123456",
                             'job_uuid': "654321",
                             'node':job_node,
                             'info':"test_start_node_recipe",
                             'parameters':
                             {'node':'dop282',
                              'environment':environment,
                              'cmd': "sleep 5;",
                              'cdw': '/home/klijn',
                              'job_parameters':{'par1':'par1'}}
                         
                             }

            msg = testFunctions.create_test_msg(send_payload)
            commandQueueBus.send(msg)


            daemon._process_command_queue()

            # Run the deadletter processer twice
            daemon._process_deadletter_queue()
            daemon._process_deadletter_queue()

            # Now check if deadletter contains a msg with n_repost = 2
            msg = testFunctions.try_get_msg(daemon._deadletterFromBus, 2) 
            if msg == None:
                commandQueueBus.close()
                daemon.close()
                raise Exception(
                     "Did not receive the expect msg on the deadletter queue")

      
            self.assertTrue(eval(msg.content().payload)['n_repost'] == 2)


    def test_start_node_recipe_no_connection_with_lib_3times(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        If the recipe does not receive the parameter msg it should be resend

        after 2 attempts the job should be killed and a log and result msg
        send

        """
        # Create the daemon and get all the default queues
        job_node = HOST_NAME
        slaveCommandQueueNameTemplate = "slaveCommandQueue_{0}"
        daemon, commandQueueBus, deadletterToQueue = \
            testFunctions.prepare_test_SCQ(PipelineSCQDaemonImp.PipelineSCQDaemonImp,
                             self.logfile, self.deadletterfile )

        with nested(daemon, commandQueueBus, 
                    deadletterToQueue) as (
                      daemon, commandQueueBus,  deadletterToQueue):
            environment = dict(
                (k, v) for (k, v) in os.environ.iteritems()
                    if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
            )

            # Test1: Create a test job payload
            send_payload =  {'type':'command',
                             'command':'run_job',
                             'session_uuid':"123456",
                             'job_uuid': "654321",
                             'node':job_node,
                             'info':"test_start_node_recipe",
                             'parameters':
                             {'node':'dop282',
                              'environment':environment,
                              'cmd': "sleep 5;",
                              'cdw': '/home/klijn',
                              'job_parameters':{'par1':'par1'}}
                         
                             }

            msg = testFunctions.create_test_msg(send_payload)
            commandQueueBus.send(msg)

            # Run the process loop, The job will be send to a bus adress that does
            # not exist, it should end up in the deadletter queue
            daemon._process_command_queue()

            # Run the deadletter processer twice
            daemon._process_deadletter_queue()
            daemon._process_deadletter_queue()
            daemon._process_deadletter_queue()

            # THe deadletter processing has been called three times.
            # this should result in a kill job action in the dameon
        

            # Then a results msg
            msg = testFunctions.try_get_msg(daemon._deadletterFromBus, 2) 
            if msg == None:
                commandQueueBus.close()
                daemon.close()
                raise Exception(
                     "Did not receive the expect msg on the deadletter queue")
    
            expected_content = {'info': 'Job killed',
                                'exit_value': -1,
                                'type': 'exit_value', 
                                'session_uuid': '123456', 
                                'job_uuid': '654321'}
            self.assertEqual(eval(msg.content().payload), expected_content)


    def test_start_node_recipe_full(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        If the recipe does not receive the parameter msg it should be resend

        after 2 attempts the job should be killed and a log and result msg
        send

        """

        # Create the daemon and get all the default queues
        job_node = HOST_NAME

        ## Connect to the results bus for this session id
        resultQueue = msgbus.FromBus("testmcqdaemon" + "/" + "result_" + "123456",
                  broker = job_node)
        daemon, commandQueueBus, deadletterToQueue = \
            testFunctions.prepare_test_SCQ(PipelineSCQDaemonImp.PipelineSCQDaemonImp,
                             self.logfile, self.deadletterfile )

        with nested(daemon, commandQueueBus, 
                    deadletterToQueue, resultQueue) as (
                      daemon, commandQueueBus,  deadletterToQueue, resultQueue):

            environment = dict(
                (k, v) for (k, v) in os.environ.iteritems()
                    if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
            )
            # This env variable is needed for the slave to start
            environment["USE_QPID_DAEMON"] = "True"  
            # Test1: Create a test job payload
            send_payload =  {'type':'command',
                             'command':'run_job',
                             'session_uuid':"123456",
                             'job_uuid': "test_start_node_recipe_full",
                             'node':job_node,
                             'info':"test_start_node_recipe_full",
                             'parameters':
                             {'node':'locus102',
                              'environment':environment,
                              'cmd': 'python /home/klijn/build/7629/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/nodes/test_recipe.py',
                              'cdw': '/home/klijn',
                              'job_parameters':{'par1':'par1'}}
                         
                             }

            msg = testFunctions.create_test_msg(send_payload)
            commandQueueBus.send(msg)


            # Run the process loop, The job will be send to a bus adress that does
            # not exist, it should end up in the deadletter queue
            daemon._process_command_queue()
            time.sleep(.5)  # Allow some time for the process to start
            daemon._process_deadletter_queue()  # force resend of the parameters

            # The script should send a logline on critical
            msg = testFunctions.try_get_msg(daemon._deadletterFromBus, 2) 
            if msg == None:
                commandQueueBus.close()
                daemon.close()
                raise Exception(
                     "Did not receive the expect msg on the deadletter queue")

  
            expected_content_sub_proces_exit_value = {'level': 'CRITICAL', 
                                'sender': HOST_NAME, 
                 'log_data': '#####We are in the test recipe and we are going good#####'} 
            self.assertEqual(eval(msg.content().payload), expected_content_sub_proces_exit_value)

            # Next we need to assure that the results are send correctly
            time.sleep(2)  # wait for the suprocess to be done
            daemon.process_state() 

            # We expect an exit value on the results queue

            results_received = False
            exit_received = False
        

            expected_content_sub_proces_exit_value = \
                               {'info': 'Subprocess Results',
                                'type': 'exit_value', 
                                'exit_value': 0,
                                'session_uuid': '123456',
                                'job_uuid': 'test_start_node_recipe_full'} 

            # There could be two diffent msg on the results queue
            for idx in range(2):
                msg = testFunctions.try_get_msg(resultQueue, 2) 
                if msg == None:
                    commandQueueBus.close()
                    daemon.close()
                    raise Exception(
                         "Did not receive the expect msg on the deadletter queue")

                payload_parsed = eval(msg.content().payload)
                print payload_parsed
                if payload_parsed == expected_content_sub_proces_exit_value:
                    exit_received = True
                elif payload_parsed['output']['output'] == "OUPUT FROM TEST RECIPE":
                    results_received = True
                else:
                    print payload_parsed
                    raise self.fail("Did not receive a valid results msg")






if __name__ == "__main__":
    unittest.main()

