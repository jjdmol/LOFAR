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
import pwd
import subprocess
import copy
import socket

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

class NCQDaemon(object):
    def __init__(self,  loop_interval=10):
        self.logger = logging.getLogger("NCQDaemon")
        self._hostname = socket.gethostname()
        self._username = pwd.getpwuid(os.getuid()).pw_name

        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval

        self._broker = "127.0.0.1" 
        self._returnQueueTemplate = "MCQDaemon.return.{0}"  # they are owned by
        self._logTopicTemplete = "MCQDaemon.log.{0}"        # the master daemon

        # create a NON durable queue: We cannot have old msg hanging in this
        # command queue
        self._CommandQueue = msgbus.FromBus(
              "username.{0}.NCQueueDaemon.CommandQueue".format(self._hostname), 
              options = "create:always, node: { type: queue, durable: False}",
              broker = self._broker)

        # The main state holding object: contains session_uuid dict
        # with information about the runs and the registered pipelines
        self._registered_pipelines = {} 

    def run(self):
      """
      Main loop of the daemon.
      while(True):
        1. For all sessions check if the jobs are done
        2. Process all incomming commands
        3. Wait for x seconds

      """
      while(True):   
          print self._registered_pipelines
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
            # now process the commands
            command = msg_content['command'] 
            if command == "start_session":
                self._process_start_session_msg(msg_content)
                self._CommandQueue.ack(msg)

            elif command == "stop_session":
                self._process_stop_msg(msg_content)
                self._CommandQueue.ack(msg)       
                
                 
            elif command == 'run_job':
                #try:
                    self._process_start_job(msg_content)

                ##except Exception, ex:
                #    self.logger.info("received an invalid job msg:")
                #    self.logger.info(msg_content)
                    
                
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
        environment = msg_content['parameters']['environment']
        uuid = msg_content['uuid']
        job_uuid = msg_content['job_uuid']

        # this should not happen but check anyways.
        if uuid not in self._registered_pipelines.keys():
            self.logger.warn("------------------Major error -----------")
            self.logger.warn("A job was requested on this deamon for an unknown")
            self.logger.warn("Pipeline id. There is a sync error between the")
            self.logger.warn("Master and Node Daemon.")
            self.logger.warn('uuid:')
            self.logger.warn(uuid)
            self.logger.warn('keys()')
            self.logger.warn(self._registered_pipelines.keys())
            self.logger.warn("------------------Major error -----------")
            return

        # Run subprocess
        process = None
        try:
            process = subprocess.Popen(
                        command,
                        cwd=working_dir,
                        env=environment,               # where to get this?
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
        except Exception, ex:
            self.logger.error("Received an command that failed in subprocesses:")
            self.logger.error(command)
            self.logger.error(str(ex))
            return    # exit, do not store the job


        # store the now created job in the list of jobs for this pipeline
        self._registered_pipelines[uuid]['jobs'][job_uuid] = (process, 
                                                  copy.deepcopy(msg_content))

        self.logger.info("Started a new job: {0}".format(command))



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
        self.logger.info("New (pipeline) session started: {0}".format(
                                                          msg_content['uuid']))
        # Connect to the queue and topic, use the uuid of the session
        (resultq_name, resultQueue), (topic_name, logTopic) = \
            self._connect_resultq_and_topic(msg_content['uuid'])

        # store them in the internal pipeline storage
        self._registered_pipelines[msg_content['uuid']] = {
                  'resultq':(resultq_name, resultQueue),
                  'topic':(topic_name, logTopic),
                  'jobs':{}}


    def _connect_resultq_and_topic(self, uuid):
        """
        Creates a temporary named result queue and a topic based on the uuid
        Return the create queue and topic name

        Both the queue and topic are created durable, they stay even when 
        there are no listeners, altough not expected. The start of the session
        might arrive before the pipeline actually connected to the queueus
        """
        # create the queues names based on the template
        resultq_name = self._returnQueueTemplate.format(uuid)
        topic_name = self._logTopicTemplete.format(uuid)

        # now register to the queues, remember, the pipeline might not have
        # connected so create and make durable. Deleting is done by the 
        # master Daemon
        resultQueue = msgbus.ToBus(resultq_name, 
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)

        # and topic s: qpid-stat -e for example and source of this code
        logTopic = msgbus.ToBus(topic_name, 
              options = "create:always, node: { type: topic, durable: True}",
              broker = self._broker)

        return (resultq_name, resultQueue), (topic_name, logTopic)
  
    def _process_stop_msg(self, msg):
        """
         Stop the current session.
        """
        # Send stop msg the subprocesses that are part of this run 
        # TODO: Still te be implemented
        # After killing the jobs. Remove the uuid from the internal storage
        self._kill_session_jobs(msg['uuid'])


    def _kill_session_jobs(self, uuid):
        """

        """
        # first check if the the queues might have been removed:
        # dueue to the nature of message, the delete might be called a second time
        if uuid not in self._registered_pipelines.keys():
            return
        

        for job_uuid in self._registered_pipelines[uuid]['jobs'].keys():
            (process, msg_content) = session_dict['jobs'][job_uuid]

            # first kill the child:
            process.terminate()
            # delete the job entry
            del self._registered_pipelines[uuid]['jobs'][job_uuid]

        # delete the session entry
        del self._registered_pipelines[uuid]


    def _process_registered_sessions(self):
        """
        process the internally stored session items.

        This is basically the work that needs to be done for all the
        pipelines that are registed at this daemon. 
        """
        # For all sessions
        for uuid  in self._registered_pipelines.keys():
            session_dict = self._registered_pipelines[uuid]
            (topicName, logTopic) = session_dict['topic']
            (queueName, resultQueue) = session_dict['resultq']
            # for all jobs in this session
            for job_uuid in session_dict['jobs'].keys():            
                (process, msg_content) = session_dict['jobs'][job_uuid]
                # CHeck if the process has ended, continue of not
                if process.poll() == None:
                    continue

                (stdoutdata, stderrdata) = process.communicate()
                exit_status = process.returncode

                

                msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemonLib",
                forUser="USERRNAME.LOCUS102.MSQDaemon",
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
                msg.payload = stdoutdata
                logTopic.send(msg)

                msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemonLib",
                forUser="USERRNAME.LOCUS102.MSQDaemon",
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
                msg.payload = stderrdata
                logTopic.send(msg)



                msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemonLib",
                forUser="USERRNAME.LOCUS102.MSQDaemon",
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
                msg.payload = {'exit_value':exit_status,
                               'uuid':uuid,
                               'job_uuid':msg_content['job_uuid']}

                logTopic.send(msg)

                del session_dict['jobs'][job_uuid]


if __name__ == "__main__":
    daemon = NCQDaemon(1)
    
    daemon.run()
        

