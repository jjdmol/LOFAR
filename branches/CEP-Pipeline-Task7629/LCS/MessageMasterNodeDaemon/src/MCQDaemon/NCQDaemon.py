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
import copy


import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
import lofar.messagebus.CQConfig as CQConfig


# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

class NCQDaemon(object):
    """
    NCQDaemon listens to a command queue and is responcible for starting
    commands received in this queue on the local host in a sub process.

    Started jobs have access to a parameter queue on which they the 
    actual paraemters for the job (limiting the command line arguments)

    The jobs are part of a session which is started based on a command.

    Available commands:
    1. Start session based on a uuid, received from the outside world
       Creates connection to session specic log and return queue
    2. Stop session, eg. when the master requests this. Kills the jobs,
       send a report of this on the log and result queu
    3. start job, based on a job_uuid is related to a session uuid
       Create a unique parameter queue and forwards this to the job together
       with the log and results queue
    4. Quit. Stop all running session, reporting this on the logqueues
       stops the daemon

    The Daemon does store its state between instantations. And by default
    the command queue is cleared at construction
    """
    def __init__(self,  loop_interval=10, clearCOmmandQueue=True):
        """
        Init function.
        Creates the logger
        Creates the connection to the commandQueue
        Clears depending on argument the waiting msgs.
        """
        self.logger = logging.getLogger("NCQDaemon")
        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval
        self._broker = CQConfig.broker 

        # Connect to the command queue
        self._CommandQueue = msgbus.FromBus(
              CQConfig.create_nodeCommandQueue_name(), 
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)

        # receive and clear command queue on init. We do not process 'old' msg
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
        1. For all sessions (active pipelines) check if there is work to do
        2. Process all incomming commands
        3. Wait for x seconds (depending on the polling rate)

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
                uuid = msg_content['uuid']
                self._stop_session(uuid)
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
                  "NCQDaemon: ***** warning **** encountered unknown command," 
                  "ignoring msg")
                self.logger.warn(msg_content)
                self._CommandQueue.ack(msg)    


    def _process_start_job(self, msg_content):
        """
        Performs the work done on receiving a valid start job msg
        """
        uuid = msg_content['uuid']   # All work centers around the uuid for 
                                     # the pipeline

        # this should not happen but check anyways, does the received uuid
        # exist? A job msg should always follow after a start session msg
        if uuid not in self._registered_pipelines.keys():
            self._unknown_uuid_msg(uuid, self._registered_pipelines.keys())
            return

        # Get all the variables needed to start the job
        command     = msg_content['parameters']['cmd']    # Command line cmd 

                          # Add the parameter queue to the command, used
                          # by the pipeline script to receive the parameters
        command += " {0}".format(self._registered_pipelines[uuid]['parameterq'][0])
        working_dir = msg_content['parameters']['cdw']
        environment = msg_content['parameters']['environment']
        parameter_dict = msg_content['parameters']['job_parameters']
        job_uuid = msg_content['job_uuid']

        # First send the a parameter msg on the parameter queue
        self._send_job_parameter_message(
                  self._registered_pipelines[uuid]['parameterq'][1],
                  msg_content)

        # Run subprocess
        process = None
        try:
            process = subprocess.Popen(
                        command,
                        cwd=working_dir,
                        env=environment, 
                        shell=True,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
            self.logger.info("Started a new job: {0}".format(command))

        except Exception, ex:
            self.logger.error("Received an command that failed in Popen:")
            self.logger.error(command)
            self.logger.error(str(ex))
            process = None         # Popen failed, signal the failure of the 
                                   # command. Mostly due to file not found errors
            
        # store the now created job in the list of jobs for this pipeline
        self._registered_pipelines[uuid]['jobs'][job_uuid] = (process, 
                                                  copy.deepcopy(msg_content))       

    def _process_quit_msg(self, msg_content):
        """
        Perform actions done on receiveing quit msg: Loop all registered 
        sessions, kill the subprocess cleanly (with msg on the log)
        and thats it.
        """
        for uuid in self._registered_pipelines.keys():
            try:            
                self._stop_session(uuid)
            except:
                # We tried a clean kill, continue
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
        queues_dict = self._connect_result_log_parameter_queues(msg_content['uuid'])
        queues_dict['jobs'] = {}

        # store them in the internal state
        self._registered_pipelines[msg_content['uuid']] = queues_dict

    def _process_registered_sessions(self):
        """
        process the internally stored session items.

        This is basically the work that needs to be done for all the
        pipelines that are registed at this daemon. 

        1. If the starting of a job failed in the int state (Popen) send
           a failed job_exit msg. Remove job from internal state

        2. Continue if the job is running

        3. If the job is done, communicate with the subprocess streams and
           forward the exit state to the master. Remove job from internal state
        """
        # For all sessions
        for uuid  in self._registered_pipelines.keys():
            session_dict = self._registered_pipelines[uuid]
            (topicName, logTopic) = session_dict['topic']
            (queueName, resultQueue) = session_dict['resultq']

            # for all jobs in this session (typically one, but future proof)
            for job_uuid in session_dict['jobs'].keys():            
                (process, msg_content) = session_dict['jobs'][job_uuid]

                # 1. If process is None, the Popen command failed
                if process is None:
                     # send the exit state to the resultsQueue
                     payload = {'type':"exit_value",
                           'exit_value':-1,
                           'uuid':uuid,
                           'job_uuid':msg_content['job_uuid']}

                     self._send_job_exit_message(resultQueue, payload )
                     del session_dict['jobs'][job_uuid]  # remove from state
                     continue


                # 2. CHeck if the process has ended, continue of not
                if process.poll() == None:
                    self.logger.error("still running: {0}".format(job_uuid))
                    continue

                # 3. We have a valid subprocess that has now ended
                (stdoutdata, stderrdata) = process.communicate()
                exit_status = process.returncode

                # Send the logging information not created using the default
                # lofar logger
                self._send_log_message(logTopic, stdoutdata, level='INFO')
                self._send_log_message(logTopic, stderrdata, level='ERROR')

                # send the exit state to the resultsQueue
                payload = {'type':"exit_value",
                           'exit_value':exit_status,
                           'uuid':uuid,
                           'job_uuid':msg_content['job_uuid']}

                self._send_job_exit_message(resultQueue, payload )

                del session_dict['jobs'][job_uuid]

    def _connect_result_log_parameter_queues(self, uuid):
        """
        Creates a temporary named result queue and a topic based on the uuid
        Return the create queue and topic name

        Both the queue and topic are created durable, they stay even when 
        there are no listeners, altough not expected. The start of the session
        might arrive before the pipeline actually connected to the queueus
        """
        # create the queues names based on the template
        resultq_name = CQConfig.create_returnQueue_name(uuid)
        topic_name = CQConfig.create_logTopic_name(uuid)
        parameterq_name = CQConfig.create_parameterQueue_name(uuid)

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

        queues_dict = {'resultq':(resultq_name, resultQueue),
                       'topic':(topic_name, logTopic),
                       'parameterq':(parameterq_name, parameterQueue)}

        return queues_dict
  
    def _stop_session(self, uuid):
        """
         Stop the current session, registered on this uuid
        """
        self.logger.info("Received stop command for uuid: {0}".format(uuid))
        # Send stop msg the subprocesses that are part of this run 
        # After killing the jobs. Remove the uuid from the internal storage
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

        # close the queues
        self._registered_pipelines[uuid]['parameterq'][1].close()
        self._registered_pipelines[uuid]['resultq'][1].close()
        self._registered_pipelines[uuid]['topic'][1].close()

        # delete the session entry
        del self._registered_pipelines[uuid]

    def _sleep(self, duration_loop_seconds):
        """
        Perform a sleep with the duration loop_interval - duration last loop
        """
        sleep_time = self._loop_interval - duration_loop_seconds
        self.logger.info("NCQDaemon: Starting sleep for {0} seconds".format(
                                                                  sleep_time))
        time.sleep(sleep_time)
          
    # ************************************************************************
    # Set of easy pretty print functions sending information to the diverse 
    # queues
    def _send_log_message(self, logTopic, log_data, level='INFO'):
        """
        Send a logging msg  with log_data to the logTOpic at the level
        """
        msg = CQConfig.create_validated_log_msg(level, 
                               log_data,"NCQDaemon:" + CQConfig.hostname)
        logTopic.send(msg)

    def _send_job_exit_message(self, resultQueue, exit_dict):
        """
        Send a job exit information msg
        """
        msg = CQConfig.create_validated_return_msg(exit_dict, "NCQDaemon")
        resultQueue.send(msg)  

    def _send_job_parameter_message(self, parameterQueue, job_dict):
        """
        Send a job exit information msg
        """

        msg = CQConfig.create_validated_parameter_msg(job_dict, "NCQDaemon")
        parameterQueue.send(msg)  

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
        


if __name__ == "__main__":
    daemon = NCQDaemon(1)   # TODO: change to slower heart beat?
    
    daemon.run()
    