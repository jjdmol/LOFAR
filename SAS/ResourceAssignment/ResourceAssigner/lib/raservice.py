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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: raservice.py 1580 2015-09-30 14:18:57Z loose $

"""
TaskSpecifiedListener listens to a bus on which specified tasks get published. It will then try
to assign resources to these tasks.
"""

import qpid.messaging
import logging
from datetime import datetime

from lofar.sas.resourceassignment.rataskspecified.RABusListener import RATaskSpecifiedBusListener
from lofar.messaging.RPC import RPC

import lofar.sas.resourceassignment.resourceassignmentservice.rpc as rarpc
from lofar.sas.resourceassignment.resourceassigner.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME
from lofar.sas.resourceassignment.resourceassigner.config import RATASKSPECIFIED_NOTIFICATION_BUSNAME, RATASKSPECIFIED_NOTIFICATIONNAME

logger = logging.getLogger(__name__)


class SpecifiedTaskListener(RATaskSpecifiedBusListener):
    def __init__(self,
                 busname=RATASKSPECIFIED_NOTIFICATION_BUSNAME,
                 subject=RATASKSPECIFIED_NOTIFICATIONNAME,
                 broker=None,
                 **kwargs):
        """
        SpecifiedTaskListener listens on the lofar ?? bus and calls onTaskSpecified
        :param busname: valid Qpid address (default: lofar.otdb.status)
        :param broker: valid Qpid broker host (default: None, which means localhost)
        additional parameters in kwargs:
                options=     <dict>    Dictionary of options passed to QPID
                exclusive= <bool>    Create an exclusive binding so no other services can consume duplicate messages (default: False)
                numthreads= <int>    Number of parallel threads processing messages (default: 1)
                verbose=     <bool>    Output extra logging over stdout (default: False)
        """
        super(SpecifiedTaskListener, self).__init__(busname=busname, subject=subject, broker=broker, **kwargs)

    def onTaskSpecified(self, sasId, modificationTime, resourceIndicators):
        logger.info('onTaskSpecified: sasId=%s' % sasId)

__all__ = ["SpecifiedTaskListener"]

def parseSpecification(specification):
    default = "CEP2"
    cluster ="CEP4"
    return cluster

def getNeededResouces(specification):
    # Used settings
    ServiceName = "ToUpper"
    BusName         = "simpletest"
 
    # Initialize a Remote Procedure Call object
    with RPC(BusName,ServiceName) as remote_fn:
        replymessage, status = remote_fn("Hello World ToUpper.")
        print replymessage

def getAvailableResources(cluster):
    # Used settings
    ServiceName = "GetServerState"
    BusName         = "simpletest"
 
    groupnames = []
    available    = []
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

def commitResources(result_id):
    pass

def onTaskSpecified(treeId, modificationTime, specification):
    cluster = parseSpecification(specification)
    needed = getNeededResouces(specification)
    available = getAvailableResources(cluster)
    if checkResources(needed, available):
        result = claimResources(needed)
        if result.success:
            commitResources(result.id)
            #SetTaskToSCHEDULED(Task.)
        else:
            #SetTaskToCONFLICT(Task.)
            pass

def main():
    from optparse import OptionParser
    from lofar.messaging import setQpidLogLevel
    from lofar.common.util import waitForInterrupt

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the resourceassigner service')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option("-b", "--busname", dest="busname", type="string", default=DEFAULT_BUSNAME, help="Name of the bus exchange on the qpid broker, default: %s" % DEFAULT_BUSNAME)
    parser.add_option("-s", "--servicename", dest="servicename", type="string", default=DEFAULT_SERVICENAME, help="Name for this service, default: %s" % DEFAULT_SERVICENAME)
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    (options, args) = parser.parse_args()

    setQpidLogLevel(logging.INFO)
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.DEBUG if options.verbose else logging.INFO)

    with SpecifiedTaskListener() as listener:
        waitForInterrupt()

if __name__ == '__main__':
    main()
