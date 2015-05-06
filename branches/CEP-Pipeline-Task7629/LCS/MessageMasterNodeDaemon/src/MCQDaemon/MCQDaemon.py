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
import pickle
import os
import uuid
import pwd
import socket

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

# programmatically interact with the qpid broker
from qpid.messaging import Connection	
from qpidtoollibs import BrokerAgent


# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

class MCQDaemon(object):
    def __init__(self, loop_interval=10, state_file_path=None, init_delay=5):
        self._hostname = socket.gethostname()
        self._username = pwd.getpwuid(os.getuid()).pw_name

        self.logger = logging.getLogger("MCQDaemon")
        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval
     
        self._registered_pipelines = {}          # dict of uuid ->(ResultQName,
                                                 #                 LogTopic)
        self._session_queueus = None             # will contain a current
                                                 # dict of sessions and queueus
        self._broker = "127.0.0.1" 
        self._returnQueueTemplate = "MCQDaemon.{0}.return.{1}"
        self._logTopicTemplate = "MCQDaemon.{0}.log.{1}"
        self._nodeCommandQueueTemplate = "{0}.{1}.NCQueueDaemon.CommandQueue"


        self._CommandQueue = msgbus.FromBus(
              "{0}.{1}.MCQueueDaemon.CommandQueue".format(self._username,
                                                          self._hostname), 
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)


        self._init_delay = init_delay        # how many loop polls te wait 
        # check for existing statefile
        # before it is asumed that the pipeline that called init has died before
        # registering as a consumer
        self._state_file_path = state_file_path  # where to store the statefile
        self._init_state_from_file_if_possible()

        self._connection = Connection.establish(self._broker)
        self._brokerAgent = BrokerAgent(self._connection)

        # A dictionary with node to queue names, used for state validation 
        # and queue retrieval of queue names
        self._registered_nodes = {}

        self._registered_nodes_queues = {}


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
          # 1.  check all registered pipeline queues for disconnects
          self._get_check_and_clear_known_queues()

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
                self.logger.warn("We received a stop_session command!!!")
                self._process_stop_msg(msg_content)
                self._CommandQueue.ack(msg)        

            elif command == 'run_job':
                self._process_start_job(msg_content)
                self._CommandQueue.ack(msg)      

            elif command == 'quit':
                self._process_quit_msg(msg_content)
                self._CommandQueue.ack(msg)                         
                return True  # do NOT save the current state, might be cleared due to
              # this command

            else:
                self.logger.warn("***** warning **** encountered unknown command")
                self.logger.warn(msg_content)

            # After each command we need to save the new state.
            # If the deamon goes down between the processing of the command
            # and this save we have an indetermined state.
            self._save_state_to_file()
        
    def _process_quit_msg(self, msg_content):
        """
        Perform actions done on receiveing quit msg

        1. If clear_state is set the state_file is removed
        """
        try:
            if msg_content['clear_state'] == "true":
                os.remove(self._state_file_path)

        except:
            # The daemon could be in a state where the file has not been written
            # eg.  in the init phase.  If the delete fails just skip and continue
            pass


    def _check_slave_and_connect(self, node):
        """
        Checks if a node is connect and created a msg queue connect if needed
        """
        if node in self._registered_nodes.keys():
            nodeQueueName = self._registered_nodes[node]['CQName']
        else:
            self.logger.debug(
                "received job for unconnected slave: {0}".format(node))

            # TODO: Het maken van deze queueu moet ergens anders
            nodeQueueName = self._nodeCommandQueueTemplate.format(
                                                  self._username, node)
            self._registered_nodes[node]={'CQName':nodeQueueName}
            self._registered_nodes_queues[node] = msgbus.ToBus(nodeQueueName, 
                options = "create:always, node: { type: queue, durable: True}",
                broker = self._broker)


    def _check_session_and_start(self, node, uuid):
        """
        Checks if a node is connect and created a msg queue connect if needed
        """
        if uuid not in self._registered_pipelines.keys():
            return
        if node not in self._registered_pipelines[uuid]['session_nodes']:
            # Send the node the start_session command
            msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemon",
                forUser="USERRNAME.{0}.NSQDaemon".format(node),
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
            start_msg_content = {'command': 'start_session',
                                 'uuid':uuid}
            msg.payload = start_msg_content
            self.logger.info("Starting node session on: {0}".format(node))
            self._registered_nodes_queues[node].send(msg)
            self._registered_pipelines[uuid]['session_nodes'].append(node)


    def _process_start_job(self, msg_content):
        """
        The meat of the Master node Daemon functionality.

        The starting of a job on one of the node servers.
        """
        # extract the job parameters from the msg
        # Should be stored in the internal storag2
        uuid = msg_content['uuid']
        node = msg_content['parameters']['node']

        self._check_slave_and_connect(node)

        self._check_session_and_start(node, uuid)

        msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemon",
                forUser="USERRNAME.{0}.NSQDaemon".format(node),
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
        msg.payload = msg_content
        self.logger.info("send job to: {0}".format(
                                self._registered_nodes[node]['CQName']))

        self._registered_nodes_queues[node].send(msg)

    

    def _process_start_session_msg(self, msg_content):
        """
        _process_start_session_msg called when a new session is requested.
        This function creates the needed queue and topic based on the uuid.
        It is also responsible to for storing the details of the created objects
        in the internal storage of the Deamon object
        """      
        uuid = msg_content['uuid']
        self.logger.info("started session with uuid: {0}".format(uuid))
        # create the needed topic and resultq
        session_queue_dict = self._create_resultq_and_topic(uuid)
    
        # store them in the internal pipeline storage
        self._registered_pipelines[uuid] = {
                  'resultq':session_queue_dict['queue_name'],
                  'topic':session_queue_dict['topic_name'],
                  'init_wait':self._init_delay,
                  'session_nodes':[]}
        # init_wait:other option is a time out
        # the problem is that the pipeline can only connect to an existing q
        # so after creating the q is not emediately 'used'.
        # its either a wait or a state or we have a countdown to allow late
        #connections

        self._session_queueus[uuid] = session_queue_dict


    def _create_resultq_and_topic(self, uuid):
        """
        Creates a temporary named result queue and a topic based on the uuid
        Return the create queue and topic name

        Both the queue and topic are created durable, they stay even when 
        there are no listeners
        """
        # create the queues names based on the template
        resultq_name = self._returnQueueTemplate.format(self._username, uuid)
        topic_name = self._logTopicTemplate.format(self._username, uuid)

        # now register the queue
        resultQueue = msgbus.ToBus(resultq_name, 
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)

        # and topic s: qpid-stat -e for example and source of this code
        logTopic = msgbus.ToBus(topic_name, 
              options = "create:always, node: { type: topic, durable: True}",
              broker = self._broker)

        session_queue_dict = {
                    'queue_name':resultq_name,
                    'queue_object':resultQueue,
                    'topic_name':topic_name,
                    'topic_object':logTopic}

        return session_queue_dict
  
    def _process_stop_msg(self, msg_content):
        """
        Stop the current session.
        """
        
        self.logger.info("deleting session with uuid: {0}".format(uuid))

        self._delete_queues_and_session(msg_content['uuid'])

    def _save_state_to_file(self):
        """
        Save the internal session information to disk     
        """      
        file = open(self._state_file_path, 'w')

        pickle.dump(self._registered_pipelines, file)

        file.flush()  # force write to disk
        file.close()


    def _get_queue_and_topic_from_broker(self):
        """
        Returns a dictionary uuid to queue and topic names and objects

        For this a connection is made with the broker.
        All queues and topics are retrieved.
        These are then matched with the internally stored session uuid
        and packages in a easy dict structure

        items in dict: 'queue_name', 'queue_object', 'topic_name', 'topic_object'
    
        throws Exception if the queue or topic for a stored session uuid is not 
        found 
        """
        # TODO: Warning!!!!  With a large amount if queues and topic this might
        # get slow!!!!!

        # First get al the registered queues and topics on the broker
        queues = self._brokerAgent.getAllQueues()
        topics = self._brokerAgent.getAllExchanges()



        # Covert to name to queue object lists
        raw_name_to_queue_dict = {}
        for queue in queues:
            raw_name_to_queue_dict[queue.name] = queue

        raw_name_to_topic_dict = {}
        for topic in topics:
            raw_name_to_topic_dict[topic.name] = topic

        # Loop all the session uuid on the broker, find the accompanying topic or
        # queue on the broker
        uuid_dict = {}
        for uuid in self._registered_pipelines.keys():
            
            session_dict = {}
            # first match the uuid with the queue
            queue_found = False
            for name, queue in raw_name_to_queue_dict.items():               
                if uuid in name and "return" in name:
                    session_dict['queue_name'] = name
                    session_dict['queue_object'] = queue
                    queue_found = True
                    break

            if not queue_found:
                # Could not find a registered session as queue on the broker
                raise Exception("Could not find a registered pipeline session queue on"
                              " the broker, the internal daemon state is corrupt!")

            # next the topic
            topic_found = False
            for name, topic in  raw_name_to_topic_dict.items():
                if uuid in name:
                     session_dict['topic_name'] = name
                     session_dict['topic_object'] = topic
                     topic_found = True
                     break
            if not topic_found:
                # Could not find a registered session as topic on the broker
                raise Exception("Could not find a registered pipeline session topic on"
                                " the broker, the internal daemon state is corrupt!")

            # Now store all our collected information in an easy dict
            uuid_dict[uuid] = session_dict

        return uuid_dict

    def _get_check_and_clear_known_queues(self):
        """
        Check all registered session queue for listeners
        If this is zero, the queues will not be read anymore and the process died

        Clear the messages in the queues and delete these
        """
        self._session_queueus = self._get_queue_and_topic_from_broker()

        for (uuid, session_dict) in self._session_queueus.items():
            # A queue might be in the init period (a grace period which allows an
            # pipelie some time to connect to the bus before the queues are
            # removed)
            if self._registered_pipelines[uuid]["init_wait"] > 0:
                self._registered_pipelines[uuid]["init_wait"] -= 1  
                continue
      
            # If the number of listeners is 0 there is no consumer of data anymore.
            # The results in the queue have no place to be stored.
            nr_listeners = session_dict['queue_object'].consumerCount
            # TODO / BUG: The consumerCount is NOT CORRECT!!!
            if (nr_listeners == 0):
                self.logger.error("Number of listeners: {0}".format(nr_listeners))
                self._delete_queues_and_session(uuid)
                self._save_state_to_file()

    def _delete_queues_and_session(self, uuid):
        """
        Deletes the queue and topic for this uuid and removes it from the
        internal storage
        """
        # first check if the the queues might have been removed:
        # dueue to the nature of message, the delete might be called a second time
        if uuid not in self._registered_pipelines.keys():
            return

        # send the quit msg to NodeDaemons
        for node in self._registered_nodes_queues.keys():
            msg = message.MessageContent(
                from_="USERNAME.LOCUS102.MCQDaemon",
                forUser="USERRNAME.{0}.NSQDaemon".format(node),
                summary="First msg to be send",
                protocol="CommandQUeueMsg",
                protocolVersion="0.0.1", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
            start_msg_content = {'command': 'stop_session',
                             'uuid':uuid}
            msg.payload = start_msg_content
            self.logger.info("sending stop_session to nodes")
            self._registered_nodes_queues[node].send(msg)

        # remove the q and topic
        qname = self._registered_pipelines[uuid]['resultq']
        topicname = self._registered_pipelines[uuid]['topic']
 
        self.logger.info("Deleted queues for session uuid: {0}".format(uuid))
        # delete the entry from the actual dict
        del self._session_queueus[uuid]
        del self._registered_pipelines[uuid]

        self._delete_queue_and_topic(qname, topicname)

        

    def _delete_queue_and_topic(self, qname, topicname):
        """
        Remove and empty queue and topic from the msg router
        """        
        # delete the command queue
        self._brokerAgent.delQueue(qname, False, False)

        # then the topic, there is no pipeline anymore logging does not have target
        # anymore, It is a topic so no need to force the remova;
        self._brokerAgent.delExchange(topicname)

    def _init_state_from_file_if_possible(self):
        """
        If a statefile exists, load the stae from this file
        """
        # if no statefile exist do nothing
        if not os.path.exists(self._state_file_path):
            self.logger.info("No Statefile found. Starting with a clear state")
            return

        # Open the statefile
        file = open(self._state_file_path, 'r')
        try:
            state = pickle.load(file)
        except:
             self.logger.error("Failed loading existing statefile, creating"
                               " bak version and restart from a fresh state")

             os.rename(self._state_file_path, self._state_file_path + "_bak")
             return

        # TODO: do not check for correct state or anything, simple assign
        self._registered_pipelines = state
        self.logger.info("succesfully loaded the Deamon state from statefile")
        self.logger.info("--- List of reloaded uuid:     ---")


        for key, value in self._registered_pipelines.items():

            self.logger.info(key)
        self.logger.info("--- End List of reloaded uuid ---")  

if __name__ == "__main__":
    daemon = MCQDaemon( 1, "daemon_state_file.pkl",40)
    
    daemon.run()
        


