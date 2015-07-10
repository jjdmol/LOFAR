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
import pickle
import os

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.CQConfig as CQConfig

# programmatically interact with the qpid broker
from qpid.messaging import Connection	
from qpidtoollibs import BrokerAgent


# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

class MCQDaemon(object):
    def __init__(self, loop_interval=10, state_file_path=None, init_delay=5):
        self.logger = logging.getLogger("MCQDaemon")

        self._init_delay = init_delay        # how many loop polls te wait 
        self._loop_interval = loop_interval  # perform loop max once per 
                                             #loop_interval
        
        self._registered_pipelines = {}          
        self._registered_nodes = {}

        # Connect to the command queue
        self._broker = CQConfig.broker
        self._CommandQueue = msgbus.FromBus(
                   CQConfig.create_masterCommandQueue_name(),
              options = "create:always, node: { type: queue, durable: True}",
              broker = self._broker)

        # check for existing statefile
        # before it is asumed that the pipeline that called init has died before
        # registering as a consumer
        self._state_file_path = state_file_path  
        self._init_state_from_file_if_possible()

        # Needed for getting information on queues 
        self._connection = Connection.establish(self._broker)
        self._brokerAgent = BrokerAgent(self._connection)

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

    def _process_commands(self):
        """
        Process in order all commands in the command queue
        """     
        while True:
            # Test if the timeout is in milli seconds or second
            msg = self._CommandQueue.get(0.1)  # get is blocking, use timeout.
            if msg == None:
               break    # exit msg processing

            # currently the expected payload is a dict
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
                self._process_run_job(msg_content)
                self._CommandQueue.ack(msg)      

            elif command == 'quit':
                self._process_quit_msg(msg_content)
                self._CommandQueue.ack(msg)                         
                return True  # do NOT save the current state, might be cleared due to
              # this command

            else:
                self.logger.warn("***** warning **** encountered unknown command")
                self.logger.warn(msg_content)
                self._CommandQueue.ack(msg)  # ack but not do anything

            # **********************************************************
            # After each command we need to save the new state.
            # If the deamon goes down between the processing of the command
            # and this save we have an indetermined state, this cannot be
            # prevented
            self._save_state_to_file()
            # **********************************************************

    # TODO: Quit msg to all the slaves.        
    def _process_quit_msg(self, msg_content):
        """
        Perform actions done on receiveing quit msg

        1. If clear_state is set the state_file is removed
        """
        for uuid in self._registered_pipelines.keys():
            self._delete_queues_and_session(uuid)

        if msg_content['clear_state'] == "true":
            # Send a stop msg to all registered slaves
            for uuid in self._registered_pipelines.keys():
                self._delete_queues_and_session(uuid)
            try:
                os.remove(self._state_file_path)
            except:
                # The daemon could be in a state where the file has not been written
                # eg.  in the init phase.  If the delete fails just skip and continue
                pass

    def _process_run_job(self, msg_content):
        """
        The meat of the Master node Daemon functionality.

        The starting of a job on one of the node servers.
        """
        # extract the job parameters from the msg
        # This information is need to perform local work and to know where
        # to send the information 
        uuid = msg_content['uuid']
        node = msg_content['parameters']['node']

        # perform the master functionality before starting a job:
        self._check_slave_and_connect(node)    
        self._check_session_and_start(node, uuid)  # start session on the slave

        msg = CQConfig.create_run_job_msg(msg_content,
                            "MCQDaemon", node)

        self._registered_nodes[node]['CQObject'].send(msg)
        self.logger.info("send job to: {0}".format(
                                self._registered_nodes[node]['CQName']))   

    def _process_start_session_msg(self, msg_content):
        """
        _process_start_session_msg called when a new session is requested.
        This function creates the needed queue and topic based on the uuid.
        It is also responsible to for storing the details of the created objects
        in the internal storage of the Deamon object
        """      
        uuid = msg_content['uuid']
        self.logger.info("started session with uuid: {0}".format(uuid))

        # create the queues names based on the template
        resultq_name = CQConfig.create_returnQueue_name(uuid)
        topic_name = CQConfig.create_logTopic_name(uuid)

        # store them in the internal pipeline storage
        self._registered_pipelines[uuid] = {
                  'resultq':resultq_name,
                  'topic':topic_name,
                  'init_wait':self._init_delay,
                  'session_nodes':[]}


        # init_wait:other option is a time out
        # the problem is that the pipeline can only connect to an existing q
        # so after creating the q is not emediately 'used'.
        # its either a wait or a state or we have a countdown to allow late
        #connections

  
    def _process_stop_msg(self, msg_content):
        """
        Stop the current session.
        """       
        self.logger.info("Receive stop msg "
                         "deleting session with uuid: {0}".format(
                                            msg_content['uuid']))

        self._delete_queues_and_session(msg_content['uuid'])

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
        # Warning  With a large amount if queues and topic this might get slow!!!!!
        # TODO: This function feels smells. Might need some cleanup
        # First get al the registered queues and topics on the broker
        queues = self._brokerAgent.getAllQueues()
        topics = self._brokerAgent.getAllExchanges()
        binding = self._brokerAgent.getAllBindings()

        # Covert to name to queue object lists
        raw_name_to_queue_dict = {}
        for queue in queues:
            raw_name_to_queue_dict[queue.name] = queue

        raw_name_to_topic_dict = {}
        for topic in topics:
            raw_name_to_topic_dict[topic.name] = topic

        # Loop all the session uuid on the broker, find the accompanying topic or
        # queue on the broker
        broker_sessions_data = {}
        for uuid in self._registered_pipelines.keys():
            
            session_dict = {}
            # first match the uuid with the queue
            queue_found = False
            for name, queue in raw_name_to_queue_dict.items(): 
                return_queue_name = CQConfig.create_returnQueue_name(uuid)
                if return_queue_name in name:
                    session_dict['queue_name'] = name
                    session_dict['queue_object'] = queue
                    queue_found = True
                    break

            if not queue_found:
                # Could not find a registered session as queue on the broker
                # TODO: Just remove the session from internal storage.
                # this could happen when all a queue is manually removed.
                raise Exception(
                     "Could not find a registered pipeline session queue on"
                     " the broker, the internal daemon state is corrupt!")

            # next the topic
            topic_found = False
            for name, topic in  raw_name_to_topic_dict.items():
                log_topic_name = CQConfig.create_logTopic_name(uuid)
                if log_topic_name in name:
                     session_dict['topic_name'] = name
                     session_dict['topic_object'] = topic
                     topic_found = True
                     break

            if not topic_found:
                # TODO: THis is not correct, just remove the session???
                # Could not find a registered session as topic on the broker
                raise Exception("Could not find a registered pipeline session topic on"
                                " the broker, the internal daemon state is corrupt!")

            # Now store all our collected information in an easy dict
            broker_sessions_data[uuid] = session_dict

        return broker_sessions_data

    def _get_check_and_clear_known_queues(self):
        """
        Check all registered session queue for listeners
        If this is zero, the queues will not be read anymore and the process died

        Clear the messages in the queues and delete these
        """
        broker_queueus = self._get_queue_and_topic_from_broker()

        for (uuid, session_dict) in broker_queueus.items():
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

    def _send_quit_msg_to_slave(self, uuid, node):
        """
        Send a stop command for session uuid on node.

        This will result in all jobs for this session to be stopped
        """
        stop_msg_content = {'command': 'stop_session',
                             'uuid':uuid}
        # Create the msg based on some template in the CQConfig
        msg = CQConfig.create_stop_session_msg(
              stop_msg_content, "MCQDaemon", node)

        self.logger.info("sending stop_session to node")
        self._registered_nodes[node]['CQObject'].send(msg)

    def _delete_queues_and_session(self, uuid):
        """
        Deletes the queue and topic for this uuid and removes it from the
        internal storage
        """
        # first check if the the queues might have been removed:
        # due to the nature of message, the delete might be called a second time
        if uuid not in self._registered_pipelines.keys():
            return

        # remove the q and topic
        qname = self._registered_pipelines[uuid]['resultq']
        topicname = self._registered_pipelines[uuid]['topic']

        # send the quit msg to NodeDaemons
        for node in self._registered_pipelines[uuid]['session_nodes']:
            self._send_quit_msg_to_slave(uuid, node)
            CQConfig.delete_queue_on_node(node, qname)
            CQConfig.delete_queue_on_node(node, topicname, is_topic=True)


 
        self.logger.info("Deleted queues for session uuid: {0}".format(uuid))
        # delete the entry from the actual dict
        del self._registered_pipelines[uuid]

        self._delete_queue_and_topic(qname, topicname)


    def _create_to_from_and_link(self, from_node, to_node, 
                                 queue_name, is_topic=False):
        """
        Creates nodes on both from and to node, and add a forwarding rule/link
        """
        # Create the queue on the from_node
        self.logger.info("Creating from_node queue")
        CQConfig.create_queue_on_node(from_node, queue_name,is_topic)

        # Create the queue on the to node
        self.logger.info("Creating to_node queue")
        CQConfig.create_queue_on_node(to_node, queue_name, is_topic)

        # now link between the nodes
        self.logger.info("Creating link")
        CQConfig.create_queue_forward(from_node, to_node, queue_name, is_topic)


    def _check_slave_and_connect(self, node):
        """
        Checks if a node is known and created a msg queue pair between
        master and node if needed

        """
        self.logger.info("Check slave state: {0}".format(node))
        if node not in self._registered_nodes.keys():
            self.logger.debug(
                "received job for unconnected slave: {0}".format(node))

            # Create the queue on the local host
            nodeQueueName = CQConfig.create_nodeCommandQueue_name(node)

            # create a connection between master and node.
            self._create_to_from_and_link(CQConfig.head_node,
                                    node, nodeQueueName)

            bus = msgbus.ToBus(nodeQueueName, 
                options = "create:always, node: { type: queue, durable: True}",
                broker = self._broker)

            self._registered_nodes[node]={'CQName': nodeQueueName,
                                          'CQObject': bus}

    def _check_session_and_start(self, node, uuid):
        """
        Checks if a node is connect and created a msg queue connect if needed
        """
        if uuid not in self._registered_pipelines.keys():
            return  # TODO: THis is an error state!!! This should be handled

        if node not in self._registered_pipelines[uuid]['session_nodes']:
            self.logger.info("Starting node session on: {0}".format(node))
            
            # Create a return queue pair on node to master
            self._create_to_from_and_link(node, CQConfig.head_node,
                    CQConfig.create_returnQueue_name(uuid))

            # Create a return queue pair on node to master
            self._create_to_from_and_link(node, CQConfig.head_node,
                    CQConfig.create_logTopic_name(uuid), True)


            # Tell the node to be ready to receive jobs for this uuid
            msg_content = {'command': 'start_session',
                           'uuid':uuid}
            msg = CQConfig.create_start_session_msg(
                msg_content, "MCQDaemon", node)
                                  
            self._registered_nodes[node]['CQObject'].send(msg)

            # store that the node is active
            self._registered_pipelines[uuid]['session_nodes'].append(node)        

    def _delete_queue_and_topic(self, qname, topicname):
        """
        Remove and empty queue and topic from the msg router
        """   
        # If a queue prefix is used prepend this for the queue names
        # delete the command queue    
        # TODO: Ugly try catch to work around the queue prefix
        # used in the msgBus classes
        try:    
            self._brokerAgent.delQueue(qname, False, False)
        except:
            self._brokerAgent.delQueue(os.environ.get("QUEUE_PREFIX", "") +
                                       qname, False, False)

        # then the topic, there is no pipeline anymore logging does not have target
        # anymore, It is a topic so no need to force the remova;
        try:
            self._brokerAgent.delExchange(topicname)
        except:
            self._brokerAgent.delExchange(os.environ.get("QUEUE_PREFIX", "") + 
                                      topicname)


    # ***********************************************************************
    # Some helper functions. Could be refactored, but the statefile loading
    # is currently tightly bound to the internals of the class 
    def _save_state_to_file(self):
        """
        Save the internal session information to disk     
        """      
        file = open(self._state_file_path, 'w')
        pickle.dump(self._registered_pipelines, file)

        file.flush()  # force write to disk
        file.close()

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

    def _sleep(self, duration_loop_seconds):
        """
        Perform a sleep with the duration loop_interval - duration last loop
        """
        sleep_time = self._loop_interval - duration_loop_seconds

        self.logger.info("Starting sleep for {0} seconds".format(sleep_time))
        time.sleep(sleep_time)

if __name__ == "__main__":
    daemon = MCQDaemon( 1, "daemon_state_file.pkl",40)
    
    daemon.run()
        


