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
import lofar.messagebus.CQCommon as CQCommon

import lofarpipe.daemons.subprocessManager as subprocessManager


class PipelineSCQDaemonImp(CQDaemon.CQDaemon):
    """
    Pipeline Slave deamon, start and stops subprocess received as jobs
    State is stored in the subprocess manager

    Accepted command:
       run_job: Start a subprocess

       stop_session: Kills all jobs part of the suplied session

    Processed deadletters:
       parameters: The started subprocess did not retrieve its paramters
                    Effect: kill the job
       exit_value or
          output: The results send by the job are not retrieved by the pipeline
                  Effect: Kill all jobs of this session, none will deliver data

    """
    def __init__(self, 
                 broker, 
                 busname, 
                 commandQueueName,
                 deadLetterQueueName, 
                 deadletterfile,
                 logfile ,
                 loop_interval=10, 
                 daemon=True,
                 max_repost=5):
        super(PipelineSCQDaemonImp, self).__init__(
                   broker,
                   busname, 
                   commandQueueName,
                   deadLetterQueueName, 
                   deadletterfile,
                   logfile,
                   loop_interval, 
                   daemon)
        # Object responcible for starting jobs, contains the state 
        self._subprocessManager = subprocessManager.SubprocessManager(
               self._broker, self._busname, self._toBus, self._logger)

        self._max_repost = max_repost # We attempt 3 times to resend a msg

    # ****************************************************************
    # The three overload process functions: command, deadletter and state
    # ****************************************************************
    def process_command(self, msg, unpacked_msg_content, command):
        """
        Process_commands, add the run_job command
        """               
        if command == 'run_job':
            self._process_run_job(unpacked_msg_content)
            return True

        if command == 'stop_session':
            self._process_stop_session(unpacked_msg_content)
            return True

        return False

    def process_deadletter(self, msg, unpacked_msg_content, msg_type):
        """
        Process deadletters queue msg
        """     
        if msg_type == 'parameters':
            self._process_deadletter_parameters_msg(unpacked_msg_content)
            return True

        if msg_type == "exit_value" or msg_type == 'output':
            subject = msg.getSubject()
            self._process_deadletter_exit_or_output_msg(unpacked_msg_content,
                                                            subject)
            return True

        # Not processed  gerereturn False
        return False

    def process_state(self):
        """
        Called by superclass run loop. Performs all the state based 
        functionality
        """
        # All the state is stored in the subprocess manager
        self._subprocessManager.check_managed_processed()


    # ****************************************************************
    # Private helper functions
    # ****************************************************************
    def _process_run_job(self, unpacked_msg_content):
        """
        The starting of a job on one of the node servers.
        """
        # Forward the content
        self._subprocessManager.start_job_from_msg(unpacked_msg_content)

    def _process_stop_session(self, unpacked_msg_content):
        """
        Forward the stop session to the subprocess manager
        """
        self._subprocessManager.quit_session(
                          unpacked_msg_content['session_uuid'])
   
    def _process_deadletter_exit_or_output_msg(self, unpacked_msg_data,
                                           subject):
        """
        We try to send a parameter msg a couple of times. After it still fails,
        send results msg to the results q of the session
        """
        n_repost = self._deadletter_get_or_set_nrepost(unpacked_msg_data)       
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
            msg = CQCommon.create_msg(unpacked_msg_data, subject)
            self._toBus.send(msg)      

    def _process_deadletter_parameters_msg(self, unpacked_msg_data):
        """
        We try to send a parameter msg a couple of times. After it still fails,
        send results msg to the results q of the session
        """
        session_uuid = unpacked_msg_data['session_uuid']
        job_uuid = unpacked_msg_data['job_uuid']

        n_repost = self._deadletter_get_or_set_nrepost(unpacked_msg_data)        
        if n_repost >= self._max_repost:
            # If we tried enough times, kill the job
            self._logger.warn(
                  "SubProcess did not retrieve parameters:\n {0}".format(
                     unpacked_msg_data))
            self._subprocessManager.kill_job_send_results(
              session_uuid, job_uuid)
        else:
            # Else resend the msg, increase the resend count
            unpacked_msg_data['n_repost'] += 1
            self._subprocessManager.send_job_parameters(
               session_uuid, job_uuid, unpacked_msg_data)
        
    def _deadletter_get_or_set_nrepost(self, unpacked_msg_data):
        """
        Small helper function. Searches for n_repost entry in the unpacked msg
        data. Sets it to zero of not found.
        Return the n_repost
        """
        n_repost = None
        if 'n_repost' in unpacked_msg_data:
            n_repost = unpacked_msg_data['n_repost'] 
        else:
            n_repost = unpacked_msg_data['n_repost'] = 0

        return n_repost

