import lofar.messagebus.MCQLib as MCQLib

import os       
import time

if __name__ == "__main__":
    MCQLib = MCQLib(logger)

    environment = dict(
            (k, v) for (k, v) in os.environ.iteritems()
                if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
        )

    parameters = {'node':'locus102',
                  #'cmd': '/home/klijn/build/7629/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/nodes/test_recipe.py',
                  'environment':environment,
                  'cmd': 'echo test',
                  #'cmd': """echo 'print "test"' | python """,
                  #'cmd':""" echo  "test" """,
                  'cdw': '/home/klijn',
                  'job_parameters':{'par1':'par1'}}



    MCQLib.run_job(parameters)

    MCQLib.run_job(parameters)
    # Connect to the HCQDaemon
    time.sleep(5)
    MCQLib._release()

    time.sleep(1)

    #if __name__ == "__main__":
    #    daemon = MCQ.MCQDaemon("daemon_state_file.pkl", 1, 2)


    ## we are testing
    ## put some commands in the queue

    ## skip one loop
    #daemon._commandQueue.append({'command':'no_msg'})  

    ## add a new pipeline session
    #daemon._commandQueue.append({'command':'start_session', 'uuid':"uuid_001"})

    ## skip one loop
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'add_consumer', 'uuid':"uuid_001"})

    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'no_msg'})
    ## delete the consumer on the session
    #daemon._commandQueue.append({'command':'del_consumer', 'uuid':"uuid_001"})
    ## This results in the session to be removed 
    #daemon._commandQueue.append({'command':'no_msg'})
    ## Create a second session: What happens if a queue is deleted when there are consumers???
    ## interesting
    #daemon._commandQueue.append({'command':'start_session', 'uuid':"uuid_002"})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'add_consumer', 'uuid':"uuid_002"})
    #daemon._commandQueue.append({'command':'no_msg'})
    #daemon._commandQueue.append({'command':'stop_session', 'uuid':"uuid_002"})
    #daemon._commandQueue.append({'command':'no_msg'})

    #daemon._commandQueue.append({'command':'quit',"clear_state":"true"})



    #daemon.run()