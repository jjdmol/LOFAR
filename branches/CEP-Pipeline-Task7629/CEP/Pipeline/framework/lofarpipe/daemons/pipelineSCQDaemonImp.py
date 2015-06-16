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
import lofarpipe.daemons.subprocessStarter as subprocessStarter


class PipelineSCQDaemonImp(CQDaemon.CQDaemon):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName, subprocessStartedExec,
                 loop_interval=10, daemon=True):
        super(PipelineSCQDaemonImp, self).__init__(
                 broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval, daemon)

        # we for ward jobs to the generic bus
        self._toBus = msgbus.ToBus(self._busname, broker = self._broker)


        self._subprocessStarter = subprocessStarter.SubprocessStarter(
               self._broker, self._busname, self._toBus, self._logger)



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
        self._subprocessStarter.start_job_from_msg(unpacked_msg_content)

    def _process_deadletter_queue(self):
        """
        Process deadletters queue

        TODO: This function still feels clutchy. Might be refactored
        """     
        while True:
            msg = self._deadletterFromBus.get(0.1)  #  use timeout.
            if msg == None:
               break    # exit msg processing

            # Get the needed information from the msg
            unpacked_msg_data, msg_type  = self._save_unpack_msg(msg)           
            if not unpacked_msg_data:  # if unpacking did not work
                self._logger.error(
                    "Could not process deadletter, incorrect content")
                self._logger.warn(msg)
                self._deadletterFromBus.ack(msg) 
                continue
            
            self._logger.error(unpacked_msg_data)
            # Select what to do based on the msg type
            if msg_type == 'command':
                raise Exception(
                    "Major error, why is a command send on this bus?") # TODO: 
                self._process_deadletter_parameters_msg(unpacked_msg_data)
                self._deadletterFromBus.ack(msg) 
                continue
            elif msg_type == 'parameters':
                self._process_deadletter_parameters_msg(unpacked_msg_data)
                self._deadletterFromBus.ack(msg) 
                continue


            self._logger.info(
               "Received a unknown msg on deadletterqueue")
            self._logger.info("msg content: {0}".format(unpacked_msg_data))
            self._logger.info("ignoring msg")
            self._deadletterFromBus.ack(msg) 


    def _process_deadletter_parameters_msg(self, unpacked_msg_data):
        pass
        #node = unpacked_msg_content['node']        
        #session_uuid = unpacked_msg_content['session_uuid']
        #queuename = self._busname + " /" + node + "." + session_uuid


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
