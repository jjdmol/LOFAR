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

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)


"""
Abstract class command queue classs containing the core functionality 
for a command queue listener

This deamon is implemented stateless: It only work when msg are received on 
its one (or two in default setting) queues: the command queue and its 
deadletter queue

1. A single commands is support (more can be user defined in subclasses)
   (a) msg with content {'command':'quit') 
   will result in the main processing loop
   to end and the owned queues to be released

2. additional command can be defined in a subclass eg::
class subclassedMCQDaemontestcommand(MCQDaemon.MCQDaemon):
    def __init__(self, *args, **kwargs):
        # call super init
        super(subclassedMCQDaemontestcommand, self).__init__(*args, **kwargs)
    
    # this is the n
    def process_commands(self, command, unpacked_msg_content, msg):
        # return false # minimal implementation no other commands
        if command == "testcommand":
            adapted_content = unpacked_msg_content
            adapted_content['testcommand']="received"

            self._process_run_job(adapted_content)
            return True

        return false


    def _process_run_job(self, adapted_content)

3. The program is typically started as a daemon: Created the object and
then call run() it will continuesly check for new msg to process.
It is possible to increase or decrease the polling interval (default is 10 sec)

WARNING: The daemon expects an instantiated bus environment.
this can be done using the script management/createbus.sh (TODO: correct name)

TODO:
    1. Initiation with config file
    2. send msg with correct LOFAR header
"""
class CQDaemon(object):
    def __init__(self, broker, busname, commandQueueName,
                 deadLetterQueueName, loop_interval=10, daemon=True):
        if type(self) == CQDaemon:
            raise NotImplementedError("CQDaemon should always be subtyped")
        
        self._logger = logging.getLogger("MCQDaemon")

        self._broker = broker

        self._busname = busname
        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval
        self.commandQueueName = commandQueueName
        self._deadLetterQueueName = deadLetterQueueName

        # Connect to the command queue
        self._logger.info("Creating bus connections on broker: {0}".format(
                    self._broker))
        self._logger.info("Connecting to command queue: {0}".format(
                        self.commandQueueName))
        self._CommandQueue = msgbus.FromBus(self.commandQueueName,
                                            broker = self._broker)
        self._logger.info("Connected")

        ## Connect tobus for the deadletter queue
        self._logger.info("Connecting toBus deadletter: {0}".format(
                        self._deadLetterQueueName))
        self._toDeadletterBus = msgbus.ToBus(self._deadLetterQueueName,
               broker = self._broker)
        self._logger.info("Connected")

        ## Connect frombus to the deadletter queue
        self._logger.info("Connecting fromBus deadletter: {0}".format(
                        self._deadLetterQueueName))
        self._deadletterFromBus = msgbus.FromBus(self._deadLetterQueueName,
               broker = self._broker)
        self._logger.info("Connected")

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.close()
        return False # 'reraises' original exception

    def close(self):
        """
        close all owned connections.
        """
        self._CommandQueue.close()
        self._toDeadletterBus.close()
        self._deadletterFromBus.close()

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

            #3. call deadletter process function with the deadletter queue
            self._process_deadletter_queue()

            #4. Do whatever internal houskeeping on a possible state
            self.process_state()

            end_tick = datetime.now()   


            # 5.  TODO: I think there is something wrong with the sleep behaviour
            # perform a sleep,
            microseconds_per_second = 10e6
            duration_loop_seconds = (end_tick - begin_tick).microseconds \
                                    / microseconds_per_second
      
            self._sleep(duration_loop_seconds)

    def _process_deadletter_queue(self):
        """
        Process deadletters queue

        Default behaviour is printing the content and acking the msg
        Create your own version of you want to handle the msg differently

        eg. You could not remove them from the queue but 
        do this in a different process
        """     
        while True:
            # Test if the timeout is in milli seconds or second
            msg = self._deadletterFromBus.get(0.1)  #  use timeout.

            if msg == None:
               break    # exit msg processing

            # Get the needed information from the msg
            unpacked_msg_data = self._unpack_msg(msg)
            if not unpacked_msg_data:  # if unpacking failed
                self._logger.error(
                    "Could not process deadletter, incorrect content")
                self._logger.warn(msg)
                self._deadletterFromBus.ack(msg) 
                continue

            # default implementation, report the deadletter and report
            unpacked_msg_content, command = unpacked_msg_data   
            self._logger.info(
               "Received on deadletterqueue command: {0}".format(command))
            self._logger.info("msg content: {0}".format(unpacked_msg_content))
            self._logger.info("ignoring msg")
            self._deadletterFromBus.ack(msg) 

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

        If a quit command is received exit value is True
        else None
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
            processed_in_subclass = True
            #try:
            processed_in_subclass = self.process_commands(
                        command, unpacked_msg_content, msg)
            ## Catch all exception, (excluding signals)
            #except Exception, ex:
            #    # We could also send to deadletter queue, but it will fail there
            #    # also i suspect
            #    self._logger.error("Processing failed in CQDaemon subclass")
            #    self._logger.error(
            #        "Deleting msg from queue and continue, error:")               
            #    self._logger.error(ex)


            if processed_in_subclass:
                try:
                    self._CommandQueue.ack(msg)  
                except Exception, ex:
                    self._logger.error("Failed to ack a msg for subclass")
                    self._logger.error("Did you ack the msg yourselve?")

                continue   # command is processed by the subclass

            elif command == 'quit':
                self._process_quit_msg(unpacked_msg_content)
                self._CommandQueue.ack(msg)                         
                return True  # TODO: test if true is returned on quit command

            else:
                self._logger.warn("***** warning **** encountered unknown command")
                self._logger.warn(unpacked_msg_content)                
                self._CommandQueue.ack(msg)  
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
                msg.payload = {"some":"content"}
                self._toDeadletterBus.send(msg) # you cannot forward a msg


    def process_state(self):
        """
        Member which allows a user of the class to perform actions based on
        internal state. This is part of the run loop

        Does nothing in the CQDaemon
        """
        pass
    
    def _process_quit_msg(self, msg_content):
        """
        Perform actions done on receiveing quit msg

        1. For now only close the connection and leave the major run loop 
        this will probably result in the end of a calling programm
        close
        """
        self.close()

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
                return (None, None)

        return (msg_content, command)

    def _sleep(self, duration_loop_seconds):
        """
        Perform a sleep with the duration loop_interval - duration last loop
        """
        sleep_time = self._loop_interval - duration_loop_seconds

        self._logger.info("Starting sleep for {0} seconds".format(sleep_time))
        time.sleep(sleep_time)