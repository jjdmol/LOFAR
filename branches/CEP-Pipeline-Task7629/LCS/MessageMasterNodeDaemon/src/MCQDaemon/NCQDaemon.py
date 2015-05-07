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
    def __init__(self,  loop_interval=10, clearCOmmandQueue=True):
        self.logger = logging.getLogger("NCQDaemon")
        self._hostname = socket.gethostname()
        self._username = pwd.getpwuid(os.getuid()).pw_name

        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval

        self._broker = "127.0.0.1" 
        self._returnQueueTemplate = "MCQDaemon.{0}.return.{1}" # they are owned by
        self._logTopicTemplate = "MCQDaemon.{0}.log.{1}"       # the master daemon
        self._parameterQueueTemplate ="NCQDaemon.{0}.parameters.{1}" #Node daemon

        # create a NON durable queue: We cannot have old msg hanging in this
        # command queue
        self._CommandQueue = msgbus.FromBus(
              "{0}.{1}.NCQueueDaemon.CommandQueue".format(self._username,
                                                               self._hostname), 
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)

        # receive and clear command queue on init once.
        if clearCOmmandQueue:
            while True:
                msg = self._CommandQueue.get(0.1)
                if msg == None:
                    break
                self._CommandQueue.ack(msg) 

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
          begin_tick = datetime.now()
          # 1.  Process the stored work items
          self._process_registered_sessions()

          # 2.  Process all incomming commands
          quit_command_received = self._process_commands()
          if quit_command_received:
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
        self.logger.info("NCQDaemon: Starting sleep for {0} seconds".format(
                                                                  sleep_time))
        time.sleep(sleep_time)

    def _process_commands(self):
        """
        Process in order all commands in the command queue
        """     
        while True:
            # Test if the timeout is in milli seconds or second
            msg = self._CommandQueue.get(0.1)  # blocking, use timeout.
            if msg == None:
               break    # We wait on a other location

            # currently the expected payload is a dict
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
                    self._process_start_job(msg_content)       
                    self._CommandQueue.ack(msg)     

            elif command == 'quit':
                self._process_quit_msg(msg_content)
                self._CommandQueue.ack(msg)                         
                return True  # do NOT save the current state, might be
                             # cleared due to this command

            else:
                self.logger.warn(
                  "NCQDaemon: ***** warning **** encountered unknown command")
                self.logger.warn(msg_content)


    def _unknown_uuid_msg(self, uuid, keys):
        """
        print log msg with detailed state information, used when unknown uuid
        is encountered
        """
        self.logger.warn("------------------Major error -----------")
        self.logger.warn("A job was requested on this deamon for an unknown")
        self.logger.warn("Pipeline id. There is a sync error between the")
        self.logger.warn("Master and Node Daemon.")
        self.logger.warn('uuid:')
        self.logger.warn(uuid)
        self.logger.warn('keys()')
        self.logger.warn(self._registered_pipelines.keys())
        self.logger.warn("------------------Major error -----------")
        
    def _process_start_job(self, msg_content):
        """

        """
        uuid = msg_content['uuid']

        # this should not happen but check anyways, does the received uuid
        # exist?
        if uuid not in self._registered_pipelines.keys():
            self._unknown_uuid_msg(uuid, self._registered_pipelines.keys())
            return


        command     = msg_content['parameters']['cmd']
        # Append the the command with the parameter queue name!
        command += " {0}".format(self._registered_pipelines[uuid]['parameterq'][0])
        working_dir = msg_content['parameters']['cdw']
        environment = msg_content['parameters']['environment']
        parameter_dict = msg_content['parameters']['job_parameters']
        job_uuid = msg_content['job_uuid']



        # Run subprocess
        process = None
        try:
            # First send the a parameter msg on the queue
           
            self._send_job_parameter_message(
                  self._registered_pipelines[uuid]['parameterq'][1],
                  msg_content)
            time.sleep(1)   # Otherwise the msg is not yet evailable on the other
                            # side
            process = subprocess.Popen(
                        command,
                        cwd=working_dir,
                        env=environment,               # where to get this?
                        shell=True,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
        except Exception, ex:
            self.logger.error("Received an command that failed in subprocesses:")
            self.logger.error(command)
            self.logger.error(str(ex))
            raise ex    # exit, do not store the job


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
        (resultq_name, resultQueue), (topic_name, logTopic), (parameterq_name, 
                                                           parameterQueue) = \
            self._connect_resultq_and_topic(msg_content['uuid'])

        # store them in the internal pipeline storage
        self._registered_pipelines[msg_content['uuid']] = {
                  'resultq':(resultq_name, resultQueue),
                  'topic':(topic_name, logTopic),
                  'parameterq':(parameterq_name, parameterQueue),
                  'jobs':{}}
        #self.logger.error(self._registered_pipelines[msg_content['uuid']])


    def _connect_resultq_and_topic(self, uuid):
        """
        Creates a temporary named result queue and a topic based on the uuid
        Return the create queue and topic name

        Both the queue and topic are created durable, they stay even when 
        there are no listeners, altough not expected. The start of the session
        might arrive before the pipeline actually connected to the queueus
        """
        # create the queues names based on the template
        resultq_name = self._returnQueueTemplate.format(self._username, uuid)
        topic_name = self._logTopicTemplate.format(self._username, uuid)
        parameterq_name = self._parameterQueueTemplate.format(self._username, uuid)

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

        parameterQueue = msgbus.ToBus(parameterq_name, 
              options = "create:always, delete:always, node: { type: queue, durable: False}",
              broker = self._broker)   # Created NON durable: delete when not
                                       # needed anymore

        return (resultq_name, resultQueue), (topic_name, logTopic), \
               (parameterq_name, parameterQueue)
  
    def _process_stop_msg(self, msg):
        """
         Stop the current session.
        """
        uuid = msg['uuid']
        self.logger.info("Received stop command for uuid: {0}".format(uuid))
        # Send stop msg the subprocesses that are part of this run 
        # TODO: Still te be implemented
        # After killing the jobs. Remove the uuid from the internal storage
        self._kill_session_jobs(uuid)


    def _kill_session_jobs(self, uuid):
        """

        """
        # first check if the the queues might have been removed:
        # dueue to the nature of message, the delete might be called a second time
        if uuid not in self._registered_pipelines.keys():
            return
        

        for job_uuid in self._registered_pipelines[uuid]['jobs'].keys():
            (process, msg_content) = self._registered_pipelines[uuid][
                                                             'jobs'][job_uuid]

            # first kill the child:
            process.terminate()
            # delete the job entry
            del self._registered_pipelines[uuid]['jobs'][job_uuid]

        # close the the results queue (owned by us)
        self._registered_pipelines[uuid]['parameterq'][1].close()
        self._registered_pipelines[uuid]['resultq'][1].close()
        self._registered_pipelines[uuid]['topic'][1].close()

        # delete the session entry
        del self._registered_pipelines[uuid]


    def _send_log_message(self, logTopic, log_data, level='info'):
        """
        Send a logging msg  with log_data to the logTOpic at the level

        msg_details:
        {'level'=level, 'log_data':log_data}
        """
        
        msg = message.MessageContent(
                from_="{0}.{1}.NCQDaemon".format(
                        self._username, self._hostname),
                forUser="{0}.MSQDaemon".format(self._username),
                summary="NCQDaemon log message",
                protocol="CommandQUeueLogMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        msg.payload = {'level':   level,
                       'log_data':log_data}

        #if not logTopic.connection.check_closed():
        #    # we cannot send to non existing queue, so.. return
        #    return
        logTopic.send(msg)


    # TODO: HERE is something wrong
    def _send_job_exit_message(self, resultQueue, exit_dict):
        """
        Send a job exit information msg

        msg_details:
                    {'type':'exit_value'
                     'exit_value':exit_status,
                     'uuid':uuid,
                     'job_uuid':msg_content['job_uuid']}
        """
        msg = message.MessageContent(
                from_="{0}.{1}.NCQDaemon".format(
                        self._username, self._hostname),
                forUser="{0}.MSQDaemon".format(self._username),
                summary="NCQDaemon job exit message",
                protocol="CommandQUeueExitMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        msg.payload = exit_dict
        resultQueue.send(msg)  

    def _send_job_parameter_message(self, parameterQueue, job_dict):
        """
        Send a job exit information msg

        msg_details:

        """
        msg = message.MessageContent(
                from_="{0}.{1}.NCQDaemon".format(
                        self._username, self._hostname),
                forUser="{0}.NSQLib".format(self._username),
                summary="NCQDaemon job parameter message",
                protocol="NCQLib",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        msg.payload = job_dict
        parameterQueue.send(msg)  
           

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
                    self.logger.error("still running: {0}".format(job_uuid))
                    continue

                (stdoutdata, stderrdata) = process.communicate()
                exit_status = process.returncode

                # Send the logging information not created using the default
                # lofar logger
                self._send_log_message(logTopic, stdoutdata, level='info')
                self._send_log_message(logTopic, stderrdata, level='error')

                # send the exit state to the resultsQueue
                payload = {'type':"exit_value",
                           'exit_value':exit_status,
                           'uuid':uuid,
                           'job_uuid':msg_content['job_uuid']}

                self._send_job_exit_message(resultQueue, payload )
                self.logger.error(payload)

                del session_dict['jobs'][job_uuid]


if __name__ == "__main__":
    daemon = NCQDaemon(1)
    
    daemon.run()
        

