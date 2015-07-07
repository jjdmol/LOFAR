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

class SubprocessManager(object):
    """
    Class responsible for starting subprocessing.
    creating the environment

    forwarding the stout and sterror and sending the exit value back to the 
    toplevel recipe
    """

    def __init__(self, broker, busname, toBus, logger):
        self._broker = broker
        self._busname = busname
        self._toBus = toBus
        self._logger = logger
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

        # TODO: WHat happens if the same job_uuid is retrieved twice??
        if job_uuid in self._registered_sessions[session_uuid]['jobs'] :
            raise Exception("received the same Job_uuid twice, error state")
            # Alternative send a.. results msg to the logger?
            # pythonic: fail early fail hard: This is a major error state

        # to start a process we first append the job_uuid after the cmd
        # this is backward compatibility issue from the pipeline framework
        # prepending exec allows us to send a kill() to the process
        # https://stackoverflow.com/questions/4789837/how-to-terminate-a-python-subprocess-launched-with-shell-true

        
        cmd_with_uuid = "exec " + command + " " + self._busname + \
                  " " + session_uuid + " " + job_uuid 
        # new start a subprocess
        process, error_str =  self._start_subprocess(
          cmd_with_uuid, working_dir, environment)

        # if the starting of the subprocess failed, send the result to the
        # the results queue
        # else store the process
        if process == None:  # error state
            self.send_process_cout_cerr(session_uuid, job_uuid,
                                         "", error_str)
            self.send_results(session_uuid, job_uuid,"-1")
        else:                # store the process
            # Send the paramters on the parameter queue
            self.send_job_parameters(session_uuid, job_uuid, msg_content)
            self._registered_sessions[session_uuid]['jobs'][job_uuid] = \
            (process, msg_content)

    def failed_job_parameter_msg(self, msg_content):
        """
        Called by the pipeline slave daemon when a parameter msg could not 
        be delivered
        """
        job_uuid     = msg_content['job_uuid']
        session_uuid = msg_content['session_uuid']
        self.kill_job_send_results(session_uuid, job_uuid)

    def quit_session(self, msg_content):
        """
        Kills all jobs connected to a queue
        """
        session_uuid = msg_content['session_uuid']

        for job_uuid in self._registered_sessions[session_uuid]['jobs'].keys():
            self.kill_job_send_results(session_uuid, job_uuid)

        # delete the whole session_uuid dictionary 
        del self._registered_sessions[session_uuid]

    def _start_subprocess(self, command, working_dir, environment):
        """

        """
        process = None
        error_str = None
        self._logger.error(command)
        try:
            process = subprocess.Popen(
                        command,
                        cwd=working_dir,
                        env=environment, 
                        shell=True,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
            self._logger.info("Started a new job: {0}".format(command))

        except Exception, ex:
            self._logger.error("Received an command that failed in Popen:")
            self._logger.error(command)
            error_str = str(ex)
            self._logger.error(error_str)            
            process = None         # Popen failed, signal the failure of the 
                                   # command. Mostly due to file not found errors

        return process, error_str
    
    def kill_job_send_results(self,  session_uuid, job_uuid):
        """
        Helper function for killing a job. The output is send on the 
        results and log queue
        """
        # Received msg on the deadletter queue for removed sessions
        if not session_uuid in self._registered_sessions:
            return

        # Received msg on the deadletter queue for removed job
        if not job_uuid in self._registered_sessions[session_uuid]['jobs']:
            return 

        (process, job_msg) = \
                  self._registered_sessions[session_uuid]['jobs'][job_uuid]

        # kill the process // TODO: find out of kill can be called on a stopped
        # process
        process.kill()

        # Get the stout en sterr, could contain information of the reason for
        # failure
        stdoutdata, stderrdata = process.communicate()
        self.send_process_cout_cerr(session_uuid, job_uuid, 
                                     stdoutdata, stderrdata)

        # Send error results to
        self.send_results(session_uuid, job_uuid, -1, "Job killed")

        del self._registered_sessions[session_uuid]['jobs'][job_uuid]

    def send_process_cout_cerr(self, session_uuid, job_uuid,
                                stdoutdata, stderrdata):
        """
        Sends the two supplied string as log to the correct session_uuid topic
        """

        with msgbus.ToBus(self._busname + "/" + "log_" + session_uuid,
                          broker = self._broker) as logTopic:
            if stdoutdata != "":  # If there is a logline to send
                payload = {'type':'log',
                           'level':   "INFO",
                           'log_data':stdoutdata,
                           'job_uuid':job_uuid,
                           'sender': self._broker}
                msg = self.create_msg(payload)
                logTopic.send(msg)

            if stderrdata != "":  # If there is a logline to send
                payload = {'type':'log',
                           'level':   "ERROR",
                           'log_data':stderrdata,
                           'job_uuid':job_uuid,
                           'sender': self._broker}
                msg = self.create_msg(payload)
                logTopic.send(msg)


    def send_job_parameters(self, session_uuid, job_uuid, msg_content):
        """
        Sends a job parameter msg on the bus.

        Both the session and job uuid are used to adress it.
        THe session uuid allows the deadletter queue to resend it.
        """
        subject = "parameters_" + session_uuid + "_" + job_uuid
        msg_content['type'] = 'parameters'
        msg_content['info'] = {"sender":"subprocessStarter",
                               "target":"SCQLib",
                               'subject':subject}
        msg = self.create_msg(msg_content)
        
        msg.set_subject(subject)
        self._toBus.send(msg)

    def send_results(self, session_uuid, job_uuid, exit_status, info_str=""):
        """
        Send a results msg to the results queue
        """
        payload = {'type':"exit_value",
                   'exit_value':exit_status,
                   'uuid':session_uuid,
                   'job_uuid':job_uuid,
                   'info':info_str}
        msg = self.create_msg(payload)

        with msgbus.ToBus(self._busname + "/" + "result_" + session_uuid,
                          broker = self._broker) as resultQueue:
               resultQueue.send(msg)


    def check_managed_processed(self):
        """

        """

        for session_uuid  in self._registered_sessions.keys():
            jobs = self._registered_sessions[session_uuid]['jobs']

            # If no more jobs are present for this session_id remove 
            # the jobs dict
            if len(self._registered_sessions[session_uuid]['jobs']) == 0:
                del self._registered_sessions[session_uuid]
                continue

            for job_uuid in jobs.keys():    
                (process, msg) = jobs[job_uuid]

                #1. CHeck if the process has ended, continue of not
                if process.poll() == None:
                    self._logger.info("still running: {0}".format(job_uuid))
                    continue

                # 3. We have a valid subprocess that has now ended
                (stdoutdata, stderrdata) = process.communicate()
                exit_status = process.returncode

                self.send_process_cout_cerr(session_uuid, job_uuid,
                                            stdoutdata, stderrdata)
                # Send the logging information not created using the default
                # lofar logger                    
                self._logger.info("sending job result for: {0}".format(
                        session_uuid))

                # send the exit state to the resultsQueue
                payload = {'type':"exit_value",
                            'exit_value':exit_status,
                            'uuid':session_uuid,
                            'job_uuid':job_uuid}

                self.send_results(session_uuid, job_uuid, exit_status,
                                  "Subprocess Results")

                del jobs[job_uuid]

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
    