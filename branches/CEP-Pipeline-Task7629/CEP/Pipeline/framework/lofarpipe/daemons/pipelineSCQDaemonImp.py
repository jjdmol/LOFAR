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

import lofar.messagebus.CQDaemon as CQDaemon
import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

import subprocess


class PipelineSCQDaemonImp(CQDaemon.CQDaemon):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName, subprocessStartedExec,
                 loop_interval=10, daemon=True):
        super(PipelineSCQDaemonImp, self).__init__(
                 broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval, daemon)

        # we for ward jobs to the generic bus
        self._toBus = msgbus.ToBus(self._busname, broker = self._broker)

        # If we discover that there is no consumer, create a subprocess
        # with a consumer. this is all the state in this class
        self._subprocessStartedExec = subprocessStartedExec
        self._job_starter_subprocess = None

    def process_commands(self, command, unpacked_msg_content, msg):
        """
        Process_commands, add the run_job command
        """
                
        # Default behaviour:
        if command == 'run_job':
            self._process_run_job(unpacked_msg_content)
            return True

        return False
  
    def _process_run_job(self, unpacked_msg_content):
        """
        The starting of a job on one of the node servers.
        """
        node = unpacked_msg_content['node']        
        session_uuid = unpacked_msg_content['session_uuid']
        # create new msg
        # TODO: FOrwarding of the received msg instead of creating a new one.
        msg = message.MessageContent()
        # set content
        msg.payload = unpacked_msg_content
        # set subject needed for dynamic routing
        subject = session_uuid + '_' + node
        msg.set_subject(subject)
        
        # send to bus using the slave as msg name allows for dynamic routing
        self._toBus.send(msg)


    def _process_deadletter_queue(self):
        """
        Process deadletters queue

        TODO: This function still feels clutchy. Might be refactored
        """     
        while True:
            # Test if the timeout is in milli seconds or second
            msg = self._deadletterFromBus.get(0.1)  #  use timeout.

            if msg == None:
               break    # exit msg processing

            # Get the needed information from the msg
            unpacked_msg_data, command = self._unpack_msg(msg)
            if not unpacked_msg_data:  # if unpacking failed
                self._logger.error(
                    "Could not process deadletter, incorrect content")
                self._logger.warn(msg)
                self._deadletterFromBus.ack(msg) 
                break

            elif command == 'run_job':
                self._process_deadletter_run_job(unpacked_msg_data)
                self._deadletterFromBus.ack(msg)                         
                continue

            self._logger.info(
               "Received on deadletterqueue command: {0}".format(command))
            self._logger.info("msg content: {0}".format(unpacked_msg_content))
            self._logger.info("ignoring msg")
            self._deadletterFromBus.ack(msg) 
    
    def _process_deadletter_run_job(self, unpacked_msg_content):
        """
        Called when a run_job msg ends up in the dead letter queue

        This means that we have to start a subprocess which is able to receive
        jobs to start.
        """

        self._job_starter_subprocess = subprocess
        node = unpacked_msg_content['node']        
        session_uuid = unpacked_msg_content['session_uuid']
        queuename = self._busname + " /" + node + "." + session_uuid
