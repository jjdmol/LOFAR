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

from datetime import datetime   # needed for duration
import time

import os

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

# programmatically interact with the qpid broker
from qpid.messaging import Connection	
from qpidtoollibs import BrokerAgent

# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)


"""
Abstract class containing the core functionality of a master slave structure

Listens on a queue, send jobs to slaves.
Declares a 'abstract' process dead letter function. throwing a not implemented
exception, when called. This functionality should be added to the

subclass

"""
class MCQDaemon(object):
    def __init__(self, broker, busname, masterCommandQueueName,
                 loop_interval=10, daemon=True):
        self._logger = logging.getLogger("MCQDaemon")
        self._broker = broker
        self._masterCommandQueueName = masterCommandQueueName
        self._busname = busname

        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval
        
        # Connect to the command queue
        self._CommandQueue = msgbus.FromBus(
                   self._masterCommandQueueName,
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)

        self._daemon = daemon

        # Connect to bus
        self._toBus = msgbus.ToBus(
                   self._busname,
              broker = self._broker)

        # Connect to the deadletter queue
                # Connect to bus
        #self._toBus = msgbus.ToBus(
        #           self._busname,
        #      options = "create:always, node: { type: queue, durable: True}",
        #      broker = self._broker)


    def run(self):
      """
      Main loop of the daemon.
      While(True)

      1. Check all the 'connected' pipeline session ques for listeners
        a. Clear queues with no listeners
      2. Process all incomming commands
      3. Call the process deadletter msg function with 
      3. Wait for x seconds

      """
      while(True):   
          begin_tick = datetime.now()
          # 1.  check all registered pipeline queues for disconnects

          # 2.  Process all incomming commands
          quit_command_received = self._process_commands()
          if (quit_command_received):
              self._logger.warn("Recveived quit command. stopping daemon")
              break     
          end_tick = datetime.now()   

          #3. call deadletter process function with the deadletter queue


          # 4.  perform a sleep,
          microseconds_per_second = 10e6
          duration_loop_seconds = (end_tick - begin_tick).microseconds \
                                  / microseconds_per_second
      
          self._sleep(duration_loop_seconds)

          if not self._daemon:
              break


    def _process_commands(self):
        """
        Process in order all commands in the command queue
        """     
        while True:
            # Test if the timeout is in milli seconds or second
            msg = self._CommandQueue.get(0.1)  # get is blocking, use timeout.
            if msg == None:
               break    # exit msg processing

            msg_content = None
            command     = None
            try:
                # currently the expected payload is a dict
                msg_content = eval(msg.content().payload)

                # now process the commands
                command = msg_content['command'] 
            except:
                self._logger.warn(
                   "***** warning **** encountered incorrect structure msg")
                self._logger.error(msg.content())
                self._CommandQueue.ack(msg)  
                continue


            if command == 'run_job':
                self._process_run_job(msg_content)
                self._CommandQueue.ack(msg)      

            elif command == 'quit':
                self._process_quit_msg(msg_content)
                self._CommandQueue.ack(msg)                         
                return True  # do NOT save the current state, might be cleared due to
              # this command

            else:
                self._logger.warn("***** warning **** encountered unknown command")
                self._logger.warn(msg_content)
                self._CommandQueue.ack(msg)  # ack but not do anything


    
    def _process_quit_msg(self, msg_content):
        """
        Perform actions done on receiveing quit msg

        1. For now do nothing
        """
        pass

    def _process_run_job(self, msg_content):
        """
        The starting of a job on one of the node servers.
        """


        # extract the job parameters from the msg
        # This information is need to perform local work and to know where
        # to send the information 
        node = msg_content['node']

        msg = message.MessageContent(
                from_="test",
                forUser="MCQDaemon",
                summary="summary",
                protocol="protocol",
                protocolVersion="test", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        msg.payload = msg_content
        msg.set_subject(node)
        print "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
        print msg_content
        print msg.payload
        self._toBus.send(msg)

        print "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"


        #        raise Exception("Received job msg")

        # send job on to the slave

 


if __name__ == "__main__":
    daemon = MCQDaemon( 1, 40)
    
    daemon.run()
        


