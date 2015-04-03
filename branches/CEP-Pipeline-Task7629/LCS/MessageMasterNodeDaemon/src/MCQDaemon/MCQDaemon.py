from datetime import datetime   # needed for duration 
import time

class MCQDaemon(object):
  def __init__(self, loop_interval=10, init_delay=10):
    self._statefilepath = None
    self._loop_interval = loop_interval  # perform loop max once per loop_interval
    self._init_delay    = init_delay     # how many loop polls te wait before
                                         # it is asumed that the pipeline that called init has died before registering as a consumer

    self._registered_pipelines = {}      # dict of uuid -> (ResultQName, 
                                         #                  LogTopic)

    self._commandQueue = []              # place holder command queue

    # check for existing statefile


  def run(self):
    """
    Main loop of the daemon.
    While(True)

    1. Check all the 'connected' pipeline session ques for listeners
      a. Clear queues if no listeners

    2. Process all incomming commands

    3. Save current statefile

    4. Wait for x seconds

    """
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

      # 3. Save current statefile
      self._save_state_to_file()
      
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

    print "  _check_and_clear_queue()"


    # Using items() as iterater allows deletion on the go
    for uuid in self._registered_pipelines.keys():
      # first check if the bus has registered listeners. None means the pipeline
      # died

      nr_listeners = self._get_number_of_queue_listener(uuid)

      # If the number of listeners is 0 there is no consumer of data anymore.
      # The results in the queue have no place to be stored. We could use

      if (nr_listeners == 0):
          self._delete_queues_and_session(uuid)

  def _delete_queues_and_session(self, uuid):
    # clear the q of msg
    self._clear_queue(uuid)
  
    # remove the q and topic
    self._delete_queue_and_topic(uuid)
  
    # delete the entry from the actual dict
    del self._registered_pipelines[uuid]

  def _get_number_of_queue_listener(self, uuid):
    """
    Check the result queue for the number us listeners and returns it
    """
    print "    check number of consumers"

    # check if we are still in the wait for consumer phase
    # this is the place were disappeared queue should be checked for.
    # could happen if daemon crashed between del of queue and save state in
    if self._registered_pipelines[uuid]["init_wait"] > 0:
      print "    init phase detected"
      self._registered_pipelines[uuid]["init_wait"] -= 1
      return 1

    # else return the number of registered users
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
        msg['command'] = 'ack'
        return True

      else:
        raise Exception("unknown command")

      # After each command we need to save the new state.
      self._save_state_to_file()

        
       

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

    print "  _save_state_to_file()"

  
      

if __name__ == "__main__":
    daemon = MCQDaemon(1, 2)


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

    daemon._commandQueue.append({'command':'quit'})



    daemon.run()

    

