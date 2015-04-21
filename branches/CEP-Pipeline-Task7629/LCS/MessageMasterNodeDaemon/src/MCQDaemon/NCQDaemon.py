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
# id.. TDB

from datetime import datetime   # needed for duration
import time
import os
import subprocess

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

class NCQDaemon(object):
    def __init__(self,  loop_interval=10):
        self.logger = logging.getLogger("NCQDaemon")
        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval

        self._registered_pipelines = {'jobs':[]} 

        self._broker = "127.0.0.1" 
        self._returnQueueTemplate = "MCQDaemon.return.{0}"
        self._logTopicTemplete = "MCQDaemon.log.{0}"
        #TODO: THe name should be depending on the locus node currently running
        # maybee the hostname should be used?
        # TOdo: this name is static!!
        self._CommandQueue = msgbus.FromBus("username.locus102.NCQueueDaemon.CommandQueue", 
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)

    def run(self):
      """
      Main loop of the daemon.
      While(True)

      1. Check all the 'connected' pipeline session ques for listeners
        a. Clear queues with no listeners
      2. Process all incomming commands
      3. Wait for x seconds

      """
      while(True):   
          begin_tick = datetime.now()
          # 1.  Process the stored work items
          self._process_registered_sessions()

          # 2.  Process all incomming commands
          quit_command_received = self._process_commands()
          if (quit_command_received):
              self.logger.warn("Recveived quit command. stopping daemon")
              break     
          end_tick = datetime.now()   

          # 3.  perform a sleep,
          microseconds_per_second = 10e6
          duration_loop_seconds = (end_tick - begin_tick).microseconds \
                                  / microseconds_per_second
      
          self._sleep(duration_loop_seconds)

    def _sleep(self, duration_loop_seconds):
        """
        Perform a sleep with the duration loop_interval - duration last loop
        """
        sleep_time = self._loop_interval - duration_loop_seconds

        self.logger.info("Starting sleep for {0} seconds".format(sleep_time))
        time.sleep(sleep_time)

    def _process_commands(self):
        """
        Process in order all commands in the command queue
        """     
        while True:
            # Test if the timeout is in milli seconds or second
            msg = self._CommandQueue.get(0.1)  # get is blocking, always use timeout.
            if msg == None:
               break    # Break the loop, we will wait on a other location



            # currently the expected payload is a list
            msg_content = eval(msg.content().payload)
            self.logger.info("-----------------------------------------")
            self.logger.info(msg_content)
            # now process the commands
            command = msg_content['command'] 
            if command == "start_session":
                self._process_start_session_msg(msg_content)
                self._CommandQueue.ack(msg)

            elif command == "stop_session":
                self._process_stop_msg(msg_content)
                self._CommandQueue.ack(msg)       
                
                 
            elif command == 'run_job':
                try:
                    self._process_start_job(msg_content)

                except:
                    self.logger.info("received an invalid job msg:")
                    self.logger.info(msg_content)

                self._CommandQueue.ack(msg)      

            elif command == 'quit':
                self._process_quit_msg(msg_content)
                self._CommandQueue.ack(msg)                         
                return True  # do NOT save the current state, might be cleared due to
              # this command

            else:
                self.logger.warn("***** warning **** encountered unknown command")
                self.logger.warn(msg_content)

        
    def _process_start_job(self, msg_content):
        """

        """
        self.logger.info("WOOOOOT WE have received a msg")
        self.logger.info(msg_content)

        command     = msg_content['parameters']['cmd']
        working_dir = msg_content['parameters']['cdw']

        uuid = msg_content['uuid']

        # this should not happen but check anyways.
        if uuid not in self._registered_pipelines:
            self.logger.warn("------------------Major error -----------")
            self.logger.warn("A job was requested on this deamon for an unknown")
            self.logger.warn("Pipeline id. There is a sync error between the")
            self.logger.warn("Master and Node Daemon.")
            self.logger.warn(uuid)
            self.logger.warn(self._registered_pipelines)
            self.logger.warn("------------------Major error -----------")

        # Run subprocess
        process = subprocess.Popen(
                        command,
                        cwd=working_dir,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

        # store the now created job in the list of jobs for this pipeline
        self._registered_pipelines['jobs'].append(process)



    def _process_quit_msg(self, msg_content):
        """
        Perform actions done on receiveing quit msg

        1. If clear_state is set the state_file is removed
        """
        try:
            # forward a quit to all jobs
            pass

        except:
            # The daemon could be in a state where the file has not been written
            # eg.  in the init phase.  If the delete fails just skip and continue
            pass

    def _process_start_session_msg(self, msg_content):
        """
        _process_start_session_msg called when a new session is requested.
        It is responsible to for storing the details of the queue names
        in the internal storage of the Deamon object
        """      
        self.logger.info("*****************")
        self.logger.info("We got a new session started: {0}".format(msg_content['uuid']))
        self.logger.info("*****************")
        resultq_name, topic_name = self._connect_resultq_and_topic(
                                                           msg_content['uuid'])

        # store them in the internal pipeline storage
        self._registered_pipelines[msg_content['uuid']] = {
                  'resultq':resultq_name,
                  'topic':topic_name}


    def _connect_resultq_and_topic(self, uuid):
        """
        Creates a temporary named result queue and a topic based on the uuid
        Return the create queue and topic name

        Both the queue and topic are created durable, they stay even when 
        there are no listeners
        """
        # create the queues names based on the template
        resultq_name = self._returnQueueTemplate.format(uuid)
        topic_name = self._logTopicTemplete.format(uuid)

        # TODO: Is this needed on the node daemon?
        # now register the queue
        resultQueue = msgbus.ToBus(resultq_name, 
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)

        # and topic s: qpid-stat -e for example and source of this code
        logTopic = msgbus.ToBus(topic_name, 
              options = "create:always, node: { type: topic, durable: True}",
              broker = self._broker)

        return resultq_name, topic_name
  
    def _process_stop_msg(self, msg):
        """
         Stop the current session.
        """
        # Send stop msg the subprocesses that are part of this run 
        # TODO: No runs have been added yet. So do nothing for now


        # After killing the jobs. Remove the uuid from the internal storage
        self._delete_queues_and_session(msg['uuid'])

    def _process_registered_sessions(self):
        """
        process the internally stored session items.

        This is basically the work that needs to be done for all the
        pipelines that are registed at this daemon. 
        """
        # Check the current running state:

        # If finished send return value to the MasterDaemon

        #perform needed cleanup of the state

        pass


if __name__ == "__main__":
    daemon = NCQDaemon(1)
    
    daemon.run()
        

