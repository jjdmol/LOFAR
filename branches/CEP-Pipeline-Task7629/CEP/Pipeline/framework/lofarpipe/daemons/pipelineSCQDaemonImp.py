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


class PipelineSCQDaemon(CQDaemon.CQDaemon):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval=10, daemon=True):
        super(PipelineSCQDaemon, self).__init__(broker, busname, masterCommandQueueName,
                 deadLetterQueueName, loop_interval, daemon)
        pass

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
        # create new msg
        # TODO: FOrwarding of the received msg instead of creating a new one.
        msg = message.MessageContent()
        # set content
        msg.payload = unpacked_msg_content
        # set subject needed for dynamic routing
        msg.set_subject(node)


        # Start a subprocess
        self._start_subprocess()


    def _start_subprocess(self):
        """
        Called when a subprocess should be started.
        TODO: better description
        """
        pass

