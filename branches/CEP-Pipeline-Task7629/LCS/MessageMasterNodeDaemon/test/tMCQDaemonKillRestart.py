import lofar.messagebus.MCQDaemon as MCQ



if __name__ == "__main__":
    print "Hello world"

    if __name__ == "__main__":
        daemon = MCQ.MCQDaemon("daemon_state_file.pkl", 1, 2)

    daemon._commandQueue.append({'command':'quit',"clear_state":"true"})

    daemon.run()