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

    for key in MCQLib._running_jobs.keys():
        print MCQLib._running_jobs[key]['output']['output']
        if not MCQLib._running_jobs[key]['output']['status']:
            print "*******************************************************"
            print "We did not have a correct responce from the node recipe"
            MCQLib._release()
            sys.exit(1)

    MCQLib._release()


