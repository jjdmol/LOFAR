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

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
import lofar.messagebus.CQDaemon as CQDaemon

# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

"""
Master command queue. Extends the command queue interface with the 
start job message and functionality
1. A single commands is support 
   (b) msg with content {'command':'run_job',
                       'node':job_node,
                       'job':{}} 
  will result in a msg being send to the command queue for job_node with the 
  copied content.

It add a bus to object where to send the job msg to
And a run_job member function performing the actual posting.

The default deadletter queue functionality is currently used (print and ignore)
"""
class MCQDaemon(CQDaemon.CQDaemon):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval=10, daemon=True):
        super(MCQDaemon, self).__init__(broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval, daemon)

        # Connect to bus ( used for sending job msg to slaves)
        self._toBus = msgbus.ToBus(self._busname, broker = self._broker)

        self._toSlaveSubjectTemplate = "slaveCommandQueue_{0}"
        self._max_repost = 5


    def close(self):
        """
        close all  connections owned by subclass
        """
        self._toBus.close()       # First our own bus
        super(MCQDaemon, self).close() # then the superclass


    def process_command(self, msg, unpacked_msg_content, command):
        """
        Process_commands, add the run_job command
        """
        # Default behaviour:
        if command == 'run_job':
            self._process_run_job(unpacked_msg_content)
            return True

        if command == "stop_session":
            self._process_stop_session(unpacked_msg_content)
            return True
  
    def process_deadletter(self, msg, unpacked_msg_content, msg_type):
        """
        Process possible deadletter
        """
        # MCQDaemon can only receive dead command msg on its dlqueue
        if not msg_type is "command":
            return False

        command = unpacked_msg_content['command']
        if command == "run_job":
            self._deadletter_process_run_job( 
                                  msg, unpacked_msg_content, msg_type)
            return True

    def _deadletter_process_run_job(self, msg, unpacked_msg_content, msg_type):
        """
        This daemon has single command it supports on this level
        When the job cannot deliver a job. Send a failure state to the sender
        """       
        if 'n_repost' in unpacked_msg_content:
            n_repost = unpacked_msg_content['n_repost'] 
        else:
            unpacked_msg_content['n_repost'] = 0
            n_repost = unpacked_msg_content['n_repost'] 

        # Test if we tried enough
        if n_repost >= self._max_repost:
            self.send_results( unpacked_msg_content, 
                     "-1", info_str="Could not deliver job to slave daemon")
        else:
            # Else resend the msg, increase the resend count
            unpacked_msg_content['n_repost'] += 1
            self._process_run_job(unpacked_msg_content)


    def _process_run_job(self, unpacked_msg_content):
        """
        The starting of a job on one of the node servers.
        """
        try:        
            node = unpacked_msg_content['node']   
            slave_commandqueue_topic_subject = \
                               self._toSlaveSubjectTemplate.format(node)
            self._logger.info("forwarding job to node: {0}".format(node))    
            # create new msg
            # TODO: FOrwarding of the received msg instead of creating a new one.
            msg = message.MessageContent()

            # set content
            unpacked_msg_content['subject'] = slave_commandqueue_topic_subject
            msg.payload = unpacked_msg_content

            # set subject needed for dynamic routing
            msg.set_subject(slave_commandqueue_topic_subject)

            # send to bus using the slave as msg name allows for dynamic routing
            self._toBus.send(msg)
        except Exception, ex:
            # Always catch all exceptions, we need to assure that the daemon
            # keeps running
            self._logger.warn(str(ex))
            self._logger.warn(unpacked_msg_content)

    def _process_stop_session(self, unpacked_msg_content):
        """
        The stop jobs on slaves
        """
        try:        
            node = unpacked_msg_content['node']   
            slave_commandqueue_topic_subject = \
                               self._toSlaveSubjectTemplate.format(node)

            self._logger.info("forwarding stop command to node: {0}".format(node))    

            # create new msg
            msg = message.MessageContent()

            # set content
            msg.payload = unpacked_msg_content
            msg.set_subject(slave_commandqueue_topic_subject)

            # send to bus using the slave as msg name allows for dynamic routing
            self._toBus.send(msg)

        except Exception, ex:
            # Always catch all exceptions, we need to assure that the daemon
            # keeps running
            self._logger.warn(str(ex))
            self._logger.warn(unpacked_msg_content)



    def send_results(self, unpacked_msg_data, 
                     exit_status, info_str=""):
        """
        Send a results msg to the results queue
        """
        payload = {'type':"exit_value",
                   'exit_value':exit_status,
                   'session_uuid':unpacked_msg_data['session_uuid'],
                   'job_uuid':unpacked_msg_data['job_uuid'],
                   'info':info_str}

        msg = self.create_msg(payload)
        msg.set_subject(unpacked_msg_data['result_topic'])
        self._toBus.send(msg)


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
    