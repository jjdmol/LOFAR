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

import lofar.messagebus.NCQLib as NCQLib


## Define logging. Until we have a python loging framework, we'll have
## to do any initialising here
#logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
#logger=logging.getLogger("MessageBus")

if __name__ == "__main__":

    pass
    #fromTopic = msgbus
    
    #NCQLibObject = NCQLib.QPIDLoggerHandler(

