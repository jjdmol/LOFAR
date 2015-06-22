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

import CQDaemonTestFunctions as testFunctions
import lofarpipe.daemons.pipelineSCQDaemonImp as PipelineSCQDaemonImp

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message



# Wraps the actual slave implementation, allows to catch calls to internal 
# function we need to validate.
class testForwardOfJobMsgToQueueuSlaveWrapper(
            PipelineSCQDaemonImp.PipelineSCQDaemonImp):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName,
                 loop_interval, daemon):
        super(testForwardOfJobMsgToQueueuSlaveWrapper, self).__init__(
           broker, busname, 
           masterCommandQueueName, deadLetterQueueName,
           loop_interval, daemon)
        pass
        self._start_subprocess_called = False
        self._process_deadletter_parameters_msg_called = False

    def _start_subprocess(self):
        """
        This function hides the actual run subprocess module
        """
        self._start_subprocess_called = True

    def _process_deadletter_parameters_msg(self, unpacked_msg_content):
        """

        """
        self._process_deadletter_parameters_msg_called = True



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
        #deadletterQueue = msgbus.FromBus("testmcqdaemon/deadletter",
        #                                 broker = "locus102")
        
        #while True:
            
        #    msg = deadletterQueue.get(0.1)
        #    if msg == None:
        #        break
        #    print msg
        #    deadletterQueue.ack(msg)

        #deadletterQueue.close()
  
    def test_run_job_results_in_parameters_msg_on_bus(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """
        # Create the daemon and get all the default queues
        job_node = 'locus102'
        daemon, commandQueueBus = \
            testFunctions.prepare_test( testForwardOfJobMsgToQueueuSlaveWrapper)

        # Test1: Create a test job payload
        send_payload =  {'type': 'parameters',
                         'command':'run_job',
                         'session_uuid':"123456321654",
                         'job_uuid': "654321",
                         'node':job_node,
                          'info': {'sender': 'subprocessStarter', 'target': 'SCQLib'},
                         'parameters':{
                           'cdw': "/home",
                           'environment':  {"ENV":"Value"},
                           'cmd': "ls"}}

        msg = testFunctions.create_test_msg(send_payload)
        commandQueueBus.send(msg)

        # Run the process loop, parameters will be send to a bus adress that does
        # not exist, it should end up in the deadletter queue
        daemon._process_commands()


        # read from the deadletter queue
        msg = testFunctions.try_get_msg(daemon._deadletterFromBus, 2) 
        if msg == None:
            raise Exception(
                 "Did not receive the expect msg on the deadletter queue")
        daemon._deadletterFromBus.ack(msg) 
        send_payload['info']['subject'] ='parameters_123456321654_654321'
        # check that the correct msg is receive in the deadletter queue        
        unpacked_msg_data = eval(msg.content().payload)
        commandQueueBus.close()
        daemon.close()

        self.assertEqual(unpacked_msg_data, send_payload)
        
        

    def test_deadletterQueue_startjob_processing(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """
        # Create the daemon and get all the default queues
        job_node = 'locus102'
        daemon, commandQueueBus = \
            testFunctions.prepare_test( testForwardOfJobMsgToQueueuSlaveWrapper)


        # Test1: Create a test job payload
        send_payload =  {'type':'command',
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

        # Run the process loop, The job will be send to a bus adress that does
        # not exist, it should end up in the deadletter queue
        daemon._process_commands()

        # Run the deadletter processer
        daemon._process_deadletter_queue()
        ## Cleanup sut
        commandQueueBus.close()
        daemon.close()

        self.assertTrue(daemon._process_deadletter_parameters_msg_called)
        

    def test_start_failing_node_recipe(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """
        # Create the daemon and get all the default queues
        job_node = 'locus102'
        daemon, commandQueueBus = \
            testFunctions.prepare_test(  testForwardOfJobMsgToQueueuSlaveWrapper)

        environment = dict(
            (k, v) for (k, v) in os.environ.iteritems()
                if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
        )

        # Test1: Create a test job payload
        send_payload =  {'type':'command',
                         'command':'run_job',
                         'session_uuid':"123456321654",
                         'job_uuid': "654321",
                         'node':job_node,
                         'info':"test_start_failing_node_recipe",
                         'parameters':
                         {'node':'dop282',
                  'environment':environment,
                  'cmd': 'noexistingexecutable',
                  'cdw': '/home/wouter',
                  'job_parameters':{'par1':'par1'}}
                         
                         }

        msg = testFunctions.create_test_msg(send_payload)
        commandQueueBus.send(msg)

        # Run the process loop, The job will be send to a bus adress that does
        # not exist, it should end up in the deadletter queue
        daemon._process_commands()

        ## Run the deadletter processer
        daemon._process_deadletter_queue()
        ### Cleanup sut
        commandQueueBus.close()
        daemon.close()
        self.assertFalse(daemon._process_deadletter_parameters_msg_called)
        
        
    def test_start_node_recipe_no_connection_with_lib_2times(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        If the recipe does not receive the parameter msg it should be resend
        """
        # Create the daemon and get all the default queues
        job_node = 'locus102'
        daemon, commandQueueBus = \
            testFunctions.prepare_test(PipelineSCQDaemonImp.PipelineSCQDaemonImp)

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
        daemon._process_commands()

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

        daemon._deadletterFromBus.ack(msg)         

        commandQueueBus.close()
        daemon.close()

        self.assertTrue(eval(msg.content().payload)['n_repost'] == 2)


    def test_start_node_recipe_no_connection_with_lib_3times(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        If the recipe does not receive the parameter msg it should be resend

        after 2 attempts the job should be killed and a log and result msg
        send

        """
        # Create the daemon and get all the default queues
        job_node = 'locus102'
        daemon, commandQueueBus = \
            testFunctions.prepare_test(PipelineSCQDaemonImp.PipelineSCQDaemonImp)

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
        daemon._process_commands()

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
        daemon._deadletterFromBus.ack(msg)         
        expected_content = {'info': 'Job killed',
                            'exit_value': -1,
                            'type': 'exit_value', 
                            'uuid': '123456', 
                            'job_uuid': '654321'}
        self.assertEqual(eval(msg.content().payload), expected_content)


        commandQueueBus.close()
        daemon.close()

    def test_start_node_recipe_full(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        If the recipe does not receive the parameter msg it should be resend

        after 2 attempts the job should be killed and a log and result msg
        send

        """

        # Create the daemon and get all the default queues
        job_node = 'locus102'
        daemon, commandQueueBus = \
            testFunctions.prepare_test(PipelineSCQDaemonImp.PipelineSCQDaemonImp)

        ## Connect to the results bus for this session id
        resultQueue = msgbus.FromBus("testmcqdaemon" + "/" + "result_" + "123456",
              broker = job_node)


        environment = dict(
            (k, v) for (k, v) in os.environ.iteritems()
                if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
        )

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
        daemon._process_commands()
        time.sleep(.5)  # Allow some time for the process to start
        daemon._process_deadletter_queue()  # force resend of the parameters



        # The script should send a logline on critical
        msg = testFunctions.try_get_msg(daemon._deadletterFromBus, 2) 
        if msg == None:
            commandQueueBus.close()
            daemon.close()
            raise Exception(
                 "Did not receive the expect msg on the deadletter queue")

        daemon._deadletterFromBus.ack(msg)         
        expected_content_sub_proces_exit_value = {'level': 'CRITICAL', 
                            'sender': 'STATIC HOSTNAME', 
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
                            'uuid': '123456',
                            'job_uuid': 'test_start_node_recipe_full'} 

        # There could be two diffent msg on the results queue
        for idx in range(2):
            msg = testFunctions.try_get_msg(resultQueue, 2) 
            if msg == None:
                commandQueueBus.close()
                daemon.close()
                raise Exception(
                     "Did not receive the expect msg on the deadletter queue")

            resultQueue.ack(msg)    

            payload_parsed = eval(msg.content().payload)
            if payload_parsed == expected_content_sub_proces_exit_value:
                exit_received = True
            elif payload_parsed['output']['output'] == "OUPUT FROM TEST RECIPE":
                results_received = True
            else:
                print payload_parsed
                raise self.fail("Did not receive a valid results msg")



        commandQueueBus.close()
        resultQueue.close()
        daemon.close()





if __name__ == "__main__":
    unittest.main()
