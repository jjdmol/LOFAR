import lofar.messagebus.MCQDaemon as MCQ



if __name__ == "__main__":
    print "Hello world"

    if __name__ == "__main__":
        daemon = MCQ.MCQDaemon("daemon_state_file.pkl", 1, 2)


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