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
        
        self._logger = logging.getLogger("CQDaemon")
        self._broker = broker
        self._busname = busname

        self._loop_interval = loop_interval  

        
        self._logger.info("Creating bus connections on broker: {0}".format(
                    self._broker))
        # Connect to the command queue
        self._logger.info("Connecting to command queue: {0}".format(
                                                             commandQueueName))
        self._CommandQueue = msgbus.FromBus(commandQueueName,
                                            broker = self._broker)
        self._logger.info("Connected")

        ## Connect to the deadletter queue
        self._logger.info("Connecting fromBus deadletter: {0}".format(
                                                          deadLetterQueueName))
        self._deadletterFromBus = msgbus.FromBus(deadLetterQueueName,
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
            # Process all incomming commands
            quit_command_received = self._process_command_queue()
            if (quit_command_received):
                self._logger.warn("Recveived quit command. stopping daemon")
                break     

            # process deadletters 
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

    def _process_command_queue(self):
        """
        Process in order all commands in the command queue

        If a quit command is received exit value is True
        else None
        """     
        while True:
            # Try to get a new msg from the command queue
            msg_available, msg_data = self._get_next_valid_msg_and_content(
                              self._CommandQueue)
            if not msg_available:
                break
                        
            (msg, unpacked_msg_content, msg_type) = msg_data
            if msg_type != "command":
                self._logger.error(
                  "Received non command msg on the command queue")
                self._logger.error(unpacked_msg_content)
                self._logger.error(msg_type)

            command = unpacked_msg_content['command']

            # First call the Subclass process command
            processed_by_subclass = self.process_command(
                    msg, unpacked_msg_content, command)

            # Try to process the command we allow
            if processed_by_subclass:
                  continue # with next msg on the queue

            elif command == 'quit':
                self._process_quit_msg(unpacked_msg_content)
                return True  

            elif command == 'echo':
                self._logger.error("Received echo command")
                self._process_echo_msg(unpacked_msg_content)

            else:
                self._logger.warn("***** encountered unknown command *****")
                self._logger.warn(unpacked_msg_content)                

        return False

    def _process_deadletter_queue(self):
        """
        Process deadletters queue

        Default behaviour is printing the content and acking the msg
        Create your own version of you want to handle the msg differently

        eg. You could not remove them from the queue but 
        do this in a different process
        """     
        while True:
            # Try to get a new msg from the deadletterbus
            msg_available, msg_data = self._get_next_valid_msg_and_content(
                              self._deadletterFromBus)

            # break on none available
            if not msg_available:
                break
            
            (msg, unpacked_msg_content, msg_type) = msg_data

            # First call the Subclass deadletter processing!!!
            processed_by_subclass = self.process_deadletter(
                    msg, unpacked_msg_content, msg_type)

            if processed_by_subclass:
                  continue      # Continue with the next msg on the queue

            # default implementation, report the deadletter and report
            self._logger.info(
               "Received on deadletterqueue type: {0}".format(msg_type))
            self._logger.info("msg content: {0}".format(unpacked_msg_content))
            self._logger.info("ignoring msg")


    def process_command(self, msg, unpacked_msg_content, command):
        """
        Abstract interface definition.

        Allows extending of the class with additional commands
        should return True of the command is processed in the called function
        The received massaged should NOT be acked!!!!
        """
        return False

    def process_deadletter(self, msg, unpacked_msg_content, type):
        """
        Abstract interface definition.

        Default return False, for we did not process the command
        """
        return False


    def _get_next_valid_msg_and_content(self, aFromBus):
        """

        """
        # Test if the timeout is in milli seconds or second
        while True:
            msg = aFromBus.get(0.1)  #  use timeout.

            if msg == None:
                return False, None

            # Get the needed information from the msg
            unpacked_msg_content, msg_type  = self._save_unpack_msg(msg)
            if not unpacked_msg_content:  # if unpacking failed
                self._logger.error(
                    "Could not process msg, incorrect content")
                self._logger.warn(msg)
                aFromBus.ack(msg) 
                continue

            aFromBus.ack(msg)
            return True, (msg, unpacked_msg_content, msg_type) 


    def _process_echo_msg(self, unpacked_msg_content):
            """
            Private function implements an echo command. Can be used to assess the
            health of a daemon.
            """
            
        #try:
            return_queue = unpacked_msg_content['return_queue']

            unpacked_msg_content['receive']=True
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
            unpacked_msg_content['type'] = 'echo'
            msg.payload = unpacked_msg_content
            with msgbus.ToBus(return_queue, broker = self._broker) as return_queue:
                return_queue.send(msg)

        # catch everything!!!
        #except Exception, ex:
        #    pass


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

        except Exception, ex:
            self._logger.warn(
                   "***** warning **** encountered incorrect structured msg:")
            self._logger.warn(msg.content())  # TODO: can we print the msg here??
            return None, None

        return msg_content, msg_type

    def _sleep(self, duration_loop_seconds):
        """
        Perform a sleep with the duration loop_interval - duration last loop
        """
        sleep_time = self._loop_interval - duration_loop_seconds

        self._logger.info("Starting sleep for {0} seconds".format(sleep_time))
        time.sleep(sleep_time)