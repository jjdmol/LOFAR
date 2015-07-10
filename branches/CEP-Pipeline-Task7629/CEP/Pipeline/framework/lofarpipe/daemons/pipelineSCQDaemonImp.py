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

import lofarpipe.daemons.subprocessManager as subprocessManager


class PipelineSCQDaemonImp(CQDaemon.CQDaemon):
    """

    """
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName, subprocessStartedExec,
                 loop_interval=10, daemon=True):
        super(PipelineSCQDaemonImp, self).__init__(
                 broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval, daemon)
        # we for ward jobs to the generic bus
        self._toBus = msgbus.ToBus(self._busname, broker = self._broker)

        # Object responcible for starting jobs, contains the state 
        self._subprocessManager = subprocessManager.SubprocessManager(
               self._broker, self._busname, self._toBus, self._logger)

        self._max_repost = 3 # We attempt 3 times to resend a msg

    def process_command(self, msg, unpacked_msg_content, command):
        """
        Process_commands, add the run_job command
        """               
        if command == 'run_job':
            self._process_run_job(unpacked_msg_content)
            return True

        return False

    def process_state(self):
        """
        Called by superclass run loop. Performs all the state based 
        functionality
        """
        # All the state is stored in the subprocess manager
        self._subprocessManager.check_managed_processed()
  
    def _process_run_job(self, unpacked_msg_content):
        """
        The starting of a job on one of the node servers.
        """
        # Forward the content
        self._subprocessManager.start_job_from_msg(unpacked_msg_content)

    def _process_deadletter_queue(self):
        """
        Process deadletters queue

        TODO: This function still feels clutchy. Might be refactored
        """     
        # First we add a msg on the queue as a stopper:
        # We should only process the queue once.  
        msg = self.create_msg({"type":"deadletter_stop"})
        msg.set_subject("deadletter")
        self._toBus.send(msg)

        # Now read msg from the deadletter queue
        while True:   # The loop exits when a deadletter_stop msg is received
            msg = self._deadletterFromBus.get(0.1)  #  use timeout.
            if msg == None:
               break    # exit loop, should not happen but to be sure

            # Get the needed information from the msg
            unpacked_msg_data, msg_type  = self._save_unpack_msg(msg)           
            self._deadletterFromBus.ack(msg)

            if not unpacked_msg_data:  # if unpacking did not work
                self._logger.error("unknown msg on deadletter bus:")
                self._logger.warn(msg)                
                continue

            # Select what to do based on the msg type
            if msg_type == "deadletter_stop":
                # we have processed the complete deadletter queue
                break # exit the while loop
            
            elif msg_type == 'parameters':
                self._process_deadletter_parameters_msg(unpacked_msg_data)
                continue

            elif msg_type == "exit_value" or msg_type == 'output':
                subject = msg.getSubject()
                self._process_deadletter_exit_or_output_msg(unpacked_msg_data,
                                                            subject)
                continue

            self._logger.info("Received a unknown msg on deadletterqueue:")
            self._logger.info("msg content: {0}".format(unpacked_msg_data))
            self._logger.info("ignoring msg")

    def _process_deadletter_exit_or_output_msg(self, unpacked_msg_data,
                                           subject):
        """
        We try to send a parameter msg a couple of times. After it still fails,
        send results msg to the results q of the session
        """
        n_repost = None

        if 'n_repost' in unpacked_msg_data:
            n_repost = unpacked_msg_data['n_repost'] 
        else:
            unpacked_msg_data['n_repost'] = 0
            n_repost = unpacked_msg_data['n_repost'] 
        
        if n_repost >= self._max_repost:
            # If we tried enough times, kill the job
            self._logger.warn(
              "Failed deliver results or exit msg to" 
              " pipeline, assume its down")
            session_uuid = unpacked_msg_data['session_uuid']

            self._subprocessManager.quit_session(
              session_uuid, send_cout=False)
        else:
            # Else resend the msg, increase the resend count
            unpacked_msg_data['n_repost'] += 1
            msg = self.create_msg(unpacked_msg_data, subject)
            self._toBus.send(msg)
        
        return

    def _process_deadletter_parameters_msg(self, unpacked_msg_data):
        """
        We try to send a parameter msg a couple of times. After it still fails,
        send results msg to the results q of the session
        """
        session_uuid = unpacked_msg_data['session_uuid']
        job_uuid = unpacked_msg_data['job_uuid']
        n_repost = None

        if 'n_repost' in unpacked_msg_data:
            n_repost = unpacked_msg_data['n_repost'] 
        else:
            unpacked_msg_data['n_repost'] = 0
            n_repost = unpacked_msg_data['n_repost'] 
        
        if n_repost >= self._max_repost:
            # If we tried enough times, kill the job
            self._logger.warn(
                  "SubProcess did not retrieve parameter msg:\n {0}".format(
                     unpacked_msg_data))
            self._subprocessManager.kill_job_send_results(
              session_uuid, job_uuid)
        else:
            # Else resend the msg, increase the resend count
            unpacked_msg_data['n_repost'] += 1
            self._subprocessManager.send_job_parameters(
               session_uuid, job_uuid, unpacked_msg_data)
        
        return

    def _save_unpack_msg(self, msg):
        """
        Private helper function unpacks a received msg and casts it to 
        a msg_Content dict, the command string is also extracted
        content and command are returned as a pair
        returns None if an error was encountered
        """
        msg_content = None
        msg_type = None
        try:
                # currently the expected payload is a dict
                msg_content = eval(msg.content().payload)
                msg_type =  msg_content['type']
        except:
                return None, None

        return msg_content, msg_type

    def create_msg(self, payload, subject=None):
        """
        TODO: should be moved into a shared code lib
        Creates a minimal valid msg with payload
        """
        msg = message.MessageContent(
                    from_="test",
                    forUser="",
                    summary="summary",
                    protocol="protocol",
                    protocolVersion="test")
        msg.payload = payload

        if subject:
            msg.set_subject(subject)

        return msg