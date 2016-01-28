#!/usr/bin/env python

# ResourceAssigner.py: ResourceAssigner listens on the lofar ?? bus and calls onTaskSpecified
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: SpecifiedTaskListener.py 1580 2015-09-30 14:18:57Z loose $

"""
TaskSpecifiedListener listens to a bus on which specified tasks get published. It will then try
to assign resources to these tasks.
"""

from lofar.messaging.messagebus import AbstractBusListener
from lofar.messaging.RPC import RPC

import qpid.messaging
import logging
from datetime import datetime

import lofar.sas.resourceassignment.rarpc as rarpc

logger = logging.getLogger(__name__)


class SpecifiedTaskListener(AbstractBusListener):
  def __init__(self, busname='lofar.?.?', subject='?.?', broker=None, **kwargs):
    """
    SpecifiedTaskListener listens on the lofar ?? bus and calls onTaskSpecified
    :param address: valid Qpid address (default: lofar.otdb.status)
    :param broker: valid Qpid broker host (default: None, which means localhost)
    additional parameters in kwargs:
        options=   <dict>  Dictionary of options passed to QPID
        exclusive= <bool>  Create an exclusive binding so no other services can consume duplicate messages (default: False)
        numthreads= <int>  Number of parallel threads processing messages (default: 1)
        verbose=   <bool>  Output extra logging over stdout (default: False)
    """
    address = "%s/%s" % (busname, subject)
    super(OTDBBusListener, self).__init__(address, broker, **kwargs)

  def _handleMessage(self, msg):
    logger.debug("SpecifiedTaskListener.handleMessage: %s" %str(msg))

    taskId =  msg.content['treeID']
    modificationTime = datetime.utcnow()
    if 'time_of_change' in msg.content:
      try:
        modificationTime = datetime.strptime(msg.content['time_of_change'], '%Y-%m-%d %H:%M:%S.%f')
      except ValueError as e:
        logger.error('could not parse time_of_change %s : %s' % (msg.content['time_of_change'], e))

    if msg.content['specification']:
      self.onTaskSpecified(treeId, modificationTime, msg.content['specification'])
    else:
      logger.error("Task %s not properly received:" % (taskId, ))

__all__ = ["SpecifiedTaskListener"]

def parseSpecification(specification):
  default = "CEP2"
  cluster ="CEP4"
  return cluster

def getNeededResouces(specification):
  # Used settings
  ServiceName = "ToUpper"
  BusName     = "simpletest"
 
  # Initialize a Remote Procedure Call object
  with RPC(BusName,ServiceName) as remote_fn:
    replymessage, status = remote_fn("Hello World ToUpper.")
    print replymessage
    
def getAvailableResources(cluster):
  # Used settings
  ServiceName = "GetServerState"
  BusName     = "simpletest"
 
  groupnames = []
  available  = []
  with RPC(BusName, ServiceName) as GetServerState:
    replymessage, status = GetServerState.getactivegroupnames()
    if not status:
      groupnames = replymessage
    else:
      logger.error("T")
  if cluster in groupnames.keys():
    with RPC(BusName, ServiceName) as GetServerState:
      replymessage, status = GetServerState.gethostsforgid(groupnames[cluster])
      if not status:
        available = replymessage
      else:
        logger.error("T")
  else:
    logger.error("T")
  return available

def checkResources(needed, available):
  return True

def claimResources(needed):
  rarpc.InsertTask()

def commitResources(result.id):
  pass

def onTaskSpecified(treeId, modificationTime, specification):
  cluster = parseSpecification(specification)
  needed = getNeededResouces(specification)
  available = getAvailableResources(cluster)
  if checkResources(needed, available):
    result = claimResources(needed)
    if result.success:
      commitResources(result.id)
    else:
      SetTaskToCONFLICT(Task.)

def main(args=None):
  print "SpecifiedTaskListener started"
  
  service = SpecifiedTaskListener()
  service.startListening()

