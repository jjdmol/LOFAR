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


import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

import logging
# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("MessageBus")

class MCQDaemon(object):
  def __init__(self, state_file_path, loop_interval=10, init_delay=10):

    self._statefilepath = None
    self._loop_interval = loop_interval  # perform loop max once per loop_interval
    self._init_delay    = init_delay     # how many loop polls te wait before
                                         # it is asumed that the pipeline that called init has died before registering as a consumer

    self._state_file_path = state_file_path  # where to store the statefile
    self._registered_pipelines = {}      # dict of uuid -> (ResultQName, 
                                         #                  LogTopic)

    self._commandQueue = []              # place holder command queue

    self.broker="127.0.0.1" 

    # check for existing statefile
    self._init_state_from_file_if_possible()

    self.CommandQueue = msgbus.FromBus("username.LOCUS102.MCQueueDaemon.CommandQueue", 
          options = "create:always, node: { type: queue, durable: True}",
          broker = self.broker)

    self.CommandQueueTemp = msgbus.ToBus("username.LOCUS102.MCQueueDaemon.CommandQueue", 
          options = "create:always, node: { type: queue, durable: True}",
          broker = self.broker)


  def run(self):
    """
    Main loop of the daemon.
    While(True)

    1. Check all the 'connected' pipeline session ques for listeners
      a. Clear queues if no listeners

    2. Process all incomming commands

    3. Wait for x seconds

    """

    msgTemp = message.MessageContent(from_="USERRNAME.LOCUS102.MSQDaemon",
                      forUser="SERRNAME.LOCUS102.MSQDaemon.test",
                      summary="First msg to be send",
                      protocol="CommandQUeueMsg",
                      protocolVersion="0.0.1", 
                      #momid="",
                      #sasid="", 
                      #qpidMsg=None
                      )

    self.CommandQueueTemp.send(msgTemp)


    msgTemp2 = self.CommandQueue.get(1)  # get is blocking, always use timeout.



    while(True):   
      # ************** Main loop ***********************
      begin_tick = datetime.now()

      print "main processing loop start"
      # 1. check all registered pipeline queues for disconnects
      self._check_and_clear_known_queues()

      # 2. Process all incomming commands
      quit_command_received = self._process_commands()
      if (quit_command_received):
        print "*************** recveived quit command **************"
        break
      
      end_tick = datetime.now()   
      # ************** Main loop ***********************

      # 4.  perform a sleep,
      microseconds_per_second = 10e6
      duration_loop_seconds = (end_tick - begin_tick).microseconds \
                              / microseconds_per_second
      
      self._sleep(duration_loop_seconds)

  def _sleep(self, duration_loop_seconds):
    """
    Perform a sleep with the duration loop_interval - duration last loop
    """
    sleep_time = self._loop_interval - duration_loop_seconds

    print "Starting sleep for {0} seconds".format(sleep_time)
    time.sleep(sleep_time)


  def _check_and_clear_known_queues(self):
    """
    Check all registered session queue for listeners
    If this is zero, the queues will not be read anymore and the process died

    Clear the messages in the queues and delete these
    """
    print "  _check_and_clear_queue()"
    # Using items() as iterater allows deletion on the go
    for uuid in self._registered_pipelines.keys():
      # A queue might be in the init period (a grace period which allows an
      # pipelie some time to connect to the bus before the queues are removed)
      if self._registered_pipelines[uuid]["init_wait"] > 0:
        print "    init phase detected"
        self._registered_pipelines[uuid]["init_wait"] -= 1  
        continue
      
      # no listeners means the pipeline
      # died
      nr_listeners = self._get_number_of_queue_listener(uuid)

      # If the number of listeners is 0 there is no consumer of data anymore.
      # The results in the queue have no place to be stored. 
      if (nr_listeners == 0):
          self._delete_queues_and_session(uuid)

      # the state has changed, save it
      self._save_state_to_file()

  def _delete_queues_and_session(self, uuid):
    """
    Deletes the queue and topic for this uuid and removes it from the
    internal storage
    """
    # clear the q of msg
    self._clear_queue(uuid)
  
    # remove the q and topic
    self._delete_queue_and_topic(uuid)
  
    # delete the entry from the actual dict
    del self._registered_pipelines[uuid]

  def _get_number_of_queue_listener(self, uuid):
    """
    get from the result queue the number of listeners
    """
    print "    check number of consumers"

    # return the number of registered users
    # TODO: currently faked
    return self._registered_pipelines[uuid]['nr_consumers']

  def _clear_queue(self, uuid):
    """ 
    Remove waiting msg on a queue
    """
    # Get the correct queue based on the    
    print  "    _clear_and_delete_queue()"

  def _delete_queue_and_topic(self, uuid):
    """
    Remove and empty queue and topic from the msg router
    """
    # First the queue
    print "    _delete_queue_and_topic: delete queue"

    # then the topic, there is no pipeline anymore logging does not have target
    # anymore
    print "    _delete_queue_and_topic: delete topic"

    print "LOGGER: No active pipeline detected. session failed. Stopping logging"

  def _process_commands(self):
    """
    Process in order all commands in the command queue
    """
    print "  _process_commands()"
    for msg in self._commandQueue:

      # ****** test/temp code needs qpid implementation ******************
      # for test we add a temporary ack command to allow ack
     
      if msg['command'] == 'ack':
        # already processed: ack set, continue with next command
        continue

      if msg['command'] == 'no_msg':
        # no msg for this loop: ack and break loop
        msg['command'] = 'ack'
        print "    no msg in queue, wait"  # there are more msg, but this is testing
        break

      if msg['command'] == 'add_consumer':
        msg['command'] = 'ack'
        self._registered_pipelines[msg['uuid']]['nr_consumers'] = 1
        self._registered_pipelines[msg['uuid']]['init_wait'] = 0
        print "    adding consumer to session"
        continue

      if msg['command'] == 'del_consumer':
        msg['command'] = 'ack'
        self._registered_pipelines[msg['uuid']]['init_wait'] = 0
        self._registered_pipelines[msg['uuid']]['nr_consumers'] = 0
        print "    removing consumer from session"
        continue

      # ****** test/temp code needs qpid implementation ******************

      command = msg['command'] 
      if command == "start_session":
        self._process_init_msg(msg)
        msg['command'] = 'ack'

      elif command == "stop_session":
        print "      receive stop command _ deleting all queues"
        self._process_stop_msg(msg)
        msg['command'] = 'ack'

      elif command == 'run_session_job':
        self._process_start_job(msg)
        msg['command'] = 'ack'

      elif command == 'quit':
        self._process_quit_msg(msg)
                
        msg['command'] = 'ack'
        return True  # do NOT save the current state, might be cleared due to
        # this command

      else:
        msg['command'] = 'ack'
        print " ***** warning **** encountered unknown command"

      # After each command we need to save the new state.
      # If the deamon goes down between the processing of the command
      # and this save we have an indetermined state.
      self._save_state_to_file()

        
  def _process_quit_msg(self, msg):
    """

    """
    try:
      if msg['clear_state'] == "true":
        os.remove(self._state_file_path)

    except:
      # The daemon could be in a state where the file has not been written
      # eg. in the init phase. If the delete fails just skip and continue
      pass



  def _process_init_msg(self, msg):
    """

    """
    print "    _process_init_msg start command with uuid: {0}".format(msg['uuid'])
        
    # create the needed
    resultq_name, topic_name = self._create_resultq_and_topic(msg['uuid'])
    
    # store them in the internal pipeline storage
    self._registered_pipelines[msg['uuid']] = {
              'resultq':resultq_name,
              'topic':topic_name,
              'init_wait':self._init_delay,      # wait decreasing idx: wait for consumers
              'nr_consumers':0}    # test_option:other option is a time out
    # the problem is that the pipeline can only connect to an existing q
    # so after creating the it is not emediately 'used'.
    # its either a wait or a state.

  def _create_resultq_and_topic(self, uuid):
    """
    Creates a temporary named result queue and a topic based on the uuid
    Return the create queue and topic name
    """
    # create the queues using qpid

    resultq_name = "{0}.que.name".format(uuid)
    topic_name = "{0}.topic.name".format(uuid)


    print "      created resultsq: {0}".format(resultq_name)
    print "      created topic: {0}".format(topic_name)

    return resultq_name, topic_name
  
  def _process_stop_msg(self, msg):
    """
     Stop the current session.
    """
    uuid = msg['uuid']
    self._delete_queues_and_session(uuid)

  def _save_state_to_file(self):
    file = open(self._state_file_path, 'w')

    pickle.dump(self._registered_pipelines, file)

    file.flush()  # force write to disk
    file.close()

    print "  _saved_state_to_file()"

  def _init_state_from_file_if_possible(self):

    # if no statefile exist do nothing
    if not os.path.exists(self._state_file_path):
      return

    # Open the statefile 
    file = open(self._state_file_path, 'r')

    state = pickle.load(file)

    # TODO: do not check for correct state or anything, simple assign
    self._registered_pipelines = state
      

if __name__ == "__main__":
    daemon = MCQDaemon("daemon_state_file.pkl", 1, 2)


    # we are testing
    # put some commands in the queue

    # skip one loop
    daemon._commandQueue.append({'command':'no_msg'})  

    # add a new pipeline session
    daemon._commandQueue.append({'command':'start_session', 'uuid':"uuid_001"})

    # skip one loop
    daemon._commandQueue.append({'command':'no_msg'})
    daemon._commandQueue.append({'command':'add_consumer', 'uuid':"uuid_001"})

    daemon._commandQueue.append({'command':'no_msg'})
    daemon._commandQueue.append({'command':'no_msg'})
    daemon._commandQueue.append({'command':'no_msg'})
    daemon._commandQueue.append({'command':'no_msg'})
    daemon._commandQueue.append({'command':'no_msg'})
    daemon._commandQueue.append({'command':'no_msg'})
    # delete the consumer on the session
    daemon._commandQueue.append({'command':'del_consumer', 'uuid':"uuid_001"})
    # This results in the session to be removed 
    daemon._commandQueue.append({'command':'no_msg'})
    # Create a second session: What happens if a queue is deleted when there are consumers???
    # interesting
    daemon._commandQueue.append({'command':'start_session', 'uuid':"uuid_002"})
    daemon._commandQueue.append({'command':'no_msg'})
    daemon._commandQueue.append({'command':'add_consumer', 'uuid':"uuid_002"})
    daemon._commandQueue.append({'command':'no_msg'})
    daemon._commandQueue.append({'command':'stop_session', 'uuid':"uuid_002"})
    daemon._commandQueue.append({'command':'no_msg'})

    daemon._commandQueue.append({'command':'quit',"clear_state":"true"})



    daemon.run()

    

