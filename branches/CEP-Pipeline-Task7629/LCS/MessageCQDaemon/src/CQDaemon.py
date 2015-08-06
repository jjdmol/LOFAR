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
import sys
import logging
import socket

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
import lofar.messagebus.CQCommon as CQCommon

"""
Abstract class command queue classs containing the core functionality 
for a command queue listener

This deamon is implemented stateless: It only work when msg are received on 
its two queues: the command queue and its deadletter queue

1. A single commands is support (more can be user defined in subclasses)
   (a) msg with content {'command':'quit') 
   will result in the main processing loop
   to end and the owned queues to be released
   (b) msg with content echo
   Implements a basic health check: It send a responce to a supplied queue

2. additional command can be defined in a subclass eg::
class subclassedMCQDaemontestcommand(MCQDaemon.MCQDaemon):
    def __init__(self, *args, **kwargs):
        # call super init
        super(subclassedMCQDaemontestcommand, self).__init__(*args, **kwargs)
    
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
                 deadLetterQueueName, deadletterfile, 
                 logfile, loop_interval=10, daemon=True):
        """
        broker:              The broker to connect to
        busname:             The bus we are communicating on
        commandQueueName:    Name of the command queue to listen to
        deadLetterQueueName: Name of the deadletter to listen to
        loop_interval:       Max wait period between functionality loop (10sec)
        daemon:              If true run loops infinitely until stop msg is 
                             received (default = True)
        """
        if type(self) == CQDaemon:
            raise NotImplementedError("CQDaemon should always be subtyped")
        
        self._add_logger(logfile)
        self._hostname = socket.gethostname()
        self._broker = broker
        self._busname = busname
        self._loop_interval = loop_interval  
        self._daemon = daemon
        self._deadletterfile = deadletterfile

        self._CommandQueue = None
        self._deadletterFromBus = None
        self._toBus = None
        self._connect_queues( busname, commandQueueName, deadLetterQueueName)

    def run(self):
        """
        Main loop of the daemon.
        While(True)

        1. Process all incomming commands
        2. Process deadletters
        3. process state

        """
        while True:   
            begin_tick = datetime.now()

            # 1. Process all incomming commands
            quit_command_received = self._process_command_queue()           
            if (quit_command_received):
                self._logger.warn("Received quit command. stopping daemon")
                break     

            # 2. process deadletters 
            self._process_deadletter_queue()

            # 3. Do whatever internal houskeeping on a possible state
            self.process_state()

            end_tick = datetime.now()   
            self._logged_sleep(end_tick -begin_tick)

            # If we started in non deamon mode (test), do the loop only once. 
            if not self._daemon:
                break
    
    # ***********************************************************************
    # The three main functions: commands, deadletter and state
    # *********************************************************************
    def _process_command_queue(self):
        """
        Process in order all commands in the command queue

        If a quit command is received exit value is True else None
        """ 
        while True:
            # Try to get a new msg from the command queue
            msg_available, msg_data = self._get_next_msg_and_content(
              self._CommandQueue)
            if not msg_available:
                break
                        
            # Assure that it is the correct type
            (msg, unpacked_msg_content, msg_type) = msg_data
            if not msg_type is "command":
                self._logger.warn(
                  "Received non command msg on the command queue: {0}".format(
                     unpacked_msg_content))
                continue
            

            # Process the commands
            command = unpacked_msg_content['command']
            # First try the subclass
            try:
                processed_by_subclass = self.process_command(
                    msg, unpacked_msg_content, command)
            except Exception, ex:
                # Catch all: subclasses should never cause the daemon to break
                self._logger.warn(
                    "Subclass of CQDaemon threw exception in process")
                self._logger.warn(str(ex))
            if processed_by_subclass:
                  continue # with next msg on the queue


            # The commands known by CQDaemon  itselve
            if command == 'quit':
                self._process_quit_msg(unpacked_msg_content)
                return True  

            elif command == 'echo':
                self._process_echo_msg(unpacked_msg_content)

            else:
                self._logger.warn(
                  "***** encountered unknown command *****: \n{0}".format(
                    unpacked_msg_content))
                self._send_to_deadletter(msg, unpacked_msg_content)

        return None

    def _process_deadletter_queue(self):
        """
        Process deadletters queue
        """     
        # First we add a msg on the queue as a stopper:
        # We should only process the queue once per loop  
        msg = CQCommon.create_msg({"type":"deadletter_stop"})
        msg.set_subject("deadletter")
        self._toBus.send(msg)
        while True:
            # Try to get a new msg from the deadletterbus
            msg_available, msg_data = self._get_next_msg_and_content(
             self._deadletterFromBus)
            if not msg_available:
                break
            (msg, unpacked_msg_content, msg_type) = msg_data

             # we have processed the complete deadletter queue
            if msg_type == "deadletter_stop":            
                break # exit the while loop

            # Call the Subclass deadletter processing.
            processed_by_subclass = False
            try:
                
                processed_by_subclass = self.process_deadletter(
                    msg, unpacked_msg_content, msg_type)
            except Exception, ex:
                # Catch all
                self._logger.warn(
                    "Subclass of CQDaemon threw exception in process deadletter")
                self._logger.warn(str(ex))

            if processed_by_subclass:
                  continue      # Continue with the next msg on the queue

            # default implementation, report the deadletter and... ignore
            self._logger.info(
               "Ignored msg on deadletterqueue".format(unpacked_msg_content))

            self._write_to_deadletter_log(msg, unpacked_msg_content)

    def _process_state(self):
        """
        Member which allows a user of the class to perform actions based on
        internal state. This is part of the run loop
        """
        # Call a possible implementation in subclass class
        try:
            self.process_state();
        except Exception, ex:
                # Catch all
            self._logger.warn(
                  "Subclass of CQDaemon threw exception in process state")
            self._logger.warn(str(ex))
        
        # The CQDaemon does not have state yet.

    # ***********************************************************************
    # The three main functions placeholders:
    # These are typically overridden in the subclass
    # *********************************************************************
    def process_command(self, msg, unpacked_msg_content, command):
        """
        'Abstract' interface definition.

        Allows extending of the class with additional commands
        should return True of the command is processed in the called function
        The received massaged should NOT be acked!
        """
        return False

    def process_deadletter(self, msg, unpacked_msg_content, type):
        """
        'Abstract' interface definition.

        Allows extending of the class with additional commands
        should return True of the command is processed in the called function
        The received massaged should NOT be acked!
        """
        return False

    def process_state(self):
        """
        Member which allows a user of the class to perform actions based on
        internal state. This is part of the run loop

        Does nothing in the CQDaemon
        """
        pass

    # ************************************************************************
    # Members needed for the pythonic "with xx as xx:" functionality
    # **********************************************************************
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
        self._toBus.close()


    # ************************************************************************
    # Private implementation members
    # **********************************************************************
    def _add_logger(self, logfile):
        self._logger = logging.getLogger(self.__class__.__name__)

        # How to format the log msgs 
        # TODO: candidate for a config parameter
        format = "%(asctime)s %(levelname)-7s %(message)s"  #%(name)s:
        datefmt = "%Y-%m-%d %H:%M:%S"
        formatter = logging.Formatter(format, datefmt)

        # Create two handlers
        file_handler = logging.FileHandler(logfile)
        stream_handler = logging.StreamHandler(sys.stdout)
        # apply the formatters
        stream_handler.setFormatter(formatter)
        file_handler.setFormatter(formatter)

        self._logger.addHandler(file_handler)
        self._logger.addHandler(stream_handler)
        self._logger.propagate = False    # do not propagate to root logger
                                         # stops double printing to stdout  


    def _connect_queues(self, busname, commandQueueName, deadLetterQueueName):
        """
        Helper function, seperates all connections in a class. Not realy 
        reusable
        """
        self._logger.info("Creating bus connections on broker: {0}".format(
                    self._broker))
        # Connect to the command queue
        self._logger.info("Command queue: {0}".format(commandQueueName))
        self._CommandQueue = msgbus.FromBus(commandQueueName,
                                            broker = self._broker)
        self._logger.info("Connected")

        ## Connect to the deadletter queue
        self._logger.info("Deadletter bus: {0}".format(deadLetterQueueName))
        self._deadletterFromBus = msgbus.FromBus(deadLetterQueueName,
                                               broker = self._broker)
        self._logger.info("Connected")

        # Connect to the bus
        self._logger.info("Connecting to Bus: {0}".format(self._busname))
        self._toBus = msgbus.ToBus(self._busname, broker = self._broker)
        self._logger.info("Connected")

    def _process_echo_msg(self, unpacked_msg_content):
        """
        Private function implements an echo command. Can be used to assess the
        health of a daemon.
        """           
        unpacked_msg_content['receive']=True
        unpacked_msg_content['type'] = 'echo'
        unpacked_msg_content['host'] = self._hostname 

        msg = CQCommon.create_msg( unpacked_msg_content,
                        unpacked_msg_content['return_subject'])
        self._toBus.send(msg)
    
    def _process_quit_msg(self, msg_content):
        """
        Perform actions done on receiveing quit msg

        1. For now only close the connection and leave the major run loop 
        this will probably result in the end of a calling programm
        close
        """
        self.close()

    def _logged_sleep(self, duration):
        """
        Perform a sleep with the duration loop_interval - duration last loop
        """
        microseconds_per_second = 1e6
        
        duration_loop_seconds = \
            duration.microseconds / microseconds_per_second

        sleep_time = self._loop_interval - duration_loop_seconds
        self._logger.info(
            "Processing took ({0}). Sleep for {1} seconds".format(
              duration_loop_seconds, sleep_time))
        time.sleep(sleep_time)

    def _send_to_deadletter(self, msg, unpacked_msg_content):
        """
        Forward a msg to the deadletter queue, typically done when a command
        is not known and not consumed by subclasses
        """
        msg = CQCommon.create_msg(unpacked_msg_content, "deadletter")

        self._toBus.send(msg)


    def _write_to_deadletter_log(self, msg, unpacked_msg_content):
        """
        Writes the unknown deadletter to the deadletter permanent log

        TODO: Could use some cleanup

        TODO: What happens on network issues (still write to net disk?)
        """
        try:
            f = open(self._deadletterfile, "a+")  # Open for writing append create 

            subclass_name = type(self)
            current_time = datetime.now()

            f.write("START DEADLETTER\n")
            f.write("{0} {1}\n".format(current_time, subclass_name))
            f.write("{0}\n".format(str(msg.content())))
            f.write("{0}\n".format(unpacked_msg_content))
            f.write("END DEADLETTER\n")
            f.close()

        except Exception, ex:            
            # Writing to file might fail if on network disk. Catch all and
            # continue
            pass

    def _get_next_msg_and_content(self, from_bus):
            """
            Helper function attempts to get the next msg from the command queue

            Unpacks the data and assign data to the second return value.
            Returns false and none if no valid msg is available

            Invalid msg are printed to logged and stored in the deadletter log
            """
            # Test if the timeout is in milli seconds or second
            while True:
                msg = from_bus.get(0.1)  #  use timeout, very short, well get 
                # there next time if more time is needed
                if msg == None:
                    return False, None

                # Get the needed information from the msg
                unpacked_msg_content, msg_type  = _save_unpack_msg(msg,
                                                                   self._logger)
                if not msg_type:  # if unpacking failed
                    self._logger.warn(
                      "Could not process msg, incorrect content: {0}".format(
                        unpacked_msg_content))
                    from_bus.ack(msg) 
                    # send the incorrect msg to the deadletter log
                    self._write_to_deadletter_log(msg, unpacked_msg_content)
                    continue
                

                from_bus.ack(msg)

                return True, (msg, unpacked_msg_content, msg_type) 

# ****************************************************************************
# Candidate functions for external lib.
# **************************************************************************

def _save_unpack_msg(msg, logger):
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
            logger.warn("Failed evaluating msg data")
            # TODO: return 'type' is not the same in error case..
            # tight coupling with calling function.
            return msg.content().payload, False  

        return msg_content, msg_type

