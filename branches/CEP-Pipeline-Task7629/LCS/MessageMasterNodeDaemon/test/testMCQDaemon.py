# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

import uuid
import copy
import os
import logging
import time
import threading 
import pwd
import socket  # needed for username TODO: is misschien een betere manier os.environ['USER']
import signal

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
from qmf.console import Session as QMFSession

import lofar.messagebus.MCQLib as MCQLib


# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("MessageBus")

if __name__ == "__main__":
    MCQLib = MCQLib.MCQLib(logger)

    environment = dict(
            (k, v) for (k, v) in os.environ.iteritems()
                if k.endswith('PATH') or k.endswith('ROOT') or k == 'QUEUE_PREFIX'
        )

    parameters = {'node':'locus102',
                  #'cmd': '/home/klijn/build/7629/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/nodes/test_recipe.py',
                  'environment':environment,
                  'cmd': 'python /home/klijn/build/7629/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/nodes/test_recipe.py',
                  #'cmd': """echo 'print "test"' | python """,
                  #'cmd':""" echo  "test" """,
                  'cdw': '/home/klijn',
                  'job_parameters':{'par1':'par1'}}



    MCQLib.run_job(parameters)

    #MCQLib.run_job(parameters)
    # Connect to the HCQDaemon
    time.sleep(7)
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