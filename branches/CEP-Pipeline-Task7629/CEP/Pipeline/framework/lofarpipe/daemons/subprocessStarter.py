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

import time
import subprocess



import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message



class SubprocessStarter(object):
    """
    Class responsible for starting subprocessing.
    creating the environment

    forwarding the stout and sterror and sending the exit value back to the 
    toplevel recipe
    """

    def __init__(self, broker, busname, toBus):
        self._broker = broker
        self._busname = busname
        self._toBus = toBus
        # The main state holding object: contains session_uuid dict
        # with information about the runs and the registered pipelines
        self._registered_sessions = {} 


    def start_job_from_msg(self, msg_content):
        """
        Start a job enz enz.
        """
        session_uuid    = msg_content['session_uuid']
        job_uuid        = msg_content['job_uuid']
        working_dir     = msg_content['parameters']['cdw']
        environment     = msg_content['parameters']['environment']
        command         = msg_content['parameters']['cmd'] 

        # First test if we have retrieved job for this session id. If not
        # create the needed queues and add to the registered sessions
        if not session_uuid in self._registered_sessions:
            # Create the session dict, with all need content
            self._registered_sessions[session_uuid] = {}
            self._registered_sessions[session_uuid]['jobs'] = {}

            # create the queues
            queues = self._connect_result_log_parameter_queues(session_uuid)
            self._registered_sessions[session_uuid]['queues'] = queues

        # TODO: WHat happens if the same job_uuid is retrieved twice??
        if job_uuid in self._registered_sessions[session_uuid]['jobs'] :
            raise Exception("received the same Job_uuid twice, error state")
            # Alternative send a.. results msg to the logger?
            # pythonic: fail early fail hard: This is a major error state

        # The needed queues are present, the job is unique. Send the paramters
        # on the parameter queue
        self._send_job_parameters(job_uuid, msg_content)

        # to start a process we first append the job_uuid after the cmd
        # this is backward compatibility issue from the pipeline framework
        append_cmd = command + " " + job_uuid
        # new start a subprocess
        process, error_str =  self._start_subprocess(
          command, working_dir, environment)
        # if the starting of the subprocess failed, send the result to the
        # the results queue
        # else store the process

        if process == None:  # error state
            self._send_job_start_failed_msg(job_uuid, error_str)
        else:                # store the process
            self._registered_sessions[session_uuid]['jobs'][job_uuid] = process

    def failed_job_parameter_msg(self, msg_content):
        """
        Called by the pipeline slave daemon when a parameter msg could not 
        be delivered
        """
        job_uuid     = msg_content['job_uuid']
        session_uuid = msg_content['session_uuid']
        self._kill_job_send_results(session_uuid, job_uuid)


    def quit_session(self, msg_content):
        """
        Kills all jobs connected to a queue
        """
        session_uuid = msg_content['session_uuid']

        for job_uuid in self._registered_sessions[session_uuid]['jobs'].keys():
            self._kill_job_send_results(session_uuid, job_uuid)

        # delete the whole session_uuid dictionary 
        del self._registered_sessions[session_uuid]


    def _start_subprocess(self, command, working_dir, environment):
        """

        """
        process = None
        error_str = None
        try:
            process = subprocess.Popen(
                        command,
                        cwd=working_dir,
                        env=environment, 
                        shell=True,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
            self.logger.info("Started a new job: {0}".format(command))

        except Exception, ex:
            self.logger.error("Received an command that failed in Popen:")
            self.logger.error(command)
            error_str = str(ex)
            self.logger.error(error_str)            
            process = None         # Popen failed, signal the failure of the 
                                   # command. Mostly due to file not found errors

        return process, error_str
    

    def _kill_job_send_results(self,  session_uuid, job_uuid):
        """
        Helper function for killing a job. The output is send on the 
        results and log queue
        """
        process = self._registered_sessions[session_uuid]['jobs'][job_uuid]

        # kill the process
        process.kill()
        # Get the stout en sterr, could contain information of the reason for
        # failure
        stdoutdata, stderrdata = process.communicate()
        self._send_process_cout_cerr(session_uuid, job_uuid, 
                                     stdoutdata, stderrdata)

        # Send error results to
        self._send_results(session_uuid, job_uuid, -1)

    def _connect_result_log_parameter_queues(self, session_uuid):
        """
        Creates a temporary named result queue and a topic based on the uuid
        Return the create queue and topic name

        Both the queue and topic are created durable, they stay even when 
        there are no listeners, altough not expected. The start of the session
        might arrive before the pipeline actually connected to the queueus
        """

        # now register to the queues, remember, the pipeline might not have
        # connected so create and make durable. Deleting is done by the 
        # master Daemon
        resultQueue = msgbus.ToBus(busname + "/" + "result_" + session_uuid,
              broker = self._broker)

        # and topic s: qpid-stat -e for example and source of this code
        logTopic = msgbus.ToBus(busname + "/" + "log_" + session_uuid,
              broker = self._broker)

        queues_dict = {'result': resultQueue,
                       'log': logTopic}

        return queues_dict


    def _send_process_cout_cerr(self, session_uuid, job_uuid,
                                stdoutdata, stderrdata):
        """
        Sends the two supplied string as log to the correct session_uuid topic
        """

        payload = {'level':   "INFO",
                   'log_data':stdoutdata,
                   'job_uuid':job_uuid}
        msg = self.create_msg(payload)
        self._registered_sessions[session_uuid]['queues']['log'].send(msg)

        payload = {'level':   "ERROR",
                   'log_data':stderrdata,
                   'job_uuid':job_uuid}
        msg = self.create_msg(payload)
        self._registered_sessions[session_uuid]['queues']['log'].send(msg)



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
    