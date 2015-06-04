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
import copy

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
                 deadLetterQueueName,
                 loop_interval=10, daemon=True):
        self._logger = logging.getLogger("MCQDaemon")

        self._broker = broker
        self._busname = busname
        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval
        self._masterCommandQueueName = masterCommandQueueName
        

        
        # Connect to the command queue
        self._CommandQueue = msgbus.FromBus(self._masterCommandQueueName,
                                            broker = self._broker)

        # Connect to bus
        self._toSlaveBus = msgbus.ToBus(self._busname, broker = self._broker)

        ## Connect to the deadletter queue
        self._toDeadletterBus = msgbus.ToBus(deadLetterQueueName,
               broker = self._broker)

    def close(self):
        """
        close all owned connections.
        """
        self._CommandQueue.close()
        self._toSlaveBus.close()
        self._toDeadletterBus.close()


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


    def _unpack_msg(self, msg):
        """
        Private helper function unpacks a received msg and casts it to 
        a msg_Content dict, the command string is also extracted
        content and command are returned as a pair
        returns None if an error was encountered
        """
        msg_content = None
        command = None
        try:
                # currently the expected payload is a dict
                msg_content = eval(msg.content().payload)

                command = msg_content['command'] 
        except:
                self._logger.warn(
                   "***** warning **** encountered incorrect structured msg:")
                self._logger.warn(msg.content())
                return None

        return (msg_content, command)

    def process_commands(self, command, unpacked_msg_content, msg):
        """
        Abstract interface definition.

        Allows extending of the class with additional commands

        should return True of the command is processed in the called function

        The received massaged should NOT be acked!!!!
        """
        raise NotImplementedError("MCQDaemon should always be inherited from")


    def _process_commands(self):
        """
        Process in order all commands in the command queue
        """     
        while True:
            # Test if the timeout is in milli seconds or second
            msg = self._CommandQueue.get(0.1)  # get is blocking, use timeout.

            if msg == None:
               break    # exit msg processing

            # Get the needed information from the msg
            unpacked_msg_data = self._unpack_msg(msg)
            if not unpacked_msg_data:
                # Forward the msg to the deadletter queue
                self._toDeadletterBus.send(msg)
                self._CommandQueue.ack(msg) 
                break

            unpacked_msg_content, command = unpacked_msg_data           

            # Send the command and msg to the process_command function
            # of the sub class
            if self.process_commands(command, unpacked_msg_content, msg):
                try:
                    self._CommandQueue.ack(msg)  
                except Exception, ex:
                    self._logger.error("Failed to ack a msg for subclass")
                    self._logger.error("Did you ack the msg yourselve?")

                continue   # command is processed by the subclass

            # Default behaviour:
            if command == 'run_job':
                
                self._process_run_job(unpacked_msg_content)               
                self._CommandQueue.ack(msg)      

            elif command == 'quit':
                self._process_quit_msg(unpacked_msg_content)
                self._CommandQueue.ack(msg)                         
                return True  

            else:
                self._logger.warn("***** warning **** encountered unknown command")
                self._logger.warn(unpacked_msg_content)
                self._toDeadletterBus.send(msg)
                self._CommandQueue.ack(msg)  # ack but not do anything

            continue
    
    def _process_quit_msg(self, msg_content):
        """
        Perform actions done on receiveing quit msg

        1. For now do nothing
        """
        pass

    def _process_run_job(self, unpacked_msg_content):
        """
        The starting of a job on one of the node servers.
        """
        node = unpacked_msg_content['node']        
        # create new msg

        msg = message.MessageContent()
        # set content
        msg.payload = unpacked_msg_content
        # set subject needed for dynamic routing
        msg.set_subject(node)

        # send to bus using the slave as msg name allows for dynamic routing
        self._toSlaveBus.send(msg)


if __name__ == "__main__":
    daemon = MCQDaemon( 1, 40)
    
    daemon.run()
        


